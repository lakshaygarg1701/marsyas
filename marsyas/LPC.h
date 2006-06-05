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
     \class LPC
    \brief Compute LPC coefficients, Pitch and Power of window.

    
*/

#ifndef MARSYAS_LPC_H
#define MARSYAS_LPC_H

#include "MarSystem.h"	
#include "Conversions.h"
#include "AutoCorrelation.h"
#include "Series.h" 
#include "HalfWaveRectifier.h" 
#include "Gain.h" 
#include "Fanout.h"
#include "TimeStretch.h"
#include "Fanin.h" 
#include "Negative.h"
#include "Peaker.h"
#include "MaxArgMax.h"

class LPC: public MarSystem
{
private: 
  void addControls();
  
  natural inSize_;
  natural outSize_;
  natural order_;
  realvec rmat_;
  realvec corr_;
  realvec pres_;
  realvec Zs_;
  realvec temp_;
  real pitch_;
  real power_;
  real minPitchRes_;
  realvec pitchres_;
  realvec pitExOut_;
  realvec fanoutOut_;
  realvec faninOut_;
  real lowFreq_;
  real highFreq_;
  natural highSamples_;
  natural lowSamples_;
  natural firstTime_;
  
  MarSystem* pitchExtractor_;
  MarSystem* fanout_;
  MarSystem* fanin_;
  MarSystem* pitchExtractorEnd_;
  AutoCorrelation* autocorr_;
  natural hopSize_;
  bool networkCreated_;
  
public:
  LPC();
  LPC(std::string name);
  
  ~LPC();
  MarSystem* clone() const;  
  
  void update();
  void predict(realvec& data, realvec& coeffs);
  void process(realvec& in, realvec& out);
  

};

#endif


	
