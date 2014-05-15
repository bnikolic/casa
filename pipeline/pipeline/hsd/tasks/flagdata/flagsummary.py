from __future__ import absolute_import

import os
import numpy
import time

import asap as sd
from taskinit import gentools

#from SDTool import ProgressTimer

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

from . import SDFlagPlotter as SDP
from .. import common

LOG = infrastructure.get_logger(__name__)


class SDFlagSummary(object):
    '''
    A class of single dish flagging task.
    This class defines per spwid flagging operation.
    '''

    def __init__(self, context, datatable, spwid_list, pollist, file_index, thresholds, flagRule, userFlag=[]):
        '''
        Constructor of worker class
        '''
        self.context = context
        self.datatable = datatable
        self.spwid_list = spwid_list
        self.pollist = pollist
        self.file_index = file_index
        self.thres_value = thresholds
        self.flagRule = flagRule
        self.userFlag = userFlag
    
    def execute(self, dry_run=True):
        """
        Summarizes flagging results.
        Iterates over antenna and polarization for a certain spw ID
        """
        start_time = time.time()

        datatable = self.datatable
        spwid_list = self.spwid_list
        pollist = self.pollist
        file_index = self.file_index
        thresholds = self.thres_value
        flagRule = self.flagRule
        #userFlag = self.userFlag

        assert len(file_index) == len(spwid_list)
        LOG.debug('Members to be processed:')
        for (a,s) in zip(file_index, spwid_list):
            LOG.debug('\tAntenna %s Spw %s Pol %s'%(a,s,pollist))
        
        # output directory
#         if self.context.subtask_counter is 0: 
#             stage_number = self.context.task_counter - 1
#         else:
#             stage_number = self.context.task_counter
        stage_number = self.context.task_counter
        FigFileDir = (self.context.report_dir+"/stage%d" % stage_number)
        ### WORKAROUND to GENERATE stage# dir manually
        if not os.path.exists(FigFileDir):
            os.mkdir(FigFileDir)
        FigFileDir += "/"

        flagSummary = []
        for (idx,spwid) in zip(file_index, spwid_list):
            LOG.debug('Performing flagdata for Antenna %s Spw %s'%(idx,spwid))
            st = self.context.observing_run[idx]
            filename_in = st.name
            ant_name = st.antenna.name
            asdm = common.asdm_name(st)
            LOG.info("*** Summarizing table: %s ***" % (os.path.basename(filename_in)))
            for pol in pollist:
                time_table = datatable.get_timetable(idx, spwid, pol)               
                # Select time gap list: 'subscan': large gap; 'raster': small gap
                if flagRule['Flagging']['ApplicableDuration'] == "subscan":
                    TimeTable = time_table[1]
                else:
                    TimeTable = time_table[0]
                flatiter = utils.flatten([ chunks[1] for chunks in TimeTable ])
                dt_idx = [ chunk for chunk in flatiter ]
                # generate summary plot
                st_prefix = st.name.rstrip('/').split('/')[-1].rstrip('\.asap').replace('\.', '_')
                iteration = _get_iteration(self.context.observing_run.reduction_group,
                                           idx, spwid, pol)
                FigFileRoot = ("FlagStat_%s_spw%d_pol%d_iter%d" % (st_prefix, spwid, pol, iteration))
                time_gap = datatable.get_timegap(idx, spwid, pol)
                # time_gap[0]: PosGap, time_gap[1]: TimeGap
                for i in range(len(thresholds)):
                    thres = thresholds[i]
                    if thres['index'] == idx and thres['spw'] == spwid and thres['pol'] == pol:
                        final_thres = thres['result_threshold']
                        thresholds.pop(i)
                        break
                t0 = time.time()
                htmlName, nflags = self.plot_flag(datatable, dt_idx, time_gap[0], time_gap[1], final_thres, flagRule, FigFileDir, FigFileRoot)
                t1 = time.time()
                LOG.info('Plot flags End: Elapsed time = %.1f sec' % (t1 - t0) )
                flagSummary.append({'html': htmlName, 'name': asdm, 'antenna': ant_name, 'spw': spwid, 'pol': pol,
                                    'nrow': len(dt_idx), 'nflags': nflags})

        end_time = time.time()
        LOG.info('PROFILE execute: elapsed time is %s sec'%(end_time-start_time))

        return flagSummary


    def _get_parmanent_flag_summary(self, pflag, FlagRule):
        # FLAG_PERMANENT[0] --- 'WeatherFlag'
        # FLAG_PERMANENT[1] --- 'TsysFlag'
        # FLAG_PERMANENT[2] --- 'UserFlag'
        types = ['WeatherFlag', 'TsysFlag', 'UserFlag']
        mask = 1
        for idx in range(len(types)):
            if FlagRule[types[idx]]['isActive'] and pflag[idx] == 0:
                mask = 0
                break
        return mask

    def _get_stat_flag_summary(self, tflag, FlagRule):
        # FLAG[0] --- 'LowFrRMSFlag' (OBSOLETE)
        # FLAG[1] --- 'RmsPostFitFlag'
        # FLAG[2] --- 'RmsPreFitFlag'
        # FLAG[3] --- 'RunMeanPostFitFlag'
        # FLAG[4] --- 'RunMeanPreFitFlag'
        # FLAG[5] --- 'RmsExpectedPostFitFlag'
        # FLAG[6] --- 'RmsExpectedPreFitFlag'
        types = ['RmsPostFitFlag', 'RmsPreFitFlag', 'RunMeanPostFitFlag', 'RunMeanPreFitFlag',
                 'RmsExpectedPostFitFlag', 'RmsExpectedPreFitFlag']
        mask = 1
        for idx in range(len(types)):
            if FlagRule[types[idx]]['isActive'] and tflag[idx+1] == 0:
                mask = 0
                break
        return mask

    def plot_flag(self, DataTable, ids, PosGap, TimeGap, threshold, FlagRule, FigFileDir, FigFileRoot):
        # Plot statistics
        NROW = len(ids)
        # Store data for plotting
        FlaggedRowsCategory = [[],[],[],[],[],[],[],[],[]]
        FlaggedRows = []
        PermanentFlag = []
        NPpdata = numpy.zeros((7,NROW), numpy.float)
        NPpflag = numpy.zeros((7,NROW), numpy.int)
        NPprows = numpy.zeros((2,NROW), numpy.int)
        N = 0
        for ID in ids:
            row = DataTable.getcell('ROW', ID)
            # Check every flags to create summary flag
            tFLAG = DataTable.getcell('FLAG',ID)
            tPFLAG = DataTable.getcell('FLAG_PERMANENT',ID)
            tTSYS = DataTable.getcell('TSYS',ID)
            tSTAT = DataTable.getcell('STATISTICS',ID)

            # permanent flag
            Flag = self._get_parmanent_flag_summary(tPFLAG, FlagRule)
            PermanentFlag.append(Flag)
            # FLAG_SUMMARY
            Flag *= self._get_stat_flag_summary(tFLAG, FlagRule)
            if Flag == 0:
                FlaggedRows.append( row )
           # Tsys flag
            NPpdata[0][N] = tTSYS
            NPpflag[0][N] = tPFLAG[1]
            NPprows[0][N] = row
            Valid = True
            if tPFLAG[1] == 0:
                FlaggedRowsCategory[0].append(row)
                Valid = False
            # Weather flag
            if tPFLAG[0] == 0:
                FlaggedRowsCategory[1].append(row)
                Valid = False
            # User flag
            if tPFLAG[2] == 0:
                FlaggedRowsCategory[2].append(row)
                Valid = False

            # commented out not to remove permanent flagged data to show
            #if Valid:
            NPprows[1][N] = row
            # RMS flag before baseline fit
            NPpdata[1][N] = tSTAT[2]
            NPpflag[1][N] = tFLAG[2]
            if tFLAG[2] == 0:
                FlaggedRowsCategory[4].append(row)
            NPpdata[2][N] = tSTAT[1]
            NPpflag[2][N] = tFLAG[1]
            if tFLAG[1] == 0:
                FlaggedRowsCategory[3].append(row)
            # Running mean flag before baseline fit
            NPpdata[3][N] = tSTAT[4]
            NPpflag[3][N] = tFLAG[4]
            if tFLAG[4] == 0:
                FlaggedRowsCategory[6].append(row)
            # Running mean flag after baseline fit
            NPpdata[4][N] = tSTAT[3]
            NPpflag[4][N] = tFLAG[3]
            if tFLAG[3] == 0:
                FlaggedRowsCategory[5].append(row)
            # Expected RMS flag before baseline fit
            NPpdata[5][N] = tSTAT[6]
            NPpflag[5][N] = tFLAG[6]
            if tFLAG[6] == 0:
                FlaggedRowsCategory[8].append(row)
            # Expected RMS flag after baseline fit
            NPpdata[6][N] = tSTAT[5]
            NPpflag[6][N] = tFLAG[5]
            if tFLAG[5] == 0:
                FlaggedRowsCategory[7].append(row)
            N += 1
        # data store finished

        ThreExpectedRMSPreFit = FlagRule['RmsExpectedPreFitFlag']['Threshold']
        ThreExpectedRMSPostFit = FlagRule['RmsExpectedPostFitFlag']['Threshold']
        plots = []
        # Tsys flag
        PlotData = {'row': NPprows[0], 'data': NPpdata[0], 'flag': NPpflag[0], \
                    'thre': [threshold[4][1], 0.0], \
                    'gap': [PosGap, TimeGap], \
                            'title': "Tsys (K)\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule['TsysFlag']['Threshold'], \
                    'xlabel': "row (spectrum)", \
                    'ylabel': "Tsys (K)", \
                    'permanentflag': PermanentFlag, \
                    'isActive': FlagRule['TsysFlag']['isActive'], \
                    'threType': "line"}
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_0')
        plots.append(FigFileRoot+'_0.png')

        # RMS flag before baseline fit
        PlotData['row'] = NPprows[1]
        PlotData['data'] = NPpdata[1]
        PlotData['flag'] = NPpflag[1]
        PlotData['thre'] = [threshold[1][1]]
        PlotData['title'] = "Baseline RMS (K) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule['RmsPreFitFlag']['Threshold']
        PlotData['ylabel'] = "Baseline RMS (K)"
        PlotData['isActive'] = FlagRule['RmsPreFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_1')
        plots.append(FigFileRoot+'_1.png')

        # RMS flag after baseline fit
        PlotData['data'] = NPpdata[2]
        PlotData['flag'] = NPpflag[2]
        PlotData['thre'] = [threshold[0][1]]
        PlotData['title'] = "Baseline RMS (K) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule['RmsPostFitFlag']['Threshold']
        PlotData['isActive'] = FlagRule['RmsPostFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_2')
        plots.append(FigFileRoot+'_2.png')

        # Running mean flag before baseline fit
        PlotData['data'] = NPpdata[3]
        PlotData['flag'] = NPpflag[3]
        PlotData['thre'] = [threshold[3][1]]
        PlotData['title'] = "RMS (K) for Baseline Deviation from the running mean (Nmean=%d) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (FlagRule['RunMeanPreFitFlag']['Nmean'], FlagRule['RunMeanPreFitFlag']['Threshold'])
        PlotData['isActive'] = FlagRule['RunMeanPreFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_3')
        plots.append(FigFileRoot+'_3.png')

        # Running mean flag after baseline fit
        PlotData['data'] = NPpdata[4]
        PlotData['flag'] = NPpflag[4]
        PlotData['thre'] = [threshold[2][1]]
        PlotData['title'] = "RMS (K) for Baseline Deviation from the running mean (Nmean=%d) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (FlagRule['RunMeanPostFitFlag']['Nmean'], FlagRule['RunMeanPostFitFlag']['Threshold'])
        PlotData['isActive'] = FlagRule['RunMeanPostFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_4')
        plots.append(FigFileRoot+'_4.png')

        # Expected RMS flag before baseline fit
        PlotData['data'] = NPpdata[1]
        PlotData['flag'] = NPpflag[5]
        PlotData['thre'] = [NPpdata[5]]
        PlotData['title'] = "Baseline RMS (K) compared with the expected RMS calculated from Tsys before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f, Red H-line(s): out of vertical scale limit(s)" % ThreExpectedRMSPreFit
        PlotData['isActive'] = FlagRule['RmsExpectedPreFitFlag']['isActive']
        PlotData['threType'] = "plot"
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_5')
        plots.append(FigFileRoot+'_5.png')

        # Expected RMS flag after baseline fit
        PlotData['data'] = NPpdata[2]
        PlotData['flag'] = NPpflag[6]
        PlotData['thre'] = [NPpdata[6]]
        PlotData['title'] = "Baseline RMS (K) compared with the expected RMS calculated from Tsys after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f" % ThreExpectedRMSPostFit
        PlotData['isActive'] = FlagRule['RmsExpectedPostFitFlag']['isActive']
        PlotData['threType'] = "plot"
        SDP.StatisticsPlot(PlotData, FigFileDir, FigFileRoot+'_6')
        plots.append(FigFileRoot+'_6.png')

        # Create Flagging Summary Page
        if FigFileDir != False:
            Filename = FigFileDir+FigFileRoot+'.html'
            relpath = os.path.basename(FigFileDir.rstrip("/")) ### stage#
            if os.access(Filename, os.F_OK): os.remove(Filename)
            # Assuming single scantable, antenna, spw, and pol
            ID = ids[0]
            ant_id = DataTable.getcell('ANTENNA',ID)
            st_row = DataTable.getcell('ROW',ID)
            #st_name = DataTable.getkeyword('FILENAMES')[ant_id]
            st = self.context.observing_run[ant_id]
            asdm = common.asdm_name(st)
            ant_name = st.antenna.name
            pol = DataTable.getcell('POL',ID)
            spw = DataTable.getcell('IF',ID)
            
            Out = open(Filename, 'w')
            #print >> Out, '<html>\n<head>\n<style>'
            #print >> Out, '.ttl{font-size:20px;font-weight:bold;}'
            #print >> Out, '.stt{font-size:18px;font-weight:bold;color:white;background-color:navy;}'
            #print >> Out, '.stp{font-size:18px;font-weight:bold;color:black;background-color:gray;}'
            #print >> Out, '.stc{font-size:16px;font-weight:normal;}'
            #print >> Out, '</style>\n</head>\n<body>'
            print >> Out, '<body>'
            print >> Out, '<p class="ttl">Data Summary</p>'
            # A table of data summary
            print >> Out, '<table border="0"  cellpadding="3">'
            print >> Out, '<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Name', asdm)
            print >> Out, '<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Antenna', ant_name)
            print >> Out, '<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Spw ID', spw)
            print >> Out, '<tr align="left" class="stp"><th>%s</th><th>:</th><th>%s</th></tr>' % ('Pol', pol)
            print >> Out, '</table>\n'
            
            print >> Out, '<HR><p class="ttl">Flagging Status</p>'
            # A table of flag statistics summary
            print >> Out, '<table border="1">'
            print >> Out, '<tr align="center" class="stt"><th>&nbsp</th><th>isActive?</th><th>SigmaThreshold<th>Flagged spectra</th><th>Flagged ratio(%)</th></tr>'
            print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('User', FlagRule['UserFlag']['isActive'], FlagRule['UserFlag']['Threshold'], len(FlaggedRowsCategory[2]), len(FlaggedRowsCategory[2])*100.0/NROW)
            print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Weather', FlagRule['WeatherFlag']['isActive'], FlagRule['WeatherFlag']['Threshold'], len(FlaggedRowsCategory[1]), len(FlaggedRowsCategory[1])*100.0/NROW)
            print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Tsys', FlagRule['TsysFlag']['isActive'], FlagRule['TsysFlag']['Threshold'], len(FlaggedRowsCategory[0]), len(FlaggedRowsCategory[0])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('RMS baseline (pre-fit)', FlagRule['RmsPreFitFlag']['isActive'], FlagRule['RmsPreFitFlag']['Threshold'], len(FlaggedRowsCategory[4]), len(FlaggedRowsCategory[4])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('RMS baseline (post-fit)', FlagRule['RmsPostFitFlag']['isActive'], FlagRule['RmsPostFitFlag']['Threshold'], len(FlaggedRowsCategory[3]), len(FlaggedRowsCategory[3])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Running Mean (pre-fit)', FlagRule['RunMeanPreFitFlag']['isActive'], FlagRule['RunMeanPreFitFlag']['Threshold'], len(FlaggedRowsCategory[6]), len(FlaggedRowsCategory[6])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Running Mean (post-fit)', FlagRule['RunMeanPostFitFlag']['isActive'], FlagRule['RunMeanPostFitFlag']['Threshold'], len(FlaggedRowsCategory[5]), len(FlaggedRowsCategory[5])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Expected RMS (pre-fit)', FlagRule['RmsExpectedPreFitFlag']['isActive'], FlagRule['RmsExpectedPreFitFlag']['Threshold'], len(FlaggedRowsCategory[8]), len(FlaggedRowsCategory[8])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Expected RMS (post-fit)', FlagRule['RmsExpectedPostFitFlag']['isActive'], FlagRule['RmsExpectedPostFitFlag']['Threshold'], len(FlaggedRowsCategory[7]), len(FlaggedRowsCategory[7])*100.0/NROW)
            print >> Out, '<tr align="center" class="stt"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Total Flagged', '-', '-', len(FlaggedRows), len(FlaggedRows)*100.0/NROW)
            print >> Out, '<tr><td colspan=4>%s</td></tr>' % ("Note: flags in grey background are permanent, <br> which are not reverted or changed during the iteration cycles.") 
            #print >> Out, '</table>\n</body>\n</html>'
            print >> Out, '</table>\n'
            # Plot figures
            print >> Out, '<HR>\nNote to all the plots below: short green vertical lines indicate position gaps; short cyan vertical lines indicate time gaps\n<HR>'
            for name in plots:
                print >> Out, '<img src="%s/%s">\n<HR>' % (relpath, name)
            #print >> Out, '</body>\n</html>'
            print >> Out, '</body>'
            Out.close()

        # User flag
        LOG.info('Number of rows flagged by User = %d /%d' % (len(FlaggedRowsCategory[2]), NROW))
        if len(FlaggedRowsCategory[2]) > 0:
            LOG.debug('Flagged rows by User =%s ' % FlaggedRowsCategory[2])
        # Weather
        LOG.info('Number of rows flagged by Weather = %d /%d' % (len(FlaggedRowsCategory[1]), NROW))
        if len(FlaggedRowsCategory[1]) > 0:
            LOG.debug('Flagged rows by Weather =%s ' % FlaggedRowsCategory[1])
        # Tsys
        LOG.info('Number of rows flagged by Tsys = %d /%d' % (len(FlaggedRowsCategory[0]), NROW))
        if len(FlaggedRowsCategory[0]) > 0:
            LOG.debug('Flagged rows by Tsys =%s ' % FlaggedRowsCategory[0])
        # Pre-fit RMS
        LOG.info('Number of rows flagged by the baseline fluctuation (pre-fit) = %d /%d' % (len(FlaggedRowsCategory[4]), NROW))
        if len(FlaggedRowsCategory[4]) > 0:
            LOG.debug('Flagged rows by the baseline fluctuation (pre-fit) =%s ' % FlaggedRowsCategory[4])
        # Post-fit RMS
        LOG.info('Number of rows flagged by the baseline fluctuation (post-fit) = %d /%d' % (len(FlaggedRowsCategory[3]), NROW))
        if len(FlaggedRowsCategory[3]) > 0:
            LOG.debug('Flagged rows by the baseline fluctuation (post-fit) =%s ' % FlaggedRowsCategory[3])
        # Pre-fit running mean
        LOG.info('Number of rows flagged by the difference from running mean (pre-fit) = %d /%d' % (len(FlaggedRowsCategory[6]), NROW))
        if len(FlaggedRowsCategory[6]) > 0:
            LOG.debug('Flagged rows by the difference from running mean (pre-fit) =%s ' % FlaggedRowsCategory[6])
        # Post-fit running mean
        LOG.info('Number of rows flagged by the difference from running mean (post-fit) = %d /%d' % (len(FlaggedRowsCategory[5]), NROW))
        if len(FlaggedRowsCategory[5]) > 0:
            LOG.debug('Flagged rows by the difference from running mean (post-fit) =%s ' % FlaggedRowsCategory[5])
        # Pre-fit expected RMS
        LOG.info('Number of rows flagged by the expected RMS (pre-fit) = %d /%d' % (len(FlaggedRowsCategory[8]), NROW))
        if len(FlaggedRowsCategory[8]) > 0:
            LOG.debug('Flagged rows by the expected RMS (pre-fit) =%s ' % FlaggedRowsCategory[8])
        # Post-fit expected RMS
        LOG.info('Number of rows flagged by the expected RMS (post-fit) = %d /%d' % (len(FlaggedRowsCategory[7]), NROW))
        if len(FlaggedRowsCategory[7]) > 0:
            LOG.debug('Flagged rows by the expected RMS (post-fit) =%s ' % FlaggedRowsCategory[7])
        # All categories
        LOG.info('Number of rows flagged by all active categories = %d /%d' % (len(FlaggedRows), NROW))
        if len(FlaggedRows) > 0:
            LOG.debug('Final Flagged rows by all active categories =%s ' % FlaggedRows)

        flag_nums = [len(FlaggedRows)] + [len(frows) for frows in FlaggedRowsCategory]

        del threshold, NPpdata, NPpflag, NPprows, PlotData, FlaggedRows, FlaggedRowsCategory
        return os.path.basename(Filename), flag_nums
    
def _get_iteration(reduction_group, antenna, spw, pol):
    for (group_id, group_desc) in reduction_group.items():
        for group_member in group_desc:
            if group_member.antenna == antenna and group_member.spw == spw and pol in group_member.pols:
                return group_member.iteration[pol]
    raise RuntimeError('Given (%s, %s, %s) is not in reduction group.'%(antenna, spw, pol))
