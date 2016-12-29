import collections
import pdb

from numba import cfunc, types, carray
from matplotlib import pylab
import cffi
import numpy



applyGradientToYLine_sig=types.void(types.longlong,
                                    types.CPointer(types.complex64),
                                    types.CPointer(types.complex64),
                                    types.double,
                                    types.double,
                                    types.longlong,
                                    types.longlong,
                                    types.longlong,
                                    types.longlong)

@cfunc(applyGradientToYLine_sig, cache=True, nogil=True)
def xapplyGradientToYLine(iy,
                         convFunctions,
			 convWeights,
                         pixXdir,
                         pixYdir,
                         convSize,
                         ndishpair,
                         nChan,
                         nPol):
    cf=carray(convFunctions, (ndishpair, nChan, nPol, convSize, convSize))
    cw=carray(convWeights, (ndishpair, nChan, nPol, convSize, convSize))    
    cf[:,:,:,:,0] *= -1.0
    return

nniter=100000
ff=cffi.FFI()
@ff.callback("void (*)(size_t, float*, float *, double, double ,size_t ,size_t ,size_t ,size_t)")
def applyGradientToYLine_ffi(iy,
                             convFunctions,
			     convWeights,
                             pixXdir,
                             pixYdir,
                             convSize,
                             ndishpair,
                             nChan,
                             nPol):
    global nniter
    if iy==0:
        vf=numpy.frombuffer(ff.buffer(convFunctions,
                                      2*8*convSize*convSize),
                            dtype="complex64",
                            count=convSize*convSize)
        vf.shape=(convSize,convSize)
        pylab.matshow(vf.real)
        pylab.savefig("t%i.png" % nniter)
        nniter += 1
        pylab.close()
    return
applyGradientToYLine=collections.namedtuple("FFIX", "address")
applyGradientToYLine.address=int(ff.cast("size_t", applyGradientToYLine_ffi))
