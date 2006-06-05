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
    \class Memory
    \brief Memory output the past memSize input observations. 

    Memory is essentially a circular buffer that holds 
past observations. It used to compute "texture" features 
-or dynamic features- which are stastics of short-term features 
-typically around 10-20 milliseconds- over a larger 
window -around 1 second-.
*/




#include "Memory.h"
using namespace std;



Memory::Memory(string name)
{
  type_ = "Memory";
  name_ = name;
  end_ = 0;
  addControls();
}


Memory::~Memory()
{
}


MarSystem* 
Memory::clone() const 
{
  return new Memory(*this);
}

void 
Memory::addControls()
{
  addDefaultControls();
  addctrl("natural/memSize", 40);
  setctrlState("natural/memSize", true);
  addctrl("bool/reset", false);
  setctrlState("bool/reset", true);
}


void
Memory::update()
{
  MRSDIAG("Memory.cpp - Memory:update");
  
  
  natural memSize = getctrl("natural/memSize").toNatural();
  
  setctrl("natural/onSamples", getctrl("natural/inSamples").toNatural() * memSize);
  setctrl("natural/onObservations", getctrl("natural/inObservations"));
  setctrl("real/osrate", getctrl("real/israte").toReal());

  reset_ = getctrl("bool/reset").toBool();

  defaultUpdate();
  
  ostringstream oss;
  string inObsNames = getctrl("string/inObsNames").toString();
  for (int i = 0; i < inObservations_; i++)
    {
      string inObsName;
      string temp;
      inObsName = inObsNames.substr(0, inObsNames.find(","));
      temp = inObsNames.substr(inObsNames.find(",")+1, inObsNames.length());
      inObsNames = temp;
      oss << "Mem" << memSize << "_" << inObsName << ",";
    }
  setctrl("string/onObsNames", oss.str());

}




void 
Memory::process(realvec& in, realvec& out)
{
  checkFlow(in,out);



  natural memSize = getctrl("natural/memSize").toNatural();

  if (reset_) 
    {
      out.setval(0.0);
      reset_ = false;
      setctrl("bool/reset", (MarControlValue)false);
      end_ = 0;
    }
  
  
  
  for (t = 0; t < inSamples_; t++)
    {
      for (o=0; o < inObservations_; o++)
	{
	  out(o, end_ * inSamples_) = in(o,t);
	}
    }
  end_ = (end_ + 1) % memSize; 		// circular buffer index 
  
}

      
      







	
	
	
