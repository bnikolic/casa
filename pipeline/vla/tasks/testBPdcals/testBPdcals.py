from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure

from pipeline.hif.tasks import gaincal
from pipeline.vla.heuristics import getCalFlaggedSoln, getBCalStatistics


LOG = infrastructure.get_logger(__name__)

class testBPdcalsInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class testBPdcalsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(testBPdcalsResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    
        
class testBPdcals(basetask.StandardTaskTemplate):
    Inputs = testBPdcalsInputs

    def prepare(self):
        
        
        gtype_delaycal_result = self._do_gtype_delaycal()

        fracFlaggedSolns = 1.0
        m = self.inputs.context.observing_run.measurement_sets[0]
        critfrac = self.inputs.context.evla['msinfo'][m.name].critfrac

        #Iterate and check the fraciton of Flagged solutions, each time running gaincal in 'K' mode
        while (fracFlaggedSolns > critfac):
            ktype_delaycal_result = self._do_ktype_delaycal()
            flaggedSolnResult = getCalFlaggedSoln(ktype_delaycal_result.__dict__['inputs']['caltable'])
            fracFlaggedSolns = _check_flagSolns(flaggedSolnResult)
        
        
    def _do_gtype_delaycal(self):
        inputs = self.inputs

        context = self.inputs.context
        m = context.observing_run.measurement_sets[0]
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = context.evla['msinfo'][m.name].tst_delay_spw
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        
        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
        delaycal_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
            vis = inputs.vis,
            caltable = 'testdelayinitialgain.g',
            field    = delay_field_select_string,
            spw      = tst_delay_spw,
            solint   = 'int',
            refant   = inputs.refant.lower(),
            calmode  = 'p',
            minsnr   = 3.0,
            minblperant = minBL_for_cal,
            solnorm = False,
            combine = 'scan')

        delaycal_task = gaincal.GTypeGaincal(delaycal_inputs)

        return self._executor.execute(delaycal_task, merge=True)

    def _do_ktype_delaycal(self):
        inputs = self.inputs

        context = self.inputs.context
        m = context.observing_run.measurement_sets[0]
        delay_field_select_string = context.evla['msinfo'][m.name].delay_field_select_string
        tst_delay_spw = context.evla['msinfo'][m.name].tst_delay_spw
        delay_scan_select_string = context.evla['msinfo'][m.name].delay_scan_select_string
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal

        #need to add scan?
        #ref antenna string needs to be lower case for gaincal
        delaycal_inputs = gaincal.KTypeGaincal.Inputs(inputs.context,
            vis = inputs.vis,
            caltable = 'testdelay.k',
            field    = delay_field_select_string,
            spw      = tst_delay_spw,
            solint   = 'inf',
            refant   = inputs.refant.lower(),
            calmode  = 'p',
            minsnr   = 3.0,
            minblperant = minBL_for_cal,
            solnorm = False, 
            combine = 'scan')

        delaycal_task = gaincal.KTypeGaincal(delaycal_inputs)

        return self._executor.execute(delaycal_task, merge=True)

    def _check_flagSolns(self, flaggedSolnResult):

        context = self.inputs.context
        
        if (flaggedSolnResult['all']['total'] > 0):
            fracFlaggedSolns=flaggedSolnResult['antmedian']['fraction']
        else:
            fracFlaggedSolns=1.0

        refant_csvstring = context.observing_run.measurement_sets[0].reference_antenna
        refantlist = [x for x in refant_csvstring.split(',')]

        m = context.observing_run.measurement_sets[0]
        critfrac = context.evla['msinfo'][m.name].critfrac

        if (fracFlaggedSolns > critfrac):
            refantlist.pop(0)
            self.inputs.context.observing_run.measurement_sets[0].reference_antenna = ','.join(refantlist)
            LOG.info("Not enough good solutions, trying a different reference antenna.")
            LOG.info("The pipeline will use antenna "+refantlist[0]+" as the reference.")

        return fracFlaggedSolns
