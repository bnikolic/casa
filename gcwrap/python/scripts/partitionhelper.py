import os
import sys
import shutil
import string
from taskinit import casalog,ms
import numpy as np


def getScanNrows(msfile, myscan):
    '''Get the number of rows of a scan in a MS. It will add the nrows of all sub-scans.
       msfile  --> name of the MS
       myscan  --> scanID
       return the number of rows in the scan.
       
       To compare with the dictionary returned by listpartition, do the following:
        
        resdict = listpartition(vis='part.mms', createdict=True)
        for k in resdict.keys():
            subms = resdict[k]['MS']
            MS = 'part.mms/SUBMSS/'+subms
            scans = resdict[k]['scanId'].keys()
            for s in scans:
                nr = resdict[k]['scanId'][s]['nrows']
                refN = getScanNrows(MS,s)
                assert nr==refN, '%s, scan=%s, nrows=%s do not match reference nrows=%s'%(MS, s, nr, refN)

    '''
        
    ms.open(msfile)
    scand = ms.getscansummary()
    ms.close()
    
    Nrows = 0
    subscans = scand[str(myscan)]
    for ii in subscans.keys():
        Nrows += scand[str(myscan)][ii]['nRow']
    
    return Nrows


def getDiskUsage(msfile):
    '''Return the size in bytes of an MS in disk.
       msfile  --> name of the MS
       This function will return a value given by
       the command du -hs'''
    
    from subprocess import Popen, PIPE, STDOUT

    # Command line to run
    ducmd = 'du -hs '+msfile
    
    p = Popen(ducmd, shell=True, stdin=PIPE, stdout=PIPE, stderr=STDOUT, close_fds=True)
    
    sizeline = p.stdout.read()
    
    # Create a list of the output string, which looks like this:
    # ' 75M\tuidScan23.data/uidScan23.0000.ms\n'
    # This will create a list with [size,sub-ms]
    mssize = sizeline.split()

    return mssize[0]

def getSpwIds(msfile, myscan):
    '''Get the Spw IDs of a scan. 
       msfile  --> name of the MS
       myscan  --> scanID
       return a numpy array with the Spw IDs. Note that the returned spw IDs are sorted.
       
       To compare with the dictionary returned by listpartition, do the following:
        
        resdict = listpartition(vis='part.mms', createdict=True)
        for k in resdict.keys():
            subms = resdict[k]['MS']
            MS = 'part.mms/SUBMSS/'+subms
            scans = resdict[k]['scanId'].keys()
            for s in scans:
                spws = resdict[k]['scanId'][s]['spwIds']
                refspws = getSpwIds(MS,s)
                
                # Compare the two arrays
                compspws = np.in1d(refspws, spws)
                indices = np.where(compspws==False)
                if np.size(indices) != 0:
                    for i in indices:
                        print "Spw=%s is not in reference array %s"%(spws[indices[i]], refspws)
                    
    '''
        
    ms.open(msfile)
    scand = ms.getscansummary()
    ms.close()
    
    subscans = scand[str(myscan)]
    aspws = np.array([],dtype=int)
    
    for ii in subscans.keys():
        sscanid = ii
        spwids = scand[str(myscan)][sscanid]['SpwIds']
        aspws = np.append(aspws,spwids)
    
    # Sort spws  and remove duplicates
    aspws.sort()
    uniquespws = np.unique(aspws)
    
    return uniquespws









