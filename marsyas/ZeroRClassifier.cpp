/*
** Copyright (C) 1998-2006 George Tzanetakis <gtzan@cs.princeton.edu>
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
    \class ZeroRClassifier

    When the mode control is set to "predict" then the output 
    of the classifier will output the class with the most instances. 

    This MarSystems serves as ground truth for classification/regression 
    MarSystem results.

*/



#include "ZeroRClassifier.h"
using namespace std;


ZeroRClassifier::ZeroRClassifier():MarSystem()
{
  type_ = "ZeroRClassifier";
}

ZeroRClassifier::ZeroRClassifier(string name)
{
  type_ = "ZeroRClassifier";
  name_ = name;
  addControls();
}


ZeroRClassifier::~ZeroRClassifier()
{
}


MarSystem* 
ZeroRClassifier::clone() const 
{
  return new ZeroRClassifier(*this);
}

void 
ZeroRClassifier::addControls()
{
  addDefaultControls();
  addctrl("string/mode", "train");
  addctrl("natural/nLabels", 1);
  setctrlState("natural/nLabels", true);
  addctrl("bool/done", false);
  addctrl("natural/prediction", 0);
}

void
ZeroRClassifier::update()
{
  MRSDIAG("ZeroRClassifier.cpp - ZeroRClassifier:update");
  
  setctrl("natural/onSamples", getctrl("natural/inSamples"));
  setctrl("natural/onObservations", (natural)2);
  setctrl("real/osrate", getctrl("real/israte"));
   
  natural nlabels = getctrl("natural/nLabels").toNatural();

  if (labelSizes_.getSize() != nlabels)
    labelSizes_.create(nlabels);  
  string mode = getctrl("string/mode").toString();
  if (mode == "predict")
    {
    
    }
  defaultUpdate();
}

void 
ZeroRClassifier::process(realvec& in, realvec& out)
{
  checkFlow(in,out);
  string mode = getctrl("string/mode").toString();
  natural nlabels = getctrl("natural/nLabels").toNatural();
  natural l;
  natural prediction = 0;
  
  real label;
  
  if (mode == "train")  
    {
      for (t=0; t < inSamples_; t++)
	{
	  label = in(inObservations_-1, t);	
	  labelSizes_((int)label) = labelSizes_((int)label) + 1;
	  out(0,t) = label;
	  out(1,t) = label;
	}
    }


  if (mode == "predict")
    {
      for (t=0; t < inSamples_; t++)
	{    
	  label = in(inObservations_-1, t);	  
	  prediction = getctrl("natural/prediction").toNatural();
	  out(0,t) = (real)prediction;
	  out(1,t) = label;
	}

    }
  
  if (getctrl("bool/done").toBool())
    {
      int max = -1;
      for (l=0; l < nlabels; l++)
	{
	  if (labelSizes_(l) > max)
	    {
	      prediction = l;
	      max = (int)labelSizes_(l);
	    }
	}
      updctrl("natural/prediction", prediction);
    }
}







	

	
