import sys
import os
import inspect
import string
import time
from taskinit import *
from task_viewerconnection import viewerconnection
from task_viewerconnection import dbus_connection

def viewer(infile=None,displaytype=None):
	""" The viewer will display images in raster, contour, vector or
	marker form.  Images can be blinked, and movies are available
	for spectral-line image cubes.  For measurement sets, many
	display and editing options are available.

	examples of usage:

	viewer
	viewer "myimage.im"
	viewer "mymeasurementset.ms"
	viewer "myrestorefile.rstr"
	
	viewer "myimage.im", "contour"

	viewer "'myimage1.im' - 2 * 'myimage2.im'", "lel"
	
	The viewer can be run outside of casapy by typing <casaviewer>.

	Executing viewer <viewer> will bring up a display panel
	window, which can be resized.  If no data file was specified,
	a Load Data window will also appear.  Click on the desired data
	file and choose the display type; the rendered data should appear
	on the display panel.

	A Data Display Options window will also appear.  It has drop-down
	subsections for related	options, most of which are self-explanatory.
	  
	The state of the viewer -- loaded data and related display
	options -- can be saved in a 'restore' file for later use.
	You can provide the restore filename on the command line or
	select it from the Load Data window.

	See the cookbook for more details on using the viewer.
	
	Keyword arguments:
	infile -- Name of file to visualize
		default: ''
		example: infile='ngc5921.image'
		If no infile is specified the Load Data window
		will appear for selecting data.
	displaytype -- (optional): method of rendering data
		visually (raster, contour, vector or marker).  
		You can also set this parameter to 'lel' and
		provide an lel expression for infile (advanced).
		default: 'raster'
		example: displaytype='contour'

	Note: there is no longer a filetype parameter; typing of
	data files is now done automatically.
	        example:  viewer infile='my.ms'
		obsolete: viewer infile='my.ms', filetype='ms'


	"""
        a=inspect.stack()
        stacklevel=0    
        for k in range(len(a)):
           if (string.find(a[k][1], 'ipython console') > 0):
                      stacklevel=k
        myf=sys._getframe(stacklevel).f_globals

	#Python script
	use_tool = True
	try:
		if type(vi.cwd( )) == str:
			use_tool = True
		else:
			use_tool = False
	except:
		use_tool = False

	if use_tool:
		panel = vi.panel("viewer")
		data = None
		if type(infile) == str:
			if type(displaytype) == str:
				data = vi.load( infile, displaytype, panel=panel )
			else:
				data = vi.load( infile, panel=panel )
	else:
		viewer_path = myf['casa']['helpers']['viewer']   #### set in casapy.py
		args = [ viewer_path ]

		if type(infile) == str:
			if type(displaytype) == str:
				args += [ infile, displaytype ]
			else:
				args += [ infile ]

		if (os.uname()[0]=='Darwin'):
			vwrpid=os.spawnvp( os.P_NOWAIT, viewer_path, args )
		elif (os.uname()[0]=='Linux'):
			vwrpid=os.spawnlp( os.P_NOWAIT, viewer_path, *args )
		else:
			print 'Unrecognized OS: No viewer available'

	return None
