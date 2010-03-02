import sys
import os
import inspect
import string
import time
from taskinit import *
import viewertool


class __viewer_class(object):
	"viewer() task with local state for created viewer tool"

	def __init__( self ):
		self.local_vi = None
		self.local_ving = None

	def __call__(self, infile=None,displaytype=None,frame=None,zoom=None,outfile=None,outscale=None,outdpi=None,outformat=None,outlandscape=None,gui=None):
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
			if a[k][1] == "<string>" or (string.find(a[k][1], 'ipython console') > 0 or string.find(a[k][1],"casapy.py") > 0):
				stacklevel=k

		myf=sys._getframe(stacklevel).f_globals

		#Python script
		try:
			## vi might not be defined in taskinit if loading
			## directly from python via casa.py...
			vwr = vi
			if type(gui) == bool and gui == False:
				vwr = ving

			if type(vwr.cwd( )) != str:
				vwr = None
		except:
			vwr = None

		if vwr == None:
			need_gui = True
			if type(gui) == bool and gui == False:
				need_gui = False

			if need_gui :
				if self.local_vi is not None:
					vwr = self.local_vi
				else:
					vwr = viewertool.viewertool( True, True, (type(myf) == dict and myf.has_key('casa') and type(myf['casa']) == type(os)) )
					self.local_vi = vwr
			else:
				if self.local_ving is not None:
					vwr = self.local_ving
				else:
					vwr = viewertool.viewertool( False, True, (type(myf) == dict and myf.has_key('casa') and type(myf['casa']) == type(os)) )
					self.local_ving = vwr

		if vwr != None :
			##
			## (1) save current *viewer*server* path
			## (2) have viewer() task follow casapy/python's cwd
			old_path = vwr.cwd( )
			vwr.cwd(os.path.abspath(os.curdir))

			panel = vwr.panel("viewer")
			data = None
			if type(infile) == str and len(infile) > 0 :
				if type(displaytype) == str:
					data = vwr.load( infile, displaytype, panel=panel )
				else:
					data = vwr.load( infile, panel=panel )

				if type(frame) == int and frame > 0 :
					vwr.frame(frame,panel=panel)
				if type(zoom) == int and zoom != 1 :
					vwr.zoom(zoom,panel=panel)
				if type(outfile) == str and len(outfile) > 0 :
					scale=1.0
					if type(outscale) == float :
						scale=outscale
					dpi=300
					if type(outdpi) == int :
						dpi=outdpi
					format="jpg"
					if type(outformat) == str :
						format=outformat
					orientation="portrait"
					if type(outlandscape) == bool and outlandscape :
						orientation="landscape"
					vwr.output(outfile,scale=scale,dpi=dpi,format=format,orientation=orientation,panel=panel)

			# it makes no sense to leave a panel open with no way of interacting with it
			if type(gui) == bool and not gui:
				vwr.close(panel)

			## (3) restore original path
			vwr.cwd(old_path)

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

viewer = __viewer_class( )
