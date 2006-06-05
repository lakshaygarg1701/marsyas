/*
** Copyright (C) 1998-2005 George Tzanetakis <gtzan@cs.uvic.ca>
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
    \class PvConvert
    \brief PvConvert

    PvConvert N real and imaginary spectrum values to 
N/2+1 pairs of magnitude and phase values. The phases 
are unwrapped and successive phase differences are 
used to compute estimates of the instantaneous frequencies 
for each phase vocoder analysis channel; decimation rate 
and sampling rate are used to render these frequencies 
directly in Hz. 
*/

#include "PvConvert.h"

#include <algorithm> 
using namespace std;


PvConvert::PvConvert(string name)
{
  type_ = "PvConvert";
  name_ = name;
  psize_ = 0;
  size_ = 0;
  
  addControls();
}


PvConvert::~PvConvert()
{
}

MarSystem* 
PvConvert::clone() const 
{
  return new PvConvert(*this);
}


void 
PvConvert::addControls()
{
  addDefaultControls();
  addctrl("natural/Decimation",MRS_DEFAULT_SLICE_NSAMPLES/4);
  addctrl("natural/Sinusoids", 1);
  setctrlState("natural/Sinusoids", true);
  
  
}




void
PvConvert::update()
{
  
  setctrl("natural/onSamples", getctrl("natural/inSamples"));
  setctrl("natural/onObservations", getctrl("natural/inObservations").toNatural() + 2);
  setctrl("real/osrate", getctrl("real/israte").toReal() * getctrl("natural/inObservations").toNatural());  

  defaultUpdate();  
  
  size_ = onObservations_ /2 +1;
  
  if (size_ != psize_)
    {
      lastphase_.stretch(size_);
      phase_.stretch(size_);
      mag_.stretch(size_);
      sortedmags_.stretch(size_);
      sortedpos_.stretch(size_);
    }
  
  psize_ = size_;
  
  
  factor_ = ((getctrl("real/osrate").toReal()) / 
	     (real)( getctrl("natural/Decimation").toNatural()* TWOPI));
  fundamental_ = (real) (getctrl("real/osrate").toReal() / (real)getctrl("natural/inObservations").toNatural());
  kmax_ = getctrl("natural/Sinusoids").toNatural();

}





void 
PvConvert::process1(realvec& in, realvec& out)
{

  
  checkFlow(in,out); 
  
  natural N2 = inObservations_/2;
  real a;
  real b;
  real phasediff;

  // handle amplitudes
  for (t=0; t <= N2; t++)
    {
      if (t==0)
	{
	  a = in(2*t,0);
	  b = 0.0;
	}
      else if (t == N2)
	{
	  a = in(1, 0);
	  b = 0.0;
	}
      else
	{
	  a = in(2*t, 0);
	  b = in(2*t+1, 0);
	}
      
      // computer magnitude value 
      mag_(t) = sqrt(a*a + b*b);
      // sortedmags_(t) = mag_(t);
      // compute phase
      phase_(t) = -atan2(b,a);
      
    }

  bool found = false;
  
  for (t=2; t <= N2; t++)
    {

      real val = mag_(t);
      if ((val > mag_(t-1)) && (val > mag_(t+1))) 
	found = true;
      else
	found = false;
      
      
      out(2*t,0) = 0.0;
      out(2*t+1,0) = t * fundamental_;
      
      if (found) 
	{
	  if (val == 0.0) 
	    phasediff = 0.0;
	  else 
	    {
	      out(2*t,0) = val;
	      phasediff = phase_(t) - lastphase_(t);
	      lastphase_(t) = phase_(t);	
	    }

	  
	  // phase unwrapping 
	  while (phasediff > PI) 
	    phasediff -= TWOPI;
	  while (phasediff < -PI) 
	    phasediff += TWOPI;
	  
	  
	  out(2*t+1, 0) = phasediff * factor_ + t * fundamental_;      
	}
    }
  
}



void 
PvConvert::process(realvec& in, realvec& out)
{
  
  checkFlow(in,out); 

  
  natural N2 = inObservations_/2;
  real a;
  real b;
  real phasediff;

  // handle amplitudes
  for (t=0; t <= N2; t++)
    {
      if (t==0)
	{
	  a = in(2*t,0);
	  b = 0.0;
	}
      else if (t == N2)
	{
	  a = in(1, 0);
	  b = 0.0;
	}
      else
	{
	  a = in(2*t, 0);
	  b = in(2*t+1, 0);
	}
      
      // computer magnitude value 
      mag_(t) = sqrt(a*a + b*b);
      sortedmags_(t) = mag_(t);
      // compute phase
      phase_(t) = -atan2(b,a);
      
    }


  real* data = sortedmags_.getData();
  sort(data, data+(N2+1), greater<real>());
  
  bool found;
  real val;
  

  for (t=0; t <= N2; t++)
    {
      found = false;
      val = mag_(t);
      
      for (c=0; c < kmax_; c++)
	 {
	   if (val == sortedmags_(c))
	    {
	      found = true;
	      break;
	    }
	}
      out(2*t,0) = 0.0;
      out(2*t+1,0) = t * fundamental_;
      

      if (found) 
	{
	  if (val == 0.0) 
	    phasediff = 0.0;
	  else 
	    {
	      out(2*t,0) = val;
	      phasediff = phase_(t) - lastphase_(t);
	      lastphase_(t) = phase_(t);	
	    }


	  // phase unwrapping 
	  while (phasediff > PI) 
	    phasediff -= TWOPI;
	  while (phasediff < -PI) 
	    phasediff += TWOPI;


	  out(2*t+1, 0) = phasediff * factor_ + t * fundamental_;      
	}
    }
  
  
}






	

	

	
	
      
