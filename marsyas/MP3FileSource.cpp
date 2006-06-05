/*
** Copyright (C) 1998-2006 George Tzanetakis <gtzan@cs.cmu.edu>
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
   \class MP3FileSource
   \brief MP3FileSource read mp3 files using libmad
   \author Stuart Bray

This class reads an mp3 file using the mad mp3 decoder library.  Some
of this code was inspired from Bertrand Petit's madlld example.  The
code to resize the buffers was borrowed from Marsyas AudioSource.
	  

*/



#include "MP3FileSource.h"
using namespace std;


MP3FileSource::MP3FileSource(string name)
{
  
  type_ = "MP3FileSource";
  name_ = name;
  addControls();
  ri_ = preservoirSize_ = 0;
  ptr_ = 0;

  fileSize_ = 0;
  fd = 0;  
  fp = NULL;
  offset = 0;

}


MP3FileSource::~MP3FileSource()
{

#ifdef MAD_MP3  
  madStructFinish();
#endif
  closeFile(); 
}





MP3FileSource::MP3FileSource(const MP3FileSource& a)
{
  type_ = a.type_;
  name_ = a.name_;
  ncontrols_ = a.ncontrols_; 		
  
  inSamples_ = a.inSamples_;
  inObservations_ = a.inObservations_;
  onSamples_ = a.onSamples_;
  onObservations_ = a.onObservations_;
  dbg_ = a.dbg_;
  mute_ = a.mute_;

  ptr_ = NULL;
  fp = NULL;
  


}





MarSystem* 
MP3FileSource::clone() const
{
  return new MP3FileSource(*this);
}


void 
MP3FileSource::addControls()
{
  addDefaultControls();

  // nChannels is one for now
  addctrl("natural/nChannels",1);
  addctrl("natural/bitRate", 160000);
  setctrlState("natural/nChannels", true);
  addctrl("bool/init", false);
  setctrlState("bool/init", true);
  addctrl("bool/notEmpty", true);
  addctrl("natural/loopPos", (natural)0);
  setctrlState("natural/loopPos", true);
  addctrl("natural/pos", (natural)0);
  setctrlState("natural/pos", true);
  addctrl("string/filename", "daufile");
  setctrlState("string/filename", true);
  addctrl("natural/size", (natural)0);
  addctrl("string/filetype", "mp3");
  addctrl("real/repetitions", 1.0);
  setctrlState("real/repetitions", true);
  addctrl("real/duration", -1.0);
  setctrlState("real/duration", true);

  addctrl("bool/advance", false);
  setctrlState("bool/advance", true);

  addctrl("bool/shuffle", false);
  setctrlState("bool/shuffle", true);

  addctrl("natural/cindex", 0);
  setctrlState("natural/cindex", true);


  addctrl("string/allfilenames", ",");
  addctrl("natural/numFiles", 1);
	
  addctrl("string/currentlyPlaying", "daufile");
}



/** 
 * Function: getHeader
 * Description: Opens the MP3 file and collects all the necessary
 * 		information to update the MarSystem. 
 */
void 
MP3FileSource::getHeader(string filename) 
{

#ifdef MAD_MP3  
  // if we have a file open already, close it
  closeFile();
  
  
  fp = fopen(filename.c_str(), "rb");
  fseek(fp, 0L, SEEK_END);
  myStat.st_size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);



  if (myStat.st_size == 0 ) {
    MRSWARN("Error reading file: " + filename);
    setctrl("natural/nChannels", (natural)1);
    setctrl("real/israte", (real)22050.0);
    setctrl("natural/size", (natural)0);
    notEmpty_ = 0;
    setctrl("bool/notEmpty", (MarControlValue)false);	  
    return;
  }
  

  
  

  ptr_ = new unsigned char[myStat.st_size];
  
  int numRead = fread(ptr_, sizeof(unsigned char), myStat.st_size, fp);
  
  if (numRead != myStat.st_size) 
    {
      MRSWARN("Error reading: " + filename + " to memory.");
      setctrl("natural/nChannels", (natural)1);
      setctrl("real/israte", (real)22050.0);
      setctrl("natural/size", (natural)0);
      notEmpty_ = 0;
      setctrl("bool/notEmpty", (MarControlValue)false);	  
      return;
    }
  
  fileSize_ = myStat.st_size;
  
  
  // initialize mad structs and fill the stream
  madStructInitialize();
  fillStream();	
 

  // if there is nothing in the stream...
  notEmpty_ = getctrl("bool/notEmpty").toBool(); 
  if (!notEmpty_) {
    pos_ = 0;
    return;
  }
  
  // decode some frames until we find the samplerate and bitrate
  while ( frame.header.samplerate == 0 && frame.header.bitrate == 0 ) 
    {
      pos_ += bufferSize_;
      currentPos_ = pos_;
      
      if ( mad_frame_decode(&frame, &stream) ) 
	{
	
	  if(MAD_RECOVERABLE(stream.error)) 
	    {
				
	      if(stream.error != MAD_ERROR_LOSTSYNC) {
		MRSWARN("MP3FileSource: recoverable frame level error");
	      }
	      
	      // get some more samples...
	      fillStream();
	      if (!notEmpty_) {
		pos_ = 0;
		return;
	      }
	      
	    } 
	  else if(stream.error==MAD_ERROR_BUFLEN) 
	    {

	      fillStream();
	      if (!notEmpty_) {
		pos_ = 0;
		return;
	      }
	      
	    } 
	  else 
	    {
	      MRSERR("MP3FileSource: unrecoverable frame level error, quitting.");
	      
	    }
	  
	  frameCount_++;
	}
    
  }
	
  frameSamples_ = 32 * MAD_NSBSAMPLES(&frame.header);
  bufferSize_ = frameSamples_; // mad frame size
  natural bitRate = frame.header.bitrate;
  real sampleRate = frame.header.samplerate;

  
  // only works for a constant bitrate, duration is (bits in file / bitrate)
  real duration_ = 2 * (fileSize_ * 8) / bitRate;
  advance_ = getctrl("bool/advance").toBool();
  cindex_ = getctrl("natural/cindex").toNatural();
  
  
  size_ = (natural) ((duration_ * sampleRate) / MAD_NCHANNELS(&frame.header));

  
  csize_ = size_ * MAD_NCHANNELS(&frame.header);
  totalFrames_ = (natural)((sampleRate * duration_) / frameSamples_);
  
  
  // update some controls 
  updctrl("real/duration", duration_);
  updctrl("natural/nChannels", MAD_NCHANNELS(&frame.header)); 
  updctrl("real/israte", sampleRate);
  updctrl("natural/size", size_ / 2);
  updctrl("natural/bitRate", bitRate);

  offset = 0;
  pos_ = samplesOut_ = frameCount_ = 0;
  currentPos_ = 0;
  notEmpty_ = 1;
  
#endif
}	


/**
 * Function: update
 *
 * Description: Performs the usual MarSystem update jobs. Additionally, 
 * 		it can update the position of the index in the file if
 * 		the user seeks forward or backward.  Note that this 
 * 		is currently going to be slow as we have to refill the 
 * 		mad buffer each time somebody seeks in the file. 
 *
 */
void 
MP3FileSource::update()
{
  MRSDIAG("MP3FileSource::update");
  
  setctrl("natural/onSamples", getctrl("natural/inSamples"));
  setctrl("natural/onObservations", getctrl("natural/inObservations"));
  setctrl("real/osrate", getctrl("real/israte"));
 
  israte_ = getctrl("real/israte").toReal();
  inSamples_ = getctrl("natural/inSamples").toNatural();

  pos_ = getctrl("natural/pos").toNatural();
  
  // if the user has seeked somewhere in the file
  if ( currentPos_ != pos_ && pos_ < size_) {
	  
    // compute a new file offset using the frame target
    real ratio = (real)pos_/size_;

#ifdef MAD_MP3     
    madStructInitialize();
#endif 
    
    natural targetOffset = (natural) (fileSize_ * (real)ratio);
  
    // if we are rewinding, we call fillStream with -1
    if (targetOffset==0) {
	    fillStream(-1);
    } else {
	    fillStream(targetOffset);
    }
    currentPos_ = pos_;
  }

  filename_ = getctrl("string/filename").toString();    
  duration_ = getctrl("real/duration").toReal();
  advance_ = getctrl("bool/advance").toBool();
  //rewindpos_ = pos_;
  
  repetitions_ = getctrl("real/repetitions").toReal();
  
  if (duration_ != -1.0)
    {
      csize_ = (natural)(duration_ * israte_);
    }
	
  defaultUpdate();
  	
  if (inSamples_ < bufferSize_/2) {
    reservoirSize_ = 2 * bufferSize_;
  } else { 
    reservoirSize_ = 2 * inSamples_;
  }
  if (reservoirSize_ > preservoirSize_) {
    reservoir_.stretch(reservoirSize_);
  }
  preservoirSize_ = reservoirSize_;

}



/**
 * Function: getLinear16
 * 
 * Description: This function does all the work. It is called by process,
 * 		and its job is to fill an output vector of the correct
 * 		size to push through the system.  The MAD decoder can
 * 		only retrieve buffer sizes of 1152, so therefore we have
 * 		to balance and maintain a reservoir.  Thus, we only get
 * 		more samples from MAD when the reservoir is below a 
 * 		threshold.  (As in AudioSource, etc). 
 *
 */
natural 
MP3FileSource::getLinear16(realvec& slice) {

		

#ifdef MAD_MP3  
  register double peak = 1.0/32767; // normalize 24-bit sample
  register mad_fixed_t left_ch, right_ch;
  register real sample;
	 
  // decode a frame if necessary 
  while (ri_ < inSamples_) {
    
    fillStream();
    
    if (!notEmpty_) {
      pos_ = 0;
      return pos_;
    }
    
    
    if (mad_frame_decode(&frame, &stream )) 
      {
	if(MAD_RECOVERABLE(stream.error)) 
	  {
	    
	    if(stream.error != MAD_ERROR_LOSTSYNC) {
	      MRSWARN("MP3FileSource: recoverable frame level error");
	    }
	    
	    fillStream();
	    if (!notEmpty_) {
	      pos_ = 0;
	      return pos_;
	    }
	    
	  } 
	else 
	  if(stream.error==MAD_ERROR_BUFLEN) 
	    {
	      
	      fillStream(); 
	      if (!notEmpty_) {
		pos_ = 0;
		return pos_;
	      }
	      
	    } 
	
	  else 
	    {
	      MRSERR("MP3FileSource: unrecoverable frame level error, quitting.");
	    }
	
	frameCount_++;
      }
    

    mad_synth_frame(&synth, &frame);  
			
    // fill the reservoir...
    for (t=0; t < bufferSize_; t++) {
			
	left_ch = synth.pcm.samples[0][t];
	
	
	sample = (real) scale(left_ch);	
	
	// for 2 channel audio we can add the channels 
	// and divide by two
	if(MAD_NCHANNELS(&frame.header)==2) {
		right_ch = synth.pcm.samples[1][t];
		sample += (real) scale(right_ch);
		sample /= 2; 
	}
	
	sample *= peak;

	reservoir_(ri_) = sample;
	ri_++;
    }
    
  } // reservoir fill

  
  // spit out the first inSamples_ in our reservoir 
  for (o=0; o < inObservations_; o++) {
    for (t=0; t < inSamples_; t++) {
      slice(0,t) = reservoir_(t);
    }
  }
	
  // keep track of where we are
  pos_ += inSamples_; // (inSamples_ * getctrl("natural/nChannels").toNatural());
  currentPos_ = pos_;	
	
	
  // move the data we ticked to the front of the reservoir
  for (t=inSamples_; t < ri_; t++) {
    reservoir_(t-inSamples_) = reservoir_(t);
  }
  
  // update our reservroi index
  ri_ = ri_ - inSamples_;	

  return pos_;
#else
  return 0;
  
#endif 

}


/**
 * Function: process
 * Description: Fills an output vector with samples.  In this case,
 * 		getLinear16 does all the work.
 */
void MP3FileSource::process(realvec& in, realvec& out)
{
  checkFlow(in,out);

  if (notEmpty_) 
    getLinear16(out);
  else
    out.setval(0.0);

  samplesOut_ += onSamples_;
  
  if (notEmpty_) {
  	notEmpty_ = (samplesOut_ < repetitions_ * csize_);
  } else{
	  // if notEmpty_ was false already it got set in fillStream
    // MRSWARN("MP3FileSource: track ended.");
  }
  
}


/*
 * Initialize mad structs
 */
#ifdef MAD_MP3  
void MP3FileSource::madStructInitialize() {

	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);
}
#endif

	

/*
 * Release mad structs
 */
#ifdef MAD_MP3  
void MP3FileSource::madStructFinish() {

	mad_stream_finish(&stream);
	mad_frame_finish(&frame);
	mad_synth_finish(&synth);
}
#endif 




/**
 * Function: fillstream()
 *
 * Description: Fill the mad stream with a chunk of audio. This function was
 * 		inspired from IzSounds maddecoder implementation. 
 *
 */
void 
MP3FileSource::fillStream( natural target ) 
{

  // fill the input buffer
#ifdef MAD_MP3  
  if (stream.buffer == NULL || stream.error == MAD_ERROR_BUFLEN) 
    {
    
      register natural remaining = 0;
      register natural chunk = INPUT_BUFFER_SIZE;
      
      // when called with the default parameter, carry on decoding...	  
      if ( stream.next_frame != NULL ) {
	offset = stream.next_frame - ptr_;  
	remaining = fileSize_ - offset;
      } else if ( target != 0 ) {	
	// we have seeked somewhere in the file...
	offset = target;
	remaining = fileSize_ - offset;
      } else if ( target == -1 ) {
	// we rewound the track...
	offset = 0;
	remaining = fileSize_;
      }
      
      // there may not be enough to fill the buffer
      if ( remaining < INPUT_BUFFER_SIZE ) {
	chunk = remaining + MAD_BUFFER_GUARD;
      }
      
      // if we have hit the end...
      if ( offset >= fileSize_ ) {
	notEmpty_ = false;
	// MRSWARN("MP3FileSource: cannot seek to offset");
      } else {
	// fill the mad buffer
	mad_stream_buffer(&stream, ptr_ + offset, chunk);
	stream.error = MAD_ERROR_NONE;
      }
    }
  
#endif
}



/**
 * Function: closeFile()
 *
 * Description: Close the file if its open, release memory, and 
 * 		release mad structs.
 *
 */
void MP3FileSource::closeFile()
{
	
  // close the file and release mad structs
  if (fp == NULL) 
    return;

  fclose(fp);
  fd = 0;

  delete [] ptr_;


#ifdef MAD_MP3  
  madStructFinish();
#endif 
}




/**
 *
 * Function: scale 
 *
 * This function (and documentation) was taken directly from minimad.c.
 * 
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 * 
 */

#ifdef MAD_MP3  
inline signed int MP3FileSource::scale(mad_fixed_t sample)
{
  // round 
  sample += (1L << (MAD_F_FRACBITS - 16));

  // clip
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  // quantize
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}
#endif 
