#import os
from taskinit import *
from imregion import *

from odict import odict

def imcontsub(imagename=None,linefile=None,contfile=None,fitorder=None,region=None,box=None,chans=None,stokes=None,xstat=None):


	#Python script
	#
	try:
	    casalog.origin('imcontsub')
	    # Get the region information
	    # Note that there could be issues if in the region file
	    # there were channel selections that don't match what the
	    # user is giving as input.
	    if ( len(region)>1 ):
		if ( len(box)>1 or len(stokes)>1 ):
		    casalog.post( "Ignoring region selection\ninformation"\
				  " the in box, chans, and stokes parameters."\
				  " Using region information\nin file: "\
				  + region, 'WARN' );
		reg=rg.fromfiletorecord( region );
	    else:
	    	#reg=imregion( imagename, chans, stokes, box, '', '' )
		reg=imregion( imagename, '', stokes, box, '', '' )

	    # Find the max and min channel.
	    axes=getimaxes(imagename)
	    ia.open( imagename )
	    fullSize=ia.boundingbox(ia.setboxregion())
	    minChan=int(fullSize['blc'][axes[2][0]])
	    maxChan=int(fullSize['trc'][axes[2][0]])


	    # Parse the channel information.
	    tmpChan=[]
	    if ( len(chans) > 0 ):
		if ( chans.count( ';' ) < 1 ):
		    tmpChans=chans.split(',')
		else:
		    tmpChans=chans.split(';')
	    else:
		tmpChans=[str(minChan)+"~"+str(maxChan)]

	    	    
	    # Now make our list of strings into a list of integers.
	    # In case someone has used ',' as the main separator we
	    # split each element up on ','
	    channels=[]
	    for i in range( len( tmpChans ) ):
		current = _parse_chans( tmpChans[i], minChan, maxChan )
		for j in range( len( current ) ):
		    channels.append( current[j] )
		    
	    # Now do the continuum subtraction.
            #ia.continuumsub( outline=linefile, outcont=contfile, region=reg, fitorder=fitorder, overwrite=True )
	    ia.continuumsub( outline=linefile, outcont=contfile, region=reg, channels=channels, fitorder=fitorder, overwrite=True )
            
            # Cleanup
            del reg
            ia.done
	    return
	    #return retValue
            
		
	except Exception, instance:
	    print '*** Error ***',instance
	    return 
	
	return

#
#TODO add a try/catch block or type checking to make
#sure all channel values are ints.
def _parse_chans( chanString='', min=0, max=0 ):
    retValue=[]
    startChan=min;
    endChan=max;

    values=chanString.split('~')
    if ( len(values)==2 ):
	# We have a min and max value
	startChan=int(values[0])
	endChan=int(values[1])
    elif ( len(values)==1 ):
	# We probably have a <, <=, > or >= sign
	if ( values[0].startswith( '<=') ):
	    endChan=int(values[0][2:])
	elif ( values[0].startswith( '<') ):
	    endChan=int(values[0][1:])-1
	elif( values[0].startswith( '>=') ):
	    startChan=int(values[0][2:])
	elif( values[0].startswith( '>') ):
	    startChan=int(values[0][1:])+1 
	else:
	    # We have a list of specific channel numbers
	    startChan=endChan=-1
	    tmpList = chanString.split( ',' )
	    for j in range( len( tmpList ) ):
		retValue.append( int( tmpList[j] ) )

    if ( startChan >= 0 and endChan >= 0 ):
	for i in range( startChan, endChan+1 ):
	    retValue.append( i )
    return retValue
