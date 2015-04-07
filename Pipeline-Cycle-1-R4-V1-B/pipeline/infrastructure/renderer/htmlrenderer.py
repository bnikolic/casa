from __future__ import absolute_import
import atexit
import collections
import contextlib
import datetime
import operator
import os
import pydoc
import re
import shutil
import StringIO
import sys
import tempfile
import types
import zipfile

import casadef
import mako
import mako.lookup as lookup

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays.bandpass as bandpass
import pipeline.infrastructure.displays.image as image
import pipeline.infrastructure.displays.summary as summary
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.filenamer as filenamer
from pipeline.infrastructure.renderer import templates
from pipeline.infrastructure.renderer.templates import resources
from . import logger
from . import qa2adapter
from .. import utils
import pipeline.hif as hif
import pipeline.hsd as hsd

LOG = infrastructure.get_logger(__name__)


def get_task_description(result_obj):
    if not isinstance(result_obj, collections.Iterable):
        return get_task_description([result_obj,])
    
    if len(result_obj) is 0:
        msg = 'Cannot get description for zero-length results list'
        LOG.error(msg)
        return msg
    
    task_cls = result_obj[0].task
    if type(task_cls) is types.NoneType:
        results_cls = result_obj[0].__class__.__name__
        msg = 'No task registered on results of type %s' % results_cls
        LOG.warning(msg)
        return msg

    if task_cls is hif.tasks.Applycal:
        return 'Apply calibrations from context'

    if task_cls is hif.tasks.Atmflag:
        return 'Flag on atmospheric transmission'

    if task_cls is hif.tasks.FlagDeterALMA:
        return 'ALMA deterministic flagging'

    if task_cls is hif.tasks.GcorFluxscale:
        return 'Phased-up fluxscale'

    if task_cls is hif.tasks.GTypeGaincal:
        return 'G-type gain calibration'
    
    if task_cls is hif.tasks.ImportData:
        names = []
        for result in result_obj:
            names.extend([ms.basename for ms in result.mses])
        return 'Register %s with pipeline' % utils.commafy(names)

    if task_cls is hif.tasks.MakeCleanList:
        return 'Compile a list of cleaned images to be calculated'

    if task_cls is hif.tasks.NormaliseFlux:
        return 'Calculate mean fluxes of calibrators'

    if task_cls is hif.tasks.PhcorBandpass:
        return 'Phase-up bandpass calibration'

    if task_cls is hif.tasks.RefAnt:
        return 'Select reference antennas'

    if task_cls is hif.tasks.Setjy:
        return 'Set calibrator model visibilities'

    if task_cls is hsd.tasks.SDImportData:
        names = []
        for result in result_obj:
            names.extend([st.basename for st in result.scantables])
        return 'Register %s with pipeline' % utils.commafy(names)

    if task_cls is hsd.tasks.SDReduction:
        return 'Single-dish end-to-end reduction'
   
    if task_cls is hsd.tasks.SDExportData:
        return 'Single-dish SDExportData'

    if task_cls is hif.tasks.Tsyscal:
        return 'Calculate Tsys calibration'

    if task_cls is hif.tasks.Tsysflag:
        return 'Flag Tsys calibration'
    
    if task_cls is hif.tasks.Tsysflagchans:
        return 'Flag channels of Tsys calibration'

    if task_cls is hif.tasks.Wvrgcal:
        return 'Calculate wvr calibration'

    if task_cls is hif.tasks.Wvrgcalflag:
        return 'Calculate and flag wvr calibration'

    if task_cls is hsd.tasks.SDInspectData:
        datatable = result_obj[0].outcome['instance']
        names = datatable.getkeyword('FILENAMES')
        return 'Inspect %s'%(utils.commafy(names))

    if task_cls is hsd.tasks.SDCalTsys:
        return 'Generage Tsys calibration table'

    if task_cls is hsd.tasks.SDCalSky:
        return 'Generage Sky calibration table'

    if task_cls is hsd.tasks.SDApplyCal:
        return 'Apply calibration tables'

    if task_cls is hsd.tasks.SDImaging:
        return 'Image single dish data'

    if task_cls is hsd.tasks.SDBaseline:
        return 'Subtract spectral baseline'

    if LOG.isEnabledFor(LOG.todo):
        LOG.todo('No task description for \'%s\'' % task_cls.__name__)
        return ('\'%s\' (developers should add a task description)'
                '' % task_cls.__name__)
    
    return ('\'%s\'' % task_cls.__name__)

def get_task_name(result_obj):
    if not isinstance(result_obj, collections.Iterable):
        return get_task_name([result_obj,])
    
    if len(result_obj) is 0:
        msg = 'Cannot get task name for zero-length results list'
        LOG.error(msg)
        return msg
    
    task_cls = result_obj[0].task
    if type(task_cls) is types.NoneType:
        results_cls = result_obj[0].__class__.__name__
        msg = 'No task registered on results of type %s' % results_cls
        LOG.warning(msg)
        return msg
    
    return task_cls.__name__

def get_stage_number(result_obj):
    if not isinstance(result_obj, collections.Iterable):
        return get_stage_number([result_obj,])
    
    if len(result_obj) is 0:
        msg = 'Cannot get stage number for zero-length results list'
        LOG.error(msg)
        return msg
    
    return result_obj[0].stage_number

def get_plot_dir(context, stage_number):
    stage_dir = os.path.join(context.report_dir, 'stage%d' % stage_number)
    plots_dir = os.path.join(stage_dir, 'plots')
    return plots_dir


class StdOutFile(StringIO.StringIO):
    def close(self):
        sys.stdout.write(self.getvalue())
        StringIO.StringIO.close(self)


class Session(object):
    def __init__(self, mses=[], name='Unnamed Session'):
        self.mses = mses
        self.name = name
    
    @staticmethod
    def get_sessions(context):
        d = collections.defaultdict(list)
        for ms in context.observing_run.measurement_sets:
            d[ms.session].append(ms)

        return [Session(v, k) for k,v in d.items()]


class RendererBase(object):
    """
    Base renderer class.
    """
    @classmethod
    def rerender(cls, context):
        return True
    
    @classmethod
    def get_path(cls, context):
        return os.path.join(context.report_dir, cls.output_file)
    
    @classmethod
    def get_file(cls, context, hardcopy):
        path = cls.get_path(context)
        file_obj = open(path, 'w') if hardcopy else StdOutFile()
        return contextlib.closing(file_obj)

    @classmethod
    def render(cls, context, hardcopy=False):
        # give the implementing class a chance to bypass rendering. This is
        # useful when the page has not changed, eg. MS description pages when
        # no subsequent ImportData has been performed
        path = cls.get_path(context)
        if os.path.exists(path) and not cls.rerender(context):
            return

        with cls.get_file(context, hardcopy) as fileobj:
            template = TemplateFinder.get_template(cls.template)
            display_context = cls.get_display_context(context)
            fileobj.write(template.render(**display_context))


class TemplateFinder(object):
    _templates_dir = os.path.dirname(templates.__file__)
    _tmp_directory = tempfile.mkdtemp()
    LOG.trace('Mako working directory: %s' % _tmp_directory)
    # Remove temporary Mako codegen directory on termination of Python process 
    atexit.register(lambda: shutil.rmtree(TemplateFinder._tmp_directory,
                                          ignore_errors=True))
    _lookup = lookup.TemplateLookup(directories=[_templates_dir], 
                                    module_directory=_tmp_directory)

    @staticmethod
    def get_template(filename, **kwargs):
        return TemplateFinder._lookup.get_template(filename)


class T1_1Renderer(RendererBase):
    """
    T1-1 OUS Splash Page renderer
    """
    output_file = 't1-1.html'
    template = 't1-1.html'

    @staticmethod
    def get_display_context(context):
        obs_start = context.observing_run.start_datetime
        obs_end = context.observing_run.end_datetime

        project_ids = ', '.join(context.observing_run.project_ids)        
        schedblock_ids = ', '.join(context.observing_run.schedblock_ids)
        execblock_ids = ', '.join(context.observing_run.execblock_ids)
        observers = ', '.join(context.observing_run.observers)
        
        array_names = set([ms.antenna_array.name 
                           for ms in context.observing_run.measurement_sets])

        # pipeline execution start, end and duration
        exec_start = context.results[0].timestamps.start
        exec_end = context.results[-1].timestamps.end
        # remove unnecessary precision for execution duration
        dt = exec_end - exec_start
        exec_duration = datetime.timedelta(days=dt.days, seconds=dt.seconds)

        out_fmt = '%Y-%m-%d %H:%M:%S'        
        return {'pcontext'       : context,
                'casa_version'   : casadef.casa_version,
                'casa_revision'  : casadef.subversion_revision,
                'obs_start'      : obs_start.strftime(out_fmt),
                'obs_end'        : obs_end.strftime(out_fmt),
                'array_names'    : utils.commafy(array_names),
                'exec_start'     : exec_start.strftime(out_fmt),
                'exec_end'       : exec_end.strftime(out_fmt),
                'exec_duration'  : str(exec_duration),
                'project_ids'    : project_ids,
                'schedblock_ids' : schedblock_ids,
                'execblock_ids'  : execblock_ids,
                'observers'      : observers}


class T1_2Renderer(RendererBase):
    """
    T1-2 Observation Summary renderer
    """
    output_file = 't1-2.html'
    template = 't1-2.html'

    @staticmethod
    def get_display_context(context):
        bands = set()
        science_freqs = []
        for ms in context.observing_run.measurement_sets:
            science_spws = ms.get_spectral_windows(science_windows_only=True)
            science_freqs.extend([spw.ref_frequency for spw in science_spws])
            bands.update([spw.band for spw in science_spws])
        bands = sorted(bands)
        
        science_sources = set()
        for ms in context.observing_run.measurement_sets:
            science_sources.update([s.name for s in ms.sources 
                                    if 'TARGET' in s.intents])
        science_sources = sorted(science_sources)

        calibrators = set()
        for ms in context.observing_run.measurement_sets:
            calibrators.update([s.name for s in ms.sources 
                                if 'TARGET' not in s.intents])
        calibrators = sorted(calibrators)

        min_baseline = min((ms.antenna_array.min_baseline 
                            for ms in context.observing_run.measurement_sets),
                           key=lambda b : b.length)
        max_baseline = max((ms.antenna_array.max_baseline 
                            for ms in context.observing_run.measurement_sets),
                            key=lambda b : b.length)
        
        return {'pcontext'        : context,
                'mses'            : context.observing_run.measurement_sets,
                'science_sources' : utils.commafy(science_sources),
                'calibrators'     : utils.commafy(calibrators),
                'science_bands'   : utils.commafy(bands),
                'min_baseline'    : min_baseline,
                'max_baseline'    : max_baseline}


class T1_3MRenderer(RendererBase):
    """
    T1-3M renderer
    """
    output_file = 't1-3.html'
    template = 't1-3m.html'
    
    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)

        return {'pcontext'   : context,
                'qa2adapter' : qa2results}


class T1_4MRenderer(RendererBase):
    """
    T1-4M renderer
    """
    output_file = 't1-4.html'
    # TODO get template at run-time
    template = 't1-4m.html'
    
    @staticmethod
    def get_display_context(context):
        return {'pcontext' : context,
                'results'  : context.results}
        

class T2_1Renderer(RendererBase):
    """
    T2-4M renderer
    """
    output_file = 't2-1.html'
    template = 't2-1.html'

    @staticmethod
    def get_display_context(context):
        sessions = Session.get_sessions(context)
        return {'pcontext' : context,
                'sessions' : sessions}


class T2_1DetailsRenderer(object):
    output_file = 't2-1_details.html'
    template = 't2-1_details.html'

    @classmethod
    def get_file(cls, context, session, ms, hardcopy):
        ms_dir = os.path.join(context.report_dir, 
                              'session%s' % session.name,
                              ms.basename)
        if hardcopy and not os.path.exists(ms_dir):
            os.makedirs(ms_dir)
        filename = os.path.join(ms_dir, cls.output_file)
        file_obj = open(filename, 'w') if hardcopy else StdOutFile()
        return contextlib.closing(file_obj)

    @staticmethod
    def get_display_context(context, ms):
        inputs = summary.IntentVsTimeChart.Inputs(context, vis=ms.basename)
        task = summary.IntentVsTimeChart(inputs)
        intent_vs_time = task.plot()

        inputs = summary.FieldVsTimeChart.Inputs(context, vis=ms.basename)
        task = summary.FieldVsTimeChart(inputs)
        field_vs_time = task.plot()
        
        return {'pcontext'       : context,
                'ms'             : ms,
                'intent_vs_time' : intent_vs_time,
                'field_vs_time'  : field_vs_time  }

    @classmethod
    def render(cls, context, hardcopy=False):
        for session in Session.get_sessions(context):
            for ms in session.mses:
                with cls.get_file(context, session, ms, hardcopy) as fileobj:
                    template = TemplateFinder.get_template(cls.template)
                    display_context = cls.get_display_context(context, ms)
                    fileobj.write(template.render(**display_context))


class MetadataRendererBase(RendererBase):
    @classmethod
    def rerender(cls, context):
        # TODO: only rerender when a new ImportData result is queued
        if cls in DEBUG_CLASSES:
            LOG.warning('Always rerendering %s' % cls.__name__)
            return True
        return False


class T2_2_1Renderer(MetadataRendererBase):
    """
    T2-2-1 renderer
    """
    output_file = 't2-2-1.html'
    template = 't2-2-1.html'

    @staticmethod
    def get_display_context(context):
        mosaics = []
        for ms in context.observing_run.measurement_sets:
            for source in ms.sources:
                num_pointings = len([f for f in ms.fields 
                                     if f.source_id == source.id])
                if num_pointings > 1:
                    task = summary.MosaicChart(context, ms, source)
                    mosaics.append((ms, source, task.plot()))

        return {'pcontext' : context,
                'mses'     : context.observing_run.measurement_sets,
                'mosaics'  : mosaics}


class T2_2_2Renderer(MetadataRendererBase):
    """
    T2-2-2 renderer
    """
    output_file = 't2-2-2.html'
    template = 't2-2-2.html'

    @staticmethod
    def get_display_context(context):
        return {'pcontext' : context,
                'mses'     : context.observing_run.measurement_sets}


class T2_2_3Renderer(MetadataRendererBase):
    """
    T2-2-3 renderer
    """
    output_file = 't2-2-3.html'
    template = 't2-2-3.html'

    @staticmethod
    def get_display_context(context):
        plot_ants = []
        for ms in context.observing_run.measurement_sets:
            task = summary.PlotAntsChart2(context, ms)
            plot_ants.append((ms, task.plot()))
        
        return {'pcontext'  : context,
                'plot_ants' : plot_ants,
                'mses'      : context.observing_run.measurement_sets}


class T2_2_4Renderer(MetadataRendererBase):
    """
    T2-2-4 renderer
    """
    output_file = 't2-2-4.html'
    template = 't2-2-4.html'

    @staticmethod
    def get_display_context(context):
        azel_plots = []
#        for ms in context.observing_run.measurement_sets:
#            task = summary.AzElChart(context, ms)
#            azel_plots.append((ms, task.plot()))

        el_vs_time_plots = []
#        for ms in context.observing_run.measurement_sets:
#            task = summary.ElVsTimeChart(context, ms)
#            el_vs_time_plots.append((ms, task.plot()))

        return {'pcontext'         : context,
                'azel_plots'       : azel_plots,
                'el_vs_time_plots' : el_vs_time_plots}


class T2_2_5Renderer(MetadataRendererBase):
    """
    T2-2-5 renderer
    """
    output_file = 't2-2-5.html'
    template = 't2-2-5.html'

    @staticmethod
    def get_display_context(context):
        weather_plots = []
        for ms in context.observing_run.measurement_sets:
            task = summary.WeatherChart(context, ms)
            weather_plots.append((ms, task.plot()))

        return {'pcontext'         : context,
                'weather_plots'    : weather_plots}


class T2_3_1MRenderer(RendererBase):
    """
    Renderer for T2-3-1M: the QA2 calibration section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-1m.html'
    # the template file for this renderer
    template = 't2-3-1m.html'

    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)
        
        # we only need to pass the calibration section through to the template
        qa2section = qa2results.sections[qa2adapter.CalibrationQA2Section]

        return {'pcontext'   : context,
                'qa2section' : qa2section}


class T2_3_2MRenderer(RendererBase):
    """
    Renderer for T2-3-2M: the QA2 line finding section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-2m.html'
    # the template file for this renderer
    template = 't2-3-2m.html'

    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)
        
        # we only need to pass the calibration section through to the template
        qa2section = qa2results.sections[qa2adapter.LineFindingQA2Section]

        return {'pcontext'   : context,
                'qa2section' : qa2section}


class T2_3_3MRenderer(RendererBase):
    """
    Renderer for T2-3-3M: the QA2 flagging section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-3m.html'
    # the template file for this renderer
    template = 't2-3-3m.html'

    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)
        
        # we only need to pass the calibration section through to the template
        qa2section = qa2results.sections[qa2adapter.FlaggingQA2Section]

        return {'pcontext'   : context,
                'qa2section' : qa2section}


class T2_3_4MRenderer(RendererBase):
    """
    Renderer for T2-3-4M: the QA2 imaging section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-4m.html'
    # the template file for this renderer
    template = 't2-3-4m.html'

    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)
        
        # we only need to pass the calibration section through to the template
        qa2section = qa2results.sections[qa2adapter.ImagingQA2Section]

        return {'pcontext'   : context,
                'qa2section' : qa2section}


class T2_3MDetailsDefaultRenderer(object):
    def __init__(self, template='t2-3m_details.html', always_rerender=False):
        self.template = template
        self.always_rerender = always_rerender
        
    def get_display_context(self, context, result):
        return {'pcontext' : context,
                'result'   : result}

    def render(self, context, result):
        display_context = self.get_display_context(context, result)
        t = TemplateFinder.get_template(self.template)
        return t.render(**display_context)


class T2_3MDetailsRenderer(object):
    # the filename component of the output file. While this is the same for
    # all results, the directory is stage-specific, so there's no risk of
    # collisions  
    output_file = 't2-3m_details.html'
    
    # the default renderer used should the task:renderer mapping not specify a
    # specialised renderer
    _default_renderer = T2_3MDetailsDefaultRenderer()

    """
    Get the file object for this renderer.

    The returned file object will be either direct output to a file or to
    stdout, depending on whether hardcopy is set to true or false
    respectively. 

    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :param hardcopy: render to disk (true) or to stdout (false). Default is
        true
    :type hardcopy: boolean
    :rtype: a file object
    """
    @classmethod
    def get_file(cls, context, result, hardcopy):
        # construct the relative filename, eg. 'stageX/t2-3m_details.html'
        path = cls.get_path(context, result)

        # to avoid any subsequent file not found errors, create the directory
        # if a hard copy is requested and the directory is missing
        stage_dir = os.path.dirname(path)
        if hardcopy and not os.path.exists(stage_dir):
            os.makedirs(stage_dir)
        
        # create a file object that writes to a file if a hard copy is 
        # requested, otherwise return a file object that flushes to stdout
        file_obj = open(path, 'w') if hardcopy else StdOutFile()
        
        # return the file object wrapped in a context manager, so we can use
        # it with the autoclosing 'with fileobj as f:' construct
        return contextlib.closing(file_obj)

    """
    Get the path to which the template will be written.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :rtype: string
    """
    @classmethod
    def get_path(cls, context, result):
        # HTML output will be written to the directory 'stageX' 
        stage = 'stage%s' % result.stage_number
        stage_dir = os.path.join(context.report_dir, stage)

        # construct the relative filename, eg. 'stageX/t2-4m_details.html'
        return os.path.join(stage_dir, cls.output_file)

    """
    Render the detailed QA2 perspective of each Results in the given context.
    
    This renderer creates detailed T2_3M output for each Results. Each Results
    in the context is passed to a specialised renderer, which generates
    custom output and plots for the Result in question.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param hardcopy: render to disk (true) or to stdout (false). Default is true.
    :type hardcopy: boolean
    """
    @classmethod
    def render(cls, context, hardcopy=False):
        # get the map of t2_3m renderers from the dictionary
        t2_3m_renderers = renderer_map[T2_3MDetailsRenderer]
        
        # for each result accepted and stored in the context..
        for result in context.results:
            # we only handle lists of results, so wrap single objects in a
            # list if necessary
            if not isinstance(result, collections.Iterable):
                l = basetask.ResultsList()
                l.append(result)
                l.timestamps = result.timestamps
                l.inputs = result.inputs
                l.stage_number = result.stage_number
                result = l
            task = result[0].task

            # find the renderer appropriate to the task..
            renderer = t2_3m_renderers.get(task, cls._default_renderer)
            LOG.trace('Using %s to render %s result' % (
                renderer.__class__.__name__, task.__name__))
            
            # details pages do not need to be updated once written
            path = cls.get_path(context, result)
            force_rerender = getattr(renderer, 'always_rerender', False)
            if os.path.exists(path) and not force_rerender:
                continue
            
            # .. get the file object to which we'll render the result
            with cls.get_file(context, result, hardcopy) as fileobj:
                # .. and write the renderer's interpretation of this result to
                # the file object  
                fileobj.write(renderer.render(context, result))


class T2_3MDetailsWvrgcalflagRenderer(T2_3MDetailsDefaultRenderer):
    """
    T2_43DetailsBandpassRenderer generates the QA2 output specific to the
    Wvrgcalflag task.
    """
    
    def __init__(self, template='t2-3m_details-wvrgcalflag.html',
                 always_rerender=False):
        # set the name of our specialised Mako template via the superclass
        # constructor 
        super(T2_3MDetailsWvrgcalflagRenderer, self).__init__(template,
                                                              always_rerender)

    """
    Get the Mako context appropriate to the results created by a Bandpass
    task.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param results: the Wvrgcalflag results to describe
    :type results: 
        :class:`~pipeline.infrastructure.tasks.wvrgcal.resultobjects.WvrgcalflagResults`
    :rtype a dictionary that can be passed to the matching bandpass Mako 
        template
    """
    def get_display_context(self, context, results):
        # get the standard Mako context from the superclass implementation 
        super_cls = super(T2_3MDetailsWvrgcalflagRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        plots_dir = os.path.join(context.report_dir, 
                                 'stage%d' % results.stage_number)
        if not os.path.exists(plots_dir):
            os.mkdir(plots_dir)

        plotter = image.ImageDisplay()
        plots = []
        for result in results:
            if result.qa2.view:
                plot = plotter.plot(context, result.qa2, reportdir=plots_dir, 
                                    prefix='qa2', change='WVR')
                plots.append(plot)
    
        # Group the Plots by axes and plot types; each logical grouping will
        # be contained in a PlotGroup  
        plot_groups = logger.PlotGroup.create_plot_groups(plots)
        for plot_group in plot_groups:
            # Write the thumbnail pages for each plot grouping to disk 
            renderer = PlotGroupRenderer(context, results, plot_group, 'qa2')
            plot_group.filename = renderer.filename
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'plot_groups' : plot_groups})
        return ctx


class T2_3MDetailsBandpassRenderer(T2_3MDetailsDefaultRenderer):
    """
    T2_43DetailsBandpassRenderer generates the QA2 output specific to the
    bandpass task.
    """
    
    def __init__(self, template='t2-3m_details-bandpass.html',
                 always_rerender=False):
        # set the name of our specialised Mako template via the superclass
        # constructor 
        super(T2_3MDetailsBandpassRenderer, self).__init__(template,
                                                           always_rerender)

    """
    Get the Mako context appropriate to the results created by a Bandpass
    task.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param results: the bandpass results to describe
    :type results: 
        :class:`~pipeline.infrastructure.tasks.bandpass.common.BandpassResults`
    :rtype a dictionary that can be passed to the matching bandpass Mako 
        template
    """
    def get_display_context(self, context, results):
        # get the standard Mako context from the superclass implementation 
        super_cls = super(T2_3MDetailsBandpassRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        plots = []
        flagged = []
        num_flagged_feeds = 0
        for result in results:
            # return early if there are no QA2 results
            if not result.qa2:
                return ctx
            adapter = qa2adapter.QA2BandpassAdapter(context, result)
            plots.append(adapter.amplitude_plots)
            plots.append(adapter.phase_plots)
            
            for antenna, feeds in adapter.flagged_feeds.items():
                flagged.append((os.path.basename(result.inputs['vis']),
                                antenna.identifier,
                                ', '.join(feeds)))
                num_flagged_feeds += len(feeds)

        # Group the Plots by axes and plot types; each logical grouping will
        # be contained in a PlotGroup  
        plot_groups = logger.PlotGroup.create_plot_groups(plots)
        # Write the thumbnail pages for each plot grouping to disk 
        for plot_group in plot_groups:
            renderer = QA2PlotRenderer(context, results, plot_group, 'qa2')
            plot_group.filename = renderer.filename 
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'plot_groups'       : plot_groups,
                    'flagged'           : flagged,
                    'num_flagged_feeds' : num_flagged_feeds})

        return ctx


class T2_4MRenderer(RendererBase):
    """
    T2-4M renderer
    """
    output_file = 't2-4m.html'
    template = 't2-4m.html'

    @staticmethod
    def get_display_context(context):
        return {'pcontext' : context,
                'results'  : context.results }


class T2_4MDetailsDefaultRenderer(object):
    def __init__(self, template='t2-4m_details-generic.html', 
                 always_rerender=False):
        self.template = template
        self.always_rerender = always_rerender
        
    def get_display_context(self, context, result):
        return {'pcontext' : context,
                'result'   : result,
                'stagelog' : self._get_stagelog(context, result),
                'taskhelp' : self._get_help(context, result)}

    def render(self, context, result):
        display_context = self.get_display_context(context, result)
        t = TemplateFinder.get_template(self.template)
        return t.render(**display_context)

    def _get_stagelog(self, context, result):
        """
        Read in the CASA log extracts from the file in the stage directory.
        """
        stagelog_path = os.path.join(context.report_dir,
                                     'stage%s' % result.stage_number,
                                     'casapy.log')

        if not os.path.exists(stagelog_path):
            return None
        
        with open(stagelog_path, 'r') as f:
            return ''.join([l.expandtabs() for l in f.readlines()])

    def _get_help(self, context, result):
        try:
            # get hif-prefixed taskname from the result from which we can
            # retrieve the XML documentation, otherwise fall back to the
            # Python class documentation              
            taskname = getattr(result, 'taskname', result[0].task)

            obj, _ = pydoc.resolve(taskname, forceload=0)
            page = pydoc.render_doc(obj)
            return '<pre>%s</pre>' % re.sub('\x08.', '', page)
        except Exception:
            return None


class PlotGroupRenderer(object):
    template = 'plotgroup.html'

    def __init__(self, context, result, plot_group, prefix=''):
        self.context = context
        self.result = result
        self.plot_group = plot_group
        self.prefix = prefix
        if self.prefix != '':
            self.prefix = '%s-' % prefix
        
    @property
    def dirname(self):
        stage = 'stage%s' % self.result.stage_number
        return os.path.join(self.context.report_dir, stage)
    
    @property
    def filename(self):
        filename = '%s%s-%s-thumbnails.html' % (self.plot_group.x_axis, 
                                                self.plot_group.y_axis,
                                                self.prefix)
        filename = filenamer.sanitize(filename)
        return filename
    
    @property
    def path(self):
        return os.path.join(self.dirname, self.filename)
    
    def get_file(self, hardcopy=True):
        if hardcopy and not os.path.exists(self.dirname):
            os.makedirs(self.dirname)
            
        file_obj = open(self.path, 'w') if hardcopy else StdOutFile()
        return contextlib.closing(file_obj)
    
    def _get_display_context(self):
        return {'pcontext'   : self.context,
                'result'     : self.result,
                'plot_group' : self.plot_group,
                'dirname'    : self.dirname}

    def render(self):
        display_context = self._get_display_context()
        t = TemplateFinder.get_template(self.template)
        return t.render(**display_context)


class QA2PlotRenderer(PlotGroupRenderer):
    template = 'qa2.html'

    def __init__(self, context, result, plot_group, prefix=''):
        super(QA2PlotRenderer, self).__init__(context, result, plot_group, 
                                                   prefix)
        json_path = os.path.join(context.report_dir, 
                                 'stage%s' % result.stage_number, 
                                 'qa2.json')
        
        if os.path.exists(json_path):
            with open(json_path, 'r') as json_file:
                self.json = json_file.readlines()[0]
        else:
            self.json = '{}'
         
    def _get_display_context(self):
        ctx = super(QA2PlotRenderer, self)._get_display_context()
        ctx.update({'json' : self.json});
        return ctx


class T2_4MDetailsWvrgcalflagRenderer(T2_4MDetailsDefaultRenderer):
    """
    T2_4MDetailsWvrgcalflagRenderer generates the detailed T2_4M-level plots
    and output specific to the wvrgcalflag task.
    """
    
    def __init__(self, template='t2-4m_details-hif_wvrgcalflag.html', 
                 always_rerender=False):
        # set the name of our specialised Mako template via the superclass
        # constructor 
        super(T2_4MDetailsWvrgcalflagRenderer, self).__init__(template,
                                                              always_rerender)

    """
    Get the Mako context appropriate to the results created by a Wvrgcalflag
    task.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param results: the bandpass results to describe
    :type results: 
        :class:`~pipeline.infrastructure.tasks.wvrgcalflag.resultobjects.WvrgcalflagResults`
    :rtype a dictionary that can be passed to the matching Mako template
    """
    def get_display_context(self, context, results):
        # get the standard Mako context from the superclass implementation 
        super_cls = super(T2_4MDetailsWvrgcalflagRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        plots_dir = os.path.join(context.report_dir, 
                                 'stage%d' % results.stage_number)
        if not os.path.exists(plots_dir):
            os.mkdir(plots_dir)

        plotter = image.ImageDisplay()
        plots = []
        for result in results:
            if result.view:
                plot = plotter.plot(context, result, reportdir=plots_dir, 
                                    prefix='flag', change='Flagging')
                plots.append(plot)

        # Group the Plots by axes and plot types; each logical grouping will
        # be contained in a PlotGroup  
        plot_groups = logger.PlotGroup.create_plot_groups(plots)
        for plot_group in plot_groups:
            # Write the thumbnail pages for each plot grouping to disk 
            renderer = PlotGroupRenderer(context, results, plot_group, 'flag')
            plot_group.filename = renderer.filename
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'plot_groups' : plot_groups})
        return ctx


class T2_4MDetailsBandpassRenderer(T2_4MDetailsDefaultRenderer):
    """
    T2_4MDetailsBandpassRenderer generates the detailed T2_4M-level plots and
    output specific to the bandpass calibration task.
    """
    
    def __init__(self, template='t2-4m_details-bandpass.html', 
                 always_rerender=False):
        # set the name of our specialised Mako template via the superclass
        # constructor 
        super(T2_4MDetailsBandpassRenderer, self).__init__(template, 
                                                           always_rerender)

    """
    Get the Mako context appropriate to the results created by a Bandpass
    task.
    
    This routine triggers the creation of the plots specific to the bandpass
    task, creating thumbnail pages as necessary. The returned dictionary  
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param results: the bandpass results to describe
    :type results: 
        :class:`~pipeline.infrastructure.tasks.bandpass.common.BandpassResults`
    :rtype a dictionary that can be passed to the matching bandpass Mako 
        template
    """
    def get_display_context(self, context, results):
        # get the standard Mako context from the superclass implementation 
        super_cls = super(T2_4MDetailsBandpassRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        # generate the bandpass-specific plots, collecting the Plot objects
        # returned by the plot generator 
        plots = []
        for result in results:
            inputs = bandpass.BandpassDisplay.Inputs(context, result)
            task = bandpass.BandpassDisplay(inputs)
            plots.append(task.plot())

        # Group the Plots by axes and plot types; each logical grouping will
        # be contained in a PlotGroup  
        plot_groups = logger.PlotGroup.create_plot_groups(plots)
        # Write the thumbnail pages for each plot grouping to disk 
        for plot_group in plot_groups:
            renderer = PlotGroupRenderer(context, results, plot_group)
            plot_group.filename = renderer.filename 
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'plot_groups' : plot_groups })

        return ctx


class T2_4MDetailsImportDataRenderer(T2_4MDetailsDefaultRenderer):
    def __init__(self, template='t2-4m_details-hif_importdata.html', 
                 always_rerender=True):
        super(T2_4MDetailsImportDataRenderer, self).__init__(template,
                                                             always_rerender)
        
    def get_display_context(self, context, result):
        super_cls = super(T2_4MDetailsImportDataRenderer, self)        
        ctx = super_cls.get_display_context(context, result)

        setjy_results = []
        for r in result:
            setjy_results.extend(r.setjy_results)

        measurements = []        
        for r in setjy_results:
            measurements.extend(r.measurements)

        num_mses = reduce(operator.add, [len(r.mses) for r in result])

        ctx.update({'flux_imported' : True if measurements else False,
                    'setjy_results' : setjy_results,
                    'num_mses'      : num_mses})

        return ctx


class T2_4MDetailsRenderer(object):
    # the filename component of the output file. While this is the same for
    # all results, the directory is stage-specific, so there's no risk of
    # collisions  
    output_file = 't2-4m_details.html'
    
    # the default renderer should the task:renderer mapping not specify a
    # specialised renderer
    _default_renderer = T2_4MDetailsDefaultRenderer()

    """
    Get the file object for this renderer.

    The returned file object will be either direct output to a file or to
    stdout, depending on whether hardcopy is set to true or false
    respectively. 

    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :param hardcopy: render to disk (true) or to stdout (false). Default is
        true
    :type hardcopy: boolean
    :rtype: a file object
    """
    @classmethod
    def get_file(cls, context, result, hardcopy):
        # construct the relative filename, eg. 'stageX/t2-4m_details.html'
        path = cls.get_path(context, result)

        # to avoid any subsequent file not found errors, create the directory
        # if a hard copy is requested and the directory is missing
        stage_dir = os.path.dirname(path)
        if hardcopy and not os.path.exists(stage_dir):
            os.makedirs(stage_dir)
        
        # create a file object that writes to a file if a hard copy is 
        # requested, otherwise return a file object that flushes to stdout
        file_obj = open(path, 'w') if hardcopy else StdOutFile()
        
        # return the file object wrapped in a context manager, so we can use
        # it with the autoclosing 'with fileobj as f:' construct
        return contextlib.closing(file_obj)

    """
    Get the path to which the template will be written.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :rtype: string
    """
    @classmethod
    def get_path(cls, context, result):
        # HTML output will be written to the directory 'stageX' 
        stage = 'stage%s' % result.stage_number
        stage_dir = os.path.join(context.report_dir, stage)

        # construct the relative filename, eg. 'stageX/t2-4m_details.html'
        return os.path.join(stage_dir, cls.output_file)

    """
    Render the detailed task-centric view of each Results in the given
    context.
    
    This renderer creates detailed, T2_4M output for each Results. Each
    Results in the context is passed to a specialised renderer, which 
    generates customised output and plots for the Result in question.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param hardcopy: render to disk (true) or to stdout (false). Default is true.
    :type hardcopy: boolean
    """
    @classmethod
    def render(cls, context, hardcopy=False):
        # get the map of t2_4m renderers from the dictionary
        t2_4m_renderers = renderer_map[T2_4MDetailsRenderer]

        # for each result accepted and stored in the context..
        for result in context.results:
            # we only handle lists of results, so wrap single objects in a
            # list if necessary
            if not isinstance(result, collections.Iterable):
                l = basetask.ResultsList()
                l.append(result)
                l.timestamps = result.timestamps
                l.stage_number = result.stage_number
                l.inputs = result.inputs
                if hasattr(result, 'taskname'):
                    l.taskname = result.taskname
                result = l
            task = result[0].task
            
            # find the renderer appropriate to the task..
            renderer = t2_4m_renderers.get(task, cls._default_renderer)
            LOG.trace('Using %s to render %s result' % (
                renderer.__class__.__name__, task.__name__))

            # details pages do not need to be updated once written unless the
            # renderer specifies that an update is required
            path = cls.get_path(context, result)
            force_rerender = getattr(renderer, 'always_rerender', False)
            if os.path.exists(path) and not force_rerender:
                continue
            
            # .. get the file object to which we'll render the result
            with cls.get_file(context, result, hardcopy) as fileobj:
                # .. and write the renderer's interpretation of this result to
                # the file object  
                try:
                    fileobj.write(renderer.render(context, result))
                except:
                    LOG.warning('Template generation failed for '
                                '%s.' % cls.__name__)
                    LOG.debug(mako.exceptions.text_error_template().render())
                    fileobj.write(mako.exceptions.html_error_template().render())


class WebLogGenerator(object):
    renderers = [T1_1Renderer,         # OUS splash page
                 T1_2Renderer,         # observation summary
                 T1_3MRenderer,        # QA2 summary
                 T1_4MRenderer,        # task summary
                 T2_1Renderer,         # session tree
                 T2_1DetailsRenderer,  # session details
                 T2_2_1Renderer,       # spatial setup
                 T2_2_2Renderer,       # spectral setup
                 T2_2_3Renderer,       # antenna setup
                 T2_2_4Renderer,       # sky setup
                 T2_2_5Renderer,       # weather
                 T2_3_1MRenderer,      # QA2 calibration
                 T2_3_2MRenderer,      # QA2 line finding
                 T2_3_3MRenderer,      # QA2 flagging
                 T2_3_4MRenderer,      # QA2 imaging
                 T2_3MDetailsRenderer, # QA2 details pages
                 T2_4MRenderer,        # task tree
                 T2_4MDetailsRenderer] # task details

    @staticmethod
    def copy_resources(context):
        outdir = os.path.join(context.report_dir, 'resources')
        
        # shutil.copytree complains if the output directory exists
        if os.path.exists(outdir):
            shutil.rmtree(outdir)
        
        # copy all uncompressed non-python resources to output directory
        src = os.path.dirname(resources.__file__)
        dst = outdir
        ignore_fn = shutil.ignore_patterns('*.zip','*.py','*.pyc','CVS*')
        shutil.copytree(src, 
                        dst, 
                        symlinks=False, 
                        ignore=ignore_fn)

        # unzip fancybox to output directory
        infile = os.path.join(src, 'fancybox.zip')
        z = zipfile.ZipFile(infile, 'r')        
        z.extractall(outdir)
    
    @staticmethod
    def render(context, hardcopy=True):
        # copy CSS, javascript etc. to weblog directory
        WebLogGenerator.copy_resources(context)

        # divide casalog into stages, copying into directories as needed
        LogCopier.write_stage_logs(context)

        for renderer in WebLogGenerator.renderers:
            try:
                LOG.trace('%s rendering...' % renderer.__name__)
                renderer.render(context, hardcopy)
            except Exception as e:
                LOG.exception('Error generating weblog: %s', e)


class LogCopier(object):
    """
    LogCopier copies and handles the CASA logs so that they may be referenced
    by the pipeline web logs. 
    
    Capturing the CASA log gives us a few problems:
    The previous log is renamed upon starting a new session. To be reliably
    referenced from the web log, we must give it an immutable name and copy it
    to a safe location within the web log directory.

    The user may want to view the web log at any time during a pipeline
    session. To avoid broken links to the CASA log, the log should be copied
    across to the web log location at the end of each task.

    Pipeline sessions may be interrupted and restored, resulting in multiple
    CASA logs for such sessions. These logs must be consolidated into one file
    alongside any previous log information.

    Adding HTML tags such as '<pre>' and HTML anchors causes the CASA log
    reader to render such entries as empty entries at the bottom of the log.
    The result is that you must scroll up to find the last log entry. To
    prevent this, we need to output anchors as CASA log comments, possibly
    timestamps, and then use javascript to navigate to the log location.
    """
    
    # Thanks to the unique timestamps in the CASA log, the implementation
    # turns out to be quite simple. Is a class overkill?
    
    @staticmethod
    def copy(context):
        output_file = os.path.join(context.report_dir, 'casapy.log')

        existing_entries = []
        if os.path.exists(output_file):
            with open(output_file, 'r') as weblog:
                existing_entries.extend(weblog.readlines())
                    
        # read existing log, appending any non-duplicate entries to our casapy
        # web log. This is Python 2.6 so we can't define the context managers
        # on the same line
        with open(output_file, 'a') as weblog:
            with open(casatools.log.logfile(), 'r') as casalog:
                to_append = [entry for entry in casalog 
                             if entry not in existing_entries]
            weblog.writelines(to_append)

    @staticmethod    
    def write_stage_logs(context):
        """
        Take the CASA log snippets attached to each result and write them to
        the appropriate weblog directory. The log snippet is deleted from the
        result after a successful write to keep the pickle size down. 
        """
        for result in context.results:
            if not hasattr(result, 'casalog'):
                continue

            stage_dir = os.path.join(context.report_dir,
                                     'stage%s' % result.stage_number)
            if not os.path.exists(stage_dir):                
                os.makedirs(stage_dir)

            stagelog_entries = result.casalog
            start = result.timestamps.start
            end = result.timestamps.end

            stagelog_path = os.path.join(stage_dir, 'casapy.log')
            with open(stagelog_path, 'w') as stagelog:
                LOG.debug('Writing CASA log entries for stage %s (%s -> %s)' %
                          (result.stage_number, start, end))                          
                stagelog.write(stagelog_entries)
                
            # having written the log entries, the CASA log entries have no 
            # further use. Remove them to keep the size of the pickle small
            delattr(result, 'casalog')

#    @staticmethod    
#    def write_stage_logs(context):
#        casalog = os.path.join(context.report_dir, 'casapy.log')
#
#        for result in context.results:
#            stage_dir = os.path.join(context.report_dir,
#                                     'stage%s' % result.stage_number)
#            stagelog_path = os.path.join(stage_dir, 'casapy.log')
#            if os.path.exists(stagelog_path):
#                LOG.trace('CASA log exists for stage %s, continuing..' 
#                          % result.stage_number)
##                continue
#
#            if not os.path.exists(stage_dir):                
#                os.makedirs(stage_dir)
#
#            # CASA log timestamps have seconds resolution, whereas our task
#            # timestamps have microsecond resolution. Cast down to seconds 
#            # resolution to make a comparison, taking care to leave the 
#            # original timestamp unaltered
#            start = result.timestamps.start.replace(microsecond=0)
#            end = result.timestamps.end.replace(microsecond=0)
#            end += datetime.timedelta(seconds=1)
#            
#            # get the hif_XXX command from the task attribute if possible,
#            # otherwise fall back to the Python class name accessible at
#            # taskname
#            task = result.taskname
#             
#            stagelog_entries = LogCopier._extract(casalog, start, end, task)
#            with open(stagelog_path, 'w') as stagelog:
#                LOG.debug('Writing CASA log entries for stage %s (%s -> %s)' %
#                          (result.stage_number, start, end))                          
#                stagelog.writelines(stagelog_entries)
#
#    @staticmethod
#    def _extract(filename, start, end, task=None):
#        with open(filename, 'r') as logfile:
#            rows = logfile.readlines()
#
#        # find the indices of the log entries recorded just after and just 
#        # before the end of task execution. We do this so that our subsequent
#        # search can begin these times, giving a more optimal search
#        pattern = re.compile('^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}')
#        timestamps = [pattern.match(r).group(0) for r in rows]
#        datetimes = [datetime.datetime.strptime(t, '%Y-%m-%d %H:%M:%S')
#                     for t in timestamps]
#        within_timestamps = [n for n, elem in enumerate(datetimes) 
#                             if elem > start and elem < end]            
#        start_idx, end_idx = within_timestamps[0], within_timestamps[-1]
#
#        # Task executions are bookended by statements log entries like this:
#        #
#        # 2013-02-15 13:55:47 INFO    hif_importdata::::casa+ ##########################################
#        #
#        # This regex matches this pattern, and therefore the start and end
#        # sections of the CASA log for this task 
#        pattern = re.compile('^.*?%s::::casa\+\t\#{42}$' % task)
#
#        # Rewinding from the starting timestamp, find the index of the task
#        # start log entry
#        for idx in range(within_timestamps[0], 0, -1):
#            if pattern.match(rows[idx]):
#                start_idx = idx
#                break
#
#        # looking forward from the end timestamp, find the index of the task
#        # end log entry
#        for idx in range(within_timestamps[-1], len(rows)-1):
#            if pattern.match(rows[idx]):
#                end_idx = min(idx+1, len(rows))
#                break
#
#        return rows[start_idx:end_idx]
            

# renderer_map holds the mapping of tasks to result renderers for
# task-dependent weblog sections. This lets us write customised output for
# each task type.
renderer_map = {
    T2_3MDetailsRenderer : {
        hif.tasks.Wvrgcalflag    : T2_3MDetailsWvrgcalflagRenderer( ),
#        hif.tasks.Bandpass       : T2_3MDetailsBandpassRenderer(),
    },
    T2_4MDetailsRenderer : {
        hif.tasks.Atmflag        : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_atmflag.html'),
        hif.tasks.Bandpass       : T2_4MDetailsBandpassRenderer(),
        hif.tasks.CleanList      : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_cleanlist.html'),
        hif.tasks.Fluxscale      : T2_4MDetailsDefaultRenderer('t2-4m_details-fluxscale.html'),
        hif.tasks.GcorFluxscale  : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_gfluxscale.html'),
        hif.tasks.ImportData     : T2_4MDetailsImportDataRenderer(),
        hif.tasks.MakeCleanList  : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_makecleanlist.html'),
        hif.tasks.NormaliseFlux  : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_normflux.html'),
        hif.tasks.RefAnt         : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_refant.html'),
        hif.tasks.Setjy          : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_setjy.html'),
        hif.tasks.Tsysflag       : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_tsysflag.html'),
        hif.tasks.Tsysflagchans  : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_tsysflagchans.html'),
        hif.tasks.Wvrgcal        : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_wvrgcal.html'),
        hif.tasks.Wvrgcalflag    : T2_4MDetailsWvrgcalflagRenderer(),
        hsd.tasks.SDReduction    : T2_4MDetailsDefaultRenderer('t2-4-singledish.html'),
        hsd.tasks.SDInspectData  : T2_4MDetailsDefaultRenderer('t2-4m_details-hsd_inspectdata.html'),
        hsd.tasks.SDCalTsys      : T2_4MDetailsDefaultRenderer('t2-4m_details-hsd_caltsys.html'),
        hsd.tasks.SDCalSky       : T2_4MDetailsDefaultRenderer('t2-4m_details-hsd_calsky.html'),
        hsd.tasks.SDImaging      : T2_4MDetailsDefaultRenderer('t2-4m_details-hsd_imaging.html')
    }
}

# adding classes to this tuple always rerenders their content, bypassing the
# cache or 'existing file' checks. This is useful for developing and debugging
# as you can just call WebLogGenerator.render(context) 
DEBUG_CLASSES = ()