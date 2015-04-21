from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary

import itertools

from pipeline.hif.tasks import gaincal
from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import applycal
from pipeline.hifv.heuristics import getCalFlaggedSoln, getBCalStatistics
import numpy as np
import pipeline.hif.heuristics.findrefant as findrefant

LOG = infrastructure.get_logger(__name__)

class SolintInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        
        self.gain_solint1 = 'int'
        self.gain_solint2 = 'int'

class SolintResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[], longsolint=None, gain_solint2=None, shortsol2=None, short_solint=None, new_gain_solint1=None):
        super(SolintResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.longsolint = longsolint
        self.gain_solint2 = gain_solint2
        
        self.shortsol2 = shortsol2
        self.short_solint = short_solint
        self.new_gain_solint1 = new_gain_solint1
        
    def merge_with_context(self, context):    
        m = context.observing_run.measurement_sets[0]
        context.evla['msinfo'][m.name].gain_solint2 = self.gain_solint2
        context.evla['msinfo'][m.name].longsolint = self.longsolint
        
        context.evla['msinfo'][m.name].shortsol2 = self.shortsol2
        context.evla['msinfo'][m.name].short_solint = self.short_solint
        context.evla['msinfo'][m.name].new_gain_solint1 = self.new_gain_solint1

class Solint(basetask.StandardTaskTemplate):
    Inputs = SolintInputs
    
    def prepare(self):
    
        #Solint section
        calMs = 'calibrators.ms'
        split_result = self._do_split(calMs)
        (longsolint, gain_solint2) = self._do_determine_solint(calMs)
        
        #Testgains section
        context = self.inputs.context
        tablebase = 'testgaincal'
        table_suffix = ['.g','3.g','10.g']
        soltimes = [1.0,3.0,10.0] 
        m = self.inputs.context.observing_run.measurement_sets[0]
        soltimes = [self.inputs.context.evla['msinfo'][m.name].int_time * x for x in soltimes]
        print soltimes
        
        solints = ['int', str(soltimes[1])+'s', str(soltimes[2])+'s']
        soltime = soltimes[0]
        solint = solints[0]
        shortsol1 = context.evla['msinfo'][m.name].shortsol1
        combtime = 'scan'

        refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis='calibrators.ms',field=refantfield,geometry=True,flagging=True, intent='', spw='')
        
        RefAntOutput=refantobj.calculate()
        
        refAnt=str(RefAntOutput[0])+','+str(RefAntOutput[1])+','+str(RefAntOutput[2])+','+str(RefAntOutput[3])


        bpdgain_touse = tablebase + table_suffix[0]
        testgains_result = self._do_gtype_testgains('calibrators.ms', 'testgaincal.g', solint=solint, context=context, combtime=combtime, refAnt=refAnt)

        flaggedSolnResult1 = getCalFlaggedSoln('testgaincal.g')
        LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult1['all']['fraction']))
        LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult1['antmedian']['fraction']))


        if (flaggedSolnResult1['all']['total'] > 0):
            fracFlaggedSolns1=flaggedSolnResult1['antmedian']['fraction']
        else:
            fracFlaggedSolns1=1.0

        shortsol2=soltime

        if (fracFlaggedSolns1 > 0.05):
            soltime = soltimes[1]
            solint = solints[1]

            testgains_result = self._do_gtype_testgains('calibrators.ms', 'testgaincal3.g', solint=solint, context=context, combtime=combtime, refAnt=refAnt)
            flaggedSolnResult3 = getCalFlaggedSoln('testgaincal3.g')
            
            LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult3['all']['fraction']))
            LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult3['antmedian']['fraction']))

            if (flaggedSolnResult3['all']['total'] > 0):
                fracFlaggedSolns3=flaggedSolnResult3['antmedian']['fraction']
            else:
                fracFlaggedSolns3=1.0

            if (fracFlaggedSolns3 < fracFlaggedSolns1):
                shortsol2 = soltime
                bpdgain_touse = tablebase + table_suffix[1]
            
                if (fracFlaggedSolns3 > 0.05):
                    soltime = soltimes[2]
                    solint = solints[2]

                    testgains_result = self._do_gtype_testgains('calibrators.ms', 'testgaincal10.g', solint=solint, context=context, combtime=combtime, refAnt=refAnt)
                    flaggedSolnResult10 = getCalFlaggedSoln(tablebase + table_suffix[2])
                    LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult3['all']['fraction']))
                    LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult3['antmedian']['fraction']))

                    if (flaggedSolnResult10['all']['total'] > 0):
                        fracFlaggedSolns10 = flaggedSolnResult10['antmedian']['fraction']
                    else:
                        fracFlaggedSolns10 = 1.0

                    if (fracFlaggedSolns10 < fracFlaggedSolns3):
                        shortsol2=soltime
                        bpdgain_touse = tablebase + table_suffix[2]

                        if (fracFlaggedSolns10 > 0.05):
                            solint='inf'
                            combtime=''
                            testgains_result = self._do_gtype_testgains('calibrators.ms', 'testgaincalscan.g', solint=solint, context=context, combtime=combtime, refAnt=refAnt)
                            flaggedSolnResultScan = getCalFlaggedSoln('testgaincalscan.g')
                            LOG.info("For solint = "+solint+" fraction of flagged solutions = "+str(flaggedSolnResult3['all']['fraction']))
                            LOG.info("Median fraction of flagged solutions per antenna = "+str(flaggedSolnResult3['antmedian']['fraction']))
                            
                            if (flaggedSolnResultScan['all']['total'] > 0):
                                fracFlaggedSolnsScan=flaggedSolnResultScan['antmedian']['fraction']
                            else:
                                fracFlaggedSolnsScan=1.0
                                
                            if (fracFlaggedSolnsScan < fracFlaggedSolns10):
                                shortsol2=context.evla['msinfo'][m.name].longsolint
                                bpdgain_touse = 'testgaincalscan.g'
                                
                                if (fracFlaggedSolnsScan > 0.05):
                                    LOG.warn("Warning, large fraction of flagged solutions, there might be something wrong with your data")

        
        short_solint=max(shortsol1,shortsol2)
        new_gain_solint1=str(short_solint)+'s'
        
        LOG.info("Using short solint = " + new_gain_solint1)
        
        return SolintResults(longsolint=longsolint, gain_solint2=gain_solint2, shortsol2=shortsol2, short_solint=short_solint, new_gain_solint1=new_gain_solint1)
    
    def analyse(self, results):
	return results
    
    def _do_split(self, calMs):
        
        m = self.inputs.context.observing_run.measurement_sets[0]
        channels = self.inputs.context.evla['msinfo'][m.name].channels
        calibrator_scan_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string
    
        LOG.info("Splitting out calibrators into calibrators.ms")
    
        task_args = {'vis'          : m.name,
                     'outputvis'    : calMs,
                     'datacolumn'   : 'corrected',
                     'field'        : '',
                     'spw'          : '',
                     'width'        : int(max(channels)),
                     'antenna'      : '',
                     'timebin'      : '0s',
                     'timerange'    : '',
                     'scan'         : calibrator_scan_select_string,
                     'intent'       : '',
                     'array'        : '',
                     'uvrange'      : '',
                     'correlation'  : '',
                     'observation'  : '',
                     'keepflags'    : False}
        
        job = casa_tasks.split(**task_args)
            
        return self._executor.execute(job)
    
    
    def _do_determine_solint(self, calMs):
        
        durations = []
        old_spws = []
        old_field = ''
        
        with casatools.MSReader(calMs) as ms:
            scan_summary = ms.getscansummary()    
            
            m = self.inputs.context.observing_run.measurement_sets[0]
            phase_scan_list = self.inputs.context.evla['msinfo'][m.name].phase_scan_list
            
            print phase_scan_list
            
            for kk in range(len(phase_scan_list)):
                ii = phase_scan_list[kk]
                
                try:
                    end_time = scan_summary[str(ii)]['0']['EndTime']
                    begin_time = scan_summary[str(ii)]['0']['BeginTime']
                    new_spws = scan_summary[str(ii)]['0']['SpwIds']
                    new_field = scan_summary[str(ii)]['0']['FieldId']
                    
                    #print end_time, begin_time, new_spws, new_field
                    
                    if ((kk > 0) and (phase_scan_list[kk-1] == ii-1) and (set(new_spws) == set(old_spws)) and (new_field == old_field)):
                        # if contiguous scans, just increase the time on the previous one
                        print "End time, old begin time", end_time, old_begin_time
                        durations[-1] = 86400*(end_time - old_begin_time)
                        #print "first durations: ", durations
                    else:
                        print "End time, begin time", end_time, begin_time
                        durations.append(86400*(end_time - begin_time))
                        old_begin_time = begin_time
                        print "append durations, old, begin:", durations, old_begin_time, begin_time
                    LOG.info("Scan "+str(ii)+" has "+str(durations[-1])+"s on source")
                    old_spws = new_spws
                    old_field = new_field
                
                except KeyError:
                    LOG.warn("WARNING: scan "+str(ii)+" is completely flagged and missing from calibrators.ms")
              
              
        longsolint = (np.max(durations))*1.01
        gain_solint2=str(longsolint)+'s'
                   
        return (longsolint, gain_solint2)

    
    def _do_gtype_testgains(self, calMs, caltable, solint='int', context=None, combtime='scan', refAnt=None):

        m = context.observing_run.measurement_sets[0]

        calibrator_scan_select_string=context.evla['msinfo'][m.name].calibrator_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal

        #Do this to get the reference antenna string
        #temp_inputs = gaincal.GTypeGaincal.Inputs(context)
        #refant = temp_inputs.refant.lower()

        task_args = {'vis'          : calMs,
                     'caltable'     : caltable,
                     'field'        : '',
                     'spw'          : '',
                     'intent'       : '',
                     'selectdata'   : True,
                     'scan'         : calibrator_scan_select_string,
                     'solint'       : solint,
                     'combine'      : combtime,
                     'preavg'       : -1.0,
                     'refant'       : refAnt.lower(),
                     'minblperant'  : minBL_for_cal,
                     'minsnr'       : 5.0,
                     'solnorm'      : False,
                     'gaintype'     : 'G',
                     'smodel'       : [],
                     'calmode'      : 'ap',
                     'append'       : False,
                     'gaintable'    : [''],
                     'gainfield'    : [''],
                     'interp'       : [''],
                     'spwmap'       : [],
                     'parang'       : False,
                     'async'        : False}

        
        job = casa_tasks.gaincal(**task_args)
            
        return self._executor.execute(job)