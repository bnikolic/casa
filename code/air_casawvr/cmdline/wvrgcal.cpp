/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file wvrgcal.cpp

   This is a command line program that reads WVR data from a CASA
   measurement set, computes the predicted complex gain of each
   antenna as a function of time from these WVR data and then writes
   these solutions out to a CASA gain table. 
   
*/


#include <iostream>

#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#include <ms/MeasurementSets/MeasurementSet.h>

#include "../casawvr/mswvrdata.hpp"
#include "../casawvr/msgaintable.hpp"
#include "../casawvr/msutils.hpp"
#include "../casawvr/msspec.hpp"
#include "../casawvr/msantdata.hpp"
#include "../src/apps/arraydata.hpp"
#include "../src/apps/arraygains.hpp"
#include "../src/apps/almaabs.hpp"
#include "../src/apps/dtdlcoeffs.hpp"
#include "../src/apps/almaresults.hpp"
#include "../src/apps/segmentation.hpp"
#include "../src/libair_main.hpp"

#include "wvrgcalerrors.hpp"
#include "wvrgcalfeedback.hpp"

using LibAIR2::fatalMsg;
using LibAIR2::errorMsg;
using LibAIR2::warnMsg;

/// Check the options and parameters supplied by the user for internal
/// consistency 
bool checkPars(const boost::program_options::variables_map &vm)
{
  if (vm.count("ms") <1)
  {
    fatalMsg("No input measurement sets given -- aborting ");
    return true;
  }

  if (vm.count("output") <1)
  {
    fatalMsg("No output file give -- aborting ");
    return true;
  }

  if (vm.count("segfield") )
  {
    fatalMsg("The --segfield option has been removed because it does not handle mosaic observations well.");
    fatalMsg("Please use the --segsource option instead");
    return true;
  }

  if (vm.count("segsource") && vm.count("cont"))
  {
    fatalMsg("Multiple retrievals using continuum estimation not yet supported");
    fatalMsg("Use only one of  --segfield OR --cont");
    return true;
  }

  if (vm["nsol"].as<int>()>1 && vm.count("cont"))
  {
    fatalMsg("Multiple retrievals using continuum estimation not yet supported");
    fatalMsg("You can not use the nsol parameter yet with cont");
    return true;
  }

 if (vm.count("sourceflag") && !vm.count("segsource"))
  {
    fatalMsg("Can only flag a source using --sourceflag if the --segsource option is also used");
    fatalMsg("Please either remove the --sourceflag option or also specify the --segsource option");
    return true;
  }

 if (vm.count("tie") && !vm.count("segsource"))
  {
    fatalMsg("Can only tie sources together if the --segsource option is also used");
    fatalMsg("Please either remove the --tie option or also specify the --segsource option");
    return true;
  }

  if (vm.count("reverse") and vm.count("reversespw"))
  {
    warnMsg("You are specifying both the reverse and reversespw options;"
	    "the latter will be ignored and all spectral windows will be reversed");
  }
  return false;

  if (vm.count("smooth") and vm["smooth"].as<double>() < 1)
  {
    warnMsg("Smooth parameter must be 1 or greater");
    return true;
  }
  return false;

  if (vm.count("maxdistm") and vm["maxdistm"].as<int>() < 1)
  {
    warnMsg("maxdistm parameter must be 0. or greater");
    return true;
  }
  return false;
  
}

void checkWarnPars(const boost::program_options::variables_map &vm)
{

  if (vm.count("statfield"))
  {
    warnMsg("The use of \"statfield\" is not recommended as mosaiced"
	    "observations are recorded as many separate fields. Recomended option"
	    "to use is \"statsource\". ");
  }
}

/* Checks on parameter that can only be done once the measurement set
   is opened.  Do not put computationally intensive checks otherwise
   feedback to the user will be too slow.
*/
void checkMSandPars(const casa::MeasurementSet &ms,
		    const boost::program_options::variables_map &vm)
{
  if (vm.count("statsource"))
  {
    std::string srcname=vm["statsource"].as<std::vector<std::string> >()[0];
    std::set<size_t> fselect=LibAIR2::getSrcFields(ms,
						  srcname);
    if (fselect.size() == 0)
    {
      std::cout<<"WARNING: No fields appear to be identified with source "<<srcname <<std::endl
	       <<"         that you supplied to the statsource option"<<std::endl
	       <<"         Statistics will be corrupted and wvrgcal may fail"<<std::endl
	       <<std::endl;
    }
  }

}

/** \brief Take a parameter than can be specified as a sequence of
    antenna numbers or names and always return as a sequence of
    antenna numbers.
 */
std::set<int> getAntPars(const std::string &s,
			 const boost::program_options::variables_map &vm,
			 const casa::MeasurementSet &ms)
{
  using namespace LibAIR2;
  aname_t anames=getAName(ms);
  std::vector<std::string> pars=vm[s].as<std::vector<std::string> >();
  std::set<int> res;
  for (size_t i=0; i<pars.size(); ++i)
  {
    if (anames.right.count(pars[i]))
    {
      res.insert(anames.right.at(pars[i]));
    }
    else
    {
      // should be an antenna number
      try {
	int n=boost::lexical_cast<int>(pars[i]);
	if (anames.left.count(n) ==0)
	{
	  throw AntIDError(n,
			   anames);
	}
	res.insert(n);
      }
      catch (const boost::bad_lexical_cast & bc)
      {
	throw AntIDError(pars[i],
			 anames);

      }

    }
  }


  return res;
}


/** Simple function to turn the command line parameters into a single
    string.

    Return by value as only needs to be called once in program
*/
static std::string buildCmdLine(int argc,
				char* argv[])
{
  std::string cmdline;
  for (size_t i=0; i< (size_t)argc; ++i)
  {
    cmdline+=std::string(argv[i]);
    cmdline+=" ";
  }
  return cmdline;
}


/* Determine the nearest n antennas not accepting ones which
   are flagged or have a distance > maxdist_m
*/
 
LibAIR2::AntSetWeight limitedNearestAnt(const LibAIR2::antpos_t &pos,
				       size_t i,
				       const LibAIR2::AntSet &flag,
				       size_t n,
				       double maxdist_m)
{
  LibAIR2::AntSetD dist=LibAIR2::antsDist(pos, i, flag);
  LibAIR2::AntSetWeight res;
    
  double total=0;
  size_t limitedn=0;
  LibAIR2::AntSetD::const_iterator s=dist.begin();
  for (size_t j=0; j<n; ++j)
  {
    if(s!=dist.end() and s->first <= maxdist_m)
    {
      total+=s->first;
      ++s;
      ++limitedn;
    }
  }

  s=dist.begin();
  for (size_t j=0; j<limitedn; ++j)
  {
    res.insert(std::make_pair<double, size_t>(s->first/total, s->second));
    ++s;
  }

  return res;

}


/** \brief Flag and interpolate WVR data
 */
void flagInterp(const casa::MeasurementSet &ms,
		const std::set<int> &wvrflag,
		LibAIR2::InterpArrayData &d,
		const double maxdist_m,
		const int minnumants,
		std::set<int> &interpImpossibleAnts)
{

  LibAIR2::antpos_t apos;
  LibAIR2::getAntPos(ms, apos);
  LibAIR2::AntSet wvrflag_s(wvrflag.begin(), 
			   wvrflag.end());

  for(std::set<int>::const_iterator i=wvrflag.begin();
      i!=wvrflag.end(); 
      ++i)
  {

    LibAIR2::AntSetWeight near=limitedNearestAnt(apos, 
						 *i, 
						 wvrflag_s, 
						 3,
						 maxdist_m);
    if(near.size()>= static_cast<unsigned int>(minnumants)){
      //LibAIR2::interpBadAntW(d, *i, near);
      const LibAIR2::InterpArrayData::wvrdata_t &data(d.g_wvrdata());
      for(size_t ii=0; ii<d.g_time().size(); ++ii){
	for(size_t k=0; k < 4; ++k){
	  double p=0;
	  for(LibAIR2::AntSetWeight::const_iterator j=near.begin(); j!=near.end(); ++j){
	    double thisData = data[ii][j->second][k];
	    if(thisData>0){
	      p+=thisData*j->first;
	    }
	    else{ // no good data; set solution to zero => will be flagged later
	      p=0.;
	      break;
	    }
	  }
	  d.set(ii, *i, k, p);
	}
      }
    }
    else
    { 
      std::ostringstream oss;
      oss << "Antenna " << *i 
	  << " has bad or no WVR and only " << near.size() << " near antennas ("
	  << maxdist_m << " m max. distance) to interpolate from. Required are " 
	  << minnumants << "." << std::endl;
      std::cout << std::endl << "*** " << oss.str();
      std::cerr << oss.str();
      for(size_t j=0; j<d.g_time().size(); ++j)
      {
        for(size_t k=0; k < 4; ++k)
        {
          d.set(j, *i, k, 0.); // set all WVR data of this antenna to zero => will be flagged later
        }
      }
      interpImpossibleAnts.insert(*i);
    }
  }
  
}



/// Work out which spectral windows might need to be reversed
std::set<size_t> reversedSPWs(const LibAIR2::MSSpec &sp,
			      const boost::program_options::variables_map &vm)
{
  std::set<size_t> reverse;
  if (vm.count("reverse"))
  {
    for (size_t spw =0; spw<sp.spws.size(); ++spw)
      reverse.insert(spw);
  }    
  if (vm.count("reversespw"))
  {
    std::vector<int> torev=vm["reversespw"].as<std::vector<int> >();
    for(size_t i=0; i<torev.size(); ++i)
    {
      if (torev[i]<0 or torev[i] >= (int)sp.spws.size())
      {
	throw LibAIR2::SPWIDError(torev[i], 
				 sp.spws.size());
      }
      reverse.insert(torev[i]);
    }
  }    
  return reverse;
}

void printExpectedPerf(const LibAIR2::ArrayGains &g,
		       const LibAIR2::dTdLCoeffsBase &coeffs,
		       const std::vector<std::pair<double, double> > &tmask)
{

  std::cout<<"  Expected performance "<<std::endl
	   <<"------------------------------------------------------------------"<<std::endl;
  
  std::vector<double> cr, err;
  coeffs.repr(cr, err);
  std::cout<<"* Estimated WVR thermal contribution to path fluctuations (micron per antenna): "
	   <<LibAIR2::thermal_error(cr)/1e-6
	   <<std::endl;
  const double grmsbl=g.greatestRMSBl(tmask);
  std::cout<<"* Greatest Estimated path fluctuation is (micron on a baseline): "
	   <<grmsbl/1e-6
	   <<std::endl;
  std::cout<<"* Rough estimate path error due to coefficient error (micron on a baseline): "
	   <<grmsbl* (err[1]/cr[1])/1e-6
	   <<std::endl
    	   <<std::endl;
}

/** Compute the time intervals over which the statistics should be
    computed
 */
void statTimeMask(const casa::MeasurementSet &ms,
		  const boost::program_options::variables_map &vm,
		  std::vector<std::pair<double, double> > &tmask,
		  const std::vector<size_t> &sortedI)
{
  std::vector<int> flds;
  std::vector<double> time;
  std::vector<int> src;
  LibAIR2::fieldIDs(ms, 
		   time,
		   flds,
		   src,
		   sortedI);
  std::vector<size_t> spws;
  LibAIR2::dataSPWs(ms, spws, sortedI);

  if (vm.count("statfield") == 0 && vm.count("statsource") == 0)
  {
    tmask.resize(0);
    tmask.push_back(std::pair<double, double>(time[0], time[time.size()-1]));
  }
  else if ( vm.count("statsource") > 0)
  {
    std::set<size_t> fselect=LibAIR2::getSrcFields(ms,
						  vm["statsource"].as<std::vector<std::string> >()[0]);
    LibAIR2::fieldTimes(time,
		       flds,
		       spws,
		       fselect,
		       0,
		       tmask);    
  }
  else
  {
    std::vector<std::string> fields=vm["statfield"].as<std::vector<std::string> >();
    LibAIR2::field_t fnames=LibAIR2::getFieldNames(ms);

    std::set<size_t> fselect;
    if (fnames.right.count(fields[0])) // User supplied  field *name*
    {
      size_t n=fnames.right.at(fields[0]);
      fselect.insert(n);
    }
    else
    {
      try 
      {
	size_t n=boost::lexical_cast<int>(fields[0]);
	fselect.insert(n);
      }
      catch (const boost::bad_lexical_cast & bc)
      {
	std::cout<<"Warning: Could not understand statfield argument. Will use zeroth field."
		 <<std::endl;
      }
    }
    LibAIR2::fieldTimes(time,
		       flds,
		       spws,
		       fselect,
		       0,
		       tmask);
  }
  LibAIR2::printStatTimes(std::cout,
			 time,
			 tmask);
}
		  

/// Compute the discrepance in path estimate between channels 1 and 3
void computePathDisc(const LibAIR2::InterpArrayData &d,
		     const std::vector<std::pair<double, double> > &tmask,
		     LibAIR2::dTdLCoeffsBase  &coeffs,
		     std::vector<double> &res)
{
  LibAIR2::ArrayGains g1(d.g_time(), 
			d.g_el(),
			d.g_state(),
			d.g_field(),
			d.g_source(),
			d.nWVRs);
  boost::array<double, 4> c1mask = {{0, 1, 0,0}};
  boost::array<double, 4> c3mask = {{0, 0, 0,1}};
  boost::array<double, 4> callmask = {{1, 1, 1,1}};
    
  coeffs.chmask=c1mask;
  g1.calc(d,
	  coeffs);    
  
  LibAIR2::ArrayGains g3(d.g_time(), 
			d.g_el(),
			d.g_state(),
			d.g_field(),
			d.g_source(),
			d.nWVRs);
  coeffs.chmask=c3mask;
  g3.calc(d,
	  coeffs);    

  g1.pathDiscAnt(g3, 
		 tmask,
		 res);

  coeffs.chmask=callmask;
  
}

void printFieldSegments(const std::vector<std::pair<double, double> >  &fb,
			double tbase)
{
  for (size_t i=0; i<fb.size(); ++i)
    {
      std::cout<<fb[i].first-tbase<<","<<fb[i].second-tbase<<std::endl;
    }

}

std::vector<std::set<std::string> > getTied(const boost::program_options::variables_map &vm)
{
  using namespace boost::algorithm;

  std::vector<std::set<std::string> > res;

  //only run if --tie option given on command line
  if (vm.count("tie"))
    {
    const std::vector<std::string>
      &input=vm["tie"].as<std::vector<std::string> >();
    
    for (size_t i=0; i< input.size(); ++i)
      {
	std::set<std::string> cs;
	const std::string &par=input[i];
	split(cs, par, is_any_of(","));
	res.push_back(cs);
      }
    }
    return res;
}

// Convert tied source names to tied source IDs
std::vector<std::set<size_t> >  tiedIDs(const std::vector<std::set<std::string> > &tied,
					const casa::MeasurementSet &ms)
{
  boost::bimap<size_t, std::string > srcmap=LibAIR2::getSourceNames(ms);
  std::vector<std::set<size_t> > res;
  for (size_t i=0; i<tied.size(); ++i)
  {
    std::set<size_t> cs;
    for(std::set<std::string>::const_iterator j=tied[i].begin();
	j!=tied[i].end();
	++j)
      {	
	try
	  {
	    int srcid=boost::lexical_cast<int>(*j);
	    cs.insert(srcid);
	  }
	catch (const std::exception& x)
	  {
	    try{
	      cs.insert(srcmap.right.at(*j));
	    }
	    catch (const std::exception& y){
	      std::ostringstream oss;
	      oss << "Parameter 'tie': The field " << *j << " is not recognised. Please check for typos." << std::endl;
	      throw LibAIR2::WVRUserError(oss.str());
	    }
	  }
	res.push_back(cs);
      } // end for
  }
  return res;
}

void printTied(const std::vector<std::set<std::string> > &tied,
	       const std::vector<std::set<size_t> > &tiedi)
{
  for(size_t i=0; i<tied.size(); ++i)
  {
    std::cout<<"Tying: ";
    BOOST_FOREACH(const std::string &x, tied[i])
      std::cout<<x<<" and ";
    std::cout<<std::endl;
  }
  if (tied.size())
    std::cout<<"Tied sets as numerical source IDs:"<<std::endl;
  for(size_t i=0; i<tiedi.size(); ++i)
  {
    std::cout<<"Tying: ";
    BOOST_FOREACH(const size_t &x, tiedi[i])
      std::cout<<x<<" and ";
    std::cout<<std::endl;
  }

}

/** Compute the set of source_ids corresponding to a vector of source
    names.
 */
std::set<size_t> sourceSet(const std::vector<std::string> &sources,
			   const casa::MeasurementSet &ms)
{
  boost::bimap<size_t, std::string > snames=LibAIR2::getSourceNames(ms);
  std::set<size_t> sset;
  for(size_t i=0; i<sources.size(); ++i)
    sset.insert(snames.right.at(sources[i]));  
  return sset;
}
  

/** Filter the set of input WVR measurements to retrieve the
    coefficients from to exclude flagged sources

    This function takes and returns two containers: the first is the
    list of WVR values to be analysed; the second is a vector of time
    ranges to use.
    
    These time ranges must be filtered together with the inputs since
    they are not referenced directly to the inputs are simply assumed
    to be "row-synchronous".
    
 */
std::pair<LibAIR2::ALMAAbsInpL,  std::vector<std::pair<double, double> > >
filterInp(const LibAIR2::ALMAAbsInpL &inp,
	  const std::vector<std::pair<double, double> > &fb,
	  const std::vector<std::string> &sourceflag,
	  const casa::MeasurementSet &ms)
{

  std::set<size_t> flagset=sourceSet(sourceflag, ms);

  LibAIR2::ALMAAbsInpL res;
  std::vector<std::pair<double, double> > rfb;
  size_t j=0;
  for(LibAIR2::ALMAAbsInpL::const_iterator i=inp.begin();
      i!=inp.end();
      ++i, ++j)
  {
    if (flagset.count(i->source) ==0)
    {
      res.push_back(*i);
      rfb.push_back(fb[j]);
    }
  }
  return std::make_pair(res, rfb);
}

/** Filter the set of input WVR measurements to retrieve the
    coefficients from to exclude flagged data points (zero Tobs)
    
 */
std::pair<LibAIR2::ALMAAbsInpL,  std::vector<std::pair<double, double> > >
filterFlaggedInp(const LibAIR2::ALMAAbsInpL &inp,
		 const std::vector<std::pair<double, double> > &fb)
{

  LibAIR2::ALMAAbsInpL res;
  std::vector<std::pair<double, double> > rfb;
  size_t j=0;
  bool fbFilled = (fb.size()>0);
  for(LibAIR2::ALMAAbsInpL::const_iterator i=inp.begin();
      i!=inp.end();
      ++i, ++j)
  {
    if(i->TObs[0]>0.) // flagged ALMAAbsInp would have TObs==0
    {
      res.push_back(*i);
      if(fbFilled)
      {
	rfb.push_back(fb[j]);
      }
    }
  }
  return std::make_pair(res, rfb);
}

/** Return the set of antenna IDs that do not have a WVR
 */
std::set<int> NoWVRAnts(const LibAIR2::aname_t &an)
{
  std::set<int> res;
  for(LibAIR2::aname_t::const_iterator i=an.begin();
      i!= an.end();
      ++i)
  {
    if (i->right[0]=='C' and i->right[1]=='M')
      res.insert(i->left);
  }
  return res;
}


static void defineOptions(boost::program_options::options_description &desc,
			  boost::program_options::positional_options_description &p)
{
  using namespace boost::program_options;
  desc.add_options()
    ("ms", value< std::vector<std::string> >(),
     "Input measurement set")
    ("output",
     value<std::string>(),
     "Name of the output file")
    ("toffset",
     value<double>()->default_value(0),
     "Time offset (in seconds) between interferometric and WVR data")
    ("nsol",
     value<int>()->default_value(1),
     "Number of solutions for phase correction coefficients to make during this observation")
    ("help",
     "Print information about program usage")
    ("segfield",
     "Do a new coefficient calculation for each field (this option is disabled, see segsource)")
    ("segsource",
     "Do a new coefficient calculation for each source")
    ("reverse",
     "Reverse the sign of correction in all SPW (e.g. due to AIV-1740)")
    ("reversespw",
     value< std::vector<int> >(),
     "Reverse the sign correction for this spw")
    ("disperse",
     "Apply correction for dispersion")
    ("cont",
     "UNTESTED! Estimate the continuum (e.g., due to clouds)")
    ("wvrflag",
     value< std::vector<std::string> >(),
     "Flag this WVR (labelled with either antenna number or antenna name) as bad, and replace its data with interpolated values ")
    ("sourceflag",
     value< std::vector<std::string> >(),
     "Flag the WVR data for this source and do not produce any phase corrections on it")
    ("statfield",
     value< std::vector<std::string> >(),
     "Compute the statistics (Phase RMS, Disc) on this field only")
    ("statsource",
     value< std::vector<std::string> >(),
     "Compute the statistics (Phase RMS, Disc) on this source only")
    ("tie",
     value< std::vector<std::string> >(),
     "Prioritise tieing the phase of these sources as well as possible")
    ("smooth",
     value<int>(),
     "Smooth WVR data by this many samples before applying the correction")
    ("scale",
     value<double>()->default_value(1.0),
     "Scale the entire phase correction by this factor")
    ("maxdistm",
     value<double>()->default_value(500.0),
     "maximum distance (m) an antenna may have to be considered for being part of the <=3 antenna set for interpolation of a solution for a flagged antenna")
    ("minnumants",
     value<int>()->default_value(2),
     "minimum number of near antennas (up to 3) required for interpolation")
    ("mingoodfrac",
     value<double>()->default_value(0.8),
     "If the fraction of unflagged data for an antenna is below this value (0. to 1.), the antenna is flagged.")
    ("usefieldtab",
     "Derive the antenna pointing information from the FIELD table instead of the POINTING table.")
    ("spw",
     value< std::vector<int> >(),
     "Only write out corrections for these SPWs.")
    ("wvrspw",
     value< std::vector<int> >(),
     "Only use data from these WVR SPWs.")
    ;
  p.add("ms", -1);
}

int main(int argc,  char* argv[])
{
  using namespace boost::program_options;

  int rval = -2;

  options_description desc("Allowed options");
  positional_options_description p;
  defineOptions(desc, p);


  variables_map vm;
  store(command_line_parser(argc, argv).
	options(desc).positional(p).run(), 
 	vm);
  notify(vm);

  LibAIR2::printBanner(std::cout);


  
  if (vm.count("help"))
  {
    std::cout<<"Write out a gain table based on WVR data"
      	     <<std::endl
	     <<std::endl
	     <<"GPL license -- you have the right to the source code. See COPYING"
	     <<std::endl
	     <<std::endl
	     <<desc;
    return 0;
  }

  if (checkPars(vm))
  {
    return -1;
  }

  checkWarnPars(vm);
  std::vector<std::set<std::string> > tied=getTied(vm);

  std::string msname(vm["ms"].as<std::vector<std::string> >()[0]);
  casa::MeasurementSet ms(msname); 

  checkMSandPars(ms, vm);

  std::vector<int> wvrspws;
  {
    LibAIR2::SPWSet thewvrspws=LibAIR2::WVRSPWIDs(ms);
    if (vm.count("wvrspw")){
      wvrspws=vm["wvrspw"].as<std::vector<int> >();
	
      if (wvrspws.size() > 0){
	for(size_t i=0; i<wvrspws.size(); i++){
	  if(thewvrspws.count(wvrspws[i])==0){
	    std::cout << "ERROR: SPW " << wvrspws[i] << " is not a WVR SPW or invalid." <<std::endl;
	    std::cerr << "ERROR: SPW " << wvrspws[i] << " is not a WVR SPW or invalid." <<std::endl;
	    return -10;
	  }
	}
	std::cout<<"Will use the following WVR SPWs:"<<std::endl;
	for(size_t i=0; i<wvrspws.size();i++){
	  std::cout<< " " << wvrspws[i];
	}
	std::cout <<std::endl;
      }
    }
    if (wvrspws.size()==0){
      std::cout<<"Will use all WVR SPWs:"<<std::endl;
      for(LibAIR2::SPWSet::const_iterator si=thewvrspws.begin(); si!=thewvrspws.end();++si){
	wvrspws.push_back(*si);
	std::cout<< " " <<  *si;
      }
      std::cout <<std::endl;
    }
  }

  std::vector<int> sciencespws;

  if (vm.count("spw")){
    sciencespws=vm["spw"].as<std::vector<int> >();
    LibAIR2::SPWSet thewvrspws=LibAIR2::WVRSPWIDs(ms);
    if (sciencespws.size() > 0){
      int nspw=LibAIR2::numSPWs(ms);
      for(size_t i=0; i<sciencespws.size(); i++){
	if(thewvrspws.count(sciencespws[i])!=0){
	  std::cout<<"WARNING: SPW "<< sciencespws[i] << " is a WVR SPW, not a science SPW." <<std::endl;
	  std::cerr<<"WARNING: SPW "<< sciencespws[i] << " is a WVR SPW, not a science SPW." <<std::endl;
	}
	if(sciencespws[i]<0 || sciencespws[i]>= nspw){
	  std::cout<<"ERROR: Invalid SPW "<< sciencespws[i] <<std::endl;
	  std::cerr<<"ERROR: Invalid SPW "<< sciencespws[i] <<std::endl;
	  return -11;
	}
      }
      std::cout<<"Will produce solutions for the following SPWs:"<<std::endl;
      for(size_t i=0; i<sciencespws.size();i++){
	std::cout<< " " << sciencespws[i];
      }
      std::cout <<std::endl;
    }
  }
  if(sciencespws.size()==0){
    std::cout<<"Will produce solutions for all SPWs:"<<std::endl;
    for(size_t i=0; i<LibAIR2::numSPWs(ms);i++){
      sciencespws.push_back(i);
      std::cout<< " " << i;
    }
    std::cout <<std::endl;
  }

  std::string fnameout=vm["output"].as<std::string>();

  std::set<size_t> useID=LibAIR2::skyStateIDs(ms);

  std::set<int> wvrflag;
  // Prepare flagging and interpolation
  if (vm.count("wvrflag"))
  {
     wvrflag=getAntPars("wvrflag", vm, ms);    
  }

  LibAIR2::aname_t anames=LibAIR2::getAName(ms);
  std::set<int> nowvr=NoWVRAnts(anames);
  
  std::set<int> interpwvrs(wvrflag); // the antennas to interpolate solutions for
  interpwvrs.insert(nowvr.begin(), nowvr.end());

  std::set<int> flaggedants; // the antennas flagged in the ANTENNA table are not to be interpolated
  LibAIR2::WVRAddFlaggedAnts(ms, flaggedants);

  wvrflag.insert(flaggedants.begin(),flaggedants.end());

  if(interpwvrs.size()+flaggedants.size()==ms.antenna().nrow()){
    std::cout << "No good antennas with WVR data found." << std::endl;
    std::cerr << "No good antennas with WVR data found." << std::endl;
    return -1;
  }

  int iterations = 0;

  while(rval<0 && iterations<2){

     iterations++;

     std::vector<size_t> sortedI; // to be filled with the time-sorted row number index
     std::set<int> flaggedantsInMain; // the antennas totally flagged in the MS main table
     boost::scoped_ptr<LibAIR2::InterpArrayData> d (LibAIR2::loadWVRData(ms,
									 wvrspws,
									 sortedI, 
									 flaggedantsInMain,
									 vm["mingoodfrac"].as<double>(),
									 vm.count("usefieldtab")==0)
						   );

     interpwvrs.insert(flaggedantsInMain.begin(),flaggedantsInMain.end()); // for flagInterp()
     wvrflag.insert(flaggedantsInMain.begin(),flaggedantsInMain.end());

     d->offsetTime(vm["toffset"].as<double>());
     
     if (vm.count("smooth"))
     {
	smoothWVR(*d, vm["smooth"].as<int>());
     }
     
     d.reset(LibAIR2::filterState(*d, useID));

     std::set<int> interpImpossibleAnts;

     // Flag and interpolate
     flagInterp(ms,
		interpwvrs,
		*d,
		vm["maxdistm"].as<double>(),
		vm["minnumants"].as<int>(),
		interpImpossibleAnts);
     
     LibAIR2::ArrayGains g(d->g_time(), 
			  d->g_el(),
			  d->g_state(),
			  d->g_field(),
			  d->g_source(),
			  d->nWVRs);
     
     boost::scoped_ptr<LibAIR2::dTdLCoeffsBase>  coeffs;
     
     // These are the segments on which coefficients are re-calculated
     std::vector<std::pair<double, double> >  fb;
     
     if ( vm.count("cont") )
     {
	std::cout<<"[Output from \"cont\" option has not yet been updated]"
		 <<std::endl;
	coeffs.reset(LibAIR2::SimpleSingleCont(*d));
     }
     else
     {
	
	LibAIR2::ALMAAbsInpL inp;
	if (vm.count("segsource"))
	{
	   std::vector<int> flds;
	   std::vector<double> time;
	   std::vector<int> src;
	   LibAIR2::fieldIDs(ms, 
			    time,
			    flds,
			    src,
			    sortedI);
	   try{
	     std::vector<std::set<size_t> >  tiedi=tiedIDs(tied, ms);
	     
	     printTied(tied, tiedi);
	     LibAIR2::fieldSegmentsTied(time,
					src,
					tiedi,
					fb);
	   }
	   catch(LibAIR2::WVRUserError& x){
	     std::cout << x.what() << std::endl;
	     std::cerr << x.what() << std::endl;
	     return -1;
	   }

	   //printFieldSegments(fb, time[0]);
	   
//       { // debugging output
// 	std::vector<double> tt(d->g_time());
// 	std::vector<double> te(d->g_el());
// 	std::vector<size_t> ts(d->g_state());
// 	std::vector<size_t> tf(d->g_field());
// 	std::vector<size_t> tsou(d->g_source());
// 	for(uint i=0; i<tt.size(); i++){
// 	  std::cerr << "i time el state field source " << i << " " << tt[i] << " ";
// 	  std::cerr << te[i] << " ";
// 	  std::cerr << ts[i] << " ";
// 	  std::cerr << tf[i] << " ";
// 	  std::cerr << tsou[i] << std::endl;
// 	}
// 	std::cerr << "nWVRs " << d->nWVRs << std::endl;
// 	for(uint i=0; i<time.size(); i++){
// 	  std::cerr << "i time  " << i << " " << time[i] << std::endl;
// 	}
// 	for(uint i=0; i<fb.size(); i++){
// 	  std::cerr << "i fb  " << i << " " << fb[i].first << " " << fb[i].second  << std::endl;
// 	}
// 	for(std::set<size_t>::iterator it = useID.begin(); it != useID.end(); it++){
// 	  std::cerr << "useID " << *it << std::endl;
// 	}
//       }

	   inp=FieldMidPointI(*d,
			      time,
			      fb,
			      useID);
	    
	}
	else
	{
	   const size_t n=vm["nsol"].as<int>();
	   inp=LibAIR2::MultipleUniformI(*d, 
					n,
					useID);
	}

	
	if (vm.count("sourceflag"))
	{
	   boost::tie(inp,fb)=filterInp(inp,
					fb,
					vm["sourceflag"].as<std::vector<std::string> >(),
					ms);
	}
	
	boost::tie(inp,fb)=filterFlaggedInp(inp,
					    fb);

	std::cerr<<"Calculating the coefficients now...";
	boost::ptr_list<LibAIR2::ALMAResBase> rlist;
	std::vector<int> problemAnts;

	rval = 0;

	try {
	   rlist=LibAIR2::doALMAAbsRet(inp,
				       problemAnts);
	}
	catch(const std::runtime_error rE){
	   std::cerr << std::endl << "WARNING: problem while calculating coefficients:"
		     << std::endl << "         LibAIR2::doALMAAbsRet: " << rE.what() << std::endl;
	   std::cout << std::endl << "WARNING: problem while calculating coefficients:"
		     << std::endl << "         LibAIR2::doALMAAbsRet: " << rE.what() << std::endl;
	   rval = -2;
	}
	
	if(problemAnts.size()>0){
	   
	   rval = -2;

	   if(iterations<2){
	      for(size_t i=0; i<problemAnts.size(); i++){
		 if(interpwvrs.count(problemAnts[i])==0){
		    std::cerr	<< "Flagging antenna " << problemAnts[i] << " == " << anames.left.at(problemAnts[i]) << std::endl;
		    std::cout	<< "Flagging antenna " << problemAnts[i] << " == " << anames.left.at(problemAnts[i]) << std::endl;
		    interpwvrs.insert(problemAnts[i]); // for flagInterp()
		    wvrflag.insert(problemAnts[i]); // for later log output
		 }
	      }
	      std::cerr	<< "Reiterating ..." << std::endl;
	      std::cout	<< "Reiterating ..." << std::endl;
	      continue;
	   }
	   else{
	      std::cerr << "Number of remaining problematic WVR measurements: " << problemAnts.size() << std::endl;
	      std::cout << "Number of remaining problematic WVR measurements: " << problemAnts.size() << std::endl;
	      std::cerr << "Will continue without further iterations ..." << std::endl;
	      std::cout << "Will continue without further iterations ..." << std::endl;
	   }	      
	}	   
	
	std::cerr<<"done!"
		 <<std::endl;
	
	
	std::cout<<"       Retrieved parameters      "<<std::endl
		 <<"----------------------------------------------------------------"<<std::endl
		 <<rlist<<std::endl;
	
	if (vm.count("segsource"))
	{
	   std::vector<int> flds;
	   std::vector<double> time;
	   std::vector<int> src;
	   LibAIR2::fieldIDs(ms, 
			    time,
			    flds,
			    src,
			    sortedI);
	   
	   coeffs.reset(LibAIR2::SimpleMultiple(*d, 
					       time,
					       fb,
					       rlist));   
	}
	else
	{
	   coeffs.reset(LibAIR2::ALMAAbsProcessor(inp, rlist));
	}  
	
     }
    
     std::vector<double> nantimes;

     if (coeffs->zeronan(nantimes))
     {
       LibAIR2::printNoSolution(std::cerr, nantimes);
     }
     g.calc(*d,
	    *coeffs);    

     if (vm.count("sourceflag"))
     {
	std::set<size_t> flagset=sourceSet(vm["sourceflag"].as<std::vector<std::string> >(),
					   ms);
	g.blankSources(flagset);
     }
     
     
     std::vector<std::pair<double, double> > tmask;
     statTimeMask(ms, vm, tmask, sortedI);
     
     std::vector<double> pathRMS;
     g.pathRMSAnt(tmask, pathRMS);
     
     
     std::vector<double> pathDisc;
     computePathDisc(*d, 
		     tmask,
		     *coeffs,
		     pathDisc);
     
     std::cout<<LibAIR2::AntITable(anames,
				  wvrflag,
				  nowvr,
				  pathRMS,
				  pathDisc,
				  interpImpossibleAnts);
     
     printExpectedPerf(g, 
		       *coeffs,
		       tmask);
     
     if (vm.count("scale"))
     {
	g.scale(vm["scale"].as<double>());
     }
     
     LibAIR2::MSSpec sp;
     loadSpec(ms, sciencespws, sp);
     std::set<size_t> reverse=reversedSPWs(sp, vm);  
     
     std::cout << "Writing gain table ..." << std::endl;

     // Write new table, including history
     LibAIR2::writeNewGainTbl(g,
			     fnameout.c_str(),
			     sp,
			     reverse,
			     vm.count("disperse")>0,
			     msname,
			     buildCmdLine(argc,
					  argv),
			     interpImpossibleAnts);

#ifdef BUILD_HD5
     LibAIR2::writeAntPath(g,
			  fnameout+".hd5");
#endif

  } // end while


  return rval;
}