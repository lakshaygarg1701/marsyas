/*
** Copyright (C) 1998-2006 George Tzanetakis <gtzan@cs.uvic.ca>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/** 
    \class MFCC
    \brief MFCC Mel-Frequency Cepstral Coefficients

    Mel-Frequency cepstral coefficients are features frequently 
used in Speech Recognition. The code is based on the correspdonging 
function in the Auditory Toolbox by Malcolm Slaney. 
*/



#include "MFCC.h"
using namespace std;


MFCC::MFCC(string name)
{
  type_ = "MFCC";
  name_ = name;
  pfftSize_ = 0;
  psamplingRate_ = 0;
  addControls();
}


MFCC::~MFCC()
{
}


MarSystem* 
MFCC::clone() const 
{
  return new MFCC(*this);
}

void 
MFCC::addControls()
{
  addDefaultControls();
}


void
MFCC::update()
{
  
  MRSDIAG("MFCC.cpp - MFCC:update");
  
  setctrl("natural/onSamples", (natural)1);
  setctrl("natural/onObservations", (natural)13);
  setctrl("real/osrate", getctrl("real/israte"));

  // Initialize frequency boundaries for filters 

  natural i,j;


  fftSize_ = 2 * getctrl("natural/inObservations").toNatural();
  samplingRate_ = (natural) (getctrl("real/israte").toReal() * getctrl("natural/inObservations").toNatural() * 2);


  

  
  if ((pfftSize_ != fftSize_) || (psamplingRate_ != samplingRate_))
    {
      
      cepstralCoefs_ = 13;
      ostringstream oss;
      
      for (i=0; i < cepstralCoefs_; i++)
	oss << "MFCC_" << i << ",";
      setctrl("string/onObsNames", oss.str());
      
      freqs_.create(42);
      lowestFrequency_ = 133.3333f;
      linearFilters_ = 13;
      linearSpacing_ = 66.66666f;
      logFilters_ = 27;
      logSpacing_ = 1.0711703f;
      
      totalFilters_ = linearFilters_ + logFilters_;
      lower_.create(totalFilters_);
      center_.create(totalFilters_);
      upper_.create(totalFilters_);
      triangle_heights_.create(totalFilters_);
      
      
      // Linear filter boundaries

      
      for (i=0; i< linearFilters_; i++)
	freqs_(i) = lowestFrequency_ + i * linearSpacing_;
      
      // Logarithmic filter boundaries  
      float first_log = freqs_(linearFilters_-1);
      for (i=1; i<=logFilters_+2; i++)
	{
	  freqs_(linearFilters_-1+i) = first_log * pow(logSpacing_, (real)i);
	}  
      
      
      // Triangles information
      for (i=0; i<totalFilters_; i++)
	lower_(i) = freqs_(i);
      
      for (i=1; i<= totalFilters_; i++)
	center_(i-1) = freqs_(i);
      
      for (i=2; i<= totalFilters_+1; i++)
	upper_(i-2) = freqs_(i);
      
      for (i=0; i<totalFilters_; i++)
	triangle_heights_(i) = (real)(2.0 / (upper_(i) - lower_(i)));
      
      
      
      fftFreqs_.stretch(fftSize_);
      cepstralCoefs_ = 13;
      
      for (i=0; i< fftSize_; i++)
	fftFreqs_(i) = (float)i / (float)fftSize_ * (float)samplingRate_;
      
      mfccFilterWeights_.create(totalFilters_, fftSize_);
      mfccDCT_.create(cepstralCoefs_, totalFilters_);
      
      natural chan;
      
      // Initialize mfccFilterWeights
      for (chan = 0; chan < totalFilters_; chan++)
	for (i=0; i< fftSize_; i++)
	  {
	    if ((fftFreqs_(i) > lower_(chan))&& (fftFreqs_(i) <= center_(chan)))
	      {
		mfccFilterWeights_(chan, i) = triangle_heights_(chan) *
		  ((fftFreqs_(i) - lower_(chan))/(center_(chan) - lower_(chan)));
	      }
	    if ((fftFreqs_(i) > center_(chan)) && (fftFreqs_(i) <= upper_(chan)))
	      {
		mfccFilterWeights_(chan, i) = triangle_heights_(chan) *
		  ((upper_(chan) - fftFreqs_(i))/(upper_(chan) - center_(chan)));
	      }
	  }
      
      // Initialize MFCC_DCT
      real scale_fac = (real)(1.0/ sqrt((real)(totalFilters_/2)));
      for (j = 0; j<cepstralCoefs_; j++)
	for (i=0; i< totalFilters_; i++)
	  {
	    mfccDCT_(j, i) = scale_fac * cos(j * (2*i +1) * PI/2/totalFilters_);
	    if (i == 0)
	      mfccDCT_(j,i) *= (real)(sqrt(2.0)/2.0);
	  }  
    }
  
  pfftSize_ = fftSize_;
  psamplingRate_ = samplingRate_;
  
  
  

  natural inSize = getctrl("natural/inObservations").toNatural();  
  fmagnitude_.stretch(inSize*2);
  earMagnitude_.stretch(totalFilters_);


  
  defaultUpdate();
}


void 
MFCC::process(realvec& in, realvec& out)
{
  checkFlow(in,out);

  natural i,k;
  
  // mirror the spectrum 
  for (o=0; o < inObservations_; o++)
    fmagnitude_(o) = in(o,0);
  for (o=0; o< inObservations_; o++)
    fmagnitude_(o+ inObservations_) = fmagnitude_(inObservations_ - o);
  
  
  real sum =0.0;
  // Calculate the filterbank responce
  for (i=0; i<totalFilters_; i++)
    { 
      sum = 0.0;
      for (k=0; k<fftSize_; k++)
	{
	  sum += (mfccFilterWeights_(i, k) * fmagnitude_(k));
	}
      if (sum != 0.0)
	earMagnitude_(i) = log10(sum);
      else 
	earMagnitude_(i) = 0.0;
    }
  
  // Take the DCT 
  for (o=0; o < cepstralCoefs_; o++)
    {
      sum =0.0;
      for (k=0; k < totalFilters_; k++)
	{
	  sum += (mfccDCT_(o,k) * earMagnitude_(k));
	}
      out(o,0) = sum;
    }  
}







	

	
