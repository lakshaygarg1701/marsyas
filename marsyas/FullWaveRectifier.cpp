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
    \class FullWaveRectifier
    \brief FullWaveRectifier


*/



#include "FullWaveRectifier.h"
using namespace std;



FullWaveRectifier::FullWaveRectifier(string name)
{
  type_ = "FullWaveRectifier";
  name_ = name;
  addControls();
}


FullWaveRectifier::~FullWaveRectifier()
{
}


MarSystem* 
FullWaveRectifier::clone() const 
{
  return new FullWaveRectifier(*this);
}

void 
FullWaveRectifier::addControls()
{
  addDefaultControls();
}


void
FullWaveRectifier::update()
{
  MRSDIAG("FullWaveRectifier.cpp - FullWaveRectifier:update");
  
  setctrl("natural/onSamples", getctrl("natural/inSamples"));
  setctrl("natural/onObservations", getctrl("natural/inObservations"));
  setctrl("real/osrate", getctrl("real/israte"));
  defaultUpdate();
}


void 
FullWaveRectifier::process(realvec& in, realvec& out)
{
  
  checkFlow(in,out);

  for (o=0; o < inObservations_; o++)
    for (t = 0; t < inSamples_; t++)
      {
	if (in(o,t) < 0.0) 
	  out(o,t) = -in(o,t);
	else 
	  out(o,t) = in(o,t);
      }


  

  
}







	

	
