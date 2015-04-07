#!/usr/lib/casapy/bin/python
# -*- coding: utf-8 -*-
from taskinit import *
import time
import os
import sys
import flaghelper as fh

debug = False


def tflagcmd(
    vis=None,
    inpmode=None,
    inpfile=None,
    tablerows=None,
    reason=None,
    useapplied=None,
    command=None,
    tbuff=None,
    ants=None,
    action=None,
    flagbackup=None,
    clearall=None,
    rowlist=None,
    plotfile=None,
    savepars=None,
    outfile=None,
    async=None,
    ):

    #
    # Task tflagcmd
    #    Reads flag commands from file or string and applies to MS

    # TO DO:
    # remove sorting
    # add reason selection in all input types
    # what to do with the TIME column?
    # implement backup for the whole input file
    # add support for ASDM.xml ????

    try:
        from xml.dom import minidom
    except:
        raise Exception, 'Failed to load xml.dom.minidom into python'

    casalog.origin('tflagcmd')
#    casalog.post('You are using flagcmd v3.6 Updated STM 2011-06-28')

    tflocal = casac.homefinder.find_home_by_name('testflaggerHome'
            ).create()
    mslocal = casac.homefinder.find_home_by_name('msHome').create()

    # MS HISTORY
    mslocal.open(vis, nomodify=False)
    mslocal.writehistory(message='taskname = tflagcmd',
                         origin='tflagcmd')
    mslocal.open(vis, nomodify=False)

    try:
        # Use a default ntime to open the MS. The user-set ntime will be
        # used in the tool later
        ntime = 0.0

        # Open the MS and attach it to the tool
        if (type(vis) == str) & os.path.exists(vis):
            tflocal.open(vis, ntime)
        else:
            raise Exception, \
                'Visibility data set not found - please verify the name'

        # Get overall MS time range for later use (if needed)
#        try:
            # this might take too long for large MS
        mslocal.open(vis)
        timd = mslocal.range(['time'])
        mslocal.close()

        ms_startmjds = timd['time'][0]
        ms_endmjds = timd['time'][1]
        t = qa.quantity(ms_startmjds, 's')
        t1sdata = t['value']
        ms_starttime = qa.time(t, form='ymd', prec=9)
        ms_startdate = qa.time(t, form=['ymd', 'no_time'])
        t0 = qa.totime(ms_startdate + '/00:00:00.00')
        t0d = qa.convert(t0, 'd')
        t0s = qa.convert(t0, 's')
        t = qa.quantity(ms_endmjds, 's')
        t2sdata = t['value']
        ms_endtime = qa.time(t, form='ymd', prec=9)
        # NOTE: could also use values from OBSERVATION table col TIME_RANGE
        casalog.post('MS spans timerange ' + ms_starttime + ' to '
                     + ms_endtime)

        myflagcmd = {}
        cmdlist = []

        if action == 'clear':
            casalog.post('Action "clear" will disregard inpmode (no reading)'
                         )
        elif inpmode == 'table':

            casalog.post('Reading from FLAG_CMD table')
            # Read from FLAG_CMD table into command list
            if inpfile == '':
                msfile = vis
            else:
                msfile = inpfile

            # Read only the selected rows for action = apply and
            # always read all rows with APPLIED=True for action = unapply
            if action == 'unapply':
                myflagcmd = readFromTable(msfile, useapplied=True,
                        myreason=reason)
            else:
                myflagcmd = readFromTable(msfile, myflagrows=tablerows,
                        useapplied=useapplied, myreason=reason)


            listmode = 'cmd'
        elif inpmode == 'file':

            casalog.post('Reading from ASCII file')
            # ##### TO DO: take time ranges calculation into account ??????
            # Parse the input file
            try:
                if inpfile == '':
                    casalog.post('Input file is empty', 'ERROR')

                cmdlist = fh.readFile(inpfile)

                # Make a FLAG_CMD compatible dictionary and select by reason
                myflagcmd = fh.makeDict(cmdlist, reason)

                # List of command keys in dictionary
                vrows = myflagcmd.keys()
            except:

                raise Exception, 'Error reading the input file ' \
                    + inpfile

            casalog.post('Read ' + str(vrows.__len__())
                         + ' lines from file ' + inpfile)

            listmode = 'file'
        elif inpmode == 'xml':

            casalog.post('Reading from Flag.xml')
            # Read from Flag.xml (also needs Antenna.xml)
            if inpfile == '':
                flagtable = vis
            else:
                flagtable = inpfile

            # Actually parse table
            myflags = fh.readXML(flagtable, mytbuff=tbuff)
            casalog.post('%s' % myflags, 'DEBUG')

            # Construct flags per antenna, selecting by reason if desired
            # Never sort!
            if ants != '' or reason != '':
                myflagcmd = selectFlags(myflags, myantenna=ants,
                        myreason=reason, myflagsort='')
            else:
                myflagcmd = myflags

            listmode = 'online'
        else:

            # command strings
            cmdlist = command
            casalog.post('Input ' + str(cmdlist.__len__())
                         + ' lines from input list')

            if cmdlist.__len__() > 0:
                myflagcmd = fh.makeDict(cmdlist, 'any')

            listmode = ''

        # Specific for some actions
        # Get the commands as a list of strings with reason back in
        # TODO: maybe this is not needed anymore!!!!!!!!!!!!!!!
        if action == 'apply' or action == 'unapply' or action == 'list':
            # If there are no flag commands, exit
            if myflagcmd.__len__() == 0:
                casalog.post('There are no flag commands in input',
                             'WARN')
                return

            # Turn into command string list (add reason back in)
            mycmdlist = []
            keylist = myflagcmd.keys()
            if keylist.__len__() > 0:
                for key in keylist:
                    cmdstr = myflagcmd[key]['command']
                    if myflagcmd[key]['reason'] != '':
                        cmdstr += " reason='" + myflagcmd[key]['reason'
                                ] + "'"
                    mycmdlist.append(cmdstr)

            casalog.post('Extracted ' + str(mycmdlist.__len__())
                         + ' flag commands', 'DEBUG')

            casalog.post('%s' % mycmdlist, 'DEBUG')
            casalog.post('%s' % myflagcmd, 'DEBUG')

        #
        # ACTION to perform on input file
        #
        casalog.post('Executing action = ' + action)

        # TODO: check row selection
        # List the flag commands from inpfile on the screen
        # and save them or not to outfile
        if action == 'list':

            # List the flag cmds on the screen
#            listFlagCmd(myflagcmd, myantenna=ants, myreason=reason, myoutfile='',
#                            listmode=listmode)
            listFlagCmd(myflagcmd, myoutfile='', listmode=listmode)

            # Save the flag cmds to the outfile
            if savepars:
                # These cmds came from the internal FLAG_CMD, only list on the screen
                if outfile == '':
                    if inpmode == 'table' and inpfile == '':
                        pass
                    else:
                        casalog.post('Saving commands to FLAG_CMD')
                        fh.writeFlagCmd(vis, myflagcmd, vrows=keylist,
                                applied=False, outfile='')
                else:
                    casalog.post('Saving commands to ' + outfile)
                    fh.writeFlagCmd(vis, myflagcmd, vrows=keylist,
                                    applied=False, outfile=outfile)
        elif action == 'apply' or action == 'unapply':

        # Apply/Unapply the flag commands to the data
            apply = True

            # Get the list of parameters
            cmdlist = []
            cmdkeys = myflagcmd.keys()
            for key in cmdkeys:
                cmdline = myflagcmd[key]['command']
                cmdlist.append(cmdline)

            # Select the data

            # Select a loose union of the data selection from the list.
            # The loose union will be calculated for field and spw only.
            # Antenna, correlation and timerange should be handled by the agent
            unionpars = {}
            if cmdlist.__len__() > 1:
#                unionpars = fh.getUnion(mslocal, vis, cmdlist)
                unionpars = fh.getUnion(mslocal, vis, myflagcmd)
                if len(unionpars.keys()) > 0:
                    casalog.post('Pre-selecting a subset of the MS: ')
                    casalog.post('%s' % unionpars)
                else:
                    casalog.post('Iterating through the entire MS')
                    
            # Get all the selection parameters, but set correlation to ''
            elif cmdlist.__len__() == 1:
                cmd0 = myflagcmd[cmdkeys[0]]['command']
                unionpars = fh.getSelectionPars(cmd0)
                casalog.post('The selected subset of the MS will be: ')
                casalog.post('%s' % unionpars)

            tflocal.selectdata(unionpars)

            # Parse the agents parameters
            if action == 'unapply':
                apply = False

            list2save = fh.setupAgent(tflocal, myflagcmd, tablerows, apply, True)

            # Initialize the Agents
            tflocal.init()

            # Backup the flags before running
            if flagbackup:
                fh.backupFlags(tflocal, 'tflagcmd')

            # Run the tool
            stats = tflocal.run(True, True)

            tflocal.done()

            # Update the APPLIED column
            valid_rows = list2save.keys()
            if valid_rows.__len__ > 0:

                if savepars:
                    # These flags came from internal FLAG_CMD. Always update APPLIED
                    if outfile == '':
                        if inpmode == 'table' and inpfile == '':
                            updateTable(vis, mycol='APPLIED',
                                    myval=apply, myrowlist=valid_rows)
                        else:
                            # save to FLAG_CMD
                            casalog.post('Saving commands to FLAG_CMD')
                            fh.writeFlagCmd(vis, myflagcmd,
                                    vrows=valid_rows, applied=apply,
                                    outfile='')
                    else:

                    # Save to a file
                        # Still need to update APPLIED column
                        if inpmode == 'table' and inpfile == '':
                            updateTable(vis, mycol='APPLIED',
                                    myval=apply, myrowlist=valid_rows)

                        casalog.post('Saving commands to file '
                                + outfile)
                        fh.writeFlagCmd(vis, myflagcmd,
                                vrows=valid_rows, applied=apply,
                                outfile=outfile)
                else:

                # Do not save cmds but maybe update APPLIED
                    if inpmode == 'table' and inpfile == '':
                        updateTable(vis, mycol='APPLIED', myval=apply,
                                    myrowlist=valid_rows)
            else:

                casalog.post('There are no valid commands to save',
                             'WARN')
        elif action == 'clear':

            # Clear flag commands from FLAG_CMD in vis
            msfile = vis

            if clearall:
                casalog.post('Deleting all rows from FLAG_CMD in MS '
                             + msfile)
                clearFlagCmd(msfile, myrowlist=rowlist)
            else:
                casalog.post('Safety Mode: you chose not to set clearall=True, no action'
                             )
        elif action == 'plot':

            keylist = myflagcmd.keys()
            if keylist.__len__() > 0:
                # Plot flag commands from FLAG_CMD or xml
                casalog.post('Warning: will only reliably plot individual per-antenna flags'
                             )
                plotflags(myflagcmd, plotfile, t1sdata, t2sdata)
            else:
                casalog.post('Warning: empty flag dictionary, nothing to plot'
                             )
        elif action == 'extract':
            # Output flag dictionary
            casalog.post('Returning extracted dictionary')
            return myflagcmd
        
    except Exception, instance:
        tflocal.done()
        casalog.post('%s' % instance, 'ERROR')
        raise

    # write history
    try:
        mslocal.open(vis, nomodify=False)
        mslocal.writehistory(message='taskname = tflagcmd',
                             origin='tflagcmd')
        mslocal.writehistory(message='vis      = "' + str(vis) + '"',
                             origin='testflagcmd')
        mslocal.writehistory(message='inpmode = "' + str(inpmode) + '"'
                             , origin='tflagcmd')
        if inpmode == 'file':
            mslocal.writehistory(message='inpfile = "' + str(inpfile)
                                 + '"', origin='tflagcmd')
        elif inpmode == 'cmd':
            for cmd in command:
                mslocal.writehistory(message='command  = "' + str(cmd)
                        + '"', origin='tflagcmd')
        mslocal.close()
    except:
        casalog.post('Cannot open vis for history, ignoring', 'WARN')

    return


# ************************************************************************
#                    Helper Functions
# ************************************************************************


def readFromTable(
    msfile,
    myflagrows=[],
    useapplied=True,
    myreason='any',
    ):
    '''Read flag commands from rows of the FLAG_CMD table of msfile
    If useapplied=False then include only rows with APPLIED=False
    If myreason is anything other than '', then select on that'''

    #
    # Return flagcmd structure:
    #
    # The flagcmd key is the integer row number from FLAG_CMD
    #
    #   Dictionary structure:
    #   key : 'id' (string)
    #         'mode' (string)         flag mode '','clip','shadow','quack'
    #         'antenna' (string)
    #         'timerange' (string)
    #         'reason' (string)
    #         'time' (float)          in mjd seconds
    #         'interval' (float)      in mjd seconds
    #         'cmd' (string)          string (for COMMAND col in FLAG_CMD)
    #         'type' (string)         'FLAG' / 'UNFLAG'
    #         'applied' (bool)        set to True here on read-in
    #         'level' (int)           set to 0 here on read-in
    #         'severity' (int)        set to 0 here on read-in

    # Open and read columns from FLAG_CMD
    mstable = msfile + '/FLAG_CMD'

    # Note, tb.getcol doesn't allow random row access, read all

    try:
        tb.open(mstable)
        f_time = tb.getcol('TIME')
        f_interval = tb.getcol('INTERVAL')
        f_type = tb.getcol('TYPE')
        f_reas = tb.getcol('REASON')
        f_level = tb.getcol('LEVEL')
        f_severity = tb.getcol('SEVERITY')
        f_applied = tb.getcol('APPLIED')
        f_cmd = tb.getcol('COMMAND')
        tb.close()
    except:
        casalog.post('Error reading table ' + mstable, 'ERROR')
        raise Exception

    nrows = f_time.__len__()

    myreaslist = []

    # Parse myreason
    if type(myreason) == str:
        if myreason != 'any':
            myreaslist.append(myreason)
    elif type(myreason) == list:
        myreaslist = myreason
    else:
        casalog.post('Cannot read reason; it contains unknown variable types'
                     , 'ERROR')
        return

    myflagcmd = {}
    if nrows > 0:
        nflagd = 0
        if myflagrows.__len__() > 0:
            rowlist = myflagrows
        else:
            rowlist = range(nrows)
        # Prune rows if needed
        if not useapplied:
            rowl = []
            for i in rowlist:
                if not f_applied[i]:
                    rowl.append(i)
            rowlist = rowl
        if myreaslist.__len__() > 0:
            rowl = []
            for i in rowlist:
                if myreaslist.count(f_reas[i]) > 0:
                    rowl.append(i)
            rowlist = rowl

        for i in rowlist:
            flagd = {}
            cmd = f_cmd[i]
            if cmd == '':
                casalog.post('Ignoring empty COMMAND string', 'WARN')
                continue

            # Extract antenna and timerange strings from cmd
            antstr = ''
            timstr = ''
            
            flagd['id'] = str(i)
            flagd['antenna'] = ''
            flagd['mode'] = ''
            flagd['time'] = f_time[i]
            flagd['interval'] = f_interval[i]
            flagd['type'] = f_type[i]
            flagd['reason'] = f_reas[i]
            flagd['level'] = f_level[i]
            flagd['severity'] = f_severity[i]
            flagd['applied'] = f_applied[i]

            # If shadow, remove the addantenna dictionary
            if cmd.__contains__('shadow') and cmd.__contains__('addantenna'):
                i0 = cmd.rfind('addantenna')
                if cmd[i0+11] == '{':
                    # It is a dictionary. Remove it from line
                    i1 = cmd.rfind('}')
                    antpar = cmd[i0+11:i1+1]
                    temp = cmd[i0:i1+1]
                    newcmd = cmd.replace(temp,'')
                    antpardict = fh.convertStringToDict(antpar)
                    flagd['addantenna'] = antpardict
                    cmd = newcmd                                
 
            flagd['command'] = cmd
            
            keyvlist = cmd.split()
            if keyvlist.__len__() > 0:
                for keyv in keyvlist:
                    try:
                        (xkey, val) = keyv.split('=')
                    except:
                        print 'Error: not key=value pair for ' + keyv
                        break
                    xval = val
                # strip quotes from value
                    if xval.count("'") > 0:
                        xval = xval.strip("'")
                    if xval.count('"') > 0:
                        xval = xval.strip('"')
                        
                    if xkey == 'mode':
                        flagd['mode'] = xval
                    elif xkey == 'timerange':
                        timstr = xval
                    elif xkey == 'antenna':
                        flagd['antenna'] = xval
                    elif xkey == 'id':
                        flagd['id'] = xval


            # STM 2010-12-08 Do not put timerange if not in command
            # if timstr=='':
            # ....    # Construct timerange from time,interval
            # ....    centertime = f_time[i]
            # ....    interval = f_interval[i]
            # ....    startmjds = centertime - 0.5*interval
            # ....    t = qa.quantity(startmjds,'s')
            # ....    starttime = qa.time(t,form="ymd",prec=9)
            # ....    endmjds = centertime + 0.5*interval
            # ....    t = qa.quantity(endmjds,'s')
            # ....    endtime = qa.time(t,form="ymd",prec=9)
            # ....    timstr = starttime+'~'+endtime
            flagd['timerange'] = timstr
            # Keep original key index, might need this later
            myflagcmd[i] = flagd
            nflagd += 1
        casalog.post('Read ' + str(nflagd)
                     + ' rows from FLAG_CMD table in ' + msfile)
    else:
        casalog.post('FLAG_CMD table in %s is empty, no flags extracted'
                      % msfile, 'WARN')

    return myflagcmd


def readFromCmd(cmdlist, ms_startmjds, ms_endmjds):
    '''Read the parameters from a list of commands'''

    # Read a list of strings and return a dictionary of parameters
    myflagd = {}
    nrows = cmdlist.__len__()
    if nrows == 0:
        casalog.post('WARNING: empty flag command list', 'WARN')
        return myflagd

    t = qa.quantity(ms_startmjds, 's')
    ms_startdate = qa.time(t, form=['ymd', 'no_time'])
    t0 = qa.totime(ms_startdate + '/00:00:00.0')
    # t0d = qa.convert(t0,'d')
    t0s = qa.convert(t0, 's')

    ncmds = 0
    for i in range(nrows):
        cmdstr = cmdlist[i]
        # break string into key=val sets
        keyvlist = cmdstr.split()
        if keyvlist.__len__() > 0:
            ant = ''
            timstr = ''
            tim = 0.5 * (ms_startmjds + ms_endmjds)
            intvl = ms_endmjds - ms_startmjds
            reas = ''
            cmd = ''
            fid = str(i)
            mode = ''
            typ = 'FLAG'
            appl = False
            levl = 0
            sevr = 0
            fmode = ''
            for keyv in keyvlist:
                # check for comment character #
                if keyv.count('#') > 0:
                    # break out of loop parsing keyvals
                    break
                try:
                    (xkey, val) = keyv.split('=')
                except:
                    print 'Not a key=val pair: ' + keyv
                    break
                xval = val
                # Use eval to deal with conversion from string
                # xval = eval(val)
                # strip quotes from value (if still a string)
                if type(xval) == str:
                    if xval.count("'") > 0:
                        xval = xval.strip("'")
                    if xval.count('"') > 0:
                        xval = xval.strip('"')

                # Strip these out of command string
                if xkey == 'reason':
                    reas = xval
                elif xkey == 'applied':
                    appl = False
                    if xval == 'True':
                        appl = True
                elif xkey == 'level':
                    levl = int(xval)
                elif xkey == 'severity':
                    sevr = int(xval)
                elif xkey == 'time':
                    tim = xval
                elif xkey == 'interval':
                    intvl = xval
                else:
                    # Extract (but keep in string)
                    if xkey == 'timerange':
                        timstr = xval
                        # Extract TIME,INTERVAL
                        try:
                            (startstr, endstr) = timstr.split('~')
                        except:
                            if timstr.count('~') == 0:
                            # print 'Assuming a single start time '
                                startstr = timstr
                                endstr = timstr
                            else:
                                print 'Not a start~end range: ' + timstr
                                print "ERROR: too may ~'s "
                                raise Exception, 'Error parsing ' \
                                    + timstr
                        t = qa.totime(startstr)
                        starts = qa.convert(t, 's')
                        if starts['value'] < 1.E6:
                            # assume a time offset from ref
                            starts = qa.add(t0s, starts)
                        startmjds = starts['value']
                        if endstr == '':
                            endstr = startstr
                        t = qa.totime(endstr)
                        ends = qa.convert(t, 's')
                        if ends['value'] < 1.E6:
                            # assume a time offset from ref
                            ends = qa.add(t0s, ends)
                        endmjds = ends['value']
                        tim = 0.5 * (startmjds + endmjds)
                        intvl = endmjds - startmjds
                    elif xkey == 'antenna':

                        ant = xval
                    elif xkey == 'id':
                        fid = xval
                    elif xkey == 'unflag':
                        if xval == 'True':
                            typ = 'UNFLAG'
                    elif xkey == 'mode':
                        fmode = xval
                    cmd = cmd + ' ' + keyv
            # Done parsing keyvals
            # Make sure there is a non-blank command string after reason/id extraction
            if cmd != '':
                flagd = {}
                flagd['id'] = fid
                flagd['mode'] = fmode
                flagd['antenna'] = ant
                flagd['timerange'] = timstr
                flagd['reason'] = reas
                flagd['command'] = cmd
                flagd['time'] = tim
                flagd['interval'] = intvl
                flagd['type'] = typ
                flagd['level'] = levl
                flagd['severity'] = sevr
                flagd['applied'] = appl
                # Insert into main dictionary
                myflagd[ncmds] = flagd
                ncmds += 1

    casalog.post('Parsed ' + str(ncmds) + ' flag command strings')

    return myflagd


def readFromFile(
    cmdlist,
    ms_startmjds,
    ms_endmjds,
    myreason='',
    ):
    '''Parse list of flag command strings and return dictionary of flagcmds
    Inputs:
       cmdlist (list,string) list of command strings (default for TIME,INTERVAL)
       ms_startmjds (float)  starting mjd (sec) of MS (default for TIME,INTERVAL)
       ms_endmjds (float)    ending mjd (sec) of MS'''

#
#   Usage: myflagcmd = getflags(cmdlist)
#
#   Dictionary structure:
#   fid : 'id' (string)
#         'mode' (string)         flag mode '','clip','shadow','quack'
#         'antenna' (string)
#         'timerange' (string)
#         'reason' (string)
#         'time' (float)          in mjd seconds
#         'interval' (float)      in mjd seconds
#         'cmd' (string)          string (for COMMAND col in FLAG_CMD)
#         'type' (string)         'FLAG' / 'UNFLAG'
#         'applied' (bool)        set to True here on read-in
#         'level' (int)           set to 0 here on read-in
#         'severity' (int)        set to 0 here on read-in
#
# v3.2 Updated STM 2010-12-03 (3.2.0) handle comments # again
# v3.2 Updated STM 2010-12-08 (3.2.0) bug fixes in flagsort use, parsing
# v3.3 Updated STM 2010-12-20 (3.2.0) bug fixes parsing errors
#

    myflagd = {}
    nrows = cmdlist.__len__()
    if nrows == 0:
        casalog.post('WARNING: empty flag command list', 'WARN')
        return myflagd

    # Parse the reason
    reasonlist = []
    if type(myreason) == str:
        if myreason != '':
            reasonlist.append(myreason)
    elif type(myreason) == list:
        reasonlist = myreason
    else:
        casalog.post('Cannot read reason; it contains unknown variable types'
                     , 'ERROR')
        return

    t = qa.quantity(ms_startmjds, 's')
    ms_startdate = qa.time(t, form=['ymd', 'no_time'])
    t0 = qa.totime(ms_startdate + '/00:00:00.0')
    # t0d = qa.convert(t0,'d')
    t0s = qa.convert(t0, 's')

    rowlist = []

    # IMPLEMENT THIS LATER
    # First select by reason. Simple selection...
#    if reasonlist.__len__() > 0:
#        for i in range(nrows):
#            cmdstr = cmdlist[i]
#            keyvlist = cmdstr.split()
#            if keyvlist.__len__() > 0:
#                for keyv in keyvlist:
#                    (xkey, xval) = keyv.split('=')
#
#                    if type(xval) == str:
#                        if xval.count("'") > 0:
#                            xval = xval.strip("'")
#                        if xval.count('"') > 0:
#                            xval = xval.strip('"')
#
#                    if xkey == 'reason':
#                        if reasonlist.count(xval) > 0
#    else:
    rowlist = range(nrows)

    # Now read the only the commands from the file that satisfies the reason selection

    ncmds = 0
#    for i in range(nrows):
    for i in rowlist:
        cmdstr = cmdlist[i]

        # break string into key=val sets
        keyvlist = cmdstr.split()
        if keyvlist.__len__() > 0:
            ant = ''
            timstr = ''
            tim = 0.5 * (ms_startmjds + ms_endmjds)
            intvl = ms_endmjds - ms_startmjds
            reas = ''
            cmd = ''
            fid = str(i)
            mode = ''
            typ = 'FLAG'
            appl = False
            levl = 0
            sevr = 0
            fmode = ''
            for keyv in keyvlist:
                # check for comment character #
                if keyv.count('#') > 0:
                    # break out of loop parsing keyvals
                    break
                try:
                    (xkey, val) = keyv.split('=')
                except:
                    print 'Not a key=val pair: ' + keyv
                    break
                xval = val
                # Use eval to deal with conversion from string
                # xval = eval(val)
                # strip quotes from value (if still a string)
                if type(xval) == str:
                    if xval.count("'") > 0:
                        xval = xval.strip("'")
                    if xval.count('"') > 0:
                        xval = xval.strip('"')

                # Strip these out of command string
                if xkey == 'reason':
                    reas = xval
                elif xkey == 'applied':

                    appl = False
                    if xval == 'True':
                        appl = True
                elif xkey == 'level':
                    levl = int(xval)
                elif xkey == 'severity':
                    sevr = int(xval)
                elif xkey == 'time':
                    tim = xval
                elif xkey == 'interval':
                    intvl = xval
                else:
                    # Extract (but keep in string)
                    if xkey == 'timerange':
                        timstr = xval
                        # Extract TIME,INTERVAL
                        try:
                            (startstr, endstr) = timstr.split('~')
                        except:
                            if timstr.count('~') == 0:
                            # print 'Assuming a single start time '
                                startstr = timstr
                                endstr = timstr
                            else:
                                print 'Not a start~end range: ' + timstr
                                print "ERROR: too may ~'s "
                                raise Exception, 'Error parsing ' \
                                    + timstr
                        t = qa.totime(startstr)
                        starts = qa.convert(t, 's')
                        if starts['value'] < 1.E6:
                            # assume a time offset from ref
                            starts = qa.add(t0s, starts)
                        startmjds = starts['value']
                        if endstr == '':
                            endstr = startstr
                        t = qa.totime(endstr)
                        ends = qa.convert(t, 's')
                        if ends['value'] < 1.E6:
                            # assume a time offset from ref
                            ends = qa.add(t0s, ends)
                        endmjds = ends['value']
                        tim = 0.5 * (startmjds + endmjds)
                        intvl = endmjds - startmjds
                    elif xkey == 'antenna':

                        ant = xval
                    elif xkey == 'id':
                        fid = xval
                    elif xkey == 'unflag':
                        if xval == 'True':
                            typ = 'UNFLAG'
                    elif xkey == 'mode':
                        fmode = xval
                    cmd = cmd + ' ' + keyv
            # Done parsing keyvals
            # Make sure there is a non-blank command string after reason/id extraction
            if cmd != '':
                flagd = {}
                flagd['id'] = fid
                flagd['mode'] = fmode
                flagd['antenna'] = ant
                flagd['timerange'] = timstr
                flagd['reason'] = reas
                flagd['command'] = cmd
                flagd['time'] = tim
                flagd['interval'] = intvl
                flagd['type'] = typ
                flagd['level'] = levl
                flagd['severity'] = sevr
                flagd['applied'] = appl
                # Insert into main dictionary
                myflagd[ncmds] = flagd
                ncmds += 1

    casalog.post('Parsed ' + str(ncmds) + ' flag command strings')

    return myflagd


def updateTable(
    msfile,
    mycol='',
    myval=None,
    myrowlist=[],
    ):
    '''Update commands in myrowlist of the FLAG_CMD table of msfile    
       Usage: updateflagcmd(msfile,myrow,mycol,myval)'''

    # Example:
    #
    #    updateflagcmd(msfile,mycol='APPLIED',myval=True)
    #       Mark all rows as APPLIED=True
    #
    #    updateflagcmd(msfile,mycol='APPLIED',myval=True,myrowlist=[0,1,2])
    #       Mark rows 0,1,2 as APPLIED=True
    #

    if mycol == '':
        casalog.post('WARNING: No column to was specified to update; doing nothing'
                     , 'WARN')
        return

    # Open and read columns from FLAG_CMD
    mstable = msfile + '/FLAG_CMD'
    try:
        tb.open(mstable, nomodify=False)
    except:
        raise Exception, 'Error opening table ' + mstable

    nrows = int(tb.nrows())

    # Check against allowed colnames
    colnames = tb.colnames()
    if colnames.count(mycol) < 1:
        casalog.post('Error: column mycol=' + mycol + ' not one of: '
                     + str(colnames))
        return

    nlist = myrowlist.__len__()

    if nlist > 0:
        rowlist = myrowlist
    else:

        rowlist = range(nrows)
        nlist = nrows

    if nlist > 0:
        try:
            tb.putcell(mycol, rowlist, myval)
        except:
            raise Exception, 'Error updating FLAG_CMD column ' + mycol \
                + ' to value ' + str(myval)

        casalog.post('Updated ' + str(nlist)
                     + ' rows of FLAG_CMD table in MS')
    tb.close()


def listFlagCmd(
    myflags=None,
    myantenna='',
    myreason='',
    myoutfile='',
    listmode='',
    ):
    '''List flags in myflags dictionary
    
    Format according to listmode:
        =''          do nothing
        ='file'      Format for flag command strings
        ='cmd'       Format for FLAG_CMD flags
        ='online'    Format for online flags'''

    #
    #   Dictionary structure:
    #   fid : 'id' (string)
    #         'mode' (string)         flag mode '','clip','shadow','quack'
    #         'antenna' (string)
    #         'timerange' (string)
    #         'reason' (string)
    #         'time' (float)          in mjd seconds
    #         'interval' (float)      in mjd seconds
    #         'cmd' (string)          string (for COMMAND col in FLAG_CMD)
    #         'type' (string)         'FLAG' / 'UNFLAG'
    #         'applied' (bool)
    #         'level' (int)
    #         'severity' (int)
    #

    useid = False
    doterm = False

    if myoutfile != '':
        try:
            lfout = open(myoutfile, 'w')
        except:
            raise Exception, 'Error opening list output file ' \
                + myoutfile

    keylist = myflags.keys()
    if keylist.__len__() == 0:
        casalog.post('There are no flags to list', 'WARN')
        return
    # Sort keys
#    keylist.sort

    # Set up any selection
    if myantenna != '':
        casalog.post('Selecting flags by antenna="' + str(myantenna)
                     + '"')
    myantlist = myantenna.split(',')

    if myreason != '':
        casalog.post('Selecting flags by reason="' + str(myreason) + '"'
                     )
    myreaslist = myreason.split(',')

    if listmode == 'online':
        phdr = '%8s %12s %8s %32s %48s' % ('Key', 'FlagID', 'Antenna',
                'Reason', 'Timerange')
    elif listmode == 'cmd':
        phdr = '%8s %45s %32s %6s %7s %3s %3s %s' % (
            'Row',
            'Timerange',
            'Reason',
            'Type',
            'Applied',
            'Level',
            'Severity',
            'Command',
            )
    elif listmode == 'file':
        phdr = '%8s %32s %s' % ('Key', 'Reason', 'Command')
    else:
        return

    if myoutfile != '':
        # list to output file
        print >> lfout, phdr
    else:
        # list to logger and screen
        if doterm:
            print phdr

        casalog.post(phdr)

    # Loop over flags
    for key in keylist:
        fld = myflags[key]
        # Get fields
        skey = str(key)
        if fld.has_key('id') and useid:
            fid = fld['id']
        else:
            fid = str(key)
        if fld.has_key('antenna'):
            ant = fld['antenna']
        else:
            ant = 'Unset'
        if fld.has_key('timerange'):
            timr = fld['timerange']
        else:
            timr = 'Unset'
        if fld.has_key('reason'):
            reas = fld['reason']
        else:
            reas = 'Unset'
        if fld.has_key('command'):
            cmd = fld['command']
            if fld.has_key('addantenna'):
                addantenna = fld['addantenna']
                cmd = cmd + ' addantenna=' + str(addantenna)
            
        else:
            cmd = 'Unset'
        if fld.has_key('type'):
            typ = fld['type']
        else:
            typ = 'FLAG'
        if fld.has_key('level'):
            levl = str(fld['level'])
        else:
            levl = '0'
        if fld.has_key('severity'):
            sevr = str(fld['severity'])
        else:
            sevr = '0'
        if fld.has_key('applied'):
            appl = str(fld['applied'])
        else:
            appl = 'Unset'

        # Print out listing
        if myantenna == '' or myantlist.count(ant) > 0:
            if myreason == '' or myreaslist.count(reas) > 0:
                if listmode == 'online':
                    pstr = '%8s %12s %8s %32s %48s' % (skey, fid, ant,
                            reas, timr)
                elif listmode == 'cmd':
                    pstr = '%8s %45s %32s %6s %7s %3s %3s %s' % (
                        skey,
                        timr,
                        reas,
                        typ,
                        appl,
                        levl,
                        sevr,
                        cmd,
                        )
                else:
                    pstr = '%8s %32s %s' % (skey, reas, cmd)
                if myoutfile != '':
                    # list to output file
                    print >> lfout, pstr
                else:
                    # list to logger and screen
                    if doterm:
                        print pstr
                    casalog.post(pstr)
    if myoutfile != '':
        lfout.close()


def backupCmd(tflocal, cmdlist):

        # Create names like this:
        # before_tflagcmd_1,
        # before_tflagcmd_2,
        #
        # Generally  before_<mode>_<i>, where i is the smallest
        # integer giving a name, which does not already exist

    prefix = 'tflagcmd'
    existing = tflocal.getflagversionlist(printflags=True)

    # remove comments from strings
    existing = [x[0:x.find(' : ')] for x in existing]
    i = 1
    while True:
        versionname = prefix + '_' + str(i)

        if not versionname in existing:
            break
        else:
            i = i + 1

    time_string = str(time.strftime('%Y-%m-%d %H:%M:%S'))

    casalog.post('Saving current flags to ' + versionname
                 + ' before applying new flags')

    tflocal.saveflagversion(versionname=versionname,
                            comment='tflagcmd autosave on '
                            + time_string, merge='replace')


def selectFlags(
    myflags=None,
    myantenna='',
    myreason='any',
    myflagsort='',
    ):

    #
    # Return dictionary of input flags using selection by antenna/reason
    # and grouped/sorted by flagsort.
    #
    #   selectFlags: Return dictionary of flags using selection by antenna/reason
    #              and grouped/sorted by flagsort.
    #      myflags (dictionary)  input flag dictionary (e.g. from readflagxml
    #      myantenna (string)    selection by antenna(s)
    #      myreason (string)     selection by reason(s)
    #      myflagsort (string)   toggle for flag group/sort
    #
    #   Usage: myflagd = selectFlags(myflags,antenna,reason,flagsort)
    #
    #   Dictionary structure:
    #   fid : 'id' (string)
    #         'mode' (string)         flag mode '','clip','shadow','quack','online'
    #         'antenna' (string)
    #         'timerange' (string)
    #         'reason' (string)
    #         'time' (float)          in mjd seconds
    #         'interval' (float)      in mjd seconds
    #         'cmd' (string)          command string (for COMMAND col in FLAG_CMD)
    #         'type' (string)         'FLAG' / 'UNFLAG'
    #         'applied' (bool)        set to True here on read-in
    #         'level' (int)           set to 0 here on read-in
    #         'severity' (int)        set to 0 here on read-in
    #
    # NOTE: flag sorting is needed to avoid error "Too many flagging agents instantiated"
    # If myflagsort=''              keep individual flags separate
    #              ='antenna'       combine all flags with a particular antenna
    #
    #
    # Check if any operation is needed
    if myantenna == '' and myreason == '' and myflagsort == '':
        print 'No selection or sorting needed - sortflags returning input dictionary'
        casalog.post('No selection or sorting needed - sortflags returning input dictionary'
                     )
        flagd = myflags
        return flagd
    #
    flagd = {}
    nflagd = 0
    keylist = myflags.keys()
    print 'Selecting from ' + str(keylist.__len__()) \
        + ' flagging commands'
    if keylist.__len__() > 0:
        #
        # Sort by key
        #
        keylist.sort()
        #
        # Construct flag command list for selected ant,reason
        #
    # print 'Selecting flags by antenna="'+str(myantenna)+'"'
        casalog.post('Selecting flags by antenna="' + str(myantenna)
                     + '"')
        myantlist = myantenna.split(',')

    # print 'Selecting flags by reason="'+str(myreason)+'"'
        casalog.post('Selecting flags by reason="' + str(myreason) + '"'
                     )
        myreaslist = []
    # Parse myreason
        if type(myreason) == str:
            if myreason == '':
                print 'WARNING: reason= is treated as selection on a blank REASON!'
                casalog.post('WARNING: reason= is treated as selection on a blank REASON!'
                             , 'WARN')
            if myreason != 'any':
                myreaslist.append(myreason)
        elif type(myreason) == list:
            myreaslist = myreason
        else:
            print 'ERROR: reason contains unallowed variable type'
            casalog.post('ERROR: reason contains unknown variable type'
                         , 'SEVERE')
            return
        if myreaslist.__len__() > 0:
            print 'Selecting for reasons: ' + str(myreaslist)
            casalog.post('Selecting for reasons: ' + str(myreaslist))
        else:
            print 'No selection on reason'
            casalog.post('No selection on reason')

    # Note antenna and reason selection checks for inclusion not exclusivity
        doselect = myantenna != '' or myreaslist.__len__() > 0

    # Now loop over flags, break into sorted and unsorted groups
        nsortd = 0
        sortd = {}
        sortdlist = []
        nunsortd = 0
        unsortd = {}
        unsortdlist = []
        if myflagsort == 'antenna':
        # will be resorting by antenna
            for key in keylist:
                myd = myflags[key]
                mymode = myd['mode']
                ant = myd['antenna']
        # work out number of selections for this flag command
                nselect = 0
                if myd.has_key('timerange'):
                    if myd['timerange'] != '':
                        nselect += 1
                if myd.has_key('spw'):
                    if myd['spw'] != '':
                        nselect += 1
                if myd.has_key('field'):
                    if myd['field'] != '':
                        nselect += 1
                if myd.has_key('corr'):
                    if myd['corr'] != '':
                        nselect += 1
                if myd.has_key('scan'):
                    if myd['scan'] != '':
                        nselect += 1
                if myd.has_key('intent'):
                    if myd['intent'] != '':
                        nselect += 1
                if myd.has_key('feed'):
                    if myd['feed'] != '':
                        nselect += 1
                if myd.has_key('uvrange'):
                    if myd['uvrange'] != '':
                        nselect += 1
                if myd.has_key('observation'):
                    if myd['observation'] != '':
                        nselect += 1
        # check if we can sort this by antenna
                antsort = False
        # can only sort mode manualflag together
                if mymode == 'online':
                    antsort = nselect == 1
                elif mymode == '' or mymode == 'manualflag':
            # must have non-blank antenna selection
                    if ant != '':
                # exclude flags with multiple/no selection
                        if myd.has_key('timerange'):
                            antsort = myd['timerange'] != '' \
                                and nselect == 1

                if antsort:
                    if ant.count('&') > 0:
                # for baseline specifications split on ;
                        antlist = ant.split(';')
                    else:
                # for antenna specifications split on ,
                        antlist = ant.split(',')
            # break this flag by antenna
                    for a in antlist:
                        if myantenna == '' or myantlist.count(a) > 0:
                            addf = False
                            reas = myd['reason']
                            reaslist = reas.split(',')
                            reastr = ''
                            if myreaslist.__len__() > 0:
                                for r in myreaslist:
                                    if r == reas or reaslist.count(r) \
    > 0:
                                        addf = True
                    # check if this is a new reason
                                        if reastr != '':
                                            rlist = reastr.split(',')
                                            if rlist.count(r) == 0:
                                                reastr += ',' + r
                                        else:
                                            reastr = r
                            else:
                                addf = True
                                reastr = reas
                #
                            if addf:
                    # check if this is a new antenna
                                if sortd.has_key(a):
                        # Already existing flag for this antenna, add this one
                                    t = sortd[a]['timerange']
                                    tlist = t.split(',')
                                    timelist = myd['timerange'
        ].split(',')
                                    for tim in timelist:
                            # check if this is a new timerange
                                        if tlist.count(tim) == 0:
                                            t += ',' + tim
                                    sortd[a]['timerange'] = t
                                    reas = sortd[a]['reason']
                                    if reastr != '':
                                        reas += ',' + reastr
                                    sortd[a]['reason'] = reas
                    # adjust timerange in command string
                                    cmdstr = ''
                                    cmdlist = sortd[a]['cmd'].split()
                                    for cmd in cmdlist:
                                        (cmdkey, cmdval) = cmd.split('='
        )
                                        if cmdkey == 'timerange':
                                            cmdstr += " timerange='" \
    + t + "'"
                                        elif cmdkey == 'reason':
                                            cmdstr += " reason='" \
    + reastr + "'"
                                        else:
                                            cmdstr += ' ' + cmd
                                    sortd[a]['cmd'] = cmdstr
                    # adjust other keys
                                    if myd['level'] > sortd[a]['level']:
                                        sortd[a]['level'] = myd['level']
                                    if myd['severity'] \
    > sortd[a]['severity']:
                                        sortd[a]['severity'] = \
    myd['severity']
                                else:
                        # add this flag (copy most of it)
                                    sortd[a] = myd
                                    sortd[a]['id'] = a
                                    sortd[a]['antenna'] = a
                                    sortd[a]['reason'] = reastr
                else:
            # cannot compress flags from this mode, add to unsortd instead
            # doesn't clash
                    unsortd[nunsortd] = myd
                    nunsortd += 1
            sortdlist = sortd.keys()
            nsortd = sortdlist.__len__()
            unsortdlist = unsortd.keys()
        else:
        # All flags are in unsorted list
            unsortd = myflags.copy()
            unsortdlist = unsortd.keys()
            nunsortd = unsortdlist.__len__()

        print 'Found ' + str(nsortd) + ' sorted flags and ' \
            + str(nunsortd) + ' incompressible flags'
        casalog.post('Found ' + str(nsortd) + ' sorted flags and '
                     + str(nunsortd) + ' incompressible flags')

    # selection on unsorted flags
        if doselect and nunsortd > 0:
            keylist = unsortd.keys()
            for key in keylist:
                myd = unsortd[key]
                ant = myd['antenna']
                antlist = ant.split(',')
                reas = myd['reason']
                reaslist = reas.split(',')
        # break this flag by antenna
                antstr = ''
                reastr = ''
                addf = False

                for a in antlist:
                    if myantenna == '' or myantlist.count(a) > 0:
                        addr = False
                        if myreaslist.__len__() > 0:
                            for r in myreaslist:
                                if reas == r or reaslist.count(r) > 0:
                                    addr = True
                    # check if this is a new reason
                                    rlist = reastr.split(',')
                                    if reastr != '':
                                        rlist = reastr.split(',')
                                        if rlist.count(r) == 0:
                                            reastr += ',' + r
                                    else:
                                        reastr = r
                        else:
                            addr = True
                            reastr = reas
                        if addr:
                            addf = True
                            if antstr != '':
                    # check if this is a new antenna
                                alist = antstr.split(',')
                                if alist.count(a) == 0:
                                    antstr += ',' + a
                            else:
                                antstr = a
                if addf:
                    flagd[nflagd] = myd
                    flagd[nflagd]['antenna'] = antstr
                    flagd[nflagd]['reason'] = reastr
                    nflagd += 1
            flagdlist = flagd.keys()
        elif nunsortd > 0:
        # just copy to flagd w/o selection
            flagd = unsortd.copy()
            flagdlist = flagd.keys()
            nflagd = flagdlist.__len__()

        if nsortd > 0:
        # Add sorted keys back in to flagd
            print 'Adding ' + str(nsortd) + ' sorted flags to ' \
                + str(nflagd) + ' incompressible flags'
            casalog.post('Adding ' + str(nsortd) + ' sorted flags to '
                         + str(nflagd) + ' incompressible flags')
            sortdlist.sort()
            for skey in sortdlist:
                flagd[nflagd] = sortd[skey]
                nflagd += 1

        if nflagd > 0:
            print 'Found total of ' + str(nflagd) \
                + ' flags meeting selection/sorting criteria'
            casalog.post('Found total of ' + str(nflagd)
                         + ' flags meeting selection/sorting criteria')
        else:
            print 'No flagging commands found meeting criteria'
            casalog.post('No flagging commands found meeting criteria')
    else:
        print 'No flags found in input dictionary'
        casalog.post('No flags found in input dictionary')

    return flagd


# Done


def clearFlagCmd(msfile, myrowlist=[]):
    #
    # Delete flag commands (rows) from the FLAG_CMD table of msfile
    #
    # Open and read columns from FLAG_CMD

    mstable = msfile + '/FLAG_CMD'
    try:
        tb.open(mstable, nomodify=False)
    except:
        raise Exception, 'Error opening table ' + mstable

    nrows = int(tb.nrows())
    casalog.post('There were ' + str(nrows) + ' rows in FLAG_CMD')
    if nrows > 0:
        if myrowlist.__len__() > 0:
            rowlist = myrowlist
        else:
            rowlist = range(nrows)
        try:
            tb.removerows(rowlist)
            casalog.post('Deleted ' + str(rowlist.__len__())
                         + ' from FLAG_CMD table in MS')
        except:
            tb.close()
            raise Exception, 'Error removing rows ' + str(rowlist) \
                + ' from table ' + mstable

        nnew = int(tb.nrows())
    else:
        casalog.post('No rows to clear')

    tb.close()


def plotflags(
    myflags,
    plotname,
    t1sdata,
    t2sdata,
    ):

    try:
        import casac
    except ImportError, e:
        print 'failed to load casa:\n', e
        exit(1)
    qatool = casac.homefinder.find_home_by_name('quantaHome')
    qa = casac.qa = qatool.create()

    try:
        import pylab as pl
    except ImportError, e:
        print 'failed to load pylab:\n', e
        exit(1)

    # get list of flag keys
    keylist = myflags.keys()

    # get list of antennas
    myants = []
    for key in keylist:
        ant = myflags[key]['antenna']
        if myants.count(ant) == 0:
            myants.append(ant)
    myants.sort()

    antind = 0
    if plotname == '':
        pl.ion()
    else:
        pl.ioff()

    f1 = pl.figure()
    ax1 = f1.add_axes([.15, .1, .75, .85])
#    ax1.set_ylabel('antenna')
#    ax1.set_xlabel('time')
    badflags = []
    for thisant in myants:
        antind += 1
        for thisflag in myflags:
            if myflags[thisflag]['antenna'] == thisant:
            # print thisant, myflags[thisflag]['reason'], myflags[thisflag]['timerange']
                thisReason = myflags[thisflag]['reason']
                if thisReason == 'FOCUS_ERROR':
                    thisColor = 'red'
                    thisOffset = 0.29999999999999999
                elif thisReason == 'SUBREFLECTOR_ERROR':
                    thisColor = 'blue'
                    thisOffset = .15
                elif thisReason == 'ANTENNA_NOT_ON_SOURCE':
                    thisColor = 'green'
                    thisOffset = 0
                elif thisReason == 'ANTENNA_NOT_IN_SUBARRAY':
                    thisColor = 'black'
                    thisOffset = -.15
                else:
                    thisColor = 'orange'
                    thisOffset = 0.29999999999999999
                mytimerange = myflags[thisflag]['timerange']
                if mytimerange != '':
                    t1 = mytimerange[:mytimerange.find('~')]
                    t2 = mytimerange[mytimerange.find('~') + 1:]
                    (t1s, t2s) = (qa.convert(t1, 's')['value'],
                                  qa.convert(t2, 's')['value'])
                else:
                    t1s = t1sdata
                    t2s = t2sdata
                myTimeSpan = t2s - t1s
                if myTimeSpan < 10000:
                    ax1.plot([t1s, t2s], [antind + thisOffset, antind
                             + thisOffset], color=thisColor, lw=2,
                             alpha=.7)
                else:
                    badflags.append((thisant, myTimeSpan, thisReason))

    # #badflags are ones which span a time longer than that used above
    # #they can be so long that including them compresses the time axis so that none of the other flags are visible
#    print 'badflags', badflags
    myXlim = ax1.get_xlim()
    myXrange = myXlim[1] - myXlim[0]
    legendFontSize = 12
    ax1.text(myXlim[0] + .05 * myXrange, 29, 'FOCUS', color='red',
             size=legendFontSize)
    ax1.text(myXlim[0] + .17 * myXrange, 29, 'SUBREFLECTOR',
             color='blue', size=legendFontSize)
    ax1.text(myXlim[0] + .42 * myXrange, 29, 'OFF SOURCE', color='green'
             , size=legendFontSize)
    ax1.text(myXlim[0] + .62 * myXrange, 29, 'NOT IN SUBARRAY',
             color='black', size=legendFontSize)
    ax1.text(myXlim[0] + .90 * myXrange, 29, 'Other', color='orange',
             size=legendFontSize)
    ax1.set_ylim([0, 30])

    ax1.set_yticks(range(1, len(myants) + 1))
    ax1.set_yticklabels(myants)
    ax1.set_xticks(pl.linspace(myXlim[0], myXlim[1], 3))

    mytime = [myXlim[0], (myXlim[1] + myXlim[0]) / 2.0, myXlim[1]]
    myTimestr = []
    for time in mytime:
        q1 = qa.quantity(time, 's')
        time1 = qa.time(q1, form='ymd', prec=9)
        myTimestr.append(time1)

    ax1.set_xticklabels([myTimestr[0], (myTimestr[1])[11:],
                        (myTimestr[2])[11:]])
    # print myTimestr
    if plotname == '':
        pl.draw()
    else:
        pl.savefig(plotname, dpi=150)
    return


def isModeValid(line):
    '''Check if mode is valid based on a line
       molinede --> line with strings
        Returns True if mode is either one of the following:
           '',manual,clip,quack,shadow,elevation      '''

    if line.__contains__('mode'):
        if (line.__contains__('manual') or line.__contains__('clip') or
            line.__contains__('quack') or line.__contains__('shadow') or
            line.__contains__('elevation')):
            return True
        else:
            return False

    # No mode means manual
    return True

