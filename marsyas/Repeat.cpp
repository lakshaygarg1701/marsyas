/*
** Copyright (C) 1998-2005 George Tzanetakis <gtzan@cs.uvic.ca>
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
   \class Repeat
   \brief 
*/
#include "Repeat.h"
using namespace std;


Repeat::Repeat() { set(false,"",0); }
Repeat::Repeat(string time_interval) { set(true,time_interval,0); }
Repeat::Repeat(string time_interval, natural rep_count) {
    set(false,time_interval,rep_count);
}
void Repeat::set(bool inf, string time_interval, natural rep_count) {
    infinite=inf;
    interval=time_interval;
    count=rep_count;
}
natural Repeat::interval2samples(real srate) {
    return time2samples(interval,srate);
}
