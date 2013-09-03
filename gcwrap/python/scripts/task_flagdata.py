from taskinit import *
import time
import os
import sys
import flaghelper as fh
from parallel.parallel_task_helper import ParallelTaskHelper

debug = False


def flagdata(vis,
             mode,
             autocorr,      # mode manual parameter
             inpfile,       # mode list parameters
             reason,
             spw,           # data selection parameters
             field,
             antenna,
             uvrange,
             timerange,
             correlation,
             scan,
             intent,
             array,
             observation,
             feed,
             clipminmax,    # mode clip parameters
             datacolumn,
             clipoutside,
             channelavg,
             clipzeros,
             quackinterval, # mode quack parameters
             quackmode,
             quackincrement,
             tolerance,      # mode shadow parameter
             addantenna,
             lowerlimit,    # mode elevation parameters
             upperlimit,
             ntime,         # mode tfcrop
             combinescans,
             timecutoff,    
             freqcutoff,
             timefit,
             freqfit,
             maxnpieces,
             flagdimension,
             usewindowstats,
             halfwin,
             extendflags,
             winsize,    # rflag parameters
             timedev,
             freqdev,
             timedevscale,
             freqdevscale,
             spectralmax,
             spectralmin,
             extendpols,    # mode extend
             growtime,
             growfreq,
             growaround,
             flagneartime,
             flagnearfreq,
             minrel,        # mode summary
             maxrel,
             minabs,
             maxabs,
             spwchan,
             spwcorr,
             basecnt,
             action,           # run or not the tool
             display,
             flagbackup,
             savepars,      # save the current parameters to FLAG_CMD  or to a file
             cmdreason,     # reason to save to flag cmd
             outfile):      # output file to save flag commands

    # Global parameters
    # vis, mode, action, savepars                      
    
    #
    # Task flagdata
    #    Flags data from an MS or calibration table based on data selection in various ways
    
    casalog.origin('flagdata')
    
    # (CAS-4119): Use absolute paths for input files to ensure that the engines find them
    if isinstance(inpfile, str) and inpfile != "":                   
        inpfile = os.path.abspath(inpfile)
        fh.addAbsPath(inpfile)
        
    elif isinstance(inpfile, list) and os.path.isfile(inpfile[0]):
        # It is a list of input files
        for inputfile in range(len(inpfile)):
            inpfile[inputfile] = os.path.abspath(inpfile[inputfile])
            fh.addAbsPath(inpfile[inputfile])
            
    if (outfile != ""):
        outfile = os.path.abspath(outfile)        
        
    if (isinstance(addantenna,str) and addantenna != ""):
        addantenna = os.path.abspath(addantenna)
        
    if (isinstance(timedev,str) and timedev != ""):
        timedev = os.path.abspath(timedev)        
        
    if (isinstance(freqdev,str) and freqdev != ""):
        freqdev = os.path.abspath(freqdev)        
        
    # CAS-4696
    if (action == 'none' or action=='' or action=='calculate' and flagbackup):
        flagbackup = False
        
    # SMC: moved the flagbackup to before initializing the cluster.
    # Note that with this change, a flag backup will be created even if
    # an error happens that prevents the flagger tool from running.    
    if (mode != 'summary' and flagbackup):
        casalog.post('Backup original flags before applying new flags')
        fh.backupFlags(aflocal=None, msfile=vis, prename='flagdata')
        # Set flagbackup to False because only the controller
        # should create a backup
        flagbackup = False


    # Save all the locals()
    orig_locals = locals().copy()
    # inputfile may have been added to the locals dictionary. The cluster does not
    # recognize it as a parameter for the task, so we have to remove it.
    if orig_locals.__contains__('inputfile'):
        del orig_locals['inputfile']
        
    
    iscal = False
    summary_stats={};
    
    # Check if vis is a cal table:
    # typevis = 1 --> cal table
    # typevis = 0 --> MS
    # typevis = 2 --> MMS
    typevis = fh.isCalTable(vis)
    if typevis == 1:
        iscal = True  

    if pCASA.is_mms(vis):
        pCASA.execute("flagdata", orig_locals)
        return

    # Take care of the trivial parallelization
    # jagonzal (CAS-4119): If flags are not going to be applied is better to run in sequential mode
#        if ParallelTaskHelper.isParallelMS(vis) and action != '' and action != 'none':
    if typevis == 2 and action != '' and action != 'none':
        # To be safe convert file names to absolute paths.
        helper = ParallelTaskHelper('flagdata', orig_locals)
        # jagonzal (CAS-4119): Override summary minabs,maxabs,minrel,maxrel 
        # so that it is done after consolidating the summaries
        
        # By-pass options to filter summary
        filterSummary = False
        if ((mode == 'summary') and ((minrel != 0.0) or (maxrel != 1.0) or (minabs != 0) or (maxabs != -1))):
            filterSummary = True
            
            myms = mstool()
            myms.open(vis)
            subMS_list = myms.getreferencedtables()
            myms.close()
            
            if (minrel != 0.0):
                minreal_dict = create_arg_dict(subMS_list,0.0)
                helper.override_arg('minrel',minreal_dict)
            if (maxrel != 1.0):
                maxrel_dict = create_arg_dict(subMS_list,1.0)
                helper.override_arg('maxrel',maxrel_dict)
            if (minabs != 0):
                minabs_dict = create_arg_dict(subMS_list,0)
                helper.override_arg('minabs',minabs_dict)
            if (maxabs != -1):
                maxabs_dict = create_arg_dict(subMS_list,-1)
                helper.override_arg('maxabs',maxabs_dict)
                
        # By-pass options to filter summary
        if savepars:  
            
            myms = mstool()
            myms.open(vis)
            subMS_list = myms.getreferencedtables()
            myms.close()
            
            savepars_dict = create_arg_dict(subMS_list,False)
            helper.override_arg('savepars',savepars_dict)
            
        retVar = helper.go()
        
        # Filter summary at MMS level
        if (mode == 'summary'):
            if filterSummary:
                retVar = filter_summary(retVar,minrel,maxrel,minabs,maxabs)
            return retVar
        # Save parameters at MMS level
        elif savepars:
            action = 'none'
        else:
            return retVar
    
    # Create local tools
    aflocal = casac.agentflagger()
    mslocal = mstool()

    try: 
        # Verify the ntime value
        newtime = 0.0
        if type(ntime) == float or type(ntime) == int:
            if ntime <= 0:
                raise Exception, 'Parameter ntime cannot be < = 0'
            else:
                # units are seconds
                newtime = float(ntime)
        
        elif type(ntime) == str:
            if ntime == 'scan':
                # iteration time step is a scan
                newtime = 0.0
            else:
                # read the units from the string
                qtime = qa.quantity(ntime)
                
                if qtime['unit'] == 'min':
                    # convert to seconds
                    qtime = qa.convert(qtime, 's')
                elif qtime['unit'] == '':
                    qtime['unit'] = 's'
                    
                # check units
                if qtime['unit'] == 's':
                    newtime = qtime['value']
                else:
                    casalog.post('Cannot convert units of ntime. Will use default 0.0s', 'WARN')
                                    
        casalog.post("New ntime is of type %s and value %s"%(type(newtime),newtime), 'DEBUG')
                
        # Open the MS and attach it to the tool
        if ((type(vis) == str) & (os.path.exists(vis))):
            aflocal.open(vis, newtime)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'


        # Get the parameters for the mode
        agent_pars = {}

        
        # By default, write flags to the MS
        writeflags = True
        
        # Only the apply action writes to the MS
        # action=apply     --> write to the MS
        # action=calculate --> do not write to the MS
        # action=''        --> do not run the tool and do not write to the MS
        if action != 'apply':
            writeflags = False
                                         
        # Default mode
        if mode == '' or mode == 'manualflag':
            mode = 'manual'
        
        # Read in the list of commands
        if mode == 'list':
            casalog.post('List mode is active')
            # Parse the input file
            try:            
                # Is it a file or a Python list?
                if isinstance(inpfile, list):
                    
                    # It is a list of input files
                    if os.path.isfile(inpfile[0]):
                        flaglist = []
                        for ifile in inpfile:
                            casalog.post('Will read commands from the file '+ifile)                    
                            flaglist = flaglist + fh.readFile(ifile)
                        
                        # Make a FLAG_CMD compatible dictionary. Select by reason if requested
                        flagcmd = fh.makeDict(flaglist, reason)
                    
                    # It is a list of strings with flag commands
                    else:
                        # Make a FLAG_CMD compatible dictionary. Select by reason if requested
                        casalog.post('Will read commands from a Python list')
                        flagcmd = fh.makeDict(inpfile, reason)
                    
                # It is only one file
                elif isinstance(inpfile, str):
                    
                    if inpfile == '':
                         casalog.post('Input file is empty', 'ERROR')
                         
                    casalog.post('Will read commands from the file '+inpfile)
                    flaglist = fh.readFile(inpfile)
                    casalog.post('%s'%flaglist,'DEBUG')
                    
                    # Make a FLAG_CMD compatible dictionary. Select by reason if requested
                    flagcmd = fh.makeDict(flaglist, reason)
                
                else:
                    casalog.post('Input type is not supported', 'ERROR')
                    
                casalog.post('%s'%flagcmd,'DEBUG')
                
                # Update the list of commands with the selection
                flaglist = []
                for k in flagcmd.keys():
                    cmdline = flagcmd[k]['command']
                    flaglist.append(cmdline)
                                    
                # List of command keys in dictionary
                vrows = flagcmd.keys()
                

            except:
                raise Exception, 'Error reading the input list '
            
            casalog.post('Read ' + str(vrows.__len__())
                         + ' lines from input list ')
                             
        elif mode == 'manual':
            agent_pars['autocorr'] = autocorr
            casalog.post('Manual mode is active')

            
        elif mode == 'clip':
                
            agent_pars['datacolumn'] = datacolumn.upper()
            agent_pars['clipoutside'] = clipoutside
            agent_pars['channelavg'] = channelavg
            agent_pars['clipzeros'] = clipzeros
            

            if type(clipminmax) != list:
                casalog.post('Error : clipminmax must be a list : [min,max]', 'ERROR')
            # If clipminmax = [], do not write it in the dictionary.
            # It will be handled by the framework to flag NaNs only
            if clipminmax.__len__() == 2:      
                # Cast to float to avoid the missing decimal point                 
                clipmin = float(clipminmax[0])
                clipmax = float(clipminmax[1])
                clipminmax = []
                clipminmax.append(clipmin)
                clipminmax.append(clipmax)     
                agent_pars['clipminmax'] = clipminmax
                
            casalog.post('Clip mode is active')
                        
        elif mode == 'shadow':
                
            agent_pars['tolerance'] = tolerance
            
            if type(addantenna) == str:
                if addantenna != '':
                    # it's a filename, create a dictionary
                    antdict = fh.readAntennaList(addantenna)
                    agent_pars['addantenna'] = antdict
                    
            elif type(addantenna) == dict:
                if addantenna != {}:
                    agent_pars['addantenna'] = addantenna
                                                               
            casalog.post('Shadow mode is active')
            
        elif mode == 'quack':
            agent_pars['quackmode'] = quackmode
            agent_pars['quackinterval'] = quackinterval
            agent_pars['quackincrement'] = quackincrement
            casalog.post('Quack mode is active')
            

        elif mode == 'elevation':

            agent_pars['lowerlimit'] = lowerlimit
            agent_pars['upperlimit'] = upperlimit
            casalog.post('Elevation mode is active')
            

        elif mode == 'tfcrop':
                
            agent_pars['ntime'] = newtime
            agent_pars['combinescans'] = combinescans            
            agent_pars['datacolumn'] = datacolumn.upper()
            agent_pars['timecutoff'] = timecutoff
            agent_pars['freqcutoff'] = freqcutoff
            agent_pars['timefit'] = timefit
            agent_pars['freqfit'] = freqfit
            agent_pars['maxnpieces'] = maxnpieces
            agent_pars['flagdimension'] = flagdimension
            agent_pars['usewindowstats'] = usewindowstats
            agent_pars['halfwin'] = halfwin
            agent_pars['extendflags'] = bool(extendflags)
            casalog.post('Time and Frequency (tfcrop) mode is active')

                      
        elif mode == 'rflag':

            agent_pars['ntime'] = newtime
            agent_pars['combinescans'] = combinescans   
            agent_pars['datacolumn'] = datacolumn.upper()
            agent_pars['winsize'] = winsize
            agent_pars['timedevscale'] = timedevscale
            agent_pars['freqdevscale'] = freqdevscale
            agent_pars['spectralmax'] = spectralmax
            agent_pars['spectralmin'] = spectralmin
            agent_pars['extendflags'] = bool(extendflags)

            # These can be double, doubleArray, or string.
            # writeflags=False : calculate and return thresholds.
            # writeflags=True : use given thresholds for this run.
            if( type(timedev) == str and writeflags == True):
                timedev = fh.readRFlagThresholdFile(timedev,'timedev')
            if( type(freqdev) == str and writeflags == True):
                freqdev = fh.readRFlagThresholdFile(freqdev,'freqdev')

            agent_pars['timedev'] = timedev
            agent_pars['freqdev'] = freqdev
            
            agent_pars['writeflags'] = writeflags
            agent_pars['display'] = display

            casalog.post('Rflag mode is active')

        elif mode == 'extend':
            agent_pars['ntime'] = newtime
            agent_pars['combinescans'] = combinescans                            
            agent_pars['extendpols'] = extendpols
            agent_pars['growtime'] = growtime
            agent_pars['growfreq'] = growfreq
            agent_pars['growaround'] = growaround
            agent_pars['flagneartime'] = flagneartime
            agent_pars['flagnearfreq'] = flagnearfreq
            casalog.post('Extend mode is active')
            
            
        elif mode == 'unflag':      
            casalog.post('Unflag mode is active')                
            
        elif mode == 'summary':
            agent_pars['spwchan'] = spwchan
            agent_pars['spwcorr'] = spwcorr
            agent_pars['basecnt'] = basecnt
            
            # Disable writeflags and savepars
            writeflags = False
            savepars = False
            casalog.post('Summary mode is active')


        # Create a flagcmd dictionary of the interface parameters to save
        # when savepars = True
        if mode != 'list' and mode != 'summary':

            # CAS-4063: remove any white space in the values of the
            # selection parameters before creating the string.
            
            # Create a dictionary of the selection parameters
            seldic = {}
            seldic['field'] = field
            seldic['spw'] = spw
            seldic['array'] = array
            seldic['feed'] = feed
            seldic['scan'] = scan
            seldic['antenna'] = antenna
            seldic['uvrange'] = uvrange
            seldic['timerange'] = timerange
            seldic['intent'] = intent
            seldic['observation'] = str(observation)
            
            # String to hold the selection parameters
            sel_pars = []
            sel_pars = ' mode='+mode
            if correlation != '':
                # Replace an empty space, in case there is one
                expr = delspace(correlation, '_')
                sel_pars = sel_pars +' correlation=' + expr
            
            # Include only parameters with values in the string
            # Remove the white spaces from the values
            for key,val in seldic.iteritems():
                if val != '':
                    # Delete any space in the value
                    if key != 'field':
                        val = delspace(val, '')
                    # Do not remove spaces from selection strings
                    # because they are valid in field names
                    sel_pars = sel_pars +' ' + key + '=' + val
                               
            # Add the agent's parameters to the same string 
            for k in agent_pars.keys():
                if agent_pars[k] != '':
                    # Remove any white space from the string value
                    nospace = delspace(str(agent_pars[k]),'')
                    sel_pars = sel_pars + ' ' + k + '=' + nospace
                
            
            # Create a dictionary according to the FLAG_CMD structure
            flagcmd = fh.makeDict([sel_pars])
                        
            # Number of commands in dictionary
            vrows = flagcmd.keys()
            casalog.post('There are %s cmds in dictionary of mode %s'%(vrows.__len__(),mode),'DEBUG')


        # Setup global parameters in the agent's dictionary
        apply = True
                    
        # Correlation does not go in selectdata, but in the agent's parameters
        if correlation != '':
            agent_pars['correlation'] = correlation.upper()
        
        
        # Hold the name of the agent
        agent_name = mode.capitalize()
        agent_pars['name'] = agent_name
        agent_pars['mode'] = mode
        agent_pars['apply'] = apply      
                          
        ##########  Only save the parameters and exit; action = ''     
        if (action == '' or action == 'none') and savepars == False:
            casalog.post('Parameter action=\'\' is only meaningful with savepars=True.', 'WARN')
            aflocal.done()
            return summary_stats
        
        if (action == '' or action == 'none') and savepars == True:
            if iscal:
                if outfile == '':
                    casalog.post('Saving to FLAG_CMD is not supported for cal tables', 'WARN')
                else:
                    casalog.post('Saving parameters to '+outfile)                            
                    fh.writeFlagCmd(vis, flagcmd, vrows, False, cmdreason, outfile) 
            else: 
                if outfile == '':
                    casalog.post('Saving parameters to FLAG_CMD')
                else:
                    casalog.post('Saving parameters to '+outfile)                            
                
                fh.writeFlagCmd(vis, flagcmd, vrows, False, cmdreason, outfile) 
                     
            aflocal.done()
            return summary_stats

        
        ######### From now on it is assumed that action = apply or calculate
        
        # Select the data and parse the agent's parameters
        if mode != 'list':
            aflocal.selectdata(field=field, spw=spw, array=array, feed=feed, scan=scan, \
                               antenna=antenna, uvrange=uvrange, timerange=timerange, \
                               intent=intent, observation=str(observation))   

            # CAS-3959 Handle channel selection at the FlagAgent level
            agent_pars['spw'] = spw
            casalog.post('Parsing the parameters for the %s mode'%mode)
            if (not aflocal.parseagentparameters(agent_pars)):
                casalog.post('Failed to parse parameters for mode %s' %mode, 'ERROR')
                
            casalog.post('%s'%agent_pars, 'DEBUG')
       
        else:        
            # Select a loose union of the data selection from the list
            # The loose union will be calculated for field and spw only
            # antenna, correlation and timerange should be handled by the agent
            if vrows.__len__() == 0:
                raise Exception, 'There are no valid commands in list'
            
            unionpars = {}
            # Do not crete union for a cal table
            if iscal:
                if vrows.__len__() == 1:
                    cmd0 = flagcmd[vrows[0]]['command']
                    unionpars = fh.getSelectionPars(cmd0)
                    casalog.post('The selected subset of the cal table will be: ');
                    casalog.post('%s'%unionpars);
                    
            else:
                if vrows.__len__() > 1:
                   unionpars = fh.getUnion(vis, flagcmd)
                   
                   if( len( unionpars.keys() ) > 0 ):
                        casalog.post('Pre-selecting a subset of the MS : ');
                        casalog.post('%s'%unionpars)
                        
                   else:
                        casalog.post('Iterating through the entire MS');
                                                
                # Get all the selection parameters, but set correlation to ''
                elif vrows.__len__() == 1:
                    cmd0 = flagcmd[vrows[0]]['command']
                    unionpars = fh.getSelectionPars(cmd0)
                    casalog.post('The selected subset of the MS will be: ');
                    casalog.post('%s'%unionpars);
                
            aflocal.selectdata(unionpars);

            # Parse the parameters for each agent in the list
            list2save = fh.setupAgent(aflocal, flagcmd, [], apply, writeflags, display)

        # Do display if requested
        if display != '':
            
            agent_pars = {}
            casalog.post('Parsing the display parameters')
                
            agent_pars['mode'] = 'display'
            # need to create different parameters for both, data and report.
            if display == 'both':
                agent_pars['datadisplay'] = True
                agent_pars['reportdisplay'] = True
            
            elif display == 'data':
                agent_pars['datadisplay'] = True
            
            elif display == 'report':
                agent_pars['reportdisplay'] = True
                
            # jagonzal: CAS-3966 Add datacolumn to display agent parameters
            agent_pars['datacolumn'] = datacolumn.upper()
            aflocal.parseagentparameters(agent_pars)
            
            # Disable saving the parameters to avoid inconsistencies
            if savepars:
                casalog.post('Disabling savepars for the display', 'WARN')
                savepars = False
                    
        # Initialize the agents
        casalog.post('Initializing the agents')
        aflocal.init()
        
        # Run the tool
        casalog.post('Running the agentflagger tool')
        summary_stats_list = aflocal.run(writeflags, True)
        
        # Inform the user when flags are not written to the MS
        if not writeflags:
            casalog.post("Flags are not written to the MS. (action=\'calculate\')")


        # Now, deal with all the modes that return output.
        # Summary : Currently, only one is allowed in the task
        # Rflag : There can be many 'rflags' in the list mode.

        ## Pull out RFlag outputs. There will be outputs only if writeflags=False
        if (mode == 'rflag' or mode== 'list') and (writeflags==False):  
            fh.extractRFlagOutputFromSummary(mode,summary_stats_list, flagcmd)

        # Save the current parameters/list to FLAG_CMD or to output
        if savepars:  
            # Cal table type
            if iscal:
                if outfile == '':
                    casalog.post('Saving to FLAG_CMD is not supported for cal tables', 'WARN')
                else:
                    casalog.post('Saving parameters to '+outfile)
                    if mode != 'list':     
                        fh.writeFlagCmd(vis, flagcmd, vrows, writeflags, cmdreason, outfile)  
                    else:
                        valid_rows = list2save.keys()
                        fh.writeFlagCmd(vis, list2save, valid_rows, writeflags, cmdreason, outfile)        
                    
            # MS type
            else:                
                if outfile == '':
                    casalog.post('Saving parameters to FLAG_CMD')        
                else:
                    casalog.post('Saving parameters to '+outfile)
                                          
                if mode != 'list':     
                    fh.writeFlagCmd(vis, flagcmd, vrows, writeflags, cmdreason, outfile)  
                else:
                    valid_rows = list2save.keys()
                    fh.writeFlagCmd(vis, list2save, valid_rows, writeflags, cmdreason, outfile)        
            
        # Destroy the tool
        aflocal.done()

        retval = True
        # Write history to the MS. Only for modes that write to the MS
        if not iscal:
            if mode != 'summary' and action == 'apply':
                try:
                    param_names = flagdata.func_code.co_varnames[:flagdata.func_code.co_argcount]
                    param_vals = [eval(p) for p in param_names]
                    retval &= write_history(mslocal, vis, 'flagdata', param_names,
                                            param_vals, casalog)
                    
                except Exception, instance:
                    casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                                 'WARN')

        # Pull out the 'summary' part of summary_stats_list.
        # (This is the task, and there will be only one such dictionary.)
        # After this step, the only thing left in summary_stats_list are the
        # list of reports/views, if any.  Return it, if the user wants it.
        if mode == 'summary':
           if type(summary_stats_list) is dict:
               nreps = summary_stats_list['nreport'];
               for rep in range(0,nreps):
                    repname = "report"+str(rep);
                    if summary_stats_list[repname]['type'] == "summary":
                          summary_stats = summary_stats_list.pop(repname);
                          summary_stats_list[repname] = {'type':'none','name':'none'};
                          break;  # pull out only one summary.
        
           # Filter out baselines/antennas/fields/spws/... from summary_stats
           # which do not fall within limits
           summary_stats = filter_summary(summary_stats,minrel,maxrel,minabs,maxabs)
        
        # if (need to return the reports/views as well as summary_stats) :
        #      return summary_stats , summary_stats_list;
        # else :
        #      return summary_stats;
        return summary_stats
    
    except Exception, instance:
        aflocal.done()
        casalog.post('%s'%instance,'ERROR')


# Helper functions
def delspace(word, replace):
    '''Replace the white space of a string with another character'''
    
    newword = word
    if word.count(' ') > 0:
        newword = word.replace(' ', replace)
    
    return newword

def filter_summary(summary_stats,minrel,maxrel,minabs,maxabs):
    
    if type(summary_stats) is dict:
        for x in summary_stats.keys():
            if type(summary_stats[x]) is dict:
                for xx in summary_stats[x].keys():
                    flagged = summary_stats[x][xx]
                    assert type(flagged) is dict
                    assert flagged.has_key('flagged')
                    assert flagged.has_key('total')
                    if flagged['flagged'] < minabs or \
                       (flagged['flagged'] > maxabs and maxabs >= 0) or \
                       flagged['flagged'] * 1.0 / flagged['total'] < minrel or \
                       flagged['flagged'] * 1.0 / flagged['total'] > maxrel:
                        del summary_stats[x][xx]
                        
    return summary_stats

def create_arg_dict(subMS_list,value):
    
    ret_dict = []
    for subMS in subMS_list:
        ret_dict.append(value)
        
    return ret_dict
    
   
    
    
    
    
    
