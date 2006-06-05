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
    \class GaussianClassifier
    \brief Single Gaussian multidimensional classifier

   Simple multidimensional Gaussian classifier. The classifier
parameters (or model) are stored in the the theta control.  When the
mode control is set to "train" the input slice is used to accumulate
information for training.  The actual final theta calculation is
completed when the control train is set to true. That can accomodate
non-incremental or batch training. The labels control is used to
provide ground truth about the label(s). The output of the classifier is
the ground truth label(s) when the mode control is train.

When the mode control is set to "predict" then the output 
of the classifier is the predicted labels using the trained 
parameter vector theta. 

This MarSystems serves as a prototypical classification/regression 
MarSystem. 

*/





#include "GaussianClassifier.h"
using namespace std;


GaussianClassifier::GaussianClassifier(string name)
{
  type_ = "GaussianClassifier";
  name_ = name;
  addControls();
}


GaussianClassifier::~GaussianClassifier()
{
}


MarSystem* 
GaussianClassifier::clone() const 
{
  return new GaussianClassifier(*this);
}

void 
GaussianClassifier::addControls()
{
  addDefaultControls();
  addctrl("string/mode", "train");
  addctrl("natural/nLabels", 1);
  setctrlState("natural/nLabels", true);
  means_.create(1);
  covars_.create(1);
  addctrl("realvec/means", means_);
  addctrl("realvec/covars", covars_);
  addctrl("bool/done", false);
  setctrlState("bool/done", true);

  
}


void
GaussianClassifier::update()
{
  MRSDIAG("GaussianClassifier.cpp - GaussianClassifier:update");

  setctrl("natural/onSamples", getctrl("natural/inSamples"));
  setctrl("natural/onObservations", (natural)2);
  setctrl("real/osrate", getctrl("real/israte"));
  
  
  natural inObservations = getctrl("natural/inObservations").toNatural();
  natural nlabels = getctrl("natural/nLabels").toNatural();


  natural mrows = (getctrl("realvec/means").toVec()).getRows();
  natural mcols = (getctrl("realvec/means").toVec()).getCols();
  natural nrows = means_.getRows();
  natural ncols = means_.getCols();
  
  
  
  if ((nlabels != mrows) || 
      (inObservations != mcols))
    {
      means_.create(nlabels, inObservations);
      covars_.create(nlabels, inObservations);
      updctrl("realvec/means", means_);
      updctrl("realvec/covars", covars_);      
      labelSizes_.create(nlabels);  
    }
  

  if ((nlabels != nrows) || 
      (inObservations != ncols))
    {
      means_.create(nlabels, inObservations);
      covars_.create(nlabels, inObservations);
    }
  
  string mode = getctrl("string/mode").toString();
  if (mode == "predict")
    {
      means_ = getctrl("realvec/means").toVec();
      covars_ = getctrl("realvec/covars").toVec();
    }

  defaultUpdate();
}


void 
GaussianClassifier::process(realvec& in, realvec& out)
{
  checkFlow(in,out);
  real v;
  string mode = getctrl("string/mode").toString();

  


  natural nlabels = getctrl("natural/nLabels").toNatural();
  natural l;
  natural prediction = 0;
  real label;
  

  real diff;
  real sq_sum=0.0;

  if (mode == "train")  
    {
      for (t = 0; t < inSamples_; t++)  
	{
	  label = in(inObservations_-1, t);
	  
	  for (o=0; o < inObservations_-1; o++)
	    {
	      v = in(o,t);
	      
	      means_((natural)label,o) = means_((natural)label,o) + v;
	      covars_((natural)label,o) = covars_((natural)label,o) + v*v;
	      out(0,t) = (real)label;	      
	      out(1,t) = (real)label;
	      
	    }
	  labelSizes_((natural)label) = labelSizes_((natural)label) + 1;
	  
	}
    }



  if (mode == "predict")
    {
      real min = 10000000.0;
      for (t = 0; t < inSamples_; t++)  
	{
	  label = in(inObservations_-1, t);

	  for (l=0; l < nlabels; l++)
	    {
	      sq_sum = 0.0;
	      
	      for (o=0; o < inObservations_-1; o++)
		{
		  v = in(o,t);
		  diff = (v - means_(l,o));
		  sq_sum += (diff * covars_(l,o) * diff);
		}
	      if (sq_sum < min)
		{
		  min = sq_sum;
		  prediction = l;
		}
	    }
	  
	  out(0,t) = (real)prediction;
	  out(1,t) = (real)label;
	}
    }
  
  if (getctrl("bool/done").toBool())
    {
      
      for (l=0; l < nlabels; l++)
	for (o=0; o < inObservations_; o++)
	  {
	    means_(l,o) = means_(l,o) / labelSizes_(l);
	    covars_(l,o) = covars_(l,o) / labelSizes_(l);
	    covars_(l, o) = covars_(l,o) - 
	      (means_(l,o) * means_(l,o));
	    if (covars_(l,o) != 0.0)
	      {
		covars_(l,o) = (real)(1.0 / covars_(l,o));
	      }
	  }
      updctrl("realvec/means", means_);
      updctrl("realvec/covars", covars_);
    }

  
}







	

	
