import os
import numpy
import re
import sys
import shutil
from __main__ import default
from recipes.listshapes import listshapes
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task split.

Features tested:
  1. Are the POLARIZATION, DATA_DESCRIPTION, and (to some extent) the
     SPECTRAL_WINDOW tables correct with and without correlation selection?
  2. Are the data shapes and values correct with and without correlation
     selection?
  3. Are the WEIGHT and SIGMA shapes and values correct with and without
     correlation selection?

Note: The time_then_chan_avg regression is a more general test of split.
'''

datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/'

def check_eq(val, expval, tol=None):
    """Checks that val matches expval within tol."""
    try:
        if tol:
            are_eq = abs(val - expval) < tol
        else:
            are_eq = val == expval
        if hasattr(are_eq, 'all'):
            are_eq = are_eq.all()
        if not are_eq:
            raise ValueError, '!='
    except ValueError:
        raise ValueError, "%r != %r" % (val, expval)


class SplitChecker(unittest.TestCase):
    """Base class for split unit tests."""
    # Don't setup class variables here - the children would squabble over them.
    
    def setUp(self):
        if self.need_to_initialize:
            self.initialize()

    def tearDown(self):
        """
        Only check_subtables() needs MSes, and it should take care of rming them.
        """
        pass
    
    def initialize(self):
        # The realization that need_to_initialize needs to be
        # a class variable more or less came from
        # http://www.gossamer-threads.com/lists/python/dev/776699
        self.__class__.need_to_initialize = False
                
        inpms = self.inpms
    
        if not os.path.exists(inpms):
            # Copying is technically unnecessary for split,
            # but self.inpms is shared by other tests, so making
            # it readonly might break them.
            shutil.copytree(datapath + inpms, inpms)

        for corrsel in self.corrsels:
            self.res = self.do_split(corrsel)

    def check_subtables(self, corrsel, expected):
        oms = self.records[corrsel]['ms']
        assert listshapes(mspat=oms)[oms] == set(expected)
        shutil.rmtree(oms)


class split_test_tav(SplitChecker):
    need_to_initialize = True
    inpms = '0420+417/0420+417.ms'
    corrsels = ['', 'rr, ll', 'rl, lr', 'rr', 'll']
    records = {}
    
    def do_split(self, corrsel):
        outms = 'tav' + re.sub(',\s*', '', corrsel) + '.ms'
        record = {'ms': outms}

        shutil.rmtree(outms, ignore_errors=True)
        try:
            print "\nTime averaging", corrsel
            splitran = split(self.inpms, outms, datacolumn='data',
                             field='', spw='', width=1, antenna='',
                             timebin='20s', timerange='',
                             scan='', array='', uvrange='',
                             correlation=corrsel, async=False)
            tb.open(outms)
            record['data']   = tb.getcell('DATA', 2)
            record['weight'] = tb.getcell('WEIGHT', 5)
            record['sigma']  = tb.getcell('SIGMA', 7)
            tb.close()
        except Exception, e:
            print "Error time averaging and reading", outms
            raise e
        self.__class__.records[corrsel] = record
        return splitran

    def test_sts(self):
        """Subtables, time avg. without correlation selection"""
        self.check_subtables('', [(4, 1)])

    def test_sts_rrll(self):
        """Subtables, time avg. RR, LL"""
        self.check_subtables('rr, ll', [(2, 1)])
        
    def test_sts_rllr(self):
        """Subtables, time avg. RL, LR"""
        self.check_subtables('rl, lr', [(2, 1)])
        
    def test_sts_rr(self):
        """Subtables, time avg. RR"""
        self.check_subtables('rr', [(1, 1)])
        
    def test_sts_ll(self):
        """Subtables, time avg. LL"""
        self.check_subtables('ll', [(1, 1)])

    ## # split does not yet return a success value, and exceptions
    ## # are captured.
    ## # But at least on June 10 it correctly exited with an error
    ## # msg for correlation = 'rr, rl, ll'.
    ## def test_abort_on_rrrlll(self):
    ##     """
    ##     Cannot slice out RR, RL, LL
    ##     """
    ##     self.assertFalse(self.doSplit('rr, rl, ll'))
        
    def test_data(self):
        """DATA[2],   time avg. without correlation selection"""
        check_eq(self.records['']['data'],
                 numpy.array([[ 0.14428490-0.03145669j],
                              [-0.00379944+0.00710297j],
                              [-0.00381106-0.00066403j],
                              [ 0.14404297-0.04763794j]]),
                 0.0001)
        
    def test_data_rrll(self):
        """DATA[2],   time avg. RR, LL"""
        check_eq(self.records['rr, ll']['data'],
                 numpy.array([[ 0.14428490-0.03145669j],
                              [ 0.14404297-0.04763794j]]),
                 0.0001)

    def test_data_rllr(self):
        """DATA[2],   time avg. RL, LR"""
        check_eq(self.records['rl, lr']['data'],
                 numpy.array([[-0.00379944+0.00710297j],
                              [-0.00381106-0.00066403j]]),
                 0.0001)
        
    def test_data_rr(self):
        """DATA[2],   time avg. RR"""
        check_eq(self.records['rr']['data'],
                 numpy.array([[ 0.14428490-0.03145669j]]),
                 0.0001)

    def test_data_ll(self):
        """DATA[2],   time avg. LL"""
        check_eq(self.records['ll']['data'],
                 numpy.array([[ 0.14404297-0.04763794j]]),
                 0.0001)

    def test_wt(self):
        """WEIGHT[5], time avg. without correlation selection"""
        check_eq(self.records['']['weight'],
                 numpy.array([143596.34375, 410221.34375,
                              122627.1640625, 349320.625]),
                 1.0)

    def test_wt_rrll(self):
        """WEIGHT[5], time avg. RR, LL"""
        check_eq(self.records['rr, ll']['weight'],
                 numpy.array([143596.34375, 349320.625]),
                 1.0)

    def test_wt_rllr(self):
        """WEIGHT[5], time avg. RL, LR"""
        check_eq(self.records['rl, lr']['weight'],
                 numpy.array([410221.34375, 122627.1640625]),
                 1.0)

    def test_wt_rr(self):
        """WEIGHT[5], time avg. RR"""
        check_eq(self.records['rr']['weight'],
                 numpy.array([143596.34375]),
                 1.0)

    def test_wt_ll(self):
        """WEIGHT[5], time avg. LL"""
        check_eq(self.records['ll']['weight'],
                 numpy.array([349320.625]),
                 1.0)

    def test_sigma(self):
        """SIGMA[7],  time avg. without correlation selection"""
        check_eq(self.records['']['sigma'],
                 numpy.array([0.00168478, 0.00179394,
                              0.00182574, 0.00194404]),
                 0.0001)
        
    def test_sigma_rrll(self):
        """SIGMA[7],  time avg. RR, LL"""
        check_eq(self.records['rr, ll']['sigma'],
                 numpy.array([0.00168478, 0.00194404]),
                 0.0001)
        
    def test_sigma_rllr(self):
        """SIGMA[7],  time avg. RL, LR"""
        check_eq(self.records['rl, lr']['sigma'],
                 numpy.array([0.00179394, 0.00182574]),
                 0.0001)
        
    def test_sigma_rr(self):
        """SIGMA[7],  time avg. RR"""
        check_eq(self.records['rr']['sigma'],
                 numpy.array([0.00168478]),
                 0.0001)
        
    def test_sigma_ll(self):
        """SIGMA[7],  time avg. LL"""
        check_eq(self.records['ll']['sigma'],
                 numpy.array([0.00194404]),
                 0.0001)


class split_test_cav(SplitChecker):
    need_to_initialize = True
    corrsels = ['', 'rr', 'll']
    inpms = 'viewertest/ctb80-vsm.ms'
    records = {}
    
    def do_split(self, corrsel):
        outms = 'cav' + re.sub(',\s*', '', corrsel) + '.ms'
        record = {'ms': outms}

        shutil.rmtree(outms, ignore_errors=True)
        try:
            print "\nChannel averaging", corrsel
            splitran = split(self.inpms, outms, datacolumn='data',
                             field='', spw='0:5~16', width=3,
                             antenna='',
                             timebin='', timerange='',
                             scan='', array='', uvrange='',
                             correlation=corrsel, async=False)
            tb.open(outms)
            record['data']   = tb.getcell('DATA', 2)
            record['weight'] = tb.getcell('WEIGHT', 5)
            record['sigma']  = tb.getcell('SIGMA', 7)
            tb.close()
        except Exception, e:
            print "Error channel averaging and reading", outms
            raise e
        self.records[corrsel] = record
        return splitran

    def test_sts(self):
        """Subtables, chan avg. without correlation selection"""
        self.check_subtables('', [(2, 4)])

    def test_sts_rr(self):
        """Subtables, chan avg. RR"""
        self.check_subtables('rr', [(1, 4)])
        
    def test_sts_ll(self):
        """Subtables, chan avg. LL"""
        self.check_subtables('ll', [(1, 4)])

    def test_data(self):
        """DATA[2],   chan avg. without correlation selection"""
        check_eq(self.records['']['data'],
                 numpy.array([[16.795681-42.226387j, 20.5655-44.9874j,
                               26.801544-49.595020j, 21.4770-52.0462j],
                              [-2.919122-38.427235j, 13.3042-50.8492j,
                                4.483857-43.986446j, 10.1733-19.4007j]]),
                 0.0005)
        
    def test_data_rr(self):
        """DATA[2],   chan avg. RR"""
        check_eq(self.records['rr']['data'],
                 numpy.array([[16.79568-42.226387j, 20.5655-44.9874j,
                               26.80154-49.595020j, 21.4770-52.0462j]]),
                 0.0001)

    def test_data_ll(self):
        """DATA[2],   chan avg. LL"""
        check_eq(self.records['ll']['data'],
                 numpy.array([[-2.919122-38.427235j, 13.3042-50.8492j,
                                4.483857-43.986446j, 10.1733-19.4007j]]),
                 0.0001)

    def test_wt(self):
        """WEIGHT[5], chan avg. without correlation selection"""
        check_eq(self.records['']['weight'],
                 numpy.array([0.38709676, 0.38709676]),
                 0.001)

    def test_wt_rr(self):
        """WEIGHT[5], chan avg. RR"""
        check_eq(self.records['rr']['weight'],
                 numpy.array([0.38709676]),
                 0.001)

    def test_wt_ll(self):
        """WEIGHT[5], chan avg. LL"""
        check_eq(self.records['ll']['weight'],
                 numpy.array([0.38709676]),
                 0.001)

    def test_sigma(self):
        """SIGMA[7],  chan avg. without correlation selection"""
        check_eq(self.records['']['sigma'],
                 numpy.array([1.60727513, 1.60727513]),
                 0.0001)
        
    def test_sigma_rr(self):
        """SIGMA[7],  chan avg. RR"""
        check_eq(self.records['rr']['sigma'],
                 numpy.array([1.60727513]),
                 0.0001)
        
    def test_sigma_ll(self):
        """SIGMA[7],  chan avg. LL"""
        check_eq(self.records['ll']['sigma'],
                 numpy.array([1.60727513]),
                 0.0001)
        

def suite():
    return [split_test_tav, split_test_cav]        
        
    
