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
    \class FM 
    \brief FM synthesis source

   Simple FM synthesis MarSystem.

*/


#include "FM.h"
using namespace std;


FM :: FM( string name ) 
{
  
  type_ = "FM";
  name_ = name;
  
  // create the wavetable.
  wavetableSize_ = 8192;
  wavetable_.create((natural)wavetableSize_);
  
  real incr = TWOPI / wavetableSize_;
  for (t=0; t < wavetableSize_; t++) {
    wavetable_(t) = (real)(0.5 * sin(incr * t));
    
    
  }
  
  mIndex_ = 0;
  oIndex_ = 0;
  
  addControls();
  
}


FM :: ~FM() {}


MarSystem* FM::clone() const 
{
  return new FM(*this);
}


void
FM::addControls() 
{
  addDefaultControls();
  addctrl("natural/nChannels",1);
  
  addctrl("real/mDepth", 15.0);						// modulator depth
  setctrlState("real/mDepth",true);
  
  addctrl("real/mSpeed", 6.0);						// modulator speed
  setctrlState("real/mSpeed", true);
  
  addctrl("real/cFrequency", 1000.0);			// carrier frequency
  setctrlState("real/cFrequency", true);
  
  addctrl("bool/noteon", false);
  setctrlState("bool/noteon", true);

}	


void FM::update() 
{
  MRSDIAG("FM.cpp - FM:update");
  
  setctrl("natural/onSamples", getctrl("natural/inSamples"));
  setctrl("natural/onObservations", getctrl("natural/inObservations"));
  setctrl("real/osrate", getctrl("real/israte"));
  
  // update the controls for the FM  
  cFrequency_ = getctrl("real/cFrequency").toReal();
  isRate_ = getctrl("real/israte").toReal();
  mSpeed_ = getctrl("real/mSpeed").toReal();
  mDepth_ = getctrl("real/mDepth").toReal();
  mRate_ = (mSpeed_ * wavetableSize_) / getctrl("real/israte").toReal();
  inSamples_ = getctrl("natural/inSamples").toNatural();
  
  defaultUpdate();
}	

void FM::process( realvec& in, realvec& out ) 
{
  checkFlow(in,out);
  

  register real mSample_;
  register real oFrequency_;								
											
  if (getctrl("bool/noteon").toBool() == false) {
  	return;
  }
  
  for (t=0; t < inSamples_; t++)  
    {
      
      // calculate the modulator output
      mSample_ = wavetable_((natural)mIndex_);
      mIndex_ += mRate_;
      mSample_ *= mDepth_;
      
      // calculate any FM and the new output rate
      oFrequency_ = cFrequency_ + mSample_;
      oRate_ = (oFrequency_ * wavetableSize_) / isRate_;
      
      out(0,t) = wavetable_((natural)oIndex_);
      
      // we are one sample behind in case this index goes off the map
      oIndex_ += oRate_;    
      
      while (mIndex_ >= wavetableSize_)
	mIndex_ -= wavetableSize_;				
      
      while (oIndex_ >= wavetableSize_)
	oIndex_ -= wavetableSize_;  
    }
  
}
