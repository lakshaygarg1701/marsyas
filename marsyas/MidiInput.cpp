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
    \class MidiInput
    \brief Update midi inpput using controls. Just through for dataflow. 
*/




#include "MidiInput.h"
using namespace std;



MidiInput::MidiInput(string name)
{
  type_ = "MidiInput";
  name_ = name;
  midiin = NULL;
  addControls();
}


MidiInput::~MidiInput()
{
  delete midiin;
}


MarSystem* 
MidiInput::clone() const 
{
  return new MidiInput(*this);
}

void 
MidiInput::addControls()
{
  addDefaultControls();
}


void
MidiInput::update()
{
  MRSDIAG("MidiInput.cpp - MidiInput:update");
  setctrl("natural/onSamples", getctrl("natural/inSamples"));
  setctrl("natural/onObservations", getctrl("natural/inObservations"));
  setctrl("real/osrate", getctrl("real/israte"));
  setctrl("string/onObsNames", getctrl("string/inObsNames"));



  
  try { 
    midiin = new RtMidiIn();
  } 
  catch (RtError &error) { 
    error.printMessage();
    return;
  }
  
  
  try { 
    midiin->openPort(0);
  }
  catch (RtError &error) 
    {
      error.printMessage();
      return;
    } 
  midiin->setCallback(&MidiInput::mycallback, this);
  midiin->ignoreTypes(false, false, false);
  



  defaultUpdate();  
}


void 
MidiInput::mycallback(double deltatime, std::vector< unsigned char > * message, void *userData) 
{
  int nBytes = 0;
  nBytes = message->size();

  MidiInput* mythis = (MidiInput*) userData;
  

  if (nBytes > 0) 
    {
      if (nBytes > 2) 
	{
	  mythis->byte3 = message->at(2); 
	  mythis->byte2 = message->at(1);
	  mythis->type = message->at(0);
	}

      if ((mythis->type == 160)&&(mythis->byte2 == 9))
	{
	  mythis->rval = mythis->byte3;
	}

    }
  
  
  
  
}

void 
MidiInput::process(realvec& in, realvec& out)
{
  checkFlow(in,out);

  
  for (o=0; o < inObservations_; o++)
    for (t = 0; t < inSamples_; t++)
      {
	out(o,t) =  in(o,t);
      }


}







	
