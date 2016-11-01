from __future__ import absolute_import
import glob
import os
import re
import shutil
import string
import tarfile
import tempfile
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.h.tasks.applycal.applycal as applycal
# import pipeline.h.tasks.importdata.importdata as importdata
from pipeline.h.tasks.restoredata import restoredata
import pipeline.h.tasks.importdata.importdata as importdata
#import pipeline.h.tasks.finalcals.applycals as applycal


from pipeline.infrastructure import casa_tasks



# the logger for this module
LOG = infrastructure.get_logger(__name__)


class VLARestoreDataInputs(restoredata.RestoreDataInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, copytoraw=None, products_dir=None, rawdata_dir=None,
                 output_dir=None, session=None, vis=None, bdfflags=None, lazy=None, asis=None,
                 ocorr_mode=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    bdfflags = basetask.property_with_default('bdfflags', False)
    ocorr_mode = basetask.property_with_default('ocorr_mode', 'co')


class VLARestoreData(restoredata.RestoreData):

    Inputs = VLARestoreDataInputs

    def prepare(self):
        """
        Prepare and execute an export data job appropriate to the
        task inputs.
        """
        # Create a local alias for inputs, so we're not saying
        # 'self.inputs' everywhere
        inputs = self.inputs

        # Force inputs.vis and inputs.session to be a list.
        sessionlist = inputs.session
        if type(sessionlist) is types.StringType:
            sessionlist = [sessionlist, ]
        tmpvislist = inputs.vis
        if type(tmpvislist) is types.StringType:
            tmpvislist = [tmpvislist, ]
        vislist = []
        for vis in tmpvislist:
            if os.path.dirname(vis) == '':
                vislist.append(os.path.join(inputs.rawdata_dir, vis))
            else:
                vislist.append(vis)

        # Download ASDMs
        #   Download ASDMs from the archive or products_dir to rawdata_dir.
        #   TBD: Currently assumed done somehow

        # Download flag versions
        #   Download from the archive or products_dir to rawdata_dir.
        if inputs.copytoraw:
            inflagfiles = glob.glob(os.path.join(inputs.products_dir, \
                '*.flagversions.tgz'))
            for flagfile in inflagfiles:
                LOG.info('Copying %s to %s' % (flagfile, inputs.rawdata_dir))
                shutil.copy(flagfile, os.path.join(inputs.rawdata_dir,
                    os.path.basename(flagfile)))

        # Download calibration tables
        #   Download calibration files from the archive or products_dir to
        # rawdata_dir.
        if inputs.copytoraw:
            incaltables = glob.glob(os.path.join(inputs.products_dir, \
                '*.caltables.tgz'))
            for caltable in incaltables:
                LOG.info('Copying %s to %s' % (caltable, inputs.rawdata_dir))
                shutil.copy(caltable, os.path.join(inputs.rawdata_dir,
                    os.path.basename(caltable)))

        # Download calibration apply lists
        #   Download from the archive or products_dir to rawdata_dir.
        #   TBD: Currently assumed done somehow
        if inputs.copytoraw:
            inapplycals = glob.glob(os.path.join(inputs.products_dir, \
                '*.calapply.txt'))
            for applycal in inapplycals:
                LOG.info('Copying %s to %s' % (applycal, inputs.rawdata_dir))
                shutil.copy(applycal, os.path.join(inputs.rawdata_dir,
                    os.path.basename(applycal)))

        # Convert ASDMS assumed to be on disk in rawdata_dir. After this step
        # has been completed the MS and MS.flagversions directories will exist
        # and MS,flagversions will contain a copy of the original MS flags,
        # Flags.Original.
        #    TBD: Add error handling
        import_results = self._do_importasdm(sessionlist=sessionlist, vislist=vislist)

        #ocorr_mode = 'co' for VLA and do hanning smoothing
        for ms in self.inputs.context.observing_run.measurement_sets:
            hanning_results = self._do_hanningsmooth(ms.name)
            self._move_hanning(ms.name)


        # Restore final MS.flagversions and flags
        flag_version_name = 'Pipeline_Final'
        flag_version_list = self._do_restore_flags(\
            flag_version_name=flag_version_name)

        # Get the session list and the visibility files associated with
        # each session.
        session_names, session_vislists = self._get_sessions()

        # Restore calibration tables
        self._do_restore_caltables(session_names=session_names,
            session_vislists=session_vislists)

        # Import calibration apply lists
        self._do_restore_calstate()

        # Apply the calibrations.
        apply_results = self._do_applycal()

        # Return the results object, which will be used for the weblog
        return restoredata.RestoreDataResults(import_results, apply_results)

    def _do_importasdm(self, sessionlist, vislist):
        inputs = self.inputs
        # The asis is temporary until we get the EVLA / ALMA factoring
        # figured out.
        importdata_inputs = importdata.ImportData.Inputs(inputs.context,
            vis=vislist, session=sessionlist, save_flagonline=False,
            lazy=inputs.lazy, bdfflags=inputs.bdfflags, dbservice=False,
            asis='Receiver CalAtmosphere', ocorr_mode=inputs.ocorr_mode)
        importdata_task = importdata.ImportData(importdata_inputs)
        return self._executor.execute(importdata_task, merge=True)

    def _do_hanningsmooth(self, vis):
        # Currently for VLA hanning smoothing
        task = casa_tasks.hanningsmooth(vis=vis,
                                        datacolumn='data',
                                        outputvis='temphanning.ms')

        return self._executor.execute(task)

    def _move_hanning(self, vis):
        # Currently for VLA hanning smoothing
        try:
            LOG.info("Removing original VIS "+vis)
            shutil.rmtree(vis)
            LOG.info("Renaming temphanning.ms to "+vis)
            os.rename('temphanning.ms', vis)
        except:
            LOG.warn('Problem encountered with hanning smoothing.')

    def _do_applycal(self):
        inputs = self.inputs
        applycal_inputs = applycal.Applycal.Inputs(inputs.context)
        # Set the following to keep the default behavior. No longer needed ?
        # applycal_inputs.flagdetailedsum = True

        #Override for VLA
        applycal_inputs.intent = ''
        applycal_inputs.field = ''
        applycal_inputs.spw = ''

        applycal_task = applycal.Applycal(applycal_inputs)
        return self._executor.execute(applycal_task, merge=True)
