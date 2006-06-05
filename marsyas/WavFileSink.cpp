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
   \class WavFileSink
   \brief WavFileSink for .wav soundfiles
   
   SoundFileSink writer for .wav sound files.
*/



#include "WavFileSink.h"
using namespace std;



WavFileSink::WavFileSink(string name)
{
  type_ = "WavFileSink";
  name_ = name;
  sdata_ = NULL;
  cdata_ = NULL;
  addControls();
}


WavFileSink::~WavFileSink()
{
}

MarSystem* 
WavFileSink::clone() const
{
  return new WavFileSink(*this);
}


void 
WavFileSink::addControls()
{
  addDefaultControls();
  addctrl("natural/nChannels", (natural)1);
  addctrl("natural/channel", (natural)0);
  addctrl("string/filename", "default");
  setctrlState("string/filename", true);
}


bool 
WavFileSink::checkExtension(string filename)
{
  FileName fn(filename);
  string wavext  = "wav";
  string sndext = "snd";
  
  if (fn.ext() == wavext)
    return true;
  else 
    return false;
}

void 
WavFileSink::update()
{

  
  MRSDIAG("WavFileSink::update");

  setctrl("natural/onSamples", getctrl("natural/inSamples"));
  setctrl("natural/onObservations", getctrl("natural/inObservations"));
  setctrl("real/osrate", getctrl("real/israte"));


  natural nChannels = getctrl("natural/nChannels").toNatural();      
  
  delete sdata_;
  delete cdata_;
  
  sdata_ = new short[getctrl("natural/inSamples").toNatural() * nChannels];
  cdata_ = new unsigned char[getctrl("natural/inSamples").toNatural() * nChannels];
  
  filename_ = getctrl("string/filename").toString();
  
  defaultUpdate();
  
}

  
void 
WavFileSink::putHeader(string filename)
{


  natural nChannels = (natural)getctrl("natural/nChannels").toNatural();
  sfp_ = fopen(filename.c_str(), "wb");
  
  written_ = 0;
  
  /* prepare header */ 
  hdr_.riff[0] = 'R';
  hdr_.riff[1] = 'I';
  hdr_.riff[2] = 'F';
  hdr_.riff[3] = 'F';
  
  hdr_.file_size = 44;
  
  
  hdr_.wave[0] = 'W';
  hdr_.wave[1] = 'A';
  hdr_.wave[2] = 'V';
  hdr_.wave[3] = 'E';
  
  hdr_.fmt[0] = 'f';
  hdr_.fmt[1] = 'm';
  hdr_.fmt[2] = 't';
  hdr_.fmt[3] = ' ';
  
#if defined(__BIG_ENDIAN__)
  hdr_.chunk_size = ByteSwapLong(16);
  hdr_.format_tag = ByteSwapShort(1);
  hdr_.num_chans = ByteSwapShort((signed short)nChannels);
  hdr_.sample_rate = ByteSwapLong((natural)getctrl("real/israte").toReal());
  hdr_.bytes_per_sec = ByteSwapLong(hdr_.sample_rate * 2);
  hdr_.bytes_per_samp = ByteSwapShort(2);
  hdr_.bits_per_samp = ByteSwapShort(16);
  hdr_.data_length = ByteSwapLong(0);
#else  
  hdr_.chunk_size = 16;
  hdr_.format_tag = 1;
  hdr_.num_chans = (signed short)nChannels;
  hdr_.sample_rate = (natural)getctrl("real/israte").toReal();
  hdr_.bytes_per_sec = hdr_.sample_rate * 2;
  hdr_.bytes_per_samp = 2;
  hdr_.bits_per_samp = 16;
  hdr_.data_length = 0;
#endif   


  hdr_.data[0] = 'd';
  hdr_.data[1] = 'a';
  hdr_.data[2] = 't';
  hdr_.data[3] = 'a';
  

  
  fwrite(&hdr_, 4, 11, sfp_);
  
  sfp_begin_ = ftell(sfp_);  
}


unsigned long 
WavFileSink::ByteSwapLong(unsigned long nLongNumber)
{
  return (((nLongNumber&0x000000FF)<<24)+((nLongNumber&0x0000FF00)<<8)+
	  ((nLongNumber&0x00FF0000)>>8)+((nLongNumber&0xFF000000)>>24));
}
 


unsigned short 
WavFileSink::ByteSwapShort (unsigned short nValue)
{
  return (((nValue>> 8)) | (nValue << 8));
}


void 
WavFileSink::putLinear16Swap(natural c, realvec& slice)
{

  
  natural nChannels = getctrl("natural/nChannels").toNatural();
  natural nSamples = getctrl("natural/inSamples").toNatural();
  
  for (t=0; t < nSamples; t++)
    {
#if defined(__BIG_ENDIAN__)
      sdata_[t*nChannels + c] = ByteSwapShort((short)(slice(0,t) * MAXSHRT));
#else
      sdata_[t*nChannels + c] = (short)(slice(0,t) * MAXSHRT);
#endif

    }

  if (c == nChannels -1) 
    {
      if ((natural)fwrite(sdata_, sizeof(short), nChannels * nSamples, sfp_) != nChannels * nSamples)
	{
	  MRSERR("Problem: could not write window to file " + filename_);
	}
    }
}



void 
WavFileSink::process(realvec& in, realvec& out)
{
  checkFlow(in,out);
  
  // copy input to output 
  for (o=0; o < inObservations_; o++)
    for (t=0; t < inSamples_; t++)
      {
	out(o,t) = in(o,t);
      }

  natural c = getctrl("natural/channel").toNatural();
  natural nChannels = getctrl("natural/nChannels").toNatural();
  setctrl("natural/channel", (c + 1)%nChannels);  
  long fileSize;
  fpos_ = ftell(sfp_);

  // jump to start and write data size
  fseek(sfp_, 40, SEEK_SET);
  written_ += inSamples_;
  fileSize = (written_ * 2 * nChannels);
  fwrite(&fileSize, 4, 1, sfp_);
  fseek(sfp_, fpos_, SEEK_SET);

  putLinear16Swap(c, in);
}








  
  



	

	

	
