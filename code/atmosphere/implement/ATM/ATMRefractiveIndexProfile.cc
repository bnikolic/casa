#include <string>
#include <vector>
#include <iostream>
#include <math.h>

using namespace std;

#include <atmosphere/ATM/ATMRefractiveIndexProfile.h>

namespace atm {

  // Constructors 

  RefractiveIndexProfile::RefractiveIndexProfile( Frequency freq, AtmProfile atmProfile):
    AtmProfile(atmProfile), SpectralGrid(freq)
  {
    mkRefractiveIndexProfile();
  }

  RefractiveIndexProfile::RefractiveIndexProfile( AtmProfile atmProfile, Frequency freq):
    AtmProfile(atmProfile), SpectralGrid(freq)
  {
    mkRefractiveIndexProfile();
  }
  
  RefractiveIndexProfile::RefractiveIndexProfile( SpectralGrid spectralGrid, AtmProfile atmProfile):
    AtmProfile(atmProfile), SpectralGrid(spectralGrid)
  {
    mkRefractiveIndexProfile();
  }
  
  RefractiveIndexProfile::RefractiveIndexProfile( AtmProfile atmProfile, SpectralGrid spectralGrid):
    AtmProfile(atmProfile), SpectralGrid(spectralGrid)
  {
    mkRefractiveIndexProfile();
  }
  
  RefractiveIndexProfile::RefractiveIndexProfile( const RefractiveIndexProfile & a)
  {
    //   cout<<"Enter RefractiveIndexProfile copy constructor version Fri May 20 00:59:47 CEST 2005"<<endl;
    
    // level AtmProfile
    type_ = a.type_;
    prLimit_ = a.prLimit_;
    v_hx_.reserve(a.v_hx_.size());
    v_px_.reserve(a.v_px_.size());
    v_tx_.reserve(a.v_tx_.size());
    for(unsigned int n=0; n<a.v_hx_.size(); n++){
      v_hx_.push_back(a.v_hx_[n]);
      v_px_.push_back(a.v_px_[n]);
      v_tx_.push_back(a.v_tx_[n]);
    }
    
    groundTemperature_ = a.groundTemperature_;
    tropoLapseRate_    = a.tropoLapseRate_;
    groundPressure_    = a.groundPressure_;
    relativeHumidity_  = a.relativeHumidity_;
    wvScaleHeight_     = a.wvScaleHeight_;
    pressureStep_      = a.pressureStep_;
    pressureStepFactor_= a.pressureStepFactor_;
    altitude_          = a.altitude_;
    topAtmProfile_     = a.topAtmProfile_;
    numLayer_          = a.numLayer_;
    newBasicParam_     = a.newBasicParam_;
    v_layerThickness_.reserve(numLayer_);
    v_layerTemperature_.reserve(numLayer_);
    v_layerWaterVapor_.reserve(numLayer_);
    v_layerCO_.reserve(numLayer_);
    v_layerO3_.reserve(numLayer_);
    v_layerN2O_.reserve(numLayer_);
    
    for(unsigned int n=0; n<numLayer_; n++){
      v_layerThickness_.push_back(a.v_layerThickness_[n]);
      v_layerTemperature_.push_back(a.v_layerTemperature_[n]);
      //cout << "n=" << n << endl;
      v_layerWaterVapor_.push_back(a.v_layerWaterVapor_[n]);
      v_layerPressure_.push_back(a.v_layerPressure_[n]);
      v_layerCO_.push_back(a.v_layerCO_[n]);
      v_layerO3_.push_back(a.v_layerO3_[n]);
      v_layerN2O_.push_back(a.v_layerN2O_[n]);
    }
    
    // level Spectral Grid
    freqUnits_               = a.freqUnits_;
    v_chanFreq_              = a.v_chanFreq_;
    
    v_numChan_               = a.v_numChan_;
    v_refChan_               = a.v_refChan_; 
    v_refFreq_               = a.v_refFreq_; 
    v_chanSep_               = a.v_chanSep_; 
    v_maxFreq_               = a.v_maxFreq_; 
    v_minFreq_               = a.v_minFreq_; 
    v_intermediateFrequency_ = a.v_intermediateFrequency_;
    v_loFreq_                = a.v_loFreq_;
    
    v_sidebandSide_          = a.v_sidebandSide_;
    v_sidebandType_          = a.v_sidebandType_;
    
    vv_assocSpwId_           = a.vv_assocSpwId_;
    vv_assocNature_          = a.vv_assocNature_;
    
    v_transfertId_           = a.v_transfertId_;
    
    
    // level Absorption Profile  
    vv_N_H2OLinesPtr_.reserve(a.v_chanFreq_.size());
    vv_N_H2OContPtr_.reserve(a.v_chanFreq_.size());
    vv_N_O2LinesPtr_.reserve(a.v_chanFreq_.size());
    vv_N_DryContPtr_.reserve(a.v_chanFreq_.size());
    vv_N_O3LinesPtr_.reserve(a.v_chanFreq_.size());
    vv_N_COLinesPtr_.reserve(a.v_chanFreq_.size());
    vv_N_N2OLinesPtr_.reserve(a.v_chanFreq_.size());

    

    vector<complex<double> >* v_N_H2OLinesPtr;     
    vector<complex<double> >* v_N_H2OContPtr;      
    vector<complex<double> >* v_N_O2LinesPtr;      
    vector<complex<double> >* v_N_DryContPtr;      
    vector<complex<double> >* v_N_O3LinesPtr;      
    vector<complex<double> >* v_N_COLinesPtr;      
    vector<complex<double> >* v_N_N2OLinesPtr;     

    
    
    for(unsigned int nc=0; nc<v_chanFreq_.size(); nc++){
      
      v_N_H2OLinesPtr = new vector<complex<double> >;  v_N_H2OLinesPtr->reserve(numLayer_);
      v_N_H2OContPtr  = new vector<complex<double> >;  v_N_H2OContPtr->reserve(numLayer_);
      v_N_O2LinesPtr  = new vector<complex<double> >;  v_N_O2LinesPtr->reserve(numLayer_);
      v_N_DryContPtr  = new vector<complex<double> >;  v_N_DryContPtr->reserve(numLayer_);
      v_N_O3LinesPtr  = new vector<complex<double> >;  v_N_O3LinesPtr->reserve(numLayer_);
      v_N_COLinesPtr  = new vector<complex<double> >;  v_N_COLinesPtr->reserve(numLayer_);
      v_N_N2OLinesPtr = new vector<complex<double> >;  v_N_N2OLinesPtr->reserve(numLayer_);
      
      for(unsigned int n=0; n<numLayer_; n++){
	
	// cout << "numLayer_=" << nc << " " << n << endl; // COMMENTED OUT BY JUAN MAY/16/2005
	
	v_N_H2OLinesPtr->push_back(a.vv_N_H2OLinesPtr_[nc]->at(n));
	v_N_H2OContPtr->push_back(a.vv_N_H2OContPtr_[nc]->at(n));
	v_N_O2LinesPtr->push_back(a.vv_N_O2LinesPtr_[nc]->at(n));
	v_N_DryContPtr->push_back(a.vv_N_DryContPtr_[nc]->at(n));
	v_N_O3LinesPtr->push_back(a.vv_N_O3LinesPtr_[nc]->at(n));
	v_N_COLinesPtr->push_back(a.vv_N_COLinesPtr_[nc]->at(n));
	v_N_N2OLinesPtr->push_back(a.vv_N_N2OLinesPtr_[nc]->at(n));


      }
      
      
      vv_N_H2OLinesPtr_.push_back(v_N_H2OLinesPtr);
      vv_N_H2OContPtr_.push_back(v_N_H2OContPtr);    
      vv_N_O2LinesPtr_.push_back(v_N_O2LinesPtr);    
      vv_N_DryContPtr_.push_back(v_N_DryContPtr);    
      vv_N_O3LinesPtr_.push_back(v_N_O3LinesPtr);    
      vv_N_COLinesPtr_.push_back(v_N_COLinesPtr);    
      vv_N_N2OLinesPtr_.push_back(v_N_N2OLinesPtr);

    }
    
  }
  
  RefractiveIndexProfile::RefractiveIndexProfile(){ }
  
  RefractiveIndexProfile::~RefractiveIndexProfile()
  {
    void rmRefractiveIndexProfile();
  }
  
  void RefractiveIndexProfile::rmRefractiveIndexProfile(){
    // for every frequency channel delete the pointer to the absorption profile
    for(unsigned int nc=0; nc<v_chanFreq_.size(); nc++){
      delete vv_N_H2OLinesPtr_[nc];
      delete vv_N_H2OContPtr_[nc];
      delete vv_N_O2LinesPtr_[nc];
      delete vv_N_DryContPtr_[nc];
      delete vv_N_O3LinesPtr_[nc];
      delete vv_N_COLinesPtr_[nc];
      delete vv_N_N2OLinesPtr_[nc];
    }
  }
  
  
  bool RefractiveIndexProfile::updateRefractiveIndexProfile( Length altitude, Pressure    groundPressure, 
							     Temperature groundTemperature, double  tropoLapseRate, 
							     Humidity relativeHumidity, Length wvScaleHeight)
  {
    
    bool updated = false;
    bool mkNewAtmProfile = updateAtmProfile( altitude, groundPressure, 
					     groundTemperature, tropoLapseRate, 
					     relativeHumidity, wvScaleHeight);
    unsigned int  numLayer     = getNumLayer();
    
    
    if(vv_N_H2OLinesPtr_.size() < v_chanFreq_.size()){
      mkNewAtmProfile=true; 
      cout<<" RefractiveIndexProfile: number of spectral windows has increased"<<endl;
    }
    if(mkNewAtmProfile){
      if(numLayer){
	mkRefractiveIndexProfile();
	updated=true;
      }else{
	cout << " RefractiveIndexProfile: ERROR:  getNumLayer() returns 0" << endl;
      }
    }
    return updated;
  }
   
  
  
  
  // NB: this interface is required because the sub-class .... overrides this method.
  bool  RefractiveIndexProfile::setBasicAtmosphericParameters(Length altitude, Pressure    groundPressure, 
							      Temperature groundTemperature, double  tropoLapseRate, 
							      Humidity relativeHumidity, Length wvScaleHeight)
  {
    bool updated = updateRefractiveIndexProfile( altitude, groundPressure, 
						 groundTemperature, tropoLapseRate, 
						 relativeHumidity, wvScaleHeight);
    return updated;
  }
  
  void RefractiveIndexProfile::mkRefractiveIndexProfile(){
    

    static const double abun_18o=0.0020439;
    static const double abun_17o=0.0003750;
    static const double abun_D=0.000298444;
    static const double o2_mixing_ratio=0.2092;
    static const double mmol_h2o=18.005059688;  //   20*0.0020439+19*(0.0003750+2*0.000298444)+18*(1-0.0020439-0.0003750-2*0.000298444)
    static const double pi=3.141592654;

    static bool first=true;

    double abun_O3, abun_CO, abun_N2O;
    double wvt, wv, t;
    double nu, nu2, nu_pi;
    //    double sumAbsO3Lines1, sumAbsCOLines1, sumAbsN2OLines1;

    
    
    
    
    //TODO we will have to put numLayer_ and v_chanFreq_.size() const
    //we do not want to resize! ==> pas de setter pour SpectralGrid
    
    //cout << "vv_N_H2OLinesPtr_.size()=" << vv_N_H2OLinesPtr_.size() << endl;
    if(vv_N_H2OLinesPtr_.size()==0){                    // first time
      vv_N_H2OLinesPtr_.reserve(v_chanFreq_.size());
      vv_N_H2OContPtr_.reserve(v_chanFreq_.size());
      vv_N_O2LinesPtr_.reserve(v_chanFreq_.size());
      vv_N_DryContPtr_.reserve(v_chanFreq_.size());
      vv_N_O3LinesPtr_.reserve(v_chanFreq_.size());
      vv_N_COLinesPtr_.reserve(v_chanFreq_.size());
      vv_N_N2OLinesPtr_.reserve(v_chanFreq_.size());
    }else{
      if(vv_N_H2OLinesPtr_.size()==v_chanFreq_.size())    // there are new basic param
	rmRefractiveIndexProfile();  // delete all the layer profiles for all the frequencies
    }
    
    vector<complex<double> >* v_N_H2OLinesPtr;     
    vector<complex<double> >* v_N_H2OContPtr;      
    vector<complex<double> >* v_N_O2LinesPtr;      
    vector<complex<double> >* v_N_DryContPtr;      
    vector<complex<double> >* v_N_O3LinesPtr;      
    vector<complex<double> >* v_N_COLinesPtr;      
    vector<complex<double> >* v_N_N2OLinesPtr;     

    
    // cout << "v_chanFreq_.size()=" << v_chanFreq_.size() << endl;
    // cout << "numLayer_=" << numLayer_ << endl;
    // cout << "v_chanFreq_[0]=" << v_chanFreq_[0] << endl;
    // check if new spectral windows have been added
    unsigned int ncmin;
    /*  cout << "vv_N_H2OLinesPtr_.size()="<<vv_N_H2OLinesPtr_.size()<<endl; */
    ncmin=vv_N_H2OLinesPtr_.size();        // will be > 0 if spectral window(s) have been added
    if(newBasicParam_)ncmin=0;
    
    //    cout << "ncmin=" << ncmin << endl;

    for(unsigned int nc=ncmin; nc<v_chanFreq_.size(); nc++){
      
      
      v_N_H2OLinesPtr = new vector<complex<double> >;
      v_N_H2OContPtr  = new vector<complex<double> >;
      v_N_O2LinesPtr  = new vector<complex<double> >;
      v_N_DryContPtr  = new vector<complex<double> >;
      v_N_O3LinesPtr  = new vector<complex<double> >;
      v_N_COLinesPtr  = new vector<complex<double> >;
      v_N_N2OLinesPtr = new vector<complex<double> >;
      v_N_H2OLinesPtr->reserve(numLayer_);
      v_N_H2OContPtr->reserve(numLayer_);
      v_N_O2LinesPtr->reserve(numLayer_);
      v_N_DryContPtr->reserve(numLayer_);
      v_N_O3LinesPtr->reserve(numLayer_);
      v_N_COLinesPtr->reserve(numLayer_);
      v_N_N2OLinesPtr->reserve(numLayer_);
      
      nu    = 1.0E-9*v_chanFreq_[nc];    // ATM uses GHz units
      nu2   = nu*nu;
      nu_pi = nu/pi;
      
      for(unsigned int j=0; j<numLayer_; j++){
	

	wv  = v_layerWaterVapor_[j]*1000.0;       // se multiplica por 10**3 por cuestión de unidades en las rutinas fortran. 
	wvt = wv*v_layerTemperature_[j]/217.0;    // v_layerWaterVapor_[j] está en kg/m**3
	t = v_layerTemperature_[j]/300.0;


	complex<double>  o2lines = mkAbs_o2(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));  
	o2lines = o2lines*(1.0-2.0*(abun_18o+abun_17o))*o2_mixing_ratio*(1.0-exp(-1556.38*1.43/v_layerTemperature_[j]))*v_layerPressure_[j]*100.0/(1.380662e-23*v_layerTemperature_[j]);
	complex<double>  o2lines_vib = mkAbs_o2_vib(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));  
	o2lines_vib = o2lines_vib*(1.0-2.0*(abun_18o+abun_17o))*o2_mixing_ratio*exp(-1556.38*1.43/v_layerTemperature_[j])*v_layerPressure_[j]*100.0/(1.380662e-23*v_layerTemperature_[j]);
	complex<double>  o16o18lines = mkAbs_16o18o(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));  
	o16o18lines = o16o18lines*2.0*abun_18o*o2_mixing_ratio*v_layerPressure_[j]*100.0/(1.380662e-23*v_layerTemperature_[j]);
	complex<double>  o16o17lines = mkAbs_16o17o(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));  
	o16o17lines = o16o17lines*2.0*abun_17o*o2_mixing_ratio*v_layerPressure_[j]*100.0/(1.380662e-23*v_layerTemperature_[j]);

	
  	v_N_O2LinesPtr->push_back(o2lines+o16o17lines+o16o18lines+o2lines_vib);
	
	

       	complex<double>  cont_h2o = mkAbs_cnth2o(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));
     	complex<double>  cont_dry = mkAbs_cntdry(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));

	v_N_H2OContPtr->push_back(cont_h2o);
	v_N_DryContPtr->push_back(cont_dry);



	if(v_layerWaterVapor_[j]>0){
	  
	  complex<double>  h2olines = mkAbs_h2o(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));  
	  h2olines = h2olines*(6.023e23*v_layerWaterVapor_[j]*1000/mmol_h2o)*(1-abun_18o-abun_17o-2.0*abun_D)*(1.0-exp(-2322.92/v_layerTemperature_[j]));  // (m^2 * m^-3 = m -1)
	  complex<double>  hh18olines = mkAbs_hh18o(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));  
	  hh18olines = hh18olines*(6.023e23*v_layerWaterVapor_[j]*1000/mmol_h2o)*abun_18o;  // (m^2 * m^-3 = m -1)
	  complex<double>  hh17olines = mkAbs_hh17o(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));  
	  hh17olines = hh17olines*(6.023e23*v_layerWaterVapor_[j]*1000/mmol_h2o)*abun_17o;  // (m^2 * m^-3 = m -1)
	  // HDO LINES DOES NOT WORK PROPERLY 5/3/2007 
	  complex<double>  hdolines = mkAbs_hdo(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));  
	  hdolines = hdolines*(6.023e23*v_layerWaterVapor_[j]*1000.0/mmol_h2o)*2.0*abun_D;  // (m^2 * m^-3 = m -1)
	  complex<double>  h2olines_v2 = mkAbs_h2o_v2(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Pressure(wvt,"mb"),Frequency(nu,"GHz"));  
	  h2olines_v2 = h2olines_v2*(6.023e23*v_layerWaterVapor_[j]*1000/mmol_h2o)*(1-abun_18o-abun_17o-2.0*abun_D)*exp(-2322.92/v_layerTemperature_[j]);  // (m^2 * m^-3 = m -1)


	  v_N_H2OLinesPtr->push_back(h2olines+h2olines_v2+hh18olines+hh17olines);

	  
	}
	


	if(v_layerO3_[j]>0){
	  abun_O3=v_layerO3_[j]*1E-6;
	  v_N_O3LinesPtr->push_back(
	    (mkAbs_o3(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Frequency(nu,"GHz"))+
	     mkAbs_o3_161617(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Frequency(nu,"GHz"))*(2*abun_17o)+
	     mkAbs_o3_161618(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Frequency(nu,"GHz"))*(2*abun_18o)+
	     mkAbs_o3_161716(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Frequency(nu,"GHz"))*(abun_17o)+
	      mkAbs_o3_161816(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Frequency(nu,"GHz"))*(abun_18o))
	     *abun_O3*1e6/(1.0+3.0*(abun_18o+abun_17o)));  // m^-1

	}

	
	if(v_layerCO_[j]>0){
	  abun_CO=v_layerCO_[j]*1E-6;  // in cm^-3
	  v_N_COLinesPtr->push_back(mkAbs_co(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Frequency(nu,"GHz"))*abun_CO*1e6);  // m^2 * m^-3 = m^-1
	}
	

	if(v_layerN2O_[j]>0){        
	  abun_N2O=v_layerN2O_[j]*1E-6;
	  v_N_N2OLinesPtr->push_back(mkAbs_n2o(Temperature(v_layerTemperature_[j],"K"),Pressure(v_layerPressure_[j],"mb"),Frequency(nu,"GHz"))*abun_N2O*1e6);  // m^2 * m^-3 = m^-1
	}


      }
      
      
      
      if(vv_N_H2OLinesPtr_.size()==0)first=true;
      


      if(vv_N_H2OLinesPtr_.size()<v_chanFreq_.size()){
	vv_N_H2OLinesPtr_.push_back(v_N_H2OLinesPtr); 
	vv_N_H2OContPtr_.push_back(v_N_H2OContPtr);
	vv_N_O2LinesPtr_.push_back(v_N_O2LinesPtr);
	vv_N_DryContPtr_.push_back(v_N_DryContPtr);
	vv_N_O3LinesPtr_.push_back(v_N_O3LinesPtr);
	vv_N_COLinesPtr_.push_back(v_N_COLinesPtr);
	vv_N_N2OLinesPtr_.push_back(v_N_N2OLinesPtr);
      }else{
	vv_N_H2OLinesPtr_[nc] = v_N_H2OLinesPtr;
	vv_N_H2OContPtr_[nc] = v_N_H2OContPtr;
	vv_N_O2LinesPtr_[nc] = v_N_O2LinesPtr;
	vv_N_DryContPtr_[nc] = v_N_DryContPtr;
	vv_N_O3LinesPtr_[nc] = v_N_O3LinesPtr;
	vv_N_COLinesPtr_[nc] = v_N_COLinesPtr;
	vv_N_N2OLinesPtr_[nc] = v_N_N2OLinesPtr;
      }
      
    }
    
    newBasicParam_     = false;
    first = false;
  }
  
  Opacity RefractiveIndexProfile::getDryOpacity(){
    return getDryOpacity(0);
  }
  
  Opacity RefractiveIndexProfile::getDryOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
       kv = kv + imag( vv_N_O2LinesPtr_[nc]->at(j) + vv_N_DryContPtr_[nc]->at(j) + 
		  vv_N_O3LinesPtr_[nc]->at(j) + vv_N_COLinesPtr_[nc]->at(j) + 
		  vv_N_N2OLinesPtr_[nc]->at(j) )* v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getDryOpacity(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getDryOpacity( v_transfertId_[spwid]+nc);
  }
  

  Opacity RefractiveIndexProfile::getAverageDryOpacity(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0))return Opacity(-999.0);
    Opacity totalaverage;
    totalaverage=Opacity(0.0,"np");
    for(unsigned int nc=0; nc<getNumChan(spwid); nc++){
      totalaverage=totalaverage+getDryOpacity(spwid,nc);
    }
    totalaverage=totalaverage/getNumChan(spwid);
    return totalaverage;}



  
  Opacity RefractiveIndexProfile::getDryContOpacity(){
    return getDryContOpacity(0);
  }
  
  Opacity RefractiveIndexProfile::getDryContOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_DryContPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  
  Opacity RefractiveIndexProfile::getO2LinesOpacity(){
    return getO2LinesOpacity(0);
  }
  
  Opacity RefractiveIndexProfile::getO2LinesOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_O2LinesPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getCOLinesOpacity(){
    return getCOLinesOpacity(0); 
  }
  
  Opacity RefractiveIndexProfile::getCOLinesOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_COLinesPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getN2OLinesOpacity(){
    return getN2OLinesOpacity(0); 
  }
  
  Opacity RefractiveIndexProfile::getN2OLinesOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_N2OLinesPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getO3LinesOpacity(){
    return getO3LinesOpacity(0); 
  }
  
  Opacity RefractiveIndexProfile::getO3LinesOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_O3LinesPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getWetOpacity(){
    return getWetOpacity(0);
  }
  
  
  Opacity RefractiveIndexProfile::getWetOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    /*  cout<<"nc="<<nc<<endl; */
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_H2OLinesPtr_[nc]->at(j) + vv_N_H2OContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 



    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getWetOpacity(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getWetOpacity( v_transfertId_[spwid]+nc);
  }
  
  Opacity RefractiveIndexProfile::getAverageWetOpacity(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0))return Opacity(-999.0);
    Opacity totalaverage;
    totalaverage=Opacity(0.0,"np");
    for(unsigned int nc=0; nc<getNumChan(spwid); nc++){
      totalaverage=totalaverage+getWetOpacity(spwid,nc);
    }
    totalaverage=totalaverage/getNumChan(spwid);
    return totalaverage;}



  Opacity RefractiveIndexProfile::getH2OLinesOpacity(){
    return getH2OLinesOpacity(0);
  }
  
  
  Opacity RefractiveIndexProfile::getH2OLinesOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      /*    cout <<"j="<<j<<" abs H2O Lines ="<<vv_N_H2OLinesPtr_[nc]->at(j) <<endl; */
      kv = kv + imag( vv_N_H2OLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getH2OLinesOpacity( unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getH2OLinesOpacity( v_transfertId_[spwid]+nc);
  }
  

  Opacity RefractiveIndexProfile::getAverageH2OLinesOpacity(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0))return Opacity(-999.0);
    Opacity totalaverage;
    totalaverage=Opacity(0.0,"np");
    for(unsigned int nc=0; nc<getNumChan(spwid); nc++){
      totalaverage=totalaverage+getH2OLinesOpacity(spwid,nc);
    }
    totalaverage=totalaverage/getNumChan(spwid);
    return totalaverage;}



  Opacity RefractiveIndexProfile::getH2OContOpacity(){
    return getH2OContOpacity(0);
  }
  
  
  Opacity RefractiveIndexProfile::getH2OContOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_H2OContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getH2OContOpacity( unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getH2OContOpacity( v_transfertId_[spwid]+nc);
  }
  
  
  Opacity RefractiveIndexProfile::getAverageH2OContOpacity(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0))return Opacity(-999.0);
    Opacity totalaverage;
    totalaverage=Opacity(0.0,"np");
    for(unsigned int nc=0; nc<getNumChan(spwid); nc++){
      totalaverage=totalaverage+getH2OLinesOpacity(spwid,nc);
    }
    totalaverage=totalaverage/getNumChan(spwid);
    return totalaverage;}



  Angle RefractiveIndexProfile::getDispersiveWetPhaseDelay(){
    return getDispersiveWetPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getDispersiveWetPathLength(){
    return getDispersiveWetPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getDispersiveWetPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_H2OLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getDispersiveWetPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getDispersiveWetPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getDispersiveWetPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getDispersiveWetPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageDispersiveWetPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getDispersiveWetPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getDispersiveWetPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getDispersiveWetPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageDispersiveWetPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getDispersiveWetPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }
  
  

  
  Angle RefractiveIndexProfile::getNonDispersiveDryPhaseDelay(){
    return getNonDispersiveDryPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getNonDispersiveDryPathLength(){
    return getNonDispersiveDryPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getNonDispersiveDryPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_DryContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getNonDispersiveDryPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getNonDispersiveDryPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getNonDispersiveDryPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getNonDispersiveDryPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageNonDispersiveDryPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersiveDryPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getNonDispersiveDryPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getNonDispersiveDryPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageNonDispersiveDryPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersiveDryPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }



  Angle RefractiveIndexProfile::getO2LinesPhaseDelay(){
    return getO2LinesPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getO2LinesPathLength(){
    return getO2LinesPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getO2LinesPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_O2LinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getO2LinesPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getO2LinesPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getO2LinesPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getO2LinesPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageO2LinesPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getO2LinesPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getO2LinesPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getO2LinesPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageO2LinesPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getO2LinesPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }










  Angle RefractiveIndexProfile::getO3LinesPhaseDelay(){
    return getO3LinesPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getO3LinesPathLength(){
    return getO3LinesPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getO3LinesPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_O3LinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getO3LinesPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getO3LinesPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getO3LinesPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getO3LinesPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageO3LinesPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getO3LinesPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getO3LinesPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getO3LinesPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageO3LinesPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getO3LinesPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }



  Angle RefractiveIndexProfile::getCOLinesPhaseDelay(){
    return getCOLinesPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getCOLinesPathLength(){
    return getCOLinesPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getCOLinesPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_COLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getCOLinesPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getCOLinesPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getCOLinesPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getCOLinesPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageCOLinesPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getCOLinesPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getCOLinesPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getCOLinesPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageCOLinesPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getCOLinesPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }





  Angle RefractiveIndexProfile::getN2OLinesPhaseDelay(){
    return getN2OLinesPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getN2OLinesPathLength(){
    return getN2OLinesPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getN2OLinesPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_N2OLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getN2OLinesPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getN2OLinesPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getN2OLinesPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getN2OLinesPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageN2OLinesPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getN2OLinesPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getN2OLinesPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getN2OLinesPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageN2OLinesPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getN2OLinesPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }



  Angle RefractiveIndexProfile::getNonDispersiveWetPhaseDelay(){
    return getNonDispersiveWetPhaseDelay(0); 
  }
  
  Length RefractiveIndexProfile::getNonDispersiveWetPathLength(){
    return getNonDispersiveWetPathLength(0); 
  }
  
  Angle RefractiveIndexProfile::getNonDispersiveWetPhaseDelay( unsigned int nc){
    double kv = 0;
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_H2OContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getNonDispersiveWetPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getNonDispersiveWetPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
  
  Angle RefractiveIndexProfile::getNonDispersiveWetPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getNonDispersiveWetPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageNonDispersiveWetPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersiveWetPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getNonDispersiveWetPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length(-999.0);}
    return getNonDispersiveWetPathLength( v_transfertId_[spwid]+nc);
  }
  
  Length RefractiveIndexProfile::getAverageNonDispersiveWetPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length(-999.0);}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersiveWetPathLength( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Length average(av,"deg");
    return average;
  }
  
  
  
  
  // NB: the function chanIndexIsValid will be overrided by ....
  bool RefractiveIndexProfile::chanIndexIsValid(unsigned int nc){
    if( nc < vv_N_H2OLinesPtr_.size() )return true;
    if( nc < v_chanFreq_.size() ){
      cout << " RefractiveIndexProfile: Requested index in a new spectral window ==> update profile" << endl;
      mkRefractiveIndexProfile();
      // cout << "...and we return" << endl;
      return true;
    }
    cout << " RefractiveIndexProfile: ERROR: Invalid channel frequency index" << endl;
    return false;
  }
  
  // NB: the function spwidAndIndexAreValid will be overrided by ...
  bool RefractiveIndexProfile::spwidAndIndexAreValid(unsigned int spwid, unsigned int idx){

    if(spwid>getNumSpectralWindow()-1 || spwid<0 ){
      cout << " RefractiveIndexProfile: ERROR: spectral window identifier out of range " << endl;
      return false;
    }
    if(idx>getNumChan(spwid)-1 || idx<0){
      cout << " RefractiveIndexProfile: ERROR: channel index out of range " << endl;
      return false;
    }
    unsigned int  nc = v_transfertId_[spwid]+idx;
    bool valid=chanIndexIsValid(nc);
    return valid;
  }
  
  void RefractiveIndexProfile::updateNewSpectralWindows(){
    mkRefractiveIndexProfile();
  }
  

  


  Frequency RefractiveIndexProfile::linebroadening(Frequency nu, Temperature tt, Pressure pp, double mmol, Frequency dv0_lines, double texp_lines){

    double pr=pp.get("mb");
    double temp=tt.get("K");
    double dv0;
    double dv;
    
    dv0=dv0_lines.get("GHz")*pr*pow((300/temp),texp_lines); // EN GHz
    
    double beta_dop=4.30e-7*nu.get("GHz")*pow((temp/mmol),0.5);
    if((dv0/beta_dop)<1.25){
      dv=0.535*dv0+pow((0.217*pow(dv0,2)+0.6931*pow(beta_dop,2)),0.5);   // "Atmospheric Remote Sensing", Janssen, pag. 59
    }else{
      dv=dv0;
    }
    return Frequency(dv,"GHz");
  }


  Frequency RefractiveIndexProfile::linebroadening_o2(Frequency nu, Temperature tt, Pressure pp, Pressure ph2o, double mmol, double ensanche1,double ensanche2){
    
    double pr=pp.get("mb");
    double eh2o=ph2o.get("mb");
    double temp=tt.get("K");
    double dv0;
    double dv;

    dv0=1e-3*ensanche1*((pr-eh2o)*pow(300/temp,ensanche2)+1.1*eh2o*300/temp);             // EN GHz

    double beta_dop=4.30e-7*nu.get("GHz")*pow((temp/mmol),0.5);
    
    if((dv0/beta_dop)<1.25){
      dv=0.535*dv0+pow((0.217*pow(dv0,2)+0.6931*pow(beta_dop,2)),0.5);   // "Atmospheric Remote Sensing", Janssen, pag. 59
      //      cout << pp.get("mb") << "mb: usando beta_dop" << endl;
    }else{
      dv=dv0;
    }
    
    return Frequency(dv,"GHz");

  }


  Frequency RefractiveIndexProfile::interf_o2(Temperature tt, Pressure pp, double ensanche3,double ensanche4){

    double temp=tt.get("K");

    double interf=1e-3*(ensanche3+ensanche4*(300/temp))*pp.get("mb")*pow((300/temp),0.8);
    return Frequency(interf,"GHz");

  }

  Frequency RefractiveIndexProfile::linebroadening_water(Frequency nu, Temperature tt, Pressure pp, Pressure ph2o, double ensanche1,double ensanche2,double ensanche3,double ensanche4){
    static const double mmol=18.0;
    double pr=pp.get("mb");
    double eh2o=ph2o.get("mb");
    double temp=tt.get("K");
    double dv0;
    double dv;
    

    if(ensanche2>0){
      dv0=1e-3*ensanche1*((pr-eh2o)*pow(300/temp,ensanche3)+ensanche2*eh2o*pow(300.0/temp,ensanche4));             // EN GHz
    }else{
      dv0=1e-3*ensanche1*((pr-eh2o)*pow(300/temp,0.68)+4.50*eh2o*pow(300.0/temp,0.80));             // EN GHz      
    }

    double beta_dop=3.58e-7*nu.get("GHz")*pow((temp/mmol),0.5);

    if((dv0/beta_dop)<1.25){
      dv=0.535*dv0+pow((0.217*pow(dv0,2)+0.6931*pow(beta_dop,2)),0.5);   // "Atmospheric Remote Sensing", Janssen, pag. 59
      //      cout << pp.get("mb") << "mb: usando beta_dop" << endl;
    }else{
      dv=dv0;
    }

    //    cout << nu.get("GHz") << "  " << pr << "  " <<  eh2o << "  " << dv << endl; 

    return Frequency(dv,"GHz");
  }

  Frequency RefractiveIndexProfile::linebroadening_hh18o_hh17o(Frequency nu, Temperature tt, Pressure pp, Pressure ph2o, double dv0, double dvlm, double temp_exp){
    double pr=pp.get("mb");
    double eh2o=ph2o.get("mb");
    double temp=tt.get("K");
    double dv;
    double rho=18.0*eh2o*100/(8.315727226*temp);   // Na*Kb=8.315727226
    double c2=4.6E-03*rho*temp/pr;

    dv=(dv0*(pr/1013.0)/(pow((temp/300.0),temp_exp)))*(1.+c2*(dvlm/dv0-1.));

    return Frequency(dv,"GHz");
  }

  complex<double>  RefractiveIndexProfile::lineshape(Frequency nu, Frequency linefreq, Frequency linebroad, Frequency interf){

    double dv=linebroad.get("GHz");
    double itf=interf.get("GHz");
    double vl=linefreq.get("GHz");
    double v=nu.get("GHz");
    double lf=dv*itf;

    double dv2=dv*dv;
    double dv2v2=dv2+v*v;
    double vl2=vl*vl;
    double vvl=2*v*vl;
    double aa=dv2v2+vl2;
    double a1=aa-vvl;
    double a2=aa+vvl;
    
    double fv=((dv-(vl-v)*itf)/a1+(dv-(vl+v)*itf)/a2);	        //   ! line profile (imaginary) 
                                                                //   !          in 1/frec units
    double frv=((vl-v+lf)/a1-(vl+v+lf)/a2);                     //   ! delay profile (real part)
    
    return complex<double> (frv,fv)*(v/vl);                              // GHz^-1*GHz (no units)

  }

  
  complex<double>  RefractiveIndexProfile::mkAbs_n2o(Temperature tt, Pressure pp, Frequency nu){

    static const double fre[39]={
      25.123248, 50.246371, 75.369242,100.491727,125.613711,
      150.735063,175.855641,200.975344,226.094031,251.211578,
      276.327844,301.442750,326.556094,351.667813,376.777750,
      401.885813,426.991813,452.095656,477.197250,502.296438,
      527.393063,552.487063,577.578250,602.666500,627.751750,
      652.833813,677.912563,702.987938,728.059750,753.127875,
      778.192188,803.252625,828.309000,853.361188,878.409063,
      903.452500,928.491375,953.525563,978.554938};

    static const double el[39]={
      0.0,   1.2,   3.6,   7.3,  12.1,
      18.1,  25.4,  33.7,  43.4,  54.3,  
      66.3,  79.6,  94.0, 109.7, 126.6, 
      144.7, 164.0, 184.5, 206.2, 229.1, 
      253.2, 278.5, 305.0, 332.7, 361.7, 
      391.8, 423.1, 455.7, 489.4, 524.3, 
      560.5, 597.8, 636.4, 676.1, 717.1, 
      759.3, 802.6, 847.2, 893.0};

    static const double flin[39]={
      0.100E+01,0.200E+01,0.300E+01,0.400E+01,0.500E+01,
      0.600E+01,0.700E+01,0.800E+01,0.900E+01,0.100E+02,
      0.110E+02,0.120E+02,0.130E+02,0.140E+02,0.150E+02,
      0.160E+02,0.170E+02,0.180E+02,0.190E+02,0.200E+02,
      0.210E+02,0.220E+02,0.230E+02,0.240E+02,0.250E+02,
      0.260E+02,0.270E+02,0.280E+02,0.290E+02,0.300E+02,
      0.310E+02,0.320E+02,0.330E+02,0.340E+02,0.350E+02,
      0.360E+02,0.370E+02,0.380E+02,0.390E+02};

    static const int ifin1[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   8,   8,   8,   8,
      8,   8,   8,   8,   8,   8,   8,   8,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  20,  20,  20,
      20,  20,  20,  20,  20,  20,  20,  20,  20,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  25,
      25,  25,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  26,  26,  26,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  27,  27,  27,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  27,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  32,  32,  32,
      32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      33,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  35,  35,  35,  35,  35,
      35,  35,  35,  35,  35,  35,  35,  36,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini1[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   6,   6,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
      8,   8,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  20,  20,  20,  20,  20,  20,  20,  20,  20,
      20,  20,  20,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  25,  25,  25,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  26,  26,  27,  27,
      27,  27,  27,  27,  27,  27,  27,  27,  27,  27,
      27,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32,  32,  32,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  35,
      35,  35,  35,  35,  35,  35,  35,  35,  35,  35,
      35,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  36,  36,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ifin2[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   8,   8,   8,
      8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  20,  20,
      20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  25,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  26,  26,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  27,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  27,  27,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  32,  32,
      32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  35,  35,  35,  35,
      35,  35,  35,  35,  35,  35,  35,  35,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini2[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   6,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   8,
      8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
      8,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  20,
      20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
      20,  20,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  25,  25,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  26,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  27,  27,  27,  27,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  29,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  35,  35,  35,
      35,  35,  35,  35,  35,  35,  35,  35,  35,  35,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ifin3[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   8,   8,
      8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  20,  20,
      20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
      20,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  25,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  25,  26,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  27,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  27,  27,  27,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  32,
      32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  35,  35,  35,  35,
      35,  35,  35,  35,  35,  35,  35,  35,  35,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini3[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   6,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   8,   8,
      8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
      8,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  20,
      20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
      20,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  25,  26,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  26,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  27,  27,  27,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  32,
      32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  35,  35,  35,
      35,  35,  35,  35,  35,  35,  35,  35,  35,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};


    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.161;  //Debyes
    static const double mmol=44.0;

    double q=1.65873970*tt.get("K");
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
            
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
        
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      if(ifin==0||ifin<ini){

	return complex<double> (0.0,0.0);

      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){

	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening(Frequency(fre[i],"GHz"),tt,pp,mmol,Frequency(2.5,"MHz"),0.76),Frequency(0.0,"GHz"));

	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*fre[i]; 

          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(0.047992745509/tt.get("K"))*(fac2fixed*pow(mu,2.0)/q);  // imaginary part: absorption coefficient in cm^2
	                                                                                                  // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )

	
      }
     
    }
 
  }


  complex<double>  RefractiveIndexProfile::mkAbs_co(Temperature tt, Pressure pp, Frequency nu){

    static const double fre[8] = {115.271203,230.538000,345.796000,461.04075,576.267938,691.47300,806.651688,921.799563};
    static const double flin[8] = {0.100E+01,0.200E+01,0.300E+01,0.400E+01,0.500E+01,0.600E+01,0.700E+01,0.800E+01};
    static const double el[8] = {0.0, 5.5,  16.6,  33.2,  55.3,83.0, 116.2, 154.9};

    static const int ifin1[500] ={1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini1[500]={  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};


    static const int ifin2[500]={  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini2[500]={  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ifin3[500]={ 1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini3[500]={  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
			   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
			   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
			   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
			   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
			   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
			   6,   6,   6,   6,   6,   6,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
			   7,   7,   7,   7,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
			   8,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
			   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};


    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.112;  //Debyes
    static const double mmol=28.0;

    double q=0.3615187262*tt.get("K");
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
            
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
        
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      if(ifin==0||ifin<ini){

	return complex<double> (0.0,0.0);

      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){

	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening(Frequency(fre[i],"GHz"),tt,pp,mmol,Frequency(2.5,"MHz"),0.76),Frequency(0.0,"GHz"));

	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*fre[i]; 

          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(0.047992745509/tt.get("K"))*(fac2fixed*pow(mu,2.0)/q);  // imaginary part: absorption coefficient in cm^2
	                                                                                                  // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )

	
      }
     
    }
 
  }

  complex<double>  RefractiveIndexProfile::mkAbs_cnth2o(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){

    //     cnth2o, empirical

    
    double t300=300/tt.get("K");
    double eee=eh2o.get("mb")/1013.0;
    double ppp=(pp.get("mb")-eh2o.get("mb"))/1013.0;
      
    double cnth2o=0.0315*pow((nu.get("GHz")/225),2)*(eee*ppp)*pow(t300,3);
    
    double delayh2o=(4.163*t300+0.239)*eh2o.get("mb")*t300*nu.get("GHz")*1.2008e-3/57.29578;
		     //		     eh2o.get("mb")*pow(t300,2.5)*.791e-6*pow(nu.get("GHz"),2)*nu.get("GHz")*1.2008e-3/57.29578;

    return complex<double> (delayh2o,cnth2o);       // (  rad m^-1 , m^-1 ) 

  }

  complex<double>  RefractiveIndexProfile::mkAbs_cntdry(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){

    //     cntdry, empirical

    double t300=300/tt.get("K");
    double ppp=(pp.get("mb")-eh2o.get("mb"));
    double f=nu.get("GHz");
    double gamma0=5.6E-04*pp.get("mb")*pow(t300,0.8);


    double nsec1=6.14e-5*ppp*pow(t300,2)*f*gamma0/(pow(f,2)+pow(gamma0,2));
    double nsec2=1.4E-12*pow(ppp,2)*pow(t300,3.5)*f;   //(1+1.9e-5*pow(f,1.5));


    double cntair=(nsec1+0.85633*nsec2)*f*0.1820/4.34e3;   // From fitting FTS data, JQSRT, Pardo et al. 2001
      
    double delaydry=(0.2588*ppp*t300-6.14e-5*ppp*pow(t300,2)*pow(f,2)/(pow(f,2)+pow(gamma0,2)))*f*1.2008e-3/57.29578;

    return complex<double> (delaydry,cntair);     // (  rad m^-1 , m^-1 ) 

  }

      

  complex<double>  RefractiveIndexProfile::mkAbs_h2o_v2(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){

    static const double fre[335]={
      67.790245,   96.738518,  120.066833,  209.379730,  232.941650,
      272.749756,  273.308319,  294.911926,  298.692719,  336.318604,
      424.615814,  429.827271,  432.365814,  439.689758,  462.861206,
      498.342834,  546.492249,  577.889099,  593.817871,  594.784973,
      657.968628,  774.061768,  859.918091,  899.371277,  902.622192,
      902.633484,  923.067688,  925.892395,  968.111328, 1077.804199,
      1132.792358, 1141.902466, 1148.058960, 1195.768677, 1205.750488,
      1214.630005, 1214.944824, 1222.725098, 1406.612549, 1421.820068,
      1428.340332, 1473.516479, 1494.088989, 1591.912231, 1637.113037,
      1644.029663, 1647.032227, 1699.797241, 1739.387451, 1740.511841,
      1753.855347, 1835.131104, 1849.346802, 1862.525635, 1864.789307,
      1894.290527, 1933.400635, 1946.366089, 1956.037354, 1991.469849,
      2086.665283, 2091.183838, 2106.667236, 2140.248047, 2218.246338,
      2227.581299, 2233.940186, 2247.307861, 2294.118164, 2337.540771,
      2372.200684, 2401.381836, 2484.194580, 2488.897461, 2501.917480,
      2503.204346, 2541.946533, 2561.416992, 2585.798584, 2590.757080,
      2646.906982, 2689.032715, 2742.224365, 2783.789062, 2808.305664,
      2820.776855, 2842.404785, 2852.419434, 2902.184814, 2973.164551,
      2998.129883, 3024.840576, 3036.465576, 3037.561279, 3051.924316,
      3065.634766, 3112.744141, 3132.590088, 3142.544922, 3159.035889,
      3161.792969, 3216.594727, 3253.378174, 3275.678467, 3310.530273,
      3328.204102, 3348.107178, 3396.036621, 3425.261475, 3474.651367,
      3495.020508, 3553.504395, 3566.186523, 3601.603516, 3624.354004,
      3631.654297, 3638.812988, 3639.941162, 3656.975342, 3661.215088,
      3708.865479, 3719.691895, 3741.291260, 3756.302002, 3781.373779,
      3797.620850, 3805.620850, 3870.276611, 3883.985840, 3907.251221,
      4007.915527, 4032.301270, 4048.079346, 4065.159668, 4120.685547,
      4133.917969, 4176.322754, 4196.534668, 4253.582520, 4265.271973,
      4265.930664, 4272.971680, 4450.476074, 4450.689453, 4457.209961,
      4520.783691, 4522.206543, 4550.888672, 4557.868164, 4574.724121,
      4583.926270, 4608.368164, 4625.347656, 4630.292480, 4631.189941,
      4632.965332, 4645.110352, 4655.048828, 4673.143555, 4681.702148,
      4688.050781, 4707.438477, 4715.640625, 4721.811035, 4758.667480,
      4809.098145, 4890.160645, 4915.819824, 4919.379883, 4956.141602,
      4991.813965, 5045.965820, 5085.471680, 5106.055664, 5151.291992,
      5164.418457, 5197.080566, 5234.995605, 5243.801758, 5264.706055,
      5270.681152, 5326.873535, 5343.042969, 5347.410645, 5348.099121,
      5364.738770, 5370.276367, 5397.082520, 5427.933105, 5430.001465,
      5430.668945, 5435.439941, 5439.126465, 5441.707031, 5441.996582,
      5442.362305, 5452.062012, 5483.936523, 5498.748047, 5550.827148,
      5558.973633, 5560.731934, 5602.951660, 5643.037598, 5760.064453,
      5777.882324, 5781.895508, 5783.031738, 5783.865723, 5822.762207,
      5937.895508, 5973.528809, 5974.892090, 6002.918457, 6046.645996,
      6101.024414, 6129.086914, 6140.286621, 6140.372559, 6159.938965,
      6160.485352, 6166.656738, 6173.993652, 6176.440918, 6178.348145,
      6179.314941, 6186.974609, 6192.049805, 6193.727051, 6318.125977,
      6319.972656, 6328.667969, 6372.734863, 6486.106445, 6490.986816,
      6505.577148, 6527.715820, 6592.956543, 6611.653809, 6655.116699,
      6660.624023, 6669.819336, 6691.077637, 6764.869629, 6783.576172,
      6783.586914, 6814.936523, 6815.015137, 6845.112305, 6845.504395,
      6856.315918, 6857.177734, 6857.993652, 6872.185059, 6873.719238,
      6936.190918, 6966.458496, 6993.731445, 7001.683594, 7122.857910,
      7240.887207, 7250.316406, 7260.501465, 7269.827148, 7312.422852,
      7352.959961, 7363.332031, 7368.429199, 7392.671875, 7393.081055,
      7406.447266, 7432.064941, 7432.066406, 7472.024414, 7472.035645,
      7485.548828, 7514.537598, 7530.646973, 7624.821289, 7747.829102,
      7884.774902, 7889.549805, 7927.257324, 7927.455078, 7929.243652,
      7941.633789, 8027.130859, 8031.218262, 8041.744141, 8177.214844,
      8181.974609, 8289.856445, 8290.503906, 8323.575195, 8455.500977,
      8460.076172, 8460.172852, 8526.576172, 8529.013672, 8538.893555,
      8564.284180, 8571.694336, 8601.731445, 8645.142578, 8759.967773,
      8991.116211, 8991.165039, 9006.700195, 9023.595703, 9029.281250,
      9041.551758, 9048.492188, 9098.749023, 9102.541016, 9111.836914,
      9167.686523, 9168.944336, 9233.576172, 9262.232422, 9262.603516,
      9369.607422, 9520.371094, 9520.396484, 9589.912109, 9631.456055,
      9633.417969, 9663.334961, 9729.836914, 9739.676758, 9748.269531,
      9766.947266, 9828.917969, 9828.990234, 9890.300781, 9945.291992};

    static const double flin[335]={
      .1440E+00,  .1170E+00,  .1090E+00,  .1000E+00,  .1000E+00,
      .7910E-01,  .7910E-01,  .8790E-01,  .8790E-01,  .7650E-01,
      .2300E+00,  .2110E+00,  .2110E+00,  .2310E+00,  .1560E+00,
      .2550E+00,  .2760E+00,  .2570E+00,  .1860E+00,  .2820E+00,
      .1500E+01,  .9220E-01,  .2120E+01,  .7260E+00,  .2820E+00,
      .4930E-01,  .3540E+00,  .3000E+00,  .2640E+00,  .3390E+00,
      .3800E+00,  .3800E+00,  .4080E+00,  .4090E+00,  .1000E+01,
      .2310E+01,  .4390E+00,  .4500E+00,  .2450E+01,  .3600E+01,
      .4500E+00,  .4100E+00,  .1210E+01,  .4340E+01,  .7090E-01,
      .1670E+01,  .3370E-01,  .3300E+00,  .2210E+01,  .5750E+00,
      .1500E+01,  .5740E+00,  .7680E+00,  .5750E+00,  .6050E+00,
      .6260E+00,  .5300E+00,  .4540E+01,  .8330E+00,  .6120E+00,
      .4460E+00,  .6150E+00,  .5820E+01,  .4570E+01,  .5590E+00,
      .1310E+01,  .2140E+01,  .6610E+01,  .3240E+01,  .2740E+01,
      .6700E+00,  .2070E+01,  .4380E+01,  .2070E+01,  .2500E-01,
      .7890E+00,  .7830E+00,  .8080E+00,  .6820E+01,  .1610E+01,
      .1060E+01,  .2960E+01,  .7990E+01,  .1490E+01,  .1020E+01,
      .6660E+01,  .8860E+01,  .8320E+00,  .1830E+01,  .3820E+01,
      .8920E+00,  .5240E+01,  .8300E+00,  .1780E+01,  .1500E+01,
      .2480E+01,  .6620E+01,  .2000E+01,  .9130E+01,  .3910E+01,
      .4150E+01,  .9990E+00,  .1270E+01,  .3990E+01,  .2990E+01,
      .9050E+00,  .1010E+02,  .2010E-01,  .9270E+00,  .8660E+01,
      .2950E+01,  .1880E+01,  .4880E+01,  .1670E+01,  .1330E+01,
      .8940E+01,  .2010E+01,  .3350E+01,  .4910E+01,  .2440E+01,
      .1070E+01,  .7190E+01,  .1060E+01,  .1980E+01,  .6320E+01,
      .2810E+01,  .1070E+01,  .1980E+01,  .3570E+01,  .4010E+01,
      .5880E+01,  .4220E+01,  .3590E+01,  .1960E+01,  .1930E+01,
      .5900E+01,  .5920E+01,  .1240E+01,  .4750E+01,  .9950E+00,
      .4800E+01,  .1710E-01,  .3850E+01,  .3530E+01,  .2400E+01,
      .6100E+01,  .3740E+01,  .5150E+01,  .2950E+01,  .1250E+01,
      .1980E+01,  .2050E+01,  .1080E+01,  .1080E+01,  .2040E+01,
      .2930E+01,  .3780E+01,  .3960E+01,  .2090E+01,  .4580E+01,
      .6920E+01,  .6920E+01,  .7710E+01,  .1970E+01,  .5320E+01,
      .3020E+01,  .5970E+01,  .2410E+01,  .5430E+01,  .2380E+01,
      .6610E+01,  .3840E+01,  .6520E+01,  .1510E-01,  .4640E+01,
      .3840E+01,  .5650E+01,  .7920E+01,  .7920E+01,  .5990E+01,
      .2000E+01,  .4760E+01,  .1990E+01,  .6940E+01,  .5620E+01,
      .1820E+01,  .3960E+01,  .3880E+01,  .3000E+01,  .1100E+01,
      .1100E+01,  .2080E+01,  .2080E+01,  .3870E+01,  .4720E+01,
      .3000E+01,  .5540E+01,  .6320E+01,  .6930E+00,  .7050E+01,
      .4790E+01,  .2350E+01,  .3900E+01,  .3070E+01,  .2230E+01,
      .8920E+01,  .8920E+01,  .1680E+01,  .5720E+01,  .5740E+01,
      .2020E+01,  .5960E+01,  .2010E+01,  .5790E+01,  .3980E+01,
      .2360E+01,  .5650E+01,  .1120E+01,  .1120E+01,  .2120E+01,
      .2120E+01,  .4800E+01,  .3060E+01,  .3050E+01,  .3940E+01,
      .3950E+01,  .3940E+01,  .4800E+01,  .5630E+01,  .9920E+01,
      .9920E+01,  .5830E+01,  .6760E+01,  .6800E+01,  .2610E+01,
      .4190E+01,  .2470E+01,  .2040E+01,  .2030E+01,  .3410E+01,
      .3400E+01,  .1980E+01,  .4010E+01,  .3990E+01,  .1130E+01,
      .1130E+01,  .2160E+01,  .2160E+01,  .3110E+01,  .3110E+01,
      .1090E+02,  .1090E+02,  .2720E+01,  .4010E+01,  .4010E+01,
      .7780E+01,  .4570E+00,  .7800E+01,  .2280E+01,  .3150E+01,
      .2060E+01,  .2050E+01,  .5340E+01,  .1030E-01,  .4040E+01,
      .4030E+01,  .3780E+01,  .3310E+01,  .1190E+02,  .1190E+02,
      .3300E+01,  .1150E+01,  .1150E+01,  .2200E+01,  .2200E+01,
      .8790E+01,  .8800E+01,  .3610E+01,  .4600E+01,  .1620E+01,
      .2080E+01,  .2080E+01,  .1290E+02,  .1290E+02,  .6450E+01,
      .5560E+01,  .9780E+01,  .3230E+01,  .9790E+01,  .3170E+01,
      .4200E-01,  .4410E+01,  .4410E+01,  .6570E+01,  .4750E+01,
      .1390E+02,  .1390E+02,  .2100E+01,  .2100E+01,  .7530E+01,
      .1080E+02,  .1080E+02,  .3160E+01,  .3070E+00,  .7590E+01,
      .1490E+02,  .1490E+02,  .2970E+01,  .4300E+01,  .4300E+01,
      .1220E+01,  .3130E+01,  .1180E+02,  .1180E+02,  .8570E+01,
      .2120E+01,  .2120E+01,  .8600E+01,  .8570E-01,  .5930E+01,
      .3200E+01,  .1590E+02,  .1590E+02,  .3430E+01,  .1270E+02,
      .1270E+02,  .9590E+01,  .9610E+01,  .4210E+01,  .3840E+01,
      .4210E+01,  .5410E+01,  .5410E+01,  .4470E+01,  .2650E+01};

    static const double el[335]={
      323.147,  764.957,  208.238, 1157.451, 1156.320,
      2167.610, 2167.583, 1625.146, 1624.965,  644.659,
      1886.021, 2463.271, 2463.250, 1885.998,  449.986,
      1378.683,  591.672, 1378.666, 1548.301,  945.555,
      34.222, 2218.351,  101.030,   57.837,  212.281,
      843.680,  770.899,  945.264, 1249.138,  989.773,
      2780.334, 2780.397, 2167.602, 2167.612,     .033,
      197.307, 1625.091, 1157.518,  255.693,  403.963,
      1624.950,  590.082,  142.295,  584.400, 2578.130,
      118.399, 1068.954, 2793.122,  320.422, 1406.368,
      34.228, 3131.727,  315.145, 3131.712, 2484.004,
      1907.088, 1156.311,  793.589,  118.424, 2483.924,
      2387.155, 1693.538, 1147.995,  886.984, 2020.240,
      208.292,  101.087, 1440.446,  660.799,  208.315,
      1906.352,  469.151, 1028.777,  472.151, 1320.426,
      2224.964,  765.005, 2835.171, 1760.800,  326.462,
      323.168,  197.346, 2255.493,  450.798,  315.222,
      1884.823, 2656.685, 2834.705,  450.816,  326.410,
      2579.511, 1548.330, 1402.572,  472.119,   65.830,
      617.872, 2106.111,  643.258, 3084.881,  320.489,
      1288.757, 3221.127,   57.861, 1249.191,  815.219,
      2971.670, 3711.515, 1597.715, 2222.712, 3235.642,
      989.764,  644.658,  472.049,  142.349,  974.057,
      3537.207,  770.863, 1041.509,  469.092,  617.888,
      591.601, 2793.082,  590.045,  765.025, 2475.221,
      974.142, 3219.758,  843.055, 1216.197, 1573.180,
      2387.149, 1489.879, 1295.622,  255.702,  843.737,
      644.602,  643.267, 1683.497, 1793.859,  118.498,
      2020.266, 1901.028, 1693.509,  815.199,  403.986,
      2867.835, 1576.867, 2126.690, 1406.355, 2573.547,
      1068.705, 1157.532,  945.517,  945.280, 1156.336,
      1402.551, 1683.468, 1882.692, 1216.223, 1998.012,
      843.708,  843.077, 3872.183, 1068.987, 2345.118,
      584.398, 2723.408,  214.076, 2487.305,  212.341,
      3402.429, 1884.530, 3131.434, 2230.146, 1041.475,
      793.645, 2971.678, 1068.958, 1068.736, 3284.532,
      1320.245, 2579.549, 1320.373, 3567.763, 2874.730,
      1998.031, 2217.466, 2224.979, 1907.099, 1378.699,
      1378.667, 1625.138, 1624.961, 2222.738, 2573.524,
      1906.406, 2958.141, 3375.511,  208.300, 3824.401,
      1028.810,  323.225, 2218.399, 1489.875,  315.159,
      1320.404, 1320.311, 2958.156, 1295.616, 3288.350,
      1597.724, 3725.614, 1597.749, 1288.703, 2577.705,
      472.195, 3642.048, 1886.011, 1886.007, 2167.668,
      2167.641, 3221.045, 2484.093, 2483.976, 2835.184,
      2578.138, 2834.770, 3219.826, 3638.646, 1597.776,
      1597.687, 3727.670, 1576.855, 1573.214, 2345.180,
      1793.880,  660.817, 1900.986, 1901.089,  450.203,
      449.938,  450.797, 2963.377, 2963.635, 2463.337,
      2463.337, 2780.332, 2780.329, 3131.684, 3131.665,
      1901.047, 1901.005,  887.066, 3517.085, 3517.011,
      1884.513,  326.461, 1882.751, 3375.470, 1148.055,
      2230.090, 2230.137, 2126.749,  101.101, 3374.657,
      3374.811, 1440.514,  591.669, 2230.105, 2230.086,
      590.044, 3107.515, 3107.515, 3460.297, 3460.296,
      2218.348, 2217.457, 2723.384, 1760.764,  617.860,
      2584.988, 2584.959, 2584.949, 2584.939, 2487.354,
      2106.159, 2578.156,  770.960, 2577.655,  765.053,
      197.425,  769.647,  769.616, 2475.229, 3131.397,
      2965.478, 2965.473, 2965.486, 2965.469, 2874.695,
      2963.677, 2963.421,  989.779,  472.096, 2867.885,
      3371.492, 3371.489,  974.144,  945.533,  945.360,
      815.171, 1249.138, 3374.826, 3374.644, 3288.298,
      3371.518, 3371.457, 3284.555,  320.480, 3567.762,
      1548.327, 3803.091, 3803.090, 1884.854, 3811.560,
      3811.566, 3727.730, 3725.638, 1157.466, 2255.554,
      1156.357, 1167.583, 1167.580, 2656.737, 1216.198};

    static const double ensanche[335][4]={
      {2.788,4.50,0.68,0.80},{2.001,4.50,0.68,0.80},{2.838,4.50,0.68,0.80},
      {1.808,4.50,0.68,0.80},{1.657,4.50,0.68,0.80},{1.172,4.50,0.68,0.80},
      {1.166,4.50,0.68,0.80},{1.423,4.50,0.68,0.80},{1.382,4.50,0.68,0.80},
      {2.681,4.50,0.68,0.80},{1.418,4.50,0.68,0.80},{1.225,4.50,0.68,0.80}, 
      {1.252,4.50,0.68,0.80},{1.527,4.50,0.68,0.80},{2.524,4.50,0.68,0.80},
      {1.716,4.50,0.68,0.80},{2.424,4.50,0.68,0.80},{1.986,4.50,0.68,0.80},
      {2.368,4.50,0.68,0.80},{2.039,4.50,0.68,0.80},{3.096,4.50,0.68,0.80},
      {1.924,4.50,0.68,0.80},{2.933,4.50,0.68,0.80},{2.921,4.50,0.68,0.80},
      {2.767,4.50,0.68,0.80},{2.518,4.50,0.68,0.80},{2.453,4.50,0.68,0.80}, 
      {2.397,4.50,0.68,0.80},{2.445,4.50,0.68,0.80},{2.442,4.50,0.68,0.80},
      {1.278,4.50,0.68,0.80},{1.326,4.50,0.68,0.80},{1.489,4.50,0.68,0.80},
      {1.648,4.50,0.68,0.80},{2.811,4.50,0.68,0.80},{2.877,4.50,0.68,0.80},
      {1.758,4.50,0.68,0.80},{2.072,4.50,0.68,0.80},{2.702,4.50,0.68,0.80},
      {2.732,4.50,0.68,0.80},{2.131,4.50,0.68,0.80},{2.619,4.50,0.68,0.80},
      {2.794,4.50,0.68,0.80},{2.764,4.50,0.68,0.80},{1.702,4.50,0.68,0.80},
      {2.897,4.50,0.68,0.80},{2.273,4.50,0.68,0.80},{2.078,4.50,0.68,0.80},
      {2.841,4.50,0.68,0.80},{2.107,4.50,0.68,0.80},{2.986,4.50,0.68,0.80},
      {1.281,4.50,0.68,0.80},{2.770,4.50,0.68,0.80},{1.344,4.50,0.68,0.80},
      {1.471,4.50,0.68,0.80},{1.708,4.50,0.68,0.80},{2.501,4.50,0.68,0.80},
      {2.687,4.50,0.68,0.80},{2.773,4.50,0.68,0.80},{1.719,4.50,0.68,0.80},
      {2.181,4.50,0.68,0.80},{2.137,4.50,0.68,0.80},{2.693,4.50,0.68,0.80},
      {2.690,4.50,0.68,0.80},{2.181,4.50,0.68,0.80},{2.714,4.50,0.68,0.80},
      {2.942,4.50,0.68,0.80},{2.590,4.50,0.68,0.80},{2.664,4.50,0.68,0.80},
      {2.776,4.50,0.68,0.80},{2.237,4.50,0.68,0.80},{2.664,4.50,0.68,0.80},
      {2.450,4.50,0.68,0.80},{2.590,4.50,0.68,0.80},{2.015,4.50,0.68,0.80},
      {1.669,4.50,0.68,0.80},{2.616,4.50,0.68,0.80},{1.388,4.50,0.68,0.80},
      {2.359,4.50,0.68,0.80},{2.563,4.50,0.68,0.80},{2.498,4.50,0.68,0.80},
      {2.847,4.50,0.68,0.80},{2.450,4.50,0.68,0.80},{2.684,4.50,0.68,0.80},
      {2.427,4.50,0.68,0.80},{2.524,4.50,0.68,0.80},{2.270,4.50,0.68,0.80},
      {1.826,4.50,0.68,0.80},{2.397,4.50,0.68,0.80},{2.477,4.50,0.68,0.80},
      {1.666,4.50,0.68,0.80},{2.584,4.50,0.68,0.80},{2.492,4.50,0.68,0.80},
      {2.302,4.50,0.68,0.80},{2.894,4.50,0.68,0.80},{2.302,4.50,0.68,0.80},
      {2.069,4.50,0.68,0.80},{2.314,4.50,0.68,0.80},{1.995,4.50,0.68,0.80},
      {2.595,4.50,0.68,0.80},{2.149,4.50,0.68,0.80},{1.264,4.50,0.68,0.80},
      {2.779,4.50,0.68,0.80},{2.584,4.50,0.68,0.80},{2.149,4.50,0.68,0.80},
      {1.758,4.50,0.68,0.80},{2.101,4.50,0.68,0.80},{1.711,4.50,0.68,0.80},
      {2.255,4.50,0.68,0.80},{2.172,4.50,0.68,0.80},{2.489,4.50,0.68,0.80},
      {1.989,4.50,0.68,0.80},{2.075,4.50,0.68,0.80},{2.838,4.50,0.68,0.80},
      {2.486,4.50,0.68,0.80},{1.728,4.50,0.68,0.80},{2.320,4.50,0.68,0.80},
      {1.935,4.50,0.68,0.80},{2.163,4.50,0.68,0.80},{2.445,4.50,0.68,0.80},
      {2.107,4.50,0.68,0.80},{2.270,4.50,0.68,0.80},{1.980,4.50,0.68,0.80},
      {2.051,4.50,0.68,0.80},{1.782,4.50,0.68,0.80},{2.039,4.50,0.68,0.80},
      {1.885,4.50,0.68,0.80},{1.850,4.50,0.68,0.80},{1.971,4.50,0.68,0.80},
      {1.782,4.50,0.68,0.80},{2.332,4.50,0.68,0.80},{1.832,4.50,0.68,0.80},
      {1.684,4.50,0.68,0.80},{2.797,4.50,0.68,0.80},{1.642,4.50,0.68,0.80},
      {1.645,4.50,0.68,0.80},{1.672,4.50,0.68,0.80},{2.350,4.50,0.68,0.80},
      {1.622,4.50,0.68,0.80},{2.823,4.50,0.68,0.80},{2.288,4.50,0.68,0.80},
      {1.332,4.50,0.68,0.80},{2.184,4.50,0.68,0.80},{2.083,4.50,0.68,0.80},
      {2.717,4.50,0.68,0.80},{1.438,4.50,0.68,0.80},{1.432,4.50,0.68,0.80},
      {1.400,4.50,0.68,0.80},{2.036,4.50,0.68,0.80},{2.163,4.50,0.68,0.80},
      {1.367,4.50,0.68,0.80},{1.859,4.50,0.68,0.80},{1.663,4.50,0.68,0.80},
      {1.607,4.50,0.68,0.80},{1.708,4.50,0.68,0.80},{1.752,4.50,0.68,0.80},
      {1.737,4.50,0.68,0.80},{1.364,4.50,0.68,0.80},{2.264,4.50,0.68,0.80},
      {1.654,4.50,0.68,0.80},{1.246,4.50,0.68,0.80},{1.240,4.50,0.68,0.80},
      {1.965,4.50,0.68,0.80},{1.311,4.50,0.68,0.80},{1.471,4.50,0.68,0.80},
      {2.557,4.50,0.68,0.80},{1.240,4.50,0.68,0.80},{2.483,4.50,0.68,0.80},
      {1.216,4.50,0.68,0.80},{2.456,4.50,0.68,0.80},{1.921,4.50,0.68,0.80},
      {1.225,4.50,0.68,0.80},{1.036,4.50,0.68,0.80},{ .962,4.50,0.68,0.80},
      {1.684,4.50,0.68,0.80},{2.267,4.50,0.68,0.80},{1.814,4.50,0.68,0.80},
      { .923,4.50,0.68,0.80},{ .909,4.50,0.68,0.80},{1.074,4.50,0.68,0.80},
      { .968,4.50,0.68,0.80},{1.734,4.50,0.68,0.80},{1.033,4.50,0.68,0.80},
      { .894,4.50,0.68,0.80},{1.065,4.50,0.68,0.80},{2.104,4.50,0.68,0.80},
      { .977,4.50,0.68,0.80},{1.666,4.50,0.68,0.80},{1.577,4.50,0.68,0.80},
      {1.302,4.50,0.68,0.80},{1.293,4.50,0.68,0.80},{1.462,4.50,0.68,0.80},
      {1.423,4.50,0.68,0.80},{1.492,4.50,0.68,0.80},{1.453,4.50,0.68,0.80},
      {1.477,4.50,0.68,0.80},{1.314,4.50,0.68,0.80},{1.110,4.50,0.68,0.80},
      {2.847,4.50,0.68,0.80},{ .912,4.50,0.68,0.80},{1.873,4.50,0.68,0.80},
      {2.560,4.50,0.68,0.80},{1.042,4.50,0.68,0.80},{1.938,4.50,0.68,0.80},
      {2.513,4.50,0.68,0.80},{ .687,4.50,0.68,0.80},{ .669,4.50,0.68,0.80}, 
      {1.944,4.50,0.68,0.80},{1.320,4.50,0.68,0.80},{ .938,4.50,0.68,0.80},
      { .684,4.50,0.68,0.80},{ .758,4.50,0.68,0.80},{ .838,4.50,0.68,0.80},
      {1.444,4.50,0.68,0.80},{ .681,4.50,0.68,0.80},{2.625,4.50,0.68,0.80},
      {1.278,4.50,0.68,0.80},{1.057,4.50,0.68,0.80},{1.057,4.50,0.68,0.80},
      {1.204,4.50,0.68,0.80},{1.199,4.50,0.68,0.80},{1.341,4.50,0.68,0.80},
      {1.305,4.50,0.68,0.80},{1.284,4.50,0.68,0.80},{1.350,4.50,0.68,0.80},
      { .873,4.50,0.68,0.80},{1.302,4.50,0.68,0.80},{1.270,4.50,0.68,0.80},
      {1.160,4.50,0.68,0.80},{ .518,4.50,0.68,0.80},{ .503,4.50,0.68,0.80},
      { .870,4.50,0.68,0.80},{1.027,4.50,0.68,0.80},{1.086,4.50,0.68,0.80},
      {1.832,4.50,0.68,0.80},{1.569,4.50,0.68,0.80},{2.669,4.50,0.68,0.80},
      { .500,4.50,0.68,0.80},{ .675,4.50,0.68,0.80},{1.971,4.50,0.68,0.80},
      {1.930,4.50,0.68,0.80},{2.592,4.50,0.68,0.80},{ .494,4.50,0.68,0.80},
      { .740,4.50,0.68,0.80},{ .900,4.50,0.68,0.80},{ .900,4.50,0.68,0.80},
      {1.042,4.50,0.68,0.80},{1.042,4.50,0.68,0.80},{1.148,4.50,0.68,0.80},
      {1.145,4.50,0.68,0.80},{ .405,4.50,0.68,0.80},{ .400,4.50,0.68,0.80},
      {2.592,4.50,0.68,0.80},{1.190,4.50,0.68,0.80},{1.181,4.50,0.68,0.80},
      { .829,4.50,0.68,0.80},{2.850,4.50,0.68,0.80},{ .817,4.50,0.68,0.80},
      {1.716,4.50,0.68,0.80},{2.388,4.50,0.68,0.80},{ .391,4.50,0.68,0.80}, 
      { .539,4.50,0.68,0.80},{1.255,4.50,0.68,0.80},{2.681,4.50,0.68,0.80},
      { .382,4.50,0.68,0.80},{ .666,4.50,0.68,0.80},{2.143,4.50,0.68,0.80},
      {2.175,4.50,0.68,0.80},{ .337,4.50,0.68,0.80},{ .334,4.50,0.68,0.80},
      {2.060,4.50,0.68,0.80},{ .796,4.50,0.68,0.80},{ .796,4.50,0.68,0.80},
      { .935,4.50,0.68,0.80},{ .935,4.50,0.68,0.80},{ .669,4.50,0.68,0.80},
      { .619,4.50,0.68,0.80},{1.497,4.50,0.68,0.80},{1.817,4.50,0.68,0.80},
      {2.625,4.50,0.68,0.80},{ .326,4.50,0.68,0.80},{ .441,4.50,0.68,0.80},
      { .296,4.50,0.68,0.80},{ .296,4.50,0.68,0.80},{1.033,4.50,0.68,0.80},
      {1.462,4.50,0.68,0.80},{ .533,4.50,0.68,0.80},{ 2.362,4.50,0.68,0.80},
      {.488,4.50,0.68,0.80},{2.157,4.50,0.68,0.80},
      {2.678,4.50,0.68,0.80},{1.506,4.50,0.68,0.80},{ 1.492,4.50,0.68,0.80},
      {1.148,4.50,0.68,0.80},{1.142,4.50,0.68,0.80},
      { .275,4.50,0.68,0.80},{ .272,4.50,0.68,0.80},{  .290,4.50,0.68,0.80},
      {  .370,4.50,0.68,0.80},{ .864,4.50,0.68,0.80},
      { .441,4.50,0.68,0.80},{ .397,4.50,0.68,0.80},{ 2.456,4.50,0.68,0.80},
      { 2.737,4.50,0.68,0.80},{ .885,4.50,0.68,0.80},
      { .263,4.50,0.68,0.80},{ .263,4.50,0.68,0.80},{ 2.231,4.50,0.68,0.80},
      { 1.711,4.50,0.68,0.80},{1.666,4.50,0.68,0.80},
      {2.592,4.50,0.68,0.80},{2.510,4.50,0.68,0.80},{  .370,4.50,0.68,0.80},
      {  .334,4.50,0.68,0.80},{ .719,4.50,0.68,0.80},
      { .272,4.50,0.68,0.80},{ .323,4.50,0.68,0.80},{  .725,4.50,0.68,0.80},
      { 2.539,4.50,0.68,0.80},{ .882,4.50,0.68,0.80},
      { 2.465,4.50,0.68,0.80},{ .257,4.50,0.68,0.80},{  .257,4.50,0.68,0.80},
      { 2.279,4.50,0.68,0.80},{ .323,4.50,0.68,0.80},
      {  .299,4.50,0.68,0.80},{ .645,4.50,0.68,0.80},{  .627,4.50,0.68,0.80},
      { 1.891,4.50,0.68,0.80},{2.033,4.50,0.68,0.80},
      { 1.770,4.50,0.68,0.80},{1.184,4.50,0.68,0.80},{ 1.181,4.50,0.68,0.80},
      { 1.773,4.50,0.68,0.80},{2.282,4.50,0.68,0.80} };

    static const double gl[335]={
      3.00,  1.00,  1.00,  1.00,  3.00,
      1.00,  3.00,  3.00,  1.00,  3.00,
      3.00,  1.00,  3.00,  1.00,  1.00,
      1.00,  1.00,  3.00,  1.00,  3.00,
      3.00,  1.00,  1.00,  1.00,  3.00,
      1.00,  3.00,  1.00,  3.00,  1.00,
      3.00,  1.00,  1.00,  3.00,  1.00,
      3.00,  3.00,  1.00,  3.00,  1.00,
      1.00,  3.00,  1.00,  3.00,  3.00,
      3.00,  3.00,  3.00,  1.00,  3.00,
      3.00,  1.00,  1.00,  3.00,  3.00,
      1.00,  3.00,  1.00,  3.00,  1.00,
      1.00,  1.00,  3.00,  1.00,  3.00,
      1.00,  1.00,  1.00,  3.00,  1.00,
      3.00,  3.00,  3.00,  1.00,  1.00,
      3.00,  1.00,  1.00,  3.00,  3.00,
      3.00,  3.00,  1.00,  3.00,  1.00,
      3.00,  3.00,  3.00,  3.00,  3.00,
      1.00,  1.00,  1.00,  1.00,  3.00,
      1.00,  1.00,  1.00,  1.00,  1.00,
      1.00,  3.00,  1.00,  3.00,  3.00,
      3.00,  3.00,  3.00,  1.00,  1.00,
      1.00,  3.00,  1.00,  1.00,  3.00,
      3.00,  3.00,  1.00,  3.00,  1.00,
      1.00,  3.00,  3.00,  1.00,  3.00,
      3.00,  1.00,  3.00,  1.00,  3.00,
      1.00,  3.00,  3.00,  3.00,  1.00,
      3.00,  1.00,  3.00,  1.00,  3.00,
      3.00,  1.00,  1.00,  3.00,  1.00,
      1.00,  1.00,  3.00,  3.00,  3.00,
      1.00,  1.00,  3.00,  1.00,  3.00,
      1.00,  3.00,  1.00,  1.00,  1.00,
      1.00,  3.00,  3.00,  3.00,  3.00,
      3.00,  1.00,  1.00,  1.00,  3.00,
      1.00,  3.00,  3.00,  3.00,  1.00,
      1.00,  3.00,  3.00,  1.00,  3.00,
      3.00,  1.00,  1.00,  1.00,  3.00,
      1.00,  3.00,  3.00,  1.00,  1.00,
      3.00,  3.00,  1.00,  1.00,  3.00,
      3.00,  1.00,  3.00,  1.00,  1.00,
      3.00,  3.00,  1.00,  3.00,  1.00,
      1.00,  3.00,  1.00,  3.00,  1.00,
      1.00,  1.00,  3.00,  1.00,  1.00,
      1.00,  1.00,  3.00,  1.00,  1.00,
      3.00,  3.00,  3.00,  1.00,  1.00,
      3.00,  3.00,  1.00,  3.00,  3.00,
      1.00,  3.00,  1.00,  3.00,  3.00,
      1.00,  3.00,  3.00,  1.00,  3.00,
      1.00,  3.00,  3.00,  1.00,  1.00,
      3.00,  3.00,  1.00,  1.00,  3.00,
      1.00,  3.00,  1.00,  3.00,  1.00,
      3.00,  3.00,  1.00,  3.00,  3.00,
      1.00,  3.00,  3.00,  1.00,  1.00,
      3.00,  1.00,  1.00,  3.00,  1.00,
      3.00,  3.00,  1.00,  1.00,  3.00,
      1.00,  3.00,  1.00,  3.00,  1.00,
      3.00,  1.00,  1.00,  3.00,  1.00,
      1.00,  3.00,  3.00,  1.00,  1.00,
      3.00,  1.00,  3.00,  3.00,  3.00,
      3.00,  1.00,  1.00,  3.00,  3.00,
      1.00,  3.00,  1.00,  1.00,  1.00,
      1.00,  3.00,  3.00,  3.00,  1.00,
      3.00,  3.00,  3.00,  1.00,  1.00,
      3.00,  1.00,  3.00,  1.00,  1.00,
      1.00,  3.00,  1.00,  3.00,  1.00,
      3.00,  3.00,  1.00,  1.00,  1.00,
      3.00,  3.00,  1.00,  3.00,  1.00};


    static const int ifin1[800]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   7,   8,   8,   8,   8,   8,   8,   8,   8,
      8,   8,   9,   9,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  12,  12,  12,
      13,  14,  14,  14,  14,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  19,  19,  19,  19,  19,  19,
      19,  19,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  25,  27,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  27,  28,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  32,  32,  32,  32,  32,  33,  33,  33,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  35,  35,  35,  35,  35,  36,  36,
      36,  36,  38,  38,  38,  38,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  40,  40,
      40,  40,  40,  40,  40,  40,  41,  41,  41,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45};

    static const int ini1[800]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   7,   8,   8,   8,   8,   8,   8,   8,   8,
      8,   8,   9,   9,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  12,  12,  12,
      13,  14,  14,  14,  14,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  19,  19,  19,  19,  19,  19,
      19,  19,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  25,  27,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  27,  28,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  32,  32,  32,  32,  32,  33,  33,  33,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  35,  35,  35,  35,  35,  36,  36,
      36,  36,  38,  38,  38,  38,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  40,  40,
      40,  40,  40,  40,  40,  40,  41,  41,  41,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44};

    static const int ifin2[800]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   8,   8,   8,   8,   8,   8,   8,   8,
      8,   8,   8,   9,   9,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  12,  12,
      13,  13,  14,  14,  14,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  19,  19,  19,  19,  19,  19,
      19,  19,  20,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  25,  25,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  27,  28,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  32,  32,  32,  32,  33,  33,  33,  33,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  35,  35,  35,  35,  35,  36,  36,
      36,  36,  36,  38,  38,  38,  38,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  40,
      40,  40,  40,  40,  40,  40,  41,  41,  41,  41,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  45,  45,  45,  45,  45,  45,  45,  45,  45};

    static const int ini2[800]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   8,   8,   8,   8,   8,   8,   8,   8,   8,
      8,   8,   9,   9,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  12,  12,  13,
      13,  14,  14,  14,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  19,  19,  19,  19,  19,  19,  19,
      19,  20,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  25,  25,  27,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  28,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  32,  32,  32,  32,  33,  33,  33,  33,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  35,  35,  35,  35,  35,  36,  36,  36,
      36,  36,  38,  38,  38,  38,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  40,  40,
      40,  40,  40,  40,  40,  41,  41,  41,  41,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44};

    static const int ifin3[800]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   8,   8,   8,   8,   8,
      8,   8,   8,   8,   8,   8,   9,   9,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  12,  12,  13,  13,  14,  14,  14,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  19,  19,  19,
      19,  19,  19,  19,  19,  20,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  25,  25,
      27,  27,  27,  27,  27,  27,  27,  27,  27,  27,
      28,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  32,  32,  32,  32,  33,
      33,  33,  33,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  35,  35,  35,  35,
      35,  36,  36,  36,  36,  36,  38,  38,  38,  38,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  40,  40,  40,  40,  40,  40,  40,  41,
      41,  41,  41,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  45,  45,  45,  45,  45,  45};

    static const int ini3[800]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   6,   6,
      6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
      6,   6,   6,   6,   6,   6,   6,   6,   8,   8,
      8,   8,   8,   8,   8,   8,   8,   8,   8,   9,
      9,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  12,  12,  13,  13,  14,  14,
      14,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  17,  17,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      19,  19,  19,  19,  19,  19,  19,  19,  20,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  25,  25,  27,  27,  27,  27,  27,  27,  27,
      27,  27,  27,  28,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
      31,  31,  31,  31,  31,  31,  31,  31,  32,  32,
      32,  32,  33,  33,  33,  33,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  34,
      34,  34,  34,  34,  34,  34,  34,  34,  34,  35,
      35,  35,  35,  35,  36,  36,  36,  36,  36,  38,
      38,  38,  38,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  40,  40,  40,  40,  40,
      40,  40,  41,  41,  41,  41,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  45,  45,  45};
      


    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=1.82332;  //Debyes

    double q=0.034256116*pow(tt.get("K"),1.5);    // Q(300 K)=178.120 JPL Line Catalog
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
  
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
            
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
        
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      if(ifin==0||ifin<ini){

	return complex<double> (0.0,0.0);

      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){

	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening_water(Frequency(fre[i],"GHz"),tt,pp,eh2o,ensanche[i][0],ensanche[i][1],ensanche[i][2],ensanche[i][3]),Frequency(0.0,"GHz"));

	  lshape=lshape*flin[i]*gl[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 

          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(fac2fixed*pow(mu,2.0)/q); // imaginary part: absorption coefficient in cm^2
	                                                                    // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)  // (  rad m^2 , m^2 )  
	
      }
     
    }
 
  }




  complex<double>  RefractiveIndexProfile::mkAbs_h2o(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){

    static const double fre[522] = {22.234617,183.310001,321.195929,
				    325.152822,   380.197554,   390.188508,   437.365786,
				    439.141666,   443.037657,   448.001261,   470.880416,
				    474.688922,   488.491469,   503.499954,   504.414095,
				    556.936071,   620.701223,   
				    752.033047,      
				    906.190476,   916.171369,   970.314874,   987.926855,
				    1001.293579, 1074.447510, 1074.450439, 1097.364624, 1113.343506,
				    1146.620850, 1153.125610, 1153.447754, 1158.324585, 1162.914429,
				    1168.382690, 1172.519287, 1190.854004, 1207.642456, 1215.775879,
				    1219.918213, 1228.790894, 1236.490234, 1236.490723, 1271.173706,
				    1278.261353, 1296.401855, 1322.062134, 1333.029297, 1333.734253,
				    1410.622559, 1440.775879, 1512.243286, 1512.358032, 1541.968506,
				    1602.218750, 1661.009277, 1669.905884, 1716.770142, 1717.219971,
				    1717.238281, 1762.049683, 1766.194336, 1794.793213, 1797.164917,
				    1856.496704, 1856.499512, 1867.754028, 1879.747314, 1884.868042,
				    1898.843628, 1903.470215, 1903.569214, 1918.484741, 1919.361206,
				    1930.157104, 1944.950439, 1969.209473, 2015.986694, 2022.371338,
				    2025.167603, 2040.480347, 2074.430908, 2162.357178, 2164.134033,
				    2175.395752, 2175.892090, 2196.348389, 2221.749512, 2244.798096,
				    2264.152344, 2317.860596, 2344.252930, 2347.451904, 2355.093506,
				    2355.178223, 2356.748779, 2358.515137, 2365.902100, 2391.573486,
				    2428.252197, 2446.835449, 2462.936035, 2531.894287, 2547.394775,
				    2567.174561, 2571.751709, 2574.961426, 2602.496582, 2618.201416,
				    2619.323242, 2630.963867, 2640.476562, 2644.914307, 2664.570557,
				    2685.642334, 2689.044434, 2712.224365, 2721.470215, 2773.979004,
				    2801.812500, 2836.786133, 2838.571289, 2848.771729, 2864.042969,
				    2880.030518, 2884.277588, 2884.942383, 2962.107666, 2968.750732,
				    2970.798340, 2989.611328, 2989.938721, 2991.528076, 2997.372559,
				    2998.565918, 3003.360596, 3011.131104, 3013.200928, 3043.768066,
				    3074.613281, 3118.934814, 3126.589844, 3135.013916, 3149.856445,
				    3165.537598, 3167.581543, 3182.188721, 3210.361084, 3230.147705,
				    3242.007324, 3245.232666, 3307.299072, 3323.194336, 3329.184326,
				    3331.462158, 3350.895508, 3354.355713, 3365.210938, 3402.357422,
				    3428.800049, 3468.105957, 3477.542969, 3495.348633, 3497.691406,
				    3497.700195, 3503.236084, 3509.395996, 3524.848633, 3536.664062,
				    3538.082275, 3547.178223, 3569.766602, 3599.643311, 3612.967041,
				    3654.606445, 3669.865967, 3674.246826, 3682.703857, 3691.316895,
				    3718.042969, 3721.492676, 3736.993408, 3762.439941, 3775.912109,
				    3798.278564, 3807.263916, 3809.183594, 3830.651611, 3855.278809,
				    3922.859619, 3936.949463, 3949.330811, 3953.483643, 3954.344238,
				    3956.030518, 3970.992676, 3977.049316, 3981.654053, 4000.157471,
				    4020.075195, 4035.976318, 4046.934570, 4053.368896, 4072.539307,
				    4080.917480, 4090.253662, 4101.301270, 4118.583008, 4157.245117,
				    4158.798828, 4161.912109, 4166.852051, 4174.057129, 4190.579102,
				    4218.437500, 4239.429688, 4240.187500, 4241.226074, 4279.597656,
				    4305.398926, 4345.487305, 4348.334961, 4366.788574, 4372.642578,
				    4380.344727, 4435.662109, 4467.616699, 4468.573730, 4509.372559,
				    4512.388672, 4519.476562, 4535.935059, 4571.626465, 4600.438477,
				    4602.698242, 4619.316406, 4668.626465, 4681.352051, 4684.310547,
				    4684.646484, 4687.294434, 4687.478027, 4689.498047, 4690.042969,
				    4690.502441, 4692.996094, 4700.997070, 4703.642090, 4715.733398,
				    4724.263672, 4734.295898, 4739.351074, 4749.300293, 4764.022461,
				    4769.729004, 4801.924316, 4802.988281, 4806.478027, 4843.365234,
				    4851.373535, 4869.814453, 4922.436523, 4929.354492, 4947.516113,
				    4971.006836, 4983.063477, 4993.749023, 4995.889160, 4997.618652,
				    5031.587891, 5039.218262, 5094.389648, 5107.286133, 5141.480469,
				    5162.856445, 5194.858398, 5197.115234, 5201.435059, 5205.333984,
				    5227.480469, 5264.046875, 5276.513672, 5280.730957, 5297.037109,
				    5322.554199, 5331.894531, 5340.038086, 5350.834961, 5354.925293,
				    5355.066406, 5363.381348, 5364.864258, 5365.890625, 5367.713867,
				    5368.607422, 5368.771484, 5372.354492, 5372.546387, 5376.338379,
				    5377.333496, 5380.180664, 5404.980957, 5424.200684, 5429.517090,
				    5437.819824, 5476.755859, 5508.605957, 5563.786133, 5566.169922,
				    5590.651855, 5628.669434, 5641.758789, 5642.085938, 5652.221191,
				    5734.314453, 5745.763672, 5772.804199, 5791.293457, 5800.348633,
				    5801.258789, 5825.623535, 5827.402832, 5831.823242, 5862.105957,
				    5870.084961, 5870.382324, 5913.849609, 5920.743164, 5927.955566,
				    5936.366699, 5948.863770, 5948.883301, 5951.507812, 5974.460938,
				    5974.598145, 5993.568848, 5995.089355, 5995.770996, 5998.833496,
				    6002.584473, 6002.696289, 6008.021973, 6009.441406, 6010.683105,
				    6011.272461, 6017.750977, 6018.080078, 6069.846680, 6076.475098,
				    6083.241699, 6132.811523, 6144.350098, 6166.270996, 6204.464844,
				    6232.336426, 6249.174805, 6252.356934, 6333.647461, 6334.460938,
				    6372.369629, 6373.128418, 6374.534180, 6413.186523, 6432.151855,
				    6449.397461, 6459.154785, 6482.924805, 6482.927734, 6498.508301,
				    6502.453125, 6520.811035, 6520.831543, 6554.908691, 6555.020020,
				    6583.322266, 6583.687988, 6583.797363, 6591.576172, 6603.834473,
				    6605.517090, 6608.801758, 6613.626465, 6617.330566, 6618.762207,
				    6622.659668, 6645.573242, 6647.482422, 6685.065430, 6702.183105,
				    6706.533691, 6772.161133, 6781.169434, 6783.467285, 6830.006348,
				    6837.352051, 6884.583496, 6916.979980, 6917.308594, 6925.285156,
				    6931.553711, 6951.812500, 6958.659668, 6992.815430, 6992.815918,
				    6994.716309, 7041.973145, 7041.976074, 7054.262695, 7064.055176,
				    7065.917480, 7087.209961, 7087.227539, 7126.841797, 7126.923340,
				    7159.030762, 7159.341309, 7181.690918, 7182.708984, 7224.776367,
				    7233.533691, 7306.716797, 7321.074707, 7330.934570, 7337.663086,
				    7355.121582, 7367.564941, 7375.817383, 7432.240723, 7459.451660,
				    7459.596680, 7479.122070, 7491.806152, 7567.957520, 7567.957520,
				    7609.100098, 7612.908691, 7613.799805, 7625.316895, 7625.520508,
				    7625.520996, 7626.206543, 7677.482910, 7677.485352, 7707.561523,
				    7722.062988, 7722.076660, 7747.363281, 7754.206543, 7758.712402,
				    7814.349121, 7831.405273, 7879.645508, 7948.519043, 7980.320801,
				    7999.686035, 7999.751465, 8021.053711, 8027.005371, 8149.468262,
				    8182.586426, 8183.017578, 8267.326172, 8274.441406, 8276.783203,
				    8278.713867, 8341.956055, 8349.770508, 8404.710938, 8404.710938,
				    8404.716797, 8451.620117, 8461.892578, 8464.212891, 8464.212891,
				    8525.195312, 8537.547852, 8537.579102, 8559.085938, 8561.903320,
				    8568.458984, 8629.490234, 8677.352539, 8686.968750, 8715.722656,
				    8735.931641, 8736.146484, 8785.902344, 8787.137695, 8823.014648,
				    8946.191406, 9049.722656, 9072.887695, 9072.902344, 9077.880859,
				    9083.127930, 9083.238281, 9087.024414, 9093.927734, 9095.279297,
				    9131.275391, 9139.863281, 9285.300781, 9285.409180, 9288.641602,
				    9289.305664, 9345.036133, 9390.063477, 9435.738281, 9445.885742,
				    9449.122070, 9602.740234, 9605.547852, 9605.554688, 9616.522461,
				    9625.801758, 9626.460938, 9643.747070, 9702.218750, 9711.139648,
				    9783.533203, 9819.860352, 9821.017578, 9830.557617, 9830.613281};


    static const double flin[522]= {.56800E-01,.10200E+00,.89500E-01,
				    .90700E-01,  .12300E+00,  .65100E-01,  .88400E-01,
				    .10100E+00,  .88400E-01,  .13200E+00,  .10200E+00,
				    .11800E+00,  .35900E-01,  .78600E-01,  .78600E-01,
				    .15000E+01,  .12200E+00,  
				    .20700E+01,  
				    .13200E+00,  .16300E+00,  .26400E+00,  .75600E+00,
				    .8270E-01,  .6070E-01,  .6070E-01,  .2180E+01,  .1000E+01,
				    .2470E-01,  .3020E+00,  .5130E-01,  .2830E+00,  .2540E+01,
				    .2310E+00,  .2580E+00,  .2320E+00,  .3640E+01,  .2090E+00,
				    .2090E+00,  .1260E+01,  .5700E-01,  .5700E-01,  .1600E+00,
				    .2610E+00,  .1980E+00,  .3160E+00,  .1920E+00,  .1920E+00,
				    .4220E+01,  .2760E+00,  .1780E+00,  .1780E+00,  .3090E+00,
				    .2050E+01,  .8330E+00,  .1500E+01,  .1740E+01,  .1670E+00,
				    .1670E+00,  .4800E+01,  .4390E+00,  .4240E+01,  .5910E+01,
				    .1580E+00,  .1580E+00,  .3430E+01,  .1870E-01,  .4390E+00,
				    .4070E+00,  .2240E+00,  .4300E-01,  .4390E+00,  .1300E+01,
				    .3750E+00,  .3750E+00,  .4100E+00,  .6410E+01,  .3480E+00,
				    .3480E+00,  .2170E+01,  .8360E+00,  .4570E+00,  .2160E+01,
				    .3250E+00,  .3250E+00,  .1090E+01,  .1920E+01,  .5330E+00,
				    .1570E+01,  .7060E+01,  .4010E+01,  .8160E+01,  .3070E+00,
				    .3070E+00,  .3190E+00,  .2510E+00,  .1040E+01,  .2830E+01,
				    .6340E+01,  .5620E+01,  .1850E+01,  .5270E+00,  .6150E+00,
				    .5580E+00,  .8610E+01,  .4360E+00,  .3750E-01,  .6000E+00,
				    .1530E-01,  .2480E+01,  .3010E+01,  .5640E+00,  .4220E+01,
				    .1710E+01,  .5660E+00,  .5290E+00,  .5290E+00,  .1500E+01,
				    .6120E+00,  .4990E+00,  .4990E+00,  .9270E+01,  .1040E+02,
				    .2950E+01,  .1860E+01,  .3060E+01,  .8640E+00,  .1240E+01,
				    .1630E+01,  .4740E+00,  .4740E+00,  .8490E+01,  .7770E+01,
				    .3810E+01,  .6030E+01,  .3390E+00,  .3910E+01,  .2070E+01,
				    .1080E+02,  .7500E+00,  .1090E+01,  .3970E+01,  .6880E+00,
				    .1090E+01,  .1790E+01,  .2020E+01,  .3270E+01,  .2870E+01,
				    .3360E-01,  .6290E+01,  .7910E+00,  .1320E-01,  .3610E+01,
				    .1670E+01,  .1260E+02,  .7650E+00,  .1140E+02,  .7270E+00,
				    .7280E+00,  .7730E+00,  .1070E+02,  .4230E+01,  .6910E+00,
				    .4640E+00,  .6910E+00,  .5030E+01,  .1310E+02,  .1860E+01,
				    .9850E+01,  .8000E+00,  .8120E+01,  .4950E+01,  .3460E+01,
				    .4980E+01,  .9030E+00,  .5760E+01,  .3720E+01,  .1830E+01,
				    .8340E+00,  .3990E+01,  .4700E+01,  .6150E+00,  .7470E+00,
				    .2660E+01,  .1970E+01,  .8310E+01,  .3060E-01,  .3030E+01,
				    .2100E+01,  .9570E+00,  .1110E+01,  .2090E+01,  .3010E+01,
				    .1110E+01,  .3870E+01,  .9240E+00,  .1170E-01,  .1500E+01,
				    .4680E+01,  .1030E+02,  .9670E+00,  .5030E+01,  .3580E+01,
				    .1180E+02,  .9350E+00,  .6990E+01,  .5400E+01,  .9400E+00,
				    .8970E+00,  .1870E+01,  .5980E+01,  .8980E+00,  .5980E+01,
				    .2450E+01,  .7800E+01,  .1860E+01,  .9750E+00,  .6010E+01,
				    .9930E+00,  .3710E+01,  .5890E+01,  .5630E+01,  .1020E+02,
				    .2810E-01,  .5250E+01,  .1060E+01,  .2390E+01,  .6490E+01,
				    .2370E+01,  .4920E+01,  .3780E+01,  .3650E+01,  .3130E+01,
				    .1050E-01,  .4000E+01,  .3080E+01,  .8900E+01,  .4850E+01,
				    .3980E+01,  .9940E+01,  .2130E+01,  .1120E+01,  .3070E+01,
				    .1120E+01,  .2130E+01,  .5700E+01,  .1160E+01,  .6460E+00,
				    .6990E+01,  .6990E+01,  .1090E+01,  .6490E+01,  .1880E+01,
				    .1150E+01,  .1880E+01,  .1400E+01,  .6840E+01,  .7200E+01,
				    .1080E+01,  .5390E+01,  .1160E+01,  .7610E+01,  .7770E+01,
				    .3720E+01,  .2380E+01,  .7810E+01,  .8290E+00,  .4000E+01,
				    .5600E+01,  .9920E+00,  .3690E+01,  .2320E+01,  .6780E+01,
				    .7080E+01,  .4880E+01,  .9580E-02,  .6110E+00,  .8290E+01,
				    .1230E+01,  .5850E+01,  .7990E+01,  .7990E+01,  .1310E+01,
				    .2160E+01,  .4950E+01,  .5480E+01,  .1890E+01,  .1140E+01,
				    .1140E+01,  .4050E+01,  .6670E+01,  .5810E+01,  .2160E+01,
				    .2160E+01,  .1890E+01,  .3130E+01,  .4940E+01,  .3130E+01,
				    .4040E+01,  .7500E+01,  .1360E+01,  .8280E+01,  .1350E+01,
				    .4980E+01,  .8660E+01,  .8980E+01,  .3740E+01,  .7240E+01,
				    .7550E+01,  .3730E+01,  .2330E+01,  .9600E+01,  .5600E+01,
				    .8600E+01,  .1390E+01,  .8830E-02,  .1390E+01,  .5950E+01,
				    .8930E+01,  .8990E+01,  .8990E+01,  .5550E+01,  .1390E+01,
				    .3470E+01,  .7670E+01,  .2110E+01,  .5990E+01,  .1910E+01,
				    .1910E+01,  .1150E+01,  .1150E+01,  .6780E+01,  .2200E+01,
				    .2200E+01,  .5890E+01,  .3170E+01,  .3170E+01,  .8450E+01,
				    .7350E+01,  .7610E+01,  .4100E+01,  .5010E+01,  .4100E+01,
				    .6750E+01,  .5890E+01,  .5000E+01,  .2460E+01,  .3390E+01,
				    .3390E+01,  .3770E+01,  .1390E+01,  .3760E+01,  .7540E+01,
				    .5630E+01,  .1850E+01,  .1490E+01,  .5600E+01,  .8200E-02,
				    .9990E+01,  .9990E+01,  .6980E+01,  .2760E+01,  .7000E+01,
				    .1330E-01,  .7430E+01,  .1160E+01,  .1160E+01,  .1920E+01,
				    .1920E+01,  .2220E+01,  .2220E+01,  .3220E+01,  .3220E+01,
				    .4160E+01,  .7680E+01,  .4160E+01,  .1910E+01,  .5070E+01,
				    .5070E+01,  .6820E+01,  .5950E+01,  .7670E+01,  .5950E+01,
				    .6820E+01,  .3890E+00,  .4620E+01,  .3800E+01,  .3790E+01,
				    .3260E+01,  .7560E+01,  .5660E+01,  .3270E+01,  .3250E+01,
				    .5650E+01,  .7680E-02,  .1100E+02,  .1100E+02,  .7500E+01,
				    .7990E+01,  .3050E+01,  .8000E+01,  .1170E+01,  .1170E+01,
				    .3990E+01,  .2250E+01,  .2250E+01,  .1810E+01,  .1940E+01,
				    .1940E+01,  .3260E+01,  .3260E+01,  .4220E+01,  .4220E+01,
				    .5130E+01,  .5130E+01,  .6010E+01,  .6010E+01,  .3830E+01,
				    .3830E+01,  .5700E+01,  .4890E+01,  .5760E+01,  .5690E+01,
				    .1430E+01,  .5080E-01,  .1810E+01,  .3160E+01,  .1200E+02,
				    .1200E+02,  .9000E+01,  .9000E+01,  .1190E+01,  .1190E+01,
				    .3070E+01,  .4390E+01,  .4390E+01,  .1960E+01,  .2280E+01,
				    .2280E+01,  .1960E+01,  .3300E+01,  .3300E+01,  .5890E+01,
				    .4270E+01,  .4270E+01,  .2660E+01,  .3860E+01,  .3860E+01,
				    .5740E+01,  .5730E+01,  .4180E+01,  .6850E+01,  .3060E+01,
				    .1300E+02,  .1300E+02,  .9990E+01,  .9990E+01,  .6910E+01,
				    .1980E+01,  .1980E+01,  .2370E+01,  .3890E+01,  .3890E+01,
				    .2590E+00,  .4260E+01,  .4260E+01,  .1200E+01,  .1200E+01,
				    .3050E+01,  .9580E-01,  .2790E+01,  .2310E+01,  .2310E+01,
				    .7900E+01,  .1400E+02,  .1400E+02,  .1100E+02,  .1100E+02,
				    .2230E+01,  .7930E+01,  .1020E+01,  .5370E+01,  .3170E+01,
				    .2000E+01,  .2000E+01,  .3930E+01,  .3920E+01,  .3640E+01,
				    .3480E+01,  .4140E+01,  .1500E+02,  .1500E+02,  .8920E+01,
				    .5390E+01,  .5390E+01,  .4130E+01,  .1200E+02,  .1200E+02,
				    .8940E+01,  .4020E+01,  .2020E+01,  .2020E+01,  .3960E+01,
				    .3960E+01,  .4770E+01,  .6540E+01,  .4410E-02,  .2380E+01,
				    .3140E+01,  .5700E+01,  .1600E+02,  .1600E+02,  .9930E+01,
				    .1300E+02,  .1300E+02,  .9940E+01,  .1240E+00,  .4020E+01,
				    .4790E+01,  .5250E+01,  .5250E+01,  .2040E+01,  .2040E+01};

    static const double el[522] = {642.4,195.9,1846.0,
				   454.4,     305.2,    2194.5,    1503.7,
				   1067.7,    1503.7,    410.7,    1067.8,
				   702.4,     843.9,    2006.9,    2007.0,
				   34.3,     702.3,   
				   100.8,    
				   1511.1,     410.9,     552.3,    53.5,
				   4620.445, 3892.334, 3892.334,  196.834,    0.032,
				   1070.771,  194.158, 2553.643,  878.209,  249.489,
				   1749.926, 1278.628, 1749.948,  396.442, 2288.852,
				   2288.853,  136.927, 4645.157, 4645.157, 3173.693,
				   1278.553, 1212.082,  732.151, 2892.024, 2891.990,
				   574.800,  951.853, 3556.723, 3556.718,  877.897,
				   319.505,  114.384,   34.257,  114.407, 4281.886,
				   4281.885,  867.334, 1090.335,  781.163, 1125.849,
				   5070.502, 5070.501,  642.461, 1324.086, 1525.040,
				   2031.169, 2732.447, 2938.743,  550.427,  204.685,
				   2605.266, 2605.257, 2031.192, 1414.347, 3243.741,
				   3243.707,  454.372,  296.842, 1524.723,  100.837,
				   3943.897, 3943.873,  305.291,  468.172, 1339.966,
				   323.537, 1845.959, 1013.293, 2213.239, 4704.473,
				   4704.469, 2325.893, 4835.409,  296.854,  204.722,
				   1729.462, 1511.070,  432.197, 1628.487, 1807.144,
				   1088.894, 2609.074, 1957.221, 3349.599, 2347.345,
				   1603.692,  598.933,  196.776, 2955.963, 1212.020,
				   470.009, 2955.945, 3629.333, 3629.289,   60.969,
				   2347.233, 4364.355, 4364.369, 3173.679, 3645.847,
				   795.579,  642.776,  951.944,  725.140,   53.421,
				   432.223, 5159.621, 5159.604, 3030.128, 2732.448,
				   1270.391, 2069.061, 4292.688,  323.488,  732.121,
				   4145.141, 2125.714,  552.346,  319.542, 1806.030,
				   550.377,  643.579,  725.178, 1021.026,  933.776,
				   3786.107, 2325.952, 2698.574, 1909.611, 1175.123,
				   136.914, 5407.781, 3340.915, 4835.494, 4048.312,
				   4048.342, 3340.856, 4668.503, 1447.749, 4817.536,
				   3783.336, 4817.470, 1957.274, 6006.033,  843.566,
				   4292.698, 2481.661, 3474.477,  469.943, 1274.304,
				   468.105, 1334.973, 2432.663, 1552.657,  843.844,
				   2697.861, 1628.495, 1750.051, 3310.430, 2876.284,
				   598.910,  249.479, 3783.287, 4248.057, 1339.975,
				   1090.331, 3085.055,  878.261, 1088.861, 1334.920,
				   877.939, 1615.421,  114.430, 2241.609,  933.821,
				   1929.366, 5214.102, 3759.877, 2080.868, 1554.548,
				   6175.246, 4500.597, 3310.467, 2275.338, 4500.482,
				   5303.108, 1070.658,  643.521, 5303.075,  642.783,
				   396.446, 3941.738, 1070.802, 2122.352, 2651.910,
				   3759.472, 1860.548, 2876.312, 2820.526, 5568.645,
				   4735.476, 2439.020, 3507.287,  195.941, 3057.583,
				   194.138, 2481.698,  795.608, 1861.395,  574.812,
				   2599.604, 2125.706, 1807.140, 4996.929, 2473.087,
				   2122.371, 5781.144, 1525.035, 1278.638, 1806.012,
				   1278.590, 1524.770, 2857.286, 4212.859, 5568.579,
				   843.870,  843.588, 3082.945, 3273.968, 1324.061,
				   4985.688, 1324.142, 1615.491, 3491.024, 3721.654,
				   3966.169, 2823.784, 4985.359, 4431.927, 4462.555,
				   2194.128, 1175.149, 4198.937, 4996.934,  781.151,
				   3233.020, 4462.554, 2194.506,  305.287, 3966.146,
				   3951.220, 1021.084, 2983.576,  204.769, 4704.382,
				   4211.619, 3507.264, 1070.766, 1070.663, 4699.981,
				   296.856, 3085.008, 3234.717, 1603.699, 1750.002,
				   1749.996, 2698.597, 3954.125, 3501.976, 2031.189,
				   2031.146, 1603.738, 2347.366, 3082.957, 2347.175,
				   2697.927, 4438.690, 2473.100, 4954.278, 5503.422,
				   1013.324, 5236.755, 5499.627, 2553.479, 4437.264,
				   4945.689, 2553.665,  454.437, 6073.421, 3670.434,
				   5778.495, 5502.745, 3393.347, 5221.660, 1274.325,
				   5794.981, 1324.112, 1324.027, 3671.315, 3501.961,
				   1447.678, 5221.664, 1929.378, 1270.447, 1909.601,
				   1909.597, 2288.798, 2288.797, 4699.971, 2605.269,
				   2605.263, 4212.952, 2955.979, 2955.947, 5764.000,
				   4948.620, 5214.614, 3340.958, 3759.890, 3340.831,
				   4696.902, 4211.691, 3759.476,  642.491,  410.673,
				   410.449, 2938.669, 5778.516, 2938.764, 5483.331,
				   4132.793,  432.185, 4696.932, 4133.231, 3828.792,
				   1603.772, 1603.736, 1554.569,  867.314, 1552.703,
				   100.876, 5484.907, 2891.967, 2891.967, 2241.619,
				   2241.629, 3243.748, 3243.747, 3629.312, 3629.306,
				   4048.348, 6053.731, 4048.325, 2857.352, 4500.564,
				   4500.483, 5503.426, 4985.694, 6051.916, 4985.448,
				   5502.660,  323.561, 1750.069, 3349.565, 3349.644,
				   1125.835, 6044.885, 4620.253,  552.343,  550.409,
				   4620.457, 4289.790, 1909.635, 1909.620, 6045.737,
				   1861.436, 2275.364, 1860.535, 3556.696, 3556.696,
				   1414.304, 3943.936, 3943.936, 3954.147, 2599.577,
				   2599.587, 4364.365, 4364.364, 4817.563, 4817.560,
				   5303.119, 5303.104, 5820.631, 5820.582, 3786.063,
				   3786.143, 5132.630, 1729.442, 2080.868, 5132.745,
				   598.908,  196.810, 5214.614,  732.106, 2241.601,
				   2241.594, 2194.556, 2194.148, 4281.893, 4281.893,
				   725.218,  702.436,  702.393, 2983.540, 4704.530,
				   4704.530, 2983.597, 5159.637, 5159.637, 2069.093,
				   5646.697, 5646.696, 3273.983, 4248.055, 4248.138,
				   5669.768, 5669.850, 2651.934, 2439.029,  951.903,
				   2599.573, 2599.570, 2553.648, 2553.462, 2432.685,
				   3393.395, 3393.375, 4438.628, 4735.487, 4735.475,
				   469.982,  878.247,  877.872, 5070.535, 5070.535,
				   1212.135,  319.584,  933.791, 5528.079, 5528.079,
				   2823.853, 2983.560, 2983.558, 2938.726, 2938.691,
				   5763.976, 2820.547,  795.650, 3057.589, 1511.109,
				   3828.839, 3828.828, 5248.140, 5248.181, 3721.760,
				   1846.048, 1090.379, 3393.367, 3393.366, 3234.728,
				   1067.776, 1067.770, 1088.889, 3349.657, 3349.593,
				   3233.065, 2213.253, 4289.873, 4289.868, 5785.812,
				   5785.881, 2609.006, 3491.045,  249.453, 1175.166,
				   4954.311, 3030.138, 3828.803, 3828.803, 3671.276,
				   3786.131, 3786.100, 3670.470,  468.164, 1339.936,
				   4198.961, 1278.718, 1278.662, 4776.305, 4776.302};

    static const double ensanche[522][4] ={{2.811,4.80,0.69,1.00},{3.050,5.30,0.64,0.85},{2.303,4.69,0.67,0.54},
					   {2.783,4.85,0.68,0.74},{2.873,5.38,0.54,0.89},{2.152,4.81,0.63,0.55},
					   {1.845,4.23,0.60,0.48},{2.100,4.29,0.63,0.52},{1.860,4.23,0.60,0.50},
					   {2.632,4.84,0.66,0.67},{2.152,4.57,0.66,0.65},{2.355,4.65,0.65,0.64},
					   {2.602,5.04,0.69,0.72},{1.612,3.98,0.61,0.43},{1.612,4.01,0.61,0.45},
					   {3.210,4.11,0.69,1.00},{2.438,4.68,0.71,0.68},{3.060,4.09,0.68,0.84},
					   {2.408,4.70,0.70,0.53},{2.670,4.78,0.70,0.78},{2.550,4.94,0.64,0.67},
					   {2.985,4.55,0.68,0.90},  
					   {1.216},{ .823},{ .823},
					   {2.877},{2.811},{2.273},
					   {2.767},{1.702},{2.039}, 
					   {2.702},{1.418},{1.716},
					   {1.527},{2.732},{1.225},
					   {1.252},{2.794},{ .758},
					   { .758},{1.867},{1.986},
					   {2.445},{2.453},{1.092}, 
					   {1.095},{2.764},{2.442},
					   { .994},{ .994},{2.397},
					   {2.841},{2.773},{2.986},
					   {2.897},{ .914},{ .914},
					   {2.690},{2.072},{2.687}, 
					   {2.693},{ .838},{ .838},
					   {2.664},{2.015},{1.758},
					   {1.489},{2.078},{1.409},
					   {2.619},{2.714},{1.278},
					   {1.326},{1.648},{2.590}, 
					   {1.151},{1.160},{2.590},
					   {2.770},{2.131},{2.942},
					   {1.054},{1.057},{2.498},
					   {2.664},{2.107},{2.563},
					   {2.524},{2.450},{2.450}, 
					   { .965},{ .965},{2.181},
					   {1.557},{2.427},{2.776},
					   {2.359},{2.584},{2.397},
					   {2.137},{1.708},{2.501},
					   {2.270},{2.181},{1.062}, 
					   {1.471},{1.711},{2.302},
					   {2.847},{1.281},{2.584},
					   {2.302},{1.344},{1.145},
					   {1.163},{2.894},{1.719},
					   {1.048},{1.051},{2.172}, 
					   {2.101},{2.149},{2.314},
					   {2.489},{2.616},{2.779},
					   {2.684},{ .953},{ .953},
					   {1.995},{2.270},{2.149},
					   {2.069},{1.752},{2.477}, 
					   {2.320},{1.927},{1.669},
					   {2.107},{2.595},{2.237},
					   {1.980},{1.989},{2.051},
					   {1.935},{2.039},{ .758},
					   {2.332},{1.388},{1.332}, 
					   {1.971},{2.838},{1.731},
					   {1.222},{1.779},{1.089},
					   {1.107},{1.329},{1.657},
					   {1.832},{ .983},{1.861},
					   { .985},{2.288},{1.601}, 
					   {1.850},{1.888},{1.666},
					   {1.728},{2.075},{1.684},
					   {2.163},{2.492},{1.782},
					   {1.782},{1.642},{1.826},
					   {2.184},{1.622},{1.847}, 
					   {1.758},{2.445},{2.797},
					   {1.965},{ .542},{2.036},
					   {1.859},{1.264},{1.663},
					   {1.708},{1.752},{1.607},
					   {1.737},{2.823},{ .962}, 
					   {2.486},{1.654},{1.429},
					   {1.095},{1.400},{1.432},
					   {1.533},{ .977},{1.921},
					   {1.471},{1.009},{ .873},
					   {1.367},{1.645},{ .876}, 
					   {1.672},{2.717},{1.492},
					   {1.311},{2.255},{1.240},
					   {1.347},{1.364},{1.814},
					   {1.438},{1.604},{ .408},
					   {1.216},{1.169},{2.483}, 
					   {1.036},{2.456},{1.734},
					   {2.083},{1.225},{2.557},
					   { .678},{1.666},{1.577},
					   {1.554},{1.453},{1.492},
					   {1.258},{1.462},{1.302}, 
					   {1.477},{1.293},{1.423},
					   {1.314},{ .935},{1.539},
					   {1.246},{1.240},{1.885},
					   {1.110},{ .968},{ .829},
					   {1.033},{2.350},{ .894}, 
					   { .912},{1.246},{1.065},
					   { .926},{1.204},{1.421},
					   { .977},{2.264},{ .764},
					   {1.506},{2.267},{1.074},
					   {1.391},{1.042},{2.560}, 
					   {1.281},{ .814},{1.684},
					   { .491},{2.847},{ .657},
					   {1.418},{1.278},{ .923},
					   { .909},{ .802},{2.513},
					   {1.341},{ .938},{ .684}, 
					   {1.057},{1.057},{1.350},
					   { .985},{1.160},{1.204},
					   {1.199},{ .838},{1.305},
					   {1.270},{1.284},{1.302},
					   { .808},{2.163},{ .678}, 
					   { .681},{1.873},{ .574},
					   { .592},{ .681},{ .787},
					   { .917},{ .873},{2.625},
					   { .527},{ .758},{ .938},
					   { .956},{ .382},{ .811}, 
					   {1.320},{ .530},{ .687},
					   { .669},{ .870},{1.900},
					   {1.938},{ .843},{2.104},
					   {1.444},{ .500},
					   { .675},{ .900},{ .900},
					   { .912},{1.042},
					   {1.042},{1.051},{1.148},
					   {1.145},{ .589},
					   { .766},{ .710},{1.190},
					   {1.157},{1.181},
					   { .873},{1.030},{1.142},
					   {2.669},{1.971},
					   {1.930},{ .494},{ .935},
					   { .740},{ .684},
					   { .539},{2.592},{1.524},
					   { .820},{ .320},
					   { .518},{ .503},{1.027},
					   {2.592},{1.086},
					   {2.681},{ .728},{ .796},
					   { .796},{ .391},
					   { .539},{ .935},{ .935},
					   {1.045},{1.042},
					   {1.086},{ .639},{1.083},
					   {1.944},{1.045},
					   {1.042},{ .784},{ .932},
					   { .630},{ .929},
					   { .781},{2.850},{1.569},
					   { .382},{ .666},
					   {2.388},{ .518},{ .405},
					   {2.175},{2.060},
					   { .740},{ .287},{ .405},
					   { .400},{ .719},
					   { .829},{1.832},{ .817},
					   { .722},{ .722},
					   {2.143},{ .858},{ .858},
					   {1.790},{ .326},
					   { .441},{ .959},{ .959},
					   { .988},{ .988},
					   { .938},{ .938},{ .829},
					   { .829},{ .320},
					   { .604},{ .334},{1.817},
					   {1.255},{ .669},
					   {2.625},{2.678},{1.554},
					   {2.362},{ .337},
					   { .334},{ .669},{ .619},
					   { .666},{ .666},
					   {2.157},{1.506},{1.492},
					   { .290},{ .790},
					   { .790},{ .370},{ .873},
					   { .873},{1.462},
					   { .885},{ .885},{1.716},
					   { .287},{ .515},
					   { .293},{ .636},{1.497},
					   {1.033},{2.456},
					   { .296},{ .296},{ .533},
					   { .488},{1.148},
					   { .272},{ .323},{1.571},
					   { .269},{ .450},
					   {2.737},{1.711},{1.666},
					   { .610},{ .610},
					   {2.510},{2.539},{2.231},
					   { .716},{ .716},
					   { .864},{ .275},{ .272},
					   { .441},{ .397},
					   {1.462},{ .885},{2.592},
					   {1.142},{2.465},
					   { .260},{ .293},{ .260},
					   { .400},{1.500},
					   {2.279},{1.891},{ .263},
					   { .263},{ .719},
					   {1.184},{1.181},{1.770},
					   { .370},{ .334},
					   { .725},{2.033},{ .257},
					   { .275},{ .257},
					   { .355},{1.773},{ .882},
					   {2.418},{2.282},
					   {1.382},{1.447},{ .257},
					   { .257},{ .645},
					   { .323},{ .299},{ .627},
					   {2.270},{2.045},
					   {1.219},{1.344},{1.335},
					   { .255},{ .263}};

    static const double gl[522] =       {3.00,1.00,  3.00,                          
					 1.00,3.00,1.00,1.00,
					 3.00,3.00,3.00,1.00,
					 1.00,1.00,3.00,1.00,
					 3.00,3.00,
					 1.00,
					 1.00,1.00,1.00,1.00,
					 3.00,  1.00,  3.00,  3.00,  1.00,
					 3.00,  3.00,  3.00,  3.00,  3.00,
					 3.00,  1.00,  1.00,  1.00,  1.00,
					 3.00,  1.00,  3.00,  1.00,  1.00,
					 3.00,  3.00,  3.00,  3.00,  1.00,
					 3.00,  1.00,  1.00,  3.00,  1.00,
					 1.00,  3.00,  3.00,  3.00,  3.00,
					 1.00,  1.00,  1.00,  1.00,  3.00,
					 1.00,  3.00,  3.00,  1.00,  3.00,
					 1.00,  3.00,  1.00,  3.00,  1.00,
					 3.00,  1.00,  3.00,  1.00,  1.00,
					 3.00,  1.00,  1.00,  1.00,  1.00,
					 3.00,  1.00,  3.00,  3.00,  3.00,
					 3.00,  3.00,  3.00,  1.00,  1.00,
					 3.00,  1.00,  3.00,  1.00,  1.00,
					 3.00,  1.00,  3.00,  1.00,  1.00,
					 3.00,  3.00,  3.00,  3.00,  3.00,
					 3.00,  1.00,  3.00,  1.00,  3.00,
					 1.00,  3.00,  3.00,  1.00,  3.00,
					 1.00,  1.00,  3.00,  1.00,  3.00,
					 3.00,  1.00,  1.00,  1.00,  1.00,
					 3.00,  3.00,  1.00,  1.00,  3.00,
					 1.00,  1.00,  1.00,  3.00,  3.00,
					 1.00,  3.00,  1.00,  1.00,  3.00,
					 3.00,  3.00,  1.00,  1.00,  3.00,
					 1.00,  1.00,  1.00,  1.00,  1.00,
					 1.00,  1.00,  3.00,  3.00,  1.00,
					 3.00,  1.00,  3.00,  3.00,  3.00,
					 3.00,  1.00,  3.00,  3.00,  3.00,
					 1.00,  1.00,  3.00,  1.00,  3.00,
					 3.00,  1.00,  3.00,  3.00,  1.00,
					 3.00,  1.00,  1.00,  1.00,  3.00,
					 1.00,  3.00,  3.00,  3.00,  3.00,
					 1.00,  3.00,  3.00,  3.00,  1.00,
					 1.00,  3.00,  3.00,  3.00,  3.00,
					 1.00,  1.00,  1.00,  3.00,  1.00,
					 3.00,  3.00,  1.00,  3.00,  1.00,
					 1.00,  1.00,  3.00,  3.00,  1.00,
					 1.00,  1.00,  3.00,  1.00,  1.00,
					 3.00,  1.00,  3.00,  1.00,  1.00,
					 1.00,  1.00,  1.00,  1.00,  3.00,
					 3.00,  1.00,  3.00,  3.00,  3.00,
					 1.00,  3.00,  1.00,  3.00,  3.00,
					 1.00,  3.00,  3.00,  1.00,  3.00,
					 3.00,  1.00,  1.00,  3.00,  1.00,
					 1.00,  3.00,  1.00,  3.00,  3.00,
					 1.00,  1.00,  3.00,  1.00,  1.00,
					 3.00,  3.00,  3.00,  3.00,  1.00,
					 3.00,  1.00,  3.00,  3.00,  1.00,
					 3.00,  1.00,  1.00,  3.00,  3.00,
					 3.00,  1.00,  3.00,  1.00,  1.00,
					 1.00,  1.00,  3.00,  1.00,  1.00,
					 1.00,  3.00,  1.00,  1.00,  3.00,
					 1.00,  1.00,  1.00,  3.00,  1.00,
					 3.00,  3.00,  3.00,  1.00,  1.00,
					 3.00,  3.00,  3.00,  1.00,  3.00,
					 3.00,  3.00,  3.00,  1.00,  1.00,
					 1.00,  3.00,  1.00,  1.00,  1.00,
					 1.00,  1.00,  1.00,  3.00,  3.00,
					 1.00,  1.00,  3.00,  3.00,  3.00,
					 3.00,  3.00,  1.00,  1.00,  3.00,
					 1.00,  1.00,  3.00,  1.00,  3.00,
					 1.00,  3.00,  1.00,  3.00,  3.00,
					 3.00,  1.00,  3.00,  1.00,  1.00,
					 3.00,  1.00,  3.00,  3.00,  3.00,
					 1.00,  3.00,  1.00,  1.00,  3.00,
					 3.00,  3.00,  3.00,  1.00,  3.00,
					 3.00,  1.00,  1.00,  1.00,  3.00,
					 1.00,  1.00,  3.00,  1.00,  1.00,
					 3.00,  1.00,  3.00,  3.00,  1.00,
					 1.00,  1.00,  3.00,  1.00,  3.00,
					 1.00,  3.00,  1.00,  3.00,  3.00,
					 1.00,  3.00,  1.00,  1.00,  3.00,
					 3.00,  1.00,  1.00,  1.00,  3.00,
					 3.00,  1.00,  1.00,  3.00,  3.00,
					 3.00,  3.00,  1.00,  1.00,  3.00,
					 1.00,  3.00,  1.00,  1.00,  3.00,
					 1.00,  1.00,  3.00,  3.00,  1.00,
					 1.00,  3.00,  3.00,  1.00,  3.00,
					 1.00,  3.00,  3.00,  3.00,  1.00,
					 1.00,  3.00,  1.00,  3.00,  3.00,
					 1.00,  1.00,  3.00,  3.00,  1.00,
					 1.00,  1.00,  3.00,  1.00,  1.00,
					 3.00,  3.00,  3.00,  1.00,  1.00,
					 1.00,  3.00,  3.00,  1.00,  3.00,
					 1.00,  3.00,  1.00,  1.00,  1.00,
					 1.00,  3.00,  3.00,  1.00,  3.00,
					 3.00,  1.00,  3.00,  3.00,  1.00,
					 1.00,  3.00,  1.00,  1.00,  3.00,
					 3.00,  1.00,  3.00,  3.00,  1.00,
					 3.00,  3.00,  1.00,  1.00,  3.00,
					 3.00,  1.00,  3.00,  3.00,  1.00,
					 1.00,  3.00,  1.00,  3.00,  1.00,
					 3.00,  1.00,  1.00,  3.00,  1.00,
					 3.00,  1.00,  3.00,  3.00,  1.00,
					 3.00,  1.00,  3.00,  1.00,  3.00,
					 1.00,  3.00,  1.00,  3.00,  1.00,
					 1.00,  1.00,  3.00,  1.00,  3.00,
					 1.00,  3.00,  1.00,  3.00,  3.00,
					 3.00,  1.00,  3.00,  1.00,  3.00};

    static const int ifin1[800] = { 226, 226, 226, 226, 226, 227, 228, 228, 228, 228,
				    229, 229, 229, 229, 229, 229, 229, 229, 230, 230,
				    230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
				    230, 230, 230, 230, 230, 230, 231, 231, 231, 231,
				    231, 231, 231, 231, 231, 231, 231, 231, 231, 231,
				    232, 233, 233, 233, 233, 233, 233, 233, 233, 233,
				    234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
				    234, 234, 234, 234, 234, 234, 234, 234, 234, 234,
				    234, 234, 234, 234, 235, 235, 235, 235, 235, 235,
				    235, 236, 238, 238, 240, 243, 244, 244, 244, 244,
				    245, 245, 246, 246, 246, 246, 246, 246, 247, 247,
				    247, 247, 248, 248, 248, 248, 248, 249, 249, 249,
				    250, 250, 250, 250, 250, 251, 251, 251, 251, 251,
				    251, 251, 252, 252, 252, 253, 253, 253, 253, 253,
				    253, 253, 253, 253, 253, 253, 253, 253, 253, 253,
				    253, 255, 255, 256, 256, 256, 256, 256, 256, 256,
				    256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
				    256, 256, 257, 257, 257, 257, 258, 258, 258, 258,
				    258, 258, 258, 258, 258, 259, 259, 259, 259, 259,
				    259, 259, 259, 259, 259, 259, 259, 259, 259, 259,
				    259, 259, 259, 259, 259, 259, 259, 259, 259, 259,
				    259, 260, 260, 260, 260, 261, 261, 261, 261, 261,
				    261, 261, 261, 261, 262, 262, 262, 262, 262, 262,
				    262, 262, 262, 262, 262, 262, 263, 263, 263, 263,
				    263, 263, 264, 264, 264, 264, 264, 265, 266, 267,
				    267, 267, 267, 267, 267, 267, 267, 267, 267, 267,
				    267, 267, 267, 267, 267, 267, 268, 268, 268, 268,
				    269, 269, 269, 269, 269, 269, 269, 269, 269, 269,
				    269, 269, 269, 269, 269, 269, 269, 269, 269, 269,
				    269, 269, 269, 269, 269, 269, 269, 270, 270, 270,
				    270, 270, 270, 270, 271, 271, 271, 271, 271, 271,
				    271, 271, 271, 271, 271, 271, 271, 271, 271, 271,
				    271, 272, 272, 272, 272, 272, 272, 272, 272, 272,
				    272, 273, 273, 273, 273, 273, 273, 273, 273, 273,
				    273, 273, 273, 273, 273, 273, 273, 274, 274, 275,
				    275, 276, 276, 277, 277, 277, 277, 277, 277, 277,
				    277, 277, 277, 277, 278, 278, 278, 278, 278, 278,
				    278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
				    278, 278, 279, 279, 279, 279, 279, 279, 280, 280,
				    281, 281, 281, 281, 281, 281, 281, 281, 281, 282,
				    282, 282, 282, 282, 282, 282, 282, 282, 282, 282,
				    282, 283, 283, 283, 283, 283, 284, 284, 284, 284,
				    285, 285, 285, 285, 285, 286, 286, 287, 288, 288,
				    288, 288, 290, 291, 294, 294, 296, 296, 297, 298,
				    299, 299, 299, 299, 299, 299, 299, 299, 299, 299,
				    299, 299, 300, 300, 300, 300, 300, 300, 300, 300,
				    300, 300, 301, 301, 301, 302, 302, 302, 302, 303,
				    303, 303, 303, 303, 303, 303, 303, 303, 303, 303,
				    303, 303, 303, 303, 303, 303, 303, 303, 304, 304,
				    304, 304, 304, 304, 304, 304, 304, 304, 304, 304,
				    304, 304, 304, 304, 305, 305, 305, 305, 305, 305,
				    305, 305, 305, 305, 305, 305, 305, 305, 305, 305,
				    305, 305, 305, 305, 305, 305, 305, 305, 305, 305,
				    305, 305, 306, 307, 307, 307, 307, 307, 307, 307,
				    307, 307, 307, 307, 307, 308, 308, 308, 308, 308,
				    308, 308, 308, 308, 308, 308, 308, 308, 308, 308,
				    308, 308, 308, 308, 309, 309, 309, 309, 309, 309,
				    309, 311, 311, 311, 311, 311, 312, 312, 312, 312,
				    312, 312, 312, 312, 312, 312, 312, 312, 312, 312,
				    312, 312, 312, 312, 312, 312, 312, 312, 312, 312,
				    312, 312, 312, 312, 312, 312, 312, 312, 312, 312,
				    312, 312, 312, 312, 312, 312, 312, 313, 313, 313,
				    313, 313, 313, 314, 314, 314, 314, 314, 314, 314,
				    314, 314, 314, 314, 314, 314, 315, 315, 315, 315,
				    315, 315, 315, 315, 315, 315, 316, 316, 316, 316,
				    317, 318, 318, 318, 318, 318, 318, 318, 318, 318,
				    318, 318, 318, 319, 320, 320, 321, 321, 321, 321,
				    321, 321, 321, 321, 321, 321, 321, 321, 321, 321,
				    321, 322, 322, 322, 322, 324, 324, 324, 324, 324,
				    324, 324, 324, 324, 324, 324, 324, 324, 324, 324,
				    324, 324, 324, 324, 324, 324, 324, 325, 325, 325,
				    326, 326, 326, 326, 327, 327, 327, 327, 328, 328,
				    328, 328, 328, 328, 330, 330, 331, 331, 331, 331,
				    331, 331, 331, 331, 331, 331, 331, 333, 333, 333,
				    333, 333, 333, 333, 333, 333, 333, 334, 336, 337,
				    337, 339, 339, 339, 340, 342, 343, 343, 343, 345,
				    345, 345, 345, 345, 345, 345, 345, 345, 345, 345,
				    345, 345, 345, 345, 345, 345, 345, 345, 345, 345,
				    345, 345, 345, 345, 345, 346, 346, 346, 347, 347,
				    347, 347, 348, 348, 348, 348, 348, 348, 348, 348};

    static const int ini1[800]={   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				   1,   1,   1,   1,   1,   1,   1,   1,   1,   1};

    static const int ifin2[800]={  75,  75,  75,  75,  75,  75,  75,  75,  76,  76,
				   76,  77,  77,  78,  78,  78,  78,  78,  78,  78,
				   79,  79,  79,  79,  79,  79,  79,  79,  79,  79,
				   79,  79,  79,  79,  79,  79,  79,  80,  80,  80,
				   80,  80,  80,  80,  80,  80,  80,  80,  80,  80,
				   80,  80,  80,  80,  80,  80,  80,  80,  80,  80,
				   80,  80,  80,  80,  80,  80,  80,  80,  80,  80,
				   80,  80,  80,  80,  80,  80,  80,  80,  80,  80,
				   80,  81,  82,  82,  82,  82,  82,  82,  84,  84,
				   84,  84,  84,  84,  84,  84,  84,  84,  85,  85,
				   85,  85,  85,  85,  85,  85,  85,  85,  85,  85,
				   85,  86,  86,  86,  86,  86,  86,  86,  86,  86,
				   86,  86,  87,  87,  87,  87,  87,  87,  87,  87,
				   87,  87,  88,  88,  88,  88,  88,  88,  88,  88,
				   88,  88,  88,  88,  88,  88,  88,  88,  88,  88,
				   88,  88,  88,  88,  88,  88,  88,  88,  88,  89,
				   89,  89,  89,  89,  89,  89,  89,  89,  89,  89,
				   89,  89,  90,  90,  91,  91,  91,  91,  94,  95,
				   95,  95,  95,  96,  96,  96,  96,  96,  96,  96,
				   96,  96,  96,  96,  96,  96,  97,  97,  97,  97,
				   97,  97,  97,  97,  97,  97,  97,  97,  97,  97,
				   97,  97,  97,  97,  98,  98,  98,  98,  98,  98,
				   98,  98,  98,  99,  99,  99,  99,  99,  99,  99,
				   99, 100, 100, 100, 100, 100, 100, 100, 100, 100,
				   100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
				   100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
				   100, 100, 100, 100, 100, 100, 101, 101, 101, 101,
				   101, 101, 101, 101, 102, 102, 102, 102, 102, 102,
				   102, 102, 102, 102, 103, 103, 104, 105, 105, 105,
				   105, 105, 105, 105, 105, 105, 105, 105, 105, 105,
				   105, 106, 106, 106, 106, 106, 106, 106, 106, 107,
				   108, 108, 108, 108, 108, 109, 109, 109, 109, 109,
				   110, 110, 111, 111, 111, 111, 111, 111, 111, 111,
				   111, 111, 112, 112, 112, 112, 112, 112, 112, 112,
				   112, 112, 112, 113, 113, 114, 114, 114, 114, 114,
				   114, 114, 114, 114, 114, 114, 115, 115, 115, 115,
				   115, 116, 116, 116, 116, 116, 116, 116, 116, 116,
				   116, 116, 116, 116, 116, 116, 116, 116, 116, 116,
				   116, 116, 116, 116, 116, 116, 116, 117, 117, 117,
				   117, 117, 117, 117, 117, 117, 117, 117, 117, 117,
				   117, 118, 118, 118, 118, 118, 118, 118, 118, 118,
				   118, 118, 118, 118, 118, 118, 118, 118, 119, 120,
				   120, 120, 120, 120, 121, 121, 121, 121, 121, 121,
				   121, 121, 122, 122, 122, 122, 122, 122, 122, 122,
				   123, 123, 125, 125, 125, 125, 125, 125, 125, 125,
				   125, 125, 125, 125, 125, 125, 125, 125, 125, 125,
				   125, 125, 125, 125, 125, 125, 125, 125, 125, 125,
				   125, 125, 125, 125, 125, 125, 125, 125, 125, 125,
				   125, 126, 126, 126, 127, 128, 128, 128, 128, 128,
				   128, 128, 128, 128, 128, 130, 131, 131, 131, 133,
				   133, 133, 134, 134, 134, 134, 135, 136, 136, 136,
				   136, 136, 136, 136, 136, 136, 136, 136, 136, 136,
				   136, 136, 137, 137, 137, 137, 137, 137, 137, 137,
				   137, 137, 137, 137, 137, 137, 137, 138, 138, 138,
				   138, 138, 138, 138, 138, 138, 138, 138, 138, 138,
				   138, 138, 138, 138, 138, 138, 138, 138, 138, 139,
				   139, 139, 139, 140, 140, 140, 140, 140, 141, 141,
				   141, 141, 141, 141, 141, 142, 142, 142, 142, 142,
				   142, 142, 142, 143, 144, 144, 144, 144, 144, 144,
				   144, 145, 145, 145, 145, 145, 145, 145, 145, 145,
				   145, 145, 145, 145, 145, 146, 146, 146, 146, 146,
				   146, 146, 146, 146, 146, 147, 147, 147, 147, 147,
				   147, 148, 148, 149, 149, 149, 149, 149, 149, 149,
				   149, 149, 149, 149, 149, 149, 149, 149, 149, 149,
				   149, 149, 149, 149, 149, 149, 149, 149, 149, 149,
				   149, 149, 149, 149, 150, 150, 150, 150, 150, 150,
				   150, 150, 151, 151, 151, 152, 153, 153, 153, 153,
				   153, 153, 153, 153, 153, 154, 154, 155, 155, 155,
				   155, 155, 155, 156, 156, 156, 156, 156, 156, 156,
				   156, 156, 156, 156, 156, 156, 156, 156, 156, 156,
				   156, 157, 157, 157, 157, 157, 157, 157, 157, 157,
				   157, 157, 157, 157, 158, 158, 158, 158, 158, 158,
				   158, 158, 158, 158, 158, 158, 158, 158, 158, 158,
				   158, 158, 158, 158, 159, 159, 159, 159, 159, 160,
				   160, 160, 160, 160, 160, 160, 160, 160, 161, 163,
				   163, 163, 164, 164, 164, 165, 165, 165, 165, 165,
				   165, 165, 166, 166, 166, 166, 166, 166, 167, 168,
				   168, 168, 168, 168, 169, 169, 169, 169, 169, 169,
				   169, 169, 169, 169, 169, 170, 170, 170, 170, 170,
				   170, 170, 170, 170, 170, 170, 170, 170, 170, 170};

    static const int ini2[800]= {   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				    1,   1,   1,   1,   1,   1,   1,   1,   1,   1};

    static const int ifin3[800] = {   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				      1,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				      2,   2,   3,   3,   3,   3,   3,   3,   3,   3,
				      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
				      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
				      3,   3,   3,   4,   4,   4,   4,   4,   4,   4,
				      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
				      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
				      5,   5,   5,   5,   5,   6,   6,   6,   6,   6,
				      6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
				      6,   6,   6,   6,   6,   6,   6,   6,   6,   7,
				      8,   8,   9,   9,  10,  10,  10,  10,  10,  10,
				      10,  10,  10,  10,  10,  11,  11,  12,  12,  12,
				      12,  12,  12,  12,  13,  13,  13,  13,  13,  13,
				      13,  13,  15,  15,  15,  15,  15,  15,  15,  15,
				      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
				      15,  15,  15,  15,  15,  15,  15,  15,  16,  16,
				      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
				      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
				      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
				      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
				      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
				      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
				      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
				      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				      18,  18,  18,  18,  18,  18,  19,  19,  19,  19,
				      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
				      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
				      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
				      19,  19,  19,  19,  19,  19,  19,  19,  20,  20,
				      20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
				      20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
				      20,  20,  20,  20,  20,  21,  21,  21,  21,  21,
				      21,  21,  21,  21,  22,  22,  22,  22,  22,  22,
				      22,  23,  23,  23,  23,  23,  23,  23,  23,  23,
				      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
				      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
				      23,  23,  23,  23,  23,  23,  23,  25,  25,  25,
				      25,  25,  25,  25,  25,  25,  25,  25,  25,  26,
				      26,  26,  26,  26,  26,  26,  26,  27,  27,  27,
				      27,  27,  27,  27,  27,  27,  27,  27,  27,  27,
				      27,  27,  27,  28,  28,  28,  28,  30,  30,  31,
				      31,  32,  32,  32,  33,  33,  34,  34,  34,  34,
				      34,  34,  34,  34,  34,  35,  35,  35,  35,  35,
				      35,  35,  35,  35,  36,  36,  36,  36,  37,  37,
				      38,  38,  38,  38,  39,  39,  39,  39,  41,  41,
				      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
				      41,  41,  41,  41,  41,  41,  42,  42,  42,  43,
				      43,  43,  43,  43,  43,  43,  43,  43,  44,  44,
				      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
				      44,  45,  45,  45,  45,  45,  45,  47,  47,  47,
				      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
				      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
				      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
				      47,  47,  47,  47,  47,  48,  48,  48,  48,  48,
				      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
				      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
				      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
				      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
				      49,  49,  49,  49,  49,  49,  51,  51,  51,  51,
				      51,  51,  51,  51,  51,  51,  51,  51,  51,  51,
				      51,  52,  52,  52,  52,  52,  52,  52,  52,  52,
				      52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
				      52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
				      52,  53,  53,  53,  53,  53,  53,  53,  53,  53,
				      53,  53,  53,  53,  53,  53,  53,  53,  53,  53};

    static const int ini3[800] = {   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				 1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				 1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				 1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				 1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				 1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				 1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
				 1,   1,   2,   2,   2,   2,   2,   2,   2,   2,
				 2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				 2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				 2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				 2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				 2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				 2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
				 2,   3,   3,   3,   3,   3,   3,   3,   3,   3,
				 3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
				 3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
				 3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
				 3,   4,   4,   5,   5,   5,   5,   5,   5,   5,
				 5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
				 5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
				 6,   6,   6,   6,   6,   7,   7,   7,   7,   7,
				 7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
				 7,   7,   7,   7,   7,   7,   7,   7,   7,   8,
				 9,   9,  10,  10,  11,  11,  11,  11,  11,  11,
				 11,  11,  11,  11,  11,  12,  12,  13,  13,  13,
				 13,  13,  13,  13,  14,  14,  14,  14,  14,  14,
				 14,  14,  16,  16,  16,  16,  16,  16,  16,  16,
				 16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
				 16,  16,  16,  16,  16,  16,  16,  16,  17,  17,
				 17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
				 17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
				 17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
				 17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
				 17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
				 17,  17,  17,  17,  17,  17,  18,  18,  18,  18,
				 18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				 18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				 18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				 18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				 18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				 18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				 18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
				 18,  18,  18,  19,  19,  19,  19,  19,  19,  19,
				 19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
				 19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
				 19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
				 19,  19,  19,  20,  20,  20,  20,  20,  21,  21,
				 21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
				 21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
				 21,  21,  21,  21,  21,  22,  22,  22,  22,  22,
				 22,  22,  22,  22,  23,  23,  23,  23,  23,  23,
				 23,  24,  24,  24,  24,  24,  24,  24,  24,  24,
				 24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
				 24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
				 24,  24,  24,  24,  24,  24,  24,  26,  26,  26,
				 26,  26,  26,  26,  26,  26,  26,  26,  26,  27,
				 27,  27,  27,  27,  27,  27,  27,  28,  28,  28,
				 28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
				 28,  28,  28,  29,  29,  29,  29,  31,  31,  32,
				 32,  33,  33,  33,  34,  34,  35,  35,  35,  35,
				 35,  35,  35,  35,  35,  36,  36,  36,  36,  36,
				 36,  36,  36,  36,  37,  37,  37,  37,  38,  38,
				 39,  39,  39,  39,  40,  40,  40,  40,  42,  42,
				 42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
				 42,  42,  42,  42,  42,  42,  43,  43,  43,  44,
				 44,  44,  44,  44,  44,  44,  44,  44,  45,  45,
				 45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
				 45,  46,  46,  46,  46,  46,  46,  48,  48,  48,
				 48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
				 48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
				 48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
				 48,  48,  48,  48,  48,  49,  49,  49,  49,  49,
				 49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
				 50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
				 50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
				 50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
				 50,  50,  50,  50,  50,  50,  52,  52,  52,  52,
				 52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
				 52,  53,  53,  53,  53,  53,  53,  53,  53,  53};

  

    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=1.855;  //Debyes

    double q=0.034278209*pow(tt.get("K"),1.5);    // Q(300 K)=178.120 JPL Line Catalog
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
  
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
            
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
        
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      if(ifin==0||ifin<ini){

	return complex<double> (0.0,0.0);

      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){

	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening_water(Frequency(fre[i],"GHz"),tt,pp,eh2o,ensanche[i][0],ensanche[i][1],ensanche[i][2],ensanche[i][3]),Frequency(0.0,"GHz"));

	  lshape=lshape*flin[i]*gl[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 

          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(fac2fixed*pow(mu,2.0)/q); // imaginary part: absorption coefficient in cm^2
	                                                                    // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )
	
      }
     
    }
 
  }


  complex<double>  RefractiveIndexProfile::mkAbs_hh17o(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){


    static const double fre[16]={ 
      13.522059,194.001224,323.819514,385.783293,
      482.981450,481.712039,489.015848,469.795466,
      514.973492,507.838397,552.020778,658.391131,
      748.458444,944.845857,987.845579,991.503015};

    static const double gl[16]={
      3.,1.,1.,3.,1.,3.,3.,
      3.,1.,1.,3.,3.,1.,1.,1.,1.};

    static const double flin[16]={ 
      .0548,.101,.088,.121,.0891,.0102,.0891,
      .132,.102,.119,1.5,.122,2.073,.164,.262,.760};

    static const double el[16]={
      642.1,204.2,468.9,322.7,
      1518.4,1084.6,1518.7,430.9,1086.2,722.6,
      60.7,729.8,136.7,453.4,597.3,100.7};

    static const double dv0[16]={
      2.85,2.68,3.03,3.19,1.5,1.94,1.51,
      2.47,1.89,2.07,3.33,2.28,3.13,2.59,2.48,3.09};
    
    static const double dvlm[16]={
      13.68,14.49,15.21,15.84,7.94,10.44,8.13,14.24,
      10.56,11.95,14.66,12.78,13.93,14.06,14.16,15.20};

    static const double temp_exp[16]={
      .626,.649,.619,.63,.29,.36,.332,.51,
      .380,.38,.645,.6,.69,.676,.56,.66};


    static const int ifin1[500]={
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   7,   7,   7,   7,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16};

    static const int ini1[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   7,   7,   7,   7,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15};

    static const int ifin2[500]={
      1,   1,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   7,   7,   7,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  16,
      16,  16,  16,  16,  16,  16,  16,  16,  16,  16};

    static const int ini2[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   7,   7,   7,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  16,  16};

    static const int ifin3[500]={
      1,   1,   1,   1,   1,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      7,   7,   7,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  16,  16,  16,  16,  16,   0,   0,   0};

    static const int ini3[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   7,   7,   7,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  16,  16,   0,   0,   0};


    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=1.855;  //Debyes

    double q=0.034412578*pow(tt.get("K"),1.5);   // Q(300 K)=178.813   JPL Line Catalog
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
  
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
      
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
      
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }
      
      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      
      if(ifin==0||ifin<ini){
	
	return complex<double> (0.0,0.0);
	
      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){
	  
	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening_hh18o_hh17o(Frequency(fre[i],"GHz"),tt,pp,eh2o,dv0[i],dvlm[i],temp_exp[i]),Frequency(0.0,"GHz"));
	  
	  lshape=lshape*flin[i]*gl[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 
	  
          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(fac2fixed*pow(mu,2.0)/q); // imaginary part: absorption coefficient in cm^2
	                                                                    // real part: delay in rad*cm^2    
	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )
	
      }
     
    }
 
  }

  complex<double>  RefractiveIndexProfile::mkAbs_hdo(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){

    static const double fre[58]={
      5.70284, 10.27828, 22.30749, 50.23643, 64.42724, 80.57828,
      143.72739,151.61604,207.11122,241.56151,305.03867,310.53348,
      464.92450,481.77936,537.79272,540.37543,559.81665,797.48669,
      848.96191,862.66339,890.26971,919.31085, 20.46006, 61.18605,
      87.96289,120.77818,138.53059,207.34569,225.89665,241.97346,
      255.05026,266.16098,313.75076,317.15128,335.39542,382.06503,
      479.94748,490.59668,509.29245,539.93585,599.92676,622.48346,
      753.41113,766.16309,774.77631,827.26306,836.99475,838.66260,
      838.95593,853.55231,859.35687,893.63861,895.87488,904.89404,
      938.41833,958.56860,984.13788,995.41144};

    static const double flin[58]={
      0.401E+01,0.332E+01,0.322E+01,0.227E+01,0.264E+01,0.150E+01,
      0.168E+01,0.219E+01,0.178E-01,0.833E+00,0.181E+01,0.130E+01,
      0.100E+01,0.587E+00,0.632E-02,0.150E+01,0.103E+01,0.463E+00,
      0.150E+01,0.126E+01,0.848E+00,0.199E+01,0.223E+00,0.298E+00,
      0.448E+00,0.238E+00,0.200E+00,0.105E+00,0.228E+00,0.460E+00,
      0.338E+00,0.132E+00,0.450E+00,0.270E+00,0.138E+00,0.267E+00,
      0.131E+00,0.633E+00,0.150E+01,0.629E+00,0.228E+01,0.548E+00,
      0.275E+01,0.231E+00,0.231E+00,0.580E+00,0.108E+00,0.789E+00,
      0.567E+00,0.670E+00,0.108E+00,0.100E+01,0.589E+00,0.414E+00,
      0.511E+00,0.787E+00,0.290E+01,0.142E+01};

    static const double el[58]={
      425.1, 156.7, 537.6, 223.6, 672.7,  42.9,
      312.3, 830.1, 216.1,  83.6,1009.6, 422.5,
      0.0, 144.5, 131.4,1210.7, 553.8, 225.0,
      42.9,1432.9, 705.7,  22.3, 225.0, 422.5,
      825.9, 319.2, 437.5, 738.4, 156.8, 825.8,
      425.2, 144.4, 538.7, 675.8, 319.2, 672.7,
      312.3,  42.9,  22.4, 983.7,  66.4, 675.8,
      131.4, 983.6, 983.2, 223.6, 538.7,1558.1,
      837.3, 983.2, 537.7,   0.0, 537.7,1236.6,
      1024.2,1164.6, 216.1,  83.6};

      static const double tr[58]={
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1};
    
    static const int ifin11[500]={
      3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   5,   5,   5,   5,   5,
      5,   5,   6,   6,   6,   6,   6,   6,   6,   6,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   8,   8,   8,   8,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  12,  12,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  16,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  20,  20,  20,  20,  20,  20,
      20,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini11[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   2,   2,   3,   3,   3,   3,   3,
      3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   5,   5,   5,   5,   5,
      5,   5,   6,   6,   6,   6,   6,   6,   6,   6,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   8,   8,   8,   8,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  12,  12,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  16,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  20,  20,  20,  20,  20,  20,
      20,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ifin21[500]={
      2,   2,   3,   3,   3,   3,   3,   3,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   5,   5,   5,   5,   5,   5,   5,   6,
      6,   6,   6,   6,   6,   6,   6,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   8,
      8,   8,   8,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      12,  12,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  16,  17,  17,  17,
      17,  17,  17,  17,  17,  17,  17,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  20,  20,  20,  20,  20,  20,  20,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini21[500]={
      1,   1,   1,   1,   1,   1,   2,   2,   3,   3,
      3,   3,   3,   3,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   5,   5,
      5,   5,   5,   5,   5,   6,   6,   6,   6,   6,
      6,   6,   6,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   8,   8,   8,   8,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  12,  12,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  16,  17,  17,  17,  17,  17,  17,  17,
      17,  17,  17,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  20,  20,  20,
      20,  20,  20,  20,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  21,  21,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,  22,  22,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ifin31[500]={
      1,   1,   2,   2,   3,   3,   3,   3,   3,   3,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   5,   5,   5,   5,   5,   5,
      5,   6,   6,   6,   6,   6,   6,   6,   6,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   8,   8,   8,   8,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  12,  12,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  16,  17,
      17,  17,  17,  17,  17,  17,  17,  17,  17,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  20,  20,  20,  20,  20,  20,  20,
      21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  22,  22,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini31[500]={
      1,   1,   1,   1,   2,   2,   3,   3,   3,   3,
      3,   3,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   5,   5,   5,   5,
      5,   5,   5,   6,   6,   6,   6,   6,   6,   6,
      6,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
      7,   7,   7,   8,   8,   8,   8,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
      11,  11,  11,  11,  12,  12,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      16,  17,  17,  17,  17,  17,  17,  17,  17,  17,
      17,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
      19,  19,  19,  19,  19,  20,  20,  20,  20,  20,
      20,  20,  21,  21,  21,  21,  21,  21,  21,  21,
      21,  21,  21,  21,  21,  21,  22,  22,  22,  22,
      22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
      22,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ifin12[500]={
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  25,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  25,  26,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
      27,  27,  27,  27,  27,  27,  27,  27,  27,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  30,  30,  30,  30,  30,  30,  30,
      30,  31,  31,  31,  31,  31,  31,  31,  32,  32,
      32,  32,  32,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  34,  34,  35,
      35,  35,  35,  35,  35,  35,  35,  35,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      38,  38,  38,  38,  38,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  44,  44,  44,
      44,  44,  44,  45,  45,  45,  45,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  47,  47,  47,  47,  48,  50,
      50,  50,  50,  50,  50,  50,  50,  51,  51,  51,
      52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
      52,  52,  52,  52,  52,  52,  52,  53,  54,  54,
      54,  54,  55,  55,  55,  55,  55,  55,  55,  55,
      55,  55,  55,  55,  55,  55,  55,  55,  55,  56,
      56,  56,  56,  56,  56,  56,  56,  56,  56,  57,
      57,  57,  57,  57,  57,  57,  57,  57,  57,  57,
      57,  57,  58,  58,  58,  58,  58,  58,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   };
    
    static const int ini12[500]={
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  25,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  25,  26,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
      27,  27,  27,  27,  27,  27,  27,  27,  27,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  29,  29,  29,  29,  29,  29,
      29,  29,  29,  30,  30,  30,  30,  30,  30,  30,
      30,  31,  31,  31,  31,  31,  31,  31,  32,  32,
      32,  32,  32,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  34,  34,  35,
      35,  35,  35,  35,  35,  35,  35,  35,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      38,  38,  38,  38,  38,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  44,  44,  44,
      44,  44,  44,  45,  45,  45,  45,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  47,  47,  47,  47,  48,  50,
      50,  50,  50,  50,  50,  50,  50,  51,  51,  51,
      52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
      52,  52,  52,  52,  52,  52,  52,  53,  54,  54,
      54,  54,  55,  55,  55,  55,  55,  55,  55,  55,
      55,  55,  55,  55,  55,  55,  55,  55,  55,  56,
      56,  56,  56,  56,  56,  56,  56,  56,  56,  57,
      57,  57,  57,  57,  57,  57,  57,  57,  57,  57};


    static const int ifin22[500]={
      23,  23,  23,  23,  23,  23,  23,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  25,  25,
      25,  25,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  26,  26,  26,  26,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  26,  27,  27,  27,
      27,  27,  27,  27,  27,  27,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  29,  29,  29,  29,  29,  29,  29,  29,  29,
      30,  30,  30,  30,  30,  30,  30,  30,  31,  31,
      31,  31,  31,  31,  31,  32,  32,  32,  32,  32,
      33,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  34,  34,  35,  35,  35,  35,
      35,  35,  35,  35,  35,  36,  36,  36,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  38,  38,  38,
      38,  38,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  44,  44,  44,  44,  44,  44,
      45,  45,  45,  45,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  47,  47,  47,  47,  48,  50,  50,  50,  50,
      50,  50,  50,  50,  51,  51,  51,  52,  52,  52,
      52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
      52,  52,  52,  52,  53,  54,  54,  54,  54,  55,
      55,  55,  55,  55,  55,  55,  55,  55,  55,  55,
      55,  55,  55,  55,  55,  55,  56,  56,  56,  56,
      56,  56,  56,  56,  56,  56,  57,  57,  57,  57,
      57,  57,  57,  57,  57,  57,  57,  57,  57,  58,
      58,  58,  58,  58,  58,   0,   0,   0,   0,   0};

    static const int ini22[500]={
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  23,  23,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  25,  25,  25,  25,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
      26,  26,  26,  27,  27,  27,  27,  27,  27,  27,
      27,  27,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  29,  29,  29,
      29,  29,  29,  29,  29,  29,  30,  30,  30,  30,
      30,  30,  30,  30,  31,  31,  31,  31,  31,  31,
      31,  32,  32,  32,  32,  32,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      34,  34,  35,  35,  35,  35,  35,  35,  35,  35,
      35,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  36,  36,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  38,  38,  38,  38,  38,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      44,  44,  44,  44,  44,  44,  45,  45,  45,  45,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  47,  47,  47,
      47,  48,  50,  50,  50,  50,  50,  50,  50,  50,
      51,  51,  51,  52,  52,  52,  52,  52,  52,  52,
      52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
      53,  54,  54,  54,  54,  55,  55,  55,  55,  55,
      55,  55,  55,  55,  55,  55,  55,  55,  55,  55,
      55,  55,  56,  56,  56,  56,  56,  56,  56,  56,
      56,  56,  57,  57,  57,  57,  57,  57,  57,  57,
      57,  57,  57,  57,  57,  58,  58,  58,  58,  58};

    static const int ifin32[500]={
      23,  23,  23,  23,  23,  23,  23,  23,  23,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      25,  25,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  26,  26,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  26,  26,  26,  27,
      27,  27,  27,  27,  27,  27,  27,  27,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  29,  29,  29,  29,  29,  29,  29,
      29,  29,  30,  30,  30,  30,  30,  30,  30,  30,
      31,  31,  31,  31,  31,  31,  31,  32,  32,  32,
      32,  32,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  34,  34,  35,  35,
      35,  35,  35,  35,  35,  35,  35,  36,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  38,
      38,  38,  38,  38,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  44,  44,  44,  44,
      44,  44,  45,  45,  45,  45,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  47,  47,  47,  47,  48,  50,  50,
      50,  50,  50,  50,  50,  50,  51,  51,  51,  52,
      52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
      52,  52,  52,  52,  52,  52,  53,  54,  54,  54,
      54,  55,  55,  55,  55,  55,  55,  55,  55,  55,
      55,  55,  55,  55,  55,  55,  55,  55,  56,  56,
      56,  56,  56,  56,  56,  56,  56,  56,  57,  57,
      57,  57,  57,  57,  57,  57,  57,  57,  57,  57,
      57,  58,  58,  58,  58,  58,  58,   0,   0,   0};

    static const int ini32[500]={
      23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
      23,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
      24,  24,  25,  25,  25,  25,  25,  25,  25,  25,
      25,  25,  25,  25,  25,  26,  26,  26,  26,  26,
      26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
      26,  27,  27,  27,  27,  27,  27,  27,  27,  27,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
      28,  28,  28,  28,  28,  29,  29,  29,  29,  29,
      29,  29,  29,  29,  30,  30,  30,  30,  30,  30,
      30,  30,  31,  31,  31,  31,  31,  31,  31,  32,
      32,  32,  32,  32,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  33,  33,  33,
      33,  33,  33,  33,  33,  33,  33,  33,  34,  34,
      35,  35,  35,  35,  35,  35,  35,  35,  35,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  36,  36,  36,  36,  36,  36,  36,  36,
      36,  36,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  38,  38,  38,  38,  38,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  44,  44,
      44,  44,  44,  44,  45,  45,  45,  45,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  47,  47,  47,  47,  48,
      50,  50,  50,  50,  50,  50,  50,  50,  51,  51,
      51,  52,  52,  52,  52,  52,  52,  52,  52,  52,
      52,  52,  52,  52,  52,  52,  52,  52,  53,  54,
      54,  54,  54,  55,  55,  55,  55,  55,  55,  55,
      55,  55,  55,  55,  55,  55,  55,  55,  55,  55,
      56,  56,  56,  56,  56,  56,  56,  56,  56,  56,
      57,  57,  57,  57,  57,  57,  57,  57,  57,  57,
      57,  57,  57,  58,  58,  58,  58,  58,  58,   0};

    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mua=0.657;  //Debyes
    static const double mub=1.732;  //Debyes

    double q=0.028263028*pow(tt.get("K"),1.5);   // Q(300 K)=179.639   JPL Line Catalog
    unsigned int vp;
    unsigned int ini1, ini2;
    unsigned int ifin1, ifin2;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
  
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
      
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
      
      if(pp.get("mb")<100){
	ini1=ini31[vp]; 
	ifin1=ifin31[vp];
	ini2=ini32[vp]; 
	ifin2=ifin32[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini1=ini21[vp];
      	  ifin1=ifin21[vp];
      	  ini2=ini22[vp];
      	  ifin2=ifin22[vp];
      	}else{
      	  ini1=ini11[vp];
      	  ifin1=ifin11[vp];
      	  ini2=ini12[vp];
      	  ifin2=ifin12[vp];
      	}
      }
      
      if(ini1>0){ini1=ini1-1;}else{ifin1=0;}
      if(ifin1>0){ifin1=ifin1-1;}else{ifin1=0;}
      if(ini2>0){ini2=ini2-1;}else{ifin2=0;}
      if(ifin2>0){ifin2=ifin2-1;}else{ifin2=0;}
      

      complex<double>  lshapeacum1(0.0,0.0);

      if(ifin1==0||ifin1<ini1){
	
	
      }else{
	
	for(unsigned int i=ini1; i<ifin1+1; i++){
	  
	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),Frequency(0.003*pp.get("mb")*pow((300/tt.get("K")),0.7),"GHz"),Frequency(0.0,"GHz"));
	  
	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 
	  
          lshapeacum1=lshapeacum1+lshape;
	  
	}
	
	lshapeacum1=lshapeacum1*(nu.get("GHz")/pi)*(fac2fixed*pow(mua,2.0)/q); // imaginary part: absorption coefficient in cm^2
	                                                                    // real part: delay in rad*cm^2    	
      }

      complex<double>  lshapeacum2(0.0,0.0);

      if(ifin2==0||ifin2<ini2){
	
	
      }else{
	
	for(unsigned int i=ini2; i<ifin2+1; i++){
	  
	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),Frequency(0.003*pp.get("mb"),"GHz"),Frequency(0.0,"GHz"));
	  
	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 
	  
          lshapeacum2=lshapeacum2+lshape;
	  
	}
	
	lshapeacum2=lshapeacum2*(nu.get("GHz")/pi)*(fac2fixed*pow(mub,2.0)/q); // imaginary part: absorption coefficient in cm^2
	                                                                    // real part: delay in rad*cm^2    
      }

      return (lshapeacum1+lshapeacum2)*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )

     
    }

  }



  complex<double>  RefractiveIndexProfile::mkAbs_hh18o(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){


    static const double fre[15]={
      5.624962,203.407381,322.464764,390.607954,
      525.168274,520.138510,531.577390,489.054219,
      554.860949,537.337649,547.676612,692.079249,
      745.320024,970.300490,994.634718};

    static const double gl[15]={
      3.,1.,1.,3.,1.,3.,3.,3.,1.,1.,3.,3.,1.,1.,1.};

    static const double flin[15]={
      .053,.100,.086,.119,.090,.103,.090,
      .132,.103,.119,1.5,.123,2.060,.165,.764};

     static const double el[15]={
     640.5,203.7,467.9,322.1,1512.3,
     1080.7,1512.6,429.7,1082.3,720.3,
     60.5,727.7,136.4,452.5,100.6};

    static const double dv0[15]={
      2.85,2.68,3.03,3.19,1.5,1.94,1.51,
      2.47,1.89,2.07,3.33,2.28,3.13,2.59,3.09};

    static const double dvlm[15]={
      13.68,14.49,15.21,15.84,7.94,10.44,8.13,
      14.24,10.56,11.95,14.66,12.78,13.93,14.06,15.20};

    static const double temp_exp[15]={
      .626,.649,.619,.63,.29,.36,.332,.51,
      .380,.38,.645,.6,.69,.676,.66};

    static const int ifin1[500]={
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   7,   7,   7,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15};

    static const int ini1[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   7,   7,   7,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  15,  15,  15,  15,  15};

    static const int ifin2[500]={
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      7,   7,   7,   9,   9,   9,   9,   9,   9,   9,
      9,   9,   9,   9,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  15,  15,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15};

    static const int ini2[500]={
      1,   1,   1,   1,   1,   1,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   7,   7,   7,   9,
      9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15};

    static const int ifin3[500]={   
      1,   1,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   7,   7,   7,   9,   9,   9,   9,   9,
      9,   9,   9,   9,   9,   9,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  15,  15,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15};

    static const int ini3[500]={
      1,   1,   1,   1,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   4,   4,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   7,   7,   7,   9,   9,   9,
      9,   9,   9,   9,   9,   9,   9,   9,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
      12,  12,  12,  12,  12,  12,  12,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
      13,  13,  13,  13,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      14,  14,  14,  14,  14,  14,  15,  15,  15,  15,
      15,  15,  15,  15,  15,  15,  15,  15,  15,  15};



    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=1.855;  //Debyes

    double q=0.034571542*pow(tt.get("K"),1.5);   // Q(300 K)=179.639   JPL Line Catalog
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
  
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
      
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
      
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }
      
      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}
      
      if(ifin==0||ifin<ini){
	
	return complex<double> (0.0,0.0);
	
      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){
	
	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening_hh18o_hh17o(Frequency(fre[i],"GHz"),tt,pp,eh2o,dv0[i],dvlm[i],temp_exp[i]),Frequency(0.0,"GHz"));
	  
	  lshape=lshape*flin[i]*gl[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 
	  
          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(fac2fixed*pow(mu,2.0)/q); // imaginary part: absorption coefficient in cm^2
	                                                                    // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )
	
      }
     
    }
 
  }


  complex<double>  RefractiveIndexProfile::mkAbs_o2_vib(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){
    
    static const double fre[6]={363.717932,420.232268,482.757899,706.772490,765.423334,825.814211};
    
    static const double flin[6]={.04800, .39245, .12804,  .04000,  .21283, .06343};
    
    static const double el[6]={5.70,2.95,2.95,25.97,23.18,23.18};
    
    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.0186;  //Debyes (M1 Transitions)
    
    double q=0.72923*tt.get("K");
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    double dv0=2.0;
    
    if(nu.get("GHz")>999.9){return complex<double> (0.0,0.0);}
    
    for(unsigned int i=0; i<6; i++){
      
      lshape=lineshape(nu,Frequency(fre[i],"GHz"), linebroadening_o2(Frequency(fre[i],"GHz"),tt,pp,eh2o,32.0,dv0,0.2),Frequency(0.0,"GHz"));
      
      lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 
      
      lshapeacum=lshapeacum+lshape;
      
    }
    
    lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(fac2fixed*pow(mu,2.0)/q); // imaginary part: absorption coefficient in cm^2
                                                                        // real part: delay in rad*cm^2    
    
    return lshapeacum*1e-4;    // to give it in SI units (m^2)   
    
  }  


  complex<double>  RefractiveIndexProfile::mkAbs_16o17o(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){

    static const double fre[14]={
      239.89920,304.81682,412.53217,474.78769,356.03171,582.32937,
      643.33256,524.57590,751.19854,811.40203,692.64446,919.63186,
      979.23889,860.48018};
    
    static const double flin[14]={
      .738E+00, .261E+00, .416E+00, .136E+00, .510E-01, .292E+00,
      .900E-01, .480E-01, .226E+00, .670E-01, .420E-01, .184E+00,
      .530E-01, .370E-01};
    
    static const double el[14]={
      .000,     .000,    3.900,    3.900,    6.600,   11.900,
      11.900,   14.600,   23.900,   23.900,   26.700,   39.900,
      39.900,   42.800};
    
    
    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.0186;  //Debyes (M1 Transitions)
    
    double q=1.536568889*tt.get("K");   
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    double dv0=2.0;
    
    if(nu.get("GHz")>999.9){return complex<double> (0.0,0.0);}
    
    for(unsigned int i=0; i<14; i++){
      
      lshape=lineshape(nu,Frequency(fre[i],"GHz"), linebroadening_o2(Frequency(fre[i],"GHz"),tt,pp,eh2o,33.0,dv0,0.2),Frequency(0.0,"GHz"));
      
      lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 
      
      lshapeacum=lshapeacum+lshape;
      
    }
    
    lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(fac2fixed*pow(mu,2.0)/q); // imaginary part: absorption coefficient in cm^2
    // real part: delay in rad*cm^2    
    
    return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )
    
  }



complex<double>  RefractiveIndexProfile::mkAbs_16o18o(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){

    static const double fre[15]={
      233.94611,298.47306,401.73972,463.77817,345.01768,566.89542,
      627.75661,508.99550,731.18612,791.29170,672.52976,895.07000,
      954.60941,835.84642,999.02720};

    static const double flin[15]={
      .776E+00, .276E+00, .439E+00, .143E+00, .540E-01, .308E+00,
      .950E-01, .510E-01, .238E+00, .710E-01, .440E-01, .194E+00,
      .560E-01, .390E-01, .340E-01};

    static const double el[15]={
      .000,     .000,    3.800,    3.800,    6.500,   11.600,
      11.600,   14.300,   23.300,   23.300,   26.100,   38.900,
      38.900,   41.700,   61.300};

    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.0186;  //Debyes (M1 Transitions)
    
    double q=1.536568889*tt.get("K");   
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    double dv0=2.0;

    if(nu.get("GHz")>999.9){return complex<double> (0.0,0.0);}

    for(unsigned int i=0; i<15; i++){
	  
      lshape=lineshape(nu,Frequency(fre[i],"GHz"), linebroadening_o2(Frequency(fre[i],"GHz"),tt,pp,eh2o,34.0,dv0,0.2),Frequency(0.0,"GHz"));

      lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 
      
      lshapeacum=lshapeacum+lshape;
	  
    }
	
    lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(fac2fixed*pow(mu,2.0)/q); // imaginary part: absorption coefficient in cm^2
	                                                                    // real part: delay in rad*cm^2    

    return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )
	
  }
     
 
  complex<double>  RefractiveIndexProfile::mkAbs_o2(Temperature tt, Pressure pp, Pressure eh2o, Frequency nu){

    static const double fre[55]={
      50.473557,50.987199,51.502912,52.021065,52.542126,
      53.066702,53.595595,54.129887,54.671077,55.221309,
      55.783762,56.264778,56.363362,56.968188,57.612473,
      58.323870,58.446594,59.164203,59.590988,60.306059,
      60.434783,61.150565,61.800158,62.411215,62.486259,
      62.997970,63.568500,64.127733,64.678846,65.223985,
      65.764647,66.301917,66.836594,67.369286,67.900462,
      68.430489,68.959664,118.750345,368.498440,424.763123,
      487.249381,715.393320,773.839669,834.145728,1061.124062,
      1120.715051,1179.879254,1406.372391,1466.807173,1525.131044,
      1751.254916,1812.405481,1870.017954,2095.777726,2157.577883};
    
    static const double flin[55]={
      74.96623,70.96429,66.96212,62.95966,58.95686,
      54.95363,50.94989,46.94549,42.94023,38.93385,
      34.92593, 2.45198,30.91587,26.90263,22.88444,
      18.85787, 6.71035,14.81540,10.80290,10.73657,
      14.85057,18.87966,22.89926,26.91337,6.53862,
      30.92400,34.93231,38.93898,42.94445,46.94902,
      50.95289,54.95621,58.95910,62.96162,66.96385,
      70.96584,74.96761, 2.00000, .04800, .39245,
      .12804,  .04000,  .21283, .06343, .03044,
      .14656,  .04174,  .02443, .11187, .03102,
      .02034,  .09050,  .02465, .01740, .07601};

    static const double el[55]={
      2893.8,2594.5,2311.3,2044.2,1793.4,
      1558.9,1340.6,1138.7, 953.1, 784.0,
      631.2,   3.0, 494.9, 375.1, 271.7,
      184.9,  23.6, 114.5,  60.8,  60.7,
      114.5, 184.7, 271.5, 374.8,  23.4,
      494.6, 630.8, 783.5, 952.7,1138.2,
      1340.0,1558.2,1792.7,2043.5,2310.5,
      2593.6,2892.9,    .0,   5.7,   3.0,
      3.0,  26.5,  23.6,  23.6,  63.6,
      60.8,  60.8, 117.4, 114.5, 114.5,
      187.7, 184.7, 184.7, 274.5, 271.5};

    static const double ensanche[55][4]={
      {.890 ,  .8 ,  .240 ,  .790},
      {.910 ,  .8 ,  .220 ,  .780},
      {.940 ,  .8 ,  .197 ,  .774},
      {.970 ,  .8 ,  .166 ,  .764},
      {.990 ,  .8 ,  .136 ,  .751},
      {1.020 ,  .8 ,  .131 ,  .714},
      {1.050 ,  .8 ,  .230 ,  .584},
      {1.070 ,  .8 ,  .335 ,  .431},
      {1.100 ,  .8 ,  .374 ,  .305},
      {1.130 ,  .8 ,  .258 ,  .339},
      {1.170 ,  .8 ,  -0.166 ,  .705},
      {1.730 ,  .8 ,  .390 ,  -0.113},
      {1.200 ,  .8 ,  -0.297 ,  .753},
      {1.240 ,  .8 ,  -0.416 ,  0.742},
      {1.280 ,  .8 ,  -0.613 ,  0.697},
      {1.330 ,  .8 ,  -0.205 ,  0.051},
      {1.520 ,  .8 ,  0.748 ,  -0.146},
      {1.390 ,  .8 ,  -0.722 ,  0.266},
      {1.430 ,  .8 ,  0.765 ,  -0.090},
      {1.450 ,  .8 ,  -0.705 ,  0.081},
      {1.360 ,  .8 ,  0.697 ,  -0.324},
      {1.310 ,  .8 ,  0.104 ,  -0.067},
      {1.270 ,  .8 ,  0.570 ,  -0.761},
      {1.230 ,  .8 ,  0.360 ,  -0.777},
      {1.540 ,  .8 ,  -0.498 ,  0.097},
      {1.200 ,  .8 ,  0.239 ,  -0.768},
      {1.170 ,  .8 ,  0.108 ,  -0.706},
      {1.130 ,  .8 ,  -0.311 ,  -0.332},
      {1.100 ,  .8 ,  -0.421 ,  -0.298},
      {1.070 ,  .8 ,  -0.375 ,  -0.423},
      {1.050 ,  .8 ,  -0.267 ,  -0.575},
      {1.020 ,  .8 ,  -0.168 ,  -0.700},
      {0.990 ,  .8 ,  -0.169 ,  -0.735},
      {0.970 ,  .8 ,  -0.200 ,  -0.744},
      {0.940 ,  .8 ,  -0.228 ,  -0.753},
      {.920 ,  .8 ,  -0.240 ,  -0.760},
      {.900 ,  .8 ,  -0.250 ,  -0.765},
      {1.630 ,  .8 ,  -0.036 , 0.009},
      {1.920 ,  .2 ,  .0 ,  .0},
      {1.930 ,  .2 ,  .0 ,  .0},
      {1.920 ,  .2 ,  .0 ,  .0},
      {1.810 ,  .2 ,  .0 ,  .0},
      {1.820 ,  .2 ,  .0 ,  .0},
      {1.810 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0},
      {1.900 ,  .2 ,  .0 ,  .0}};


    static const int ifin1[800]={
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  51,  51,  51,  51,
      51,  51,  51,  51,  51,  51,  51,  51,  51,  51,
      51,  51,  51,  51,  51,  51,  51,  51,  51,  51,
      51,  51,  51,  51,  51,  51,  52,  52,  52,  52,
      52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
      52,  52,  52,  52,  52,  52,  52,  52,  52,  52,
      52,  52,  52,  52,  52,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  53,  53,
      53,  53,  53,  53,  53,  53,  53,  53,  54,  54};

    static const int ini1[800]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46};

    static const int ifin2[800]={
      37,  37,  37,  37,  37,  37,  37,  37,  37,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50};
    
    static const int ini2[800]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,  34,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50};

    static const int ifin3[800]={
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   2,   5,   9,  14,  17,
      21,  26,  29,  33,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
      37,  37,  37,  37,   0,   0,   0,   0,   0,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,   0,   0,   0,   0,   0,   0,
      0,   0,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,   0,   0,
      0,   0,   0,   0,   0,   0,   0,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    static const int ini3[800]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,  34,   1,   1,   1,   1,   1,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,  38,
      38,  38,  38,  38,  38,  38,  38,  38,  38,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,  39,  39,  39,  39,  39,  39,
      39,  39,  39,  39,   1,   1,   1,   1,   1,   1,
      1,   1,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
      40,  40,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,  41,  41,  41,  41,  41,  41,
      41,  41,  41,  41,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,  42,  42,
      42,  42,  42,  42,  42,  42,  42,  42,   1,   1,
      1,   1,   1,   1,   1,   1,   1,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,  43,  43,  43,
      43,  43,  43,  43,  43,  43,  43,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,  44,  44,  44,
      44,  44,  44,  44,  44,  44,  44,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,  45,  45,  45,  45,  45,  45,  45,  45,  45,
      45,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      46,  46,  46,  46,  46,  46,  46,  46,  46,  46,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      47,  47,  47,  47,  47,  47,  47,  47,  47,  47,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,  48,  48,  48,  48,  48,  48,  48,
      48,  48,  48,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,  49,  49,  49,  49,  49,  49,  49,
      49,  49,  49,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
      50,  50,  50,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1};

    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.0186;  //Debyes (M1 Transitions)
    
    double q=0.72923*tt.get("K");   
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
  
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
      
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
      
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }
      
      if(ini<38&&fabs(fre[ini-1]-nu.get("GHz"))>50){ini=38;}

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      
      if(ifin==0||ifin<ini||(ini>0&&ifin==36)){
	
	return complex<double> (0.0,0.0);
       
      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){
	  
	  lshape=lineshape(nu,Frequency(fre[i],"GHz"), 
			   linebroadening_o2(Frequency(fre[i],"GHz"),tt,pp,eh2o,32.0,ensanche[i][0],ensanche[i][1]),
			   interf_o2(tt,pp,ensanche[i][2],ensanche[i][3]));

	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*(1-exp(-0.047992745509*fre[i]/tt.get("K"))); 
	  
          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(fac2fixed*pow(mu,2.0)/q); // imaginary part: absorption coefficient in cm^2
	                                                                    // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )
	
      }
     
    }
 
  }



  complex<double>  RefractiveIndexProfile::mkAbs_o3(Temperature tt, Pressure pp, Frequency nu){

    static const double fre[1151] ={ .78976,  1.08767,  2.59593,  9.20030, 10.22572, 10.61320,
				      11.07245, 14.82086, 14.86659, 16.16240, 23.85967, 25.22979,
				      25.51069, 25.65096, 27.45849, 27.86192, 28.96050, 30.05176,
				      30.18124, 30.52406, 32.10058, 36.02192, 37.83244, 39.43795,
				      42.83246, 43.65329, 43.85406, 44.87117, 50.03501, 51.05293,
				      51.97576, 51.98456, 53.68814, 55.35451, 56.97138, 58.09390,
				      58.41096, 61.34749, 61.92676, 63.07862, 65.23609, 66.05852,
				      67.24960, 67.35624, 67.83418, 68.42197, 73.91994, 75.84892,
				      76.39355, 76.53375, 77.60251, 78.99297, 80.84080, 81.28997,
				      93.84435, 93.95511, 95.79638, 96.22841, 99.24696,100.63735,
				      100.69174,101.73676,101.83535,103.87833,109.55926,110.76027,
				      110.83592,111.04747,114.97919,118.36451,119.27765,123.34913,
				      124.08738,125.38943,125.41324,127.71597,128.09479,128.31390,
				      130.95467,132.38531,133.04182,136.33851,136.86024,139.84279,
				      140.76732,142.17510,143.88753,143.96251,144.91038,144.91937,
				      148.74487,153.72392,153.95322,154.04643,156.10713,164.77106,
				      164.95185,165.78451,166.58440,167.57277,170.29955,171.41234,
				      173.48552,175.18642,175.44569,178.57654,180.00098,183.96584,
				      184.37781,184.74874,185.55711,187.13209,187.63116,187.88484,
				      190.57362,193.35113,195.43042,195.72116,197.53299,199.38432,
				      203.45298,206.13208,207.48507,208.64242,210.42326,210.76234,
				      210.80322,214.95567,218.11966,223.89749,224.85276,226.05407,
				      229.57500,231.28151,232.98522,235.70982,237.14613,238.43215,
				      239.09326,240.90420,242.31869,243.45378,244.14666,244.15842,
				      247.76177,248.18340,249.78857,249.96196,250.72901,252.32213,
				      256.88481,258.20217,258.71612,262.85791,263.69263,263.88630,
				      264.92592,267.26655,271.09271,273.05091,274.47843,276.92353,
				      277.04073,279.47150,279.48579,279.89250,280.99444,281.95751,
				      282.83714,286.08769,286.15656,286.29474,288.95915,290.97525,
				      293.17135,293.54831,297.17368,298.60079,298.79388,300.68514,
				      301.81299,303.16494,303.57254,310.06346,315.87514,316.32719,
				      316.68179,317.19502,319.99655,320.36011,323.70451,326.90097,
				      327.84458,329.78920,332.70454,332.88174,335.27126,335.64323,
				      343.18080,343.23744,343.50567,349.74474,352.32405,352.59292,
				      352.81524,355.01819,356.08933,357.62983,358.19981,358.85334,
				      359.64942,359.64962,361.66938,361.82624,363.46929,363.71411,
				      364.40371,364.44990,366.77690,368.33221,368.94938,371.02286,
				      375.17918,375.97875,376.23838,378.28118,378.69490,382.19229,
				      387.09846,387.28764,387.98989,390.29654,392.65396,395.06319,
				      396.52948,397.02778,400.07700,401.21773,401.93101,402.39532,
				      403.83738,408.29394,408.30687,413.01193,414.05422,414.25452,
				      420.79513,422.69422,423.66944,424.84594,425.15777,426.06961,
				      427.94447,428.79221,429.42011,430.33383,434.29166,437.49354,
				      438.48078,440.08212,441.34037,442.11759,445.45871,446.14385,
				      446.64933,448.87627,451.42006,452.23896,453.79011,454.66871,
				      457.41562,460.25306,462.36541,463.18961,463.24632,464.10876,
				      466.03399,466.64126,467.96711,468.39935,469.10425,469.32511,
				      469.48238,469.95390,470.99090,472.30100,472.89886,473.07366,
				      474.16599,476.02061,479.39607,479.63320,480.01423,480.18234,
				      480.71724,481.19896,481.62011,483.54546,486.12325,486.45055,
				      487.34851,489.53718,491.94286,491.95142,496.25398,497.09785,
				      497.97297,498.74741,500.43357,501.77262,505.25728,505.36989,
				      507.73610,507.81539,508.03152,508.19293,509.27262,511.95526,
				      514.99270,517.43040,517.79039,520.13476,522.04607,522.96962,
				      523.30879,523.53191,529.68389,530.50108,531.07699,534.27304,
				      535.92257,537.71873,539.37266,540.35606,542.58044,543.59397,
				      543.61287,544.85750,548.76415,549.07066,549.82036,550.98944,
				      551.43571,556.71114,557.97024,559.47804,559.83438,563.43980,
				      565.70359,566.29352,566.73496,569.34609,570.09715,570.99933,
				      572.87715,573.60274,574.51809,574.54071,576.51486,582.14407,
				      582.30716,583.47499,589.10641,590.33759,591.00445,591.24880,
				      592.63069,595.05617,595.08096,599.97903,600.23048,607.84932,
				      609.73051,610.26257,610.36515,611.27659,611.98653,614.73166,
				      614.78406,616.27977,616.28251,617.30449,620.68671,620.72197,
				      620.82566,623.68767,625.37152,625.90529,632.17691,633.35354,
				      634.46153,634.52013,638.53691,638.90302,639.54779,640.09254,
				      640.31195,641.53424,642.34964,644.80494,645.66998,646.34897,
				      647.84000,650.73303,651.47581,651.54302,651.55697,653.76244,
				      654.71344,654.85300,655.00521,655.12032,655.20355,655.29016,
				      655.60630,655.87260,655.96211,656.00568,656.22377,656.25258,
				      656.38405,656.41983,656.46161,656.47779,657.51496,658.03858,
				      658.81583,661.45825,664.48824,665.67764,665.85431,666.63088,
				      666.76787,669.85708,671.66925,671.80444,671.93737,672.45339,
				      672.79461,673.91438,676.06188,676.90348,677.14338,677.14643,
				      683.63910,684.24403,691.35879,691.66547,691.98609,692.36851,
				      693.47373,694.76090,696.09057,697.49073,698.98040,700.85298,
				      701.36487,701.50803,702.71667,707.09907,708.14345,709.63870,
				      711.49549,716.82967,718.29616,718.35952,719.79226,723.01028,
				      723.37729,726.64407,727.08660,728.25588,730.01142,731.02271,
				      732.62802,734.38955,735.00438,736.48234,740.36731,742.26844,
				      743.16504,745.87234,746.50099,746.87407,748.49865,748.71056,
				      751.17579,751.30176,751.72060,751.90349,753.76544,754.45631,
				      757.24983,758.13032,760.34313,765.10202,766.30130,767.67812,
				      773.95746,776.26805,776.62787,777.14588,779.24717,779.71446,
				      782.43831,783.60719,783.66087,784.69136,785.65813,785.68858,
				      787.72308,792.26350,793.06067,793.21304,794.66359,798.76156,
				      799.38884,802.36591,802.37422,804.66276,804.70251,807.62416,
				      809.05998,810.64622,810.95175,811.46341,813.14024,813.99104,
				      818.41849,819.26277,819.74577,824.79452,825.71518,825.84467,
				      826.56013,827.28922,827.59102,828.00631,830.13306,830.55691,
				      830.94177,831.00898,831.07889,831.57387,832.11797,832.78965,
				      832.92176,833.60896,834.48990,835.21016,835.25747,835.68548,
				      836.22654,836.44614,836.89268,837.81431,838.39607,838.60022,
				      839.65543,839.69616,839.76456,840.76066,840.93713,841.58733,
				      841.81086,842.00871,842.19483,842.38878,842.61448,842.75442,
				      842.88346,842.97080,843.03967,843.08617,843.11810,843.13769,
				      843.14864,843.75362,849.05109,849.94415,850.17635,851.35740,
				      852.70682,852.79868,855.54027,857.96792,859.89817,860.54837,
				      861.45133,861.48476,862.91528,866.12613,869.06812,869.49660,
				      870.37565,874.06409,875.53602,880.92558,883.05624,885.28598,
				      885.35070,886.45181,886.72861,887.28515,888.15902,888.66322,
				      894.36392,896.06374,900.87519,901.28523,902.35869,906.29038,
				      906.48993,908.25899,908.74287,910.01156,910.65449,911.96010,
				      914.01207,914.66584,918.84528,919.64288,920.57170,925.23034,
				      926.19546,926.39868,927.53952,931.63601,932.38690,933.13981,
				      933.21985,936.00479,937.18113,939.34455,939.87274,944.90674,
				      948.85039,950.72195,951.46451,951.49837,955.90760,956.96379,
				      958.48951,958.54005,959.90335,961.33800,962.39343,964.66154,
				      969.10916,969.45417,970.01419,970.15719,970.77567,971.29614,
				      973.00835,973.78714,976.78538,979.60929,980.02984,982.27496,
				      982.88204,986.02067,986.65516,987.59861,989.96393,992.66980,
				      994.30024,995.39579,995.71631,997.54150,998.32354,998.61320,
				      1000.142334, 1000.616089, 1002.057922, 1002.973938,
				      1003.242737, 1003.342834, 1006.224670, 1007.570740, 1008.683655,
				      1009.318787, 1009.338623, 1010.374207, 1011.957336, 1012.409058,
				      1012.673767, 1012.798096, 1013.995117, 1014.107178, 1015.252563,
				      1015.307312, 1017.653748, 1017.947327, 1019.487915, 1019.981934,
				      1020.279663, 1020.515137, 1020.624084, 1022.284546, 1022.748901,
				      1022.907410, 1023.964233, 1024.485107, 1025.336792, 1025.823853,
				      1026.430298, 1026.843262, 1026.854492, 1026.873291, 1026.942383,
				      1027.278198, 1027.317261, 1027.605713, 1027.916016, 1028.161987,
				      1028.379395, 1028.554443, 1028.701660, 1028.819092, 1028.913452,
				      1028.986572, 1029.042480, 1029.083374, 1029.112305, 1029.131348,
				      1029.142944, 1029.148926, 1032.042480, 1032.852295, 1036.147095,
				      1036.412598, 1037.245483, 1040.528198, 1042.734497, 1042.999756,
				      1044.670044, 1045.843262, 1048.484863, 1049.960571, 1052.564819,
				      1056.882690, 1058.120850, 1058.917603, 1059.646240, 1062.520508,
				      1065.791870, 1067.967041, 1069.038208, 1069.843384, 1071.055054,
				      1073.674805, 1074.329956, 1076.277222, 1077.712769, 1077.801880,
				      1080.274780, 1083.291016, 1083.377319, 1085.138794, 1087.783325,
				      1091.044067, 1093.189697, 1095.005981, 1096.260254, 1096.562744,
				      1096.879761, 1098.021973, 1098.394531, 1101.819458, 1103.029541,
				      1108.622925, 1109.341919, 1113.035156, 1116.285889, 1117.412720,
				      1118.403198, 1120.150635, 1120.477051, 1121.218872, 1121.460205,
				      1127.107788, 1128.248779, 1132.504395, 1133.858643, 1138.276489,
				      1141.517700, 1143.608154, 1143.739014, 1145.281860, 1146.656006,
				      1147.276978, 1147.689575, 1153.460815, 1154.850220, 1157.105469,
				      1159.085205, 1160.202881, 1163.185181, 1163.508423, 1165.281494,
				      1166.740601, 1167.251587, 1168.805054, 1170.376221, 1170.447266,
				      1171.848633, 1171.941650, 1177.531494, 1178.542114, 1178.666626,
				      1180.300537, 1182.944458, 1184.303955, 1185.175049, 1187.243652,
				      1188.731567, 1190.542603, 1190.629272, 1191.254883, 1191.954956,
				      1192.794556, 1193.096924, 1193.994507, 1194.919922, 1195.130981,
				      1195.466064, 1198.208130, 1198.857666, 1201.110718, 1201.947632,
				      1203.635376, 1203.867065, 1204.508789, 1205.493286, 1205.800781,
				      1206.626465, 1207.632690, 1208.369019, 1209.161499, 1209.515503,
				      1209.791992, 1210.418823, 1210.585449, 1210.942505, 1211.437012,
				      1211.860474, 1211.864258, 1212.246704, 1212.580811, 1212.775269,
				      1212.878052, 1213.024658, 1213.134521, 1213.358032, 1213.549194,
				      1213.712524, 1213.849731, 1213.947021, 1213.964355, 1214.058472,
				      1214.073608, 1214.134644, 1214.195068, 1214.241943, 1214.277222,
				      1214.302612, 1214.320068, 1214.330933, 1214.336670, 1214.338501,
				      1217.161743, 1219.177246, 1220.016724, 1220.474854, 1229.063232,
				      1230.683350, 1234.720825, 1239.155273, 1242.361328, 1244.353638,
				      1245.153442, 1245.524902, 1246.510254, 1249.386963, 1252.765747,
				      1254.255737, 1255.869385, 1258.291748, 1259.920654, 1264.357178,
				      1264.826782, 1267.554810, 1269.524292, 1270.285889, 1270.386230,
				      1281.049805, 1282.001831, 1285.115601, 1287.351074, 1289.553345,
				      1292.742065, 1294.689697, 1295.414185, 1295.426025, 1304.632935,
				      1306.222290, 1310.306763, 1314.744507, 1317.355469, 1317.924561,
				      1319.744873, 1319.850464, 1320.028320, 1320.538574, 1329.906616,
				      1331.385010, 1342.587891, 1344.641724, 1345.006958, 1345.144897,
				      1345.659912, 1356.535278, 1360.679443, 1363.169678, 1368.275757,
				      1369.288086, 1375.074219, 1377.438110, 1377.654297, 1378.891479,
				      1379.498413, 1380.942261, 1381.670776, 1382.180786, 1383.842651,
				      1385.012451, 1386.380005, 1387.444946, 1388.581787, 1389.526367,
				      1390.294189, 1390.476318, 1390.682739, 1391.297974, 1392.092651,
				      1392.180542, 1392.795776, 1393.445801, 1393.458374, 1394.051514,
				      1394.600464, 1394.673340, 1395.093750, 1395.217529, 1395.309326,
				      1395.544434, 1395.894409, 1395.948730, 1396.313843, 1396.640015,
				      1396.931396, 1397.189575, 1397.417725, 1397.617798, 1397.792114,
				      1397.942871, 1398.072266, 1398.182129, 1398.274536, 1398.350952,
				      1398.413330, 1398.463379, 1398.502441, 1398.532104, 1398.553711,
				      1398.568359, 1398.577393, 1398.581909, 1401.963867, 1406.787598,
				      1408.207886, 1409.693237, 1415.471680, 1415.876953, 1416.285889,
				      1416.883179, 1418.008301, 1418.612915, 1420.284790, 1422.716309,
				      1429.991211, 1431.884521, 1438.892334, 1439.103394, 1440.646729,
				      1441.068848, 1444.031250, 1445.600464, 1449.490112, 1456.954712,
				      1461.404419, 1465.970703, 1466.257446, 1470.743042, 1471.231445,
				      1482.000610, 1486.584229, 1490.992920, 1491.441650, 1493.280151,
				      1493.410278, 1495.484009, 1496.341431, 1505.242676, 1507.007324,
				      1512.878174, 1516.164795, 1516.619751, 1519.260254, 1520.550537,
				      1521.023560, 1531.989868, 1541.336304, 1541.790649, 1542.540039,
				      1544.418945, 1556.910889, 1558.333740, 1558.594849, 1564.069458,
				      1566.952393, 1566.974731, 1568.224243, 1568.455444, 1569.334839,
				      1569.577515, 1570.418213, 1570.685303, 1571.300903, 1571.399292,
				      1572.337524, 1573.195801, 1574.005737, 1574.750366, 1575.446167,
				      1576.086426, 1576.680176, 1577.225586, 1577.726807, 1577.728271,
				      1578.188477, 1578.609863, 1578.993896, 1579.343018, 1579.659424,
				      1579.944824, 1580.201538, 1580.431274, 1580.635864, 1580.817261,
				      1580.977051, 1581.116577, 1581.238037, 1581.342529, 1581.431641,
				      1581.506592, 1581.569092, 1581.620117, 1581.661133, 1581.692993,
				      1581.717041, 1581.734253, 1581.745605, 1581.753540, 1581.754395,
				      1581.820435, 1585.437500, 1591.679932, 1592.007202, 1592.103271,
				      1600.119385, 1601.828735, 1606.624268, 1608.413574, 1616.852661,
				      1617.241333, 1621.577637, 1621.886963, 1625.314941, 1631.458984,
				      1641.974365, 1642.026367, 1642.364746, 1642.742920, 1646.291382,
				      1656.097412, 1657.602173, 1667.470947, 1668.445923, 1670.213623,
				      1670.969238, 1672.109009, 1675.698975, 1680.371338, 1680.872559,
				      1682.616333, 1684.010864, 1688.135376, 1692.557861, 1692.849121,
				      1693.472778, 1695.374878, 1696.044556, 1697.221924, 1700.885498,
				      1705.268555, 1706.074463, 1713.986694, 1717.623047, 1718.064819,
				      1718.502686, 1720.537354, 1722.336304, 1726.067017, 1730.031372,
				      1742.663208, 1742.934082, 1745.172485, 1745.701538, 1746.027344,
				      1746.201660, 1747.452515, 1751.242554, 1753.478149, 1753.956055,
				      1754.058228, 1754.780518, 1755.550659, 1756.272827, 1756.946533,
				      1757.575439, 1758.160645, 1758.704712, 1759.209351, 1759.676636,
				      1760.108032, 1760.505737, 1760.871094, 1761.206299, 1761.512329};

    static const double flin[1151]={.548E+01, .559E+01, .804E+01, .302E+01, .211E+01, .150E+01,
				    .159E+01, .685E+01, .412E+01, .443E+01, .290E+01, .786E+01,
				    .664E+01, .118E+01, .126E+01, .700E+01, .415E+01, .279E+01,
				    .132E+01, .101E+01, .824E+01, .302E+01, .329E+01, .178E+01,
				    .512E+00, .139E+01, .647E+01, .837E+00, .524E+01, .590E+01,
				    .453E+01, .840E+01, .147E+01, .291E+01, .767E+01, .463E+01,
				    .209E+01, .508E+01, .268E+01, .586E+01, .136E+01, .269E+01,
				    .241E+01, .278E+01, .207E+01, .684E+00, .106E+01, .722E+01,
				    .382E+01, .283E+01, .370E+01, .627E+01, .733E+01, .748E+01,
				    .120E+01, .231E+01, .474E+01, .248E+01, .616E+01, .558E+00,
				    .608E+01, .434E+01, .486E+01, .237E+01, .384E+01, .242E+01,
				    .597E+01, .728E+01, .896E+00, .100E+01, .468E+01, .496E+01,
				    .731E+01, .412E+01, .344E+01, .905E+00, .474E+01, .926E+00,
				    .623E+01, .587E+01, .763E+01, .708E+01, .327E+01, .757E+01,
				    .458E+00, .827E+01, .282E+01, .828E+01, .636E+01, .367E+01,
				    .200E+01, .463E+01, .446E+01, .185E+01, .567E+01, .687E+01,
				    .201E+01, .885E+01, .324E+01, .561E+00, .808E+01, .440E+01,
				    .391E+00, .426E+01, .303E+01, .367E+01, .407E+01, .789E+01,
				    .564E+01, .445E+01, .546E+01, .648E+01, .781E+00, .380E+00,
				    .667E+01, .284E+01, .906E+01, .159E+01, .787E+01, .542E+01,
				    .659E+01, .404E+01, .790E+01, .307E+01, .526E+01, .164E+00,
				    .139E+01, .464E+01, .646E+01, .767E+01, .651E+01, .261E+01,
				    .383E+01, .902E+01, .809E+01, .130E+02, .109E+02, .504E+01,
				    .149E+02, .322E+00, .877E+01, .735E+01, .625E+01, .115E+01,
				    .239E+01, .165E+02, .424E+01, .679E+01, .746E+01, .684E+00,
				    .643E+01, .361E+01, .503E+01, .516E+01, .176E+02, .483E+01,
				    .937E+00, .350E+01, .604E+01, .882E+01, .216E+01, .216E+01,
				    .725E+01, .823E+01, .854E+00, .592E+01, .697E+01, .623E+01,
				    .339E+01, .577E+01, .183E+02, .145E+01, .555E+01, .461E+01,
				    .251E+01, .714E+00, .583E+01, .277E+00, .592E+01, .194E+01,
				    .922E+01, .344E+01, .704E+01, .317E+01, .185E+02, .428E+01,
				    .439E+01, .502E+00, .857E+01, .610E+00, .561E+01, .702E+01,
				    .172E+01, .682E+01, .501E+01, .149E+01, .295E+01, .803E+01,
				    .417E+01, .598E+01, .302E+00, .539E+01, .564E+01, .149E+01,
				    .183E+02, .167E+01, .661E+01, .708E+01, .240E+02, .112E+02,
				    .244E+00, .218E+02, .272E+01, .782E+01, .258E+02, .738E+01,
				    .650E+01, .868E+01, .194E+02, .126E+00, .395E+01, .833E+01,
				    .617E+01, .517E+01, .271E+02, .169E+02, .127E+01, .639E+01,
				    .250E+01, .179E+01, .760E+01, .553E+00, .146E+02, .373E+01,
				    .179E+02, .278E+02, .211E+01, .660E+01, .495E+01, .105E+02,
				    .105E+01, .126E+02, .617E+01, .228E+01, .739E+01, .133E+02,
				    .351E+01, .219E+00, .107E+02, .815E+01, .695E+01, .280E+02,
				    .473E+01, .860E+01, .836E+00, .694E+01, .595E+01, .914E+01,
				    .205E+01, .717E+01, .124E+02, .262E+01, .208E+01, .174E+02,
				    .328E+01, .771E+01, .785E+01, .718E+01, .451E+01, .628E+00,
				    .642E+01, .573E+01, .720E+01, .520E+01, .278E+02, .183E+01,
				    .695E+01, .402E+01, .153E+02, .284E+01, .160E+01, .884E+00,
				    .223E+01, .342E+01, .457E+01, .306E+01, .570E+01, .329E+02,
				    .350E+02, .682E+01, .305E+02, .429E+01, .431E+00, .802E+01,
				    .792E+01, .319E+01, .144E+02, .365E+02, .551E+01, .200E+00,
				    .899E+01, .161E+01, .673E+01, .278E+02, .100E+02, .740E+01,
				    .283E+01, .103E+02, .170E+02, .375E+02, .252E+00, .406E+01,
				    .110E+02, .809E+01, .273E+02, .227E+01, .251E+02, .529E+01,
				    .139E+01, .382E+01, .651E+01, .174E+02, .119E+02, .773E+01,
				    .165E+02, .261E+01, .226E+02, .101E+00, .384E+01, .755E+01,
				    .795E+01, .507E+01, .126E+02, .117E+01, .799E+01, .629E+01,
				    .838E+01, .249E+01, .751E+01, .238E+01, .185E+00, .454E+01,
				    .203E+02, .361E+01, .166E+02, .266E+02, .133E+02, .484E+01,
				    .958E+00, .185E+02, .122E+02, .607E+01, .261E+01, .195E+02,
				    .183E+02, .766E+01, .729E+01, .216E+01, .233E+01, .535E+01,
				    .339E+01, .139E+02, .462E+01, .942E+01, .754E+00, .791E+01,
				    .164E+02, .585E+01, .277E+01, .194E+01, .707E+01, .316E+01,
				    .148E+02, .144E+02, .206E+02, .630E+01, .173E+00, .440E+01,
				    .774E+01, .260E+02, .559E+00, .163E+02, .295E+01, .562E+01,
				    .216E+02, .134E+02, .172E+01, .685E+01, .142E+02, .294E+01,
				    .120E+02, .895E+01, .333E+02, .739E+01, .903E+01, .148E+02,
				    .417E+01, .378E+00, .107E+02, .314E+01, .790E+01, .540E+01,
				    .949E+01, .150E+01, .829E+01, .663E+01, .225E+01, .712E+01,
				    .100E+02, .886E+01, .112E+02, .595E+01, .769E+01, .226E+02,
				    .653E+01, .479E+01, .537E+01, .124E+02, .420E+01, .361E+01,
				    .299E+01, .235E+01, .167E+01, .908E+00, .779E+01, .137E+02,
				    .272E+01, .148E+02, .306E+02, .866E+01, .395E+01, .160E+02,
				    .216E+00, .236E+02, .333E+01, .110E+02, .518E+01, .163E+00,
				    .151E+02, .171E+02, .129E+01, .254E+02, .161E+02, .640E+01,
				    .182E+02, .249E+01, .372E+01, .281E+02, .840E-01, .101E+02,
				    .354E+01, .163E+02, .192E+02, .495E+01, .790E+01, .246E+02,
				    .108E+01, .783E+01, .618E+01, .741E+01, .153E+02, .227E+01,
				    .201E+02, .350E+01, .257E+02, .259E+02, .118E+02, .473E+01,
				    .370E+01, .879E+00, .101E+02, .596E+01, .209E+02, .154E+00,
				    .718E+01, .206E+01, .205E+01, .160E+02, .393E+01, .327E+01,
				    .239E+02, .155E+02, .786E+01, .267E+02, .450E+01, .136E+02,
				    .790E+01, .127E+02, .215E+02, .686E+00, .573E+01, .185E+02,
				    .350E+01, .696E+01, .184E+01, .222E+02, .277E+02, .305E+01,
				    .428E+01, .404E+01, .221E+02, .503E+00, .551E+01, .155E+02,
				    .359E+01, .674E+01, .206E+02, .431E+01, .162E+01, .156E+02,
				    .147E+00, .788E+01, .283E+01, .287E+02, .160E+02, .191E+02,
				    .405E+01, .791E+01, .336E+00, .226E+02, .528E+01, .372E+01,
				    .651E+01, .177E+02, .141E+01, .206E+02, .176E+02, .297E+02,
				    .261E+01, .114E+02, .164E+02, .383E+01, .469E+01, .181E+01,
				    .151E+02, .157E+02, .189E+00, .146E+02, .506E+01, .195E+02,
				    .433E+01, .182E+02, .208E+02, .138E+02, .170E+02, .389E+01,
				    .221E+02, .157E+02, .629E+01, .126E+02, .144E+02, .230E+02,
				    .121E+01, .233E+02, .132E+02, .114E+02, .120E+02, .790E+01,
				    .102E+02, .108E+02, .307E+02, .964E+01, .906E+01, .848E+01,
				    .790E+01, .245E+02, .732E+01, .674E+01, .615E+01, .557E+01,
				    .497E+01, .437E+01, .375E+01, .311E+01, .244E+01, .172E+01,
				    .925E+00, .239E+01, .197E+02, .257E+02, .361E+01, .160E+02,
				    .792E+01, .720E-01, .483E+01, .407E+01, .606E+01, .267E+02,
				    .317E+02, .101E+01, .508E+01, .228E+02, .217E+01, .233E+02,
				    .158E+02, .277E+02, .338E+01, .461E+01, .425E+01, .584E+01,
				    .792E+01, .328E+02, .812E+00, .218E+02, .454E+01, .707E+01,
				    .195E+01, .550E+01, .316E+01, .167E+02, .793E+01, .438E+01,
				    .159E+02, .445E+01, .338E+02, .130E+02, .561E+01, .629E+00,
				    .684E+01, .158E+02, .249E+02, .174E+01, .307E-04, .597E+01,
				    .294E+01, .153E+01, .239E+02, .416E+01, .793E+01, .464E+01,
				    .348E+02, .539E+01, .458E+00, .662E+01, .280E+02, .153E+01,
				    .465E+01, .651E+01, .794E+01, .272E+01, .358E+02, .394E+01,
				    .484E+01, .264E+02, .159E+02, .516E+01, .151E+00, .639E+01,
				    .450E+01, .260E+02, .270E+02, .133E+01, .189E+02, .514E-03,
				    .715E+01, .250E+02, .250E+01, .794E+01, .368E+02, .371E+01,
				    .503E+01, .235E+02, .494E+01, .840E-01, .617E+01, .791E+01,
				    .457E+01, .113E+01, .222E+02, .282E+02, .147E+02, .268E+02,
				    .7940E+01,  .2540E+02,  .2290E+01,  .3780E+02,
				    .2410E+02,  .2080E+02,  .2280E+02,  .3490E+01,  .5220E+01,
				    .1950E+02,  .2140E+02,  .8820E+01,  .4720E+01,  .2010E+02,
				    .2810E+02,  .3150E-01,  .1820E+02,  .4640E+01,  .5940E+01,
				    .1890E+02,  .1700E+02,  .1760E+02,  .4690E+01,  .2910E+02,
				    .1640E+02,  .1580E+02,  .9390E+00,  .1510E+02,  .1450E+02,
				    .2720E-02,  .1390E+02,  .1340E+02,  .1280E+02,  .1220E+02,
				    .1160E+02,  .1100E+02,  .3880E+02,  .9910E+01,  .7950E+01,
				    .1040E+02,  .2070E+01,  .9840E+01,  .9260E+01,  .8680E+01,
				    .8100E+01,  .7520E+01,  .6930E+01,  .6340E+01,  .5740E+01,
				    .5130E+01,  .4510E+01,  .3870E+01,  .3210E+01,  .2510E+01,
				    .1760E+01,  .9370E+00,  .5410E+01,  .3270E+01,  .1260E+01,
				    .2110E+02,  .4490E+01,  .5720E+01,  .6950E+01,  .1120E+02,
				    .4840E+01,  .3770E+00,  .7950E+01,  .3980E+02,  .1860E+01,
				    .3010E+02,  .3050E+01,  .5590E+01,  .1260E+02,  .4270E+01,
				    .5500E+01,  .6720E+01,  .3120E+02,  .5000E+01,  .2900E+00,
				    .4080E+02,  .7960E+01,  .9060E-02,  .1430E+02,  .1660E+01,
				    .5780E+01,  .1670E+02,  .2840E+01,  .4510E+01,  .4050E+01,
				    .5270E+01,  .6500E+01,  .5180E+01,  .2100E+00,  .7960E+01,
				    .4180E+02,  .1620E+02,  .2330E+02,  .3220E+02,  .7270E+00,
				    .2600E+01,  .5940E+01,  .1910E+01,  .5040E+01,  .3320E+02,
				    .6270E+01,  .5370E+01,  .4280E+02,  .1810E+02,  .1370E+00,
				    .6130E+01,  .6280E+00,  .2280E-01,  .1200E+01,  .1800E+01,
				    .4790E+01,  .5680E+01,  .4380E+02,  .5560E+01,  .7480E-01,
				    .3430E+02,  .2020E+02,  .5340E+00,  .1030E+01,  .2560E+02,
				    .1100E+01,  .6260E+01,  .4240E+01,  .3370E+01,  .3530E+02,
				    .2300E+01,  .4480E+02,  .2910E+01,  .5750E+01,  .2800E+02,
				    .5600E-01,  .6460E+01,  .2240E+02,  .2660E+02,  .4400E+00,
				    .5500E+01,  .2720E+02,  .9910E+00,  .2530E+02,  .2590E+02,
				    .3000E+01,  .4580E+02,  .2390E+02,  .2460E+02,  .2190E+01,
				    .4690E-01,  .3630E+02,  .2790E+01,  .2330E+02,  .2260E+02,
				    .5940E+01,  .2200E+02,  .2140E+02,  .2070E+02,  .2010E+02,
				    .1950E+02,  .3500E+00,  .1890E+02,  .5560E+01,  .1830E+02,
				    .1770E+02,  .1710E+02,  .1650E+02,  .1590E+02,  .8910E+00,
				    .1530E+02,  .1470E+02,  .2460E+02,  .1410E+02,  .1350E+02,
				    .1290E+02,  .6540E+01,  .1240E+02,  .1180E+02,  .3730E+02,
				    .1120E+02,  .2780E+02,  .1060E+02,  .1000E+02,  .9470E+01,
				    .8880E+01,  .8300E+01,  .1480E+01,  .7710E+01,  .7110E+01,
				    .6780E+01,  .6510E+01,  .5890E+01,  .5270E+01,  .4630E+01,
				    .3970E+01,  .3280E+01,  .1280E+01,  .8940E+00,  .4730E+00,
				    .2070E+01,  .2700E+01,  .3320E+01,  .6140E+01,  .2700E+00,
				    .5660E+01,  .7950E+00,  .1380E+01,  .1980E+01,  .2600E+01,
				    .3140E+01,  .6330E+01,  .2670E+02,  .3880E+01,  .7070E+01,
				    .3870E+00,  .5800E+01,  .8230E-01,  .6850E+00,  .1280E+01,
				    .6760E+01,  .1830E+01,  .2510E+01,  .3040E+01,  .6530E+01,
				    .5950E+01,  .8460E+00,  .5950E+00,  .7360E+01,  .1160E+01,
				    .1770E+01,  .2360E+01,  .3100E+01,  .6720E+01,  .1370E+00,
				    .6120E+01,  .1010E+01,  .1220E+01,  .7660E+01,  .3280E+01,
				    .6920E+01,  .2370E+01,  .6910E+01,  .3080E+01,  .1270E+00,
				    .6290E+01,  .8000E+01,  .3440E+01,  .1800E+01,  .7100E+01,
				    .4070E+01,  .6480E+01,  .6610E+00,  .8380E+01,  .1260E+01,
				    .7280E+01,  .2710E+02,  .6990E+01,  .2640E+02,  .2580E+02,
				    .8850E+01,  .2510E+02,  .6670E+01,  .2450E+02,  .2390E+02,
				    .2330E+02,  .2260E+02,  .2200E+02,  .2140E+02,  .2080E+02,
				    .1510E+01,  .2020E+02,  .3250E+01,  .1960E+02,  .1900E+02,
				    .9440E+01,  .1840E+02,  .1920E+01,  .1780E+02,  .1720E+02,
				    .1660E+02,  .7460E+01,  .1610E+02,  .5600E+01,  .3830E+01,
				    .1550E+02,  .5040E+01,  .1490E+02,  .1430E+02,  .1370E+02,
				    .1320E+02,  .1260E+02,  .1200E+02,  .1140E+02,  .1080E+02,
				    .1020E+02,  .9660E+01,  .9070E+01,  .8480E+01,  .7880E+01,
				    .7270E+01,  .6660E+01,  .6030E+01,  .5390E+01,  .2370E+01,
				    .2030E+01,  .1680E+01,  .1290E+01,  .1020E+02,  .6860E+01,
				    .1740E+00,  .1100E+02,  .1320E+01,  .3270E+01,  .1210E+02,
				    .6970E+00,  .7640E+01,  .2440E+01,  .6040E+01,  .1330E+02,
				    .1480E+02,  .7050E+01,  .6970E+01,  .1650E+02,  .1130E+01,
				    .3320E+01,  .7810E+01,  .3400E+01,  .2990E+01,  .7250E+01,
				    .1260E+00,  .7980E+01,  .6770E+01,  .3190E+01,  .4380E+01,
				    .7440E+01,  .4580E-01,  .7830E+00,  .6910E+01,  .8140E+01,
				    .2190E+00,  .5380E+01,  .4170E+01,  .6820E+01,  .7640E+01,
				    .8310E+01,  .6220E+00,  .7070E+01,  .1630E+01,  .5160E+01,
				    .2770E+01,  .7830E+01,  .4720E+00,  .7230E+01,  .8440E+01,
				    .1440E+01,  .8030E+01,  .8610E+01,  .5820E+00,  .2550E+01,
				    .7410E+01,  .2640E+02,  .2580E+02,  .1750E-03,  .2520E+02,
				    .1260E+01,  .2460E+02,  .4720E+01,  .2350E+01,  .2400E+02,
				    .2330E+02,  .2270E+02,  .2210E+02,  .2150E+02,  .2100E+02,
				    .2040E+02,  .1980E+02,  .1920E+02,  .6540E+01,  .1860E+02,
				    .1800E+02,  .1740E+02,  .1680E+02,  .1630E+02,  .1570E+02,
				    .1510E+02,  .1450E+02,  .1390E+02,  .1340E+02,  .1280E+02,
				    .1220E+02,  .1160E+02,  .1100E+02,  .1040E+02,  .9840E+01,
				    .9250E+01,  .8640E+01,  .8030E+01,  .3710E+01,  .3390E+01,
				    .3070E+01,  .2730E+01,  .2410E+01,  .2640E+01,  .3400E+01,
				    .8220E+01,  .2570E+00,  .2160E+00,  .8710E+01,  .7590E+01,
				    .7500E+01,  .8890E+01,  .8410E+01,  .6550E-03,  .1160E+00,
				    .7780E+01,  .1950E+01,  .3100E+01,  .7540E+01,  .8600E+01,
				    .8940E+01,  .4200E-01,  .7970E+01,  .9150E+01,  .1880E-02,
				    .8780E+01,  .6150E+01,  .8160E+01,  .5050E+01,  .5870E+00,
				    .3850E+01,  .2680E+01,  .3910E+01,  .9380E+01,  .8970E+01,
				    .4580E-02,  .2840E+00,  .2160E+01,  .8350E+01,  .9130E+01,
				    .4830E+01,  .4440E+00,  .3640E+01,  .2480E+01,  .3920E+01,
				    .9150E+01,  .4950E+00,  .9600E+01,  .8550E+01,  .1000E-01,
				    .4610E+01,  .3150E+00,  .2280E+01,  .4010E+01,  .9330E+01,
				    .8740E+01,  .9810E+01,  .9260E+01,  .2020E+00,  .5640E+01,
				    .3220E+01,  .2080E+01,  .8180E+01,  .2000E-01,  .2590E+02,
				    .9500E+01,  .2530E+02,  .2470E+02,  .2410E+02,  .2350E+02,
				    .2290E+02,  .2230E+02,  .2170E+02,  .2110E+02,  .2050E+02,
				    .2000E+02,  .1940E+02,  .1880E+02,  .1820E+02,  .1760E+02};

    static const double el[1151]={712.962,  751.948, 1527.675,  295.458,   72.409,  769.490,
				  11.568, 1105.888,  405.486,  497.424,  246.754, 1470.888,
				  1057.475,  183.068,  856.081, 1154.462,  434.109,  184.357,
				  144.750,  226.134, 1584.658,  347.970,  225.783,  686.506,
				  3.643,  110.903, 1010.594,  273.645,  673.097,  833.248,
				  465.603, 1642.703,   49.321,  405.541, 1414.963,  564.310,
				  608.694,  636.153,  202.325,  791.870,   81.766,  468.127,
				  535.870,   22.065, 1510.442,  325.813,  946.649, 1204.057,
				  374.731,   97.724,  347.973,  964.106, 1255.017, 1360.495,
				  57.192,  146.187,  636.099,    3.578,  919.433,  382.566,
				  919.363,   12.113,  599.109,  162.710,  270.538, 1392.980,
				  25.276, 1307.066,   30.677,     .015,  564.271,  529.975,
				  43.340,   37.277,  320.276, 1041.865,  712.947,   37.337,
				  875.510,  875.541, 1360.510, 1255.051,  295.426, 1307.083,
				  443.738,   66.071, 1280.589, 1701.985, 1010.539,  127.839,
				  127.855,  794.816,  529.905,  113.001,  833.202, 1204.086,
				  3.577,   93.437, 1173.198,   22.051, 1642.720,  881.767,
				  16.967,  497.385,  270.573, 1070.922,  973.854, 1470.864,
				  57.144,  320.226,  791.787, 1106.611, 1141.487,  509.575,
				  1154.446,  247.913,  125.413,   97.699, 1584.612,  599.123,
				  964.027,  465.599, 1414.934,   12.078,  751.893,   11.576,
				  84.874,  162.675, 1105.923, 1527.745, 1207.700,  225.742,
				  435.473,  161.891, 1586.209,  172.978,  134.809,  713.047,
				  216.116,  579.929,  101.361,   81.706, 1058.673,   72.372,
				  205.299,  263.879,   25.302,   72.894, 1472.057, 1245.680,
				  1313.761,  406.198,   49.273,  374.774,  316.434,  675.525,
				  61.675,   30.662, 1012.679,  202.984,   16.916,  185.798,
				  1417.693, 1527.676,    8.275,  673.056, 1057.503, 1424.857,
				  378.414,  202.358,  373.655,   11.548,   43.320,  639.124,
				  22.118,   51.800,  967.826,  654.757, 1541.048,  167.657,
				  110.903,   37.338, 1364.418,  351.707,  435.327,   57.206,
				  603.989,   43.464,  248.530, 1354.412,  924.251,   65.998,
				  150.653, 1312.359,   81.719,    5.711,  326.296, 1768.078,
				  570.016,  434.013,   36.100,  881.901,  110.877,  134.964,
				  501.653,    8.247, 1261.596,  246.722,  591.495,  144.663,
				  734.125,  518.625,  302.028, 1708.920,  669.041, 1155.730,
				  751.897,   93.394,  450.483,   30.108,  537.278,  298.679,
				  144.679,  840.641,  751.328,  387.330,  120.410, 1211.942,
				  279.006,   11.497, 1650.864, 1467.553,  329.140,  505.824,
				  572.453,  838.329,   16.983,  183.128,  800.694,  125.372,
				  107.102,  275.788, 1163.588,  257.162, 1594.112,  183.102,
				  475.488,  817.997,  227.550,  353.193,  498.178,  929.734,
				  761.944,  295.904,   94.974,  226.130, 1116.340,  184.286,
				  236.638, 1538.461,  161.901,   30.664,   22.103,  647.670,
				  446.446,  146.139, 1258.917,  835.678,  724.503,   84.061,
				  113.029, 1070.393,  273.691,   84.852, 1025.749,  217.207,
				  1484.115,   61.686,  226.122,   43.401,   30.067,   25.257,
				  36.149,   51.827,   72.377,  418.614,   97.685, 1150.477,
				  1257.824,  127.835, 1047.973,  688.162,   74.443,  412.335,
				  162.710,   49.286,  202.966, 1369.774, 1025.650,  906.434,
				  202.391,  199.086, 1430.970,  950.170,  246.763,  325.823,
				  391.981,  349.644,  727.363, 1486.298,   65.931,  653.025,
				  295.912,  567.108, 1126.098,   37.290,  857.338,  982.109,
				  182.163,   72.846, 1378.929,  273.716,  349.724, 1843.380,
				  248.564,  366.553,  769.458,   58.718,  619.191,  382.537,
				  475.958,  939.772,  408.192,  166.345,  924.138, 1328.189,
				  1366.889,   21.629, 1784.241,  342.326,  999.290,  101.334,
				  686.513,  586.459,  811.499, 1230.926,  471.309,  898.636,
				  151.827,  298.599,  408.178, 1278.652,   25.216,  325.773,
				  608.683,  443.748, 1726.304,  319.303,   57.208,  134.738,
				  555.030,  539.074,  858.703,  640.744,  138.512,  544.000,
				  535.834, 1230.318,   30.116,  297.581, 1669.469,  524.803,
				  468.113,  611.387,  353.182,  173.038, 1096.704,  819.972,
				  509.570, 1340.198,  126.498,  900.149,   36.086, 1183.185,
				  382.480,  405.542,  276.961, 1613.936,  471.335,  495.678,
				  347.925, 1017.522, 1510.429,  216.111, 1479.680,  688.354,
				  782.544,  115.585,  295.446,   43.428,  616.686, 1137.254,
				  247.882,  257.643,  205.308, 1559.604,   81.704,  167.698,
				  270.552,  225.745,  320.238,  134.932,  185.728,  412.324,
				  150.609,  107.096,  120.392,  374.690,   94.979,   84.078,
				  74.372,   65.970,   58.668,   52.667,  579.917,  434.092,
				  467.855,  498.128, 1392.982,  263.925,  746.217,  567.080,
				  105.873,  443.725,   51.838,  719.131, 1092.525, 1198.700,
				  769.884,  640.730,  239.527, 1453.786,  993.175, 1506.474,
				  719.163,  441.234,  711.192, 1280.577,   97.462,  316.443,
				  61.690,  539.028,  802.265, 1048.997,  693.826,  475.936,
				  222.511,  654.704, 1454.546, 1927.636,  855.986,  415.914,
				  890.025,  677.268,  509.598, 1173.195,  373.626, 1006.671,
				  72.354,  206.797, 1115.576, 1403.820,  982.435, 1305.087,
				  1868.510,  110.925,  391.695, 1090.724,   84.838,  644.646,
				  1070.903,  946.673,  734.143,  544.025,  965.547,  435.337,
				  775.419,  802.213, 1079.492,  192.284, 1354.294,  611.461,
				  47.827, 1810.585,  368.678,  973.850,  579.892,  613.226,
				  925.624,   97.713, 1181.196,  178.971, 1305.970,  501.648,
				  52.617, 1753.861,  881.758,  113.009,  346.962, 1041.761,
				  1415.963,  817.945,  582.907,  616.699, 1192.830,  794.833,
				  887.003,  861.560,  166.959, 1287.449, 1258.848,   58.707,
				  1698.338,  712.962,  326.347,  688.423,  572.442,  654.701,
				  553.889, 1218.448,  636.125,  849.482,  146.138,  144.632,
				  564.298, 1141.463,  156.048,  890.028, 1212.926,  835.606,
				  127.787,  751.884,  924.181,  497.357,  673.031,   65.898,
				  1017.492,  599.059, 1644.017,  435.382,  529.880, 1398.259,
				  307.033, 1115.623,  465.601,  378.357,  406.229,  906.419,
				  326.269,  351.667,  693.763,  302.016,  279.007,  257.176,
				  236.565, 1218.456,  217.247,  199.037,  182.127,  166.420,
				  151.814,  138.509,  126.406,  115.604,  105.902,   97.401,
				  90.201,  526.072,  647.617, 1326.074,  813.263, 1299.307,
				  952.242,  146.337, 1168.206,   74.389, 1590.896, 1438.365,
				  734.122,  288.820,  184.351,  769.897,  499.456, 1513.635,
				  1245.693, 1555.416,  778.145, 1124.786,   84.084, 1538.977,
				  999.274,  775.421,  271.908,  727.381,  162.639, 2020.615,
				  474.141,  227.559,  744.328,  982.408, 1047.457, 1082.468,
				  1410.258,   94.973,  817.950, 1326.089, 1488.258,  256.196,
				  1961.497, 1354.366,  855.965,  449.927,    3.582,  275.858,
				  711.712,  183.102,  811.447, 1041.351, 1096.714,  107.078,
				  861.574, 1438.741,  241.684, 1903.580, 1541.055,  426.913,
				  202.324,  329.134, 1147.098,  680.296,  906.385, 1001.534,
				  120.361, 1424.858, 1467.593, 1390.424,  228.373, 1846.964,
				  84.151,  900.134,  946.607,  405.200, 1079.471,   12.046,
				  387.363, 1313.726,  649.982, 1198.646,  952.226,  962.818,
				  134.889, 1207.638, 1343.308,  216.262, 1791.449,  450.519,
				  90.141,  384.588, 1106.573, 1597.285, 1438.347, 1479.633,
				  1251.300, 1366.878,  621.008,  999.365,
				  1258.952, 1010.547, 1155.808,  925.344,  150.691,
				  919.460, 1057.459,  518.609, 1297.533,  964.112,
				  993.199,  205.493,  833.236,  246.730, 1737.175,
				  875.473,  751.860,  791.846,   97.472, 1041.848,
				  713.034,  675.523,  365.317,  639.138,  604.015,
				  25.308,  570.057,  537.332,  505.791,  475.568,
				  446.439,  418.619, 1047.518,  591.418, 1305.114,
				  391.998,  593.196,  366.582,  342.367,  319.356,
				  297.545,  277.037,  257.630,  239.524,  222.620,
				  206.816,  192.313,  179.011,  166.910,  156.109,
				  146.409,  137.908,  167.669,  889.131,  226.072,
				  1181.260, 1252.820, 1684.062, 2182.656,  669.044,
				  105.963,  347.207, 1359.980, 1096.809,  566.685,
				  1090.777,  854.118,  185.780,  751.345, 1209.307,
				  1632.150, 2122.345, 1141.494,  115.555,  330.197,
				  1147.171, 1416.040,   43.347,  838.278,  541.273,
				  205.355, 1555.410,  820.206,  295.921, 1167.094,
				  1581.538, 2063.335,  126.448,  314.487, 1473.173,
				  1198.658,  929.803, 1287.485, 1192.821,  517.063,
				  787.594,  225.760, 1125.982, 1532.026, 1245.672,
				  2005.425,  138.541, 1251.325, 1025.690,  299.978,
				  247.907,  494.052,   66.048,  756.183, 1086.071,
				  1483.715, 1948.715, 1305.109,  151.935,  286.669,
				  1299.339, 1126.119,  472.342,  273.676, 1398.267,
				  725.972,  270.619,  349.676, 1047.460, 1354.375,
				  1436.605, 1359.980, 1893.306,  166.430, 1706.427,
				  274.660,  295.455, 1230.987, 1586.138,  451.733,
				  130.654, 1645.227,  696.962, 1470.920, 1527.621,
				  1009.950, 1416.062, 1360.558, 1414.928, 1390.795,
				  93.455, 1410.340, 1838.997, 1307.052, 1255.042,
				  182.126, 1204.095, 1154.464, 1105.955, 1058.715,
				  1012.634,  432.423,  967.892,  137.945,  924.230,
				  881.891,  840.742,  800.707,  761.969,  669.152,
				  724.439,  688.209, 1340.201,  653.084,  619.160,
				  586.539,  320.239,  555.021,  524.805, 1467.596,
				  495.791, 1513.684,  467.878,  441.268,  415.858,
				  391.751,  368.744,  973.739,  346.939,  326.434,
				  347.933,  307.030,  288.927,  271.925,  256.224,
				  241.722,  228.421,  216.321,  205.421,  195.821,
				  1346.085, 1785.888, 2292.948,  199.126,  414.314,
				  146.436,  642.542,  938.730, 1302.576, 1733.980,
				  2232.742,  217.224, 1453.876,  408.138,  405.576,
				  397.405,  156.027,  125.411,  617.133,  904.820,
				  374.797, 1260.366, 1683.372, 2173.635,  236.631,
				  166.919,  325.873,  593.024,  468.117,  872.211,
				  1219.258, 1633.864, 2115.830,  257.229,  367.187,
				  179.011,  570.015,  840.802,  535.876, 1179.449,
				  434.062, 1585.557,  279.048, 2059.124,  161.974,
				  192.303,  608.665,  471.367, 1538.549,  302.043,
				  2003.618,  206.896,  527.697,  686.578, 1103.333,
				  326.284, 1762.606,  498.193, 1701.983, 1642.723,
				  769.494, 1584.625,  222.590, 1527.765, 1472.086,
				  1417.629, 1364.464, 1312.413, 1261.558, 1212.013,
				  753.776, 1163.567,  187.357, 1116.428, 1070.390,
				  857.385, 1025.656, 1067.025,  982.124,  939.796,
				  898.669,  351.666,  858.746, 2411.740, 1447.935,
				  820.024, 1896.307,  782.505,  746.287,  711.171,
				  677.357,  644.645,  613.234,  583.024,  553.916,
				  526.109,  499.503,  474.097,  449.993,  426.989,
				  405.186,  384.684,  365.282,  347.181,  330.280,
				  314.479,  299.978,  286.678,  950.216,  239.484,
				  203.016, 1047.945,  727.168,  195.748, 1150.529,
				  382.500,  378.446, 1032.017, 2351.537, 1257.820,
				  1369.771,  257.680,  567.144, 1486.333,  701.859,
				  205.439,  406.197, 1362.222,  539.135,  276.977,
				  457.563,  435.444,  216.330, 1321.215, 1744.292,
				  297.575,  443.055,  654.643,  228.422,  465.633,
				  248.627, 2177.927, 1695.987,  640.759,  319.375,
				  497.393,  632.935,  241.713,  903.887, 2122.525,
				  1242.702,  342.376,  612.327,  256.205,  529.969,
				  874.879,  366.580,  564.311,  443.799,  611.436,
				  271.998, 1828.597, 1768.137,   30.626, 1708.883,
				  847.072, 1650.931, 2015.119, 1168.989, 1594.084,
				  1538.539, 1484.098, 1430.959, 1379.023, 1328.190,
				  1278.659, 1230.331, 1183.205,  719.181, 1137.281,
				  1092.558, 1049.038, 1006.720,  965.603,  925.688,
				  886.974,  849.562,  813.251,  778.241,  744.332,
				  711.725,  680.318,  650.112,  621.007,  593.203,
				  566.599,  541.296,  517.094,  494.092,  472.290,
				  451.789,  432.388,  414.288,  367.187,  381.687,
				  391.984,  298.710,  574.911,  599.095,  288.891,
				  254.106,  636.124,  418.594,   49.308,  557.903,
				  307.084, 1100.076, 1472.661,  263.697,  446.502,
				  673.097,  542.195,  326.378,  712.960,   72.890,
				  475.519,  802.247,  346.973, 2305.227,  747.942,
				  1814.906, 1391.751,  286.679,  794.854,  505.830,
				  101.347,  353.280,  688.382,  368.770,  751.956,
				  2249.726,  726.234, 1767.802, 1353.146,  299.970,
				  537.359,  509.621,  881.841,  391.667,  134.745,
				  2195.524,  705.627, 1315.640,  314.561,  570.071,
				  415.865,  973.852,  835.644,  686.319,  890.003,
				  1677.295, 1279.435,  330.253,  173.046, 1903.723,
				  604.018, 1843.383, 1784.246, 1726.312, 1669.579,
				  1613.949, 1559.621, 1506.495, 1454.571, 1403.848,
				  1354.328, 1306.009, 1258.891, 1212.975, 1168.260};


    static const int ifin1[800]={ 7,   8,  10,  11,  11,  11,  11,  12,  15,  18,
				  21,  22,  22,  23,  24,  25,  26,  29,  29,  29,
				  30,  33,  34,  36,  38,  38,  40,  41,  43,  47,
				  47,  47,  48,  51,  53,  54,  55,  55,  55,  55,
				  55,  55,  57,  59,  59,  62,  64,  65,  65,  65,
				  68,  69,  70,  70,  71,  72,  72,  74,  76,  79,
				  80,  81,  82,  84,  84,  86,  87,  91,  91,  92,
				  92,  92,  95,  96,  96,  96,  96,  98, 100, 101,
				  102, 103, 104, 106, 107, 108, 108, 111, 112, 115,
				  116, 116, 117, 119, 120, 121, 121, 122, 123, 125,
				  128, 128, 129, 129, 130, 130, 130, 132, 133, 133,
				  134, 136, 136, 137, 139, 141, 142, 145, 145, 147,
				  150, 151, 151, 152, 154, 154, 155, 158, 158, 159,
				  159, 160, 162, 163, 164, 168, 170, 170, 173, 174,
				  175, 175, 177, 177, 180, 181, 182, 184, 184, 184,
				  185, 185, 185, 188, 189, 191, 191, 192, 193, 194,
				  195, 197, 197, 199, 199, 199, 199, 202, 202, 202,
				  203, 206, 206, 208, 211, 213, 215, 219, 220, 222,
				  222, 223, 223, 226, 228, 228, 229, 229, 229, 232,
				  233, 234, 234, 236, 237, 238, 241, 242, 242, 244,
				  244, 244, 247, 247, 247, 248, 249, 251, 253, 255,
				  257, 257, 258, 258, 260, 261, 263, 263, 266, 267,
				  267, 269, 271, 271, 272, 273, 274, 277, 279, 281,
				  286, 288, 290, 291, 291, 296, 298, 299, 301, 302,
				  303, 305, 305, 306, 309, 310, 311, 311, 313, 317,
				  318, 319, 320, 320, 322, 323, 325, 327, 327, 327,
				  329, 330, 331, 332, 333, 335, 336, 339, 339, 340,
				  343, 344, 344, 345, 346, 348, 348, 349, 352, 352,
				  355, 356, 359, 360, 360, 360, 362, 363, 363, 363,
				  365, 368, 368, 370, 370, 372, 372, 372, 372, 373,
				  376, 378, 380, 382, 383, 386, 386, 387, 389, 389,
				  389, 390, 393, 393, 395, 398, 400, 401, 403, 404,
				  435, 440, 441, 443, 445, 445, 445, 447, 447, 447,
				  447, 451, 441, 443, 445, 445, 445, 447, 447, 447,
				  447, 451, 453, 454, 456, 457, 460, 460, 460, 462,
				  463, 464, 464, 465, 467, 468, 468, 470, 471, 473,
				  474, 476, 477, 479, 479, 480, 481, 482, 485, 487,
				  487, 491, 493, 493, 495, 496, 496, 496, 498, 499,
				  499, 499, 500, 502, 503, 505, 506, 509, 511, 512,
				  512, 513, 516, 516, 517, 518, 520, 522, 522, 523,
				  526, 527, 529, 529, 530, 532, 532, 533, 536, 539,
				  582, 585, 585, 586, 587, 589, 592, 592, 593, 593,
				  596, 596, 597, 598, 598, 589, 592, 592, 593, 593,
				  596, 596, 597, 598, 598, 599, 599, 600, 604, 607,
				  607, 607, 608, 609, 609, 610, 612, 612, 614, 616,
				  618, 619, 621, 621, 622, 624, 624, 624, 627, 628,
				  628, 630, 632, 633, 634, 636, 636, 637, 637, 638,
				  639, 641, 641, 643, 645, 646, 648, 649, 649, 649,
				  654, 655, 657, 658, 658, 660, 662, 662, 664, 665,
				  666, 667, 668, 670, 673, 675, 677, 679, 680, 682,
				  685, 689, 691, 693, 695, 700, 703, 705, 712, 723,
				  729, 731, 731, 733, 734, 735, 737, 738, 739, 740,
				  741, 742, 742, 743, 745, 746, 747, 747, 748, 749,
				  751, 752, 754, 755, 757, 758, 758, 760, 761, 762,
				  762, 763, 764, 768, 770, 770, 771, 772, 772, 773,
				  774, 774, 775, 776, 778, 780, 782, 782, 782, 784,
				  784, 785, 786, 786, 787, 787, 788, 790, 792, 794,
				  794, 794, 796, 796, 797, 799, 799, 801, 803, 805,
				  807, 809, 809, 809, 812, 813, 814, 815, 816, 818,
				  820, 823, 826, 828, 830, 830, 832, 835, 838, 840,
				  846, 853, 872, 872, 873, 876, 876, 876, 876, 876,
				  878, 878, 879, 879, 879, 880, 881, 882, 885, 885,
				  886, 887, 888, 889, 890, 891, 891, 893, 893, 894,
				  897, 897, 897, 897, 897, 897, 899, 899, 900, 901,
				  902, 903, 904, 906, 906, 906, 906, 907, 908, 908,
				  909, 909, 910, 910, 912, 916, 916, 916, 916, 916,
				  917, 918, 918, 918, 918, 918, 919, 920, 923, 923,
				  923, 923, 923, 924, 924, 925, 925, 926, 926, 927,
				  928, 928, 928, 929, 932, 934, 936, 937, 939, 941,
				  945, 949, 954, 963, 980, 980, 981, 981, 982, 983,
				  984, 984, 984, 988, 990, 991, 992, 992, 992, 992,
				  993, 994, 994, 994, 995, 997, 998, 999,1000,1000,
				  1001,1001,1001,1002,1002,1002,1003,1003,1005,1005,
				  1006,1007,1007,1007,1007,1007,1008,1008,1009,1009,
				  1010,1011,1013,1015,1015,1015,1015,1015,1016,1017,
				  1017,1018,1018,1020,1020,1022,1023,1023,1023,1023,
				  1023,1024,1024,1024,1024,1024,1027,1028,1028,1028,
				  1028,1028,1028,1029,1031,1031,1031,1032,1034,1036,
				  1040,1043,1046,1049,1055,1063,1078,1078,1079,1079,
				  1079,1082,1082,1082,1082,1083,1084,1084,1085,1086};

    static const int ini1[800]={ 1,   1,   1,   1,   1,   2,   4,   4,   4,   4,
				 7,   8,  10,  11,  11,  11,  11,  12,  15,  18,
				 21,  22,  22,  23,  24,  25,  26,  29,  29,  29,
				 30,  33,  34,  36,  38,  38,  40,  41,  43,  47,
				 47,  47,  48,  51,  53,  54,  55,  55,  55,  55,
				 55,  55,  57,  59,  59,  62,  64,  65,  65,  65,
				 68,  69,  70,  70,  71,  72,  72,  74,  76,  79,
				 80,  81,  82,  84,  84,  86,  87,  91,  91,  92,
				 92,  92,  95,  96,  96,  96,  96,  98, 100, 101,
				 102, 103, 104, 106, 107, 108, 108, 111, 112, 115,
				 116, 116, 117, 119, 120, 121, 121, 122, 123, 125,
				 128, 128, 129, 129, 130, 130, 130, 132, 133, 133,
				 134, 136, 136, 137, 139, 141, 142, 145, 145, 147,
				 150, 151, 151, 152, 154, 154, 155, 158, 158, 159,
				 159, 160, 162, 163, 164, 168, 170, 170, 173, 174,
				 175, 175, 177, 177, 180, 181, 182, 184, 184, 184,
				 185, 185, 185, 188, 189, 191, 191, 192, 193, 194,
				 195, 197, 197, 199, 199, 199, 199, 202, 202, 202,
				 203, 206, 206, 208, 211, 213, 215, 219, 220, 222,
				 222, 223, 223, 226, 228, 228, 229, 229, 229, 232,
				 233, 234, 234, 236, 237, 238, 241, 242, 242, 244,
				 244, 244, 247, 247, 247, 248, 249, 251, 253, 255,
				 257, 257, 258, 258, 260, 261, 263, 263, 266, 267,
				 267, 269, 271, 271, 272, 273, 274, 277, 279, 281,
				 286, 288, 290, 291, 291, 296, 298, 299, 301, 302,
				 303, 305, 305, 306, 309, 310, 311, 311, 313, 317,
				 318, 319, 320, 320, 322, 323, 325, 327, 327, 327,
				 329, 330, 331, 332, 333, 335, 336, 339, 339, 340,
				 343, 344, 344, 345, 346, 348, 348, 349, 352, 352,
				 355, 356, 359, 360, 360, 360, 362, 363, 363, 363,
				 365, 368, 368, 370, 370, 372, 372, 372, 372, 373,
				 376, 378, 380, 382, 383, 386, 386, 387, 389, 389,
				 376, 378, 380, 382, 383, 386, 386, 387, 389, 389,
				 389, 390, 411, 425, 428, 428, 429, 430, 434, 434,
				 435, 440, 441, 443, 445, 445, 445, 447, 447, 447,
				 447, 451, 453, 454, 456, 457, 460, 460, 460, 462,
				 463, 464, 464, 465, 467, 468, 468, 470, 471, 473,
				 474, 476, 477, 479, 479, 480, 481, 482, 485, 487,
				 487, 491, 493, 493, 495, 496, 496, 496, 498, 499,
				 499, 499, 500, 502, 503, 505, 506, 509, 511, 512,
				 512, 513, 516, 516, 517, 518, 520, 522, 522, 523,
				 512, 513, 516, 516, 517, 518, 520, 522, 522, 523,
				 526, 527, 529, 529, 530, 564, 573, 579, 579, 579,
				 582, 585, 585, 586, 587, 589, 592, 592, 593, 593,
				 596, 596, 597, 598, 598, 599, 599, 600, 604, 607,
				 607, 607, 608, 609, 609, 610, 612, 612, 614, 616,
				 618, 619, 621, 621, 622, 624, 624, 624, 627, 628,
				 628, 630, 632, 633, 634, 636, 636, 637, 637, 638,
				 639, 641, 641, 643, 645, 646, 648, 649, 649, 649,
				 654, 655, 657, 658, 658, 660, 662, 662, 664, 665,
				 666, 667, 668, 670, 673, 675, 677, 679, 680, 682,
				 685, 689, 691, 693, 695, 700, 703, 705, 712, 723,
				 729, 731, 731, 733, 734, 735, 737, 738, 739, 740,
				 741, 742, 742, 743, 745, 746, 747, 747, 748, 749,
				 751, 752, 754, 755, 757, 758, 758, 760, 761, 762,
				 762, 763, 764, 768, 770, 770, 771, 772, 772, 773,
				 774, 774, 775, 776, 778, 780, 782, 782, 782, 784,
				 784, 785, 786, 786, 787, 787, 788, 790, 792, 794,
				 794, 794, 796, 796, 797, 799, 799, 801, 803, 805,
				 807, 809, 809, 809, 812, 813, 814, 815, 816, 818,
				 820, 823, 826, 828, 830, 830, 832, 835, 838, 840,
				 846, 853, 872, 872, 873, 876, 876, 876, 876, 876,
				 878, 878, 879, 879, 879, 880, 881, 882, 885, 885,
				 886, 887, 888, 889, 890, 891, 891, 893, 893, 894,
				 897, 897, 897, 897, 897, 897, 899, 899, 900, 901,
				 902, 903, 904, 906, 906, 906, 906, 907, 908, 908,
				 909, 909, 910, 910, 912, 916, 916, 916, 916, 916,
				 917, 918, 918, 918, 918, 918, 919, 920, 923, 923,
				 923, 923, 923, 924, 924, 925, 925, 926, 926, 927,
				 928, 928, 928, 929, 932, 934, 936, 937, 939, 941,
				 945, 949, 954, 963, 980, 980, 981, 981, 982, 983,
				 984, 984, 984, 988, 990, 991, 992, 992, 992, 992,
				 993, 994, 994, 994, 995, 997, 998, 999,1000,1000,
				 1001,1001,1001,1002,1002,1002,1003,1003,1005,1005,
				 1006,1007,1007,1007,1007,1007,1008,1008,1009,1009,
				 1010,1011,1013,1015,1015,1015,1015,1015,1016,1017,
				 1017,1018,1018,1020,1020,1022,1023,1023,1023,1023,
				 1023,1024,1024,1024,1024,1024,1027,1028,1028,1028,
				 1028,1028,1028,1029,1031,1031,1031,1032,1034,1036,
				 1040,1043,1046,1049,1055,1063,1078,1078,1079,1079};

    static const int ifin2[800]={ 4,   4,   4,   5,   8,   8,  10,  11,  11,  11,
				  12,  15,  17,  18,  21,  22,  22,  24,  25,  25,
				  28,  29,  29,  29,  33,  34,  35,  36,  38,  40,
				  41,  42,  46,  47,  47,  48,  49,  52,  53,  55,
				  55,  55,  55,  55,  55,  57,  58,  59,  60,  64,
				  65,  65,  65,  66,  69,  69,  70,  70,  72,  72,
				  73,  76,  77,  79,  80,  82,  82,  84,  85,  86,
				  89,  91,  91,  92,  92,  94,  95,  96,  96,  96,
				  96,  99, 101, 101, 103, 104, 106, 106, 107, 108,
				  109, 112, 115, 115, 116, 117, 119, 120, 121, 121,
				  122, 122, 124, 125, 128, 128, 129, 129, 130, 130,
				  131, 132, 133, 134, 135, 136, 137, 138, 140, 141,
				  143, 145, 146, 149, 150, 151, 151, 152, 154, 154,
				  157, 158, 159, 159, 160, 161, 162, 164, 167, 169,
				  170, 170, 173, 174, 175, 177, 177, 178, 180, 182,
				  184, 184, 184, 184, 185, 185, 186, 189, 190, 191,
				  192, 192, 194, 195, 195, 197, 199, 199, 199, 199,
				  202, 202, 202, 203, 203, 206, 207, 209, 213, 215,
				  217, 219, 220, 222, 223, 223, 225, 226, 228, 228,
				  229, 229, 232, 232, 233, 234, 235, 237, 237, 240,
				  242, 242, 242, 244, 244, 245, 247, 247, 247, 248,
				  250, 252, 254, 256, 257, 257, 258, 259, 260, 262,
				  263, 264, 266, 267, 268, 270, 271, 272, 272, 273,
				  276, 277, 280, 285, 286, 289, 290, 291, 293, 298,
				  299, 299, 302, 303, 305, 305, 305, 308, 309, 311,
				  311, 313, 315, 318, 319, 319, 320, 322, 322, 323,
				  327, 327, 327, 328, 330, 330, 332, 333, 334, 335,
				  338, 339, 339, 342, 344, 344, 344, 346, 348, 348,
				  349, 350, 352, 353, 355, 357, 359, 360, 360, 360,
				  363, 363, 363, 364, 367, 368, 370, 370, 371, 372,
				  372, 372, 373, 374, 378, 378, 380, 383, 383, 386,
				  387, 389, 389, 389, 389, 391, 393, 393, 396, 399,
				  403, 404, 405, 408, 411, 425, 428, 428, 429, 430,
				  434, 434, 434, 435, 438, 441, 441, 445, 445, 445,
				  446, 447, 447, 447, 450, 452, 453, 455, 456, 459,
				  460, 460, 461, 463, 464, 464, 464, 465, 468, 468,
				  470, 470, 472, 473, 475, 476, 478, 479, 479, 480,
				  482, 483, 485, 487, 491, 492, 493, 494, 495, 496,
				  496, 497, 499, 499, 499, 500, 500, 503, 505, 505,
				  508, 511, 512, 512, 512, 515, 516, 516, 518, 518,
				  520, 522, 523, 524, 527, 529, 529, 529, 532, 532,
				  536, 539, 542, 548, 550, 556, 559, 564, 573, 579,
				  579, 579, 582, 585, 585, 585, 586, 587, 588, 591,
				  592, 592, 593, 595, 596, 596, 598, 598, 598, 599,
				  600, 602, 605, 607, 607, 607, 608, 609, 609, 611,
				  612, 612, 614, 616, 619, 619, 621, 621, 623, 624,
				  624, 625, 628, 628, 629, 632, 632, 634, 636, 636,
				  636, 637, 637, 638, 641, 641, 642, 643, 646, 647,
				  648, 649, 649, 651, 655, 657, 657, 658, 659, 660,
				  662, 662, 665, 666, 666, 667, 670, 671, 673, 675,
				  679, 679, 681, 684, 686, 690, 693, 695, 697, 700,
				  704, 707, 716, 729, 729, 731, 731, 734, 734, 735,
				  737, 739, 739, 741, 741, 742, 742, 743, 746, 746,
				  747, 748, 749, 751, 752, 753, 754, 757, 757, 758,
				  760, 761, 762, 762, 763, 764, 765, 768, 770, 771,
				  772, 772, 772, 774, 774, 775, 775, 777, 778, 782,
				  782, 782, 783, 784, 784, 786, 786, 786, 787, 788,
				  790, 791, 794, 794, 794, 795, 796, 797, 798, 799,
				  801, 802, 804, 805, 809, 809, 809, 810, 812, 813,
				  814, 816, 817, 818, 822, 825, 828, 828, 830, 832,
				  834, 837, 839, 843, 849, 861, 872, 873, 874, 876,
				  876, 876, 876, 877, 878, 878, 879, 879, 880, 880,
				  881, 884, 885, 886, 886, 887, 889, 889, 891, 891,
				  891, 893, 894, 895, 897, 897, 897, 897, 897, 898,
				  899, 900, 901, 902, 902, 903, 906, 906, 906, 906,
				  906, 907, 908, 908, 909, 909, 910, 912, 914, 916,
				  916, 916, 916, 917, 918, 918, 918, 918, 918, 918,
				  919, 923, 923, 923, 923, 923, 923, 924, 924, 925,
				  926, 926, 926, 928, 928, 928, 929, 931, 933, 935,
				  937, 938, 940, 942, 946, 951, 960, 968, 980, 981,
				  981, 981, 982, 984, 984, 984, 986, 988, 990, 991,
				  992, 992, 992, 993, 994, 994, 994, 994, 996, 998,
				  998,1000,1000,1001,1001,1001,1001,1002,1002,1003,
				  1003,1004,1005,1005,1007,1007,1007,1007,1007,1007,
				  1008,1008,1009,1009,1011,1013,1014,1015,1015,1015,
				  1015,1016,1017,1017,1017,1018,1018,1020,1021,1023,
				  1023,1023,1023,1023,1024,1024,1024,1024,1024,1026,
				  1027,1028,1028,1028,1028,1028,1028,1029,1031,1031,
				  1031,1032,1034,1038,1042,1044,1047,1052,1058,1078,
				  1078,1079,1079,1079,1080,1082,1082,1082,1082,1084};

    static const int ini2[800]={ 1,   1,   3,   4,   4,   4,   5,   8,   8,  10,
				 11,  11,  11,  12,  15,  17,  18,  21,  22,  22,
				 24,  25,  25,  28,  29,  29,  29,  33,  34,  35,
				 36,  38,  40,  41,  42,  46,  47,  47,  48,  49,
				 52,  53,  55,  55,  55,  55,  55,  55,  57,  58,
				 59,  60,  64,  65,  65,  65,  66,  69,  69,  70,
				 70,  72,  72,  73,  76,  77,  79,  80,  82,  82,
				 84,  85,  86,  89,  91,  91,  92,  92,  94,  95,
				 96,  96,  96,  96,  99, 101, 101, 103, 104, 106,
				 106, 107, 108, 109, 112, 115, 115, 116, 117, 119,
				 120, 121, 121, 122, 122, 124, 125, 128, 128, 129,
				 129, 130, 130, 131, 132, 133, 134, 135, 136, 137,
				 138, 140, 141, 143, 145, 146, 149, 150, 151, 151,
				 152, 154, 154, 157, 158, 159, 159, 160, 161, 162,
				 164, 167, 169, 170, 170, 173, 174, 175, 177, 177,
				 178, 180, 182, 184, 184, 184, 184, 185, 185, 186,
				 189, 190, 191, 192, 192, 194, 195, 195, 197, 199,
				 199, 199, 199, 202, 202, 202, 203, 203, 206, 207,
				 209, 213, 215, 217, 219, 220, 222, 223, 223, 225,
				 226, 228, 228, 229, 229, 232, 232, 233, 234, 235,
				 237, 237, 240, 242, 242, 242, 244, 244, 245, 247,
				 247, 247, 248, 250, 252, 254, 256, 257, 257, 258,
				 259, 260, 262, 263, 264, 266, 267, 268, 270, 271,
				 272, 272, 273, 276, 277, 280, 285, 286, 289, 290,
				 291, 293, 298, 299, 299, 302, 303, 305, 305, 305,
				 308, 309, 311, 311, 313, 315, 318, 319, 319, 320,
				 322, 322, 323, 327, 327, 327, 328, 330, 330, 332,
				 333, 334, 335, 338, 339, 339, 342, 344, 344, 344,
				 346, 348, 348, 349, 350, 352, 353, 355, 357, 359,
				 360, 360, 360, 363, 363, 363, 364, 367, 368, 370,
				 370, 371, 372, 372, 372, 373, 374, 378, 378, 380,
				 383, 383, 386, 387, 389, 389, 389, 389, 391, 393,
				 393, 393, 395, 398, 400, 401, 403, 404, 405, 408,
				 411, 425, 429, 429, 432, 434, 435, 438, 441, 441,
				 445, 445, 445, 446, 447, 447, 447, 450, 452, 453,
				 455, 456, 459, 460, 460, 461, 463, 464, 464, 464,
				 465, 468, 468, 470, 470, 472, 473, 475, 476, 478,
				 479, 479, 480, 482, 483, 485, 487, 491, 492, 493,
				 494, 495, 496, 496, 497, 499, 499, 499, 500, 500,
				 503, 505, 505, 508, 511, 512, 512, 512, 515, 516,
				 516, 518, 518, 520, 522, 523, 524, 527, 529, 529,
				 529, 529, 530, 532, 532, 533, 536, 539, 542, 548,
				 550, 556, 559, 564, 573, 579, 581, 583, 585, 586,
				 587, 588, 591, 592, 592, 593, 595, 596, 596, 598,
				 598, 598, 599, 600, 602, 605, 607, 607, 607, 608,
				 609, 609, 611, 612, 612, 614, 616, 619, 619, 621,
				 621, 623, 624, 624, 625, 628, 628, 629, 632, 632,
				 634, 636, 636, 636, 637, 637, 638, 641, 641, 642,
				 643, 646, 647, 648, 649, 649, 651, 655, 657, 657,
				 658, 659, 660, 662, 662, 665, 666, 666, 667, 670,
				 671, 673, 675, 679, 679, 681, 684, 686, 690, 693,
				 695, 697, 700, 704, 707, 716, 729, 729, 731, 731,
				 734, 734, 735, 737, 739, 739, 741, 741, 742, 742,
				 743, 746, 746, 747, 748, 749, 751, 752, 753, 754,
				 757, 757, 758, 760, 761, 762, 762, 763, 764, 765,
				 768, 770, 771, 772, 772, 772, 774, 774, 775, 775,
				 777, 778, 782, 782, 782, 783, 784, 784, 786, 786,
				 786, 787, 788, 790, 791, 794, 794, 794, 795, 796,
				 797, 798, 799, 801, 802, 804, 805, 809, 809, 809,
				 810, 812, 813, 814, 816, 817, 818, 822, 825, 828,
				 828, 830, 832, 834, 837, 839, 843, 849, 861, 872,
				 873, 874, 876, 876, 876, 876, 877, 878, 878, 879,
				 879, 880, 880, 881, 884, 885, 886, 886, 887, 889,
				 889, 891, 891, 891, 893, 894, 895, 897, 897, 897,
				 897, 897, 898, 899, 900, 901, 902, 902, 903, 906,
				 906, 906, 906, 906, 907, 908, 908, 909, 909, 910,
				 912, 914, 916, 916, 916, 916, 917, 918, 918, 918,
				 918, 918, 918, 919, 923, 923, 923, 923, 923, 923,
				 924, 924, 925, 926, 926, 926, 928, 928, 928, 929,
				 931, 933, 935, 937, 938, 940, 942, 946, 951, 960,
				 968, 980, 981, 981, 981, 982, 984, 984, 984, 986,
				 988, 990, 991, 992, 992, 992, 993, 994, 994, 994,
				 994, 996, 998, 998,1000,1000,1001,1001,1001,1001,
				 1002,1002,1003,1003,1004,1005,1005,1007,1007,1007,
				 1007,1007,1007,1008,1008,1009,1009,1011,1013,1014,
				 1015,1015,1015,1015,1016,1017,1017,1017,1018,1018,
				 1020,1021,1023,1023,1023,1023,1023,1024,1024,1024,
				 1024,1024,1026,1027,1028,1028,1028,1028,1028,1028,
				 1029,1031,1031,1031,1032,1034,1038,1042,1044,1047,
				 1052,1058,1078,1078,1079,1079,1079,1080,1082,1082};

    static const int ifin3[800]={ 3,   4,   4,   4,   5,   8,   8,  10,  11,  11,
				  11,  12,  15,  17,  18,  21,  22,  22,  24,  25,
				  25,  28,  29,  29,  29,  33,  34,  35,  36,  38,
				  40,  41,  42,  46,  47,  47,  48,  49,  52,  53,
				  55,  55,  55,  55,  55,  55,  57,  58,  59,  60,
				  64,  65,  65,  65,  66,  69,  69,  70,  70,  72,
				  72,  73,  76,  77,  79,  80,  82,  82,  84,  85,
				  86,  89,  91,  91,  92,  92,  94,  95,  96,  96,
				  96,  96,  99, 101, 101, 103, 104, 106, 106, 107,
				  108, 109, 112, 115, 115, 116, 117, 119, 120, 121,
				  121, 122, 122, 124, 125, 128, 128, 129, 129, 130,
				  130, 131, 132, 133, 134, 135, 136, 137, 138, 140,
				  141, 143, 145, 146, 149, 150, 151, 151, 152, 154,
				  154, 157, 158, 159, 159, 160, 161, 162, 164, 167,
				  169, 170, 170, 173, 174, 175, 177, 177, 178, 180,
				  182, 184, 184, 184, 184, 185, 185, 186, 189, 190,
				  191, 192, 192, 194, 195, 195, 197, 199, 199, 199,
				  199, 202, 202, 202, 203, 203, 206, 207, 209, 213,
				  215, 217, 219, 220, 222, 223, 223, 225, 226, 228,
				  228, 229, 229, 232, 232, 233, 234, 235, 237, 237,
				  240, 242, 242, 242, 244, 244, 245, 247, 247, 247,
				  248, 250, 252, 254, 256, 257, 257, 258, 259, 260,
				  262, 263, 264, 266, 267, 268, 270, 271, 272, 272,
				  273, 276, 277, 280, 285, 286, 289, 290, 291, 293,
				  298, 299, 299, 302, 303, 305, 305, 305, 308, 309,
				  311, 311, 313, 315, 318, 319, 319, 320, 322, 322,
				  323, 327, 327, 327, 328, 330, 330, 332, 333, 334,
				  335, 338, 339, 339, 342, 344, 344, 344, 346, 348,
				  348, 349, 350, 352, 353, 355, 357, 359, 360, 360,
				  360, 363, 363, 363, 364, 367, 368, 370, 370, 371,
				  372, 372, 372, 373, 374, 378, 378, 380, 383, 383,
				  386, 387, 389, 389, 389, 389, 391, 393, 393, 396,
				  400, 401, 403, 404, 405, 408, 411, 425, 428, 428,
				  429, 430, 432, 434, 435, 438, 441, 441, 445, 445,
				  445, 446, 447, 447, 447, 450, 452, 453, 455, 456,
				  459, 460, 460, 461, 463, 464, 464, 464, 465, 468,
				  468, 470, 470, 472, 473, 475, 476, 478, 479, 479,
				  480, 482, 483, 485, 487, 491, 492, 493, 494, 495,
				  496, 496, 497, 499, 499, 499, 500, 500, 503, 505,
				  505, 508, 511, 512, 512, 512, 515, 516, 516, 518,
				  518, 520, 522, 523, 524, 527, 529, 529, 529, 532,
				  532, 533, 536, 539, 542, 548, 550, 556, 559, 564,
				  573, 579, 579, 579, 582, 583, 585, 586, 587, 588,
				  591, 592, 592, 593, 595, 596, 596, 598, 598, 598,
				  599, 600, 602, 605, 607, 607, 607, 608, 609, 609,
				  611, 612, 612, 614, 616, 619, 619, 621, 621, 623,
				  624, 624, 625, 628, 628, 629, 632, 632, 634, 636,
				  636, 636, 637, 637, 638, 641, 641, 642, 643, 646,
				  647, 648, 649, 649, 651, 655, 657, 657, 658, 659,
				  660, 662, 662, 665, 666, 666, 667, 670, 671, 673,
				  675, 679, 679, 681, 684, 686, 690, 693, 695, 697,
				  700, 704, 707, 716, 729, 729, 731, 731, 734, 734,
				  735, 737, 739, 739, 741, 741, 742, 742, 743, 746,
				  746, 747, 748, 749, 751, 752, 753, 754, 757, 757,
				  758, 760, 761, 762, 762, 763, 764, 765, 768, 770,
				  771, 772, 772, 772, 774, 774, 775, 775, 777, 778,
				  782, 782, 782, 783, 784, 784, 786, 786, 786, 787,
				  788, 790, 791, 794, 794, 794, 795, 796, 797, 798,
				  799, 801, 802, 804, 805, 809, 809, 809, 810, 812,
				  813, 814, 816, 817, 818, 822, 825, 828, 828, 830,
				  832, 834, 837, 839, 843, 849, 861, 872, 873, 874,
				  876, 876, 876, 876, 877, 878, 878, 879, 879, 880,
				  880, 881, 884, 885, 886, 886, 887, 889, 889, 891,
				  891, 891, 893, 894, 895, 897, 897, 897, 897, 897,
				  898, 899, 900, 901, 902, 902, 903, 906, 906, 906,
				  906, 906, 907, 908, 908, 909, 909, 910, 912, 914,
				  916, 916, 916, 916, 917, 918, 918, 918, 918, 918,
				  918, 919, 923, 923, 923, 923, 923, 923, 924, 924,
				  925, 926, 926, 926, 928, 928, 928, 929, 931, 933,
				  935, 937, 938, 940, 942, 946, 951, 960, 968, 980,
				  981, 981, 981, 982, 984, 984, 984, 986, 988, 990,
				  991, 992, 992, 992, 993, 994, 994, 994, 994, 996,
				  998, 998,1000,1000,1001,1001,1001,1001,1002,1002,
				  1003,1003,1004,1005,1005,1007,1007,1007,1007,1007,
				  1007,1008,1008,1009,1009,1011,1013,1014,1015,1015,
				  1015,1015,1016,1017,1017,1017,1018,1018,1020,1021,
				  1023,1023,1023,1023,1023,1024,1024,1024,1024,1024,
				  1026,1027,1028,1028,1028,1028,1028,1028,1029,1031,
				  1031,1031,1032,1034,1038,1042,1044,1047,1052,1058,
				  1078,1078,1079,1079,1079,1080,1082,1082,1082,1082};

    static const int ini3[800]={ 1,   3,   4,   4,   4,   5,   8,   8,  10,  11,
				 11,  11,  12,  15,  17,  18,  21,  22,  22,  24,
				 25,  25,  28,  29,  29,  29,  33,  34,  35,  36,
				 38,  40,  41,  42,  46,  47,  47,  48,  49,  52,
				 53,  55,  55,  55,  55,  55,  55,  57,  58,  59,
				 60,  64,  65,  65,  65,  66,  69,  69,  70,  70,
				 72,  72,  73,  76,  77,  79,  80,  82,  82,  84,
				 85,  86,  89,  91,  91,  92,  92,  94,  95,  96,
				 96,  96,  96,  99, 101, 101, 103, 104, 106, 106,
				 107, 108, 109, 112, 115, 115, 116, 117, 119, 120,
				 121, 121, 122, 122, 124, 125, 128, 128, 129, 129,
				 130, 130, 131, 132, 133, 134, 135, 136, 137, 138,
				 140, 141, 143, 145, 146, 149, 150, 151, 151, 152,
				 154, 154, 157, 158, 159, 159, 160, 161, 162, 164,
				 167, 169, 170, 170, 173, 174, 175, 177, 177, 178,
				 180, 182, 184, 184, 184, 184, 185, 185, 186, 189,
				 190, 191, 192, 192, 194, 195, 195, 197, 199, 199,
				 199, 199, 202, 202, 202, 203, 203, 206, 207, 209,
				 213, 215, 217, 219, 220, 222, 223, 223, 225, 226,
				 228, 228, 229, 229, 232, 232, 233, 234, 235, 237,
				 237, 240, 242, 242, 242, 244, 244, 245, 247, 247,
				 247, 248, 250, 252, 254, 256, 257, 257, 258, 259,
				 260, 262, 263, 264, 266, 267, 268, 270, 271, 272,
				 272, 273, 276, 277, 280, 285, 286, 289, 290, 291,
				 293, 298, 299, 299, 302, 303, 305, 305, 305, 308,
				 309, 311, 311, 313, 315, 318, 319, 319, 320, 322,
				 322, 323, 327, 327, 327, 328, 330, 330, 332, 333,
				 334, 335, 338, 339, 339, 342, 344, 344, 344, 346,
				 348, 348, 349, 350, 352, 353, 355, 357, 359, 360,
				 360, 360, 363, 363, 363, 364, 367, 368, 370, 370,
				 371, 372, 372, 372, 373, 374, 378, 378, 380, 383,
				 383, 386, 387, 389, 389, 389, 389, 391, 393, 393,
				 395, 398, 400, 401, 403, 404, 405, 408, 411, 425,
				 428, 428, 429, 432, 434, 435, 438, 441, 441, 445,
				 445, 445, 446, 447, 447, 447, 450, 452, 453, 455,
				 456, 459, 460, 460, 461, 463, 464, 464, 464, 465,
				 468, 468, 470, 470, 472, 473, 475, 476, 478, 479,
				 479, 480, 482, 483, 485, 487, 491, 492, 493, 494,
				 495, 496, 496, 497, 499, 499, 499, 500, 500, 503,
				 505, 505, 508, 511, 512, 512, 512, 515, 516, 516,
				 518, 518, 520, 522, 523, 524, 527, 529, 529, 529,
				 530, 532, 532, 533, 536, 539, 542, 548, 550, 556,
				 559, 564, 573, 579, 579, 581, 583, 585, 586, 587,
				 588, 591, 592, 592, 593, 595, 596, 596, 598, 598,
				 598, 599, 600, 602, 605, 607, 607, 607, 608, 609,
				 609, 611, 612, 612, 614, 616, 619, 619, 621, 621,
				 623, 624, 624, 625, 628, 628, 629, 632, 632, 634,
				 636, 636, 636, 637, 637, 638, 641, 641, 642, 643,
				 646, 647, 648, 649, 649, 651, 655, 657, 657, 658,
				 659, 660, 662, 662, 665, 666, 666, 667, 670, 671,
				 673, 675, 679, 679, 681, 684, 686, 690, 693, 695,
				 697, 700, 704, 707, 716, 729, 729, 731, 731, 734,
				 734, 735, 737, 739, 739, 741, 741, 742, 742, 743,
				 746, 746, 747, 748, 749, 751, 752, 753, 754, 757,
				 757, 758, 760, 761, 762, 762, 763, 764, 765, 768,
				 770, 771, 772, 772, 772, 774, 774, 775, 775, 777,
				 778, 782, 782, 782, 783, 784, 784, 786, 786, 786,
				 787, 788, 790, 791, 794, 794, 794, 795, 796, 797,
				 798, 799, 801, 802, 804, 805, 809, 809, 809, 810,
				 812, 813, 814, 816, 817, 818, 822, 825, 828, 828,
				 830, 832, 834, 837, 839, 843, 849, 861, 872, 873,
				 874, 876, 876, 876, 876, 877, 878, 878, 879, 879,
				 880, 880, 881, 884, 885, 886, 886, 887, 889, 889,
				 891, 891, 891, 893, 894, 895, 897, 897, 897, 897,
				 897, 898, 899, 900, 901, 902, 902, 903, 906, 906,
				 906, 906, 906, 907, 908, 908, 909, 909, 910, 912,
				 914, 916, 916, 916, 916, 917, 918, 918, 918, 918,
				 918, 918, 919, 923, 923, 923, 923, 923, 923, 924,
				 924, 925, 926, 926, 926, 928, 928, 928, 929, 931,
				 933, 935, 937, 938, 940, 942, 946, 951, 960, 968,
				 980, 981, 981, 981, 982, 984, 984, 984, 986, 988,
				 990, 991, 992, 992, 992, 993, 994, 994, 994, 994,
				 996, 998, 998,1000,1000,1001,1001,1001,1001,1002,
				 1002,1003,1003,1004,1005,1005,1007,1007,1007,1007,
				 1007,1007,1008,1008,1009,1009,1011,1013,1014,1015,
				 1015,1015,1015,1016,1017,1017,1017,1018,1018,1020,
				 1021,1023,1023,1023,1023,1023,1024,1024,1024,1024,
				 1024,1026,1027,1028,1028,1028,1028,1028,1028,1029,
				 1031,1031,1031,1032,1034,1038,1042,1044,1047,1052,
				 1058,1078,1078,1079,1079,1079,1080,1082,1082,1082};
    


    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.53;  //Debyes
    static const double mmol=48.0;

    double q=0.649698902072*pow(tt.get("K"),1.5);
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
            
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
        
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      if(ifin==0||ifin<ini){

	return complex<double> (0.0,0.0);

      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){

	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening(Frequency(fre[i],"GHz"),tt,pp,mmol,Frequency(2.5,"MHz"),0.76),Frequency(0.0,"GHz"));

	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*fre[i]; 

          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(0.047992745509/tt.get("K"))*(fac2fixed*pow(mu,2.0)/q);  // imaginary part: absorption coefficient in cm^2
	                                                                                                  // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )

	
      }
     
    }
 
  }


  complex<double>  RefractiveIndexProfile::mkAbs_o3_161618(Temperature tt, Pressure pp, Frequency nu){


    static const double fre[1376]={
      1.21070,  1.40107,  3.24421,  4.44257,  5.07082,  6.45606,
      6.46214,  7.18274,  9.22571,  9.64238,  9.82332, 11.81293,
      11.82647, 12.32145, 14.04262, 14.87170, 15.08468, 15.96915,
      17.14708, 18.02649, 19.26349, 19.34807, 20.07627, 21.08581,
      21.68351, 22.91616, 25.51893, 26.38841, 26.68980, 27.60790,
      28.23913, 28.81666, 29.11111, 30.14474, 30.87775, 32.13823,
      32.25339, 32.32639, 32.74316, 33.53720, 33.59888, 33.93079,
      34.10322, 34.61207, 37.60662, 38.26852, 38.42324, 39.44573,
      40.07897, 40.55548, 41.06401, 43.27116, 44.38350, 44.59401,
      44.62618, 46.08673, 46.72318, 47.12931, 47.41900, 48.45840,
      50.03510, 50.37651, 51.06955, 51.64189, 52.14983, 52.73190,
      53.05164, 55.78990, 56.49044, 57.11214, 57.21999, 57.36937,
      58.22631, 58.62853, 58.90553, 58.98963, 59.51822, 60.80101,
      61.31357, 61.39819, 62.09401, 63.00827, 63.24591, 64.66193,
      64.95043, 68.52572, 69.15639, 69.53537, 70.20760, 70.46912,
      75.33346, 75.63018, 75.66417, 78.29599, 78.52193, 78.57124,
      79.68900, 80.20561, 81.15699, 81.36882, 82.27121, 82.66297,
      83.55060, 83.85309, 83.92603, 85.09023, 86.71541, 87.71149,
      88.13735, 90.86654, 92.34901, 93.35448, 93.76880, 94.74870,
      95.57437, 96.67105, 96.86870, 98.64027, 98.65137, 99.74864,
      100.20649,103.43355,103.78249,104.02308,104.63445,104.66194,
      105.77239,106.87446,106.98571,107.20001,107.97867,109.69159,
      110.35937,110.39784,110.67766,112.79684,113.44836,114.25342,
      114.33874,114.61949,114.73743,115.77980,116.21822,118.28928,
      118.98282,119.00652,119.91436,120.82849,121.77770,121.91278,
      123.19865,123.67594,127.37543,127.45341,130.59339,131.46054,
      131.51147,131.80246,131.99127,132.30167,132.52231,136.14401,
      136.61964,137.18896,137.46570,138.07772,138.20511,138.71467,
      140.45848,140.79993,142.04328,143.80623,144.15361,144.42956,
      145.42213,146.06243,148.07961,148.26074,148.66918,149.09690,
      149.53689,150.23368,150.30267,154.58863,155.63695,155.93990,
      156.33691,156.44072,157.27813,157.64697,158.59949,159.16465,
      159.22943,159.46644,159.94657,162.86428,165.32427,166.27687,
      166.71393,166.79824,168.32138,169.11601,169.12907,169.27002,
      169.57357,169.84888,169.99077,171.99468,172.06613,173.26686,
      174.77337,175.17621,175.85839,176.40117,178.01611,179.44935,
      181.02347,181.09009,181.43048,181.91066,182.34172,182.90828,
      183.81597,184.08711,185.12444,185.79099,187.44265,189.03764,
      189.71983,192.39247,192.94900,193.11355,194.32362,194.65798,
      196.42626,196.80010,197.99957,199.20019,201.46688,202.22950,
      202.38556,203.99797,204.59163,205.50254,205.73039,206.58183,
      207.33057,208.08472,210.67238,211.16086,213.35477,215.99872,
      216.88220,217.86269,219.30263,219.30829,220.75569,221.31972,
      221.67886,221.96171,222.37245,222.43228,223.07527,225.03877,
      225.97494,226.86866,227.76401,230.34551,232.05756,232.52508,
      232.61065,232.72369,233.17642,233.35120,234.05609,234.18358,
      235.08209,236.62098,236.77838,237.58731,237.91819,238.11050,
      240.50749,240.63652,240.72718,240.88279,243.14828,244.21995,
      244.35517,244.92848,245.79591,246.82881,247.04143,248.32117,
      249.85553,249.98649,252.00142,252.47524,252.55800,253.04464,
      253.70145,253.82768,254.92297,256.05710,256.31106,256.67155,
      257.76324,258.01642,258.18432,258.63286,259.48289,260.77196,
      260.85656,261.34863,262.04071,264.48142,264.64912,264.92363,
      268.18885,268.26990,269.05626,270.07786,270.74416,271.29182,
      271.88301,272.04176,273.30214,273.87460,274.47497,275.21735,
      275.87025,277.93454,278.10630,279.45353,279.99077,281.47786,
      282.06916,282.26109,282.79926,283.11005,284.84589,286.16590,
      287.00506,287.53565,288.31652,288.32538,288.98279,289.83845,
      292.51278,293.24555,293.81634,294.83623,295.43284,295.79586,
      296.07556,296.40526,296.77259,296.84768,296.96538,297.41369,
      300.55634,302.97931,303.03389,303.93073,307.72839,307.95579,
      308.85807,309.37899,310.26732,311.72203,313.04314,313.99606,
      314.51046,316.45409,318.09095,318.50051,319.28458,319.37540,
      320.36075,320.89596,321.72852,321.81515,323.69618,324.26344,
      324.84141,326.23545,326.39518,326.75229,327.68991,328.35058,
      331.78907,332.55927,332.72412,332.81002,334.01651,335.91561,
      336.23573,337.09179,340.63209,342.09518,342.81981,343.11010,
      343.25458,343.49694,343.51777,344.03505,344.85343,346.53073,
      346.59400,347.26703,349.54691,351.47981,352.11836,352.26070,
      352.71111,353.24723,353.52947,354.47160,355.20874,357.29833,
      357.29960,357.41797,357.54469,358.16141,360.58411,361.11968,
      361.80965,362.24997,362.61347,362.87669,364.51324,366.41657,
      367.32397,367.35722,367.44416,367.53860,367.64723,367.76921,
      367.90274,368.07446,369.00065,371.25908,371.30493,372.59695,
      373.03881,373.22172,374.36477,376.14807,376.71115,377.01207,
      377.50259,379.77293,381.49996,381.95635,382.04049,384.76928,
      384.78635,384.79948,385.12103,385.29918,385.65111,386.07136,
      386.57446,386.80212,388.93012,390.04362,391.29440,391.97457,
      392.11465,393.17538,394.13259,395.91801,395.95097,397.86955,
      399.49470,401.05800,401.25461,401.59086,402.92698,403.44027,
      404.36556,406.53582,406.59498,407.56509,408.16451,408.90905,
      409.01121,409.09286,409.39842,409.41079,410.39895,411.45191,
      411.55042,413.11147,414.85819,416.24018,416.42041,416.51558,
      418.92858,420.51165,420.53513,421.83401,423.07628,423.14086,
      424.85389,425.31568,425.44098,427.10250,427.40775,429.59674,
      430.07153,431.04376,431.08489,431.93129,432.14601,432.93552,
      433.03452,433.04109,433.97195,433.98085,435.51484,440.40239,
      440.51886,440.78997,440.82803,440.85359,442.36363,444.49337,
      445.02359,445.04384,445.06041,445.49603,449.06842,449.49662,
      449.50509,449.57428,449.78999,450.49226,452.86621,455.48542,
      455.51367,455.61197,456.81297,456.89766,456.94716,457.79229,
      457.95642,458.13239,458.49264,458.49899,459.46966,459.51841,
      460.71411,460.93572,461.59860,462.19501,462.57085,462.78686,
      462.89527,462.97203,463.01661,463.10521,463.24839,463.25918,
      463.50350,463.66841,463.86652,464.37959,465.07661,465.09069,
      465.13247,465.27112,465.57704,465.99368,466.50100,466.51057,
      467.16853,467.39917,468.23762,468.30861,468.64011,469.51821,
      469.52980,470.44808,471.33120,471.56604,472.01682,472.63227,
      473.61159,473.65823,474.16089,475.23223,476.36350,477.69281,
      478.28672,478.84256,479.86567,479.88482,480.81084,480.83334,
      481.83326,482.48197,482.65870,482.96312,482.96762,484.55028,
      485.07625,485.38736,485.90768,489.32926,489.35618,489.62506,
      489.75896,490.40345,490.54375,490.83933,490.94956,491.04281,
      493.02604,493.42750,493.93817,493.94619,495.77279,497.66995,
      497.69697,497.73528,498.01955,498.48951,501.62334,502.11550,
      504.18900,504.20181,504.68754,504.69654,507.03933,507.07912,
      507.38591,507.38907,507.98950,508.12085,508.29298,509.30978,
      513.51173,513.52871,514.90757,515.28747,516.74694,518.30695,
      518.31244,518.44813,518.53666,521.67988,521.69490,521.97193,
      523.14180,523.47093,523.50482,524.13541,526.16012,526.83019,
      528.45962,528.46806,528.53782,528.54082,528.75390,531.01496,
      531.69871,531.76344,531.76564,534.36396,534.36882,534.38430,
      536.60446,537.13850,537.19543,537.64362,537.65409,538.70044,
      539.11790,540.96923,542.62762,542.63134,544.51878,545.64855,
      545.65651,547.48854,549.62671,550.44846,550.96046,551.13229,
      552.68145,552.68693,553.28706,554.45594,555.44790,556.09803,
      556.09955,556.33799,557.70863,558.14266,558.22028,559.22392,
      561.67547,561.72999,561.73628,563.06379,563.33596,564.35990,
      564.38276,566.90311,566.90560,567.64302,567.77946,568.92470,
      568.92529,569.58232,569.58630,573.11337,576.60902,576.85819,
      576.86169,577.15570,578.60486,579.99351,580.39191,580.39295,
      581.85702,582.09052,582.20978,582.57828,582.80127,582.85387,
      585.26930,585.77550,585.77918,586.13341,586.77421,586.81389,
      588.41716,591.13620,591.13785,592.40975,592.49106,593.20438,
      593.20479,593.48680,593.48866,594.48768,594.59683,598.13452,
      600.98317,600.99333,600.99553,602.15579,602.21239,602.56882,
      604.64725,604.64795,605.46928,605.82998,606.01564,606.60048,
      609.10048,609.78446,609.78654,610.27681,610.39657,611.69661,
      611.95624,615.30837,615.32958,615.33066,617.36698,617.36778,
      617.44993,617.45021,617.55894,618.92843,620.78631,620.78745,
      622.16715,622.20531,623.98025,625.09017,625.09152,625.56371,
      625.93958,628.29531,628.86612,628.86659,628.92592,629.67728,
      630.02155,631.77535,633.17849,633.59738,633.76086,633.76200,
      636.12410,638.27739,638.57395,638.79307,639.48580,639.48649,
      640.58817,640.71147,640.84415,641.22732,641.22762,641.66298,
      641.66317,642.07819,642.22502,642.52477,643.52080,643.53934,
      644.44788,644.58149,645.11523,645.39685,645.89917,645.90669,
      645.99080,645.99468,646.02552,646.12819,646.22268,646.29884,
      646.48600,646.50248,646.61535,646.67610,646.85773,646.85869,
      647.02614,647.11674,647.17345,647.20729,647.29790,647.30082,
      647.39871,647.42761,647.47671,647.51154,647.53391,647.56424,
      647.57321,647.59497,647.59796,647.61098,647.61173,647.65449,
      648.03515,649.13717,649.13946,649.15182,649.15263,650.55275,
      652.16298,652.32191,653.05053,653.05084,653.76962,654.13975,
      654.48483,654.58846,656.38805,656.75948,657.02914,657.08130,
      657.70843,657.70903,660.15023,661.26387,663.13956,663.60730,
      663.60774,663.72916,665.02113,665.07175,665.07186,665.84512,
      665.84525,667.39255,667.85380,669.04196,669.60675,672.54626,
      672.55760,673.18126,673.18173,675.10720,677.08732,677.20242,
      677.20263,677.43809,677.87139,678.40771,678.98427,679.78431,
      680.29571,681.63063,681.63093,682.06554,682.27075,683.22329,
      683.82297,684.01971,686.64712,687.69645,687.69673,688.90380,
      688.90383,689.29147,689.99789,689.99798,690.43686,693.11450,
      695.11110,696.93648,697.18129,697.18157,697.67762,697.73402,
      700.37592,701.32367,701.32381,702.69069,702.86925,703.94073,
      705.13677,705.53068,705.53082,705.58801,705.68098,705.73165,
      705.85869,706.59435,708.21471,708.37987,711.75551,711.75568,
      714.12279,714.12285,714.44306,717.84641,719.78800,720.82789,
      721.15460,721.15475,721.66234,723.87750,724.39538,724.42402,
      725.41609,725.41618,725.49517,725.85055,726.46394,727.57503,
      728.64719,729.41157,729.41164,729.48425,730.20257,733.51382,
      733.98269,734.04019,735.52556,735.78664,735.78674,738.22127,
      738.22131,740.49217,741.35002,741.40945,742.74841,742.85912,
      742.85923,744.93222,745.10372,745.10381,747.97617,748.21215,
      749.48144,749.48149,749.86413,750.21580,750.21582,750.79297,
      752.10047,752.58302,753.11651,753.27609,753.27612,756.55071,
      757.83843,759.79193,759.79199,760.33449,761.15853,761.48412,
      761.53470,762.27689,762.29474,762.29477,764.72410,766.66813,
      766.66888,768.88878,769.03107,769.03112,769.08833,769.51994,
      769.65027,770.85405,773.52139,773.52143,774.06070,774.27140,
      774.27141,777.04769,777.12681,777.12682,777.36452,777.95894,
      780.13672,780.74968,781.93579,782.48747,783.77339,783.77342,
      784.22447,784.43941,786.34458,786.34459,788.39610,788.48236,
      790.47164,790.47467,790.64591,791.46491,792.93893,792.93895,
      793.43155,794.17310,795.67750,796.07213,796.11832,796.35108,
      797.53761,797.53763,798.30588,798.30589,800.36014,800.96608,
      800.96608,801.38744,801.40866,801.91008,803.77677,805.80766,
      807.73293,807.73295,809.66695,810.09626,810.37211,810.37212,
      812.67120,812.96025,813.02079,813.40062,814.26482,814.27392,
      814.75766,815.46083,815.92320,815.95370,816.82946,816.82947,
      818.12520,818.42552,819.47775,819.48443,819.67566,819.73235,
      820.04328,820.25948,820.33320,820.48590,820.54776,820.57556,
      821.10480,821.15862,821.53165,821.53167,821.84791,822.30778,
      822.32035,822.32035,822.41712,822.59040,823.36365,823.90797,
      823.98981,824.14808,824.79606,824.79606,824.92696,825.17136,
      825.26691,825.32901,825.68627,825.94455,826.41465,826.46609,
      827.10322,827.42861,827.74537,828.24064,828.33656,828.45584,
      828.87411,828.92314,829.35693,829.49427,829.52073,829.76144,
      829.78510,829.78530,829.96976,830.11913,830.16065,830.36321,
      830.48531,830.68638,830.76232,830.94950,830.99525,831.16144,
      831.18802,831.32994,831.34476,831.46180,831.46967,831.47887,
      831.56298,831.56692,831.63873,831.64057,831.67239,831.67240,
      831.69369,831.69449,831.73197,831.73228,831.75717,831.75727,
      831.77243,831.77246,831.78050,831.78051,834.37862,834.37863,
      835.04892,835.43977,838.04109,838.06387,838.17139,838.42823,
      839.19916,840.21170,840.61148,840.63667,840.70470,840.70470,
      842.08261,843.96024,845.50507,845.50507,846.31583,846.31583,
      847.43928,848.61872,848.61872,848.95703,849.35994,854.93955,
      855.59353,855.59354,856.05814,857.07756,857.56647,858.36538,
      858.36538,860.86160,861.23930,861.79181,861.84200,862.32467,
      864.54651,864.56657,864.56657,865.60487,866.00194,868.34710,
      869.45932,869.45932,870.29335,870.29335,870.78002,872.49606,
      876.68766,879.49804,879.49804,879.76652,882.11104,882.33360,
      882.33360,883.25925,884.24714,884.45818,884.75270,885.50586,
      885.60640,885.69812,888.41687,888.41687,890.97350,893.39583,
      893.39583,894.25391,894.25392,895.89991,898.48648,898.70178,
      899.71318,902.02728,903.22322,903.32775,903.38753,903.38753,
      904.98777,905.62572,906.28445,906.28445,906.60719,909.16916,
      909.35615,912.25730,912.25730,912.85657,913.25405,915.56947,
      917.31597,917.31597,918.19848,918.19849,919.18324,922.80658,
      924.33760,925.16960,925.95414,927.26352,927.26353,927.96478,
      928.76747,928.88596,929.16975,929.16975,930.21909,930.21909,
      931.55544,932.76419,933.09182,936.08941,936.08941,936.66866,
      936.70166,939.83106,940.78265,941.22106,941.22107,942.12800,
      942.12800,942.35934,942.64329,942.69961,945.12794,946.68766,
      947.50874,950.27954,950.83765,950.83765,950.93544,951.12749,
      951.12749,952.80471,952.89747,953.07341,953.07341,954.13862,
      954.13862,956.26938,956.81594,957.65233,959.74004,959.91465,
      959.91465,962.79858,963.74344,963.79680,965.11238,965.11238,
      965.44009,965.98141,966.04339,966.04339,967.19873,969.06889,
      971.58011,972.57255,973.10806,973.21488,974.64913,974.64914,
      974.98080,974.98080,976.21870,976.58902,976.96461,976.96461,
      977.09168,977.31514,978.04411,978.04411,979.65849,980.53350,
      981.06081,983.23500,983.73436,983.73436,984.78731,985.04662,
      985.34090,986.17747,987.19226,987.50214,988.35245,988.43609,
      988.59501,988.99113,988.99113,989.62723,989.94554,989.94554,
      990.98800,991.47382,991.75139,992.40854,992.98404,993.86508,
      994.55760,994.84591,995.05568,995.28280,995.33640,996.80376,
      997.05175,998.25096,998.45785,998.45787,998.82477,998.82477,
      999.26827,999.66418};

    static const double flin[1376]={
      .150E+01, .849E+01, .208E+01, .848E+01, .314E+01, .709E+01,
      .158E+01, .725E+01, .451E+01, .318E+01, .582E+01, .464E+01,
      .327E+01, .570E+01, .598E+01, .163E+01, .455E+01, .434E+01,
      .590E+01, .707E+01, .104E+01, .138E+01, .303E+01, .307E+01,
      .728E+01, .319E+01, .830E+01, .121E+01, .129E+01, .242E+01,
      .113E+01, .177E+01, .137E+01, .829E+01, .436E+01, .475E+01,
      .104E+01, .742E+01, .216E+01, .176E+01, .142E+01, .565E+01,
      .690E+01, .317E+01, .614E+01, .298E+01, .950E+00, .127E+01,
      .146E+01, .192E+01, .551E+01, .688E+01, .511E+00, .312E+01,
      .353E+01, .413E+01, .866E+00, .475E+01, .608E+01, .147E+01,
      .208E+01, .746E+01, .278E+01, .485E+01, .811E+01, .305E+01,
      .420E+01, .810E+01, .286E+01, .786E+00, .224E+01, .758E+01,
      .547E+01, .145E+01, .295E+01, .279E+01, .276E+01, .628E+01,
      .671E+01, .117E+01, .239E+01, .283E+01, .145E+01, .255E+01,
      .270E+01, .670E+01, .531E+01, .713E+00, .492E+01, .141E+01,
      .392E+01, .273E+01, .403E+01, .380E+01, .791E+01, .627E+01,
      .764E+01, .497E+01, .255E+01, .791E+01, .774E+01, .528E+01,
      .641E+01, .133E+01, .646E+00, .108E+01, .340E+01, .497E+01,
      .651E+01, .318E+01, .116E+01, .150E+01, .650E+01, .248E+01,
      .257E+01, .511E+01, .343E+01, .385E+01, .123E+01, .435E+01,
      .586E+00, .521E+01, .371E+01, .499E+01, .252E+01, .772E+01,
      .652E+01, .772E+01, .789E+01, .509E+01, .602E+01, .781E+01,
      .231E+01, .999E+00, .645E+01, .408E+01, .675E+01, .408E+01,
      .518E+01, .631E+01, .110E+01, .100E+01, .240E+01, .532E+00,
      .631E+01, .498E+01, .740E+01, .891E+00, .271E+01, .366E+01,
      .359E+01, .491E+01, .662E+01, .796E+01, .752E+01, .803E+01,
      .350E+01, .489E+01, .937E+00, .752E+01, .495E+01, .843E+01,
      .292E+01, .927E+00, .221E+01, .484E+00, .150E+01, .208E+01,
      .798E+01, .611E+01, .480E+01, .664E+01, .611E+01, .489E+01,
      .346E+01, .881E+01, .438E+01, .671E+01, .632E+00, .313E+01,
      .540E+01, .470E+01, .758E+00, .480E+01, .817E+01, .404E+01,
      .732E+01, .469E+01, .909E+01, .732E+01, .328E+01, .334E+01,
      .201E+01, .442E+00, .201E+01, .468E+01, .863E+00, .185E+01,
      .591E+01, .356E+01, .677E+01, .326E+01, .454E+01, .590E+01,
      .564E+00, .929E+01, .556E+01, .378E+01, .815E+01, .437E+01,
      .399E+01, .419E+01, .390E+00, .449E+01, .682E+01, .829E+01,
      .253E+01, .448E+01, .181E+01, .711E+01, .405E+00, .711E+01,
      .940E+01, .470E+01, .306E+01, .563E+01, .681E+01, .452E+01,
      .362E+00, .570E+01, .305E+01, .162E+01, .570E+01, .806E+00,
      .832E+01, .832E+01, .637E+01, .944E+01, .307E+01, .428E+01,
      .171E+00, .159E+01, .832E+01, .683E+01, .427E+01, .373E+00,
      .691E+01, .691E+01, .164E+00, .284E+01, .700E+01, .942E+01,
      .283E+01, .549E+01, .139E+01, .549E+01, .504E+01, .363E+01,
      .812E+01, .811E+01, .682E+01, .504E+01, .587E+01, .756E+00,
      .723E+01, .138E+01, .406E+01, .406E+01, .344E+00, .128E+02,
      .670E+01, .139E+02, .670E+01, .118E+02, .148E+02, .936E+01,
      .107E+02, .157E+02, .262E+01, .964E+01, .679E+01, .848E+01,
      .166E+02, .422E+01, .861E+01, .261E+01, .527E+01, .527E+01,
      .762E+01, .116E+01, .173E+02, .791E+01, .791E+01, .668E+01,
      .719E+01, .115E+01, .673E+01, .579E+01, .179E+02, .384E+01,
      .926E+01, .812E+01, .384E+01, .559E+01, .711E+00, .497E+01,
      .649E+01, .541E+01, .649E+01, .319E+00, .484E+01, .419E+01,
      .184E+02, .612E+01, .240E+01, .664E+01, .347E+01, .239E+01,
      .280E+01, .506E+01, .506E+01, .937E+00, .188E+02, .215E+01,
      .770E+01, .770E+01, .933E+00, .152E+01, .915E+01, .652E+01,
      .852E+00, .551E+01, .362E+01, .362E+01, .833E+00, .905E+01,
      .145E+01, .190E+02, .627E+01, .627E+01, .200E+01, .298E+00,
      .217E+01, .739E+01, .672E+00, .251E+01, .217E+01, .619E+01,
      .299E+01, .484E+01, .484E+01, .716E+00, .192E+02, .580E+01,
      .621E+01, .902E+01, .714E+00, .749E+01, .749E+01, .345E+01,
      .640E+01, .340E+01, .389E+01, .340E+01, .606E+01, .606E+01,
      .100E+02, .430E+01, .192E+02, .194E+01, .194E+01, .695E+01,
      .279E+00, .469E+01, .462E+01, .462E+01, .502E+00, .889E+01,
      .501E+00, .638E+00, .727E+01, .727E+01, .683E+01, .506E+01,
      .150E+01, .149E+01, .759E+01, .192E+02, .318E+01, .318E+01,
      .774E+01, .584E+01, .584E+01, .540E+01, .623E+01, .110E+02,
      .172E+01, .172E+01, .669E+01, .571E+01, .440E+01, .440E+01,
      .877E+01, .302E+00, .263E+00, .302E+00, .191E+02, .706E+01,
      .706E+01, .167E+01, .857E+01, .163E+01, .600E+01, .295E+01,
      .295E+01, .247E+02, .236E+02, .257E+02, .225E+02, .266E+02,
      .563E+01, .563E+01, .752E+01, .213E+02, .149E+01, .149E+01,
      .273E+02, .200E+02, .120E+02, .627E+01, .189E+02, .780E+01,
      .188E+02, .944E+01, .418E+01, .126E+00, .418E+01, .126E+00,
      .866E+01, .280E+02, .188E+01, .684E+01, .684E+01, .669E+01,
      .248E+00, .176E+02, .651E+01, .285E+02, .273E+01, .273E+01,
      .179E+01, .165E+02, .700E+01, .541E+01, .541E+01, .806E+01,
      .806E+01, .127E+01, .127E+01, .103E+02, .187E+02, .288E+02,
      .673E+01, .153E+02, .130E+02, .211E+01, .826E+01, .396E+01,
      .396E+01, .856E+01, .143E+02, .663E+01, .663E+01, .291E+02,
      .692E+01, .250E+01, .250E+01, .132E+02, .236E+00, .113E+02,
      .196E+01, .519E+01, .519E+01, .802E+01, .185E+02, .105E+01,
      .123E+02, .105E+01, .784E+01, .784E+01, .236E+01, .719E+01,
      .292E+02, .710E+01, .141E+02, .114E+02, .374E+01, .374E+01,
      .848E+01, .641E+01, .641E+01, .122E+02, .734E+01, .106E+02,
      .904E+01, .228E+01, .228E+01, .292E+02, .725E+01, .977E+01,
      .262E+01, .496E+01, .496E+01, .182E+02, .210E+01, .836E+00,
      .836E+00, .225E+00, .763E+01, .763E+01, .903E+01, .151E+02,
      .132E+02, .351E+01, .832E+01, .351E+01, .739E+01, .291E+02,
      .841E+01, .619E+01, .619E+01, .765E+01, .289E+01, .205E+01,
      .700E+01, .205E+01, .826E+01, .773E+01, .638E+01, .474E+01,
      .474E+01, .577E+01, .180E+02, .628E+00, .628E+00, .518E+01,
      .751E+01, .988E+01, .741E+01, .741E+01, .459E+01, .142E+02,
      .401E+01, .222E+01, .343E+01, .284E+01, .223E+01, .159E+01,
      .883E+00, .883E+00, .159E+01, .223E+01, .215E+00, .283E+01,
      .342E+01, .289E+02, .399E+01, .456E+01, .513E+01, .329E+01,
      .329E+01, .771E+01, .162E+02, .569E+01, .357E+02, .346E+02,
      .625E+01, .318E+01, .334E+02, .366E+02, .681E+01, .597E+01,
      .597E+01, .736E+01, .836E+01, .322E+02, .374E+02, .791E+01,
      .183E+01, .183E+01, .762E+01, .846E+01, .309E+02, .381E+02,
      .899E+01, .152E+02, .452E+01, .452E+01, .431E+00, .431E+00,
      .952E+01, .295E+02, .178E+02, .719E+01, .719E+01, .286E+02,
      .348E+01, .386E+02, .100E+02, .306E+01, .306E+01, .831E+01,
      .282E+02, .172E+02, .105E+02, .231E+01, .771E+01, .108E+02,
      .852E+01, .206E+00, .575E+01, .575E+01, .110E+02, .161E+01,
      .161E+01, .831E+01, .268E+02, .163E+02, .115E+02, .380E+01,
      .429E+01, .429E+01, .252E+00, .252E+00, .283E+02, .255E+02,
      .697E+01, .697E+01, .811E+01, .119E+02, .779E+01, .176E+02,
      .283E+01, .283E+01, .182E+02, .124E+02, .243E+02, .552E+01,
      .552E+01, .173E+02, .414E+01, .139E+01, .139E+01, .237E+01,
      .128E+02, .199E+00, .117E+02, .828E+01, .786E+01, .230E+02,
      .407E+01, .407E+01, .101E+00, .101E+00, .894E+01, .280E+02,
      .131E+02, .675E+01, .675E+01, .449E+01, .250E+01, .250E+01,
      .174E+02, .219E+02, .881E+01, .261E+01, .261E+01, .183E+02,
      .193E+02, .135E+02, .530E+01, .530E+01, .792E+01, .117E+01,
      .117E+01, .208E+02, .488E+01, .826E+01, .138E+02, .855E+01,
      .384E+01, .384E+01, .192E+00, .240E+01, .126E+02, .653E+01,
      .653E+01, .277E+02, .197E+02, .261E+01, .261E+01, .193E+02,
      .141E+02, .238E+01, .238E+01, .203E+02, .797E+01, .528E+01,
      .172E+02, .508E+01, .508E+01, .188E+02, .961E+01, .775E+01,
      .775E+01, .958E+00, .958E+00, .144E+02, .824E+01, .362E+01,
      .362E+01, .179E+02, .572E+01, .204E+02, .630E+01, .630E+01,
      .277E+01, .277E+01, .912E+01, .801E+01, .186E+00, .273E+02,
      .147E+02, .216E+01, .216E+01, .170E+02, .213E+02, .136E+02,
      .239E+01, .485E+01, .485E+01, .619E+01, .171E+02, .753E+01,
      .753E+01, .753E+00, .753E+00, .162E+02, .902E+01, .149E+02,
      .214E+02, .339E+01, .339E+01, .154E+02, .805E+01, .823E+01,
      .608E+01, .608E+01, .296E+01, .669E+01, .295E+01, .103E+02,
      .146E+02, .194E+01, .194E+01, .224E+02, .270E+02, .151E+02,
      .180E+00, .139E+02, .463E+01, .463E+01, .559E+00, .559E+00,
      .731E+01, .731E+01, .146E+02, .723E+01, .170E+02, .132E+02,
      .224E+02, .808E+01, .235E+01, .317E+01, .317E+01, .126E+02,
      .153E+02, .822E+01, .586E+01, .586E+01, .315E+01, .119E+02,
      .314E+01, .781E+01, .113E+02, .234E+02, .172E+01, .172E+01,
      .107E+02, .954E+01, .101E+02, .267E+02, .440E+01, .440E+01,
      .945E+01, .175E+00, .155E+02, .377E+00, .377E+00, .708E+01,
      .708E+01, .333E+02, .885E+01, .811E+01, .234E+02, .826E+01,
      .843E+01, .768E+01, .111E+02, .709E+01, .106E+02, .100E+02,
      .942E+01, .112E+02, .652E+01, .883E+01, .118E+02, .825E+01,
      .767E+01, .594E+01, .124E+02, .709E+01, .536E+01, .651E+01,
      .594E+01, .478E+01, .536E+01, .130E+02, .478E+01, .420E+01,
      .420E+01, .360E+01, .360E+01, .299E+01, .299E+01, .235E+01,
      .235E+01, .167E+01, .167E+01, .908E+00, .908E+00, .157E+02,
      .136E+02, .142E+02, .169E+02, .294E+01, .294E+01, .148E+02,
      .335E+01, .154E+02, .563E+01, .563E+01, .822E+01, .334E+01,
      .160E+02, .320E+02, .156E+02, .244E+02, .910E+01, .166E+02,
      .150E+01, .150E+01, .171E+02, .229E+01, .813E+01, .417E+01,
      .417E+01, .177E+02, .244E+02, .216E+00, .216E+00, .686E+01,
      .686E+01, .307E+02, .182E+02, .170E+00, .981E+01, .158E+02,
      .188E+02, .272E+01, .272E+01, .355E+01, .167E+02, .541E+01,
      .541E+01, .168E+02, .193E+02, .353E+01, .822E+01, .255E+02,
      .295E+02, .129E+01, .129E+01, .101E+02, .106E+02, .119E+02,
      .198E+02, .815E+01, .255E+02, .395E+01, .395E+01, .839E-01,
      .839E-01, .159E+02, .664E+01, .664E+01, .202E+02, .283E+02,
      .114E+02, .166E+00, .249E+01, .249E+01, .375E+01, .207E+02,
      .219E+01, .518E+01, .518E+01, .265E+02, .372E+01, .822E+01,
      .816E+01, .108E+01, .108E+01, .167E+02, .272E+02, .211E+02,
      .178E+02, .160E+02, .122E+02, .265E+02, .372E+01, .372E+01,
      .641E+01, .641E+01, .215E+02, .262E+02, .394E+01, .128E+02,
      .227E+01, .227E+01, .131E+02, .218E+02, .162E+00, .161E+02,
      .496E+01, .496E+01, .275E+02, .107E+02, .818E+01, .390E+01,
      .822E+01, .879E+00, .879E+00, .253E+02, .275E+02, .167E+02,
      .189E+02, .222E+02, .140E+02, .350E+01, .350E+01, .619E+01,
      .619E+01, .243E+02, .414E+01, .208E+01, .162E+02, .350E+01,
      .350E+01, .225E+02, .205E+01, .205E+01, .819E+01, .285E+02,
      .473E+01, .473E+01, .150E+02, .742E+01, .742E+01, .235E+02,
      .285E+02, .407E+01, .823E+01, .686E+00, .686E+00, .228E+02,
      .137E+02, .327E+01, .327E+01, .227E+02, .167E+02, .200E+02,
      .162E+02, .433E+01, .596E+01, .596E+01, .160E+02, .359E+01,
      .359E+01, .231E+02, .184E+01, .184E+01, .219E+02, .114E+02,
      .820E+01, .295E+02, .451E+01, .451E+01, .295E+02, .719E+01,
      .719E+01, .211E+02, .503E+00, .503E+00, .823E+01, .423E+01,
      .170E+02, .163E+02, .233E+02, .453E+01, .305E+01, .305E+01,
      .204E+02, .196E+01, .574E+01, .574E+01, .210E+02, .166E+02,
      .373E+01, .373E+01, .196E+02, .821E+01, .162E+01, .162E+01,
      .305E+02, .147E+02, .235E+02, .305E+02, .180E+02, .189E+02,
      .428E+01, .428E+01, .697E+01, .697E+01, .163E+02, .336E+00,
      .336E+00, .183E+02, .824E+01, .472E+01, .438E+01, .176E+02,
      .283E+01, .283E+01, .169E+02, .237E+02, .551E+01, .551E+01,
      .191E+02, .121E+02, .163E+02, .822E+01, .389E+01, .389E+01,
      .221E+02, .166E+02, .156E+02, .315E+02, .141E+01, .141E+01,
      .316E+02, .150E+02, .200E+02, .207E+02, .194E+02, .213E+02,
      .188E+02, .220E+02, .164E+02, .491E+01, .181E+02, .144E+02,
      .226E+02, .175E+02, .406E+01, .406E+01, .169E+02, .232E+02,
      .675E+01, .675E+01, .138E+02, .162E+02, .156E+02, .239E+02,
      .132E+02, .150E+02, .189E+00, .189E+00, .144E+02, .239E+02,
      .824E+01, .126E+02, .138E+02, .245E+02, .132E+02, .120E+02,
      .126E+02, .114E+02, .120E+02, .108E+02, .114E+02, .251E+02,
      .108E+02, .102E+02, .102E+02, .961E+01, .182E+01, .157E+02,
      .201E+02, .961E+01, .903E+01, .451E+01, .903E+01, .845E+01,
      .845E+01, .788E+01, .788E+01, .730E+01, .730E+01, .672E+01,
      .672E+01, .614E+01, .614E+01, .556E+01, .556E+01, .257E+02,
      .497E+01, .497E+01, .436E+01, .436E+01, .261E+01, .261E+01,
      .375E+01, .375E+01, .311E+01, .311E+01, .244E+01, .244E+01,
      .172E+01, .172E+01, .924E+00, .924E+00, .529E+01, .529E+01,
      .262E+02, .823E+01, .407E+01, .407E+01, .511E+01, .326E+02,
      .268E+02, .326E+02, .232E+02, .164E+02, .120E+01, .120E+01,
      .166E+02, .273E+02, .383E+01, .383E+01, .652E+01, .652E+01,
      .212E+02, .719E-01, .719E-01, .824E+01, .279E+02, .531E+01,
      .239E+01, .239E+01, .128E+02, .462E+01, .824E+01, .506E+01,
      .506E+01, .336E+02, .164E+02, .426E+01, .426E+01, .336E+02,
      .167E+02, .100E+01, .100E+01, .222E+02, .243E+02, .166E+02,
      .361E+01, .361E+01, .630E+01, .630E+01, .551E+01, .825E+01,
      .168E+01, .217E+01, .217E+01, .824E+01, .165E+02, .484E+01,
      .484E+01, .346E+02, .233E+02, .346E+02, .470E+01, .445E+01,
      .445E+01, .573E+01, .812E+00, .812E+00, .253E+02, .338E+01,
      .338E+01, .607E+01, .607E+01, .825E+01, .178E+02, .136E+02,
      .596E+01, .825E+01, .165E+02, .243E+02, .195E+01, .195E+01,
      .261E-04, .356E+02, .461E+01, .461E+01, .356E+02, .465E+01,
      .465E+01, .629E+00, .629E+00, .621E+01, .477E+01, .264E+02,
      .316E+01, .316E+01, .584E+01, .584E+01, .826E+01, .254E+02,
      .825E+01, .647E+01, .155E+01, .174E+01, .174E+01, .366E+02,
      .366E+02, .137E-03, .708E+01, .708E+01, .439E+01, .439E+01,
      .188E+02, .485E+01, .485E+01, .457E+00, .457E+00, .284E+02,
      .675E+01, .274E+02, .145E+02, .294E+01, .294E+01, .562E+01,
      .562E+01, .826E+01, .264E+02, .480E+01, .276E+02, .826E+01,
      .706E+01, .376E+02, .450E+01, .450E+01, .376E+02, .153E+01,
      .153E+01, .269E+02, .438E-03, .685E+01, .685E+01, .416E+01,
      .416E+01, .505E+01, .504E+01, .740E+01, .261E+02, .151E+00,
      .151E+00, .275E+02, .199E+02, .285E+02, .272E+01, .272E+01,
      .827E+01, .254E+02, .539E+01, .539E+01, .776E+01, .826E+01,
      .247E+02, .386E+02, .386E+02, .481E+01, .457E+01, .457E+01,
      .133E+01, .133E+01, .816E+01, .240E+02, .663E+01, .663E+01,
      .109E-02, .141E+01, .394E+01, .394E+01, .524E+01, .524E+01,
      .233E+02, .285E+02, .839E-01, .839E-01, .859E+01, .226E+02,
      .280E+02, .273E+02, .266E+02, .295E+02, .259E+02, .827E+01,
      .220E+02, .250E+01, .250E+01, .253E+02, .517E+01, .517E+01,
      .246E+02, .827E+01, .213E+02, .239E+02, .906E+01, .232E+02,
      .207E+02, .396E+02, .211E+02, .396E+02, .226E+02, .219E+02,
      .200E+02, .213E+02, .469E+01, .469E+01, .113E+01, .113E+01,
      .194E+02, .206E+02};

    static const double el[1376]={
      810.942, 1561.533,   69.044, 1561.387,  280.556, 1097.290,
      11.090, 1144.555,  472.557,  305.037,  752.528,  504.033,
      215.032,  714.508,  792.025,  768.886,  443.075,  413.633,
      752.976, 1096.834,    6.875,  853.271,  194.636,  256.787,
      1144.558,  330.399, 1505.474,  193.332,  173.518,   80.574,
      214.244,  728.016,  154.802, 1505.252,  441.717,  536.156,
      236.251, 1192.647,   15.527,   57.289,  137.186,  713.570,
      1050.162,  356.937,  832.094,  234.062,  259.354,  896.805,
      120.575,  688.252,  676.528, 1049.822,    3.468,  384.658,
      235.656,  384.686,  283.456,  472.836,  791.922,  105.172,
      649.797, 1192.680,  174.847,  569.519, 1450.395,  413.667,
      411.952, 1450.320,  212.487,  308.757,  612.452, 1241.944,
      675.703,   90.784,  443.771,   93.067,   21.041,  873.380,
      1004.255,  941.351,  576.317,  475.074,   46.762,  541.394,
      507.580, 1003.908,  639.678,  335.060,  604.028,   77.515,
      356.981,  192.067,  383.366,  257.439, 1396.628,  832.026,
      1241.872,  503.747,  156.202, 1396.492, 1292.348,  639.029,
      915.887,   65.372,  362.469,  986.913,   27.735,  639.687,
      959.466,  106.735,   37.364,    1.116,  959.196,    3.449,
      172.809,  603.957,    6.847,  355.862,   54.261,   11.409,
      390.887,   17.132,  330.415,  676.503, 1428.674, 1343.873,
      959.519, 1343.766, 1343.861,  603.551,   23.913, 1292.231,
      138.799, 1033.597,  873.284,  280.282,   31.851,   35.512,
      535.908,  915.894,   44.389,     .039,  154.618,  420.418,
      915.685,  714.584,   40.940,   29.096, 1372.651,  329.544,
      121.582,  569.459, 1004.282,   51.178, 1292.327, 1396.586,
      304.983,  569.069,   35.460, 1292.245,  753.735,   62.461,
      1317.838, 1081.410,  137.497,  451.068,    1.162,  122.537,
      1343.753,  873.437,   44.377,  915.693,  873.276,  794.163,
      304.315,   74.884,  304.387, 1050.179,   22.059, 1264.238,
      569.117,  536.184,   27.680,  835.775, 1450.424,  137.510,
      1241.891,  535.886,   88.345, 1241.828,  280.782, 1211.855,
      121.552,  482.740,    3.417,  878.577, 1130.159,  107.513,
      832.092, 1160.588, 1097.315,  280.277,  922.576,  831.969,
      21.055,  102.942,   54.335, 1110.539, 1396.435,  967.777,
      1061.805, 1014.186,   16.153,  504.027,  959.249, 1505.480,
      6.805,  503.802,  106.785, 1192.662,  515.542, 1192.614,
      118.671,  329.558,  257.808,  603.476, 1145.597,  154.620,
      15.487,  791.959,  257.432,   93.624,  791.866, 1180.050,
      1617.565, 1617.547,   65.389,  135.432,   11.423,  472.986,
      11.479,   93.101, 1450.273, 1195.029,  472.818,  549.377,
      1144.641, 1144.605,   11.081,  235.957, 1003.952,  153.325,
      235.682,  753.035,   80.966,  752.966,  355.896,   17.069,
      1561.552, 1561.538, 1245.719,  172.816,  639.085, 1231.091,
      77.546,   80.603,  443.160,  443.036,  584.354,  163.631,
      1097.627,  183.422, 1097.600,  144.991,  204.358,  172.251,
      127.508,  226.434,  215.227,  111.088, 1297.472, 1505.263,
      249.648,   23.941,   95.937,  215.030,  715.221,  715.169,
      81.863,   69.435,  273.894, 1506.644, 1506.634,   68.972,
      1049.799,   69.192, 1350.496,   57.273,  299.369,  414.445,
      192.214,   90.808,  414.355,  192.101, 1283.089,   46.671,
      1051.819,  383.307, 1051.799,  620.277,   31.836,   37.374,
      325.870,  675.747,  195.713, 1404.696,   29.088,  195.575,
      22.018,  678.514,  678.476,   59.027,  353.495,   16.169,
      1452.941, 1452.933,   58.872,   11.445,  213.216, 1460.080,
      7.949,   40.950,  386.942,  386.877,    7.751,  105.180,
      11.051,  382.142, 1007.216, 1007.201,   15.518,  657.355,
      177.314, 1096.789, 1336.151,   21.051,  177.219,  212.478,
      27.750,  643.014,  642.987,   49.838,  411.909,  411.892,
      51.179,  235.363,   49.745, 1400.341, 1400.336,   35.514,
      713.563,  360.547,   44.344,  360.501,  963.719,  963.708,
      120.565,   54.340,  442.797,  160.127,  160.064,   62.418,
      695.493,   65.400,  608.621,  608.601,   41.764,  258.559,
      41.712, 1390.386, 1348.946, 1348.942,  234.052,   77.525,
      5.597,    5.530, 1144.922,  474.705,  335.360,  335.328,
      74.863,  921.326,  921.318,   90.814,  441.656,  137.165,
      144.150,  144.108,  752.538,  105.168,  575.433,  575.419,
      282.812,   34.901,  734.700,   34.875,  507.736, 1298.555,
      1298.552,    7.920,   88.410,    7.717,  120.587,  311.280,
      311.258,  595.432,  559.319,  632.774,  524.338,  671.138,
      880.138,  880.132,  256.726,  490.396,  129.180,  129.154,
      710.621,  457.700,  154.783,  137.170,  541.791, 1194.200,
      426.156,  102.955,  543.350,   29.246,  543.341,   29.235,
      308.228,  751.220,   11.476, 1249.467, 1249.465,  472.503,
      774.982,  395.673,  154.818,  792.932,  288.405,  288.391,
      11.067,  366.458,  792.675,  840.153,  840.149, 1683.018,
      1683.017,  115.517,  115.501,  118.693,  576.976,  835.656,
      173.532,  338.321,  173.518,   16.165,  280.505,  512.472,
      512.465,  334.615,  311.469, 1201.383, 1201.381,  879.589,
      193.311,  266.636,  266.626,  285.710,  816.346,  135.421,
      15.477,  801.173,  801.170, 1244.723,  613.095,  102.859,
      261.154,  102.850, 1627.035, 1627.035,   22.087,  504.537,
      924.532,  214.257,  193.373,  237.807,  482.698,  482.693,
      362.078, 1154.501, 1154.500,  153.238,  833.878,  215.575,
      305.493,  246.071,  246.065,  970.485,  236.270,  194.565,
      29.142,  763.496,  763.494,  650.353,   21.043,   91.405,
      91.400,  858.800, 1572.155, 1572.154,  174.781,  214.246,
      172.240,  454.027,  156.226,  454.024,  259.352, 1017.550,
      390.624, 1108.823, 1108.822,  138.801,   37.330,  226.609,
      122.509,  226.605, 1296.295,  537.661,  107.447,  726.822,
      726.820,   93.615,  688.658,   81.154,   81.151,   80.911,
      283.503,  331.494, 1518.477, 1518.477,   69.430,  192.228,
      59.070,   27.760,   49.828,   41.799,   34.881,   29.171,
      24.666,   24.662,   29.160,   34.856,  902.249,   41.748,
      49.736, 1065.729,   58.919,   69.194,   80.561,  426.560,
      426.558,  876.352,  236.237,   93.117, 1241.892, 1189.392,
      106.760,   46.749, 1138.109, 1295.406,  121.590, 1064.248,
      1064.247,  137.503,  420.160, 1087.949, 1350.028,  154.598,
      208.251,  208.249,  308.725,  172.773, 1038.919, 1405.755,
      192.026,  213.200,  691.351,  691.350,   72.005,   72.004,
      212.556,  991.025,  728.016, 1466.002, 1466.002, 1114.825,
      57.300, 1462.585,  234.060,  400.296,  400.295,  571.982,
      944.275,  259.344,  256.738,   35.423,  335.018,  358.614,
      1349.018,  946.799, 1020.775, 1020.774,  280.486,  191.095,
      191.094,  450.892,  898.679,  235.356,  305.405,   68.982,
      657.082,  657.082,   63.958,   63.958, 1165.045,  854.243,
      1414.629, 1414.628,  919.900,  331.493,  362.385,  768.436,
      375.134,  375.134,  283.467,  358.549,  810.979,  978.504,
      978.504,  258.597,   81.893,  175.142,  175.141,   44.328,
      386.772,  992.456,  386.854,  482.624,  390.927,  768.895,
      623.916,  623.916,   57.113,   57.112,  607.302, 1216.394,
      416.161, 1364.358, 1364.358,   95.933,   21.233,   21.232,
      809.925,  728.000, 1402.897,  351.075,  351.075,  282.824,
      308.704,  446.615,  937.336,  937.336,  420.445,  160.191,
      160.190,  688.302,  111.100,  515.360,  478.136,  964.527,
      591.853,  591.853, 1039.124,   54.268,  416.120, 1315.189,
      1315.189, 1268.677,  649.711,   24.591,   24.587,  308.139,
      510.721,  328.218,  328.218,  335.054,  451.041,  127.492,
      852.491,  897.270,  897.270,  612.434,  643.828, 1757.373,
      1757.373,  146.541,  146.541,  544.472,  549.304,  560.992,
      560.991,  576.277,  145.008,  334.541, 1267.222, 1267.222,
      29.152,   29.140, 1457.935,  482.717, 1086.906, 1322.004,
      579.288,  306.463,  306.463,  541.346,  362.415,  446.613,
      65.336,  858.406,  858.406,  163.645,  896.041, 1701.406,
      1701.406,  133.893,  133.893,  507.645, 1010.340,  615.170,
      362.033,  531.232,  531.232,  475.077,  515.574,  584.257,
      1220.457, 1220.457,   34.917,  183.400,   34.891,  681.463,
      443.743,  285.910,  285.910,  390.886, 1376.481,  652.118,
      1135.706,  413.645,  820.744,  820.744,  122.546,  122.546,
      1646.642, 1646.642,  478.137,  204.371,  940.682,  384.682,
      390.615,  549.413,   77.528,  502.675,  502.675,  356.952,
      690.234,  620.221, 1174.694, 1174.694,   41.791,  330.355,
      41.738,  226.454,  304.986,  420.466,  266.458,  266.458,
      280.745, 1057.341,  257.727, 1431.917,  784.183,  784.183,
      235.930, 1185.625,  729.318,  112.200,  112.200, 1592.979,
      1592.979, 1485.859,  215.252,  584.337,  420.190,  195.689,
      249.645,  177.339,  720.213,  160.100,  280.275,  257.375,
      235.671,  304.371,  144.069,  215.064,  329.560,  195.556,
      177.247,  129.246,  355.841,  160.038,  115.429,  144.029,
      129.221,  102.917,  115.414,  383.313,  102.908,   91.408,
      91.403,   81.102,   81.100,   71.998,   71.997,   63.995,
      63.995,   57.094,   57.094,   51.393,   51.393,  510.691,
      411.873,  441.620,  986.420,  475.219,  475.219,  472.552,
      49.875,  504.567, 1130.132, 1130.132,  657.297,   49.780,
      537.663, 1428.658,  769.572,  451.054,  273.941,  571.938,
      248.208,  248.208,  607.291,   90.737,  620.347,  748.725,
      748.725,  643.819,  450.857,  103.054,  103.054, 1540.517,
      1540.517, 1372.643,  681.521, 1236.564,  299.337,  810.895,
      720.195,  448.965,  448.965,   59.072,  544.477, 1086.772,
      1086.772, 1033.160,  760.140,   58.914,  695.486,  482.748,
      1317.823,  230.959,  230.959, 1105.338,  325.828,  760.083,
      801.054,  657.344,  482.618,  714.368,  714.368,   95.110,
      95.110,  853.291, 1489.157, 1489.157,  843.136, 1264.207,
      353.412, 1288.624,  423.812,  423.812,   69.388,  886.386,
      105.159, 1044.513, 1044.513,  515.547,   69.139,  734.687,
      695.530,  215.011,  215.011, 1081.008, 1211.804,  930.701,
      579.295,  896.760,  382.182,  515.374,  681.312,  681.312,
      1438.898, 1438.898,  976.083, 1160.620,   80.926,  801.076,
      399.761,  399.761,  411.936, 1022.530, 1341.705,  941.304,
      1003.356, 1003.356,  549.352, 1154.535,  734.706,   80.552,
      775.001,  200.164,  200.164, 1110.561,  549.226, 1129.867,
      615.144, 1070.142,  442.770,  649.258,  649.258, 1389.841,
      1389.841, 1061.732,   93.591,  120.588,  986.923,   46.818,
      46.818, 1118.818,  376.910,  376.910,  774.972,  584.261,
      963.400,  963.400,  474.682, 1840.565, 1840.565, 1014.137,
      584.174,   93.051,  816.325,  186.418,  186.418, 1168.560,
      843.199,  618.405,  618.405,  967.779, 1179.839,  652.123,
      1033.621,  107.485, 1341.885, 1341.885,  507.768,   51.374,
      51.374, 1219.468,  355.161,  355.161,  922.558, 1204.937,
      816.331,  620.273,  924.545,  924.545,  620.219, 1784.609,
      1784.609,  878.576,  173.872,  173.872,  858.761,  106.732,
      541.828, 1081.298, 1271.341,  122.515,  588.653,  588.653,
      835.731,  137.121, 1295.129, 1295.129,  690.231, 1230.827,
      57.131,   57.131,  794.123,  858.783,  334.613,  334.613,
      657.389,  886.353, 1324.381,  657.262,  576.960,  753.749,
      886.792,  886.792, 1729.855, 1729.855, 1130.156,  162.427,
      162.427,  714.507,  902.206,  138.782,  121.592,  676.494,
      560.102,  560.102,  639.609, 1378.488, 1249.375, 1249.375,
      613.165, 1256.351,  603.948,  902.230,   63.988,   63.988,
      729.365, 1282.931,  569.509,  695.507,  315.165,  315.165,
      695.403,  536.088,  833.938,  876.338,  792.628,  919.826,
      752.511,  964.500, 1180.097,  156.189,  713.586,  503.985,
      1010.360,  675.657,  850.239,  850.239,  639.024, 1057.302,
      1676.301, 1676.301,  472.997,  603.488,  569.051, 1105.325,
      443.121,  535.814,  152.082,  152.082,  503.776, 1433.664,
      946.760,  414.457,  472.840, 1154.527,  443.005,  386.902,
      414.372,  360.556,  386.841,  335.317,  360.512, 1204.907,
      335.286,  311.284,  311.263,  288.357,  154.755,  930.644,
      650.343,  288.343,  266.634,  137.527,  266.625,  246.015,
      246.009,  226.599,  226.596,  208.287,  208.285,  191.077,
      191.075,  175.069,  175.068,  160.262,  160.262, 1256.361,
      146.457,  146.457,  133.954,  133.954,  532.652,  532.652,
      122.451,  122.451,  112.249,  112.249,  103.048,  103.048,
      95.047,   95.047,   88.247,   88.247, 1204.922, 1204.922,
      1308.890,  946.771,   71.946,   71.945,  174.740,  734.728,
      1362.591,  734.642,  769.623, 1231.022,  296.818,  296.818,
      1336.052, 1417.362,  814.788,  814.788, 1623.849, 1623.849,
      688.695,  142.938,  142.938,  992.422, 1473.302,  194.634,
      506.403,  506.403, 1308.881,  154.532,  992.408, 1161.470,
      1161.470,  774.950, 1283.032,   81.105,   81.103,  774.980,
      976.073,  279.672,  279.672,  728.022,  810.903, 1390.190,
      780.537,  780.537, 1572.497, 1572.497,  215.574, 1039.091,
      173.490,  481.255,  481.255, 1039.142, 1336.129, 1119.219,
      1119.219,  816.374,  768.427,  816.317,  172.702,   91.466,
      91.461,  237.757,  263.726,  263.726,  853.304,  747.387,
      747.387, 1522.246, 1522.246, 1086.867, 1022.543, 1362.632,
      261.084, 1086.873, 1390.315,  809.910,  457.207,  457.207,
      3.431,  858.800, 1078.168, 1078.168,  858.753,  102.830,
      102.821,  248.881,  248.881,  285.653,  192.033,  896.722,
      715.438,  715.438, 1473.196, 1473.196, 1135.649,  852.475,
      1135.701,  311.461,  193.323,  434.361,  434.361,  902.227,
      902.188,    6.783, 1932.369, 1932.369, 1038.119, 1038.119,
      1070.154,  115.496,  115.481,  235.137,  235.137, 1516.609,
      338.307,  941.257, 1417.411,  684.590,  684.590, 1425.247,
      1425.247, 1185.535,  896.022,  212.519, 1460.102, 1185.628,
      366.388,  946.755,   82.528,   82.528,  946.823,  412.614,
      412.614, 1404.634,   11.329, 1876.521, 1876.521,  999.370,
      999.370,  129.167,  129.141,  395.701, 1350.501,  222.592,
      222.592,  940.654, 1118.808,  986.906,  654.843,  654.843,
      1236.527, 1297.501, 1378.498, 1378.498,  426.142, 1236.552,
      1245.632,  992.384,  992.458,  234.053,   88.184,   88.184,
      392.069,  392.069,  457.709, 1194.991, 1821.773, 1821.773,
      17.067,  214.256,  961.621,  961.621,  144.044,  144.002,
      1145.576,  986.372,  211.148,  211.148,  490.397, 1097.285,
      1514.071, 1457.831, 1402.882, 1033.567, 1349.026, 1288.522,
      1050.215,  626.296,  626.296, 1296.265, 1332.850, 1332.850,
      1244.700, 1288.576, 1004.263, 1194.231,  524.304, 1144.861,
      959.428, 1039.114, 1168.604, 1039.093, 1096.791, 1049.720,
      915.808, 1003.851,   95.041,   95.041,  372.623,  372.623,
      873.402,  959.183};

    static const int ifin1[500]={
      5,   8,   9,  12,  15,  17,  19,  21,  24,  27,
        27,  30,  33,  36,  40,  45,  45,  48,  51,  52,
        56,  58,  61,  63,  67,  68,  70,  77,  79,  82,
        86,  86,  87,  91,  91,  91,  94,  97,  99, 103,
       106, 108, 110, 111, 112, 115, 118, 120, 122, 122,
       127, 130, 132, 136, 137, 142, 144, 146, 149, 151,
       153, 153, 155, 156, 162, 162, 164, 169, 171, 172,
       175, 177, 180, 184, 184, 185, 189, 192, 196, 197,
       197, 201, 202, 208, 210, 212, 215, 216, 217, 223,
       225, 227, 228, 230, 232, 235, 237, 238, 239, 242,
       244, 247, 249, 250, 251, 252, 254, 255, 258, 263,
       264, 267, 268, 269, 273, 277, 280, 283, 287, 287,
       291, 293, 295, 297, 300, 304, 307, 311, 314, 316,
       319, 319, 321, 324, 327, 330, 332, 334, 336, 340,
       342, 343, 348, 349, 350, 353, 360, 361, 362, 363,
       365, 365, 368, 370, 371, 374, 375, 377, 381, 383,
       386, 389, 391, 391, 395, 396, 398, 399, 400, 402,
       408, 410, 411, 412, 416, 419, 420, 425, 426, 431,
       432, 433, 441, 442, 445, 448, 450, 452, 453, 456,
       459, 465, 466, 467, 470, 472, 474, 475, 476, 480,
       482, 484, 487, 492, 494, 496, 499, 500, 502, 503,
       506, 508, 510, 512, 517, 521, 522, 522, 527, 528,
       529, 533, 533, 539, 540, 540, 546, 551, 555, 561,
       569, 577, 582, 585, 589, 592, 594, 597, 601, 606,
       607, 610, 610, 618, 619, 623, 624, 629, 629, 631,
       635, 635, 642, 643, 643, 646, 648, 652, 652, 655,
       659, 661, 666, 666, 670, 673, 674, 679, 681, 683,
       684, 686, 687, 690, 693, 695, 699, 702, 703, 706,
       710, 712, 716, 718, 718, 719, 722, 724, 727, 733,
       733, 739, 740, 740, 744, 750, 750, 751, 754, 757,
       759, 763, 763, 768, 770, 770, 773, 779, 781, 783,
       784, 788, 792, 794, 795, 799, 800, 803, 808, 815,
       819, 835, 854, 859, 861, 867, 869, 873, 874, 875,
       879, 884, 886, 888, 890, 892, 893, 900, 902, 906,
       909, 910, 914, 918, 918, 919, 921, 925, 926, 930,
       931, 939, 941, 941, 943, 946, 946, 947, 949, 952,
       955, 960, 962, 966, 966, 969, 972, 974, 975, 980,
       981, 983, 985, 991, 993, 996, 997, 998,1001,1007,
      1008,1010,1011,1017,1017,1022,1022,1027,1029,1031,
      1035,1037,1039,1042,1045,1047,1051,1055,1058,1061,
      1062,1063,1065,1069,1071,1076,1081,1083,1093,1103,
      1110,1117,1125,1140,1163,1165,1167,1171,1177,1178,
      1179,1183,1187,1188,1188,1189,1192,1196,1197,1201,
      1204,1206,1207,1212,1213,1213,1214,1214,1217,1220,
      1224,1227,1229,1230,1230,1234,1235,1237,1238,1239,
      1244,1248,1248,1250,1253,1254,1255,1259,1260,1261,
      1262,1264,1269,1273,1275,1276,1280,1280,1282,1289,
      1289,1291,1292,1296,1300,1304,1306,1307,1310,1311,
      1313,1319,1320,1321,1323,1329,1333,1337,1339,1340,
      1344,1347,1354,1358,1362,1365,1369,1375,1376,1376};

    static const int ini1[500]={
      1,   1,   1,   3,   5,   8,   9,  12,  15,  17,
      19,  21,  24,  27,  27,  30,  33,  36,  40,  45,
      45,  48,  51,  52,  56,  58,  61,  63,  67,  68,
      70,  77,  79,  82,  86,  86,  87,  91,  91,  91,
      94,  97,  99, 103, 106, 108, 110, 111, 112, 115,
      118, 120, 122, 122, 127, 130, 132, 136, 137, 142,
      144, 146, 149, 151, 153, 153, 155, 156, 162, 162,
      164, 169, 171, 172, 175, 177, 180, 184, 184, 185,
      189, 192, 196, 197, 197, 201, 202, 208, 210, 212,
      215, 216, 217, 223, 225, 227, 228, 230, 232, 235,
      237, 238, 239, 242, 244, 247, 249, 250, 251, 252,
      254, 255, 258, 263, 264, 267, 268, 269, 273, 277,
      280, 283, 287, 287, 291, 293, 295, 297, 300, 304,
      307, 311, 314, 316, 319, 319, 321, 324, 327, 330,
      332, 334, 336, 340, 342, 343, 348, 349, 350, 353,
      360, 361, 362, 363, 365, 365, 368, 370, 371, 374,
      375, 377, 381, 383, 386, 389, 391, 391, 395, 396,
      398, 399, 400, 402, 408, 410, 411, 412, 416, 419,
      420, 425, 426, 431, 432, 433, 441, 442, 445, 448,
      450, 452, 453, 456, 459, 465, 466, 467, 470, 472,
      474, 475, 476, 480, 482, 484, 487, 492, 494, 496,
      499, 500, 502, 503, 506, 508, 510, 512, 517, 521,
      522, 522, 527, 528, 529, 533, 533, 539, 540, 540,
      546, 551, 555, 561, 569, 577, 582, 585, 589, 592,
      594, 597, 601, 606, 607, 610, 610, 618, 619, 623,
      624, 629, 629, 631, 635, 635, 642, 643, 643, 646,
      648, 652, 652, 655, 659, 661, 666, 666, 670, 673,
      674, 679, 681, 683, 684, 686, 687, 690, 693, 695,
      699, 702, 703, 706, 710, 712, 716, 718, 718, 719,
      722, 724, 727, 733, 733, 739, 740, 740, 744, 750,
      750, 751, 754, 757, 759, 763, 763, 768, 770, 770,
      773, 779, 781, 783, 784, 788, 792, 794, 795, 799,
      800, 803, 808, 815, 819, 835, 854, 859, 861, 867,
      869, 873, 874, 875, 879, 884, 886, 888, 890, 892,
      893, 900, 902, 906, 909, 910, 914, 918, 918, 919,
      921, 925, 926, 930, 931, 939, 941, 941, 943, 946,
      946, 947, 949, 952, 955, 960, 962, 966, 966, 969,
      972, 974, 975, 980, 981, 983, 985, 991, 993, 996,
      997, 998,1001,1007,1008,1010,1011,1017,1017,1022,
      1022,1027,1029,1031,1035,1037,1039,1042,1045,1047,
      1051,1055,1058,1061,1062,1063,1065,1069,1071,1076,
      1081,1083,1093,1103,1110,1117,1125,1140,1163,1165,
      1167,1171,1177,1178,1179,1183,1187,1188,1188,1189,
      1192,1196,1197,1201,1204,1206,1207,1212,1213,1213,
      1214,1214,1217,1220,1224,1227,1229,1230,1230,1234,
      1235,1237,1238,1239,1244,1248,1248,1250,1253,1254,
      1255,1259,1260,1261,1262,1264,1269,1273,1275,1276,
      1280,1280,1282,1289,1289,1291,1292,1296,1300,1304,
      1306,1307,1310,1311,1313,1319,1320,1321,1323,1329,
      1333,1337,1339,1340,1344,1347,1354,1358,1362,1365};

    static const int ifin2[500]={
      3,   5,   8,   9,  12,  15,  16,  19,  21,  24,
      26,  27,  29,  32,  35,  39,  44,  45,  48,  50,
      52,  54,  57,  61,  63,  66,  68,  70,  74,  78,
      82,  84,  86,  86,  91,  91,  91,  94,  95,  99,
      102, 106, 107, 110, 110, 112, 114, 116, 118, 122,
      122, 125, 128, 132, 135, 136, 140, 144, 145, 148,
      151, 153, 153, 155, 155, 161, 162, 163, 168, 170,
      172, 175, 177, 179, 184, 184, 184, 189, 191, 196,
      196, 197, 199, 202, 208, 210, 211, 215, 216, 217,
      222, 225, 227, 228, 230, 231, 234, 236, 238, 239,
      242, 243, 246, 249, 249, 251, 252, 253, 255, 257,
      263, 264, 266, 268, 269, 270, 277, 278, 283, 283,
      287, 290, 292, 295, 297, 299, 303, 306, 310, 312,
      316, 317, 319, 321, 323, 327, 330, 332, 334, 336,
      339, 341, 343, 347, 349, 349, 352, 357, 361, 361,
      362, 365, 365, 367, 370, 371, 373, 375, 376, 380,
      383, 385, 388, 391, 391, 392, 396, 398, 399, 399,
      401, 407, 408, 411, 412, 415, 419, 420, 425, 425,
      429, 431, 433, 441, 442, 444, 448, 449, 452, 453,
      456, 456, 463, 465, 467, 470, 472, 474, 475, 476,
      479, 482, 482, 486, 492, 494, 495, 498, 499, 500,
      503, 505, 508, 510, 512, 516, 521, 522, 522, 523,
      528, 529, 533, 533, 539, 539, 540, 543, 551, 553,
      557, 569, 575, 581, 585, 588, 592, 594, 596, 599,
      603, 606, 610, 610, 615, 619, 623, 624, 629, 629,
      631, 633, 635, 642, 643, 643, 645, 647, 651, 652,
      655, 659, 660, 663, 666, 670, 673, 673, 678, 680,
      681, 683, 686, 687, 689, 691, 695, 699, 702, 703,
      706, 710, 710, 714, 718, 718, 719, 719, 723, 727,
      730, 733, 737, 740, 740, 744, 749, 750, 751, 751,
      756, 757, 762, 763, 768, 770, 770, 773, 778, 779,
      783, 784, 788, 789, 794, 795, 799, 800, 801, 805,
      814, 818, 830, 854, 858, 861, 866, 868, 873, 874,
      875, 879, 884, 886, 888, 888, 892, 893, 899, 902,
      906, 909, 909, 912, 918, 918, 919, 920, 925, 926,
      928, 931, 938, 941, 941, 943, 946, 946, 947, 948,
      952, 955, 960, 961, 966, 966, 969, 972, 974, 975,
      977, 980, 983, 985, 990, 992, 996, 996, 998,1001,
      1007,1007,1008,1010,1016,1017,1022,1022,1027,1028,
      1031,1035,1037,1039,1041,1043,1047,1051,1055,1056,
      1061,1062,1063,1065,1069,1069,1075,1079,1083,1091,
      1102,1107,1117,1123,1136,1163,1165,1167,1171,1173,
      1178,1179,1183,1184,1188,1188,1188,1192,1196,1196,
      1201,1201,1206,1207,1211,1213,1213,1213,1214,1217,
      1220,1223,1227,1229,1229,1230,1234,1235,1236,1238,
      1239,1243,1247,1248,1250,1252,1254,1255,1259,1260,
      1260,1262,1264,1267,1273,1274,1276,1278,1280,1281,
      1287,1289,1290,1292,1293,1298,1304,1305,1307,1310,
      1310,1313,1319,1320,1321,1322,1325,1330,1337,1338,
      1340,1343,1347,1351,1357,1361,1363,1368,1373,1376};

    static const int ini2[500]={   
      1,   3,   4,   6,   9,  10,  12,  15,  18,  20,
      23,  25,  27,  27,  30,  34,  36,  40,  45,  45,
      49,  52,  53,  56,  60,  61,  64,  68,  68,  73,
      77,  81,  84,  86,  86,  88,  91,  91,  92,  94,
      97, 101, 103, 107, 108, 110, 111, 113, 115, 118,
      120, 122, 123, 127, 131, 132, 136, 138, 142, 144,
      147, 149, 152, 153, 155, 155, 157, 162, 162, 166,
      169, 171, 172, 176, 177, 181, 184, 184, 185, 190,
      195, 196, 197, 198, 201, 205, 208, 211, 213, 215,
      217, 220, 223, 226, 228, 229, 230, 233, 235, 237,
      239, 240, 242, 244, 248, 249, 251, 252, 252, 254,
      257, 259, 264, 265, 267, 268, 269, 275, 278, 280,
      283, 287, 288, 291, 294, 295, 297, 301, 304, 307,
      312, 315, 316, 319, 319, 322, 325, 328, 331, 332,
      335, 337, 341, 342, 344, 348, 349, 351, 354, 361,
      361, 362, 364, 365, 365, 369, 370, 372, 374, 375,
      379, 381, 383, 386, 389, 391, 391, 395, 396, 399,
      399, 400, 405, 408, 411, 411, 413, 417, 420, 423,
      425, 427, 431, 432, 436, 442, 444, 447, 448, 451,
      452, 454, 456, 461, 465, 466, 468, 471, 472, 474,
      476, 478, 481, 482, 484, 491, 493, 495, 496, 499,
      500, 502, 505, 508, 510, 510, 514, 519, 521, 522,
      522, 527, 528, 533, 533, 535, 539, 540, 541, 546,
      552, 555, 565, 573, 579, 582, 586, 589, 593, 594,
      597, 601, 606, 609, 610, 612, 619, 621, 623, 624,
      629, 629, 631, 635, 639, 643, 643, 643, 647, 648,
      652, 652, 657, 659, 661, 666, 667, 670, 673, 676,
      680, 681, 683, 684, 687, 687, 691, 694, 696, 699,
      703, 703, 708, 710, 712, 716, 718, 719, 719, 723,
      724, 727, 733, 734, 739, 740, 742, 748, 750, 750,
      751, 754, 757, 760, 763, 764, 768, 770, 773, 777,
      779, 781, 783, 786, 788, 792, 794, 796, 799, 800,
      805, 810, 815, 821, 844, 858, 859, 863, 867, 871,
      873, 875, 876, 882, 885, 887, 888, 892, 893, 897,
      900, 902, 907, 909, 910, 915, 918, 919, 920, 923,
      925, 928, 930, 932, 939, 941, 941, 943, 946, 946,
      947, 951, 952, 958, 960, 965, 966, 966, 969, 972,
      974, 977, 980, 983, 983, 987, 991, 996, 996, 997,
      998,1003,1007,1008,1010,1014,1017,1017,1022,1026,
      1027,1029,1031,1035,1037,1039,1043,1046,1047,1051,
      1055,1060,1061,1062,1063,1065,1069,1073,1077,1081,
      1085,1095,1104,1113,1119,1127,1147,1163,1167,1167,
      1172,1177,1178,1179,1184,1188,1188,1188,1189,1193,
      1196,1198,1201,1204,1206,1209,1212,1213,1213,1214,
      1216,1217,1221,1224,1227,1229,1230,1232,1234,1235,
      1237,1238,1243,1244,1248,1250,1250,1254,1254,1257,
      1260,1260,1261,1263,1266,1271,1273,1276,1276,1280,
      1280,1284,1289,1290,1291,1292,1298,1302,1304,1306,
      1307,1310,1311,1316,1320,1321,1321,1325,1329,1335,
      1337,1340,1341,1346,1348,1354,1359,1362,1368,1370};

    static const int ifin3[500]={
      3,   4,   6,   9,  10,  12,  15,  18,  20,  23,
      25,  27,  27,  30,  34,  36,  40,  45,  45,  49,
      52,  53,  56,  60,  61,  64,  68,  68,  73,  77,
      81,  84,  86,  86,  88,  91,  91,  92,  94,  97,
      101, 103, 107, 108, 110, 111, 113, 115, 118, 120,
      122, 123, 127, 131, 132, 136, 138, 142, 144, 147,
      149, 152, 153, 155, 155, 157, 162, 162, 166, 169,
      171, 172, 176, 177, 181, 184, 184, 185, 190, 195,
      196, 197, 198, 201, 205, 208, 211, 213, 215, 217,
      220, 223, 226, 228, 229, 230, 233, 235, 237, 239,
      240, 242, 244, 248, 249, 251, 252, 252, 254, 257,
      259, 264, 265, 267, 268, 269, 275, 278, 280, 283,
      287, 288, 291, 294, 295, 297, 301, 304, 307, 312,
      315, 316, 319, 319, 322, 325, 328, 331, 332, 335,
      337, 341, 342, 344, 348, 349, 351, 354, 361, 361,
      362, 364, 365, 365, 369, 370, 372, 374, 375, 379,
      381, 383, 386, 389, 391, 391, 395, 396, 399, 399,
      400, 405, 408, 411, 411, 413, 417, 420, 423, 425,
      427, 431, 432, 436, 442, 444, 447, 448, 451, 452,
      454, 456, 461, 465, 466, 468, 471, 472, 474, 476,
      478, 481, 482, 484, 491, 493, 495, 496, 499, 500,
      502, 505, 508, 510, 510, 514, 519, 521, 522, 522,
      527, 528, 533, 533, 535, 539, 540, 541, 546, 552,
      555, 565, 573, 579, 582, 586, 589, 593, 594, 597,
      601, 606, 609, 610, 612, 619, 621, 623, 624, 629,
      629, 631, 635, 639, 643, 643, 643, 647, 648, 652,
      652, 657, 659, 661, 666, 667, 670, 673, 676, 680,
      681, 683, 684, 687, 687, 691, 694, 696, 699, 703,
      703, 708, 710, 712, 716, 718, 719, 719, 723, 724,
      727, 733, 734, 739, 740, 742, 748, 750, 750, 751,
      754, 757, 760, 763, 764, 768, 770, 773, 777, 779,
      781, 783, 786, 788, 792, 794, 796, 799, 800, 805,
      810, 815, 821, 844, 858, 859, 863, 867, 871, 873,
      875, 876, 882, 885, 887, 888, 892, 893, 897, 900,
      902, 907, 909, 910, 915, 918, 919, 920, 923, 925,
      928, 930, 932, 939, 941, 941, 943, 946, 946, 947,
      951, 952, 958, 960, 965, 966, 966, 969, 972, 974,
      977, 980, 983, 983, 987, 991, 996, 996, 997, 998,
      1003,1007,1008,1010,1014,1017,1017,1022,1026,1027,
      1029,1031,1035,1037,1039,1043,1046,1047,1051,1055,
      1060,1061,1062,1063,1065,1069,1073,1077,1081,1085,
      1095,1104,1113,1119,1127,1147,1163,1167,1167,1172,
      1177,1178,1179,1184,1188,1188,1188,1189,1193,1196,
      1198,1201,1204,1206,1209,1212,1213,1213,1214,1216,
      1217,1221,1224,1227,1229,1230,1232,1234,1235,1237,
      1238,1243,1244,1248,1250,1250,1254,1254,1257,1260,
      1260,1261,1263,1266,1271,1273,1276,1276,1280,1280,
      1284,1289,1290,1291,1292,1298,1302,1304,1306,1307,
      1310,1311,1316,1320,1321,1321,1325,1329,1335,1337,
      1340,1341,1346,1348,1354,1359,1362,1368,1370,1376};

    static const int ini3[500]={
      1,   3,   5,   8,   9,  12,  15,  16,  19,  21,
      24,  26,  27,  29,  32,  35,  39,  44,  45,  48,
      50,  52,  54,  57,  61,  63,  66,  68,  70,  74,
      78,  82,  84,  86,  86,  91,  91,  91,  94,  95,
      99, 102, 106, 107, 110, 110, 112, 114, 116, 118,
      122, 122, 125, 128, 132, 135, 136, 140, 144, 145,
      148, 151, 153, 153, 155, 155, 161, 162, 163, 168,
      170, 172, 175, 177, 179, 184, 184, 184, 189, 191,
      196, 196, 197, 199, 202, 208, 210, 211, 215, 216,
      217, 222, 225, 227, 228, 230, 231, 234, 236, 238,
      239, 242, 243, 246, 249, 249, 251, 252, 253, 255,
      257, 263, 264, 266, 268, 269, 270, 277, 278, 283,
      283, 287, 290, 292, 295, 297, 299, 303, 306, 310,
      312, 316, 317, 319, 321, 323, 327, 330, 332, 334,
      336, 339, 341, 343, 347, 349, 349, 352, 357, 361,
      361, 362, 365, 365, 367, 370, 371, 373, 375, 376,
      380, 383, 385, 388, 391, 391, 392, 396, 398, 399,
      399, 401, 407, 408, 411, 412, 415, 419, 420, 425,
      425, 429, 431, 433, 441, 442, 444, 448, 449, 452,
      453, 456, 456, 463, 465, 467, 470, 472, 474, 475,
      476, 479, 482, 482, 486, 492, 494, 495, 498, 499,
      500, 503, 505, 508, 510, 512, 516, 521, 522, 522,
      523, 528, 529, 533, 533, 539, 539, 540, 543, 551,
      553, 557, 569, 575, 581, 585, 588, 592, 594, 596,
      599, 603, 606, 610, 610, 615, 619, 623, 624, 629,
      629, 631, 633, 635, 642, 643, 643, 645, 647, 651,
      652, 655, 659, 660, 663, 666, 670, 673, 673, 678,
      680, 681, 683, 686, 687, 689, 691, 695, 699, 702,
      703, 706, 710, 710, 714, 718, 718, 719, 719, 723,
      727, 730, 733, 737, 740, 740, 744, 749, 750, 751,
      751, 756, 757, 762, 763, 768, 770, 770, 773, 778,
      779, 783, 784, 788, 789, 794, 795, 799, 800, 801,
      805, 814, 818, 830, 854, 858, 861, 866, 868, 873,
      874, 875, 879, 884, 886, 888, 888, 892, 893, 899,
      902, 906, 909, 909, 912, 918, 918, 919, 920, 925,
      926, 928, 931, 938, 941, 941, 943, 946, 946, 947,
      948, 952, 955, 960, 961, 966, 966, 969, 972, 974,
      975, 977, 980, 983, 985, 990, 992, 996, 996, 998,
      1001,1007,1007,1008,1010,1016,1017,1022,1022,1027,
      1028,1031,1035,1037,1039,1041,1043,1047,1051,1055,
      1056,1061,1062,1063,1065,1069,1069,1075,1079,1083,
      1091,1102,1107,1117,1123,1136,1163,1165,1167,1171,
      1173,1178,1179,1183,1184,1188,1188,1188,1192,1196,
      1196,1201,1201,1206,1207,1211,1213,1213,1213,1214,
      1217,1220,1223,1227,1229,1229,1230,1234,1235,1236,
      1238,1239,1243,1247,1248,1250,1252,1254,1255,1259,
      1260,1260,1262,1264,1267,1273,1274,1276,1278,1280,
      1281,1287,1289,1290,1292,1293,1298,1304,1305,1307,
      1310,1310,1313,1319,1320,1321,1322,1325,1330,1337,
      1338,1340,1343,1347,1351,1357,1361,1363,1368,1373};

    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.53;  //Debyes
    static const double mmol=50.0;

    double q=1.387429043*pow(tt.get("K"),1.5);
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
            
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
        
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      if(ifin==0||ifin<ini){

	return complex<double> (0.0,0.0);

      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){

	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening(Frequency(fre[i],"GHz"),tt,pp,mmol,Frequency(2.5,"MHz"),0.76),Frequency(0.0,"GHz"));

	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*fre[i]; 

          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(0.047992745509/tt.get("K"))*(fac2fixed*pow(mu,2.0)/q);  // imaginary part: absorption coefficient in cm^2
	                                                                                                  // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )

	
      }
     
    }

  }


  complex<double>  RefractiveIndexProfile::mkAbs_o3_161617(Temperature tt, Pressure pp, Frequency nu){

    static const double fre[1363]={
      .16176,  1.89024,  1.91956,  3.04142,  3.19196,  3.89097,
      3.98601,  4.78954,  7.53146,  8.07908,  8.66614,  8.89708,
      10.84322, 13.23648, 14.05621, 16.41521, 17.09709, 17.79825,
      17.98169, 18.90229, 20.63486, 21.93736, 21.94474, 22.58606,
      24.20730, 24.28898, 25.74250, 26.06893, 26.49450, 27.87676,
      28.04065, 29.18037, 29.37095, 29.85546, 29.91941, 30.31878,
      31.25160, 31.27915, 31.86630, 31.86800, 32.58596, 33.34581,
      34.13127, 34.21584, 35.00331, 35.70629, 37.96981, 38.90970,
      39.16398, 40.61023, 41.05211, 41.33156, 41.43973, 41.95740,
      43.63227, 44.72885, 45.46450, 46.08818, 47.37485, 47.90563,
      49.63892, 49.99009, 50.74731, 51.22195, 53.10312, 54.33059,
      55.42888, 55.62713, 55.83772, 56.11096, 56.68505, 57.37603,
      58.24194, 58.66677, 60.78366, 61.71863, 62.32849, 62.64629,
      63.25550, 64.24054, 64.47763, 64.81511, 65.69582, 66.07205,
      66.43960, 66.99494, 67.03879, 67.36079, 67.91227, 69.00303,
      70.63709, 72.25570, 73.87043, 76.01097, 77.40594, 79.36521,
      79.36784, 79.85448, 80.19440, 81.25876, 81.35755, 82.69506,
      82.98017, 87.93170, 88.15081, 88.66292, 89.24746, 91.23571,
      91.31394, 91.71814, 92.16217, 93.17298, 93.97666, 94.51998,
      95.43642, 96.30901, 97.65726, 99.81734, 99.95576,100.22626,
      100.65495,100.67633,102.37038,102.52416,103.34147,104.54263,
      105.02453,106.70192,107.04454,107.91061,107.92510,108.14973,
      109.19686,109.31612,109.36213,110.25739,111.72639,112.63072,
      115.06657,115.87951,116.99311,117.41811,118.00931,119.55885,
      120.11865,121.65420,121.87147,124.75045,125.36546,125.72860,
      128.84477,128.95417,129.81329,130.19322,130.31844,131.89264,
      131.90153,131.92750,132.26762,133.55542,133.67831,135.32990,
      135.46064,135.79457,136.35156,137.09759,138.97576,139.68504,
      140.00953,140.38775,142.72788,143.12219,146.40438,146.68932,
      148.12885,148.21691,148.48269,148.87717,149.43930,150.05814,
      151.59005,152.92607,154.14075,154.16195,156.79552,160.07718,
      160.11742,160.41422,160.94716,161.27583,161.37581,162.31051,
      162.56888,162.93489,163.22775,165.52922,167.21981,167.29719,
      168.06224,168.57621,170.24416,170.39093,171.00722,171.49618,
      171.97143,172.09340,172.88506,173.16004,174.54333,174.68908,
      175.04178,175.06221,175.29462,175.69270,176.84444,176.87418,
      177.39072,177.56918,177.67091,181.31447,183.91704,184.52283,
      186.38087,187.07582,187.33263,188.15759,189.19894,189.28070,
      189.29330,190.90857,193.39711,197.08258,197.19944,197.45271,
      199.98650,201.36323,201.39318,201.99706,202.62725,202.83385,
      203.12245,204.86297,205.38126,205.45719,205.50351,205.60283,
      205.84344,208.01709,209.08738,210.67028,211.05997,212.48532,
      213.19967,215.20366,215.68556,218.71310,221.99112,222.86677,
      223.04200,223.20099,223.32035,223.38631,223.42964,225.19501,
      227.47478,228.77953,229.09702,233.73348,233.93689,234.30381,
      234.46646,234.51826,235.77737,235.98260,236.32325,238.34927,
      238.44538,238.97727,239.45352,239.56178,239.85730,240.79738,
      241.41987,241.64133,241.79123,242.33475,243.99501,244.00067,
      244.97250,245.03958,245.27379,247.14227,248.89362,249.04875,
      249.10153,249.74476,250.01357,250.04687,253.28696,253.40055,
      254.02916,254.22242,254.35832,255.17235,257.59739,257.66496,
      258.43014,261.83090,262.83914,262.85415,264.26675,264.27393,
      264.66665,265.84959,266.91752,267.21793,267.56303,267.65017,
      269.23139,269.52956,270.37601,271.07046,272.76284,274.64475,
      274.79896,275.37046,275.45873,277.54437,277.68432,277.85512,
      278.39761,279.16153,279.34381,279.85697,280.91669,281.85607,
      283.15222,283.15984,283.76367,284.03156,284.62315,286.85382,
      286.93824,287.61323,289.96746,289.98230,290.26975,290.51376,
      290.58093,291.17168,293.06050,293.29703,294.41403,294.96520,
      295.16722,300.09709,300.30203,300.41556,301.74269,304.77596,
      304.91185,305.10790,305.27512,305.98278,306.38628,306.53092,
      307.57362,307.87992,312.62825,315.58057,315.78902,318.22112,
      318.39250,318.76172,319.18861,319.39207,319.53757,320.03883,
      320.11224,320.29505,321.04720,323.49029,325.87147,325.95442,
      327.88894,328.21886,329.34852,330.25973,330.61362,330.85485,
      331.55940,331.63024,332.59826,336.12647,337.17122,338.36792,
      338.89450,339.88513,341.07766,341.22739,343.12507,343.71061,
      344.07003,344.59908,346.85358,346.89696,350.90565,351.35858,
      351.41871,354.18407,355.31773,355.38116,355.62668,355.87430,
      356.02410,356.27175,356.43622,356.60752,356.69086,357.29655,
      357.39492,357.85852,358.30858,358.40360,358.57268,361.18488,
      361.20807,362.14550,362.32775,364.43802,366.31170,366.48162,
      366.58806,366.83697,367.84327,368.09421,368.16027,368.66138,
      369.24408,369.59760,372.48054,372.59775,373.19803,374.13241,
      374.61443,374.92278,375.87011,376.76844,376.81167,378.89869,
      379.49898,380.68979,380.88895,381.05980,381.60412,381.96781,
      382.05193,382.10532,385.50594,385.90626,386.56727,386.58408,
      391.79965,391.87451,393.31467,393.39531,393.47054,394.30286,
      394.53386,394.76757,396.92902,397.09981,397.65269,400.93655,
      401.00741,402.08059,402.10574,402.13654,406.05722,406.17332,
      406.40034,406.57240,406.60825,406.99085,408.25199,408.59081,
      409.19902,409.77847,410.66535,411.88776,411.89985,412.09321,
      415.10912,415.61759,416.10765,417.03829,417.09034,419.26265,
      419.40988,421.23466,423.33345,426.02584,426.19572,427.37484,
      427.39659,427.47560,427.82570,430.13527,431.06894,431.10474,
      431.14091,431.21672,431.21853,434.88480,435.75346,436.18271,
      436.40464,437.14946,437.15810,437.68166,438.42121,442.06049,
      442.20350,442.23927,443.54124,443.66169,444.13745,444.22872,
      447.05077,449.84129,450.35850,451.19291,451.35261,451.81262,
      452.57981,452.59503,453.54618,454.97074,455.18296,455.73815,
      455.79415,456.14843,456.19941,457.93444,458.29463,458.85726,
      460.29309,461.80680,462.11092,462.35534,462.36146,463.01005,
      463.46151,464.42252,465.07109,465.48010,465.71534,465.83351,
      465.91704,465.96531,466.06140,466.22847,466.49365,466.88764,
      467.30077,467.32503,467.44437,468.09907,468.13318,468.20048,
      468.93936,468.99417,469.19487,469.24341,469.49081,469.98202,
      470.46817,470.88778,471.52108,472.06218,473.91662,474.01935,
      474.51501,475.20753,476.38218,477.42484,477.54555,477.72450,
      477.73503,477.84786,479.19270,479.23774,480.19856,480.31761,
      480.34307,480.40581,481.08729,481.12014,482.49196,484.16289,
      485.01411,486.31948,486.95469,487.50822,487.51252,487.99400,
      488.88682,489.89084,489.94540,490.71282,491.96823,492.33514,
      492.35136,493.67882,493.71058,494.17651,494.67755,495.70714,
      497.93851,499.02770,501.33477,502.05413,502.52746,502.81252,
      502.81973,503.38485,504.55444,504.57542,504.85693,504.86711,
      505.96426,505.98499,507.01782,507.62491,509.35534,510.24272,
      511.12197,512.61079,512.61379,512.87735,514.60329,516.00959,
      517.31129,517.32199,518.04977,518.36493,518.38258,520.60691,
      520.80805,522.29192,522.95026,526.09984,527.84728,527.85217,
      529.36718,529.37057,529.58838,530.70886,530.78550,530.79828,
      530.85896,531.22909,531.44929,532.40947,533.39788,537.66565,
      537.66772,539.21870,539.27832,539.29517,539.49600,539.86805,
      542.22866,542.23361,542.24055,543.00565,543.01500,543.07801,
      544.01867,544.18951,546.28272,546.82930,546.83013,549.77918,
      551.50741,551.77269,552.83205,552.83532,554.30767,554.62039,
      555.55662,555.56430,557.31108,557.77438,558.18238,558.32803,
      560.44762,560.81804,562.67528,562.67670,563.70461,563.78910,
      564.86895,565.02515,567.10619,567.11062,567.60799,567.61267,
      570.01420,571.82903,571.82960,571.87439,572.77599,572.95848,
      575.07398,576.67936,577.76991,577.77208,577.89945,578.76463,
      580.28277,580.28725,581.90576,584.05616,584.43974,584.63074,
      585.70594,587.27949,587.64208,587.64304,588.06613,588.32033,
      589.61387,590.87151,591.93290,591.93567,592.17814,592.18033,
      593.58513,594.38897,596.75934,596.79061,596.79101,596.98538,
      597.89734,598.13817,600.08171,601.33850,601.85445,602.66383,
      602.66524,602.98963,604.96870,604.97124,608.53737,609.38405,
      610.65033,611.93881,612.31651,612.56836,612.56900,612.65886,
      612.91137,615.01460,616.03643,616.14636,616.71736,616.71907,
      616.72159,616.72252,618.28630,618.64101,621.71587,621.71614,
      622.13867,623.27242,625.03540,627.01124,627.37978,627.51663,
      627.51754,628.37755,629.61882,629.62021,631.90594,632.70295,
      634.10335,635.70816,635.76650,636.39752,636.99951,637.45633,
      637.45676,637.59050,637.80841,637.87387,639.01958,639.03160,
      639.72569,640.12126,641.24320,641.24356,641.46303,641.46405,
      641.50497,641.56070,641.72811,641.78393,643.95650,645.76794,
      646.46645,646.60656,646.60675,647.22241,647.75793,648.37538,
      649.27701,649.52824,649.97178,649.98070,650.00936,650.04229,
      650.15782,650.16800,650.33521,650.45874,650.49850,650.52488,
      650.72148,650.85581,650.89054,650.91290,650.94767,651.09020,
      651.17618,651.24737,651.37908,651.38097,651.48976,651.51881,
      651.57432,651.61133,651.63663,651.66259,651.66949,651.67964,
      651.70351,651.70690,651.72136,651.72220,652.33102,652.33159,
      652.64351,653.93196,654.23721,654.23794,654.48172,655.57043,
      655.88521,657.60185,659.33962,659.87588,660.06899,660.09945,
      660.23970,662.30815,662.30843,662.39212,663.01390,665.74732,
      665.74744,666.17315,666.17376,666.47739,667.39863,670.22327,
      670.49846,670.74275,671.46440,671.46452,671.62754,672.30147,
      672.50778,674.54713,675.11383,675.51489,675.74675,676.84072,
      677.08514,677.10959,677.10995,678.82766,678.82803,680.35058,
      680.35750,680.67731,681.45740,682.01903,683.76360,685.76352,
      686.26033,687.12588,687.12606,687.35723,689.34158,690.23778,
      690.23781,690.85084,690.85119,692.84970,693.43401,696.29104,
      696.29112,698.10641,698.91211,699.47147,700.02534,700.14918,
      701.23111,701.85484,701.85506,703.07451,703.39372,703.39389,
      704.28216,705.78501,706.74389,706.88175,706.97482,708.17835,
      711.77619,711.91154,711.91165,712.53379,715.49903,715.49922,
      715.86351,716.43499,716.95252,720.37033,721.08809,721.08815,
      721.55231,724.19978,724.92159,725.16864,725.66100,726.48268,
      726.56915,726.56928,726.64641,726.68373,727.74627,727.93865,
      727.93873,729.14192,729.43733,729.43735,729.50146,734.39511,
      734.50691,736.05146,736.66704,736.66712,736.77536,737.97732,
      740.12050,740.12060,743.88908,744.90109,745.85712,745.85715,
      746.97136,747.22646,747.83267,748.88851,749.09516,749.59330,
      749.59342,751.25482,751.25490,751.52843,751.88526,752.46551,
      752.46554,754.04451,754.19221,754.19223,755.57690,757.59559,
      757.64748,759.15858,759.65229,761.39427,761.39432,762.94270,
      763.10125,763.75139,763.90678,764.71789,764.71795,767.26428,
      768.16681,769.19849,770.53925,770.59964,770.59966,771.61394,
      772.22760,772.87097,774.05769,774.05854,775.91405,775.91409,
      776.05213,776.85196,776.97710,776.97711,778.92322,778.92323,
      779.49046,782.76977,784.00910,785.41897,785.54782,786.09503,
      786.09506,789.29372,789.29375,789.92813,791.04762,791.15273,
      791.36148,791.53797,791.57970,793.44446,794.19023,795.31713,
      795.31715,795.66381,797.51621,798.51604,798.51947,799.59819,
      800.54895,800.54897,801.47601,801.47601,801.60215,802.85916,
      803.14448,803.63155,803.63155,803.99145,808.09043,810.77107,
      810.77108,812.41162,812.43528,812.65197,813.17556,813.85036,
      813.85037,814.09374,814.36025,814.49537,815.19089,816.16740,
      816.58880,816.81326,818.79388,819.41661,820.01102,820.01103,
      822.21594,822.96306,822.97335,823.33427,824.58190,824.61871,
      824.64500,824.76025,824.91093,825.16154,825.16155,825.22032,
      825.34304,825.90705,825.96460,825.96460,826.00354,826.15502,
      826.57132,826.67409,827.15202,827.30706,828.08849,828.26535,
      828.31836,828.31836,828.42678,828.70810,828.89291,829.70072,
      829.79892,829.81213,829.91688,830.49539,830.71379,831.17997,
      831.26331,831.99368,832.00573,832.24736,832.67830,833.14638,
      833.21026,833.31132,833.88903,833.90082,834.40958,834.53126,
      834.87271,835.05547,835.27950,835.42406,835.42407,835.48877,
      835.63211,835.84440,835.93356,836.13378,836.18752,836.23738,
      836.36683,836.39805,836.55213,836.56954,836.69722,836.70646,
      836.72135,836.80866,836.81330,836.89224,836.89441,836.95305,
      836.95398,836.99557,836.99593,837.02374,837.02386,837.04101,
      837.04104,837.05034,837.05035,837.57712,838.39003,838.39004,
      839.33926,839.47385,839.83319,843.82414,844.03346,844.16060,
      844.68269,844.68270,845.92887,846.16826,847.39151,847.41729,
      847.73650,848.87126,849.75376,849.75377,849.89357,850.44501,
      850.44501,850.52968,850.98542,852.98479,852.98479,854.37660,
      859.44521,859.49469,860.05564,860.05564,860.57606,860.61403,
      862.16417,862.91485,862.91485,864.16234,865.30469,865.52794,
      866.21997,866.81583,869.33350,869.33351,871.73471,871.79186,
      871.84867,873.16943,874.32746,874.32746,874.74450,877.63194,
      877.63194,882.24984,882.76712,883.61220,884.66737,884.66737,
      884.87761,885.21409,886.73429,887.42679,887.42679,891.26568,
      893.96475,893.96475,896.15180,896.26560,897.75858,898.81674,
      898.88440,898.88440,899.18079,899.80532,900.16980,900.43089,
      902.26090,902.26090,903.33089,904.67893,905.12150,906.57671,
      907.60857,908.41871,909.26076,909.26077,909.59440,909.59440,
      911.81582,911.92770,911.92770,912.25910,916.59917,918.57770,
      918.57770,920.45575,920.66739,922.76227,923.42628,923.42628,
      924.57204,924.97667,926.87270,926.87270,928.04806,929.51177,
      930.03271,930.32889,930.35232,933.28978,933.30407,933.83728,
      933.83728,934.18307,934.18307,936.41929,936.41929,936.82059,
      937.28891,941.57305,942.80802,943.17358,943.17358,944.68419,
      944.85034,945.05502,946.58320,946.59550,947.95469,947.95469,
      948.80841,951.01888,951.45973,951.46838,951.46838,951.55227,
      952.42083,953.10490,958.39831,958.39831,958.75731,958.75731,
      959.29366,959.38668,959.38668,959.59763,960.66907,960.90317,
      960.90317,961.50530,965.47152,966.22916,966.35435,966.47585,
      967.75358,967.75358,968.81314,969.43174,969.72462,970.32930,
      972.47115,972.47115,972.69306,974.02467,975.30674,975.86298,
      976.04891,976.04891,978.36493,979.26244,982.94520,982.94520,
      983.31793,983.31793,983.42658,983.85357,983.85357,985.38079,
      985.38079,986.38869,986.39509,987.93415,988.28893,989.14104,
      990.60603,990.77758,991.43585,991.94190,992.31510,992.31885,
      992.31885,992.47661,992.81333,993.37843,993.80363,993.97511,
      994.59089,995.50114,995.91977,996.88667,996.97711,996.97711,
      997.05739,997.33479,998.17182,998.62374,998.65962,998.74226,
      998.80826};

    static const double flin[1363]={
       .433E+01, .308E+01, .826E+01, .447E+01, .209E+01, .826E+01,
       .703E+01, .150E+01, .578E+01, .568E+01, .159E+01, .303E+01,
       .688E+01, .138E+01, .303E+01, .311E+01, .562E+01, .104E+01,
       .706E+01, .433E+01, .164E+01, .549E+01, .685E+01, .844E+01,
       .328E+01, .459E+01, .808E+01, .124E+01, .116E+01, .131E+01,
       .177E+01, .107E+01, .311E+01, .721E+01, .845E+01, .807E+01,
       .412E+01, .294E+01, .594E+01, .137E+01, .454E+01, .127E+01,
       .980E+00, .178E+01, .244E+01, .216E+01, .141E+01, .587E+01,
       .669E+01, .308E+01, .279E+01, .894E+00, .418E+01, .544E+01,
       .512E+00, .469E+01, .193E+01, .143E+01, .725E+01, .667E+01,
       .284E+01, .302E+01, .812E+00, .530E+01, .789E+01, .209E+01,
       .116E+01, .737E+01, .609E+01, .142E+01, .788E+01, .293E+01,
       .354E+01, .146E+01, .225E+01, .391E+01, .735E+00, .282E+01,
       .277E+01, .478E+01, .401E+01, .240E+01, .270E+01, .475E+01,
       .256E+01, .526E+01, .649E+01, .281E+01, .139E+01, .271E+01,
       .606E+01, .255E+01, .648E+01, .665E+00, .742E+01, .107E+01,
       .622E+01, .510E+01, .769E+01, .753E+01, .132E+01, .483E+01,
       .769E+01, .383E+01, .229E+01, .117E+01, .255E+01, .341E+01,
       .370E+01, .602E+00, .507E+01, .382E+01, .150E+01, .629E+01,
       .248E+01, .122E+01, .343E+01, .629E+01, .487E+01, .321E+01,
       .496E+01, .434E+01, .634E+01, .231E+01, .625E+01, .520E+01,
       .990E+00, .769E+01, .750E+01, .489E+01, .247E+01, .760E+01,
       .749E+01, .600E+01, .545E+00, .239E+01, .365E+01, .109E+01,
       .672E+01, .487E+01, .100E+01, .487E+01, .893E+00, .410E+01,
       .349E+01, .609E+01, .736E+01, .645E+01, .266E+01, .609E+01,
       .494E+00, .410E+01, .790E+01, .932E+00, .485E+01, .208E+01,
       .783E+01, .220E+01, .917E+00, .363E+01, .730E+01, .729E+01,
       .469E+01, .345E+01, .518E+01, .643E+01, .836E+01, .777E+01,
       .150E+01, .287E+01, .467E+01, .480E+01, .653E+01, .633E+00,
       .483E+01, .328E+01, .589E+01, .755E+00, .871E+01, .450E+00,
       .589E+01, .308E+01, .472E+01, .201E+01, .796E+01, .325E+01,
       .709E+01, .185E+01, .852E+00, .897E+01, .709E+01, .201E+01,
       .448E+01, .330E+01, .461E+01, .441E+01, .660E+01, .409E+01,
       .447E+01, .563E+00, .447E+01, .352E+01, .830E+01, .830E+01,
       .661E+01, .794E+01, .411E+00, .541E+01, .915E+01, .390E+00,
       .568E+01, .431E+01, .373E+01, .306E+01, .560E+01, .180E+01,
       .568E+01, .413E+01, .394E+01, .808E+01, .253E+01, .304E+01,
       .689E+01, .665E+01, .689E+01, .162E+01, .361E+00, .924E+01,
       .427E+01, .794E+00, .426E+01, .809E+01, .376E+00, .809E+01,
       .159E+01, .548E+01, .458E+01, .172E+00, .284E+01, .474E+01,
       .547E+01, .307E+01, .819E+01, .811E+01, .926E+01, .642E+01,
       .667E+01, .680E+01, .283E+01, .164E+00, .564E+01, .668E+01,
       .668E+01, .139E+01, .405E+01, .405E+01, .743E+00, .347E+00,
       .789E+01, .923E+01, .789E+01, .667E+01, .137E+01, .363E+01,
       .526E+01, .526E+01, .262E+01, .261E+01, .129E+02, .729E+01,
       .119E+02, .139E+02, .511E+01, .108E+02, .149E+02, .973E+01,
       .647E+01, .647E+01, .158E+02, .664E+01, .827E+01, .509E+01,
       .869E+01, .116E+01, .384E+01, .915E+01, .383E+01, .165E+02,
       .423E+01, .769E+01, .698E+01, .115E+01, .768E+01, .673E+01,
       .768E+01, .321E+00, .589E+01, .172E+02, .584E+01, .505E+01,
       .699E+00, .658E+01, .505E+01, .239E+01, .500E+01, .178E+02,
       .239E+01, .422E+01, .905E+01, .819E+01, .486E+01, .626E+01,
       .626E+01, .349E+01, .182E+02, .649E+01, .937E+00, .362E+01,
       .362E+01, .280E+01, .568E+01, .933E+00, .215E+01, .747E+01,
       .747E+01, .843E+01, .152E+01, .853E+00, .298E+00, .185E+02,
       .638E+01, .484E+01, .547E+01, .483E+01, .217E+01, .833E+00,
       .217E+01, .553E+01, .718E+01, .145E+01, .892E+01, .659E+00,
       .200E+01, .623E+01, .616E+01, .605E+01, .605E+01, .187E+02,
       .251E+01, .913E+01, .716E+00, .340E+01, .340E+01, .299E+01,
       .714E+00, .345E+01, .726E+01, .726E+01, .624E+01, .462E+01,
       .188E+02, .629E+01, .462E+01, .388E+01, .194E+01, .278E+00,
       .880E+01, .194E+01, .429E+01, .583E+01, .583E+01, .502E+00,
       .588E+01, .317E+01, .101E+02, .501E+00, .317E+01, .467E+01,
       .699E+01, .625E+00, .188E+02, .738E+01, .705E+01, .705E+01,
       .150E+01, .503E+01, .149E+01, .440E+01, .440E+01, .644E+01,
       .867E+01, .172E+01, .172E+01, .261E+00, .537E+01, .778E+01,
       .188E+02, .695E+01, .562E+01, .562E+01, .302E+00, .302E+00,
       .295E+01, .295E+01, .111E+02, .568E+01, .167E+01, .683E+01,
       .683E+01, .594E+00, .163E+01, .238E+02, .418E+01, .418E+01,
       .248E+02, .227E+02, .855E+01, .862E+01, .149E+01, .149E+01,
       .596E+01, .633E+01, .257E+02, .186E+02, .215E+02, .804E+01,
       .804E+01, .203E+02, .266E+02, .759E+01, .246E+00, .540E+01,
       .540E+01, .191E+02, .126E+00, .126E+00, .272E+02, .622E+01,
       .273E+01, .273E+01, .179E+02, .675E+01, .188E+01, .121E+02,
       .766E+01, .950E+01, .278E+02, .662E+01, .662E+01, .167E+02,
       .184E+02, .645E+01, .396E+01, .396E+01, .127E+01, .127E+01,
       .845E+01, .179E+01, .282E+02, .155E+02, .783E+01, .783E+01,
       .518E+01, .518E+01, .144E+02, .104E+02, .666E+01, .250E+01,
       .250E+01, .211E+01, .233E+00, .285E+02, .681E+01, .134E+02,
       .131E+02, .182E+02, .640E+01, .640E+01, .373E+01, .373E+01,
       .105E+01, .781E+01, .105E+01, .685E+01, .836E+01, .124E+02,
       .842E+01, .195E+01, .286E+02, .761E+01, .761E+01, .114E+02,
       .708E+01, .236E+01, .115E+02, .496E+01, .496E+01, .228E+01,
       .228E+01, .702E+01, .106E+02, .180E+02, .286E+02, .618E+01,
       .618E+01, .142E+02, .222E+00, .984E+01, .123E+02, .836E+00,
       .351E+01, .836E+00, .351E+01, .828E+01, .262E+01, .717E+01,
       .908E+01, .740E+01, .740E+01, .733E+01, .209E+01, .836E+01,
       .474E+01, .474E+01, .923E+01, .285E+02, .205E+01, .205E+01,
       .768E+01, .805E+01, .133E+02, .177E+02, .703E+01, .730E+01,
       .596E+01, .596E+01, .152E+02, .640E+01, .290E+01, .628E+00,
       .628E+00, .328E+01, .328E+01, .578E+01, .744E+01, .212E+00,
       .519E+01, .822E+01, .460E+01, .718E+01, .718E+01, .284E+02,
       .401E+01, .343E+01, .284E+01, .223E+01, .159E+01, .884E+00,
       .883E+00, .159E+01, .223E+01, .284E+01, .342E+01, .400E+01,
       .451E+01, .451E+01, .456E+01, .742E+01, .221E+01, .513E+01,
       .183E+01, .183E+01, .569E+01, .349E+02, .338E+02, .143E+02,
       .625E+01, .358E+02, .326E+02, .681E+01, .319E+01, .737E+01,
       .366E+02, .313E+02, .792E+01, .175E+02, .101E+02, .574E+01,
       .574E+01, .790E+01, .846E+01, .163E+02, .373E+02, .431E+00,
       .431E+00, .300E+02, .306E+01, .306E+01, .899E+01, .281E+02,
       .752E+01, .952E+01, .287E+02, .696E+01, .696E+01, .379E+02,
       .817E+01, .203E+00, .153E+02, .100E+02, .349E+01, .429E+01,
       .429E+01, .161E+01, .161E+01, .831E+01, .273E+02, .105E+02,
       .382E+02, .229E+01, .110E+02, .783E+01, .760E+01, .552E+01,
       .552E+01, .260E+02, .173E+02, .173E+02, .252E+00, .252E+00,
       .283E+01, .283E+01, .278E+02, .115E+02, .381E+01, .164E+02,
       .110E+02, .674E+01, .674E+01, .247E+02, .119E+02, .813E+01,
       .407E+01, .407E+01, .851E+01, .139E+01, .139E+01, .768E+01,
       .195E+00, .123E+02, .234E+02, .415E+01, .529E+01, .529E+01,
       .101E+00, .101E+00, .183E+02, .127E+02, .261E+01, .261E+01,
       .174E+02, .235E+01, .275E+02, .171E+02, .222E+02, .652E+01,
       .652E+01, .774E+01, .250E+01, .250E+01, .859E+01, .131E+02,
       .451E+01, .384E+01, .384E+01, .117E+01, .117E+01, .811E+01,
       .211E+02, .119E+02, .826E+01, .774E+01, .774E+01, .134E+02,
       .189E+00, .184E+02, .507E+01, .507E+01, .194E+02, .200E+02,
       .238E+01, .238E+01, .272E+02, .490E+01, .916E+01, .779E+01,
       .137E+02, .169E+02, .630E+01, .630E+01, .261E+01, .261E+01,
       .236E+01, .190E+02, .362E+01, .362E+01, .958E+00, .958E+00,
       .809E+01, .752E+01, .752E+01, .140E+02, .532E+01, .194E+02,
       .180E+02, .129E+02, .485E+01, .485E+01, .784E+01, .204E+02,
       .216E+01, .216E+01, .182E+00, .143E+02, .268E+02, .171E+02,
       .890E+01, .576E+01, .607E+01, .607E+01, .277E+01, .277E+01,
       .168E+02, .873E+01, .339E+01, .339E+01, .754E+00, .754E+00,
       .163E+02, .205E+02, .807E+01, .730E+01, .730E+01, .145E+02,
       .788E+01, .987E+01, .235E+01, .624E+01, .155E+02, .462E+01,
       .462E+01, .215E+02, .194E+01, .194E+01, .139E+02, .147E+02,
       .148E+02, .177E+00, .295E+01, .585E+01, .585E+01, .265E+02,
       .295E+01, .675E+01, .215E+02, .140E+02, .317E+01, .317E+01,
       .559E+00, .559E+00, .791E+01, .167E+02, .708E+01, .708E+01,
       .133E+02, .807E+01, .150E+02, .225E+02, .126E+02, .440E+01,
       .440E+01, .731E+01, .172E+01, .172E+01, .120E+02, .923E+01,
       .340E+02, .924E+01, .113E+02, .315E+01, .230E+01, .563E+01,
       .563E+01, .314E+01, .106E+02, .225E+02, .107E+02, .794E+01,
       .149E+02, .151E+02, .377E+00, .377E+00, .294E+01, .294E+01,
       .790E+01, .171E+00, .101E+02, .262E+02, .947E+01, .887E+01,
       .326E+02, .685E+01, .685E+01, .828E+01, .166E+02, .769E+01,
       .710E+01, .806E+01, .653E+01, .100E+02, .106E+02, .943E+01,
       .112E+02, .884E+01, .826E+01, .118E+02, .595E+01, .768E+01,
       .710E+01, .235E+02, .537E+01, .652E+01, .124E+02, .594E+01,
       .479E+01, .537E+01, .420E+01, .479E+01, .420E+01, .360E+01,
       .360E+01, .299E+01, .299E+01, .130E+02, .235E+01, .235E+01,
       .167E+01, .167E+01, .908E+00, .908E+00, .417E+01, .417E+01,
       .136E+02, .142E+02, .150E+01, .150E+01, .854E+01, .148E+02,
       .153E+02, .154E+02, .313E+02, .235E+02, .160E+02, .796E+01,
       .334E+01, .540E+01, .540E+01, .333E+01, .166E+02, .216E+00,
       .216E+00, .272E+01, .272E+01, .171E+02, .923E+01, .160E+02,
       .177E+02, .167E+00, .663E+01, .663E+01, .259E+02, .154E+02,
       .300E+02, .245E+02, .182E+02, .806E+01, .223E+01, .165E+02,
       .114E+02, .395E+01, .395E+01, .129E+01, .129E+01, .996E+01,
       .187E+02, .979E+01, .799E+01, .245E+02, .354E+01, .288E+02,
       .192E+02, .518E+01, .518E+01, .353E+01, .156E+02, .840E-01,
       .840E-01, .249E+01, .249E+01, .197E+02, .107E+02, .641E+01,
       .641E+01, .256E+02, .276E+02, .162E+00, .171E+02, .202E+02,
       .806E+01, .372E+01, .372E+01, .800E+01, .108E+01, .108E+01,
       .256E+02, .164E+02, .116E+02, .374E+01, .157E+02, .206E+02,
       .266E+02, .495E+01, .495E+01, .371E+01, .227E+01, .227E+01,
       .123E+02, .213E+01, .210E+02, .124E+02, .618E+01, .618E+01,
       .266E+02, .256E+02, .802E+01, .158E+02, .104E+02, .214E+02,
       .350E+01, .350E+01, .266E+02, .807E+01, .158E+00, .879E+00,
       .879E+00, .181E+02, .741E+01, .741E+01, .394E+01, .133E+02,
       .164E+02, .246E+02, .473E+01, .473E+01, .217E+02, .389E+01,
       .205E+01, .205E+01, .158E+02, .276E+02, .596E+01, .596E+01,
       .803E+01, .237E+02, .220E+02, .143E+02, .276E+02, .350E+01,
       .350E+01, .327E+01, .327E+01, .413E+01, .807E+01, .686E+00,
       .686E+00, .132E+02, .719E+01, .719E+01, .154E+00, .192E+02,
       .228E+02, .201E+01, .223E+02, .450E+01, .450E+01, .163E+02,
       .159E+02, .406E+01, .153E+02, .184E+01, .184E+01, .220E+02,
       .286E+02, .804E+01, .110E+02, .573E+01, .573E+01, .286E+02,
       .226E+02, .432E+01, .359E+01, .359E+01, .305E+01, .305E+01,
       .212E+02, .807E+01, .503E+00, .503E+00, .696E+01, .696E+01,
       .163E+02, .160E+02, .205E+02, .203E+02, .229E+02, .428E+01,
       .428E+01, .162E+01, .162E+01, .421E+01, .163E+02, .197E+02,
       .296E+02, .141E+02, .805E+01, .452E+01, .296E+02, .551E+01,
       .551E+01, .173E+02, .190E+02, .372E+01, .372E+01, .231E+02,
       .283E+01, .283E+01, .336E+00, .336E+00, .808E+01, .160E+02,
       .183E+02, .674E+01, .674E+01, .188E+01, .177E+02, .405E+01,
       .405E+01, .170E+02, .183E+02, .214E+02, .471E+01, .141E+01,
       .141E+01, .806E+01, .233E+02, .306E+02, .117E+02, .163E+02,
       .435E+01, .306E+02, .163E+02, .157E+02, .528E+01, .528E+01,
       .151E+02, .389E+01, .389E+01, .161E+02, .201E+02, .144E+02,
       .195E+02, .208E+02, .188E+02, .261E+01, .261E+01, .214E+02,
       .182E+02, .175E+02, .189E+00, .189E+00, .220E+02, .808E+01,
       .169E+02, .138E+02, .227E+02, .163E+02, .156E+02, .151E+02,
       .652E+01, .652E+01, .132E+02, .233E+02, .150E+02, .144E+02,
       .194E+02, .235E+02, .126E+02, .138E+02, .239E+02, .120E+02,
       .132E+02, .126E+02, .490E+01, .114E+02, .120E+02, .108E+02,
       .245E+02, .114E+02, .108E+02, .102E+02, .102E+02, .963E+01,
       .963E+01, .904E+01, .904E+01, .383E+01, .383E+01, .847E+01,
       .847E+01, .789E+01, .789E+01, .731E+01, .731E+01, .251E+02,
       .673E+01, .673E+01, .615E+01, .615E+01, .556E+01, .556E+01,
       .807E+01, .497E+01, .497E+01, .437E+01, .437E+01, .375E+01,
       .375E+01, .311E+01, .311E+01, .244E+01, .244E+01, .172E+01,
       .172E+01, .925E+00, .925E+00, .316E+02, .120E+01, .120E+01,
       .224E+02, .316E+02, .257E+02, .448E+01, .262E+02, .161E+02,
       .506E+01, .506E+01, .236E+02, .163E+02, .407E+01, .407E+01,
       .204E+02, .268E+02, .239E+01, .239E+01, .509E+01, .720E-01,
       .720E-01, .809E+01, .175E+01, .629E+01, .629E+01, .273E+02,
       .808E+01, .125E+02, .361E+01, .361E+01, .278E+02, .327E+02,
       .327E+02, .100E+01, .100E+01, .162E+02, .161E+02, .235E+02,
       .215E+02, .530E+01, .483E+01, .483E+01, .458E+01, .426E+01,
       .426E+01, .163E+02, .217E+01, .217E+01, .809E+01, .607E+01,
       .607E+01, .808E+01, .551E+01, .337E+02, .338E+01, .338E+01,
       .337E+02, .225E+02, .162E+02, .812E+00, .812E+00, .246E+02,
       .461E+01, .461E+01, .445E+01, .445E+01, .573E+01, .810E+01,
       .195E+01, .195E+01, .172E+02, .163E+02, .161E+01, .466E+01,
       .584E+01, .584E+01, .133E+02, .236E+02, .809E+01, .347E+02,
       .347E+02, .162E+02, .316E+01, .316E+01, .707E+01, .707E+01,
       .596E+01, .629E+00, .629E+00, .283E-04, .256E+02, .438E+01,
       .438E+01, .465E+01, .465E+01, .810E+01, .174E+01, .174E+01,
       .246E+02, .621E+01, .562E+01, .562E+01, .810E+01, .357E+02,
       .471E+01, .162E+02, .357E+02, .183E+02, .286E+02, .294E+01,
       .294E+01, .685E+01, .685E+01, .458E+00, .458E+00, .148E-03,
       .649E+01, .267E+02, .278E+02, .416E+01, .416E+01, .484E+01,
       .257E+02, .484E+01, .141E+02, .810E+01, .153E+01, .153E+01,
       .678E+01, .810E+01, .270E+02, .539E+01, .539E+01, .147E+01,
       .367E+02, .367E+02, .272E+01, .272E+01, .662E+01, .662E+01,
       .263E+02, .450E+01, .450E+01, .710E+01, .474E+01, .302E+00,
       .302E+00, .474E-03, .267E+02, .277E+02, .255E+02, .194E+02,
       .394E+01, .394E+01, .504E+01, .504E+01, .745E+01, .811E+01,
       .133E+01, .133E+01, .248E+02, .811E+01, .377E+02, .377E+02,
       .517E+01, .517E+01, .241E+02, .783E+01, .250E+01, .250E+01,
       .640E+01, .640E+01, .234E+02, .457E+01, .457E+01, .839E-01,
       .839E-01, .118E-02, .278E+02, .227E+02, .825E+01, .150E+02,
       .288E+02, .281E+02, .274E+02, .221E+02, .267E+02, .371E+01,
       .371E+01, .473E+01, .523E+01, .260E+02, .523E+01, .811E+01,
       .254E+02, .214E+02, .247E+02, .870E+01, .113E+01, .113E+01,
       .811E+01, .240E+02, .387E+02, .387E+02, .207E+02, .205E+02,
       .233E+02};

    static const double el[1363]={
      424.292,  287.709, 1543.708,  484.554,   70.647, 1543.513,
      1124.609,  789.770,  771.538,  732.512,   11.284,  199.673,
      1076.979,  832.364,  263.425,  312.612,  731.779,    7.045,
      1124.536,  452.992,  747.709,  693.846, 1076.446, 1600.115,
      220.137,  516.433, 1487.264,  178.048,  198.427,  158.861,
      58.753,  219.898,  338.689, 1172.866, 1600.063, 1487.044,
      394.599,  240.098,  811.569,  140.769,  453.735,  875.898,
      242.461,  706.857,   82.419,   15.885,  123.776,  771.431,
      1029.719,  366.049,  179.328,  266.115,  422.410,  692.885,
      3.504,  549.552,  667.216,  107.886, 1172.824, 1029.399,
      218.016,  394.599,  290.962,  655.940, 1431.849,  628.790,
      920.538, 1222.328,  852.818,   93.105, 1431.677,  424.344,
      241.302,   47.982,  591.680,  366.035,  316.806,  455.291,
      21.562,  583.814,  393.003,  555.687,  487.544,  484.426,
      521.009,  655.182,  983.680,   95.264,   79.538,  196.986,
      811.407,  160.229,  983.452,  343.849, 1222.182,  966.188,
      895.288,  619.264, 1377.648, 1272.897,   66.992,  619.228,
      1377.514,  364.876, 1467.466,   38.241,  177.113,   28.418,
      338.714,  371.994,  618.673,  263.625,    1.186,  938.860,
      3.516,   55.674,    7.009,  938.605,  655.899,  109.286,
      516.165,   11.764,  938.883,  142.275,  852.636,   17.578,
      1013.055, 1324.675, 1324.658,  583.817, 1409.916, 1272.805,
      1324.555,   24.549,  401.247,  158.404,  337.733,   45.490,
      32.773,  693.834,    -.020,  583.360,   29.832,   36.357,
      312.630,  895.257,   42.046,  983.708, 1353.578,  895.061,
      431.511,  287.206,   52.565,   36.346,  732.940,  125.565,
      1377.664,  140.863, 1060.947,  124.585, 1272.879, 1272.800,
      549.493,  311.877,  549.151,  895.015,   64.125, 1324.590,
      1.175, 1298.357,  549.144,  773.225, 1029.768,   22.554,
      45.485,  287.781,  852.768,   28.349,   76.922,  462.992,
      852.619, 1244.454,  814.796,  124.495, 1431.869,  287.211,
      1222.309,  110.095, 1109.969,   90.753, 1222.249,    3.504,
      516.491, 1191.774,  857.660,  311.849, 1076.968,  140.864,
      516.227,   21.603,  901.623, 1140.316, 1657.686, 1657.662,
      938.640, 1377.534,  495.496,  583.383,  105.716,   16.509,
      811.592,  946.891, 1090.080,  264.061,   55.706,  109.304,
      811.479,  993.371, 1041.166, 1487.191,    7.066,  263.637,
      1172.848, 1125.414, 1172.802,   95.862,   15.812,  121.908,
      484.608, 1160.030,  484.411, 1600.133,  529.128, 1600.116,
      95.294,  771.528,  158.426,   11.697,  241.567,  337.757,
      771.443,   11.760, 1543.735, 1431.731,  139.029,   67.024,
      1175.113,  983.408,  241.257,   11.281,  618.662, 1124.594,
      1124.559,   82.763,  453.940,  453.795, 1211.237,  563.895,
      1543.687,  157.379, 1543.673, 1225.970,   82.368,   17.583,
      732.574,  732.511,  220.296,  220.173,  168.063,   79.546,
      148.938,  188.435,  177.175,  130.965,  209.949,  114.151,
      1077.547, 1077.521,  232.598, 1277.993, 1486.979,  364.834,
      98.504,   70.893,  424.386,  176.860,  424.280,  256.380,
      24.633,   84.030, 1029.319,   70.629, 1488.445,   70.837,
      1488.435,  599.704,  655.191,  281.389,   58.734,  694.928,
      1263.598, 1331.289,  694.882,  200.243,   47.927,  307.424,
      200.087,   38.323,  197.375,   93.174,   32.806, 1031.606,
      1031.587,   29.830,  334.679, 1385.765,   60.248,  396.144,
      396.068,   22.554,  197.013,   60.080,   16.598, 1434.408,
      1434.401, 1543.473,   11.669,    8.069,  636.662,  363.054,
      1441.528,  658.391,  393.082,  658.358,  181.407,    7.862,
      181.299,   42.099, 1076.470,   11.257,  218.929, 1317.021,
      15.817, 1498.485,  692.872,  986.971,  986.957,  392.546,
      21.542,  107.914,   50.823,  369.012,  368.958,   28.332,
      50.722,   36.385, 1381.476, 1381.470,   52.506,  623.061,
      423.054,  217.945,  623.037,   45.403,  163.683,  674.676,
      241.626,  163.612,   55.683,  943.442,  943.432,   42.615,
      422.407,  343.089,  123.768,   42.559,  343.052,   67.028,
      64.124, 1371.515,  454.779, 1124.762, 1329.847, 1329.843,
      5.650,   79.535,    5.580,  588.937,  588.920,  731.808,
      265.474,  147.271,  147.224,  713.855,   93.105,   76.947,
      487.522,  240.074,  901.117,  901.110,   35.519,   35.490,
      318.373,  318.348,  140.740,  107.837,    8.045, 1279.323,
      1279.320, 1427.087,    7.833,  574.630,  556.018,  556.006,
      611.699,  538.587,  290.379,   90.771,  131.967,  131.938,
      123.733,  453.011,  649.989,  521.385,  503.777, 1723.351,
      1723.350,  470.205,  689.396, 1174.295,  754.105,  859.897,
      859.892,  437.680,   29.631,   29.619,  730.016,  140.692,
      294.864,  294.847,  406.409,  771.803,   11.674,  158.829,
      263.406,  105.791,  771.646, 1230.003, 1230.001,  376.400,
      556.371,  158.814,  524.205,  524.196,  117.770,  117.753,
      316.349,   11.346,  814.483,  347.464, 1665.832, 1665.831,
      819.981,  819.977,  319.808,  121.904,  178.100,  272.460,
      272.449,   16.538,  795.434,  858.426,  484.699,  293.342,
      178.038,  592.387, 1181.786, 1181.784,  493.696,  493.690,
      104.879, 1224.971,  104.869,  198.451,  343.490,  268.074,
      287.845,   15.817,  903.474, 1609.516, 1609.515,  139.106,
      813.061,   22.537,  244.113,  781.168,  781.166,  251.361,
      251.354,  219.867,  221.266,  629.537,  949.528, 1134.772,
      1134.771,  198.467,  837.850,  199.739,  157.394,   93.093,
      464.291,   93.087,  464.287,  371.611,   29.869,  242.449,
      179.338, 1554.402, 1554.402,  517.577,   21.541,  160.166,
      743.560,  743.558,  313.395,  996.789,  231.367,  231.362,
      142.227, 1276.793,  176.768,  667.728,  125.520,  266.098,
      1088.961, 1088.960,  219.915,  110.046,   38.236,   82.509,
      82.507,  435.990,  435.987,   95.804,  855.487,  881.360,
      82.791,  400.918,   70.903, 1500.392, 1500.391, 1045.060,
      60.238,   50.792,   42.561,   35.541,   29.630,   24.925,
      24.920,   29.618,   35.514,   42.506,   50.693,   60.074,
      707.054,  707.053,   70.647,  290.916,   28.414,   82.411,
      212.475,  212.473,   95.263, 1221.961, 1169.249,  197.325,
      109.302, 1275.782, 1117.651,  124.525,   47.936,  140.931,
      1330.708, 1067.274,  158.418,  707.068,  340.162, 1044.353,
      1044.353,  551.647,  177.083,  242.481, 1386.835,   73.129,
      73.128, 1018.025,  408.992,  408.990,  196.924, 1094.444,
      316.803,  217.941,  970.010, 1447.583, 1447.583, 1444.060,
      431.317,  926.069,  218.966,  240.129,   58.769,  671.852,
      671.851,  194.887,  194.885, 1329.763,  923.139,  263.390,
      1502.382,   36.330,  287.819,  899.085,  343.862, 1000.848,
      1000.848,  877.421,  747.465,  266.164,   64.950,   64.950,
      383.097,  383.096, 1144.846,  313.417,   70.834,  241.691,
      367.949, 1395.978, 1395.978,  832.965,  340.082,  462.714,
      637.752,  637.751,  586.816,  178.401,  178.400,  371.994,
      971.784,  368.013,  789.681,   84.030,  958.546,  958.546,
      57.873,   57.873,  290.962,  396.908,  358.405,  358.404,
      265.401,   45.383, 1196.373,  788.927,  747.679, 1345.474,
      1345.474,  401.200,   21.397,   21.396, 1383.986,  427.068,
      98.455,  604.855,  604.854,  163.118,  163.117,  495.314,
      706.769,  396.961,  943.760, 1798.134, 1798.134,  458.292,
      1018.609,  290.397,  917.446,  917.446,  316.875,  667.160,
      334.915,  334.914, 1249.031,  114.108,  623.189,  431.482,
      490.680,  831.462, 1296.173, 1296.173,   24.923,   24.919,
      55.668,  628.760,  573.060,  573.060,  149.036,  149.036,
      529.020, 1740.733, 1740.733,  524.231,  130.888,  316.379,
      591.677,  427.100,  877.548,  877.548,  462.942,  343.800,
      312.627,  312.627, 1066.549,  558.846, 1302.727,  555.718,
      1439.367,  148.891, 1248.074, 1248.074,   29.653,   29.641,
      875.079,  989.719,  542.568,  542.567,  136.156,  136.156,
      520.988,  343.450,  563.736, 1684.434, 1684.434,  594.525,
      495.481,  660.669,   66.976,  168.116,  487.491,  838.752,
      838.752,  371.936,  291.441,  291.441,  458.270,  455.329,
      631.368, 1115.507,   35.488, 1201.076, 1201.076, 1357.472,
      35.460,  188.459,  371.610,  424.304,  513.177,  513.177,
      124.377,  124.377,  529.102,  919.885, 1629.337, 1629.337,
      394.517,  599.662,  669.378,  401.183,  365.965,  801.158,
      801.158,  209.917,  271.457,  271.457,  338.748, 1495.909,
      1526.242, 1036.765,  312.562,   42.532,   79.503, 1155.281,
      1155.281,   42.474,  699.364,  400.861,  287.706,  563.905,
      490.672,  708.453,  113.799,  113.799,  484.888,  484.888,
      232.586, 1165.584,  264.076, 1413.273,  241.569,  220.282,
      1467.448, 1575.341, 1575.341,  200.312,  965.686,  181.356,
      163.713,  636.601,  147.180,  263.679,  287.178,  241.276,
      311.871,  220.070,  200.062,  337.756,  131.954,  181.253,
      163.644,  431.537,  117.836,  147.135,  364.833,  131.926,
      104.822,  117.818,   93.112,  104.812,   93.107,   82.505,
      82.503,   73.101,   73.100,  392.999,   64.898,   64.898,
      57.897,   57.896,   51.996,   51.996,  764.766,  764.766,
      422.451,  452.990,  252.675,  252.675,  256.363,  484.711,
      748.596,  517.613, 1409.930,  431.304,  551.595,  599.693,
      50.787, 1110.687, 1110.687,   50.683,  586.853,  104.422,
      104.422,  457.902,  457.902,  623.187,  281.343,  524.207,
      660.694, 1216.782, 1522.547, 1522.547, 1470.140,  789.907,
      1353.497,  462.999,  699.372,  674.653,   93.142, 1012.589,
      739.177,  729.476,  729.476,  234.994,  234.994,  307.421,
      739.220, 1085.005,  636.667,  462.740,   60.257, 1298.361,
      780.137, 1067.295, 1067.295,   60.084,  832.389,   96.146,
      96.146,  432.016,  432.016,  822.220,  334.692, 1470.955,
      1470.955,  495.468, 1244.429, 1269.102,  558.776,  865.470,
      713.818,  695.388,  695.388,  674.729,  218.614,  218.614,
      495.371, 1060.599,  363.053,   70.946,  875.842,  909.784,
      1191.711, 1025.005, 1025.005,   70.675,  407.432,  407.432,
      780.115,  107.887,  955.262,  392.498, 1420.564, 1420.564,
      529.142, 1140.314,  713.880,  920.468, 1134.344, 1001.905,
      662.501,  662.501,  528.997,  753.995, 1322.544,  203.335,
      203.335,  594.577, 1881.863, 1881.863,   82.760,  423.025,
      1109.619, 1090.045,  983.916,  983.915, 1049.610,   82.353,
      383.850,  383.850,  966.169,  563.920, 1371.274, 1371.274,
      754.121, 1041.108, 1098.479,  454.728,  563.719,   47.295,
      47.295,  630.815,  630.815,   95.802,  795.385,  189.157,
      189.157,  822.181, 1824.374, 1824.374, 1377.007,  631.410,
      993.408,  123.735, 1148.511,  943.928,  943.928, 1159.753,
      1013.046,   95.215,  487.507,  361.568,  361.568,  946.846,
      599.702,  795.453, 1184.889, 1323.186, 1323.186,  599.637,
      1199.607,  110.077,   52.020,   52.019,  600.330,  600.330,
      901.624,  837.785,  176.279,  176.279, 1768.186, 1768.186,
      521.359, 1060.901,  857.642,  669.374, 1251.768,  905.241,
      905.241,  340.388,  340.388,  109.257, 1211.003,  814.798,
      636.688,  865.380,  837.878,  125.588,  636.552, 1276.198,
      1276.198,  556.382,  773.293,   57.845,   57.845, 1305.093,
      570.947,  570.947,  164.503,  164.503,  881.396, 1109.936,
      732.922, 1713.099, 1713.099,  140.682,  693.785,  867.656,
      867.656,  655.877,  592.376,  708.466,  142.241,  320.408,
      320.408,  881.396, 1359.484,  674.677, 1236.644,  619.197,
      124.477,  674.666, 1263.371,  583.741, 1230.512, 1230.512,
      549.506,   64.870,   64.870, 1160.053,  855.493,  516.391,
      813.090,  898.984,  771.777,  542.765,  542.765,  943.762,
      731.756,  692.929,  153.926,  153.926,  989.724,  926.017,
      655.197,  484.592, 1036.769,  618.662,  583.324,  909.816,
      1659.113, 1659.113,  453.908, 1084.994,  549.186,  516.147,
      629.542, 1415.041,  424.436,  484.409, 1134.398,  396.076,
      453.772,  424.337,  160.136,  369.024,  396.004,  343.081,
      1184.978,  368.973,  343.046,  318.345,  318.321,  294.815,
      294.798,  272.490,  272.479,  831.272,  831.272,  251.269,
      251.262,  231.352,  231.347,  212.538,  212.535, 1236.633,
      194.827,  194.825,  178.418,  178.417,  163.111,  163.110,
      926.010,  149.005,  149.005,  136.101,  136.101,  124.398,
      124.398,  113.796,  113.796,  104.395,  104.395,   96.194,
      96.194,   89.094,   89.094,  713.868,  301.629,  301.629,
      748.584,  713.777, 1289.460,  140.868, 1343.458, 1211.252,
      1185.927, 1185.927, 1471.667, 1316.856,   73.097,   73.096,
      667.780, 1398.626,  515.684,  515.684,  179.277,  144.550,
      144.550,  971.746,  158.824, 1606.428, 1606.428, 1454.862,
      971.718, 1289.516,  795.989,  795.989, 1512.264,  754.062,
      754.087,  284.051,  284.051,  955.291, 1263.537,  789.926,
      707.093,  199.664, 1142.443, 1142.443,  158.428,   82.525,
      82.522, 1371.359,  489.903,  489.903, 1018.583, 1554.845,
      1554.845, 1018.523,  221.298,  795.457,  762.007,  762.007,
      795.396,  747.480, 1317.007,  267.574,  267.574,  832.290,
      1100.260, 1100.260,   93.055,   93.049,  244.078, 1066.527,
      465.224,  465.224, 1001.909, 1426.979,  178.062,  177.149,
      1504.361, 1504.361, 1343.510,  788.945, 1066.524,  837.854,
      837.805, 1371.466,  729.125,  729.125, 1974.209, 1974.209,
      268.103,  252.297,  252.297,    3.481,  875.873, 1059.178,
      1059.178,  104.888,  104.877, 1115.477,  441.745,  441.745,
      831.490,  293.370, 1455.079, 1455.079, 1115.523,  881.353,
      196.928, 1427.013,  881.412, 1049.671, 1556.670,  697.445,
      697.445, 1916.828, 1916.828,  238.221,  238.221,    7.001,
      319.779,  920.473, 1498.514, 1019.196, 1019.196,  117.824,
      875.116,  117.806, 1398.633, 1165.632,  419.367,  419.367,
      347.526, 1165.619, 1441.498, 1406.998, 1406.998,  198.394,
      926.052,  926.019,  666.865,  666.865, 1860.548, 1860.548,
      1385.822,   83.218,   83.218,  376.307,  217.956,  225.345,
      225.345,   11.716,  919.825,  966.189, 1331.283, 1098.477,
      980.516,  980.516,  131.865,  131.835,  406.421, 1216.792,
      398.289,  398.289, 1277.978, 1216.814,  971.753,  971.726,
      1360.117, 1360.117, 1225.906,  437.663,  637.486,  637.486,
      1805.568, 1805.568, 1175.063,   89.142,   89.142,  213.569,
      213.569,   17.521,  965.620, 1125.446,  470.129, 1454.888,
      1013.018, 1553.610, 1495.878, 1076.954, 1439.336,  942.936,
      942.936,  240.028,  147.212, 1383.985,  147.164, 1269.056,
      1329.827, 1029.683, 1276.763,  503.816,  378.312,  378.312,
      1269.108, 1224.895, 1018.555, 1018.533,  983.731, 1148.427,
      1174.224};


    static const int ifin1[500]={
      9,   9,  13,  14,  14,  16,  17,  21,  22,  25,
        27,  30,  32,  37,  42,  45,  47,  49,  51,  55,
        57,  59,  61,  64,  65,  67,  72,  75,  76,  79,
        83,  87,  90,  92,  93,  94,  95,  96, 100, 104,
       104, 104, 107, 108, 112, 115, 117, 118, 123, 125,
       127, 129, 133, 137, 139, 139, 142, 144, 146, 148,
       149, 151, 153, 156, 160, 162, 166, 168, 171, 172,
       173, 175, 179, 181, 183, 185, 186, 186, 190, 195,
       196, 197, 201, 203, 208, 211, 217, 220, 220, 221,
       223, 224, 227, 231, 231, 232, 232, 235, 236, 241,
       243, 248, 249, 251, 253, 254, 256, 257, 257, 259,
       264, 265, 267, 268, 268, 273, 276, 279, 283, 287,
       290, 292, 294, 299, 299, 304, 305, 308, 308, 311,
       314, 316, 319, 322, 324, 326, 328, 332, 336, 337,
       342, 344, 345, 350, 351, 355, 356, 356, 359, 360,
       362, 367, 369, 369, 370, 370, 372, 375, 381, 382,
       383, 385, 387, 391, 394, 394, 395, 398, 399, 401,
       405, 407, 407, 408, 410, 411, 420, 426, 426, 430,
       431, 435, 439, 441, 443, 447, 450, 451, 454, 459,
       459, 463, 463, 463, 465, 471, 472, 474, 475, 479,
       479, 485, 487, 490, 493, 493, 496, 498, 500, 501,
       502, 504, 508, 509, 514, 515, 518, 522, 522, 525,
       529, 529, 530, 532, 537, 539, 544, 547, 548, 552,
       555, 565, 573, 579, 581, 584, 586, 591, 597, 600,
       601, 604, 608, 611, 614, 618, 619, 620, 621, 626,
       631, 633, 635, 637, 641, 642, 643, 648, 650, 652,
       652, 653, 655, 662, 665, 666, 666, 668, 673, 676,
       681, 684, 684, 685, 689, 691, 693, 697, 699, 701,
       704, 705, 709, 710, 715, 715, 717, 721, 723, 724,
       727, 728, 733, 735, 739, 741, 745, 747, 748, 753,
       755, 755, 756, 758, 764, 764, 771, 773, 773, 776,
       777, 778, 783, 785, 787, 788, 792, 797, 801, 809,
       810, 814, 817, 834, 854, 858, 860, 861, 866, 869,
       870, 875, 876, 879, 884, 885, 889, 894, 897, 899,
       900, 902, 905, 910, 911, 912, 914, 916, 919, 922,
       926, 930, 931, 931, 935, 935, 940, 940, 941, 944,
       946, 953, 956, 960, 960, 962, 966, 967, 969, 969,
       971, 974, 977, 980, 986, 989, 990, 992, 994, 997,
      1002,1002,1004,1008,1011,1013,1019,1021,1022,1023,
      1024,1028,1028,1031,1036,1038,1041,1044,1047,1051,
      1055,1055,1056,1058,1061,1067,1071,1072,1075,1078,
      1084,1095,1104,1110,1116,1124,1150,1159,1162,1162,
      1167,1169,1173,1180,1182,1183,1183,1183,1189,1192,
      1193,1197,1197,1199,1202,1206,1206,1208,1208,1210,
      1214,1216,1218,1218,1219,1221,1223,1227,1231,1233,
      1235,1237,1239,1243,1247,1247,1248,1250,1252,1253,
      1257,1259,1260,1264,1264,1270,1273,1274,1274,1276,
      1280,1283,1286,1286,1292,1293,1293,1297,1304,1305,
      1305,1309,1312,1315,1318,1319,1323,1324,1325,1327,
      1332,1336,1338,1341,1348,1352,1357,1363,1363,1363};

    static const int ini1[500]={
	1,   1,   2,   4,   9,   9,  13,  14,  14,  16,
        17,  21,  22,  25,  27,  30,  32,  37,  42,  45,
        47,  49,  51,  55,  57,  59,  61,  64,  65,  67,
        72,  75,  76,  79,  83,  87,  90,  92,  93,  94,
        95,  96, 100, 104, 104, 104, 107, 108, 112, 115,
	117, 118, 123, 125, 127, 129, 133, 137, 139, 139,
	142, 144, 146, 148, 149, 151, 153, 156, 160, 162,
	166, 168, 171, 172, 173, 175, 179, 181, 183, 185,
	186, 186, 190, 195, 196, 197, 201, 203, 208, 211,
	217, 220, 220, 221, 223, 224, 227, 231, 231, 232,
	232, 235, 236, 241, 243, 248, 249, 251, 253, 254,
	256, 257, 257, 259, 264, 265, 267, 268, 268, 273,
	276, 279, 283, 287, 290, 292, 294, 299, 299, 304,
	305, 308, 308, 311, 314, 316, 319, 322, 324, 326,
	328, 332, 336, 337, 342, 344, 345, 350, 351, 355,
	356, 356, 359, 360, 362, 367, 369, 369, 370, 370,
	372, 375, 381, 382, 383, 385, 387, 391, 394, 394,
	395, 398, 399, 401, 405, 407, 407, 408, 410, 411,
	420, 426, 426, 430, 431, 435, 439, 441, 443, 447,
	450, 451, 454, 459, 459, 463, 463, 463, 465, 471,
	472, 474, 475, 479, 479, 485, 487, 490, 493, 493,
	496, 498, 500, 501, 502, 504, 508, 509, 514, 515,
	518, 522, 522, 525, 529, 529, 530, 532, 537, 539,
	544, 547, 548, 552, 555, 565, 573, 579, 581, 584,
	586, 591, 597, 600, 601, 604, 608, 611, 614, 618,
	619, 620, 621, 626, 631, 633, 635, 637, 641, 642,
	643, 648, 650, 652, 652, 653, 655, 662, 665, 666,
	666, 668, 673, 676, 681, 684, 684, 685, 689, 691,
	693, 697, 699, 701, 704, 705, 709, 710, 715, 715,
	717, 721, 723, 724, 727, 728, 733, 735, 739, 741,
	745, 747, 748, 753, 755, 755, 756, 758, 764, 764,
	771, 773, 773, 776, 777, 778, 783, 785, 787, 788,
	792, 797, 801, 809, 810, 814, 817, 834, 854, 858,
	860, 861, 866, 869, 870, 875, 876, 879, 884, 885,
	889, 894, 897, 899, 900, 902, 905, 910, 911, 912,
	914, 916, 919, 922, 926, 930, 931, 931, 935, 935,
	940, 940, 941, 944, 946, 953, 956, 960, 960, 962,
	966, 967, 969, 969, 971, 974, 977, 980, 986, 989,
	990, 992, 994, 997,1002,1002,1004,1008,1011,1013,
	1019,1021,1022,1023,1024,1028,1028,1031,1036,1038,
	1041,1044,1047,1051,1055,1055,1056,1058,1061,1067,
	1071,1072,1075,1078,1084,1095,1104,1110,1116,1124,
	1150,1159,1162,1162,1167,1169,1173,1180,1182,1183,
	1183,1183,1189,1192,1193,1197,1197,1199,1202,1206,
	1206,1208,1208,1210,1214,1216,1218,1218,1219,1221,
	1223,1227,1231,1233,1235,1237,1239,1243,1247,1247,
	1248,1250,1252,1253,1257,1259,1260,1264,1264,1270,
	1273,1274,1274,1276,1280,1283,1286,1286,1292,1293,
	1293,1297,1304,1305,1305,1309,1312,1315,1318,1319,
	1323,1324,1325,1327,1332,1336,1338,1341,1348,1352};

      static const int ifin2[500]={
	4,   8,   9,  11,  13,  14,  16,  17,  20,  21,
        24,  27,  30,  32,  37,  41,  45,  47,  48,  50,
        55,  56,  59,  61,  63,  65,  67,  71,  74,  75,
        78,  82,  86,  90,  91,  93,  94,  95,  96, 100,
	102, 104, 104, 106, 108, 112, 114, 117, 118, 121,
	124, 126, 128, 133, 137, 138, 139, 141, 144, 146,
	148, 148, 151, 151, 156, 160, 162, 166, 167, 171,
	171, 173, 174, 178, 181, 182, 185, 185, 186, 189,
	193, 196, 197, 200, 203, 207, 209, 215, 220, 220,
	221, 222, 224, 227, 230, 231, 232, 232, 235, 236,
	239, 242, 248, 249, 250, 253, 254, 256, 256, 257,
	258, 264, 265, 266, 268, 268, 272, 276, 278, 282,
	287, 289, 292, 293, 299, 299, 304, 305, 308, 308,
	309, 313, 315, 319, 322, 323, 324, 328, 332, 335,
	337, 341, 342, 345, 348, 351, 354, 356, 356, 359,
	360, 360, 366, 369, 369, 369, 370, 372, 374, 381,
	382, 383, 385, 387, 389, 393, 394, 395, 397, 399,
	401, 404, 405, 407, 407, 410, 411, 418, 425, 426,
	430, 431, 433, 438, 441, 442, 445, 448, 450, 452,
	459, 459, 461, 463, 463, 465, 469, 471, 474, 474,
	479, 479, 482, 486, 489, 493, 493, 496, 498, 500,
	501, 502, 504, 508, 509, 514, 514, 518, 522, 522,
	525, 529, 529, 530, 532, 535, 538, 544, 546, 548,
	552, 555, 564, 571, 578, 581, 583, 586, 591, 597,
	600, 601, 603, 607, 610, 614, 617, 619, 620, 621,
	623, 627, 633, 635, 637, 638, 641, 643, 648, 648,
	651, 652, 653, 655, 658, 665, 666, 666, 668, 673,
	676, 681, 682, 684, 685, 687, 690, 693, 697, 698,
	699, 703, 705, 709, 710, 713, 715, 716, 720, 723,
	724, 726, 728, 733, 734, 739, 741, 741, 747, 748,
	750, 753, 755, 755, 757, 760, 764, 767, 772, 773,
	776, 777, 778, 783, 785, 786, 788, 791, 797, 801,
	809, 810, 812, 817, 828, 853, 858, 860, 861, 866,
	869, 870, 875, 876, 878, 883, 884, 888, 892, 896,
	899, 900, 902, 905, 908, 910, 912, 914, 915, 919,
	922, 926, 927, 931, 931, 934, 935, 939, 940, 941,
	944, 945, 949, 956, 960, 960, 961, 963, 967, 969,
	969, 970, 973, 976, 980, 986, 989, 990, 992, 994,
	996,1000,1002,1004,1005,1010,1013,1016,1019,1022,
	1022,1024,1028,1028,1031,1036,1038,1041,1042,1045,
	1050,1055,1055,1056,1056,1060,1067,1069,1071,1075,
	1076,1079,1093,1102,1109,1115,1122,1137,1159,1162,
	1162,1165,1169,1172,1178,1180,1183,1183,1183,1187,
	1190,1193,1196,1197,1199,1202,1205,1206,1208,1208,
	1209,1211,1215,1218,1218,1219,1221,1223,1224,1231,
	1233,1234,1236,1239,1243,1247,1247,1247,1248,1251,
	1252,1255,1257,1260,1264,1264,1270,1272,1274,1274,
	1275,1278,1281,1285,1286,1292,1293,1293,1295,1301,
	1305,1305,1309,1311,1315,1317,1319,1323,1324,1325,
	1325,1332,1336,1338,1339,1347,1351,1354,1360,1363};

      static const int ini2[500]={ 
	1,   2,   6,   9,   9,  13,  14,  15,  16,  18,
        21,  22,  25,  27,  30,  34,  39,  43,  46,  47,
        50,  54,  55,  58,  60,  61,  65,  66,  68,  73,
        75,  76,  80,  83,  89,  91,  92,  93,  94,  96,
        98, 102, 104, 104, 104, 108, 110, 113, 116, 117,
	118, 123, 126, 128, 130, 136, 137, 139, 140, 143,
	144, 146, 148, 150, 151, 153, 156, 160, 164, 167,
	168, 171, 173, 173, 175, 180, 181, 183, 185, 186,
	186, 192, 196, 196, 199, 201, 205, 209, 214, 218,
	220, 221, 221, 223, 226, 230, 231, 232, 232, 235,
	235, 238, 242, 245, 248, 250, 252, 254, 255, 256,
	257, 257, 264, 265, 266, 268, 268, 268, 273, 276,
	280, 284, 287, 292, 293, 296, 299, 301, 305, 305,
	308, 308, 311, 314, 317, 320, 323, 324, 328, 328,
	334, 336, 339, 342, 344, 345, 351, 353, 356, 356,
	356, 359, 360, 364, 367, 369, 369, 370, 370, 372,
	377, 382, 383, 383, 385, 388, 391, 394, 394, 396,
	398, 401, 402, 405, 407, 407, 410, 410, 413, 422,
	426, 428, 430, 431, 435, 440, 441, 444, 447, 450,
	452, 455, 459, 459, 463, 463, 463, 468, 471, 473,
	474, 476, 479, 479, 485, 488, 490, 493, 494, 498,
	500, 501, 502, 502, 507, 508, 514, 514, 515, 520,
	522, 522, 525, 529, 530, 530, 534, 537, 540, 544,
	547, 548, 554, 557, 568, 576, 579, 581, 585, 587,
	593, 599, 600, 602, 604, 608, 611, 614, 618, 619,
	621, 622, 627, 631, 634, 636, 638, 641, 642, 645,
	648, 650, 652, 652, 653, 657, 664, 666, 666, 666,
	672, 673, 679, 681, 684, 684, 685, 689, 691, 694,
	697, 699, 701, 705, 707, 709, 710, 715, 716, 717,
	721, 723, 724, 727, 729, 733, 735, 739, 741, 745,
	747, 749, 753, 755, 755, 757, 758, 764, 765, 771,
	773, 773, 777, 778, 780, 783, 785, 787, 788, 794,
	799, 805, 809, 810, 815, 818, 840, 854, 858, 860,
	862, 866, 870, 870, 876, 876, 881, 884, 886, 892,
	894, 898, 899, 900, 905, 906, 910, 912, 912, 914,
	917, 920, 925, 926, 930, 931, 931, 935, 937, 940,
	940, 943, 944, 947, 953, 959, 960, 960, 962, 966,
	967, 969, 969, 971, 975, 978, 982, 986, 989, 990,
	993, 996, 998,1002,1003,1005,1008,1011,1013,1019,
	1022,1022,1023,1025,1028,1030,1034,1037,1040,1041,
	1044,1049,1052,1055,1055,1056,1058,1062,1068,1071,
	1073,1075,1079,1088,1097,1104,1112,1119,1129,1156,
	1161,1162,1162,1167,1171,1173,1180,1182,1183,1183,
	1185,1189,1192,1194,1197,1199,1199,1203,1206,1206,
	1208,1208,1210,1215,1218,1218,1219,1219,1221,1223,
	1228,1231,1234,1236,1237,1241,1243,1247,1247,1248,
	1250,1252,1255,1257,1259,1260,1264,1266,1270,1274,
	1274,1274,1278,1281,1283,1286,1290,1293,1293,1293,
	1300,1304,1305,1306,1309,1313,1315,1318,1320,1323,
	1325,1325,1330,1334,1336,1339,1342,1349,1352,1359};

      static const int ifin3[500]={ 
	2,   6,   9,   9,  13,  14,  15,  16,  18,  21,
        22,  25,  27,  30,  34,  39,  43,  46,  47,  50,
        54,  55,  58,  60,  61,  65,  66,  68,  73,  75,
        76,  80,  83,  89,  91,  92,  93,  94,  96,  98,
	102, 104, 104, 104, 108, 110, 113, 116, 117, 118,
	123, 126, 128, 130, 136, 137, 139, 140, 143, 144,
	146, 148, 150, 151, 153, 156, 160, 164, 167, 168,
	171, 173, 173, 175, 180, 181, 183, 185, 186, 186,
	192, 196, 196, 199, 201, 205, 209, 214, 218, 220,
	221, 221, 223, 226, 230, 231, 232, 232, 235, 235,
	238, 242, 245, 248, 250, 252, 254, 255, 256, 257,
	257, 264, 265, 266, 268, 268, 268, 273, 276, 280,
	284, 287, 292, 293, 296, 299, 301, 305, 305, 308,
	308, 311, 314, 317, 320, 323, 324, 328, 328, 334,
	336, 339, 342, 344, 345, 351, 353, 356, 356, 356,
	359, 360, 364, 367, 369, 369, 370, 370, 372, 377,
	382, 383, 383, 385, 388, 391, 394, 394, 396, 398,
	401, 402, 405, 407, 407, 410, 410, 413, 422, 426,
	428, 430, 431, 435, 440, 441, 444, 447, 450, 452,
	455, 459, 459, 463, 463, 463, 468, 471, 473, 474,
	476, 479, 479, 485, 488, 490, 493, 494, 498, 500,
	501, 502, 502, 507, 508, 514, 514, 515, 520, 522,
	522, 525, 529, 530, 530, 534, 537, 540, 544, 547,
	548, 554, 557, 568, 576, 579, 581, 585, 587, 593,
	599, 600, 602, 604, 608, 611, 614, 618, 619, 621,
	622, 627, 631, 634, 636, 638, 641, 642, 645, 648,
	650, 652, 652, 653, 657, 664, 666, 666, 666, 672,
	673, 679, 681, 684, 684, 685, 689, 691, 694, 697,
	699, 701, 705, 707, 709, 710, 715, 716, 717, 721,
	723, 724, 727, 729, 733, 735, 739, 741, 745, 747,
	749, 753, 755, 755, 757, 758, 764, 765, 771, 773,
	773, 777, 778, 780, 783, 785, 787, 788, 794, 799,
	805, 809, 810, 815, 818, 840, 854, 858, 860, 862,
	866, 870, 870, 876, 876, 881, 884, 886, 892, 894,
	898, 899, 900, 905, 906, 910, 912, 912, 914, 917,
	920, 925, 926, 930, 931, 931, 935, 937, 940, 940,
	943, 944, 947, 953, 959, 960, 960, 962, 966, 967,
	969, 969, 971, 975, 978, 982, 986, 989, 990, 993,
	996, 998,1002,1003,1005,1008,1011,1013,1019,1022,
	1022,1023,1025,1028,1030,1034,1037,1040,1041,1044,
	1049,1052,1055,1055,1056,1058,1062,1068,1071,1073,
	1075,1079,1088,1097,1104,1112,1119,1129,1156,1161,
	1162,1162,1167,1171,1173,1180,1182,1183,1183,1185,
	1189,1192,1194,1197,1199,1199,1203,1206,1206,1208,
	1208,1210,1215,1218,1218,1219,1219,1221,1223,1228,
	1231,1234,1236,1237,1241,1243,1247,1247,1248,1250,
	1252,1255,1257,1259,1260,1264,1266,1270,1274,1274,
	1274,1278,1281,1283,1286,1290,1293,1293,1293,1300,
	1304,1305,1306,1309,1313,1315,1318,1320,1323,1325,
	1325,1330,1334,1336,1339,1342,1349,1352,1359,1363};

      static const int ini3[500]={
	2,   4,   8,   9,  11,  13,  14,  16,  17,  20,
        21,  24,  27,  30,  32,  37,  41,  45,  47,  48,
        50,  55,  56,  59,  61,  63,  65,  67,  71,  74,
        75,  78,  82,  86,  90,  91,  93,  94,  95,  96,
	100, 102, 104, 104, 106, 108, 112, 114, 117, 118,
	121, 124, 126, 128, 133, 137, 138, 139, 141, 144,
	146, 148, 148, 151, 151, 156, 160, 162, 166, 167,
	171, 171, 173, 174, 178, 181, 182, 185, 185, 186,
	189, 193, 196, 197, 200, 203, 207, 209, 215, 220,
	220, 221, 222, 224, 227, 230, 231, 232, 232, 235,
	236, 239, 242, 248, 249, 250, 253, 254, 256, 256,
	257, 258, 264, 265, 266, 268, 268, 272, 276, 278,
	282, 287, 289, 292, 293, 299, 299, 304, 305, 308,
	308, 309, 313, 315, 319, 322, 323, 324, 328, 332,
	335, 337, 341, 342, 345, 348, 351, 354, 356, 356,
	359, 360, 360, 366, 369, 369, 369, 370, 372, 374,
	381, 382, 383, 385, 387, 389, 393, 394, 395, 397,
	399, 401, 404, 405, 407, 407, 410, 411, 418, 425,
	426, 430, 431, 433, 438, 441, 442, 445, 448, 450,
	452, 459, 459, 461, 463, 463, 465, 469, 471, 474,
	474, 479, 479, 482, 486, 489, 493, 493, 496, 498,
	500, 501, 502, 504, 508, 509, 514, 514, 518, 522,
	522, 525, 529, 529, 530, 532, 535, 538, 544, 546,
	548, 552, 555, 564, 571, 578, 581, 583, 586, 591,
	597, 600, 601, 603, 607, 610, 614, 617, 619, 620,
	621, 623, 627, 633, 635, 637, 638, 641, 643, 648,
	648, 651, 652, 653, 655, 658, 665, 666, 666, 668,
	673, 676, 681, 682, 684, 685, 687, 690, 693, 697,
	698, 699, 703, 705, 709, 710, 713, 715, 716, 720,
	723, 724, 726, 728, 733, 734, 739, 741, 741, 747,
	748, 750, 753, 755, 755, 757, 760, 764, 767, 772,
	773, 776, 777, 778, 783, 785, 786, 788, 791, 797,
	801, 809, 810, 812, 817, 828, 853, 858, 860, 861,
	866, 869, 870, 875, 876, 878, 883, 884, 888, 892,
	896, 899, 900, 902, 905, 908, 910, 912, 914, 915,
	919, 922, 926, 927, 931, 931, 934, 935, 939, 940,
	941, 944, 945, 949, 956, 960, 960, 961, 963, 967,
	969, 969, 970, 973, 976, 980, 986, 989, 990, 992,
	994, 996,1000,1002,1004,1005,1010,1013,1016,1019,
	1022,1022,1024,1028,1028,1031,1036,1038,1041,1042,
	1045,1050,1055,1055,1056,1056,1060,1067,1069,1071,
	1075,1076,1079,1093,1102,1109,1115,1122,1137,1159,
	1162,1162,1165,1169,1172,1178,1180,1183,1183,1183,
	1187,1190,1193,1196,1197,1199,1202,1205,1206,1208,
	1208,1209,1211,1215,1218,1218,1219,1221,1223,1224,
	1231,1233,1234,1236,1239,1243,1247,1247,1247,1248,
	1251,1252,1255,1257,1260,1264,1264,1270,1272,1274,
	1274,1275,1278,1281,1285,1286,1292,1293,1293,1295,
	1301,1305,1305,1309,1311,1315,1317,1319,1323,1324,
	1325,1325,1332,1336,1338,1339,1347,1351,1354,1360};

    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.53;  //Debyes
    static const double mmol=49.0;

    double q=1.344455353*pow(tt.get("K"),1.5);
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
            
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
        
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      if(ifin==0||ifin<ini){

	return complex<double> (0.0,0.0);

      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){

	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening(Frequency(fre[i],"GHz"),tt,pp,mmol,Frequency(2.5,"MHz"),0.76),Frequency(0.0,"GHz"));

	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*fre[i]; 

          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(0.047992745509/tt.get("K"))*(fac2fixed*pow(mu,2.0)/q);  // imaginary part: absorption coefficient in cm^2
	                                                                                                  // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )

	
      }
     
    }

  }

  complex<double>  RefractiveIndexProfile::mkAbs_o3_161816(Temperature tt, Pressure pp, Frequency nu){

    static const double fre[755]={
      3.83292,  4.65377,  6.96684,  8.18944, 10.01083, 14.86876,
      16.35579, 16.66993, 17.65795, 18.76805, 19.10049, 22.20599,
      23.42240, 25.11912, 25.28206, 25.97734, 26.04029, 29.22771,
      29.88932, 30.91416, 31.70964, 32.82510, 32.83978, 33.19019,
      34.81487, 34.96913, 35.14310, 35.28012, 36.98948, 44.62285,
      46.36781, 46.44679, 48.80693, 49.69103, 49.73307, 51.04926,
      51.20879, 53.21264, 55.63426, 56.18237, 60.13629, 60.81826,
      62.47732, 64.42406, 65.42776, 67.12047, 67.50656, 69.39121,
      73.58799, 74.74345, 75.11129, 76.26604, 78.67523, 79.49845,
      81.08149, 83.95127, 84.83387, 85.14570, 86.41835, 86.96452,
      87.39463, 88.55293, 91.01000, 94.48388, 97.90016, 99.82872,
      101.03610,102.21188,102.65862,104.35540,105.77757,107.38846,
      110.37325,112.38954,112.61345,112.66861,113.05508,114.43400,
      116.40134,116.75312,118.85573,120.91444,122.72406,123.29053,
      126.32539,128.96342,133.10971,133.43143,136.83712,137.07259,
      138.79255,142.05328,143.14371,144.77817,145.45266,146.44470,
      149.85713,150.35073,152.52462,152.99859,155.57508,156.49877,
      158.46862,158.51873,158.64756,159.25580,159.82491,164.03247,
      164.73359,164.90625,167.12339,169.91475,171.03352,171.14418,
      171.49232,172.81548,173.20336,177.77700,178.73046,186.92652,
      187.87893,188.89628,191.80608,192.68817,195.89534,197.14164,
      197.62817,198.61779,199.54910,203.72888,205.67110,205.85097,
      206.27231,212.63955,213.29923,213.53850,216.06545,217.50672,
      219.32561,223.00496,223.39665,224.76185,224.81267,225.85068,
      225.92993,226.33694,230.17465,231.27666,233.02210,234.34000,
      236.60412,236.87640,238.71616,239.96505,240.00467,240.81710,
      241.50344,242.99812,246.29104,249.07765,249.57444,250.20690,
      250.51788,255.22420,255.82478,256.67860,258.33642,260.03970,
      260.32286,260.40244,263.10906,265.62341,266.28459,270.47669,
      272.37055,274.14843,275.68242,278.11561,278.39212,278.45228,
      279.25255,281.19315,281.98575,282.98466,284.01884,284.29217,
      285.94293,288.15370,289.12425,292.49157,292.69421,295.31851,
      298.51724,301.71864,303.33763,304.82222,304.88585,307.47498,
      309.23460,309.28475,310.44523,312.55129,312.90127,315.91555,
      316.45163,317.75446,318.84800,324.87483,325.67949,325.95658,
      327.97217,328.52950,329.27763,329.52600,331.05490,331.79643,
      333.95897,335.13562,335.44920,336.17226,336.20044,341.55239,
      343.87600,345.02728,345.51899,347.79544,349.84917,349.94988,
      353.31769,353.90940,358.45988,361.57683,362.19394,362.30451,
      363.08692,363.23870,363.76981,369.27392,370.00690,371.04433,
      373.49566,375.47774,375.54191,377.59078,377.87362,379.90755,
      383.40365,384.85564,386.30918,387.60494,387.86222,390.34108,
      391.48763,392.77610,394.97120,397.03735,397.95595,400.62942,
      403.58905,405.64510,405.72190,405.91167,407.60362,408.45488,
      411.58254,413.58354,413.61499,415.65825,416.80280,416.87472,
      417.34226,420.21523,420.35432,422.46480,422.61307,422.91977,
      425.93070,426.84736,429.02889,429.53428,429.90988,430.18675,
      430.38023,430.96072,430.96297,431.52580,432.24357,433.23994,
      434.59257,435.17843,438.41843,439.18372,439.48427,439.66855,
      439.92899,444.14141,444.57753,445.80585,446.86354,448.75772,
      451.08893,451.32667,451.67143,452.16743,453.10418,454.91506,
      455.41838,456.02127,457.21469,459.89060,462.86401,464.74901,
      465.33235,468.94947,471.11983,471.67118,471.93272,474.51420,
      475.10504,476.25304,476.53976,479.25665,480.18144,480.92470,
      481.02619,481.25150,481.88799,482.86516,486.30544,487.97260,
      490.02427,490.21094,491.11631,493.42821,494.93747,495.48512,
      496.42341,500.22050,500.65098,505.41688,507.03113,508.43688,
      513.67627,514.33022,515.63956,516.84921,517.44907,519.57521,
      520.75559,521.66152,522.10516,522.37042,525.86278,526.94989,
      530.57062,532.76320,532.99433,533.96301,534.02473,537.33757,
      541.00365,541.94707,542.42052,542.52755,546.87243,547.71140,
      547.95870,549.72559,550.41192,551.45616,552.31934,555.40426,
      556.52856,557.93083,558.44783,558.99983,559.43073,562.10658,
      564.37106,565.94533,566.32968,567.90875,568.15856,569.05963,
      572.04795,572.68115,573.62373,575.78660,576.99703,580.39350,
      581.01700,582.96753,584.08887,584.85480,585.67564,589.24596,
      590.24184,591.61144,592.24582,593.74229,594.64527,594.75797,
      595.09359,597.20014,598.14685,599.24980,599.95341,600.09191,
      600.23554,600.33302,600.72735,600.94867,601.06949,601.12041,
      601.42871,601.44900,601.62611,601.66946,601.72570,601.74742,
      602.46741,602.49328,602.96104,603.50217,604.76765,606.31913,
      607.28923,609.68760,609.92484,610.23327,610.50070,611.56931,
      615.48367,616.85978,617.68805,619.12658,619.28325,622.33177,
      624.83837,627.94546,627.94835,629.36940,631.95965,633.86045,
      634.39329,634.43876,635.24684,635.57353,642.07626,642.52221,
      642.61675,642.65821,644.78296,647.51211,647.69069,650.39144,
      651.49095,652.81863,653.35839,653.50852,654.39692,656.07946,
      659.11013,660.76831,660.87739,661.04778,664.00348,665.69527,
      667.26653,668.46057,670.24465,672.17350,673.89715,675.91056,
      678.73410,679.00108,683.82312,686.14941,686.25431,689.74875,
      691.84583,692.43390,693.39555,695.67034,698.68691,699.30180,
      701.39751,702.13130,702.34477,702.48820,704.07605,706.26241,
      707.49624,709.76873,710.23525,711.39242,711.70668,711.79951,
      713.26094,715.67344,717.58224,720.66197,720.79252,721.06265,
      721.16860,726.42670,726.85383,727.22008,728.02089,729.09961,
      729.38670,735.42306,736.60983,737.12743,737.19574,739.42523,
      740.67487,741.11239,741.46323,742.71479,743.05573,744.96105,
      746.42369,750.21693,752.28113,752.30623,754.17066,754.66886,
      757.22144,757.90534,759.13298,760.51590,761.21131,761.41386,
      761.80451,762.18538,762.19844,762.25674,762.41079,762.51836,
      763.86965,764.74950,765.10416,765.52014,765.81702,766.52539,
      766.54990,767.15948,767.83460,768.29860,768.65076,769.32555,
      769.91571,770.00305,770.92372,771.15243,771.67935,771.75575,
      771.90757,772.21012,772.38477,772.55600,772.66899,772.76102,
      772.82340,772.86753,772.89555,772.91231,774.07319,776.31917,
      777.36488,777.68091,779.92497,780.39530,785.35574,785.66551,
      786.09590,786.97931,787.88133,790.31998,791.16450,791.91368,
      792.23247,792.85473,795.61897,797.06088,798.96001,802.44569,
      803.05468,805.15748,807.49403,807.55209,812.13684,813.21804,
      815.05421,815.13775,815.74654,817.27849,817.31488,817.37136,
      817.47005,818.38029,821.66406,822.34113,827.39998,828.12169,
      828.40385,828.88785,829.05267,830.36868,832.03569,834.76605,
      837.27954,837.74457,838.53019,842.62098,842.96858,845.39140,
      847.59843,849.79458,851.78627,852.52821,853.72984,854.43234,
      855.56074,856.43991,862.40968,863.81941,864.67406,866.47943,
      866.53082,867.37663,867.94232,869.50051,871.68406,872.83467,
      873.83971,875.18077,877.19425,878.47998,879.03398,880.73574,
      881.16448,882.46587,884.57198,887.52937,889.08728,893.24362,
      894.43629,896.67266,898.05161,898.38497,900.40603,901.19854,
      902.55658,904.31761,904.89231,905.86123,905.89562,906.44547,
      907.80704,911.73914,912.64052,914.16299,914.18098,914.30076,
      914.33530,915.18675,915.45812,915.92272,916.43023,917.52807,
      917.67791,918.52599,919.44787,920.47375,923.06617,923.25098,
      923.47608,923.54971,923.62232,924.96025,926.53233,926.66934,
      927.29475,928.16050,929.58199,929.65680,931.04223,931.54989,
      931.70958,932.04296,932.07041,932.39291,933.28634,934.80766,
      935.07157,936.87075,937.37219,938.58165,939.12825,939.56495,
      939.96039,940.45714,941.03969,941.44865,941.85891,942.17315,
      942.45952,942.68728,942.88216,943.03783,943.16469,943.26405,
      943.34120,943.39912,943.44137,943.47083,943.49025,943.50199,
      943.79047,947.87800,948.43442,948.53483,948.56294,952.12116,
      952.58316,952.96844,954.82836,955.21079,956.17730,956.95770,
      964.75002,964.77764,965.65016,966.57334,969.03812,971.45067,
      973.60348,973.64299,975.08323,978.60769,980.05791,981.30244,
      981.73115,982.19069,984.42432,989.97471,990.96917,994.26992,
	994.48807,996.00256,998.71200,998.75965,999.58274};

    static const double flin[755]={
      .130E+01, .273E+01, .618E+01, .719E+01, .601E+01, .281E+01,
      .503E+01, .367E+01, .513E+01, .161E+01, .401E+01, .113E+01,
      .972E+00, .818E+01, .738E+01, .164E+01, .373E+01, .149E+01,
      .257E+01, .124E+01, .700E+01, .140E+01, .276E+01, .479E+01,
      .801E+00, .156E+01, .514E+00, .216E+01, .107E+01, .464E+01,
      .755E+01, .580E+01, .126E+01, .409E+01, .639E+01, .799E+01,
      .232E+01, .266E+01, .186E+01, .645E+00, .652E+01, .422E+01,
      .680E+01, .245E+01, .217E+01, .544E+01, .562E+01, .231E+01,
      .339E+01, .115E+01, .283E+01, .194E+01, .325E+01, .779E+01,
      .541E+01, .775E+01, .441E+01, .334E+01, .895E+00, .517E+00,
      .661E+01, .248E+01, .907E+00, .431E+01, .432E+01, .541E+01,
      .790E+01, .294E+01, .423E+01, .589E+01, .760E+01, .902E+00,
      .100E+01, .675E+01, .197E+01, .684E+01, .185E+01, .568E+01,
      .641E+01, .230E+01, .712E+01, .453E+01, .300E+01, .522E+01,
      .417E+00, .431E+01, .422E+01, .739E+01, .401E+01, .282E+01,
      .793E+01, .620E+01, .756E+00, .809E+01, .554E+00, .270E+01,
      .394E+00, .578E+01, .417E+01, .501E+01, .822E+01, .201E+01,
      .381E+01, .158E+01, .585E+01, .394E+01, .719E+01, .711E+01,
      .313E+01, .833E+01, .391E+01, .600E+01, .139E+01, .384E+01,
      .355E+01, .259E+01, .341E+00, .480E+01, .709E+01, .699E+01,
      .163E+00, .359E+01, .582E+01, .238E+01, .579E+01, .499E+01,
      .839E+01, .593E+01, .308E+01, .797E+01, .651E+00, .459E+01,
      .115E+01, .338E+01, .678E+01, .733E+01, .112E+02, .132E+02,
      .909E+01, .558E+01, .216E+01, .149E+02, .616E+01, .937E+00,
      .703E+01, .285E+00, .777E+01, .437E+01, .590E+01, .517E+01,
      .463E+01, .824E+01, .162E+02, .428E+01, .657E+01, .316E+01,
      .491E+01, .357E+01, .194E+01, .536E+01, .742E+01, .760E+01,
      .218E+01, .713E+00, .857E+00, .756E+01, .416E+01, .169E+02,
      .749E+01, .575E+01, .145E+01, .294E+01, .636E+01, .250E+01,
      .571E+00, .171E+01, .515E+01, .551E+01, .503E+00, .563E+01,
      .546E+01, .341E+01, .799E+01, .734E+01, .394E+01, .244E+00,
      .458E+01, .506E+01, .171E+02, .272E+01, .614E+01, .422E+01,
      .149E+01, .493E+01, .756E+01, .656E+01, .302E+00, .954E+01,
      .149E+01, .713E+01, .372E+01, .617E+01, .491E+01, .718E+01,
      .545E+01, .250E+01, .593E+01, .127E+01, .775E+01, .169E+02,
      .472E+01, .224E+02, .203E+02, .126E+00, .167E+01, .775E+01,
      .549E+01, .242E+02, .692E+01, .350E+01, .179E+02, .511E+00,
      .227E+01, .571E+01, .214E+00, .155E+02, .105E+01, .253E+02,
      .890E+01, .450E+01, .596E+01, .670E+01, .327E+01, .132E+02,
      .116E+02, .612E+01, .177E+01, .205E+01, .165E+02, .549E+01,
      .259E+02, .836E+00, .212E+01, .769E+01, .112E+02, .428E+01,
      .764E+01, .755E+01, .634E+01, .649E+01, .305E+01, .700E+01,
      .108E+02, .940E+01, .183E+01, .527E+01, .643E+01, .628E+00,
      .748E+01, .786E+01, .405E+01, .259E+02, .809E+01, .191E+00,
      .466E+00, .627E+01, .283E+01, .650E+01, .263E+01, .137E+02,
      .662E+01, .160E+02, .160E+01, .204E+01, .524E+01, .505E+01,
      .432E+00, .404E+01, .285E+01, .726E+01, .160E+01, .885E+00,
      .224E+01, .128E+02, .344E+01, .383E+01, .459E+01, .318E+02,
      .572E+01, .295E+02, .684E+01, .261E+01, .605E+01, .743E+01,
      .335E+02, .793E+01, .269E+02, .139E+01, .255E+02, .483E+01,
      .252E+00, .683E+01, .684E+01, .898E+01, .932E+01, .322E+01,
      .704E+01, .345E+02, .361E+01, .242E+02, .996E+01, .238E+01,
      .583E+01, .157E+02, .117E+01, .174E+00, .148E+02, .461E+01,
      .156E+02, .101E+00, .109E+02, .216E+02, .759E+01, .752E+01,
      .431E+00, .682E+01, .349E+02, .338E+01, .218E+01, .698E+01,
      .387E+01, .216E+01, .561E+01, .117E+02, .735E+01, .248E+02,
      .959E+00, .439E+01, .192E+02, .249E+01, .661E+01, .316E+01,
      .123E+02, .168E+02, .194E+01, .539E+01, .347E+02, .178E+02,
      .112E+02, .754E+00, .171E+02, .463E+01, .416E+01, .709E+01,
      .261E+01, .638E+01, .153E+02, .294E+01, .161E+00, .129E+02,
      .172E+01, .153E+02, .772E+01, .516E+01, .559E+00, .361E+02,
      .738E+01, .732E+01, .242E+02, .394E+01, .551E+01, .277E+01,
      .218E+01, .189E+02, .616E+01, .137E+02, .272E+01, .897E+01,
      .134E+02, .332E+02, .150E+01, .717E+01, .494E+01, .199E+02,
      .378E+00, .122E+02, .716E+01, .814E+01, .372E+01, .654E+01,
      .294E+01, .109E+02, .594E+01, .249E+01, .132E+02, .303E+02,
      .959E+01, .129E+01, .150E+02, .472E+01, .138E+02, .151E+00,
      .836E+01, .216E+00, .716E+01, .694E+01, .599E+01, .893E+01,
      .101E+02, .774E+01, .657E+01, .481E+01, .113E+02, .540E+01,
      .422E+01, .362E+01, .300E+01, .236E+01, .167E+01, .910E+00,
      .209E+02, .349E+01, .125E+02, .731E+01, .313E+01, .137E+02,
      .776E+01, .572E+01, .236E+02, .227E+01, .722E+01, .149E+02,
      .276E+02, .108E+01, .219E+02, .160E+02, .450E+01, .841E-01,
      .672E+01, .327E+01, .140E+02, .171E+02, .331E+01, .918E+01,
      .878E+01, .204E+01, .550E+01, .205E+01, .880E+00, .252E+02,
      .180E+02, .106E+02, .427E+01, .153E+02, .230E+02, .650E+01,
      .149E+02, .352E+01, .305E+01, .142E+00, .726E+01, .731E+01,
      .189E+02, .527E+01, .184E+01, .108E+02, .142E+02, .240E+02,
      .687E+00, .231E+02, .405E+01, .231E+02, .890E+01, .627E+01,
      .283E+01, .196E+02, .366E+01, .162E+01, .505E+01, .126E+02,
      .213E+02, .504E+00, .250E+02, .383E+01, .390E+01, .728E+01,
      .605E+01, .349E+01, .202E+02, .144E+02, .261E+01, .175E+02,
      .731E+01, .148E+02, .134E+00, .141E+01, .483E+01, .196E+02,
      .260E+02, .705E+01, .336E+00, .146E+02, .124E+02, .360E+01,
      .181E+01, .100E+02, .583E+01, .358E+01, .181E+02, .208E+02,
      .239E+01, .228E+02, .121E+01, .460E+01, .398E+01, .270E+02,
      .167E+02, .683E+01, .427E+01, .189E+00, .145E+02, .730E+01,
      .338E+01, .153E+02, .561E+01, .372E+01, .166E+02, .217E+01,
      .140E+02, .732E+01, .212E+02, .280E+02, .185E+02, .172E+02,
      .101E+01, .197E+02, .197E+02, .127E+02, .159E+02, .438E+01,
      .146E+02, .210E+02, .127E+00, .133E+02, .115E+02, .661E+01,
      .148E+02, .121E+02, .721E-01, .103E+02, .109E+02, .222E+02,
      .971E+01, .912E+01, .853E+01, .794E+01, .735E+01, .316E+01,
      .677E+01, .618E+01, .559E+01, .499E+01, .438E+01, .376E+01,
      .312E+01, .244E+01, .172E+01, .926E+00, .983E+01, .234E+02,
      .388E+01, .538E+01, .196E+01, .465E+01, .146E+02, .291E+02,
      .245E+02, .813E+00, .416E+01, .187E+02, .732E+01, .638E+01,
      .215E+02, .422E+01, .144E+02, .294E+01, .255E+02, .406E+01,
      .516E+01, .174E+01, .733E+01, .301E+02, .630E+00, .394E+01,
      .505E+01, .264E+02, .218E+02, .616E+01, .115E+02, .291E+02,
      .153E+01, .121E+00, .147E+02, .272E+01, .424E+01, .217E+02,
      .494E+01, .208E+02, .146E+02, .153E+01, .311E+02, .271E+02,
      .458E+00, .733E+01, .371E+01, .594E+01, .273E+02, .549E+01,
      .250E+01, .419E-04, .436E+01, .443E+01, .471E+01, .321E+02,
      .133E+01, .734E+01, .303E+00, .349E+01, .256E+02, .219E+02,
      .165E+02, .239E+02, .571E+01, .229E+02, .601E+01, .229E+01,
      .147E+02, .147E+02, .462E+01, .331E+02, .449E+01, .113E+01,
      .671E+01, .240E+02, .734E+01, .168E+00, .327E+01, .549E+01,
      .663E+01, .226E+02, .207E+01, .449E+01, .701E-03, .341E+02,
      .481E+01, .427E+01, .734E+01, .132E+02, .940E+00, .649E+01,
      .212E+02, .250E+02, .315E-01, .258E+02, .272E+02, .739E+01,
      .305E+01, .438E+01, .245E+02, .285E+02, .198E+02, .260E+02,
      .231E+02, .527E+01, .147E+02, .218E+02, .185E+02, .186E+01,
      .457E+01, .204E+02, .125E+01, .351E+02, .500E+01, .191E+02,
      .148E+02, .172E+02, .405E+01, .178E+02, .756E+00, .735E+01,
      .627E+01, .830E+01, .159E+02, .165E+02, .187E+02, .153E+02,
      .147E+02, .141E+02, .135E+02, .129E+02, .123E+02, .284E+01,
      .117E+02, .111E+02, .105E+02, .990E+01, .931E+01, .872E+01,
      .814E+01, .755E+01, .696E+01, .636E+01, .576E+01, .515E+01,
      .452E+01, .388E+01, .321E+01, .251E+01, .176E+01, .938E+00,
      .504E+01, .361E+02, .166E+01, .940E+01, .469E+01, .370E-02,
      .518E+01, .735E+01, .383E+01, .271E+02, .291E+00, .604E+01,
      .107E+02, .262E+01, .147E+02, .281E+02, .482E+01, .371E+02,
      .146E+01, .483E+01, .536E+01, .735E+01, .361E+01, .210E+00,
      .122E+02, .582E+01, .427E+01, .241E+01, .152E+02, .460E+01,
      .381E+02, .210E+02, .499E+01, .126E+01, .292E+02};

    static const double el[755]={
      682.516,  243.976,  951.665, 1290.007,  907.719,  181.886,
      627.914,  343.899,  664.252,   11.099,  428.283,  143.333,
      181.475, 1678.993, 1342.286, 1386.052,  370.149,   48.796,
      199.964,  109.815, 1238.177, 1501.723,  291.423,  591.306,
      224.128,  604.720,    3.612,   70.605,  763.623,  556.957,
      1395.373,  863.669,   80.856,  399.113,  996.311, 1619.948,
      144.440,  343.944,  531.828,  271.301, 1042.311,  490.079,
      1187.299,  401.506,  464.157,  740.476,  821.657,  160.567,
      315.865,   56.510,   21.592, 1273.737,  291.421, 1561.881,
      701.605, 1449.668,  522.425,  221.710,  849.049,  323.023,
      1137.802,    3.647,   30.228,   12.062,  556.898,  780.205,
      1505.247,   95.890,  490.169,   25.187, 1505.219,   36.742,
      0.000, 1089.402,  125.791, 1137.788,  111.270,  821.603,
      1089.409, 1166.292,   43.091,  458.292,  266.405,  740.478,
      379.232,  628.705,   36.706, 1449.691,  458.327,  243.916,
      65.633, 1042.277,  938.924, 1561.946,   21.613, 1063.966,
      16.602,  780.278,  705.574,  701.651, 1619.927,    3.583,
      428.288,   95.886,  907.680,  266.350, 1395.323, 1238.221,
      966.687,   92.879,  787.473,  996.338,   83.185,  125.879,
      874.463,  221.699,  439.980,  664.261, 1187.215, 1342.221,
      11.076,  399.027,   56.487,  201.445,  951.691,  522.431,
      124.607,  998.760,   12.015, 1739.214, 1033.221,  627.912,
      70.692,  371.386, 1290.355, 1343.443,  134.022,  172.152,
      100.765,  908.188,  181.870,  215.004,  863.702,   60.052,
      72.348,  505.228, 1680.244,  592.891, 1094.707,   48.844,
      315.835,  160.822,  262.534,   25.174, 1239.672,  344.733,
      160.600,   30.228,  163.870,  865.936, 1290.012,   80.882,
      16.567,   50.241,    7.812, 1622.371,  559.091,  314.709,
      1453.696, 1195.792,   11.162,  319.341, 1190.209,   21.608,
      1132.117,  146.832,  824.858,  591.341,   41.828,   43.125,
      1301.987,   36.693,  201.455, 1565.707,  526.458,  574.945,
      1529.665, 1413.259,  371.412,  295.151, 1141.941,   56.515,
      131.261,  785.007, 1568.830,  951.958,   34.555,  109.831,
      5.246, 1510.244,  495.088,  199.987,   80.870,   65.626,
      370.100,  272.237, 1094.985,  116.695, 1397.557,  432.643,
      746.446,  516.120,  448.284,   28.572,    7.898,  246.663,
      109.759,  588.602, 1456.087,  464.853,  385.351, 1235.394,
      250.483, 1049.127,  649.204,  327.194,  103.496,  665.691,
      92.829,  709.001,  143.382, 1403.032,  435.903,  273.997,
      143.360,  665.052,   11.127,  229.963,  498.327, 1004.478,
      747.460,   91.365,   16.562, 1807.963,  225.750,  672.752,
      244.184,  296.214,  181.444, 1351.182,  408.170, 1045.151,
      124.596,  182.534,  210.728,  961.029,  429.185,   80.556,
      1749.014,  144.416,  637.812,  833.703, 1510.021,  727.881,
      1343.130, 1300.634,  381.633,  111.235,   30.180,  181.476,
      224.154,  568.416,  192.609,   21.608,   83.201,  918.886,
      70.841,   60.097,   41.892, 1691.268,   28.550,   23.737,
      34.527,  160.800,   50.199,  603.972,   70.638, 1043.090,
      95.825,  945.697,  125.841,  356.205, 1251.190,  350.381,
      1145.169,  160.566,  853.146,  175.687,  924.336,  877.845,
      62.433,  271.321,  743.705,  199.981,  292.936,   48.849,
      1634.625, 1251.796,  571.439,  765.410,  244.167,  332.077,
      1203.049,  224.075,  160.071,  811.044,  201.432,  838.108,
      642.979,   55.224,  293.010,  682.480,  493.035, 1143.200,
      1455.395, 1579.284, 1362.953,  540.106,   36.741,  323.061,
      72.363,  309.254, 1156.010,  346.399,  408.927, 1019.300,
      145.555,  799.473,  604.652,   20.123, 1525.146,  509.978,
      404.526,  246.595,  287.532, 1110.274, 1478.445,  271.343,
      346.486,  132.343,  531.822,  100.709,  762.141,  379.289,
      23.715, 1472.110,  721.899,  480.952,  898.649,  467.190,
      267.014,  464.069,  827.146, 1065.641,  120.232, 1503.292,
      1885.080,  471.895, 1118.762,  726.012,  134.070,   28.522,
      56.468,  296.201, 1420.376,  401.449,  453.229,  561.600,
      534.491, 1386.016,  247.697,  440.022, 1022.310,  323.066,
      109.423,  343.892, 1826.147, 1246.043,  690.985,  172.122,
      34.592,  291.398, 1369.844,  426.708,  404.568, 1273.697,
      243.949,  229.583,  805.253,  980.081,  606.337,  990.732,
      201.416,   99.715,  163.869, 1768.416,  131.182,  221.676,
      266.369,  181.864,  146.845,  103.434,  315.829,  116.726,
      91.411,   80.510,   70.902,   62.400,   55.197,   49.196,
      350.362,  657.260,  370.138,  539.412,   41.851,  429.177,
      215.030, 1320.415, 1222.503,  401.389,  505.176,  493.024,
      1166.236,  212.770,  379.230,  561.561,  939.154,   91.307,
      1711.887,  624.738,  682.738,  634.869,   50.245,  262.554,
      915.328,   80.826, 1272.287,  377.271,  197.059, 1063.938,
      712.833,  634.931,  899.429,  467.198,  408.889, 1656.560,
      893.007,   60.043,  593.417, 1087.410,  574.967,  611.386,
      795.441, 1225.361,  354.456,  314.748,  763.606,  440.025,
      182.649,  966.592,  860.806, 1330.613, 1353.731, 1602.334,
      563.298,  882.685,   70.654,  332.742, 1179.637,  371.469,
      874.468,  169.340,  471.894,  823.485,   83.140,  649.110,
      1549.410,   44.374,  974.564,  848.957,  534.381,  534.476,
      687.917,  985.208, 1188.385,  312.230, 1135.014,  787.410,
      505.240, 1970.224,  157.332,  432.684,  712.878,  787.365,
      109.760, 1008.207, 1497.687,   49.169,  705.531, 1070.879,
      506.665, 1443.175,  292.918, 1091.693,   95.790,  539.483,
      628.723, 1911.302,  111.285,  146.525,  938.909,  727.817,
      752.447,  556.865, 1447.166,   55.164,  498.275,  480.151,
      490.128,  768.795, 1171.836,  574.970,  743.637,  665.127,
      274.908,  827.090,  606.289,  428.286,  591.279, 1049.574,
      522.409,  915.267, 1293.950,  458.330,  371.315, 1853.681,
      1081.980,  399.051,  136.818,  319.396,  344.679, 1008.247,
      295.118,  272.214,  250.470,  229.959,  210.634,  718.730,
      192.623,  175.708,  160.000,  145.592,  132.286,  120.282,
      109.379,   99.777,   91.275,   84.075, 1466.119, 1105.911,
      62.461, 1397.845,  454.838,  144.415, 1033.277,  611.462,
      1208.341,  257.999, 1008.656,  568.438,  810.998, 1797.162,
      1277.347,  125.817,  795.484,  686.215, 1315.323,   70.856,
      1349.727,  430.726,  854.213,  649.111,  242.390,  968.839,
      182.550, 1427.046,  682.717, 1741.843, 1105.942, 1529.639,
      143.334, 1403.891, 1183.033,  654.900,   80.557, 1387.323,
      1302.809,  642.886, 1132.078,  407.815,  687.935, 1543.303,
      227.883,  898.660,  930.323, 1687.726, 1413.209,  225.793,
      624.687,    3.582,  160.486,   91.350, 1257.093,  727.859,
      386.105,  944.162,  214.676,  893.008, 1301.967, 1501.780,
      882.678,  763.537, 1634.810,  721.835,  274.030,  595.875,
      1235.427, 1288.662,  103.466,  768.804, 1212.477,  365.695,
      2063.475, 1195.812,  990.811,  202.569,  856.894, 1583.095,
      327.137, 1094.730,  568.164,   78.048,   11.951,  811.013,
      116.647, 1169.163, 1038.535, 1208.289,  346.387, 2004.560,
      998.695,  805.206,  191.763, 1353.690, 1466.189,  385.283,
      821.982,  199.941, 1246.028, 1583.405,  907.681,  848.928,
      1143.221, 1532.580, 1343.136, 1045.187,  821.562,  541.653,
      84.042,  951.939,  181.435,  854.271,  131.196,  863.689,
      1398.659,  740.417, 1127.049,  780.246,  328.279, 1087.355,
      1946.947,  448.231,  664.230,  701.614,  974.471,  627.898,
      592.885,  558.999,  526.475,  495.017,  464.891,  788.270,
      435.851,  408.127,  381.599,  356.279,  332.059,  309.144,
      287.531,  267.020,  247.710,  229.603,  212.697,  197.092,
      182.588,  169.385,  157.283,  146.482,  136.881,  128.381,
      1483.267,  898.670,  516.344,  516.039,   91.337,   25.167,
      146.844, 1137.326, 1086.137,  893.018,  311.472, 1890.434,
      588.560,  755.759, 1455.417,  938.872, 1435.154,  944.138,
      492.235,   99.733,  163.864, 1188.394, 1046.425,  295.765,
      665.744, 1835.122,  244.115,  724.448, 1315.301, 1388.242,
      990.732, 1070.959,  109.429,  469.326,  985.287};

    static const int ifin1[500]={
      3,   4,   5,   6,   6,   7,   9,  11,  12,  13,
      14,  18,  18,  21,  24,  27,  30,  30,  30,  30,
      31,  33,  34,  36,  38,  39,  41,  41,  43,  44,
      45,  46,  48,  49,  49,  51,  53,  54,  55,  56,
      58,  61,  63,  63,  64,  65,  65,  66,  67,  70,
      71,  72,  73,  74,  77,  79,  81,  82,  83,  84,
      85,  86,  87,  87,  87,  89,  90,  92,  92,  93,
      95,  97,  97,  99, 101, 101, 103, 106, 108, 108,
      111, 111, 112, 113, 117, 118, 118, 120, 120, 120,
      120, 121, 123, 123, 125, 125, 126, 129, 130, 130,
      131, 134, 134, 134, 135, 137, 138, 139, 140, 140,
      144, 147, 147, 148, 149, 151, 153, 154, 157, 159,
      159, 160, 160, 164, 164, 164, 167, 168, 171, 171,
      172, 174, 174, 175, 176, 177, 178, 181, 182, 185,
      187, 188, 189, 190, 192, 192, 193, 194, 194, 195,
      198, 198, 199, 202, 204, 204, 206, 208, 208, 208,
      209, 211, 213, 215, 217, 218, 222, 222, 222, 223,
      224, 226, 227, 229, 229, 231, 231, 232, 232, 235,
      238, 238, 238, 240, 241, 242, 244, 246, 247, 247,
      249, 250, 252, 253, 255, 256, 256, 258, 259, 259,
      260, 263, 265, 265, 266, 268, 271, 272, 274, 277,
      277, 279, 279, 286, 288, 290, 291, 292, 296, 296,
      298, 300, 301, 301, 305, 307, 309, 310, 311, 312,
      313, 314, 315, 315, 318, 319, 322, 322, 325, 329,
      329, 330, 331, 333, 334, 336, 338, 338, 340, 340,
      340, 341, 343, 343, 343, 345, 347, 348, 350, 353,
      353, 355, 355, 356, 358, 360, 360, 361, 361, 365,
      365, 366, 368, 370, 372, 372, 374, 377, 378, 379,
      380, 382, 384, 385, 387, 388, 390, 390, 391, 393,
      395, 396, 396, 398, 400, 403, 404, 406, 413, 424,
      426, 427, 428, 432, 433, 433, 435, 436, 438, 439,
      440, 440, 442, 443, 444, 447, 449, 449, 449, 453,
      454, 454, 456, 457, 459, 462, 463, 463, 466, 467,
      468, 469, 471, 472, 473, 474, 475, 476, 477, 477,
      478, 480, 480, 481, 483, 484, 485, 486, 487, 491,
      492, 493, 494, 496, 499, 500, 501, 502, 504, 506,
      506, 508, 510, 512, 512, 512, 514, 516, 518, 521,
      523, 524, 524, 525, 527, 529, 529, 531, 533, 541,
      543, 548, 552, 556, 569, 570, 571, 573, 575, 575,
      575, 579, 580, 581, 585, 585, 586, 588, 588, 589,
      590, 591, 593, 593, 594, 595, 598, 603, 603, 605,
      605, 605, 609, 611, 612, 613, 613, 616, 616, 618,
      618, 619, 620, 621, 623, 625, 627, 627, 627, 628,
      630, 632, 634, 635, 637, 638, 639, 641, 643, 645,
      646, 646, 647, 648, 648, 650, 651, 653, 654, 656,
      658, 661, 662, 662, 664, 668, 672, 675, 677, 677,
      683, 685, 687, 689, 695, 697, 699, 701, 705, 712,
      722, 722, 726, 726, 729, 730, 733, 733, 733, 733,
      735, 737, 737, 738, 739, 741, 742, 743, 744, 747,
      748, 748, 748, 750, 750, 752, 753, 755, 755, 755};

      static const int ini1[500]={
	1,   1,   1,   1,   3,   4,   5,   6,   6,   7,
	9,  11,  12,  13,  14,  18,  18,  21,  24,  27,
        30,  30,  30,  30,  31,  33,  34,  36,  38,  39,
        41,  41,  43,  44,  45,  46,  48,  49,  49,  51,
        53,  54,  55,  56,  58,  61,  63,  63,  64,  65,
        65,  66,  67,  70,  71,  72,  73,  74,  77,  79,
        81,  82,  83,  84,  85,  86,  87,  87,  87,  89,
        90,  92,  92,  93,  95,  97,  97,  99, 101, 101,
	103, 106, 108, 108, 111, 111, 112, 113, 117, 118,
	118, 120, 120, 120, 120, 121, 123, 123, 125, 125,
	126, 129, 130, 130, 131, 134, 134, 134, 135, 137,
	138, 139, 140, 140, 144, 147, 147, 148, 149, 151,
	153, 154, 157, 159, 159, 160, 160, 164, 164, 164,
	167, 168, 171, 171, 172, 174, 174, 175, 176, 177,
	178, 181, 182, 185, 187, 188, 189, 190, 192, 192,
	193, 194, 194, 195, 198, 198, 199, 202, 204, 204,
	206, 208, 208, 208, 209, 211, 213, 215, 217, 218,
	222, 222, 222, 223, 224, 226, 227, 229, 229, 231,
	231, 232, 232, 235, 238, 238, 238, 240, 241, 242,
	244, 246, 247, 247, 249, 250, 252, 253, 255, 256,
	256, 258, 259, 259, 260, 263, 265, 265, 266, 268,
	271, 272, 274, 277, 277, 279, 279, 286, 288, 290,
	291, 292, 296, 296, 298, 300, 301, 301, 305, 307,
	309, 310, 311, 312, 313, 314, 315, 315, 318, 319,
	322, 322, 325, 329, 329, 330, 331, 333, 334, 336,
	338, 338, 340, 340, 340, 341, 343, 343, 343, 345,
	347, 348, 350, 353, 353, 355, 355, 356, 358, 360,
	360, 361, 361, 365, 365, 366, 368, 370, 372, 372,
	374, 377, 378, 379, 380, 382, 384, 385, 387, 388,
	390, 390, 391, 393, 395, 396, 396, 398, 400, 403,
	404, 406, 413, 424, 426, 427, 428, 432, 433, 433,
	435, 436, 438, 439, 440, 440, 442, 443, 444, 447,
	449, 449, 449, 453, 454, 454, 456, 457, 459, 462,
	463, 463, 466, 467, 468, 469, 471, 472, 473, 474,
	475, 476, 477, 477, 478, 480, 480, 481, 483, 484,
	485, 486, 487, 491, 492, 493, 494, 496, 499, 500,
	501, 502, 504, 506, 506, 508, 510, 512, 512, 512,
	514, 516, 518, 521, 523, 524, 524, 525, 527, 529,
	529, 531, 533, 541, 543, 548, 552, 556, 569, 570,
	571, 573, 575, 575, 575, 579, 580, 581, 585, 585,
	586, 588, 588, 589, 590, 591, 593, 593, 594, 595,
	598, 603, 603, 605, 605, 605, 609, 611, 612, 613,
	613, 616, 616, 618, 618, 619, 620, 621, 623, 625,
	627, 627, 627, 628, 630, 632, 634, 635, 637, 638,
	639, 641, 643, 645, 646, 646, 647, 648, 648, 650,
	651, 653, 654, 656, 658, 661, 662, 662, 664, 668,
	672, 675, 677, 677, 683, 685, 687, 689, 695, 697,
	699, 701, 705, 712, 722, 722, 726, 726, 729, 730,
	733, 733, 733, 733, 735, 737, 737, 738, 739, 741,
	742, 743, 744, 747, 748, 748, 748, 750, 750, 752};

	static const int ifin2[500]={
	  1,   2,   3,   5,   6,   6,   6,   8,  10,  12,
	  13,  14,  18,  18,  20,  22,  25,  29,  30,  30,
	  30,  30,  33,  33,  36,  38,  39,  41,  41,  42,
	  44,  45,  46,  48,  49,  49,  50,  53,  53,  55,
	  56,  57,  60,  62,  63,  64,  65,  65,  66,  67,
	  69,  71,  72,  73,  74,  75,  79,  80,  81,  82,
	  83,  85,  86,  86,  87,  87,  89,  89,  91,  92,
	  93,  94,  97,  97,  99,  99, 101, 103, 104, 108,
	  108, 109, 111, 112, 113, 116, 118, 118, 119, 120,
	  120, 120, 120, 122, 123, 124, 125, 126, 128, 130,
	  130, 131, 134, 134, 134, 134, 137, 138, 139, 140,
	  140, 142, 147, 147, 148, 149, 151, 151, 153, 156,
	  158, 159, 160, 160, 163, 164, 164, 166, 168, 171,
	  171, 172, 174, 174, 175, 176, 177, 178, 181, 182,
	  184, 187, 188, 189, 190, 191, 192, 193, 193, 194,
	  195, 196, 198, 199, 202, 202, 204, 206, 207, 208,
	  208, 208, 211, 212, 215, 217, 218, 222, 222, 222,
	  223, 224, 226, 227, 229, 229, 231, 231, 232, 232,
	  235, 238, 238, 238, 240, 241, 242, 244, 246, 247,
	  247, 248, 250, 252, 253, 254, 255, 256, 258, 258,
	  259, 260, 263, 265, 265, 266, 268, 269, 272, 274,
	  275, 277, 278, 279, 284, 288, 289, 291, 292, 296,
	  296, 297, 299, 300, 301, 305, 306, 309, 310, 311,
	  311, 312, 314, 314, 315, 318, 318, 321, 322, 324,
	  328, 329, 330, 331, 333, 334, 335, 338, 338, 339,
	  340, 340, 341, 343, 343, 343, 345, 346, 348, 349,
	  353, 353, 354, 355, 355, 356, 360, 360, 361, 361,
	  364, 365, 365, 368, 370, 372, 372, 373, 376, 378,
	  379, 380, 382, 384, 385, 386, 388, 389, 390, 391,
	  392, 394, 396, 396, 398, 400, 401, 404, 406, 411,
	  423, 425, 427, 428, 431, 433, 433, 434, 436, 438,
	  439, 439, 440, 442, 443, 444, 447, 449, 449, 449,
	  450, 453, 454, 456, 457, 458, 462, 463, 463, 464,
	  467, 468, 469, 471, 472, 473, 474, 475, 475, 477,
	  477, 478, 480, 480, 481, 483, 484, 485, 485, 487,
	  491, 492, 493, 494, 496, 499, 500, 501, 502, 502,
	  506, 506, 507, 510, 512, 512, 512, 513, 516, 517,
	  520, 522, 524, 524, 525, 527, 528, 529, 531, 532,
	  540, 542, 546, 551, 555, 562, 570, 571, 573, 575,
	  575, 575, 578, 580, 581, 584, 585, 586, 587, 588,
	  589, 590, 591, 593, 593, 594, 595, 598, 603, 603,
	  605, 605, 605, 608, 611, 612, 612, 613, 615, 616,
	  616, 618, 619, 620, 621, 622, 625, 627, 627, 627,
	  628, 629, 631, 634, 635, 636, 638, 639, 641, 642,
	  645, 645, 646, 647, 648, 648, 650, 650, 653, 654,
	  655, 657, 661, 662, 662, 663, 668, 672, 674, 677,
	  677, 682, 683, 687, 689, 695, 696, 698, 700, 705,
	  710, 722, 722, 724, 726, 727, 729, 732, 733, 733,
	  733, 733, 736, 737, 738, 739, 741, 742, 742, 744,
	  747, 748, 748, 748, 749, 750, 752, 753, 753, 755};

	  static const int ini2[500]={
	    1,   1,   1,   3,   4,   5,   6,   6,   7,   9,
	    12,  12,  14,  16,  18,  19,  21,  25,  29,  30,
	    30,  30,  30,  31,  33,  34,  38,  39,  39,  41,
	    41,  43,  44,  46,  47,  49,  49,  49,  52,  53,
	    55,  56,  56,  59,  62,  63,  64,  64,  65,  65,
	    66,  68,  70,  71,  73,  73,  74,  78,  79,  81,
	    82,  83,  85,  85,  86,  87,  87,  89,  89,  91,
	    92,  92,  94,  96,  97,  97,  99, 101, 101, 103,
	    107, 108, 108, 111, 112, 112, 116, 118, 118, 118,
	    120, 120, 120, 120, 121, 123, 123, 125, 125, 127,
	    129, 130, 130, 131, 134, 134, 134, 136, 137, 138,
	    140, 140, 142, 144, 147, 147, 149, 150, 151, 153,
	    154, 157, 159, 159, 160, 161, 164, 164, 165, 167,
	    168, 171, 172, 172, 174, 174, 175, 176, 177, 178,
	    182, 183, 185, 187, 188, 190, 190, 192, 193, 193,
	    194, 194, 196, 198, 199, 201, 202, 204, 204, 206,
	    208, 208, 208, 209, 211, 214, 216, 217, 220, 222,
	    222, 222, 223, 225, 226, 227, 229, 230, 231, 231,
	    232, 232, 237, 238, 238, 239, 241, 242, 243, 244,
	    246, 247, 248, 249, 250, 252, 254, 255, 256, 257,
	    258, 259, 259, 260, 263, 265, 265, 266, 268, 272,
	    272, 274, 277, 277, 279, 280, 286, 289, 291, 291,
	    294, 296, 296, 298, 300, 301, 303, 306, 308, 310,
	    310, 311, 312, 314, 314, 315, 316, 318, 320, 322,
	    323, 327, 329, 329, 330, 331, 334, 335, 337, 338,
	    338, 340, 340, 341, 342, 343, 343, 343, 345, 348,
	    348, 350, 353, 353, 355, 355, 356, 358, 360, 361,
	    361, 362, 365, 365, 366, 368, 371, 372, 373, 374,
	    378, 378, 379, 380, 382, 385, 385, 387, 388, 390,
	    390, 392, 393, 395, 396, 397, 398, 400, 404, 405,
	    407, 417, 424, 426, 428, 428, 432, 433, 434, 435,
	    438, 438, 439, 440, 440, 443, 443, 444, 448, 449,
	    449, 449, 453, 454, 454, 456, 458, 460, 462, 463,
	    464, 467, 467, 468, 470, 471, 472, 473, 474, 475,
	    477, 477, 477, 478, 480, 480, 481, 484, 484, 485,
	    487, 488, 491, 492, 494, 494, 497, 500, 500, 501,
	    502, 506, 506, 506, 509, 512, 512, 512, 513, 516,
	    517, 520, 522, 523, 524, 524, 525, 527, 529, 530,
	    532, 535, 541, 544, 549, 553, 557, 569, 570, 572,
	    573, 575, 575, 576, 579, 580, 582, 585, 585, 587,
	    588, 588, 590, 591, 592, 593, 593, 595, 597, 602,
	    603, 603, 605, 605, 606, 610, 611, 612, 613, 614,
	    616, 616, 618, 619, 619, 620, 621, 623, 625, 627,
	    627, 627, 628, 630, 633, 634, 635, 637, 639, 640,
	    642, 644, 645, 646, 646, 648, 648, 649, 650, 651,
	    653, 655, 656, 658, 661, 662, 662, 664, 670, 672,
	    676, 677, 680, 683, 686, 687, 690, 696, 698, 700,
	    702, 707, 720, 722, 722, 726, 726, 729, 731, 733,
	    733, 733, 733, 735, 737, 738, 739, 739, 742, 742,
	    743, 745, 747, 748, 748, 748, 750, 750, 752, 753};

      static const int ifin3[500]={
	1,   1,   3,   4,   5,   6,   6,   7,   9,  12,
        12,  14,  16,  18,  19,  21,  25,  29,  30,  30,
        30,  30,  31,  33,  34,  38,  39,  39,  41,  41,
        43,  44,  46,  47,  49,  49,  49,  52,  53,  55,
        56,  56,  59,  62,  63,  64,  64,  65,  65,  66,
        68,  70,  71,  73,  73,  74,  78,  79,  81,  82,
        83,  85,  85,  86,  87,  87,  89,  89,  91,  92,
        92,  94,  96,  97,  97,  99, 101, 101, 103, 107,
	108, 108, 111, 112, 112, 116, 118, 118, 118, 120,
	120, 120, 120, 121, 123, 123, 125, 125, 127, 129,
	130, 130, 131, 134, 134, 134, 136, 137, 138, 140,
	140, 142, 144, 147, 147, 149, 150, 151, 153, 154,
	157, 159, 159, 160, 161, 164, 164, 165, 167, 168,
	171, 172, 172, 174, 174, 175, 176, 177, 178, 182,
	183, 185, 187, 188, 190, 190, 192, 193, 193, 194,
	194, 196, 198, 199, 201, 202, 204, 204, 206, 208,
	208, 208, 209, 211, 214, 216, 217, 220, 222, 222,
	222, 223, 225, 226, 227, 229, 230, 231, 231, 232,
	232, 237, 238, 238, 239, 241, 242, 243, 244, 246,
	247, 248, 249, 250, 252, 254, 255, 256, 257, 258,
	259, 259, 260, 263, 265, 265, 266, 268, 272, 272,
	274, 277, 277, 279, 280, 286, 289, 291, 291, 294,
	296, 296, 298, 300, 301, 303, 306, 308, 310, 310,
	311, 312, 314, 314, 315, 316, 318, 320, 322, 323,
	327, 329, 329, 330, 331, 334, 335, 337, 338, 338,
	340, 340, 341, 342, 343, 343, 343, 345, 348, 348,
	350, 353, 353, 355, 355, 356, 358, 360, 361, 361,
	362, 365, 365, 366, 368, 371, 372, 373, 374, 378,
	378, 379, 380, 382, 385, 385, 387, 388, 390, 390,
	392, 393, 395, 396, 397, 398, 400, 404, 405, 407,
	417, 424, 426, 428, 428, 432, 433, 434, 435, 438,
	438, 439, 440, 440, 443, 443, 444, 448, 449, 449,
	449, 453, 454, 454, 456, 458, 460, 462, 463, 464,
	467, 467, 468, 470, 471, 472, 473, 474, 475, 477,
	477, 477, 478, 480, 480, 481, 484, 484, 485, 487,
	488, 491, 492, 494, 494, 497, 500, 500, 501, 502,
	506, 506, 506, 509, 512, 512, 512, 513, 516, 517,
	520, 522, 523, 524, 524, 525, 527, 529, 530, 532,
	535, 541, 544, 549, 553, 557, 569, 570, 572, 573,
	575, 575, 576, 579, 580, 582, 585, 585, 587, 588,
	588, 590, 591, 592, 593, 593, 595, 597, 602, 603,
	603, 605, 605, 606, 610, 611, 612, 613, 614, 616,
	616, 618, 619, 619, 620, 621, 623, 625, 627, 627,
	627, 628, 630, 633, 634, 635, 637, 639, 640, 642,
	644, 645, 646, 646, 648, 648, 649, 650, 651, 653,
	655, 656, 658, 661, 662, 662, 664, 670, 672, 676,
	677, 680, 683, 686, 687, 690, 696, 698, 700, 702,
	707, 720, 722, 722, 726, 726, 729, 731, 733, 733,
	733, 733, 735, 737, 738, 739, 739, 742, 742, 743,
	745, 747, 748, 748, 748, 750, 750, 752, 753, 755};

      static const int ini3[500]={
	1,   1,   2,   3,   5,   6,   6,   6,   8,  10,
        12,  13,  14,  18,  18,  20,  22,  25,  29,  30,
        30,  30,  30,  33,  33,  36,  38,  39,  41,  41,
        42,  44,  45,  46,  48,  49,  49,  50,  53,  53,
        55,  56,  57,  60,  62,  63,  64,  65,  65,  66,
        67,  69,  71,  72,  73,  74,  75,  79,  80,  81,
        82,  83,  85,  86,  86,  87,  87,  89,  89,  91,
        92,  93,  94,  97,  97,  99,  99, 101, 103, 104,
	108, 108, 109, 111, 112, 113, 116, 118, 118, 119,
	120, 120, 120, 120, 122, 123, 124, 125, 126, 128,
	130, 130, 131, 134, 134, 134, 134, 137, 138, 139,
	140, 140, 142, 147, 147, 148, 149, 151, 151, 153,
	156, 158, 159, 160, 160, 163, 164, 164, 166, 168,
	171, 171, 172, 174, 174, 175, 176, 177, 178, 181,
	182, 184, 187, 188, 189, 190, 191, 192, 193, 193,
	194, 195, 196, 198, 199, 202, 202, 204, 206, 207,
	208, 208, 208, 211, 212, 215, 217, 218, 222, 222,
	222, 223, 224, 226, 227, 229, 229, 231, 231, 232,
	232, 235, 238, 238, 238, 240, 241, 242, 244, 246,
	247, 247, 248, 250, 252, 253, 254, 255, 256, 258,
	258, 259, 260, 263, 265, 265, 266, 268, 269, 272,
	274, 275, 277, 278, 279, 284, 288, 289, 291, 292,
	296, 296, 297, 299, 300, 301, 305, 306, 309, 310,
	311, 311, 312, 314, 314, 315, 318, 318, 321, 322,
	324, 328, 329, 330, 331, 333, 334, 335, 338, 338,
	339, 340, 340, 341, 343, 343, 343, 345, 346, 348,
	349, 353, 353, 354, 355, 355, 356, 360, 360, 361,
	361, 364, 365, 365, 368, 370, 372, 372, 373, 376,
	378, 379, 380, 382, 384, 385, 386, 388, 389, 390,
	391, 392, 394, 396, 396, 398, 400, 401, 404, 406,
	411, 423, 425, 427, 428, 431, 433, 433, 434, 436,
	438, 439, 439, 440, 442, 443, 444, 447, 449, 449,
	449, 450, 453, 454, 456, 457, 458, 462, 463, 463,
	464, 467, 468, 469, 471, 472, 473, 474, 475, 475,
	477, 477, 478, 480, 480, 481, 483, 484, 485, 485,
	487, 491, 492, 493, 494, 496, 499, 500, 501, 502,
	502, 506, 506, 507, 510, 512, 512, 512, 513, 516,
	517, 520, 522, 524, 524, 525, 527, 528, 529, 531,
	532, 540, 542, 546, 551, 555, 562, 570, 571, 573,
	575, 575, 575, 578, 580, 581, 584, 585, 586, 587,
	588, 589, 590, 591, 593, 593, 594, 595, 598, 603,
	603, 605, 605, 605, 608, 611, 612, 612, 613, 615,
	616, 616, 618, 619, 620, 621, 622, 625, 627, 627,
	627, 628, 629, 631, 634, 635, 636, 638, 639, 641,
	642, 645, 645, 646, 647, 648, 648, 650, 650, 653,
	654, 655, 657, 661, 662, 662, 663, 668, 672, 674,
	677, 677, 682, 683, 687, 689, 695, 696, 698, 700,
	705, 710, 722, 722, 724, 726, 727, 729, 732, 733,
	733, 733, 733, 736, 737, 738, 739, 741, 742, 742,
	744, 747, 748, 748, 748, 749, 750, 752, 753, 753};



    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.53;  //Debyes
    static const double mmol=50.0;

    double q=0.678068387*pow(tt.get("K"),1.5);
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
            
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
        
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      if(ifin==0||ifin<ini){

	return complex<double> (0.0,0.0);

      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){

	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening(Frequency(fre[i],"GHz"),tt,pp,mmol,Frequency(2.5,"MHz"),0.76),Frequency(0.0,"GHz"));

	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*fre[i]; 

          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(0.047992745509/tt.get("K"))*(fac2fixed*pow(mu,2.0)/q);  // imaginary part: absorption coefficient in cm^2
	                                                                                                  // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )

	
      }
     
    }

  }

  complex<double>  RefractiveIndexProfile::mkAbs_o3_161716(Temperature tt, Pressure pp, Frequency nu){

    static const double fre[518]={
       62.76459, 71.39120, 71.47840, 83.61184, 85.42109, 88.04973,
       90.14488, 92.15323, 97.76336, 97.87746,102.26823,106.17652,
      107.36952,114.12740,117.15852,117.16913,121.25437,129.47907,
      129.50442,132.37166,135.76030,136.85658,140.27986,147.38072,
      155.81048,158.83333,160.47778,161.94325,163.59658,165.16470,
      169.29684,175.92717,188.36100,189.73936,191.92469,196.39166,
      197.75142,203.83920,212.12739,218.22219,218.59019,221.38467,
      224.02561,225.78283,225.79133,229.07147,230.02775,231.10419,
      234.01458,237.17463,240.60559,242.64470,244.20503,244.60764,
      245.62233,245.76119,245.77722,247.11398,248.48864,254.39664,
      261.10277,261.77222,266.93481,271.36219,271.84013,273.19681,
      273.28959,273.98772,277.56366,281.11616,283.41975,286.95772,
      291.22319,291.48047,296.60584,298.12088,298.32084,299.33084,
      300.21441,304.93606,305.13694,320.33794,320.37503,321.10191,
      322.13381,323.00503,323.64013,323.86488,323.89909,324.93750,
      326.08625,342.11884,342.31228,342.49441,344.65313,346.14872,
      348.69697,350.12113,350.12597,350.29453,354.70875,355.49000,
      358.55691,361.09322,361.25797,366.20106,368.61775,371.47622,
      372.30213,372.95256,374.80906,374.82953,375.18247,375.86506,
      376.30481,378.29719,383.83059,387.07788,393.18197,396.60013,
      399.41819,399.74128,400.73016,400.78484,401.74266,401.79978,
      408.36522,410.38247,413.86419,415.83272,423.30341,425.98513,
      426.03928,426.65250,427.40231,427.46472,428.71859,432.78763,
      433.24456,435.05838,435.33794,435.80709,441.70841,441.97697,
      445.55072,447.51875,448.31069,448.55922,448.73425,449.26056,
      450.41788,451.30684,452.43425,452.48581,452.53063,452.54428,
      452.80950,453.17644,454.53281,456.01588,456.36928,459.98531,
      461.00234,461.11731,461.22431,467.30231,468.55400,468.69338,
      473.68481,476.44588,476.50663,478.19244,478.26169,478.36063,
      478.43863,478.74288,478.89166,488.24347,490.95234,491.93175,
      492.12778,496.53600,499.98684,502.48097,502.95584,503.60044,
      503.85847,504.29516,506.57453,510.84003,512.87178,517.51753,
      518.51475,520.04906,523.93669,525.39825,528.20000,528.92513,
      528.93744,529.48456,529.76250,530.28438,534.62225,535.88663,
      537.00200,543.62213,547.51281,549.13200,553.87206,554.19413,
      555.04488,555.19638,555.47956,561.84100,565.81550,566.40950,
      570.18019,570.51863,572.92225,574.03994,574.11419,576.33000,
      577.32538,579.43781,579.48969,580.56256,580.57538,585.18706,
      589.70250,591.49169,594.97450,598.24144,599.64719,601.54706,
      601.95606,604.65063,605.06325,605.91937,606.03019,606.18119,
      610.62575,611.03819,612.50281,617.40456,618.72919,619.36625,
      621.63288,622.18563,623.58250,624.29188,625.76744,625.77938,
      625.86763,626.08431,626.31100,626.48819,626.73825,626.86538,
      627.15150,627.17713,627.33081,627.37063,627.41950,627.43856,
      627.73294,629.84256,630.40106,630.59131,631.22488,631.45844,
      633.22975,634.70838,635.98013,636.63131,640.95238,641.04956,
      642.30644,645.54138,648.79775,649.79781,650.60625,651.28988,
      651.76731,655.91900,656.07981,656.50056,656.84700,661.28369,
      663.78206,667.74631,667.84888,671.94300,675.67162,675.77656,
      679.80069,681.52987,681.74825,682.20156,683.85681,690.24456,
      693.47144,695.30294,696.95438,698.71363,700.47331,701.55906,
      702.38694,703.67794,704.12988,704.82062,705.28825,706.94538,
      706.97281,707.52431,714.47969,714.54981,715.82044,718.32550,
      727.11906,728.01138,730.74250,732.17725,732.32831,732.81900,
      733.56500,735.72531,738.82556,743.00581,744.14238,745.90538,
      753.14875,753.32869,753.51775,755.41869,755.51056,756.16550,
      757.36500,757.68169,758.08825,759.27388,761.87475,763.42563,
      765.83506,766.44738,767.37900,775.98550,776.02219,778.21681,
      778.28350,781.55938,782.53900,783.00775,783.33506,785.38538,
      786.34838,789.31669,791.99356,792.31738,793.76263,793.95619,
      794.47588,795.42563,795.76575,796.62394,797.20363,797.35375,
      798.56106,799.02050,799.68719,800.60744,800.68338,801.83725,
      801.84250,802.01256,803.18250,803.32419,803.39456,803.75325,
      804.10250,804.33275,804.78538,804.99800,805.26119,805.41906,
      805.56900,805.66937,805.74988,805.79488,805.80450,805.84263,
      805.86656,805.88050,806.92606,808.30887,808.56200,810.22206,
      810.66394,810.76931,811.37206,814.42581,821.05538,821.74656,
      828.22538,828.52281,830.88406,832.26763,833.58738,833.77169,
      835.14719,835.79469,837.74563,838.02275,838.55556,849.90469,
      853.54381,855.43356,857.58575,857.92525,858.84531,858.96625,
      865.31344,866.75163,867.43850,868.71844,869.44206,872.35469,
      877.57025,878.70731,882.37419,882.88244,884.08475,884.14806,
      885.44663,897.10544,898.55838,903.48250,904.49425,904.94394,
      905.86669,908.15925,909.30763,909.31906,917.04800,918.74525,
      918.94906,919.89788,921.14338,924.50056,928.83394,929.00944,
      930.73675,931.16769,931.59069,931.99544,932.55481,933.41788,
      934.51581,942.11850,943.85519,951.85931,953.02500,953.10894,
      953.14819,953.16531,954.28994,956.36775,956.45038,956.73050,
      958.65994,959.06650,959.71100,960.37725,960.70275,961.77019,
      962.09931,965.23144,965.98231,968.28294,968.73731,970.32463,
      971.12588,973.67925,973.68888,975.66875,975.90106,976.29381,
      977.66413,977.77969,978.30550,978.93994,979.32569,979.84962,
      980.56431,981.02256,981.52956,981.71613,981.86656,981.89938,
      982.25925,982.54019,982.79219,982.99413,983.16544,983.30244,
      983.41344,983.50006,983.56688,983.61663,983.65250,983.67713,
      983.69288,983.70194,983.88694,984.89481,985.87362,993.86519,
       997.95150,998.69363};

    static const double flin[518]={
      .331E+01, .232E+01, .280E+01, .117E+01, .234E+01, .453E+01,
      .289E+01, .248E+01, .342E+01, .432E+01, .901E+00, .326E+01,
      .593E+01, .100E+01, .914E+00, .444E+01, .721E+01, .199E+01,
      .417E+01, .185E+01, .389E+01, .424E+01, .810E+01, .302E+01,
      .558E+00, .376E+01, .201E+01, .497E+01, .283E+01, .858E+01,
      .403E+01, .158E+01, .573E+01, .139E+01, .382E+01, .872E+01,
      .260E+01, .308E+01, .454E+01, .481E+01, .239E+01, .360E+01,
      .115E+01, .131E+02, .111E+02, .477E+01, .894E+01, .149E+02,
      .861E+01, .691E+01, .545E+01, .164E+02, .937E+00, .426E+01,
      .339E+01, .460E+01, .510E+01, .748E+01, .216E+01, .354E+01,
      .173E+02, .217E+01, .855E+00, .438E+01, .194E+01, .713E+00,
      .316E+01, .145E+01, .838E+01, .250E+01, .559E+01, .177E+02,
      .530E+01, .343E+01, .503E+00, .416E+01, .294E+01, .172E+01,
      .597E+01, .938E+01, .425E+01, .149E+01, .177E+02, .710E+01,
      .496E+01, .302E+00, .601E+01, .394E+01, .149E+01, .272E+01,
      .813E+01, .222E+02, .167E+01, .557E+01, .242E+02, .199E+02,
      .494E+01, .127E+01, .372E+01, .250E+01, .257E+02, .174E+02,
      .879E+01, .174E+02, .114E+02, .606E+01, .151E+02, .736E+01,
      .620E+01, .266E+02, .178E+01, .472E+01, .105E+01, .350E+01,
      .227E+01, .791E+01, .129E+02, .211E+01, .646E+01, .106E+02,
      .109E+02, .269E+02, .450E+01, .836E+00, .205E+01, .328E+01,
      .169E+02, .665E+01, .927E+01, .135E+02, .678E+01, .628E+00,
      .779E+01, .428E+01, .183E+01, .305E+01, .263E+01, .776E+01,
      .206E+01, .268E+02, .646E+01, .126E+02, .522E+01, .896E+01,
      .403E+01, .285E+01, .160E+01, .884E+00, .224E+01, .343E+01,
      .458E+01, .431E+00, .406E+01, .313E+02, .334E+02, .571E+01,
      .160E+01, .283E+01, .726E+01, .683E+01, .701E+01, .288E+02,
      .165E+02, .350E+02, .793E+01, .320E+01, .899E+01, .156E+02,
      .261E+02, .146E+02, .252E+00, .383E+01, .139E+01, .999E+01,
      .263E+02, .261E+01, .359E+02, .766E+01, .109E+02, .234E+02,
      .719E+01, .223E+01, .743E+01, .484E+01, .385E+01, .117E+01,
      .361E+01, .238E+01, .118E+02, .108E+02, .210E+02, .160E+02,
      .167E+02, .176E+02, .249E+01, .125E+02, .461E+01, .958E+00,
      .256E+02, .339E+01, .216E+01, .731E+01, .187E+02, .458E+01,
      .852E+01, .761E+01, .131E+02, .261E+01, .439E+01, .754E+00,
      .316E+01, .194E+01, .168E+02, .187E+02, .225E+01, .543E+01,
      .197E+02, .740E+01, .136E+02, .277E+01, .151E+02, .157E+02,
      .127E+02, .559E+00, .417E+01, .294E+01, .172E+01, .250E+02,
      .135E+02, .835E+01, .642E+01, .759E+01, .295E+01, .141E+02,
      .121E+02, .378E+00, .394E+01, .150E+01, .272E+01, .208E+02,
      .319E+02, .108E+02, .747E+01, .954E+01, .999E+01, .218E+02,
      .833E+01, .758E+01, .314E+01, .714E+01, .101E+02, .890E+01,
      .597E+01, .772E+01, .113E+02, .655E+01, .480E+01, .538E+01,
      .421E+01, .361E+01, .300E+01, .235E+01, .167E+01, .909E+00,
      .125E+02, .216E+00, .137E+02, .372E+01, .129E+01, .249E+01,
      .144E+02, .149E+02, .155E+02, .292E+02, .148E+02, .160E+02,
      .215E+01, .244E+02, .892E+01, .171E+02, .332E+01, .228E+02,
      .758E+01, .751E+01, .349E+01, .108E+01, .227E+01, .181E+02,
      .267E+02, .147E+02, .238E+02, .353E+01, .105E+02, .190E+02,
      .450E+01, .327E+01, .880E+00, .205E+01, .946E+01, .245E+02,
      .199E+02, .154E+02, .248E+02, .117E+02, .754E+01, .169E+02,
      .368E+01, .122E+02, .759E+01, .148E+02, .427E+01, .305E+01,
      .686E+00, .184E+01, .206E+02, .226E+02, .258E+02, .391E+01,
      .193E+01, .349E+01, .405E+01, .504E+00, .283E+01, .162E+01,
      .141E+02, .209E+02, .212E+02, .269E+02, .150E+02, .757E+01,
      .358E+01, .194E+02, .153E+02, .759E+01, .401E+01, .383E+01,
      .336E+00, .261E+01, .141E+01, .191E+02, .429E+01, .279E+02,
      .161E+02, .216E+02, .179E+02, .108E+02, .135E+02, .165E+02,
      .372E+01, .360E+01, .189E+00, .239E+01, .121E+01, .151E+02,
      .152E+02, .289E+02, .758E+01, .139E+02, .184E+02, .196E+02,
      .171E+02, .209E+02, .158E+02, .127E+02, .220E+02, .145E+02,
      .222E+02, .133E+02, .115E+02, .121E+02, .182E+02, .103E+02,
      .467E+01, .109E+02, .967E+01, .909E+01, .388E+01, .234E+02,
      .850E+01, .792E+01, .734E+01, .675E+01, .617E+01, .558E+01,
      .498E+01, .438E+01, .376E+01, .760E+01, .311E+01, .244E+01,
      .172E+01, .926E+00, .338E+01, .217E+01, .101E+01, .153E+02,
      .427E+01, .299E+02, .245E+02, .213E+02, .166E+01, .256E+02,
      .152E+02, .406E+01, .223E+02, .316E+01, .195E+01, .813E+00,
      .266E+02, .309E+02, .506E+01, .203E+02, .760E+01, .156E+02,
      .425E+01, .761E+01, .294E+01, .319E+02, .174E+01, .629E+00,
      .153E+02, .123E+02, .234E+02, .445E+01, .549E+01, .152E+02,
      .224E+02, .444E+01, .329E+02, .272E+01, .153E+01, .458E+00,
      .761E+01, .598E+01, .268E+02, .463E+01, .762E+01, .339E+02,
      .371E+01, .250E+01, .133E+01, .302E+00, .253E+02, .255E+02,
      .245E+02, .177E+02, .656E+01, .138E+01, .482E+01, .349E+02,
      .452E+01, .349E+01, .450E+01, .238E+02, .762E+01, .229E+01,
      .113E+01, .726E+01, .224E+02, .360E+02, .501E+01, .763E+01,
      .210E+02, .270E+02, .257E+02, .243E+02, .327E+01, .457E+01,
      .207E+01, .229E+02, .939E+00, .197E+02, .810E+01, .266E+02,
      .216E+02, .203E+02, .184E+02, .190E+02, .276E+02, .171E+02,
      .177E+02, .165E+02, .158E+02, .370E+02, .152E+02, .146E+02,
      .910E+01, .140E+02, .134E+02, .520E+01, .128E+02, .122E+02,
      .116E+02, .110E+02, .105E+02, .305E+01, .469E+01, .987E+01,
      .929E+01, .870E+01, .812E+01, .753E+01, .694E+01, .635E+01,
      .575E+01, .514E+01, .452E+01, .387E+01, .321E+01, .251E+01,
      .176E+01, .937E+00, .186E+01, .755E+00, .200E+02, .103E+02,
      .446E+01, .380E+02};

    static const double el[518]={
      223.585,  145.271,   21.867,   56.784,  161.597,  461.771,
      96.770,    3.574,  317.904,   11.999,   30.388,  293.300,
      25.243,     .018,   36.972,  525.972,   43.176,  126.781,
      36.979,  112.142,  268.379,  493.526,   65.862,  268.321,
      21.816,  126.771,    3.592,  525.921,  245.842,   93.167,
      461.768,   96.750,   56.852,   83.986,  431.681,  124.967,
      223.601,   12.009,  317.911,  669.618,  203.300,  402.466,
      71.439,  172.555,  134.355,  161.597,  101.051,  215.499,
      161.360,   72.608,  595.043,  263.245,   60.870,   25.251,
      374.702,  633.195,   48.995,   81.230,  183.764,   30.381,
      315.558,   16.726,    8.078,  598.166,  165.643,   50.977,
      347.973,   11.339,  202.268,   21.797,   43.186,  372.516,
      372.312,   36.999,   42.653,  564.280,  322.671,  148.622,
      201.180,  110.353,   56.843,    5.513,  434.011,   65.876,
      81.227,   35.285,  668.955,  531.544,  132.942,  298.392,
      247.537,  517.267,    8.058,  110.349,  590.045,  449.373,
      792.451,  118.483,  500.082,  275.474,  667.462,  386.325,
      93.077,  499.956,  144.048,  144.010,  328.094,  245.357,
      431.517,  749.386,   11.297,  753.796,  105.179,  469.946,
      253.625,  297.429,  274.863,   16.707,  182.214,  124.950,
      226.615,  835.999,  716.252,   93.049,  233.103,  440.900,
      570.385,  747.588,  183.421,  182.226,  225.067,   82.239,
      145.236,  680.007,  213.770,  413.067,   30.407,  351.712,
      21.790,  927.103,  112.089,  161.367,   83.983,  294.370,
      60.799,   42.604,   29.266,   24.454,   35.346,   51.021,
      71.465,   72.522,  644.968, 1045.566, 1147.863,   96.763,
      195.650,  386.532,  495.467,  126.796,  272.479,  947.905,
      645.257, 1254.851,  161.546,   49.054,  201.094,  225.087,
      855.147,  202.215,   64.112,  611.131,  178.728,  245.423,
      1022.619,  361.104, 1366.497,  410.548,  294.318,  767.371,
      324.362,   36.950,  831.184,  887.664,   72.541,  163.010,
      578.498,  336.977,  347.968,  347.963,  684.465,  724.542,
      247.594,  272.420,   20.834,  406.263,  847.829,  148.494,
      1122.493,  547.167,  313.954,  380.829,  606.520,  101.060,
      564.206,  473.888,  469.201,   24.423,  809.196,  135.280,
      516.939,  292.232,  533.719,  297.413,   56.822,  134.394,
      324.412,  441.796,  536.781,   29.327,  466.023,  808.317,
      406.269,  123.168,  771.765,  487.914,  271.713, 1226.792,
      403.375,  919.589,  172.521,  541.665,   35.297,  608.806,
      345.786,  112.257,  735.537,  252.396,  460.190,  351.783,
      1389.470,  293.250,  507.379,  245.744,  637.680,  380.850,
      203.241,  215.514,   42.647,  165.713,  268.342,  223.642,
      132.938,  183.727,  317.916,  148.608,  105.196,  118.390,
      93.076,   82.175,   72.567,   64.065,   56.863,   50.762,
      372.348,  102.647,  431.520,  700.511,  234.280,  433.569,
      685.484,  495.513,  896.552, 1277.021,  469.213,  564.208,
      81.248, 1335.493,  263.236,  637.688,   50.949,  410.616,
      613.993,  577.394,  666.686,  217.366,  408.250,  715.936,
      1169.616,  766.726,  441.821,   60.824,  315.545,  798.740,
      951.647,  634.064,  201.653,  384.032, 1012.752, 1067.245,
      886.290,  989.202,  473.923,  715.839,  651.854,  536.702,
      71.462,  372.500,  690.778,  852.445,  911.823,  602.743,
      187.142,  361.115,  978.381,  969.978,  507.317,   83.997,
      110.274,   45.931,  873.200,  173.931,  572.524,  339.400,
      433.965,  877.961, 1075.112,  541.711,  942.656,  730.872,
      50.824,  790.915, 1086.306,  772.115,   96.710,  835.779,
      161.821,  543.606,  318.887,  608.830,  112.105,  577.330,
      500.014, 1176.485,  709.040, 1110.627,  798.725,  632.220,
      56.817,  799.559,  151.012,  515.790,  299.574, 1037.475,
      560.429,  613.987,  814.458,  493.542,  747.673,  831.164,
      668.839,  919.593,  594.977,  431.636, 1282.308,  525.901,
      1012.743,  461.720,  374.688,  402.444,  570.441,  322.585,
      145.185,  347.977,  298.421,  275.414,   64.110, 1110.593,
      253.676,  233.065,  213.744,  195.633,  178.721,  163.013,
      148.506,  135.201,  123.197,  857.895,  112.295,  102.593,
      94.192,   86.991,  764.541,  489.274,  281.462, 1187.882,
      126.761,  651.856, 1213.227,  685.480,  143.962, 1320.629,
      1136.718,   72.503, 1392.790,  730.724,  463.860,  264.551,
      1432.585,  690.754,  183.360,  645.247,  902.421,  886.276,
      82.202,  948.111,  698.207,  730.891,  439.747,  248.841,
      1293.936, 1213.267,  766.734,  161.573,  226.638, 1240.498,
      724.547,   93.093,  772.117,  666.792,  416.835,  234.331,
      994.969,  274.809, 1418.739,  105.103, 1042.854,  814.433,
      945.588,  636.678,  395.123,  221.122, 1307.551,  852.470,
      808.360,  978.414,  328.054,  182.193,  118.385,  857.877,
      201.094,  908.273,   80.953, 1201.433, 1092.006,  607.665,
      374.612,  386.247, 1100.364,  902.479,  132.923, 1142.119,
      1004.317, 1472.716, 1360.062, 1252.163,  872.059,   86.945,
      579.952, 1149.233,  355.302,  913.270,  449.354,  896.503,
      1050.987,  957.737,  827.101,  869.290,  942.668,  745.892,
      785.754,  707.031,  669.531,  948.135,  633.224,  598.105,
      517.340,  564.234,  531.609,  148.678,  500.060,  469.835,
      440.900,  413.078,  386.454,  837.045,   94.138,  361.136,
      336.919,  314.005,  292.193,  271.684,  252.375,  234.269,
      217.363,  201.659,  187.156,  173.854,  161.852,  150.951,
      141.350,  132.850,  553.341,  337.192, 1075.145,  589.961,
      245.365,  994.930};
    
    static const int ifin1[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   2,
      2,   2,   2,   2,   4,   4,   4,   4,   4,   4,
      5,   6,   7,   8,   9,   9,   9,  11,  11,  12,
      12,  13,  14,  14,  14,  15,  15,  17,  17,  18,
      18,  18,  18,  20,  21,  21,  23,  23,  24,  24,
      24,  24,  25,  25,  25,  25,  26,  27,  28,  29,
      30,  31,  31,  32,  32,  32,  33,  33,  33,  33,
      33,  33,  34,  35,  36,  36,  37,  38,  38,  38,
      39,  39,  39,  39,  40,  40,  40,  42,  42,  43,
      44,  46,  46,  48,  49,  50,  50,  51,  52,  53,
      55,  58,  60,  60,  60,  61,  61,  61,  61,  63,
      63,  64,  64,  64,  66,  69,  69,  70,  70,  71,
      72,  73,  73,  73,  75,  75,  76,  78,  80,  80,
      81,  82,  82,  82,  82,  82,  82,  82,  84,  86,
      91,  92,  92,  92,  92,  92,  92,  92,  92,  95,
      96,  97,  98, 101, 101, 102, 103, 104, 104, 106,
      106, 107, 108, 108, 111, 113, 116, 117, 117, 117,
      118, 118, 119, 119, 119, 120, 121, 121, 125, 127,
      127, 127, 128, 129, 129, 130, 131, 131, 131, 131,
      132, 135, 138, 138, 139, 140, 143, 143, 143, 145,
      145, 146, 150, 152, 158, 160, 162, 162, 163, 166,
      166, 166, 169, 169, 169, 170, 172, 178, 178, 178,
      178, 178, 179, 180, 182, 182, 183, 183, 184, 186,
      189, 190, 190, 191, 192, 192, 192, 194, 195, 195,
      196, 197, 200, 203, 203, 204, 205, 206, 206, 206,
      207, 207, 208, 209, 209, 211, 214, 214, 214, 215,
      215, 217, 217, 219, 220, 222, 223, 224, 228, 228,
      228, 229, 229, 230, 231, 232, 232, 233, 234, 236,
      237, 241, 241, 242, 244, 244, 244, 246, 247, 249,
      251, 259, 266, 269, 271, 273, 275, 275, 276, 278,
      278, 279, 280, 282, 284, 284, 288, 288, 288, 289,
      290, 290, 292, 292, 293, 293, 295, 295, 296, 299,
      300, 300, 300, 301, 301, 302, 304, 305, 306, 308,
      311, 314, 315, 315, 315, 317, 318, 319, 319, 319,
      319, 319, 321, 322, 325, 326, 327, 328, 328, 328,
      330, 331, 331, 331, 331, 334, 337, 340, 341, 342,
      343, 345, 346, 346, 346, 346, 348, 350, 350, 352,
      354, 356, 356, 357, 359, 362, 365, 368, 372, 375,
      383, 394, 396, 399, 400, 401, 401, 401, 401, 403,
      403, 403, 405, 406, 407, 409, 411, 414, 414, 414,
      414, 414, 414, 415, 415, 416, 417, 421, 421, 421,
      421, 423, 425, 426, 427, 427, 427, 429, 429, 431,
      433, 434, 434, 434, 434, 434, 434, 436, 436, 436,
      439, 440, 441, 443, 443, 443, 443, 446, 447, 448,
      449, 449, 450, 452, 456, 458, 458, 458, 458, 459,
      460, 460, 460, 460, 461, 466, 469, 470, 474, 476,
      476, 478, 480, 481, 482, 484, 487, 491, 494, 503,
      515, 516, 516, 516, 516, 517, 517, 518, 518, 518};

    static const int ini1[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   2,   2,   2,   2,   2,   4,   4,
      4,   4,   4,   4,   5,   6,   7,   8,   9,   9,
      9,  11,  11,  12,  12,  13,  14,  14,  14,  15,
      15,  17,  17,  18,  18,  18,  18,  20,  21,  21,
      23,  23,  24,  24,  24,  24,  25,  25,  25,  25,
      26,  27,  28,  29,  30,  31,  31,  32,  32,  32,
      33,  33,  33,  33,  33,  33,  34,  35,  36,  36,
      37,  38,  38,  38,  39,  39,  39,  39,  40,  40,
      40,  42,  42,  43,  44,  46,  46,  48,  49,  50,
      50,  51,  52,  53,  55,  58,  60,  60,  60,  61,
      61,  61,  61,  63,  63,  64,  64,  64,  66,  69,
      69,  70,  70,  71,  72,  73,  73,  73,  75,  75,
      76,  78,  80,  80,  81,  82,  82,  82,  82,  82,
      82,  82,  84,  86,  91,  92,  92,  92,  92,  92,
      92,  92,  92,  95,  96,  97,  98, 101, 101, 102,
      103, 104, 104, 106, 106, 107, 108, 108, 111, 113,
      116, 117, 117, 117, 118, 118, 119, 119, 119, 120,
      121, 121, 125, 127, 127, 127, 128, 129, 129, 130,
      131, 131, 131, 131, 132, 135, 138, 138, 139, 140,
      143, 143, 143, 145, 145, 146, 150, 152, 158, 160,
      162, 162, 163, 166, 166, 166, 169, 169, 169, 170,
      172, 178, 178, 178, 178, 178, 179, 180, 182, 182,
      183, 183, 184, 186, 189, 190, 190, 191, 192, 192,
      192, 194, 195, 195, 196, 197, 200, 203, 203, 204,
      205, 206, 206, 206, 207, 207, 208, 209, 209, 211,
      214, 214, 214, 215, 215, 217, 217, 219, 220, 222,
      223, 224, 228, 228, 228, 229, 229, 230, 231, 232,
      232, 233, 234, 236, 237, 241, 241, 242, 244, 244,
      244, 246, 247, 249, 251, 259, 266, 269, 271, 273,
      275, 275, 276, 278, 278, 279, 280, 282, 284, 284,
      288, 288, 288, 289, 290, 290, 292, 292, 293, 293,
      295, 295, 296, 299, 300, 300, 300, 301, 301, 302,
      304, 305, 306, 308, 311, 314, 315, 315, 315, 317,
      318, 319, 319, 319, 319, 319, 321, 322, 325, 326,
      327, 328, 328, 328, 330, 331, 331, 331, 331, 334,
      337, 340, 341, 342, 343, 345, 346, 346, 346, 346,
      348, 350, 350, 352, 354, 356, 356, 357, 359, 362,
      365, 368, 372, 375, 383, 394, 396, 399, 400, 401,
      401, 401, 401, 403, 403, 403, 405, 406, 407, 409,
      411, 414, 414, 414, 414, 414, 414, 415, 415, 416,
      417, 421, 421, 421, 421, 423, 425, 426, 427, 427,
      427, 429, 429, 431, 433, 434, 434, 434, 434, 434,
      434, 436, 436, 436, 439, 440, 441, 443, 443, 443,
      443, 446, 447, 448, 449, 449, 450, 452, 456, 458,
      458, 458, 458, 459, 460, 460, 460, 460, 461, 466,
      469, 470, 474, 476, 476, 478, 480, 481, 482, 484,
      487, 491, 494, 503, 515, 516, 516, 516, 516, 517};

    static const int ifin2[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   2,   2,   2,   2,   4,   4,   4,   4,   4,
      4,   5,   6,   7,   8,   9,   9,   9,  11,  11,
      12,  12,  13,  14,  14,  14,  15,  15,  17,  17,
      18,  18,  18,  18,  20,  21,  21,  22,  23,  24,
      24,  24,  24,  25,  25,  25,  25,  26,  26,  28,
      29,  30,  31,  31,  32,  32,  32,  33,  33,  33,
      33,  33,  33,  34,  35,  36,  36,  37,  38,  38,
      38,  39,  39,  39,  39,  40,  40,  40,  41,  42,
      43,  44,  46,  46,  48,  49,  50,  50,  51,  51,
      52,  54,  58,  60,  60,  60,  61,  61,  61,  61,
      63,  63,  63,  64,  64,  66,  69,  69,  70,  70,
      71,  72,  72,  73,  73,  75,  75,  75,  78,  80,
      80,  80,  82,  82,  82,  82,  82,  82,  82,  84,
      86,  90,  92,  92,  92,  92,  92,  92,  92,  92,
      95,  95,  97,  97, 101, 101, 101, 103, 103, 104,
      106, 106, 107, 107, 108, 110, 111, 116, 117, 117,
      117, 118, 118, 119, 119, 119, 120, 120, 121, 123,
      127, 127, 127, 128, 129, 129, 130, 131, 131, 131,
      131, 132, 134, 137, 138, 138, 140, 143, 143, 143,
      145, 145, 146, 148, 152, 155, 159, 162, 162, 163,
      166, 166, 166, 167, 169, 169, 170, 171, 176, 178,
      178, 178, 178, 179, 179, 182, 182, 182, 183, 184,
      185, 189, 189, 190, 190, 191, 192, 192, 193, 195,
      195, 196, 197, 198, 203, 203, 203, 205, 206, 206,
      206, 207, 207, 208, 209, 209, 211, 214, 214, 214,
      215, 215, 217, 217, 218, 219, 222, 223, 224, 226,
      228, 228, 229, 229, 230, 231, 231, 232, 233, 234,
      236, 236, 241, 241, 241, 243, 244, 244, 245, 247,
      249, 251, 257, 266, 268, 271, 272, 274, 275, 275,
      278, 278, 279, 279, 281, 284, 284, 286, 288, 288,
      289, 290, 290, 292, 292, 293, 293, 295, 295, 296,
      299, 300, 300, 300, 301, 301, 302, 303, 304, 306,
      308, 310, 312, 315, 315, 315, 316, 318, 319, 319,
      319, 319, 319, 321, 321, 324, 326, 327, 327, 328,
      328, 330, 331, 331, 331, 331, 334, 337, 340, 341,
      342, 343, 345, 346, 346, 346, 346, 348, 350, 350,
      351, 354, 356, 356, 357, 359, 362, 364, 367, 370,
      375, 381, 393, 395, 397, 400, 401, 401, 401, 401,
      403, 403, 403, 404, 405, 407, 409, 411, 413, 414,
      414, 414, 414, 414, 415, 415, 416, 417, 419, 421,
      421, 421, 422, 424, 426, 427, 427, 427, 428, 429,
      430, 433, 434, 434, 434, 434, 434, 434, 435, 436,
      436, 438, 440, 441, 443, 443, 443, 443, 444, 447,
      448, 448, 449, 449, 451, 455, 457, 458, 458, 458,
      459, 460, 460, 460, 460, 461, 466, 468, 469, 473,
      476, 476, 478, 479, 481, 482, 484, 487, 490, 493,
      500, 514, 516, 516, 516, 516, 517, 517, 518, 518};

    static const int ini2[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   2,   2,   2,   2,   4,   4,   4,   4,
      4,   4,   4,   6,   6,   7,   8,   9,   9,   9,
      11,  11,  12,  12,  14,  14,  14,  14,  15,  17,
      17,  18,  18,  18,  18,  19,  20,  21,  21,  23,
      23,  24,  24,  24,  25,  25,  25,  25,  25,  26,
      27,  28,  29,  31,  31,  32,  32,  32,  32,  33,
      33,  33,  33,  33,  33,  34,  35,  36,  36,  37,
      38,  38,  38,  39,  39,  39,  39,  40,  40,  40,
      42,  43,  43,  44,  46,  47,  49,  49,  50,  51,
      51,  52,  53,  55,  59,  60,  60,  60,  61,  61,
      61,  62,  63,  63,  64,  64,  65,  68,  69,  69,
      70,  71,  72,  72,  73,  73,  75,  75,  75,  76,
      79,  80,  80,  82,  82,  82,  82,  82,  82,  82,
      82,  85,  87,  91,  92,  92,  92,  92,  92,  92,
      92,  92,  95,  96,  97,  98, 101, 101, 103, 103,
      104, 106, 106, 106, 107, 108, 109, 111, 114, 116,
      117, 117, 117, 118, 119, 119, 119, 120, 120, 121,
      122, 125, 127, 127, 127, 128, 129, 129, 130, 131,
      131, 131, 132, 132, 137, 138, 138, 140, 142, 143,
      143, 143, 145, 145, 146, 151, 153, 159, 160, 162,
      162, 166, 166, 166, 167, 169, 169, 169, 170, 172,
      178, 178, 178, 178, 178, 179, 180, 182, 182, 183,
      183, 184, 186, 189, 190, 190, 191, 192, 192, 192,
      194, 195, 195, 197, 197, 201, 203, 203, 204, 206,
      206, 206, 206, 207, 207, 209, 209, 209, 214, 214,
      214, 214, 215, 215, 217, 217, 219, 220, 222, 224,
      226, 228, 228, 229, 229, 229, 231, 231, 232, 232,
      233, 234, 236, 238, 241, 241, 243, 244, 244, 245,
      247, 247, 249, 251, 265, 266, 271, 272, 273, 275,
      275, 277, 278, 278, 279, 280, 283, 284, 284, 288,
      288, 289, 289, 290, 290, 292, 292, 293, 293, 295,
      295, 296, 299, 300, 300, 300, 301, 302, 303, 304,
      305, 306, 308, 312, 314, 315, 315, 315, 317, 318,
      319, 319, 319, 319, 320, 321, 322, 325, 326, 327,
      328, 328, 329, 330, 331, 331, 331, 333, 335, 338,
      341, 341, 343, 343, 346, 346, 346, 346, 346, 348,
      350, 350, 354, 355, 356, 357, 357, 359, 363, 367,
      369, 372, 378, 385, 394, 396, 400, 400, 401, 401,
      401, 402, 403, 403, 403, 405, 406, 407, 410, 411,
      414, 414, 414, 414, 414, 414, 415, 415, 417, 417,
      421, 421, 421, 422, 424, 426, 426, 427, 427, 427,
      429, 429, 431, 434, 434, 434, 434, 434, 434, 435,
      436, 436, 437, 439, 440, 443, 443, 443, 443, 444,
      446, 448, 448, 449, 449, 451, 453, 457, 458, 458,
      458, 458, 459, 460, 460, 460, 460, 465, 466, 469,
      471, 474, 476, 477, 478, 480, 482, 482, 484, 487,
      492, 495, 506, 515, 516, 516, 516, 516, 517, 517};

    static const int ifin3[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   2,   2,   2,   2,   4,   4,   4,   4,   4,
      4,   4,   6,   6,   7,   8,   9,   9,   9,  11,
      11,  12,  12,  14,  14,  14,  14,  15,  17,  17,
      18,  18,  18,  18,  19,  20,  21,  21,  23,  23,
      24,  24,  24,  25,  25,  25,  25,  25,  26,  27,
      28,  29,  31,  31,  32,  32,  32,  32,  33,  33,
      33,  33,  33,  33,  34,  35,  36,  36,  37,  38,
      38,  38,  39,  39,  39,  39,  40,  40,  40,  42,
      43,  43,  44,  46,  47,  49,  49,  50,  51,  51,
      52,  53,  55,  59,  60,  60,  60,  61,  61,  61,
      62,  63,  63,  64,  64,  65,  68,  69,  69,  70,
      71,  72,  72,  73,  73,  75,  75,  75,  76,  79,
      80,  80,  82,  82,  82,  82,  82,  82,  82,  82,
      85,  87,  91,  92,  92,  92,  92,  92,  92,  92,
      92,  95,  96,  97,  98, 101, 101, 103, 103, 104,
      106, 106, 106, 107, 108, 109, 111, 114, 116, 117,
      117, 117, 118, 119, 119, 119, 120, 120, 121, 122,
      125, 127, 127, 127, 128, 129, 129, 130, 131, 131,
      131, 132, 132, 137, 138, 138, 140, 142, 143, 143,
      143, 145, 145, 146, 151, 153, 159, 160, 162, 162,
      166, 166, 166, 167, 169, 169, 169, 170, 172, 178,
      178, 178, 178, 178, 179, 180, 182, 182, 183, 183,
      184, 186, 189, 190, 190, 191, 192, 192, 192, 194,
      195, 195, 197, 197, 201, 203, 203, 204, 206, 206,
      206, 206, 207, 207, 209, 209, 209, 214, 214, 214,
      214, 215, 215, 217, 217, 219, 220, 222, 224, 226,
      228, 228, 229, 229, 229, 231, 231, 232, 232, 233,
      234, 236, 238, 241, 241, 243, 244, 244, 245, 247,
      247, 249, 251, 265, 266, 271, 272, 273, 275, 275,
      277, 278, 278, 279, 280, 283, 284, 284, 288, 288,
      289, 289, 290, 290, 292, 292, 293, 293, 295, 295,
      296, 299, 300, 300, 300, 301, 302, 303, 304, 305,
      306, 308, 312, 314, 315, 315, 315, 317, 318, 319,
      319, 319, 319, 320, 321, 322, 325, 326, 327, 328,
      328, 329, 330, 331, 331, 331, 333, 335, 338, 341,
      341, 343, 343, 346, 346, 346, 346, 346, 348, 350,
      350, 354, 355, 356, 357, 357, 359, 363, 367, 369,
      372, 378, 385, 394, 396, 400, 400, 401, 401, 401,
      402, 403, 403, 403, 405, 406, 407, 410, 411, 414,
      414, 414, 414, 414, 414, 415, 415, 417, 417, 421,
      421, 421, 422, 424, 426, 426, 427, 427, 427, 429,
      429, 431, 434, 434, 434, 434, 434, 434, 435, 436,
      436, 437, 439, 440, 443, 443, 443, 443, 444, 446,
      448, 448, 449, 449, 451, 453, 457, 458, 458, 458,
      458, 459, 460, 460, 460, 460, 465, 466, 469, 471,
      474, 476, 477, 478, 480, 482, 482, 484, 487, 492,
      495, 506, 515, 516, 516, 516, 516, 517, 517, 518};

    static const int ini3[500]={
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   2,   2,   2,   2,   4,   4,   4,   4,
      4,   4,   5,   6,   7,   8,   9,   9,   9,  11,
      11,  12,  12,  13,  14,  14,  14,  15,  15,  17,
      17,  18,  18,  18,  18,  20,  21,  21,  22,  23,
      24,  24,  24,  24,  25,  25,  25,  25,  26,  26,
      28,  29,  30,  31,  31,  32,  32,  32,  33,  33,
      33,  33,  33,  33,  34,  35,  36,  36,  37,  38,
      38,  38,  39,  39,  39,  39,  40,  40,  40,  41,
      42,  43,  44,  46,  46,  48,  49,  50,  50,  51,
      51,  52,  54,  58,  60,  60,  60,  61,  61,  61,
      61,  63,  63,  63,  64,  64,  66,  69,  69,  70,
      70,  71,  72,  72,  73,  73,  75,  75,  75,  78,
      80,  80,  80,  82,  82,  82,  82,  82,  82,  82,
      84,  86,  90,  92,  92,  92,  92,  92,  92,  92,
      92,  95,  95,  97,  97, 101, 101, 101, 103, 103,
      104, 106, 106, 107, 107, 108, 110, 111, 116, 117,
      117, 117, 118, 118, 119, 119, 119, 120, 120, 121,
      123, 127, 127, 127, 128, 129, 129, 130, 131, 131,
      131, 131, 132, 134, 137, 138, 138, 140, 143, 143,
      143, 145, 145, 146, 148, 152, 155, 159, 162, 162,
      163, 166, 166, 166, 167, 169, 169, 170, 171, 176,
      178, 178, 178, 178, 179, 179, 182, 182, 182, 183,
      184, 185, 189, 189, 190, 190, 191, 192, 192, 193,
      195, 195, 196, 197, 198, 203, 203, 203, 205, 206,
      206, 206, 207, 207, 208, 209, 209, 211, 214, 214,
      214, 215, 215, 217, 217, 218, 219, 222, 223, 224,
      226, 228, 228, 229, 229, 230, 231, 231, 232, 233,
      234, 236, 236, 241, 241, 241, 243, 244, 244, 245,
      247, 249, 251, 257, 266, 268, 271, 272, 274, 275,
      275, 278, 278, 279, 279, 281, 284, 284, 286, 288,
      288, 289, 290, 290, 292, 292, 293, 293, 295, 295,
      296, 299, 300, 300, 300, 301, 301, 302, 303, 304,
      306, 308, 310, 312, 315, 315, 315, 316, 318, 319,
      319, 319, 319, 319, 321, 321, 324, 326, 327, 327,
      328, 328, 330, 331, 331, 331, 331, 334, 337, 340,
      341, 342, 343, 345, 346, 346, 346, 346, 348, 350,
      350, 351, 354, 356, 356, 357, 359, 362, 364, 367,
      370, 375, 381, 393, 395, 397, 400, 401, 401, 401,
      401, 403, 403, 403, 404, 405, 407, 409, 411, 413,
      414, 414, 414, 414, 414, 415, 415, 416, 417, 419,
      421, 421, 421, 422, 424, 426, 427, 427, 427, 428,
      429, 430, 433, 434, 434, 434, 434, 434, 434, 435,
      436, 436, 438, 440, 441, 443, 443, 443, 443, 444,
      447, 448, 448, 449, 449, 451, 455, 457, 458, 458,
      458, 459, 460, 460, 460, 460, 461, 466, 468, 469,
      473, 476, 476, 478, 479, 481, 482, 484, 487, 490,
      493, 500, 514, 516, 516, 516, 516, 517, 517, 518};

    static const double pi=3.141592654;
    static const double fac2fixed=4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19 
    static const double mu=0.53;  //Debyes
    static const double mmol=49.0;

    double q=0.664313224*pow(tt.get("K"),1.5);
    unsigned int vp;
    unsigned int ini;
    unsigned int ifin;
    complex<double>  lshape;
    complex<double>  lshapeacum;
    
    if(nu.get("GHz")>999.9){
      
      return complex<double> (0.0,0.0);
      
    }else{
            
      if(nu.get("GHz")<1.0){
	vp=0;
      }else{
	vp = (int) round((nu.get("GHz")+1.0)/2.0);
	vp=vp-1;
      }
        
      if(pp.get("mb")<100){
	ini=ini3[vp]; 
	ifin=ifin3[vp];
      }else{
      	if(pp.get("mb")<300){
      	  ini=ini2[vp];
      	  ifin=ifin2[vp];
      	}else{
      	  ini=ini1[vp];
      	  ifin=ifin1[vp];
      	}
      }

      if(ini>0){ini=ini-1;}else{ifin=0;}
      if(ifin>0){ifin=ifin-1;}else{ifin=0;}

      if(ifin==0||ifin<ini){

	return complex<double> (0.0,0.0);

      }else{
	
	for(unsigned int i=ini; i<ifin+1; i++){

	  lshape=lineshape(nu,Frequency(fre[i],"GHz"),linebroadening(Frequency(fre[i],"GHz"),tt,pp,mmol,Frequency(2.5,"MHz"),0.76),Frequency(0.0,"GHz"));

	  lshape=lshape*flin[i]*exp(-el[i]/tt.get("K"))*fre[i]; 

          lshapeacum=lshapeacum+lshape;
	  
	}
	
	lshapeacum=lshapeacum*(nu.get("GHz")/pi)*(0.047992745509/tt.get("K"))*(fac2fixed*pow(mu,2.0)/q);  // imaginary part: absorption coefficient in cm^2
	                                                                                                  // real part: delay in rad*cm^2    

	return lshapeacum*1e-4;    // to give it in SI units (m^2)    // (  rad m^2 , m^2 )

	
      }
     
    }

  }


}
