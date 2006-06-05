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
    \class NetworkUDPSource
    \brief NetworkUDPSource Source using UDP for communication 


*/

#ifndef MARSYAS_NETWORKUDPSOURCE_H
#define MARSYAS_NETWORKUDPSOURCE_H

#include "NetworkSocket.h"	


class NetworkUDPSource: public NetworkSocket
{
 private: 
 	
  void addControls(); 
 	
 public:
  	
  NetworkUDPSource( std::string name );
  NetworkUDPSource();
  virtual ~NetworkUDPSource();
  MarSystem* clone() const;  
  std::string getClientAddr();
	  
  void update();
  void process( realvec& in, realvec& out );
  void refresh();
};


#endif 
