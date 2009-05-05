# (partial) TODO list - also search for XXX in the code
# Find timerange when at least one of the inputs is up, limit the
#       observing time to it, and warn or abort (printing the valid
#       timerange) depending on how badly the user missed it.
# look for antenna list in default repository also
# allow user to report statistics in Jy/bm instead of Jy/arcsec
# Pad input image to a composite number of pixels on a side.  I don't
#       know if it would speed things up much, but it would suppress a warning.
#       Large primes are surprisingly common.



import os
from clean import clean
from taskinit import *
from simutil import *
import pylab as pl
import pdb


def simdata(modelimage=None, modifymodel=None, refdirection=None, refpixel=None, incell=None, inbright=None, complist=None, antennalist=None, checkinputs=None, project=None, refdate=None, totaltime=None, integration=None, startfreq=None, chanwidth=None, nchan=None, direction=None, pointingspacing=None, relmargin=None, cell=None, imsize=None, niter=None, threshold=None, psfmode=None, weighting=None, robust=None, uvtaper=None, outertaper=None, innertaper=None, noise=None, npixels=None, stokes=None, noise_thermal=None, t_amb=None, tau0=None, fidelity=None, display=None, verbose=False, async=None):




# helper function to plot an image (optionally), and calculate its statistics
# we could move this to the utility object

    def statim(image,plot=True,incell=None):
        ia.open(image)
        imunit=ia.summary()['header']['unit']            
        if imunit == 'Jy/beam':
            # stupid for dirty image:
            if len(ia.restoringbeam())>0:
                bm=ia.summary()['header']['restoringbeam']['restoringbeam']
                toJyarcsec=1./(qa.convert(bm['major'],'arcsec')['value']*
                               qa.convert(bm['minor'],'arcsec')['value']*pl.pi/4)
            else:
                toJyarcsec=1.
            pix=ia.summary()['header']['incr']
            toJypix=toJyarcsec*abs(pix[0]*pix[1])*206265.0**2
        elif imunit == 'Jy/pixel':
            pix=ia.summary()['header']['incr']
            toJyarcsec=1./abs(pix[0]*pix[1])/206265.0**2
            toJypix=1.
        else:
            msg("WARN: don't know image units for %s" % image,color="31")
            toJyarcsec=1.
            toJypix=1.
        stats=ia.statistics(robust=True)
        #im_max=stats['max']*toJyarcsec
        #im_min=stats['min']*toJyarcsec
        im_max=stats['max']*toJypix
        im_min=stats['min']*toJypix
        imsize=ia.shape()[0:2]
        reg1=rg.box([0,0],[imsize[0]*.25,imsize[1]*.25])
        stats=ia.statistics(region=reg1)
        #im_rms=stats['rms']*toJyarcsec
        im_rms=stats['rms']*toJypix
        if len(im_rms)==0: im_rms=0.
        data_array=ia.getchunk([-1,-1,1,1],[-1,-1,1,1],[1],[],True,True,False)
        data_array=pl.array(data_array)
        tdata_array=pl.transpose(data_array)
        ttrans_array=tdata_array.tolist()
        ttrans_array.reverse()
        if (plot):
            csys=ia.coordsys()
            pixsize= csys.increment()["numeric"][0:2]*180/pl.pi*3600 # to arcsec
            if incell != None:
                if type(incell)=='str':
                    incell=qa.quantity(incell)
                pixsize=qa.convert(incell,'arcsec')['value']+pl.zeros(2)
            if verbose: msg("pixel size= %s" % pixsize,origin="statim")
            xextent=imsize[0]*abs(pixsize[0])*0.5
            xextent=[xextent,-xextent]
            yextent=imsize[1]*abs(pixsize[1])*0.5
            yextent=[-yextent,yextent]
            # remove top .5% of pixels:
            nbin=200
            imhist=ia.histograms(cumu=True,nbins=nbin)['histout']
            ii=nbin-1
            highcounts=imhist['counts'][ii]
            while imhist['counts'][ii]>0.995*highcounts and ii>0: 
                ii=ii-1
            highvalue=imhist['values'][ii]
            #
            pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,vmax=highvalue)
            ax=pl.gca()
            l=ax.get_xticklabels()
            pl.setp(l,fontsize="x-small")
            l=ax.get_yticklabels()
            pl.setp(l,fontsize="x-small")
            pl.title(image,fontsize="x-small")
            from matplotlib.font_manager import fontManager, FontProperties
            font= FontProperties(size='x-small');
            pl.legend(("min=%7.1e" % im_min,"max=%7.1e" % im_max,"RMS=%7.1e" % im_rms),pad=0.15,prop=font)
        ia.done()
        return im_min,im_max,im_rms





    ############ main function ############################################


    casalog.origin('simdata')

    bandwidth=qa.mul(qa.quantity(nchan),qa.quantity(chanwidth))
    util=simutil(direction,startfreq=qa.quantity(startfreq),
                 bandwidth=bandwidth)
    if verbose: util.verbose=True
    msg=util.msg
    
    if((not os.path.exists(modelimage)) and (not os.path.exists(complist))):
        msg("ERROR -- No sky input found.  At least one of modelimage or complist must be set.",color="31")
        return
    
    try:
        ##################################################################
        # set up pointings
        # uses imsize*cell, or could use insize*incell I suppose

        nfld, pointings = util.calc_pointings(pointingspacing,imsize,cell,direction,relmargin)
        if nfld==1:
            imagermode=''
        else:
            imagermode="mosaic"
        ave , off = util.average_direction(pointings)

        nbands = 1;    
        fband  = 'band'+startfreq

        msfile=project+'.ms'
        casalog.origin('simdata')

        ##################################################################
        # if needed, make a model image from the input clean component list

        if (modelimage == ''):
            if incell=="header":
                incell=cell
            # should we be using incell since there is not an image?
            if verbose: msg("creating an image from your clean components")
            components_only=True
            modelimage=project+'.ccmodel'
            ia.fromshape(modelimage,[imsize[0],imsize[1],1,nchan])
            cs=ia.coordsys()
            epoch,ra,dec=util.direction_splitter(direction)
            cs.setunits(['rad','rad','','Hz'])
            cell_rad=qa.convert(qa.quantity(incell),"rad")['value']
            cs.setincrement([-cell_rad,cell_rad],'direction')
            cs.setreferencevalue([qa.convert(ra,'rad')['value'],qa.convert(dec,'rad')['value']],'direction')
            cs.setreferencevalue(startfreq,'spectral')
            ia.setcoordsys(cs.torecord())
            cl.open(complist)
            ia.setbrightnessunit("Jy/pixel")
            ia.modify(cl.torecord(),subtract=False)
            cl.done()
            ia.done() # to make sure its saved to disk at the start
        else:
            components_only=False
            if (complist != ''):
                msg("WARNING: I can't use clean components AND an image yet - using image",color=31)
        
            



        # open original model image
        modelia=ia.newimagefromfile(modelimage)
        
        # grab model image size and coordsys:
        insize=modelia.shape()
        incsys=modelia.coordsys()




        # for people who have put in an array, we should print an error.
        if type(incell) == 'list':
            incell=incell[0]
        # optionally use image header for cell size
        if incell=='header':
            increments=incsys.increment(type="direction")['numeric']
            incellx=abs(increments[0])
            incelly=increments[1]
            # warn if incells are not square
            if (abs(incellx)-abs(incelly))/abs(incellx) > 0.001:
                msg("input pixels are not square!",color="31")
                msg("using incell = incellx = %s (incelly=%s)" % (incellx,incelly),color="31")
            incellx=qa.quantity(incellx,'rad')
            incelly=qa.quantity(incelly,'rad')
            incell=qa.convert(incellx,'arcsec')
        else:
            incellx=incell
            incelly=incell

        # now that incell is set by the user or has been retrieved from the 
        # image header, we can use it for the output cell also
        if cell=="incell":
            cell=incell


        if verbose: msg("imsize= %s" % imsize)




        ##################################################################
        # read antenna file:

        stnx, stny, stnz, stnd, nant, telescopename = util.readantenna(antennalist)
        antnames=[]
        for k in range(0,nant): antnames.append('A%02d'%k)
        aveant=stnd.mean()




        ##################################################################
        # check inputs - need to add atmospheric window, better display of
        # where the actual observation block lies on the ephemeris window
        
        if checkinputs=="yes" or checkinputs=="only":
            currfignum=0
            pl.figure(currfignum)
            pl.ion()
            pl.clf()
            pl.subplot(121)
            model_min,model_max, model_rms = statim(modelimage,plot=display,incell=incell)
            lims=pl.xlim(),pl.ylim()
            tt=pl.array(range(25))*pl.pi/12
            pb=1.2*0.3/qa.convert(qa.quantity(startfreq),'GHz')['value']/aveant*3600.*180/pl.pi
            if max(max(lims)) > pb/2:
                plotcolor='w'
            else:
                plotcolor='k'
            for i in range(off.shape[1]):
                pl.plot(pl.cos(tt)*pb/2+off[0,i]*3600,pl.sin(tt)*pb/2+off[1,i]*3600,plotcolor)
            xlim=max(abs(pl.array(lims[0])))
            ylim=max(abs(pl.array(lims[1])))
            # show entire pb: (statim doesn't by default)
            pl.xlim([max([xlim,pb/2]),min([-xlim,-pb/2])])
            pl.ylim([min([-ylim,-pb/2]),max([ylim,pb/2])])
            pl.text(0,max([ylim,pb/2])*1.2,"regridded model:",horizontalalignment='center')
            # ephemeris:
            pl.subplot(222)
            util.ephemeris(refdate)  # util already knows the direction from above
            
            pl.subplot(224)
            pl.plot(range(2),'o')
            ax=pl.gca()
            l=ax.get_xticklabels()
            pl.setp(l,fontsize="x-small")
            l=ax.get_yticklabels()
            pl.setp(l,fontsize="x-small")
            pl.xlabel("coming soon",fontsize="x-small")
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.08,top=0.96,hspace=0.2,wspace=0.2)
            
            if checkinputs=="only":
                return
            else:
                if display==True:
                    pl.figure(currfignum+1)
                    pl.clf()
        else:
            model_min,model_max, model_rms = statim(modelimage,plot=False,incell=incell)





        ##################################################################
        # set up observatory, feeds, etc:

        sm.open(msfile)
        posobs=me.observatory(telescopename)
        diam=stnd;
        sm.setconfig(telescopename=telescopename, x=stnx, y=stny, z=stnz, 
                 dishdiameter=diam.tolist(), 
                 mount=['alt-az'], antname=antnames,
                 coordsystem='global', referencelocation=posobs)
        sm.setspwindow(spwname=fband, freq=startfreq, deltafreq=chanwidth, 
                   freqresolution=chanwidth, nchannels=nchan, 
                   stokes='XX YY')
        sm.setfeed(mode='perfect X Y',pol=[''])
        sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
        sm.setauto(0.0)
        for k in range(0,nfld):
            src=project+'_%d'%k
            sm.setfield(sourcename=src, sourcedirection=pointings[k],
                    calcode="C", distance='0m')
        reftime = me.epoch('TAI', refdate)
        sm.settimes(integrationtime=integration, usehourangle=True, 
                referencetime=reftime)
        for k in range(0,nfld) :
            src=project+'_%d'%k
            sttime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(totaltime),'s')['value']/nfld*k
            endtime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(totaltime),'s')['value']/nfld*(k+1)
            # this only creates blank uv entries
            sm.observe(sourcename=src, spwname=fband,
                   starttime=qa.quantity(sttime, "s"),
                   stoptime=qa.quantity(endtime, "s"));
        sm.setdata(fieldid=range(0,nfld))
        sm.setvp()

        # set imcenter to the center of the mosaic;  this is used in clean
        # as the phase center, which could be nonideal if there is no
        # pointing exactly in the middle of the mosaic.
        if type(direction) == list:
            imcenter , discard = util.average_direction(direction)
            if verbose: 
                msg("using pointings: ")
                for dir in direction:
                    msg("   "+dir)
            else:
                msg("using phasecenter " + imcenter)
        else:
            imcenter = direction

        msg("done setting up observations")







        ##################################################################
        # fit modelimage into a 4 coordinate image defined by the parameters

        if (modelimage != ''):

            # deal with brightness scaling since we may close modelia below.
            if (inbright=="unchanged") or (inbright=="default"):
                scalefactor=1.
            else:
                stats=modelia.statistics()
                highvalue=stats['max']
                scalefactor=float(inbright)/highvalue.max()
                
                
            # truncate model image name to craete new images in current dir:
            (modelimage_path,modelimage_local)=os.path.split(os.path.normpath(modelimage))
            modelimage_local=modelimage_local.strip()
            if modelimage_local.endswith(".fits"): modelimage_local=modelimage_local.replace(".fits","")
            if modelimage_local.endswith(".FITS"): modelimage_local=modelimage_local.replace(".FITS","")
            if modelimage_local.endswith(".fit"): modelimage_local=modelimage_local.replace(".fit","")
                
            # create a local copy of the model image to modify.
            scaledmodelfile=project+"."+modelimage_local+'.rescale'
                
            # make it look like our model came from the desired telescope
            # (else regrid complains)
            incsys.settelescope(telescopename)

            # does input model have stokes data?
            instokes=incsys.findcoordinate("stokes")['return']
            if not instokes:
                # assume that we haven't created scaledmodelfile yet
                modelia.adddegaxes(stokes="I",outfile=scaledmodelfile,overwrite=True)
                modelia.close()
                modelia.open(scaledmodelfile)
                incsys.addcoordinate(stokes="I")

            # does input model have spectral data?
            inspectral=incsys.findcoordinate("spectral")['return']
            if not inspectral:
                if nchan>1:
                    msg("WARNING: you're creating a cube from a flat image")
                if modelia.name(strippath=True)==scaledmodelfile:
                    # already added an axis and created the file so
                    scaledmodelfile=scaledmodelfile+"2"
                modelia.adddegaxes(spectral=True,outfile=scaledmodelfile,overwrite=True)
                modelia.close()
                modelia.open(scaledmodelfile)
                incsys.addcoordinate(spectral=True)
                incsys.setreferencevalue(value=startfreq,type="spectral")
                incsys.setincrement(value=bandwidth,type="spectral")

            inspax=incsys.findcoordinate("spectral")['pixel']
            modelia.setcoordsys(csys=incsys.torecord())

            # make a blank 4-dimensional image in the imager tool, with the desired out shape:
            # this sets up the image for processing inside of the simulated ms,
            # and also gives us a template coordinate system
            modelimage4d=project+"."+modelimage_local+'.coord'        
            im.open(msfile)    
            im.selectvis(field=range(nfld), spw=0)

            # imcenter is the average of the desired pointings
            # we put the phase center there, rather than at the
            # reference direction for the input model (refdirection)

            # assumes first two axes are spatial.
            # that assumption is made in CoordinateUtil and ImagerInterface also.            
            if inspectral:
                if insize[inspax] > 1:
                    if nchan<=1:
                        msg("WARNING: your model image is a cube but you're creating a flat image")
                # we should actually set the spectral start/step to the input, and then
                # regrid the spectral axis after we putchunk below.
                im.defineimage(nx=insize[0],ny=insize[1], cellx=incellx,celly=incelly,
                               phasecenter=imcenter, nchan=insize[inspax], 
                               mode="FREQ-LSRK",start=startfreq,step=chanwidth)
            else:
                im.defineimage(nx=insize[0],ny=insize[1], cellx=incellx,celly=incelly,
                               phasecenter=imcenter)

            im.make(modelimage4d)
            im.close()
            ia.open(modelimage4d)
            modelcsys=ia.coordsys()
            modelsize=ia.shape()

            # we now have modelia 4d and ia 4d, but the axis order may be different and
            # the pointing center and cell size for the modelia may be wrong.
            # the cell size should be right for ia, but the crval may be wrong.
            # actually, if we're just going to pullchunk from modelia below, and not
            # use regrid, since regrid doesn't reorder axes, we don't need to be
            # setting the crval and pixel sizes in incsys and modelia, but it doesn't
            # hurt, and helps with debugging and diagnostics.

            # now modify the input model image (modelia) and the projected modelimage4d (ia)
            if refdirection!=None:
                if refdirection=="header":
                    incoord=incsys.referencevalue(type="direction")['numeric']
                    ra=incoord[0]
                    dec=incoord[1]
                else:
                    if refdirection=="direction":
                        epoch, ra, dec = util.direction_splitter(imcenter)
                    else:
                        epoch, ra, dec = util.direction_splitter(refdirection)
                if verbose: msg("   setting model image direction to ra="+qa.angle(ra)+" dec="+qa.angle(dec))
                modelcsys.setreferencevalue(type="direction",value=[qa.convert(ra,'rad')['value'],qa.convert(dec,'rad')['value']])
                incsys.setreferencevalue(type="direction",value=[qa.convert(ra,'rad')['value'],qa.convert(dec,'rad')['value']])

            # this line assumes you want to do a frequency conversion - not in general
            #modelcs.setreferencevalue(
            #    qa.convert(startfreq,modelcs.units(type="spectral"))['value'],
            #    type="spectral")

            if refpixel!=None:
                if refpixel=="header":
                    crpix=incsys.referencepixel(type="direction")['numeric']
                else:
                    if refpixel=="center":
                        crpix=[insize[0]/2,insize[1]/2]
                    else:
                        # XXX this is pretty fragile code right now
                        refpixel=refpixel.replace('[','')
                        refpixel=refpixel.replace(']','')
                        crpix=pl.refpixel.split(',')
                if verbose: msg("   setting model image ref pixel to %f,%f" % (crpix[0],crpix[1]))
                # this may may not have the desired result, so just leave at center:
                # modelcsys.setreferencepixel(type="direction",value=crpix)
                incsys.setreferencepixel(type="direction",value=crpix)
            
            modelia.setcoordsys(incsys.torecord())
            ia.setcoordsys(modelcsys.torecord())

            # now the centers, pixel sizes, etc are correct in the modelia and ia,
            # and both are 4d, but the axis orders may still be different.

            # first assure that the ia image created by Imager has the expected
            # order - Kumar has enough hardcoded that it is unlikely to be otherwise.
            expected=['Direction', 'Direction', 'Stokes', 'Spectral']
            if modelcsys.axiscoordinatetypes() != expected:
                msg("internal error with coordinate axis order created by Imager",color=31)
                msg(modelcsys.axiscoordinatetypes().__str__(),color=31)
                return -1

            actual=incsys.axiscoordinatetypes()
            axmap=[0,1,2,3]
            if (actual[0] != 'Direction') or (actual[1] != 'Direction'):
                msg("WARNING: trying to transpose your input data, may not have desired effects regarding longitude and latitude order in the image",color=31)
            axmap[0]=actual.index('Direction')
            axmap[1]=actual.index('Direction',axmap[0]+1)
            axmap[2]=actual.index('Stokes')
            axmap[3]=actual.index('Spectral')

            # more checks:
            foo=pl.array(modelsize)
            if modelia.shape() != foo.take(axmap).tolist():
                msg("internal error: I'm confused about the shape if your model data cube",color=31)
                msg("have "+foo.take(axmap).__str__()+", want "+modelia.shape().__str__(),color=31)
                return -1

            if verbose: msg("rearranging input data (may take some time for large cubes)")
            arr=modelia.getchunk()
            for ax in range(4):
                if axmap[ax] != ax:
                    if arr.shape[ax] > 1 and arr.shape[axmap[ax]] > 1:
                        arr.swapaxes(ax,axmap[ax])                        
                        tmp=axmap[ax]
                        axmap[ax]=ax
                        axmap[tmp]=tmp
                    else:
                        arr=arr.reshape(ia.shape())
                        axmap=range(4) # assumes that (checked above) ia has canonical order

            # there's got to be a better way: :)
            for i0 in range(arr.shape[0]):
                for i1 in range(arr.shape[1]):
                    for i2 in range(arr.shape[2]):
                        for i3 in range(arr.shape[3]):
                            foo=arr[i0,i1,i2,i3]
                            if foo!=foo: arr[i0,i1,i2,i3]=0.0

            if verbose: msg("model array minmax= %f %f" % (arr.min(),arr.max()))
            ia.putchunk(arr*scalefactor)                    
            ia.close()

            # modelia.regrid(outfile=modelimage4d,csys=rec,overwrite=True)
            # modelia.close()
            # now open spatially rescaled output image
            # modelia.open(modelimage4d)            
            # modelia.putchunk(modelia.getchunk()*scalefactor)

            modelia.close()

            # coord image should now have correct Coordsys and shape


            
        ##################################################################
        # do actual calculation of visibilities from the model image:

        if not components_only:
            # right now this is ok - if we only had components,
            # we have created modelimage4d from them but if
            # we had components and model image they are not yet combined
            if verbose:
                msg("predicting from "+modelimage4d)
                if arr.nbytes > 5e7:
                    msg(" your model is large - this may take a while")
            sm.predict(imagename=[modelimage4d],complist=complist)
            #sm.predict(imagename=modelimage4d)
        else:   # if we're doing only components
            if verbose: msg("predicting from "+complist)
            sm.predict(complist=complist)

        sm.done()
        
        msg('generation of measurement set ' + msfile + ' complete.')






        ######################################################################
        # noisify

        noise_any=False
    
        if noise_thermal:
            if not (util.telescopename == 'ALMA' or util.telescopename == 'ACA'):
                msg("WARNING: thermal noise only works properly for ALMA/ACA",color=31)
                
            noise_any=True

            noisymsfile = project + ".noisy.ms"
            msg('adding thermal noise to ' + noisymsfile)

            eta_p, eta_s, eta_b, eta_t, eta_q, t_rx = util.noisetemp()

            # antenna efficiency
            eta_a = eta_p * eta_s * eta_b * eta_t
            if verbose: msg('antenna efficiency = ' + str(eta_a))
 
            # Ambient surface radiation temperature in K. 
            # FOR NOW, T_atm = T_ground = T_amb = parameter, also tau0=parameter
            t_ground = t_amb
            # Atmospheric radiation temperature in K. 
            t_atm = t_amb

            # Cosmic background radiation temperature in K. 
            t_cmb = 2.275

            if os.path.exists(noisymsfile):
                cu.removetable(noisymsfile)
            os.system("cp -r "+msfile+" "+noisymsfile)
            # XXX check for and copy flagversions file as well
            
            sm.openfromms(noisymsfile);    # an existing MS
            sm.setdata();                # currently defaults to fld=0,spw=0
            # type = B BPOLY G GSPLINE D P T TOPAC GAINCURVE
            sm.setapply(type='TOPAC',opacity=tau0);  # arrange opac corruption
            # NewMSSimulator needs 2-temp formula not just t_atm
            sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                        antefficiency=eta_a,trx=t_rx,
                        tau=tau0,tatmos=t_atm,tcmb=t_cmb,
                        mode="calculate")
            sm.corrupt();
            sm.done();

            if verbose: msg("done corrupting with thermal noise")

            
        # not yet implemented:
        #        noise_phase=True
        #
        #        if noise_phase:
        #            # phase noise
        #            # noise_any=True
        #            noisycalfile = project + ".atmos.cal"
        #            casalog.post('adding phase noise to ' + noisycalfile)
        #            gaincal_defaults()
        #            # make cal file to be modified
        #            gaincal(vis=msfile,caltable=noisycalfile,solint=-1)
            






        #####################################################################
        # clean if desired, WILL use noisy image for further calculation if present!
        # do we want to invert the noiseless one anyway for comparison?

        if noise_any:
            mstoimage = noisymsfile
        else:
            mstoimage = msfile

        if fidelity == True and psfmode == "none":
            msg("You can't calculate fidelity without imaging, so change psfmode if you want a fidelity image calculated.")
            fidelity=False

        if display == True and psfmode == "none":
            msg("Without creating an image, there's very little to display, so I'm turning off display.  Change psfmode if you want to make an image.")
            display=False

        if psfmode != "none": 
            if niter == 0:
                image=project+'.dirty'
                msg("inverting to "+image)
            else:
                image=project+'.clean'
                msg("cleaning to "+image)            
        else:
            image=project+'.clean'

        if uvtaper == False:
            outertaper=[]
            innertaper=[]

        #if verbose:
        # print clean inputs no matter what, so user can use them.
        msg("clean inputs:")
        msg("clean(vis='"+mstoimage+"',imagename='"+image+"',field='',spw='',selectdata=False,timerange='',uvrange='',antenna='',scan='',mode='channel',niter="+str(niter)+",gain=0.1,threshold="+threshold+",psfmode='"+psfmode+"',imagermode='"+imagermode+"',ftmachine='mosaic',mosweight=False,scaletype='SAULT',multiscale=[],negcomponent=-1,interactive=False,mask=[],nchan="+str(nchan)+",start=0,width=1,imsize="+str(imsize)+",cell='"+str(cell)+"',phasecenter='"+str(imcenter)+"',restfreq='',stokes='"+stokes+"',weighting='"+weighting+"',robust="+str(robust)+",uvtaper="+str(uvtaper)+",outertaper="+str(outertaper)+",innertaper="+str(innertaper)+",modelimage='',restoringbeam=[''],pbcor=False,minpb=0.1,noise="+str(noise)+",npixels="+str(npixels)+",npercycle=100,cyclefactor=1.5,cyclespeedup=-1)")

        if psfmode != "none": 
            if int(casalog.version().split()[2].split('.')[1]) <= 3:
                # XXX 2.3.1 syntax:
                clean(vis=mstoimage,imagename=image,field='',spw='',selectdata=False,timerange='',uvrange='',antenna='',scan='',mode="channel",niter=niter,gain=0.1,threshold=threshold,psfmode=psfmode,imagermode=imagermode,ftmachine="mosaic",mosweight=False,scaletype="SAULT",multiscale=[],negcomponent=-1,interactive=False,mask=[],nchan=nchan,start=0,width=1,imsize=imsize,cell=cell,phasecenter=imcenter,restfreq='',stokes=stokes,weighting=weighting,robust=robust,uvtaper=uvtaper,outertaper=outertaper,innertaper=innertaper,modelimage='',restoringbeam=[''],pbcor=False,minpb=0.1,noise=noise,npixels=npixels,npercycle=100,cyclefactor=1.5,cyclespeedup=-1)
                # XXX 2.4.0 syntax:
            else:
                clean(vis=mstoimage, imagename=image, field='', spw='', selectdata=False,
                      timerange='', uvrange='', antenna='', scan='', mode="channel",
                      interpolation='nearest', niter=niter, gain=0.1, threshold=threshold,
                      psfmode=psfmode, imagermode=imagermode, ftmachine="mosaic",
                      mosweight=False, scaletype="SAULT", multiscale=[],
                      negcomponent=-1, interactive=False, mask=[], nchan=nchan,
                      start=0, width=1, imsize=imsize, cell=cell, phasecenter=imcenter,
                      restfreq='', stokes=stokes, weighting=weighting, robust=robust,
                      uvtaper=uvtaper, outertaper=outertaper, innertaper=innertaper,
                      modelimage='', restoringbeam=[''], pbcor=False, minpb=0.1,
                      noise=noise, npixels=npixels, npercycle=100,
                      cyclefactor=1.5, cyclespeedup=-1)
        else:
            image=project




        # need this filename whether or not we create the output image
        modelregrid=project+"."+modelimage_local+".flat2"



        #####################################################################
        # prepare for diff and fidelity and display by making a moment 0 image

        if fidelity == True or display == True:

            inspectax=modelcsys.findcoordinate('spectral')['pixel']
            innchan=modelsize[inspectax]
            # modelimage4d is the .coord version w/spectral axis.
            
            # modelflat should be the moment zero of that
            modelflat=project+"."+modelimage_local+".flat" 
            if innchan>1:
                if verbose: msg("creating moment zero input image")
                # actually run ia.moments
                ia.open(modelimage4d)
                ia.moments(moments=[-1],outfile=modelflat,overwrite=True)
                ia.done()
            else:
                # just remove degenerate axes from modelimage4d
                ia.newimagefromimage(infile=modelimage4d,outfile=modelflat,dropdeg=True,overwrite=True)
                # why can't I just remove spectral and leave stokes?
                os.system("mv "+modelflat+" "+modelflat+".tmp")
                ia.open(modelflat+".tmp")
                ia.adddegaxes(outfile=modelflat,stokes='I',overwrite=True)
                ia.done()
                os.system("rm -rf "+modelflat+".tmp")


            # flat output
            outflat=image+".flat"
            if nchan>1:
                # image is either .clean or .dirty
                if verbose: msg("creating moment zero output image")
                ia.open(image+".image")
                ia.moments(moments=[-1],outfile=outflat,overwrite=True)
                ia.done()
            else:
                if verbose: msg("flattening output image to "+outflat)
                # just remove degenerate axes from image
                ia.newimagefromimage(infile=image+".image",outfile=outflat,dropdeg=True,overwrite=True)
                # seems to not be a way to just drop the spectral and keep the stokes.  sigh.
                os.system("mv "+outflat+" "+outflat+".tmp")
                ia.open(outflat+".tmp")
                ia.adddegaxes(outfile=outflat,stokes='I',overwrite=True)
                ia.done()
                os.system("rm -rf "+outflat+".tmp")
            
            # be sure to get outflatcoordsys from outflat
            ia.open(outflat)
            outflatcoordsys=ia.coordsys()
            outflatshape=ia.shape()
            ia.done()            

            # regrid flat input to flat output shape, for convolution, etc
            modelia.open(modelflat)
            imrr = modelia.regrid(outfile=modelregrid, overwrite=True,
                                  csys=outflatcoordsys.torecord(),shape=outflatshape)
            imrr.done()

            # XXX when we get to using both clean components and model image,
            # we should add them together here if not before.
            # modelia.immath

            modelia.done()
            outflatcoordsys.done()
            del outflatcoordsys
            del imrr

            if innchan==1:
                os.system("rm -rf "+modelflat)  # no need to keep this


        if psfmode != "none":
            msg("done inverting and cleaning")








        #####################################################################
        # fidelity  

        if os.path.exists(modelimage) and (fidelity == True or display == True):
            # get beam from output clean image
            if verbose: msg("getting beam from "+image+".image")
            ia.open(image+".image")
            beam=ia.restoringbeam()
            ia.done()

        if os.path.exists(modelimage) and fidelity == True: 
            # from math import sqrt
            
            # Convolve model with beam.
            convolved = project + '.convolved.im'
            modelia.open(modelregrid)
            modelia.convolve2d(convolved,major=beam['major'],minor=beam['minor'],
                          pa=beam['positionangle'],overwrite=True)

            # Make difference image.
            difference = project + '.diff.im'
            modelia.imagecalc(difference, "'%s' - '%s'" % (convolved, outflat), overwrite = True)
            modelia.done()
            # Get rms of difference image.
            ia.open(difference)
            diffstats = ia.statistics(robust = True)
            ia.done()
            maxdiff=diffstats['medabsdevmed']
            if maxdiff!=maxdiff: maxdiff=0.
            # Make fidelity image.
            absdiff = project + '.absdiff.im'
            ia.imagecalc(absdiff, "max(abs('%s'), %f)" % (difference,
                                                          maxdiff/pl.sqrt(2.0)), overwrite = True)
            fidelityim = project + '.fidelity.im'
            ia.imagecalc(fidelityim, "abs('%s') / '%s'" % (convolved, absdiff), overwrite = True)
            ia.done()

            msg("fidelity image calculated")

            # clean up moment zero maps if we don't need them anymore
            if nchan==1:
                os.system("rm -rf "+outflat) 







        #####################################################################
        # display and statistics:

        # plot whichever image we have - preference is for noisy and cleaned
        if display==True:
            pl.ion()
            pl.clf()
            if fidelity == True:
                pl.subplot(232)
            else:
                pl.subplot(222)
        if psfmode != "none":
            sim_min,sim_max,sim_rms = statim(image+".image",plot=display)

        # plot model image if exists
        if os.path.exists(modelimage) and display==True:
            if fidelity == True:
                pl.subplot(231)
            else:
                pl.subplot(221)
            # modelregrid might not exist if there's no display or fidelity
            if os.path.exists(modelregrid):
                model_min,model_max, model_rms = statim(modelregrid,plot=display)
                xlim=max(pl.xlim())
                ylim=max(pl.ylim())
            if display==True:
                tt=pl.array(range(25))*pl.pi/12
                pb=1.2*0.3/qa.convert(qa.quantity(startfreq),'GHz')['value']/aveant*3600.*180/pl.pi
                # XXX change this to use tb.open(ms/POINTINGS)/direction
                # and them make it a helper function
                for i in range(off.shape[1]):
                    pl.plot(pl.cos(tt)*pb/2+off[0,i]*3600,pl.sin(tt)*pb/2+off[1,i]*3600,'w')
                pl.xlim([xlim,-xlim])
                pl.ylim([-ylim,ylim])

            
            # fidelity image would only exist if there's a model image
            if modelimage != '' and fidelity == True:
                if display: pl.subplot(233)
                statim(project+".diff.im",plot=display)
                if display: pl.subplot(234)
                fidel_min, fidel_max, fidel_rms = statim(project+".fidelity.im",plot=display)

        if display:
            tb.open(mstoimage)
            # XXX use rob's FFT of the PB instead
            rawdata=tb.getcol("UVW")
            tb.done()
            if fidelity == True:
                pl.subplot(235)
            else:
                pl.subplot(223)
            pl.box()
            maxbase=max([max(rawdata[0,]),max(rawdata[1,])])  # in m
            klam_m=300/qa.convert(qa.quantity(startfreq),'GHz')['value']
            pl.plot(rawdata[0,]/klam_m,rawdata[1,]/klam_m,'b,')
            pl.plot(-rawdata[0,]/klam_m,-rawdata[1,]/klam_m,'b,')
            ax=pl.gca()
            ax.yaxis.LABELPAD=-4
            l=ax.get_xticklabels()
            pl.setp(l,fontsize="x-small")
            l=ax.get_yticklabels()
            pl.setp(l,fontsize="x-small")
            pl.xlabel('u[klambda]',fontsize='x-small')
            pl.ylabel('v[klambda]',fontsize='x-small')
            pl.axis('equal')

            if modelimage != '' and psfmode != "none":   
                if fidelity == True:
                    pl.subplot(236)
                else:
                    pl.subplot(224)
                ia.open(image+".psf")
                beamcs=ia.coordsys()
                beam_array=ia.getchunk(axes=[beamcs.findcoordinate("spectral")['pixel'],beamcs.findcoordinate("stokes")['pixel']],dropdeg=True)
                pixsize=(qa.convert(qa.quantity(cell),'arcsec')['value'])
                xextent=imsize[0]*pixsize*0.5
                xextent=[xextent,-xextent]
                yextent=imsize[1]*pixsize*0.5
                yextent=[-yextent,yextent]
                flipped_array=beam_array.transpose()
                ttrans_array=flipped_array.tolist()
                ttrans_array.reverse()
                pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,origin="bottom")
                pl.title(image+".psf",fontsize="x-small")
                b=qa.convert(beam['major'],'arcsec')['value']
                pl.xlim([-3*b,3*b])
                pl.ylim([-3*b,3*b])
                ax=pl.gca()
                l=ax.get_xticklabels()
                pl.setp(l,fontsize="x-small")
                l=ax.get_yticklabels()
                pl.setp(l,fontsize="x-small")
                from matplotlib.font_manager import fontManager, FontProperties
                font= FontProperties(size='x-small');
                pl.legend(("bmaj=%7.1e" % beam['major']['value'],"bmin=%7.1e" % beam['minor']['value']),prop=font)                
                ia.done()

                
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.08,top=0.96,hspace=0.1,wspace=0.2)
        
        # if not displaying still print stats:
        if psfmode != "none":
            msg('Simulation rms: '+str(sim_rms))
            msg('Simulation max: '+str(sim_max))
        
        if display == True or fidelity == True:
            msg('Model rms: '+str(model_rms))
            msg('Model max: '+str(model_max))
            msg('Beam bmaj: '+str(beam['major']['value'])+' bmin: '+str(beam['minor']['value'])+' bpa: '+str(beam['positionangle']['value']))






    except TypeError, e:
        msg("task_simdata -- TypeError: %s" % e,color="red")
        return
    except ValueError, e:
        print "task_simdata -- OptionError: ", e
        return
    except Exception, instance:
        print '***Error***',instance
        return
