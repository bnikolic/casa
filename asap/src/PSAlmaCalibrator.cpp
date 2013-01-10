//
// C++ Implementation: PSAlmaCalibrator
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <assert.h>

#include "PSAlmaCalibrator.h"

using namespace casa;

namespace asap {

PSAlmaCalibrator::PSAlmaCalibrator()
  : Calibrator()
{}

PSAlmaCalibrator::PSAlmaCalibrator(unsigned int nchan)
  : Calibrator(nchan)
{}

void PSAlmaCalibrator::calibrate()
{
  assert(source_);
  assert(ref_);
  assert(scaler_);
  assert(calibrated_);

  Float *p_s = source_;
  Float *p_r = ref_;
  Float *p_c = calibrated_;
  for (unsigned int i = 0; i < nchan_; i++) {
    *p_c = *p_s / *p_r - 1.0;
    p_s++;
    p_r++;
    p_c++;
  }
  
  p_c = calibrated_;
  p_s = scaler_;
  if (nchanS_ == 1) {
    for (unsigned int i = 0; i < nchan_; i++) {
      *p_c *= *p_s;
      p_c++;
    }
  }
  else {
    for (unsigned int i = 0; i < nchan_; i++) {
      *p_c *= *p_s;
      p_c++;
      p_s++;
    }
  }
}

}
