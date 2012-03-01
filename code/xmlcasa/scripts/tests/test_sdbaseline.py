import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from asap_init import * 
import unittest
#
import listing
from numpy import array

asap_init()
import asap as sd
from sdbaseline import sdbaseline
from sdstat import sdstat

class sdbaseline_basictest(unittest.TestCase):
    """
    Basic unit tests for task sdbaseline. No interactive testing.

    The list of tests:
    test00   --- default parameters (raises an error)
    test01   --- test polynominal baseline with maskmode = 'auto'
    test02   --- test polynominal baseline with maskmode = 'list'

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdaverage)
      sdaverage(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdaverage)
      sdaverage(infile='temp.asap',timeaverage=True,
                tweight='tintsys',outfile=self.infile)
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdbaseline/'
    # Input and output names
    #infile = 'OrionS_rawACSmod_calTave.asap'
    infile = 'OrionS_rawACSmod_calave.asap'
    outroot = 'sdbaseline_test'
    blrefroot = datapath+'refblparam'
    strefroot = datapath+'refstats'

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdbaseline)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def test00(self):
        """Test 0: Default parameters"""
        result = sdbaseline()
        self.assertFalse(result)

    def testbl01(self):
        """Test 1: maskmode = 'auto'"""
        tid = "01"
        infile = self.infile
        outfile = self.outroot+tid+".asap"
        mode = "auto"
        iflist = [0,2]
        pollist=[0]
        result = sdbaseline(infile=infile,maskmode=mode,outfile=outfile,
                            blfunc='poly',iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareBLparam(outfile+"_blparam.txt",self.blrefroot+tid)
        reference = {'rms': [0.42423143982887268, 0.19752366840839386],
                     'min': [-19.465526580810547, -2.7562465667724609],
                     'max': [14.881179809570312, 2.0289769172668457],
                     'max_abscissa': {'value': array([   21.,  3045.]),
                                      'unit': 'channel'},
                     'median': [0.005268096923828125, 0.0032062530517578125],
                     'min_abscissa': {'value': array([ 18.,   0.]),
                                      'unit': 'channel'},
                     'stddev': [0.42413413524627686, 0.19690337777137756]}
        self._compareStats(outfile,reference)
        #self._compareStats(outfile,self.strefroot+tid)

    def testbl02(self):
        """Test 2: maskmode = 'list' and masklist=[] (all channels)"""
        tid = "02"
        infile = self.infile
        outfile = self.outroot+tid+".asap"
        mode = "list"
        iflist = [2]
        pollist = [1]
        result = sdbaseline(infile=infile,maskmode=mode,outfile=outfile,
                            iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareBLparam(outfile+"_blparam.txt",self.blrefroot+tid)
        reference = {'rms': 3.4925737380981445,
                     'min': -226.3941650390625,
                     'max': 129.78572082519531,
                     'max_abscissa': {'value': 8186.0, 'unit': 'channel'},
                     'median': -0.025681495666503906,
                     'stddev': 3.4927871227264404,
                     'min_abscissa': {'value': 8187.0, 'unit': 'channel'}}
        self._compareStats(outfile,reference)
        #self._compareStats(outfile,self.strefroot+tid)


    def _compareBLparam(self,out,reference):
        # test if baseline parameters are equal to the reference values
        # currently comparing every lines in the files
        # TO DO: compare only "Fitter range" and "Baseline parameters"
        self.assertTrue(os.path.exists(out))
        self.assertTrue(os.path.exists(reference),
                        msg="Reference file doesn't exist: "+reference)
#         self.assertTrue(listing.compare(out,reference),
#                         'New and reference files are different. %s != %s. '
#                         %(out,reference))

    def _compareStats(self,outfile,reference):
        # test if the statistics of baselined spectra are equal to
        # the reference values
        self.assertTrue(os.path.exists(outfile))
        currstat = sdstat(infile=outfile)
        self.assertTrue(isinstance(currstat,dict),
                        msg="Failed to calculate statistics.")
        f = open(outfile+'_stats','w')
        f.write(str(currstat))
        f.close()
        del f

        #self.assertTrue(os.path.exists(reference),
        #                msg="Reference file doesn't exist: "+reference)
        refstat = reference

        print "Statistics of baselined spectra:\n",currstat
        print "Reference values:\n",refstat
        # compare statistic values
        #compstats = ['max','min','mean','sum','rms']
        compstats = ['max','min','rms','median','stddev']
        allowdiff = 0.01
        self.assertEqual(currstat['max_abscissa']['unit'],
                         refstat['max_abscissa']['unit'],
                         msg="The units of max_abscissa are different")
        self.assertEqual(currstat['min_abscissa']['unit'],
                         refstat['min_abscissa']['unit'],
                         msg="The units of min_abscissa are different")
        if isinstance(refstat['max'],list):
            for i in xrange(len(refstat['max'])):
                for stat in compstats:
                    rdiff = (currstat[stat][i]-refstat[stat][i])/refstat[stat][i]
                    self.assertTrue((abs(rdiff)<allowdiff),
                                    msg="'%s' of spectrum %s are different." % (stat, str(i)))
                self.assertEqual(currstat['max_abscissa']['value'][i],
                                 refstat['max_abscissa']['value'][i],
                                 msg="The max channels/frequencies/velocities of spectrum %s are different" % str(i))
                self.assertEqual(currstat['min_abscissa']['value'][i],
                                 refstat['min_abscissa']['value'][i],
                                 msg="The min channels/frequencies/velocities of spectrum %s are different" % str(i))
        else:
            for stat in compstats:
                rdiff = (currstat[stat]-refstat[stat])/refstat[stat]
                self.assertTrue((abs(rdiff)<allowdiff),
                                msg="'%s' of spectrum %s are different." % (stat, str(0)))
            self.assertEqual(currstat['max_abscissa']['value'],
                             refstat['max_abscissa']['value'],
                             msg="The max channels/frequencies/velocities of spectrum %s are different" % str(0))
            self.assertEqual(currstat['min_abscissa']['value'],
                             refstat['min_abscissa']['value'],
                             msg="The min channels/frequencies/velocities of spectrum %s are different" % str(0))




class sdbaseline_masktest(unittest.TestCase):
    """
    Unit tests for task sdbaseline. Test various mask selections.
    Polynominal baselining. No interactive testing.

    The list of tests:
    masktest01-02 --- test masklist (list)
    masktest03-04 --- test masklist (string)
    masktest05-08 --- test specunit='GHz'
    masktest09-12 --- test specunit='km/s'

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdaverage)
      sdaverage(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdaverage)
      sdaverage(infile='temp.asap',timeaverage=True,
                tweight='tintsys',outfile=self.infile)
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdbaseline/'
    # Input and output names
    #infile = 'OrionS_rawACSmod_calTave.asap'
    infile = 'OrionS_rawACSmod_calave.asap'
    outroot = 'sdbaseline_masktest'
    blrefroot = datapath+'refblparam_mask'
    #strefroot = datapath+'refstats_mask'
    tid = None

    # Channel range excluding bad edge
    search = [[200,7599]]
    # Baseline channels. should be identical to one selected by 'auto' mode
    blchan0 = [[200,3979],[4152,7599]]
    blchan2 = [[200,2959],[3120,7599]]

    # reference values for specunit='channel'
    ref_pol0if0 =  {'linemaxpos': 4102.0, 'linesum': 103.81604766845703,
                    'linemax': 1.6280698776245117,
                    'baserms': 0.15021507441997528,
                    'basestd': 0.15022546052932739}
    ref_pol0if2 = {'linemaxpos': 3045.0, 'linesum': 127.79755401611328,
                   'linemax': 2.0193681716918945,
                   'baserms': 0.13134850561618805,
                   'basestd': 0.1313575953245163}
    # reference values for specunit='GHz'
    ref_pol0if0f = {'linemaxpos': 4102.0, 'linesum': 101.49118804931641,
                    'linemax': 1.6132903099060059,
                    'baserms': 0.15236321091651917,
                    'basestd': 0.15236999094486237}
    ref_pol0if2f = {'linemaxpos': 3045.0, 'linesum': 129.6041259765625,
                    'linemax': 2.0308537483215332,
                    'baserms': 0.13158561289310455,
                    'basestd': 0.13159450888633728}

    # reference values for specunit='km/s'
    ref_pol0if0v = {'linemaxpos': 4102.0, 'linesum': 103.81607055664062,
                    'linemax': 1.6280698776245117,
                    'baserms': 0.15021507441997528,
                    'basestd': 0.15022547543048859}
    ref_pol0if2v = {'linemaxpos': 3045.0, 'linesum': 128.9298095703125,
                    'linemax': 2.0264592170715332,
                    'baserms': 0.13150280714035034,
                    'basestd': 0.13151165843009949}
     
    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdbaseline)


    def tearDown(self):
        self._compareBLparam()
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)


    def testblmask01(self):
        """Mask test 1: test masklist (list) with maskmode = 'auto'"""
        self.tid="01"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        masklist = self.search
        iflist = [0,2]
        pollist=[0]

        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareDictVal(testval,self.ref_pol0if0)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2)

    def testblmask02(self):
        """Mask test 2: test masklist (list) with maskmode = 'list'"""
        self.tid="02"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        masklist = self.blchan2
        iflist = [2]
        pollist=[0]

        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2)

    def testblmask03(self):
        """Mask test 3: test masklist (string) with maskmode = 'auto'"""
        self.tid="03"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        #masklist = "0~2:200~7599"
        iflist = [0,2]
        pollist=[0]

        searchrange = self._get_range_in_string(self.search[0])
        masklist = "0~2:"+searchrange
        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareDictVal(testval,self.ref_pol0if0)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2)

    def testblmask04(self):
        """Mask test 4: test masklist (string) with maskmode = 'list'"""
        self.tid="04"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        #masklist = "0:200~3979;4152~7599, 2:200~2959;3120~7599"
        iflist = [0,2]
        pollist=[0]

        sblrange=[]
        chanlist = (self.blchan0, self.blchan2)
        for i in xrange(len(iflist)):
            sblrange.append("")
            for chanrange in chanlist[i]:
                if len(sblrange[i]): sblrange[i] += ";"
                sblrange[i] += self._get_range_in_string(chanrange)
        masklist = "0:"+sblrange[0]+", 2:"+sblrange[1]
        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareDictVal(testval,self.ref_pol0if0)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2)

    def testblmask05(self):
        """Mask test 5: test specunit='GHz' with masklist (list) and maskmode = 'auto'"""
        self.tid="05"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "GHz"
        #masklist = [[44.0511472,44.0963125]]
        iflist = [2]
        pollist=[0]

        masklist = self._get_chanval(infile,self.search,specunit,spw=iflist[0],addedge=True)
        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2f)

    def testblmask06(self):
        """Mask test 6: test specunit='GHz' with masklist (list) and maskmode = 'list'"""
        self.tid="06"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "GHz"
        #masklist = [[44.0511472,44.0679889],[44.0689716,44.0963125]]
        iflist = [2]
        pollist=[0]

        masklist = self._get_chanval(infile,self.blchan2,specunit,spw=iflist[0],addedge=True)
        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2f)

    def testblmask07(self):
        """Mask test 7: test specunit='GHz' with masklist (string) and maskmode = 'auto'"""
        self.tid="07"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "GHz"
        #masklist = "0:45.4655714~45.5107367, 2:44.0511472~44.0963125"
        iflist = [0,2]
        pollist=[0]

        sblrange=[]
        chanlist = (self.search, self.search)
        for i in xrange(len(iflist)):
            sblrange.append("")
            mlist = self._get_chanval(infile,chanlist[i],specunit,spw=iflist[i],addedge=True)
            for valrange in mlist:
                if len(sblrange[i]): sblrange[i] += ";"
                sblrange[i] += self._get_range_in_string(valrange)
        masklist = "0:"+sblrange[0]+", 2:"+sblrange[1]
        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareDictVal(testval,self.ref_pol0if0f)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2f)

    def testblmask08(self):
        """Mask test 8: test specunit='GHz' with masklist (string) and maskmode = 'list'"""
        self.tid="08"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "GHz"
        #masklist = "0:45.4655714~45.4886394;45.4896954~45.5107367, 2:44.0511472~44.0679889;44.0689716~44.0963125"
        iflist = [0,2]
        pollist=[0]

        sblrange=[]
        chanlist = (self.blchan0, self.blchan2)
        for i in xrange(len(iflist)):
            sblrange.append("")
            mlist = self._get_chanval(infile,chanlist[i],specunit,spw=iflist[i],addedge=True)
            for valrange in mlist:
                if len(sblrange[i]): sblrange[i] += ";"
                sblrange[i] += self._get_range_in_string(valrange)
        masklist = "0:"+sblrange[0]+", 2:"+sblrange[1]
        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareDictVal(testval,self.ref_pol0if0f)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2f)

    def testblmask09(self):
        """Mask test 9: test specunit='km/s' with masklist (list) and maskmode = 'auto'"""
        self.tid="09"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "km/s"
        #masklist = [[9186.458, 9484.109]]
        iflist = [2]
        pollist=[0]

        masklist = self._get_chanval(infile,self.search,specunit,spw=iflist[0],addedge=True)
        print "masklist =", masklist
        
        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2v)

    def testblmask10(self):
        """Mask test 10: test specunit='km/s' with masklist (list) and maskmode = 'list'"""
        self.tid="10"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "km/s"
        #masklist = [[9186.458, 9366.642],[9373.118, 9484.109]]
        iflist = [2]
        pollist=[0]

        masklist = self._get_chanval(infile,self.blchan2,specunit,spw=iflist[0],addedge=True)
        print "masklist =", masklist
        
        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2v)

    def testblmask11(self):
        """Mask test 11: test specunit='km/s' with masklist (string) and maskmode = 'auto'"""
        self.tid="11"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "km/s"
        #masklist = "0:-134.960~162.691, 2:9186.458~9484.109"
        iflist = [0,2]
        pollist=[0]

        sblrange=[]
        chanlist = (self.search, self.search)
        for i in xrange(len(iflist)):
            sblrange.append("")
            mlist = self._get_chanval(infile,chanlist[i],specunit,spw=iflist[i],addedge=True)
            for valrange in mlist:
                if len(sblrange[i]): sblrange[i] += ";"
                sblrange[i] += self._get_range_in_string(valrange)
        masklist = "0:"+sblrange[0]+", 2:"+sblrange[1]
        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareDictVal(testval,self.ref_pol0if0v)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2v)

    def testblmask12(self):
        """Mask test 12: test specunit='km/s' with masklist (string) and maskmode = 'list'"""
        self.tid="12"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "km/s"
        #masklist = "0:-134.960~3.708;10.667~162.691, 2:9373.118~9484.109;9186.458~9366.642"
        iflist = [0,2]
        pollist=[0]

        sblrange=[]
        chanlist = (self.blchan0, self.blchan2)
        for i in xrange(len(iflist)):
            sblrange.append("")
            mlist = self._get_chanval(infile,chanlist[i],specunit,spw=iflist[i],addedge=True)
            for valrange in mlist:
                if len(sblrange[i]): sblrange[i] += ";"
                sblrange[i] += self._get_range_in_string(valrange)
        masklist = "0:"+sblrange[0]+", 2:"+sblrange[1]
        print "masklist =", masklist

        result = sdbaseline(infile=infile,maskmode=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareDictVal(testval,self.ref_pol0if0v)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareDictVal(testval,self.ref_pol0if2v)


    def _get_range_in_string(self,valrange):
        if isinstance(valrange, list) or isinstance(valrange, tuple):
            return str(valrange[0])+"~"+str(valrange[1])
        else:
            return False

    def _get_chanval(self,file,chanrange,unit,spw=0,addedge=False):
        mylist = []
        scan = sd.scantable(file, average=False)
        scan.set_unit(unit)
        scan.set_selection(ifs=[spw])
        chanval = scan._getabcissa(0)
        edge = 0
        if addedge:
            # add 1/2 chan to both edges
            nchan = len(chanval)
            #edge = 0.0
            edge = 0.5*abs(chanval[nchan-1]-chanval[0])/float(nchan-1)
        for schan, echan in chanrange:
            lval = max(chanval[schan],chanval[echan])
            sval = min(chanval[schan],chanval[echan])
            mylist.append([sval-edge,lval+edge])
        del scan, nchan, edge, lval, sval
        return mylist

    def _compareBLparam(self):
        # test if baseline parameters are equal to the reference values
        # currently comparing every lines in the files
        # TO DO: compare only "Fitter range" and "Baseline parameters"
        out = self.outroot+self.tid+'.asap_blparam.txt'
        reference = self.blrefroot+self.tid
        self.assertTrue(os.path.exists(out))
        self.assertTrue(os.path.exists(reference),
                        msg="Reference file doesn't exist: "+reference)
#         self.assertTrue(listing.compare(out,reference),
#                         'New and reference files are different. %s != %s. '
#                         %(out,reference))


    def _getStats(self,filename,basechan,ispw):
        self.assertTrue(os.path.exists(filename),
                        msg=("Output file '%s' doesn't exist" % filename))
        linechan = [basechan[0][1]+1,basechan[1][0]-1]
        scan = sd.scantable(filename,average=False)
        scan.set_selection(ifs=[ispw])
        scan.set_unit('channel')
        linmsk = scan.create_mask(linechan)
        blmsk = scan.create_mask(basechan)
        # only the fist row is returned
        linmax = scan.stats('max',mask=linmsk)[0]
        linmaxpos = scan.stats('max_abc',mask=linmsk)[0]
        linesum = scan.stats('sum',mask=linmsk)[0]
        blrms = scan.stats('rms',mask=blmsk)[0]
        blstd = scan.stats('stddev',mask=blmsk)[0]
        del scan, linmsk, blmsk
        retdic = {'linemax': linmax, 'linemaxpos': linmaxpos,
                  'linesum': linesum, 'baserms': blrms, 'basestd': blstd}
        del linmax, linmaxpos, linesum, blrms, blstd
        print 'Current run (IF',ispw,'):',retdic
        return retdic
 
    def _compareDictVal(self,testdict,refdict,places=4):
        for stat, refval in refdict.iteritems():
            self.assertTrue(testdict.has_key(stat),
                            msg = "'%s' is not defined in the current run" % stat)
            allowdiff = 0.01
            #print "Comparing '%s': %f (current run), %f (reference)" % \
            #      (stat,testdict[stat],refval)
            reldiff = (testdict[stat]-refval)/refval
            self.assertTrue(reldiff < allowdiff,\
                            msg="'%s' differs: %f (ref) != %f" % \
                            (stat, refval, testdict[stat]))

class sdbaseline_functest(unittest.TestCase):
    """
    Unit tests for task sdbaseline. No interactive testing.

    The list of tests:
    testCSpline01  --- test cubic spline fitting with maskmode = 'list'
    testCSpline02  --- test cubic spline fitting with maskmode = 'auto'
    testSinusoid01 --- test sinusoidal fitting with maskmode = 'list'
    testSinusoid02 --- test sinusoidal fitting with maskmode = 'auto'

    Note: (1) the rms noise of input data is 1.0.
          (2) the max_rms value comes from +3sigma level of Chi-square
              distribution with freedom of (2048-38) for cspline, (2048-27)
              for sinusoid, respectively.

    created 19/04/2011 by Wataru Kawasaki
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdbaseline/'
    # Input and output names
    infile_cspline  = 'Artificial_CubicSpline.asap'
    infile_sinusoid = 'Artificial_Sinusoid.asap'
    blparamfile_suffix = '_blparam.txt'
    outroot = 'sdbaseline_test'
    tid = None

    def setUp(self):
        if os.path.exists(self.infile_cspline):
            shutil.rmtree(self.infile_cspline)
        shutil.copytree(self.datapath+self.infile_cspline, self.infile_cspline)
        if os.path.exists(self.infile_sinusoid):
            shutil.rmtree(self.infile_sinusoid)
        shutil.copytree(self.datapath+self.infile_sinusoid, self.infile_sinusoid)

        default(sdbaseline)

    def tearDown(self):
        if os.path.exists(self.infile_cspline):
            shutil.rmtree(self.infile_cspline)
        if os.path.exists(self.infile_sinusoid):
            shutil.rmtree(self.infile_sinusoid)

    def testCSpline01(self):
        """Test CSpline01: Cubic spline fitting with maskmode = 'list'"""
        self.tid = "CSpline01"
        infile = self.infile_cspline
        mode = "list"
        outfile = self.outroot+self.tid+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = sdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc='cspline',npiece=35)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self.checkRms(blparamfile, 1.038696)   #the actual rms should be 1.02407 though

    def testCSpline02(self):
        """Test CSpline02: Cubic spline fitting with maskmode = 'auto'"""
        self.tid = "CSpline02"
        infile = self.infile_cspline
        mode = "auto"
        outfile = self.outroot+self.tid+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = sdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc='cspline',npiece=35)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self.checkRms(blparamfile, 1.038696)   #the actual rms should be 1.02407 though

    def testSinusoid01(self):
        """Test Sinusoid01: Sinusoidal fitting with maskmode = 'list'"""
        self.tid = "Sinusoid01"
        infile = self.infile_sinusoid
        mode = "list"
        outfile = self.outroot+self.tid+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = sdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc='sinusoid')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self.checkRms(blparamfile, 1.10)   #the actual rms should be 1.09705 though

    def testSinusoid02(self):
        """Test Sinusoid02: Sinusoidal fitting with maskmode = 'auto'"""
        self.tid = "Sinusoid02"
        infile = self.infile_sinusoid
        mode = "auto"
        outfile = self.outroot+self.tid+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = sdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc='sinusoid')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self.checkRms(blparamfile, 1.10)   #the actual rms should be 1.09705 though

    def checkRms(self, blparamfile, max_rms):
        rms = 10000.0
        for line in open(blparamfile,'r'):
            items = line.split()
            if (items[0] == 'rms') and (items[1] == '='):
                rms = float(items[2])
                break

        self.assertTrue((rms <= max_rms), msg = "CSpline fitting failed.")


class sdbaseline_multi_IF_test(unittest.TestCase):
    """
    Unit tests for task sdbaseline. No interactive testing.

    This test intends to check whether sdbaseline task works fine
    for data that has multiple IFs whose nchan differ each other. 

    The list of tests:
    test0 --- test multi IF data input

    created 24/02/2012 by Takeshi Nakazato
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdbaseline/'
    # Input and output names
    infile = 'testMultiIF.asap'
    blparamfile_suffix = '_blparam.txt'
    outroot = 'sdbaseline_test'
    refblparamfile = 'refblparam_multiIF'

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(sdbaseline)

    def tearDown(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)

    def test01multi(self):
        """test01multi: Test the task works with multi IF data"""
        infile = self.infile
        mode = "list"
        blfunc = "poly"
        order = 1
        outfile = self.outroot+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = sdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc=blfunc,order=order)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._compareBLparam(blparamfile,self.datapath+self.refblparamfile)
        reference = {5: {'rms': 1.4250789880752563,
                         'min': -4.2702846527099609,
                         'max': 5.5566844940185547,
                         'max_abscissa': {'value': 823.0,
                                          'unit': 'channel'},
                         'median': 0.017315864562988281,
                         'min_abscissa': {'value': 520.0,
                                          'unit': 'channel'},
                         'stddev': 1.425775408744812},
                     7: {'rms': 1.4971292018890381,
                         'min': -4.7103700637817383,
                         'max': 5.4820127487182617,
                         'max_abscissa': {'value': 1335.0,
                                          'unit': 'channel'},
                         'median': 0.027227401733398438,
                         'min_abscissa': {'value': 1490.0,
                                          'unit': 'channel'},
                         'stddev': 1.4974949359893799}}
        # sdstat must run each IF separately
        for ifno in [5,7]:
            self._compareStats(outfile,ifno,reference[ifno])
        
    def _compareBLparam(self,out,reference):
        # test if baseline parameters are equal to the reference values
        # currently comparing every lines in the files
        # TO DO: compare only "Fitter range" and "Baseline parameters"
        self.assertTrue(os.path.exists(out))
        self.assertTrue(os.path.exists(reference),
                        msg="Reference file doesn't exist: "+reference)
        blparse_out = BlparamFileParser( out )
        blparse_out.parse()
        coeffs_out = blparse_out.coeff()
        rms_out = blparse_out.rms()
        blparse_ref = BlparamFileParser( reference )
        blparse_ref.parse()
        coeffs_ref = blparse_ref.coeff()
        rms_ref = blparse_ref.rms()
        allowdiff = 0.01
        print 'Check baseline parameters:'
        for irow in xrange(len(rms_out)):
            print 'Row %s:'%(irow)
            print '   Reference rms  = %s'%(rms_ref[irow])
            print '   Calculated rms = %s'%(rms_out[irow])
            print '   Reference coeffs  = %s'%(coeffs_ref[irow])
            print '   Calculated coeffs = %s'%(coeffs_out[irow])
            r0 = rms_ref[irow]
            r1 = rms_out[irow]
            rdiff = ( r1 - r0 ) / r0
            self.assertTrue((abs(rdiff)<allowdiff),
                            msg='row %s: rms is different'%(irow))
            c0 = coeffs_ref[irow]
            c1 = coeffs_out[irow]
            for ic in xrange(len(c1)):
                rdiff = ( c1[ic] - c0[ic] ) / c0[ic]
                self.assertTrue((abs(rdiff)<allowdiff),
                                msg='row %s: coefficient for order %s is different'%(irow,ic))
        print ''
#         self.assertTrue(listing.compare(out,reference),
#                         'New and reference files are different. %s != %s. '
#                         %(out,reference))

    def _compareStats(self,outfile,ifno,reference):
        # test if the statistics of baselined spectra are equal to
        # the reference values
        self.assertTrue(os.path.exists(outfile))
        currstat = sdstat(infile=outfile,iflist=ifno)
        self.assertTrue(isinstance(currstat,dict),
                        msg="Failed to calculate statistics.")

        #self.assertTrue(os.path.exists(reference),
        #                msg="Reference file doesn't exist: "+reference)
        refstat = reference

        print "Statistics of baselined spectra:\n",currstat
        print "Reference values:\n",refstat
        # compare statistic values
        #compstats = ['max','min','mean','sum','rms']
        compstats = ['max','min','rms','median','stddev']
        allowdiff = 0.01
        self.assertEqual(currstat['max_abscissa']['unit'],
                         refstat['max_abscissa']['unit'],
                         msg="The units of max_abscissa are different")
        self.assertEqual(currstat['min_abscissa']['unit'],
                         refstat['min_abscissa']['unit'],
                         msg="The units of min_abscissa are different")
        if isinstance(refstat['max'],list):
            for i in xrange(len(refstat['max'])):
                for stat in compstats:
                    rdiff = (currstat[stat][i]-refstat[stat][i])/refstat[stat][i]
                    self.assertTrue((abs(rdiff)<allowdiff),
                                    msg="'%s' of spectrum %s are different." % (stat, str(i)))
                self.assertEqual(currstat['max_abscissa']['value'][i],
                                 refstat['max_abscissa']['value'][i],
                                 msg="The max channels/frequencies/velocities of spectrum %s are different" % str(i))
                self.assertEqual(currstat['min_abscissa']['value'][i],
                                 refstat['min_abscissa']['value'][i],
                                 msg="The min channels/frequencies/velocities of spectrum %s are different" % str(i))
        else:
            for stat in compstats:
                rdiff = (currstat[stat]-refstat[stat])/refstat[stat]
                self.assertTrue((abs(rdiff)<allowdiff),
                                msg="'%s' of spectrum %s are different." % (stat, str(0)))
            self.assertEqual(currstat['max_abscissa']['value'],
                             refstat['max_abscissa']['value'],
                             msg="The max channels/frequencies/velocities of spectrum %s are different" % str(0))
            self.assertEqual(currstat['min_abscissa']['value'],
                             refstat['min_abscissa']['value'],
                             msg="The min channels/frequencies/velocities of spectrum %s are different" % str(0))
    


def suite():
    return [sdbaseline_basictest, sdbaseline_masktest, sdbaseline_functest,
            sdbaseline_multi_IF_test]

### Utilities for reading blparam file
class FileReader( object ):
    def __init__( self, filename ):
        self.__filename = filename
        self.__data = None
        self.__nline = None

    def read( self ):
        if self.__data is None:
            f = open(self.__filename, 'r')
            self.__data = f.readlines()
            f.close()
            self.__nline = len( self.__data )
        return

    def nline( self ):
        self.read()
        return self.__nline

    def index( self, txt, start ):
        return self.__data[start:].index( txt ) + 1 + start

    def getline( self, idx ):
        return self.__data[idx]

class BlparamFileParser( FileReader ):
    def __init__( self, blfile ):
        FileReader.__init__( self, blfile )
        self.__nrow = None
        self.__coeff = None
        self.__rms = None
        self.__ctxt = 'Baseline parameters\n'
        self.__rtxt = 'Results of baseline fit\n'

    def nrow( self ):
        self.read()
        if self.__nrow is None:
            return self._nrow()
        else:
            return self.__nrow

    def coeff( self ):
        self.read()
        if self.__coeff is None:
            self.parseCoeff()
        return self.__coeff

    def rms( self ):
        self.read()
        if self.__rms is None:
            self.parseRms()
        return self.__rms

    def _nrow( self ):
        self.__nrow = 0
        for i in xrange(self.nline()):
            if self.getline( i ) == self.__ctxt:
                self.__nrow += 1
        return self.__nrow

    def parse( self ):
        self.read()
        self.parseCoeff()
        self.parseRms()
        return
        
    def parseCoeff( self ):
        self.__coeff = []
        nrow = self.nrow()
        idx = 0
        while ( len(self.__coeff) < nrow ):
            try:
                idx = self.index( self.__ctxt, idx )
                coeffs = []
                while( self.getline( idx ) != self.__rtxt ):
                    coeff = self.__parseCoeff( idx )
                    coeffs += coeff
                    idx += 1
                self.__coeff.append( coeffs )
            except:
                break
        return

    def parseRms( self ):
        self.__rms = []
        nrow = self.nrow()
        idx = 0
        while ( len(self.__rms) < nrow ):
            try:
                idx = self.index( self.__rtxt, idx )
                self.__rms.append( self.__parseRms( idx ) )
            except:
                break   
        return

    def __parseCoeff( self, idx ):
        return parseCoeff( self.getline( idx ) )

    def __parseRms( self, idx ):
        return parseRms( self.getline( idx ) )

def parseCoeff( txt ):
    clist = txt.rstrip( '\n' ).split(',')
    ret = []
    for c in clist:
        ret.append( float( c.split('=')[1] ) )
    return ret
    
def parseRms( txt ):
    t = txt.lstrip().rstrip( '\n' )[6:]
    return float( t )

