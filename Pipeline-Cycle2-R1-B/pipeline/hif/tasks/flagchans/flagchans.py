from __future__ import absolute_import

import collections
import numpy as np 
import os
import re
import types

import casa
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.hif.tasks.common import commonhelpermethods
from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter

from .resultobjects import FlagchansResults
from ..common import commonresultobjects
from ..common import viewflaggers

LOG = infrastructure.get_logger(__name__)


class FlagchansInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None, spw=None,
      intent=None,
      metric=None, flag_hilo=None, fhl_limit=None, fhl_minsample=None,
      flag_tmf=None, tmf_frac_limit=None, tmf_nchan_limit=None, niter=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def spw(self):
        if self._spw is not None:
            print 'property', self._spw, type(self._spw)
            return str(self._spw)

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('spw')

        science_spws = self.ms.get_spectral_windows(self._spw, with_channels=True)
        return ','.join([str(spw.id) for spw in science_spws])

    @spw.setter
    def spw(self, value):
        print 'setter', value, type(value)
        self._spw = value

    @property
    def intent(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('intent')

        if self._intent is not None:
            return self._intent
        
        # if the spw was set, look to see which intents were observed in that
        # spectral window and return the intent based on our order of
        # preference: BANDPASS, PHASE, AMPLITUDE
        preferred_intents = ('BANDPASS', 'PHASE', 'AMPLITUDE')
        if self.spw:
            for spw in self.ms.get_spectral_windows(self.spw):
                for intent in preferred_intents:
                    if intent in spw.intents:
                        if intent != preferred_intents[0]:
                            LOG.warning('%s spw %s: %s not present, %s used instead' %
                              (os.path.basename(self.vis), spw.id, 
                              preferred_intents[0], intent))
                        return intent
            
        # spw was not set, so look through the spectral windows
        for intent in preferred_intents:
            intentok = True
            for spw in self.ms.spectral_windows:
                if intent not in spw.intents:
                    intentok = False
                    break
                if not intentok:
                    break
            if intentok:
                if intent != preferred_intents[0]:
                    LOG.warning('%s %s: %s not present, %s used instead' %
                      (os.path.basename(self.vis), spw.id, preferred_intents[0],
                      intent))
                return intent
                
        # current fallback - return an empty intent
        return ''

    @intent.setter
    def intent(self, value):
        self._intent = value

    @property
    def metric(self):
        return self._metric

    @metric.setter
    def metric(self, value):
        if value is None:
           value = 'combined'
        self._metric = value

    @property
    def flag_hilo(self):
        return self._flag_hilo

    @flag_hilo.setter
    def flag_hilo(self, value):
        if value is None:
            value = True
        self._flag_hilo = value

    @property
    def fhl_limit(self):
        return self._fhl_limit

    @fhl_limit.setter
    def fhl_limit(self, value):
        if value is None:
            value = 10.0
        self._fhl_limit = value

    @property
    def fhl_minsample(self):
        return self._fhl_minsample

    @fhl_minsample.setter
    def fhl_minsample(self, value):
        if value is None:
            value = 5
        self._fhl_minsample = value

    @property
    def flag_tmf(self):
        if self._flag_tmf is None:
            return False
        return self._flag_tmf

    @flag_tmf.setter
    def flag_tmf(self, value):
        self._flag_tmf = value

    @property
    def tmf_frac_limit(self):
        if self._tmf_frac_limit is None:
            return 0.3
        return self._tmf_frac_limit

    @tmf_frac_limit.setter
    def tmf_frac_limit(self, value):
        self._tmf_frac_limit = value

    @property
    def tmf_nchan_limit(self):
        if self._tmf_nchan_limit is None:
            return 10
        return self._tmf_nchan_limit

    @tmf_nchan_limit.setter
    def tmf_nchan_limit(self, value):
        self._tmf_nchan_limit = value

    @property
    def niter(self):
        if self._niter is None:
            return 1
        return self._niter

    @niter.setter
    def niter(self, value):
        self._niter = value


class Flagchans(basetask.StandardTaskTemplate):
    Inputs = FlagchansInputs

    def prepare(self):
        inputs = self.inputs

        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = FlagchansWorkerInputs(context=inputs.context,
          output_dir=inputs.output_dir, vis=inputs.vis, spw=inputs.spw,
          intent=inputs.intent, metric=inputs.metric)
        datatask = FlagchansWorker(datainputs)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, inpfile=[], table=inputs.vis)
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Translate the input flagging parameters to a more compact
        # list of rules.
        rules = viewflaggers.MatrixFlagger.make_flag_rules (
          flag_hilo=inputs.flag_hilo, fhl_limit=inputs.fhl_limit,
          fhl_minsample=inputs.fhl_minsample,
          flag_tmf1=inputs.flag_tmf, tmf1_axis='Antenna2',
          tmf1_limit=inputs.tmf_frac_limit)
        flagger = viewflaggers.MatrixFlagger
 
        # Construct the flagger task around the data view task  and the
        # flagger task. When executed this will:
        #   loop:
        #     execute datatask to obtain view from underlying data
        #     examine view, raise flags
        #     execute flagsetter task to set flags in underlying data        
        #     exit loop if no flags raised or if # iterations > niter 
        flaggerinputs = flagger.Inputs(
          context=inputs.context, output_dir=inputs.output_dir,
          vis=inputs.vis, datatask=datatask, flagsettertask=flagsettertask,
          rules=rules, niter=inputs.niter)
        flaggertask = flagger(flaggerinputs)

	# Execute it to flag the data view
        result = self._executor.execute(flaggertask)
        return result

    def analyse(self, result):
        return result


class FlagchansWorkerInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, spw=None,
      intent=None, metric=None):
        self._init_properties(vars())


class FlagchansWorker(basetask.StandardTaskTemplate):
    Inputs = FlagchansWorkerInputs

    def __init__(self, inputs):
        super(FlagchansWorker, self).__init__(inputs)
        self.result = FlagchansResults()

    def _readfile(self, file):

        with open(file, 'r') as f:
            lines = f.readlines()
            for line in lines:
                if line[0] == '#':
                    # ignore comments
                    continue

                try:
                    words = line.split()
                    chan = int(words[0])
                    data = float(words[1])
                    spwid = int(words[11])
                    corr = words[12]
                    ant1 = int(words[5])
                    ant2 = int(words[6])

                    self._plotdata[(spwid,corr,ant1,ant2,chan)] = data
                except:
                    print 'fail:', line

    def prepare(self):
        inputs = self.inputs

        final = []

        self.result.vis = inputs.vis

        # Get the MS object.
        self.ms = inputs.context.observing_run.get_ms(name=self.result.vis)

        # Get the spws to use
        print 'spw', inputs.spw
        spwids = map(int, inputs.spw.split(','))

        # Get antenna names, ids
        self.antenna_name, self.antenna_ids = \
          commonhelpermethods.get_antenna_names(self.ms)

        LOG.info ('Computing flagging metrics for vis %s ' % (self.result.vis))

        # calculate views
        if inputs.metric == 'separate':
            self.calculate_separate_view(spwids, inputs.intent)
        elif inputs.metric == 'combined':
            self.calculate_combined_view(spwids, inputs.intent)
        else:
            raise Exception, 'bad metric: %s' % inputs.metric

        return self.result

    def analyse(self, result):
        return result

    def calculate_separate_view(self, spwids, intent):
        """
        spwid     -- view will be calculated using data for this spw id.
        """

        # the current view will be very similar to the last, if available.
        # For now approximate as being identical which will save having to
        # recalculate
        prev_descriptions = self.result.descriptions()
        if prev_descriptions:
            for description in prev_descriptions:
                prev_result = self.result.last(description)
                self.result.addview(description, prev_result)

            # EARLY RETURN
            return

        # plotms uses field and not intent
        fieldids = [field.id for field in self.ms.get_fields(intent=intent)]
        if not fieldids:
            LOG.error('no data for intent: %s' % intent)
            raise Exception, 'no data for intent: %s' % intent

        fieldids = ','.join([str(fieldid) for fieldid in fieldids])
        plotfile='%s_temp.txt' % os.path.basename(self.inputs.vis)
        print 'plotfile', plotfile

        # plotms export seems unreliable except for small bits of dataset
        # so this is inefficient but might work better in future
        self._plotdata = {}
        ants = np.array(self.antenna_ids)
        for spwid in spwids:
            for ant in ants:
                print 'plotting', ant
                casa.plotms(vis=self.inputs.vis, xaxis='channel',
                  yaxis='amp', field=fieldids, spw=str(spwid),
                  antenna=str(ant),
                  averagedata=True, avgtime='3600',
                  plotfile=plotfile, expformat='txt', overwrite=True,
                  showgui=False)

                print 'reading'
                self._readfile(plotfile)

        # now construct the views
        for spwid in spwids:
            corrs = commonhelpermethods.get_corr_products(self.ms, spwid)
            spw = self.ms.get_spectral_window(spwid)
            nchans = spw.num_channels

            axes = [
              commonresultobjects.ResultAxis(name='channels',
              units='', data=np.arange(nchans)),
              commonresultobjects.ResultAxis(name='Antenna2',
              units='id', data=np.arange(self.antenna_ids[-1]+1))]

            for corrlist in corrs:
                corr = corrlist[0]
                data = np.zeros([self.antenna_ids[-1]+1, nchans, 
                  self.antenna_ids[-1]+1])
                flag = np.ones([self.antenna_ids[-1]+1, nchans,
                  self.antenna_ids[-1]+1], np.bool)
                ants = np.array(self.antenna_ids)

                for ant1 in ants:
                    slice = np.zeros([nchans])
                    slice_flag = np.ones([nchans])

                    for ant2 in ants[ants > ant1]: 
                        for chan in range(nchans): 
                            try:
                                slice[chan] = \
                                  self._plotdata[(spwid,corr,ant1,ant2,chan)]
                                slice_flag[chan] = 0
                            except:
                                pass

                        slice -= np.median(slice)
                        data[ant1,:,ant2] = slice
                        data[ant2,:,ant1] = slice
                        flag[ant1,:,ant2] = slice_flag
                        flag[ant2,:,ant1] = slice_flag

                for ant1 in ants:
                    viewresult = commonresultobjects.ImageResult(
                      filename=self.inputs.vis, data=data[ant1],
                      flag=flag[ant1], axes=axes, datatype='Mean amplitude',
                      spw=spwid, pol=corr, ant=self.antenna_name[ant1])

                    # add the view results and their children results to the
                    # class result structure
                    self.result.addview(viewresult.description, viewresult)

        # tidy up
        os.system('rm -fr %s' % plotfile)

    def calculate_combined_view(self, spwids, intent):
        """
        spwids -- views will be calculated using data for each spw id
                  in this list.
        """

        # the current view will be very similar to the last, if available.
        # For now approximate as being identical which will save having to
        # recalculate
        prev_descriptions = self.result.descriptions()
        if prev_descriptions:
            for description in prev_descriptions:
                prev_result = self.result.last(description)
                self.result.addview(description, prev_result)

            # EARLY RETURN
            return

        # plotms uses field and not intent
        fieldids = [field.id for field in self.ms.get_fields(intent=intent)]
        if not fieldids:
            LOG.error('no data for intent: %s' % intent)
            raise Exception, 'no data for intent: %s' % intent

        fieldids = ','.join([str(fieldid) for fieldid in fieldids])
        spwstring = ','.join([str(spwid) for spwid in spwids])
        plotfile='%s_temp.txt' % os.path.basename(self.inputs.vis)
        print 'plotfile', plotfile

        # plotms export seems unreliable except for small bits of dataset
        # so this is inefficient but might work better in future
        self._plotdata = {}
        ants = np.array(self.antenna_ids)
        for spwid in spwids:
            for ant in ants:
                print 'plotting', ant
                casa.plotms(vis=self.inputs.vis, xaxis='channel',
                  yaxis='amp', field=fieldids, spw=str(spwid),
                  antenna=str(ant),
                  averagedata=True, avgtime='3600',
                  plotfile=plotfile, expformat='txt', overwrite=True,
                  showgui=False)

                print 'reading'
                self._readfile(plotfile)

        # now construct the views
        for spwid in spwids:
            corrs = commonhelpermethods.get_corr_products(self.ms, spwid)
            spw = self.ms.get_spectral_window(spwid)
            nchans = spw.num_channels

            ants = np.array(self.antenna_ids)
            baselines = []
            for ant1 in ants:
                for ant2 in ants: 
                    baselines.append('%s&%s' % (ant1, ant2))

            axes = [
              commonresultobjects.ResultAxis(name='channels',
              units='', data=np.arange(nchans)),
              commonresultobjects.ResultAxis(name='Baseline',
              units='', data=np.array(baselines), channel_width=1)]

            for corrlist in corrs:
                corr = corrlist[0]
                data = np.zeros([nchans, 
                  (self.antenna_ids[-1]+1) * (self.antenna_ids[-1]+1)])
                flag = np.ones([nchans, 
                  (self.antenna_ids[-1]+1) * (self.antenna_ids[-1]+1)], np.bool)

                # build the basic view
                for ant1 in ants:
                    slice = np.zeros([nchans])
                    slice_flag = np.ones([nchans])

                    for ant2 in ants[ants > ant1]:
                        for chan in range(nchans): 
                            try:
                                slice[chan] = \
                                  self._plotdata[(spwid,corr,ant1,ant2,chan)]
                                slice_flag[chan] = 0
                            except:
                                pass

                        slice -= np.median(slice)

                        ant_offset = ant1 * (ants[-1] + 1) + ant2
                        data[:,ant_offset] = slice
                        flag[:,ant_offset] = slice_flag

                        ant_offset = ant2 * (ants[-1] + 1) + ant1
                        data[:,ant_offset] = slice
                        flag[:,ant_offset] = slice_flag

                # refine the view by subtracting the median over all baselines
                # for each channel
                for chan in range(nchans):
                    data[chan,:] -= np.median(data[chan,:])

                for ant1 in self.antenna_ids:
                    viewresult = commonresultobjects.ImageResult(
                      filename=self.inputs.vis, data=data,
                      flag=flag, axes=axes, datatype='Mean amplitude',
                      spw=spwid, pol=corr, intent=intent)

                    # add the view results to the result structure
                    self.result.addview(viewresult.description, viewresult)

        # tidy up
        os.system('rm -fr %s' % plotfile)