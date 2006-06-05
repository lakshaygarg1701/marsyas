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
    \class AbsMax
    \brief Calculate the maximum of the absolute values of the signal 
    
Calculates the maximum of the absolute values of 
the singal. Used for sending waveform plots to editor. 
*/

#ifndef MARSYAS_ABSMAX_H
#define MARSYAS_ABSMAX_H

#include "MarSystem.h"	

class AbsMax: public MarSystem
{
private:
  real max_;
  
  void addControls();
  
public:
  AbsMax(std::string name);
  ~AbsMax();
  
  MarSystem* clone() const;
  void update();
  void process(realvec& in, realvec& out);
  
  
};

#endif

	

	

	
