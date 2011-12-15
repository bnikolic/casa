##########################################################################
# imfit_test.py
#
# Copyright (C) 2008, 2009
# Associated Universities, Inc. Washington DC, USA.
#
# This script is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be adressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# <author>
# Dave Mehringer
# </author>
#
# <summary>
# Test suite for the CASA task specfit and tool method ia.fitprofile
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_specfit.py:description">imcollapse</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the specfit task and ia.fitprofile() tool method.
# </etymology>
#
# <synopsis>
# Test the specfit task and the ia.fitprofile() method upon which it is built.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_specfit[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the specfit task to ensure
# coding changes do not break the associated bits 
# </motivation>
#

###########################################################################
import shutil
import casac
from tasks import *
from taskinit import *
from __main__ import *
import unittest
import numpy

twogauss = "specfit_multipix_2gauss.fits"
polyim = "specfit_multipix_poly_2gauss.fits"
solims = [
    "amp_0", "ampErr_0", "amp_1", "ampErr_1",
    "center_0", "centerErr_0", "center_1", "centerErr_1",
    "fwhm_0", "fwhmErr_0", "fwhm_1", "fwhmErr_1"
]

nanvalue = 4.53345345

datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/specfit/'

def run_fitprofile (
    imagename, box, region, chans, stokes,
    axis, mask, ngauss, poly, multifit, model,
    residual, amp="", amperr="", center="", centererr="",
    fwhm="", fwhmerr="", estimates=""
):
    myia = iatool.create()
    myia.open(imagename)
    if (not myia.isopen()):
        myia.done()
        raise Exception
    res = myia.fitprofile(
        box=box, region=region, chans=chans,
        stokes=stokes, axis=axis, mask=mask,
        ngauss=ngauss, poly=poly, estimates=estimates,
        multifit=multifit,
        model=model, residual=residual, amp=amp,
        amperr=amperr, center=center, centererr=centererr,
        fwhm=fwhm, fwhmerr=fwhmerr
    )
    myia.close()
    myia.done()
    return res

def run_specfit(
    imagename, box, region, chans, stokes,
    axis, mask, ngauss, poly, multifit, model,
    residual, amp="", amperr="", center="", centererr="",
    fwhm="", fwhmerr="", wantreturn=True, estimates=""
):
    return specfit(
        imagename=imagename, box=box, region=region,
        chans=chans, stokes=stokes, axis=axis, mask=mask,
        ngauss=ngauss, poly=poly, estimates=estimates,
        multifit=multifit,
        model=model, residual=residual, amp=amp,
        amperr=amperr, center=center, centererr=centererr,
        fwhm=fwhm, fwhmerr=fwhmerr,
        wantreturn=wantreturn
    )

class specfit_test(unittest.TestCase):
    
    def setUp(self):
        shutil.copy(datapath + twogauss, twogauss)
        shutil.copy(datapath + polyim, polyim)

    def tearDown(self):
        os.remove(twogauss)
        os.remove(polyim)

    def checkImage(self, gotImage, expectedName):
        print "*** expected name " + expectedName
        expected = iatool.create()                                
        expected.open(expectedName)
        got = iatool.create()
        if type(gotImage) == str:
            got.open(gotImage)
        else:
            got = gotImage
        self.assertTrue(got.shape() == expected.shape())
        gotchunk = got.getchunk()
        expchunk = expected.getchunk()
        if (numpy.isnan(gotchunk).any()):
            gotchunk = gotchunk.ravel()
            for i in range(len(gotchunk)):
                if isnan(gotchunk[i]):
                    gotchunk[i] = nanvalue
        if (numpy.isnan(expchunk).any()):
            expchunk = expchunk.ravel()
            for i in range(len(expchunk)):
                if isnan(expchunk[i]):
                    expchunk[i] = nanvalue
        diffData = gotchunk - expchunk
        self.assertTrue(abs(diffData).max() < 2e-11)
        self.assertTrue(
            (
                got.getchunk(getmask=T) == expected.getchunk(getmask=T)
            ).all()
        )
        gotCsys = got.coordsys()
        expectedCsys = expected.coordsys()
        diffPixels = gotCsys.referencepixel()['numeric'] - expectedCsys.referencepixel()['numeric']
        self.assertTrue(abs(diffPixels).max() == 0)
        
        fracDiffRef = (
            gotCsys.referencevalue()['numeric'] - expectedCsys.referencevalue()['numeric']
        )/expectedCsys.referencevalue()['numeric'];
        self.assertTrue(abs(fracDiffRef).max() <= 3e-13)
        got.close()
        got.done()
        expected.close()
        expected.done()

    def test_exceptions(self):
        """specfit: Test various exception cases"""
        
        def testit(
            imagename, box, region, chans, stokes,
            axis, mask, ngauss, poly, multifit, model,
            residual
        ):
            for i in [0,1]:
                if (i==0):
                    self.assertRaises(
                        Exception, run_fitprofile, imagename,
                        box, region, chans, stokes, axis, mask,
                        ngauss, poly, multifit, model, residual
                    )
                else:
                    self.assertFalse(
                        run_specfit(
                            imagename, box, region, chans,
                            stokes, axis, mask, ngauss, poly,
                            multifit, model, residual
                        )
                    )
        # Exception if no image name given",
        testit(
            "", "", "", "", "", 2, "", False, 1, -1, "", ""
        )
        # Exception if bogus image name given
        testit(
            "my bad", "", "", "", "", 2, "", 1, -1, False, "", ""
        )
        # Exception if given axis is out of range
        testit(
            twogauss, "", "", "", "", 5, "", 1, -1, False, "", ""
        )
        # Exception if bogus box string given #1
        testit(
            twogauss, "abc", "", "", "", 2, "", 1, -1, False, "", ""
        )
        # Exception if bogus box string given #2
        testit(
            twogauss, "0,0,1000,1000", "", "", "", 2, "", 1, -1, False, "", ""
        )
        # Exception if bogus chans string given #1
        testit(
            twogauss, "", "", "abc", "", 2, "", 1, -1, False, "", ""
        )
        # Exception if bogus chans string given #2
        testit(
            twogauss, "", "", "0-200", "", 2, "", 1, -1, False, "", ""
        )        
        # Exception if bogus stokes string given #1   
        testit(
            twogauss, "", "", "", "abc", 2, "", 1, -1, False, "", ""
        )       
        # Exception if bogus stokes string given #2 
        testit(
            twogauss, "", "", "", "v", 2, "", 1, -1, False, "", ""
        )       
        # Exception if no gaussians and no polynomial specified
        testit(
            twogauss, "", "", "", "", 2, "", 0, -1, False, "", ""
        )         
        
    def test_1(self):
        """Tests of averaging over a region and then fitting"""
        imagename = twogauss
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        ngauss = 2
        poly = -1
        multifit = False
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual
            )
            self.assertTrue(len(res["converged"]) == 1)
            self.assertTrue(res["converged"][0,0,0,0])
            # even though two components given, only one is fit
            self.assertTrue(res["ncomps"][0,0,0,0] == 1)
            # the fit component is a gaussian
            self.assertTrue(res["type0"][0,0,0,0] == "GAUSSIAN")
            self.assertAlmostEqual(res["amp0"][0,0,0,0], 49.7, 1, "amplitude determination failure")
            self.assertAlmostEqual(res["ampErr0"][0,0,0,0], 4.0, 1, "amplitude error determination failure")
            self.assertAlmostEqual(res["center0"][0,0,0,0], -237.7, 1, "center determination failure")
            self.assertAlmostEqual(res["centerErr0"][0,0,0,0], 1.7, 1, "center error determination failure")
            self.assertAlmostEqual(res["fwhm0"][0,0,0,0], 42.4, 1, "fwhm determination failure")
            self.assertAlmostEqual(res["fwhmErr0"][0,0,0,0], 4.0, 1, "fwhm error determination failure")

            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")
 
    def test_2(self):
        """ multipixel, two gaussian fit"""
        imagename = twogauss
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        ngauss = 2
        poly = -1
        multifit = True
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual
            )
            self.assertTrue(len(res["converged"].ravel()) == 81)
            self.assertTrue(res["converged"].all())
            self.assertTrue(res["ncomps"][0, 0, 0, 0] == 1)
            self.assertTrue((res["ncomps"][:, 1:, 0, 0] == 2).all())
            self.assertTrue((res["ncomps"][1:, 0, 0, 0] == 2).all())
            self.assertTrue((res["type0"] == "GAUSSIAN").all())
            self.assertTrue(res["type1"][0, 0, 0, 0] == "UNDEF")
            self.assertTrue((res["type1"][:, 1:, 0, 0] == "GAUSSIAN").all())
            self.assertTrue((res["type1"][1:, 0, 0, 0] == "GAUSSIAN").all())

            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")
            
    def test_3(self):
        """ Test two gaussian + one polynomial image"""
        imagename = polyim
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        ngauss = 2
        poly = 3
        multifit = True
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual
            )
        self.assertTrue(len(res["converged"].ravel()) == 81)
        # fit #72 did not converge
        self.assertTrue(res["converged"][:, :7, 0, 0].all())
        self.assertTrue(res["converged"][1:,8,0,0].all())
        self.assertFalse(res["converged"][0, 8, 0, 0])

        self.assertTrue(res["xUnit"] == "km/s")
        self.assertTrue(res["yUnit"] == "Jy")

    def test_4(self):
        """writing solution images for multipixel, two gaussian fit"""
        imagename = twogauss
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        ngauss = 2
        poly = -1
        multifit = True
        model = ""
        residual = ""
        amp = "amp"
        amperr = "ampErr"
        center = "center"
        centererr = "centerErr"
        fwhm = "fwhm"
        fwhmerr = "fwhmErr"
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual, amp,
                amperr, center, centererr, fwhm, fwhmerr
            )
            for im in solims:
                self.checkImage(im, datapath + im)
                
        for im in solims:
            shutil.rmtree(im)
            
    def test_5(self):
        """test results of multi-pixel one gaussian fit with estimates file"""
        imagename = twogauss
        ngauss=10
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        poly = -1
        estimates = datapath + "goodProfileEstimatesFormat_2.txt"
        multifit = True
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual, 
                estimates=estimates
            )
        # no tests yet, just confirm it runs to completion

    def test_6(self):
        """test results of non-multi-pixel one gaussian fit with estimates file"""
        imagename = twogauss
        ngauss=10
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        poly = -1
        estimates = datapath + "goodProfileEstimatesFormat_2.txt"
        multifit = False
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual, 
                estimates=estimates
            )
            self.assertTrue(res['converged'] == 1)
            self.assertTrue(res['converged'][0][0][0][0])
            self.assertTrue(res['ncomps'][0][0][0][0] == 1)
            self.assertTrue(res['type0'][0][0][0][0] == "GAUSSIAN")
            self.assertTrue(res["type0"][0,0,0,0] == "GAUSSIAN")
            self.assertAlmostEqual(res["amp0"][0,0,0,0], 49.7, 1, "amplitude determination failure")
            self.assertAlmostEqual(res["ampErr0"][0,0,0,0], 4.0, 1, "amplitude error determination failure")
            self.assertAlmostEqual(res["center0"][0,0,0,0], -237.7, 1, "center determination failure")
            self.assertAlmostEqual(res["centerErr0"][0,0,0,0], 1.7, 1, "center error determination failure")
            self.assertAlmostEqual(res["fwhm0"][0,0,0,0], 42.4, 1, "fwhm determination failure")
            self.assertAlmostEqual(res["fwhmErr0"][0,0,0,0], 4.0, 1, "fwhm error determination failure")
            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")

    def test_7(self):
        """test results of non-multi-pixel one gaussian fit with estimates file keeping peak fixed"""
        imagename = twogauss
        ngauss=10
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        poly = -1
        estimates = datapath + "goodProfileEstimatesFormat_3.txt"
        multifit = False
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual, 
                estimates=estimates
            )
            self.assertTrue(res['converged'] == 1)
            self.assertTrue(res['converged'][0][0][0][0])
            self.assertTrue(res['ncomps'][0][0][0][0] == 1)
            self.assertTrue(res['type0'][0][0][0][0] == "GAUSSIAN")
            self.assertTrue(res["type0"][0,0,0,0] == "GAUSSIAN")
            self.assertAlmostEqual(res["amp0"][0,0,0,0], 45, 1, "amplitude determination failure")
            self.assertAlmostEqual(res["ampErr0"][0,0,0,0], 0, 1, "amplitude error determination failure")
            self.assertAlmostEqual(res["center0"][0,0,0,0], -237.7, 1, "center determination failure")
            self.assertAlmostEqual(res["centerErr0"][0,0,0,0], 1.9, 1, "center error determination failure")
            self.assertAlmostEqual(res["fwhm0"][0,0,0,0], 45.6, 1, "fwhm determination failure")
            self.assertAlmostEqual(res["fwhmErr0"][0,0,0,0], 3.8, 1, "fwhm error determination failure")
            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")

            
    def test_stretch(self):
        """specfit : test mask stretch"""
        imagename = twogauss
        yy = iatool.create()
        yy.open(imagename)
        mycsys = yy.coordsys().torecord()
        yy.done()
        mymask = "maskim"
        yy.fromshape(mymask, [9, 9, 1, 1])
        yy.setcoordsys(mycsys)
        yy.addnoise()
        yy.done()
        yy.open(imagename)
        self.assertRaises(
            Exception, yy.fitprofile,
            ngauss=2, mask=mymask + ">-100",
            stretch=False
        )
        zz = specfit(
            imagename, ngauss=2, mask=mymask + ">-100",
            stretch=False
        )
        self.assertTrue(zz == None)
        print "*** here 4"
        zz = yy.fitprofile(
            ngauss=2, mask=mymask + ">-100",
            stretch=True
        )
        self.assertTrue(len(zz.keys()) > 0)
        yy.done()
        zz = specfit(
            imagename, ngauss=2, mask=mymask + ">-100",
            stretch=True
        )
        self.assertTrue(len(zz.keys()) > 0)


def suite():
    return [specfit_test]
