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
    \class PvConvolve
    \brief PvConvolve


*/

#include "PvConvolve.h"
using namespace std;


PvConvolve::PvConvolve(string name)
{
  type_ = "PvConvolve";
  name_ = name;
  
  addControls();
}


PvConvolve::~PvConvolve()
{
}

MarSystem* 
PvConvolve::clone() const 
{
  return new PvConvolve(*this);
}


void 
PvConvolve::addControls()
{
  addDefaultControls();
}




void
PvConvolve::update()
{
  setctrl("natural/onSamples", getctrl("natural/inSamples"));
  setctrl("natural/onObservations", getctrl("natural/inObservations").toNatural() / 2);
  setctrl("real/osrate", getctrl("real/israte").toReal());
  defaultUpdate();  
}





void 
PvConvolve::process(realvec& in, realvec& out)
{
  
  checkFlow(in,out); 


  natural N2 = onObservations_ / 2;
  real sqN = (real)N2 * N2;
  
  real r1, i1, r2, i2;
  
  for (o=0; o < N2; o++)  
    {
      
      if (o==0) 
	{
	  r1 = in(2*o,0);
	  i1 = 0.0;
	  r2 = in(2*o + onObservations_, 0);
	  i2 = 0.0;
	  out(2*o, 0) = r1 * r2;
	}
      else if (o == N2) 
	{
	  r1 = in(1,0);
	  i1 = 0.0;
	  r2 = in(1 + onObservations_, 0);
	  i2 = 0.0;
	  out(1,0) = r1 * r2;
	}
      else
	{
	  r1 = in(2*o,0);
	  i1 = in(2*o+1,0);
	  r2 = in(2*o + onObservations_, 0);
	  i2 = in(2*o+1 + onObservations_, 0);
	  
	  out(2*o,0) = (r1 * r2 - i1 * i2) * sqN;
	  out(2*o+1, 0) = (r1 * i2 + r2 * i1) * sqN;
	  
	  // out(2*o,0) = r1;
	  // out(2*o+1, 0) = i1;
	  
	}
      


    }
  

  


  
}






	

	

	
	
      
	
