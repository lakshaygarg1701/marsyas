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
    \class PvFold
    \brief PvFold

    Multiply with window (both length Nw) using modulus arithmetic;
fold and rotate windowed input into output array (FFT) (length N) 
according to current input time (t)
*/



#include "PvFold.h"
using namespace std;

PvFold::PvFold(string name):MarSystem()
{
  type_ = "PvFold";
  name_ = name;
  addControls();
}


PvFold::~PvFold()
{
}

MarSystem* 
PvFold::clone() const 
{
  return new PvFold(*this);
}


void 
PvFold::addControls()
{
  addDefaultControls();
  addctrl("natural/WindowSize", MRS_DEFAULT_SLICE_NSAMPLES);
  addctrl("natural/FFTSize", MRS_DEFAULT_SLICE_NSAMPLES);
  setctrlState("natural/FFTSize", true);
  addctrl("natural/Decimation", MRS_DEFAULT_SLICE_NSAMPLES/2);
  setctrlState("natural/Decimation", true);
  n_ = 0;
}



void
PvFold::update()
{
  setctrl("natural/onSamples", getctrl("natural/FFTSize"));
  setctrl("natural/onObservations", (natural)1);
  setctrl("real/osrate", getctrl("real/israte"));  

  
  N_ = getctrl("natural/onSamples").toNatural();
  Nw_ = getctrl("natural/inSamples").toNatural();
  D_ = getctrl("natural/Decimation").toNatural();



  // create analysis window if necessary
  if (Nw_ != PNw_)
    {
      n_ = -Nw_;
      awin_.stretch(Nw_);
      for (t=0; t < Nw_; t++)
	{
	  awin_(t) = (real)(0.54 - 0.46 * cos(TWOPI * t/(Nw_-1)));
	}
      /* when Nw_ > N also apply interpolating (sinc) windows 
       * to ensure that window are 0 at increments of N (the 
       * FFT length) aways from the center of the analysis
       * window 
       */ 
      if (Nw_ > N_) 
	{
	  real x;
	  x = (real)(-(Nw_ -1) / 2.0);
	  for (t=0; t < Nw_; t++, x += 1.0)
	    {
	      if (x != 0.0) 
		awin_(t) *= N_ * sin (PI * x/N_) / (PI *x);
	    }
	}
      /* normalize window for unit gain */ 
      real sum = 0.0;
      
      for (t =0; t < Nw_; t++)
	{
	  sum += awin_(t);
	}
      
      real afac = (real)(2.0/ sum);
      awin_ *= afac;
    }

  PNw_ = Nw_;
  
  defaultUpdate();
}






void 
PvFold::process(realvec& in, realvec& out)
{
  checkFlow(in,out);
  
  n_ += D_;

  for (t=0; t < N_; t++)
    {
      out(0,t) = 0.0;
    }
  while (n_ < 0)
    n_ += N_;
  n_ %= N_;
  for (t=0; t < Nw_; t++)
    {
      out(0,n_) += in(0,t)*awin_(t);
      if (++n_ == N_)
	n_ = 0;
    }
}






	

	

	
