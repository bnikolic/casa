//# ChannelAverageTVI.h: This file contains the implementation of the ChannelAverageTVI class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#include <mstransform/TVI/ChannelAverageTVI.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVI class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
ChannelAverageTVI::ChannelAverageTVI(	ViImplementation2 * inputVii,
										const Record &configuration):
										FreqAxisTVI (inputVii,configuration)
{
	// Parse and check configuration parameters
	// Note: if a constructor finishes by throwing an exception, the memory
	// associated with the object itself is cleaned up — there is no memory leak.
	if (not parseConfiguration(configuration))
	{
		throw AipsError("Error parsing ChannelAverageTVI configuration");
	}

	initialize();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool ChannelAverageTVI::parseConfiguration(const Record &configuration)
{
	int exists = 0;
	Bool ret = True;

	// Parse chanbin parameter (mandatory)
	exists = configuration.fieldNumber ("chanbin");
	if (exists >= 0)
	{
		if ( configuration.type(exists) == casa::TpInt )
		{
			Int freqbin;
			configuration.get (exists, freqbin);
			chanbin_p = Vector<Int>(spwInpChanIdxMap_p.size(),freqbin);
		}
		else if ( configuration.type(exists) == casa::TpArrayInt)
		{
			configuration.get (exists, chanbin_p);
		}
		else
		{
			ret = False;
			logger_p << LogIO::SEVERE << LogOrigin("ChannelAverageTVI", __FUNCTION__)
					<< "Wrong format for chanbin parameter (only Int and arrayInt are supported) "
					<< LogIO::POST;
		}

		logger_p << LogIO::NORMAL << LogOrigin("ChannelAverageTVI", __FUNCTION__)
				<< "Channel bin is " << chanbin_p << LogIO::POST;
	}
	else
	{
		ret = False;
		logger_p << LogIO::SEVERE << LogOrigin("ChannelAverageTVI", __FUNCTION__)
				<< "chanbin parameter not found in configuration "
				<< LogIO::POST;
	}

	// Check consistency between chanbin vector and selected SPW/Chan map
	if (chanbin_p.size() !=  spwInpChanIdxMap_p.size())
	{
		ret = False;
		logger_p << LogIO::SEVERE << LogOrigin("ChannelAverageTVI", __FUNCTION__)
				<< "Number of elements in chanbin vector does not match number of selected SPWs"
				<< LogIO::POST;
	}

	return ret;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::initialize()
{
	// Populate nchan input-output maps
	Int spw;
	uInt spw_idx = 0;
	map<Int,vector<Int> >::iterator iter;
	for(iter=spwInpChanIdxMap_p.begin();iter!=spwInpChanIdxMap_p.end();iter++)
	{
		spw = iter->first;

		// Make sure that chanbin is greater than 1
		if ((uInt)chanbin_p(spw_idx) <= 1)
		{
			logger_p << LogIO::DEBUG1 << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Selected chanbin for spw " << spw
					<< " set to 1 fallbacks to the default number of"
					<< " existing/selected channels: " << iter->second.size()
					<< LogIO::POST;

			spwChanbinMap_p[spw] = iter->second.size();
		}
		// Make sure that chanbin does not exceed number of selected channels
		else if ((uInt)chanbin_p(spw_idx) > iter->second.size())
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Number of selected channels " << iter->second.size()
					<< " for SPW " << spw
					<< " is smaller than specified chanbin " << chanbin_p(spw_idx) << endl
					<< "Setting chanbin to " << iter->second.size()
					<< " for SPW " << spw
					<< LogIO::POST;
			spwChanbinMap_p[spw] = iter->second.size();
		}
		else
		{
			spwChanbinMap_p[spw] = chanbin_p(spw_idx);
		}

		// Calculate number of output channels per spw
		spwOutChanNumMap_p[spw] = spwInpChanIdxMap_p[spw].size() / spwChanbinMap_p[spw];
		if (spwInpChanIdxMap_p[spw].size() % spwChanbinMap_p[spw] > 0) spwOutChanNumMap_p[spw] += 1;

		spw_idx++;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::flag(Cube<Bool>& flagCube) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure Transformation Engine
	LogicalANDKernel<Bool> kernel;
	uInt width = spwChanbinMap_p[inputSPW];
	ChannelAverageTransformEngine<Bool> transformer(&kernel,width);

	// Transform data
	transformFreqAxis(vb->flagCube(),flagCube,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::floatData (Cube<Float> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	DataCubeHolder<Float> weightCubeHolder(vb->weightSpectrum());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);
	auxiliaryData.add(MS::WEIGHT_SPECTRUM,weightCubeHolder);

	// Configure Transformation Engine
	uInt width = spwChanbinMap_p[inputSPW];
	WeightedChannelAverageKernel<Float> kernel(&auxiliaryData);
	ChannelAverageTransformEngine<Float> transformer(&kernel,width);

	// Transform data
	transformFreqAxis(vb->visCubeFloat(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::visibilityObserved (Cube<Complex> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Get weightSpectrum from sigmaSpectrum
	Cube<Float> weightSpFromSigmaSp;
	weightSpFromSigmaSp.resize(vb->sigmaSpectrum().shape(),False);
	weightSpFromSigmaSp = vb->sigmaSpectrum(); // = Operator makes a copy
	arrayTransformInPlace (weightSpFromSigmaSp,sigmaToWeight);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	DataCubeHolder<Float> weightCubeHolder(weightSpFromSigmaSp);
	auxiliaryData.add(MS::FLAG,flagCubeHolder);
	auxiliaryData.add(MS::WEIGHT_SPECTRUM,weightCubeHolder);

	// Configure Transformation Engine
	uInt width = spwChanbinMap_p[inputSPW];
	WeightedChannelAverageKernel<Complex> kernel(&auxiliaryData);
	ChannelAverageTransformEngine<Complex> transformer(&kernel,width);

	// Transform data
	transformFreqAxis(vb->visCube(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::visibilityCorrected (Cube<Complex> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	DataCubeHolder<Float> weightCubeHolder(vb->weightSpectrum());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);
	auxiliaryData.add(MS::WEIGHT_SPECTRUM,weightCubeHolder);

	// Configure Transformation Engine
	uInt width = spwChanbinMap_p[inputSPW];
	WeightedChannelAverageKernel<Complex> kernel(&auxiliaryData);
	ChannelAverageTransformEngine<Complex> transformer(&kernel,width);

	// Transform data
	transformFreqAxis(vb->visCubeCorrected(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::visibilityModel (Cube<Complex> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	DataCubeHolder<Float> weightCubeHolder(vb->weightSpectrum());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);
	auxiliaryData.add(MS::WEIGHT_SPECTRUM,weightCubeHolder);

	// Configure Transformation Engine
	uInt width = spwChanbinMap_p[inputSPW];
	WeightedChannelAverageKernel<Complex> kernel(&auxiliaryData);
	ChannelAverageTransformEngine<Complex> transformer(&kernel,width);

	// Transform data
	transformFreqAxis(vb->visCubeModel(),vis,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::weightSpectrum(Cube<Float> &weightSp) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);

	// Configure Transformation Engine
	ChannelAccumulationKernel<Float> kernel(&auxiliaryData);
	uInt width = spwChanbinMap_p[inputSPW];
	ChannelAverageTransformEngine<Float> transformer(&kernel,width);

	// Transform data
	transformFreqAxis(vb->weightSpectrum(),weightSp,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::sigmaSpectrum(Cube<Float> &sigmaSp) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Get weightSpectrum from sigmaSpectrum
	Cube<Float> weightSpFromSigmaSp;
	weightSpFromSigmaSp.resize(vb->sigmaSpectrum().shape(),False);
	weightSpFromSigmaSp = vb->sigmaSpectrum(); // = Operator makes a copy
	arrayTransformInPlace (weightSpFromSigmaSp,sigmaToWeight);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);

	// Configure Transformation Engine
	uInt width = spwChanbinMap_p[inputSPW];
	ChannelAccumulationKernel<Float> kernel(&auxiliaryData);
	ChannelAverageTransformEngine<Float> transformer(&kernel,width);

	// Transform data
	transformFreqAxis(weightSpFromSigmaSp,sigmaSp,transformer);

	// Transform back from weight format to sigma format
	arrayTransformInPlace (sigmaSp,weightToSigma);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Vector<Double> ChannelAverageTVI::getFrequencies (	Double time,
													Int frameOfReference,
													Int spectralWindowId,
													Int msId) const
{
	// Get frequencies from input VI
	Vector<Double> inputFrequencies = getVii()->getFrequencies(time,frameOfReference,
																spectralWindowId,msId);

	// Produce output (transformed) frequencies
	Vector<Double> outputFrecuencies(spwOutChanNumMap_p[spectralWindowId]);

	// Configure Transformation Engine
	PlainChannelAverageKernel<Double> kernel;
	uInt width = spwChanbinMap_p[spectralWindowId];
	ChannelAverageTransformEngine<Double> transformer(&(kernel),width);

	// Transform data
	transformer.transform(inputFrequencies,outputFrecuencies);

	return outputFrecuencies;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::writeFlag (const Cube<Bool> & flag)
{
	// Create a flag cube with the input VI shape
	Cube<Bool> propagatedFlagCube;
	propagatedFlagCube = getVii()->getVisBuffer()->flagCube();

	// Propagate flags from the input cube to the propagated flag cube
	propagateChanAvgFlags(flag,propagatedFlagCube);

	// Pass propagated flag cube downstream for further propagation and/or writting
	getVii()->writeFlag(propagatedFlagCube);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::propagateChanAvgFlags (const Cube<Bool> &transformedFlagCube,
												Cube<Bool> &propagatedFlagCube)
{
	// Get current SPW and chanbin
	VisBuffer2 *inputVB = getVii()->getVisBuffer();
	Int inputSPW = inputVB->spectralWindows()(0);
	uInt width = spwChanbinMap_p[inputSPW];

	// Get propagated (input) shape
	IPosition inputShape = propagatedFlagCube.shape();
	size_t nCorr = inputShape(0);
	size_t nChan = inputShape(1);
	size_t nRows = inputShape(2);

	// Get transformed (output) shape
	IPosition transformedShape = transformedFlagCube.shape();
	size_t nTransChan = transformedShape(1);

	// Map input-output channel
	uInt binCounts = 0;
	uInt transformedIndex = 0;
	Vector<uInt> inputOutputChan(nChan);
	for (size_t chan_i =0;chan_i<nChan;chan_i++)
	{
		binCounts += 1;

		if (binCounts > width)
		{
			binCounts = 1;
			transformedIndex += 1;
		}

		inputOutputChan(chan_i) = transformedIndex;
	}

	// Propagate chan-avg flags
	uInt outChan;
	for (size_t row_i =0;row_i<nRows;row_i++)
	{
		for (size_t chan_i =0;chan_i<nChan;chan_i++)
		{
			outChan = inputOutputChan(chan_i);
			if (outChan < nTransChan) // outChan >= nChan  may happen when channels are dropped
			{
				for (size_t corr_i =0;corr_i<nCorr;corr_i++)
				{
					if (transformedFlagCube(corr_i,outChan,row_i)) propagatedFlagCube(corr_i,chan_i,row_i) = True;
				}
			}
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVIFactory class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
ChannelAverageTVIFactory::ChannelAverageTVIFactory (Record &configuration,
													ViImplementation2 *inputVii)
{
	inputVii_p = inputVii;
	configuration_p = configuration;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * ChannelAverageTVIFactory::createVi(VisibilityIterator2 *) const
{
	return new ChannelAverageTVI(inputVii_p,configuration_p);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * ChannelAverageTVIFactory::createVi() const
{
	return new ChannelAverageTVI(inputVii_p,configuration_p);
}

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTransformEngine class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ChannelAverageTransformEngine<T>::setRowIndex(uInt row)
{
	chanAvgKernel_p->setRowIndex(row);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ChannelAverageTransformEngine<T>::setCorrIndex(uInt corr)
{
	chanAvgKernel_p->setCorrIndex(corr);
}

//////////////////////////////////////////////////////////////////////////
// ChannelAverageKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ChannelAverageKernel<T>::setRowIndex(uInt row)
{
	if (auxiliaryData_p != NULL) auxiliaryData_p->setMatrixIndex(row);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ChannelAverageKernel<T>::setCorrIndex(uInt corr)
{
	if (auxiliaryData_p != NULL) auxiliaryData_p->setVectorIndex(corr);
}

//////////////////////////////////////////////////////////////////////////
// PlainChannelAverageKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> PlainChannelAverageKernel<T>::PlainChannelAverageKernel(DataCubeMap *auxiliaryData)
{
	auxiliaryData_p = auxiliaryData;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void PlainChannelAverageKernel<T>::kernel(	Vector<T> &inputVector,
																Vector<T> &outputVector,
																uInt startInputPos,
																uInt outputPos,
																uInt width)
{
	uInt pos = startInputPos + 1;
	uInt counts = 1;
	T avg = inputVector(startInputPos);
	while (counts < width)
	{
		avg += inputVector(pos);
		counts += 1;
		pos += 1;
	}

	if (counts > 0)
	{
		avg /= counts;
	}

	outputVector(outputPos) = avg;

	return;
}

//////////////////////////////////////////////////////////////////////////
// WeightedChannelAverageKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> WeightedChannelAverageKernel<T>::WeightedChannelAverageKernel(DataCubeMap *auxiliaryData)
{
	auxiliaryData_p = auxiliaryData;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void WeightedChannelAverageKernel<T>::kernel(	Vector<T> &inputVector,
																Vector<T> &outputVector,
																uInt startInputPos,
																uInt outputPos,
																uInt width)
{
	T avg = 0;
	T normalization = 0;
	uInt inputPos = 0;
	DataCubeMap &auxiliaryData = *auxiliaryData_p;
	Vector<Bool> &inputFlagVector = auxiliaryData.getVector<Bool>(MS::FLAG);
	Vector<Float> &inputWeightVector = auxiliaryData.getVector<Float>(MS::WEIGHT_SPECTRUM);
	Bool accumulatorFlag = inputFlagVector(startInputPos);

	for (uInt sample_i=0;sample_i<width;sample_i++)
	{
		// Get input index
		inputPos = startInputPos + sample_i;

		// True/True or False/False
		if (accumulatorFlag == inputFlagVector(inputPos))
		{
			normalization += inputWeightVector(inputPos);
			avg += inputVector(inputPos)*inputWeightVector(inputPos);
		}
		// True/False: Reset accumulation when accumulator switches from flagged to unflag
		else if ( (accumulatorFlag == True) and (inputFlagVector(inputPos) == False) )
		{
			accumulatorFlag = False;
			normalization = inputWeightVector(inputPos);
			avg = inputVector(inputPos)*inputWeightVector(inputPos);
		}
	}


	// Apply normalization factor
	if (normalization > 0)
	{
		avg /= normalization;
		outputVector(outputPos) = avg;
	}
	// If all weights are zero set accumulatorFlag to True
	else
	{
		accumulatorFlag = True;
		outputVector(outputPos) = 0; // If all weights are zero then the avg is 0 too
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
// LogicalANDKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> LogicalANDKernel<T>::LogicalANDKernel(DataCubeMap *auxiliaryData)
{
	auxiliaryData_p = auxiliaryData;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void LogicalANDKernel<T>::kernel(	Vector<T> &inputVector,
													Vector<T> &outputVector,
													uInt startInputPos,
													uInt outputPos,
													uInt width)
{
	Bool outputFlag = True;
	for (uInt sample_i=0;sample_i<width;sample_i++)
	{
		if (not inputVector(startInputPos+sample_i))
		{
			outputFlag = False;
			break;
		}
	}

	outputVector(outputPos) = outputFlag;

	return;
}

//////////////////////////////////////////////////////////////////////////
// ChannelAccumulationKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> ChannelAccumulationKernel<T>::ChannelAccumulationKernel(DataCubeMap *auxiliaryData)
{
	auxiliaryData_p = auxiliaryData;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ChannelAccumulationKernel<T>::kernel(	Vector<T> &inputVector,
																Vector<T> &outputVector,
																uInt startInputPos,
																uInt outputPos,
																uInt width)
{
	T acc = 0;
	uInt inputPos = 0;
	DataCubeMap &auxiliaryData = *auxiliaryData_p;
	Vector<Bool> &inputFlagVector = auxiliaryData.getVector<Bool>(MS::FLAG);
	Bool accumulatorFlag = inputFlagVector(startInputPos);

	for (uInt sample_i=0;sample_i<width;sample_i++)
	{
		// Get input index
		inputPos = startInputPos + sample_i;

		// True/True or False/False
		if (accumulatorFlag == inputFlagVector(inputPos))
		{
			acc += inputVector(inputPos);
		}
		// True/False: Reset accumulation when accumulator switches from flagged to unflag
		else if ( (accumulatorFlag == True) and (inputFlagVector(inputPos) == False) )
		{
			accumulatorFlag = False;
			acc = inputVector(inputPos);
		}
	}


	outputVector(outputPos) = acc;

	return;
}

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END


