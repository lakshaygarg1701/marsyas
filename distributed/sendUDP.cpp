
#include <stdio.h>
#include <iostream>
#include <string>
#include "Collection.h"
#include "AuFileSource.h"  
#include "SocketException.h"
#include "NetworkUDPSink.h"
#include "AudioSink.h"
#include "MrsDoctor.h"
#include "Gain.h" 
#include "Spectrum.h"
#include "InvSpectrum.h"
#include "fft.h" 
#include "AutoCorrelation.h" 
#include "Peaker.h" 
#include "SineSource.h" 
#include "MarSystemManager.h"
#include "HalfWaveRectifier.h"
#include "CommandLineOptions.h"

CommandLineOptions cmd_options;


#define EMPTYSTRING "MARSYAS_EMPTY"
string host = EMPTYSTRING;


int helpopt;
int usageopt;
long offset = 0;
long port = 0;
long duration = 1000 * 44100;
float start = 0.0f;
float length = 1000.0f;
float gain = 1.0f;
float repetitions = 1;



void 
printUsage(string progName)
{
  MRSDIAG("sendTCP.cpp - printUsage");
  cerr << "Usage : " << progName << " [-g gain] [-o offset(samples)] [-d duration(samples)] [-s start(seconds)] [-l length(seconds)] [-f outputfile] [-h host] [-p portNumber] [-r repetitions] file1 file2 file3" << endl;
  cerr << endl;
  cerr << "where file1, ..., fileN are sound files in a MARSYAS supported format or collections " << endl;
  exit(1);
}

void 
printHelp(string progName)
{
  MRSDIAG("sendUDP.cpp - printHelp");
  cerr << "sendUDP, MARSYAS, Copyright George Tzanetakis " << endl;
  cerr << "--------------------------------------------" << endl;
  cerr << "Prints information about the sound files provided as arguments " << endl;
  cerr << endl;
  cerr << "Usage : " << progName << "file1 file2 file3" << endl;
  cerr << endl;
  cerr << "where file1, ..., fileN are sound files in a Marsyas supported format" << endl;
  cerr << "Help Options:" << endl;
  cerr << "-u --usage      : display short usage info" << endl;
  cerr << "-h --help       : display this information " << endl;
  cerr << "-f --file       : output to file " << endl;
  cerr << "-g --gain       : linear volume gain " << endl;
  cerr << "-o --offset     : playback start offset in samples " << endl;
  cerr << "-d --duration   : playback duration in samples     " << endl;
  cerr << "-s --start      : playback start offest in seconds " << endl;
  cerr << "-l --length     : playback length in seconds " << endl;
  cerr << "-p --port       : host port number " << endl;
  cerr << "-t --host	   : host IP address " << endl;
  cerr << "-r --repetitions: number of repetitions " << endl;
  
  exit(1);
}



// Play soundfile given by sfName, msys contains the playback 
// network of MarSystem objects 
void sfplayFile(MarSystem& msys, natural offset, natural duration, 
real start, real length, real gain, real repetitions, string sfName )
{
  msys.updctrl("SoundFileSource/src/string/filename", sfName);
  natural nChannels = msys.getctrl("SoundFileSource/src/natural/nChannels").toNatural();
  real srate = msys.getctrl("SoundFileSource/src/real/israte").toReal();
  
  // playback offset & duration
  offset = (natural) (start * srate * nChannels);
  duration = (natural) (length * srate * nChannels);
  
  // udpate controls
  msys.updctrl("natural/inSamples", MRS_DEFAULT_SLICE_NSAMPLES);
  msys.updctrl("Gain/gt/real/gain", gain);
  msys.updctrl("SoundFileSource/src/natural/pos", offset);      
  
  natural wc=0;
  natural samplesPlayed = 0;
  natural onSamples = msys.getctrl("natural/onSamples").toNatural();
  // natural repeatId = 1;
  
  while (msys.getctrl("SoundFileSource/src/bool/notEmpty").toBool())
    {
      msys.tick();
      wc ++;
      samplesPlayed += onSamples;	
    }
    cerr << "Played " << wc << " slices of " << onSamples << " samples" << endl;
}


// Play a collection l of soundfiles
void sfplay(Collection l, natural offset, natural duration, real start, real length, real gain, real repetitions)
{
  MRSDIAG("sendUDP.cpp - sendUDP");
  int i;

  // Load first soundfile in collection 
  string sfName = l.entry(0);
  SoundFileSource* src = new SoundFileSource("src");
  src->updctrl("string/filename", sfName);

  // create the network sink
  NetworkUDPSink* netSink = new NetworkUDPSink("netSink");
  
  // update controls if they are passed on cmd line...
  if ( host != EMPTYSTRING && port != 0 ) {
  	netSink->updctrl("string/host", host);
  	netSink->updctrl("natural/port", port);
  }
  if ( port != 0 && host == EMPTYSTRING ) {
  	cerr << "Please specify a hostname." << endl;
  	exit(1);
  } else if ( port == 0 && host != EMPTYSTRING ) {
  	cerr << "Please specify a port." << endl;
  	exit(1);
  }
  
  AudioSink* auSink = new AudioSink("auSink");
  
  // create playback network 
  MarSystemManager mn;
  Series playbacknet("playbacknet");
  playbacknet.addMarSystem(src);
  playbacknet.addMarSystem(mn.create("Gain", "gt"));
  playbacknet.addMarSystem(auSink);
  playbacknet.addMarSystem(netSink);
  
  playbacknet.updctrl("AudioSink/auSink/natural/nChannels", 1);
  
  // output the MarSystem
  cout << playbacknet << endl;      
 
  // refresh the connection...

  netSink->refresh();
  cout << "Connecting to host: " << netSink->getctrl("string/host");
  cout << " on port: " << netSink->getctrl("natural/port") << endl;
 
 
  // For each file in collection playback the sound
  for (i=0; i < l.size(); i++)
    {
      sfName = l.entry(i);
      sfplayFile(playbacknet, offset, duration, 
		 start, length, gain, repetitions, sfName );
    }
}


void
readCollection(Collection& l, string name)
{
  MRSDIAG("sendUDP.cpp - readCollection");
  string collectionstr = MARSYAS_MFDIR;
  collectionstr += "/collections/";
  collectionstr += name;
  ifstream from(collectionstr.c_str());
  ifstream from1(name.c_str());
  natural attempts  =0;
  
  
  MRSDIAG("Trying default MARSYAS mf directory: " + collectionstr);
  if (from.good() == false)
    {
      attempts++;
    }
  else
    from >> l;
  
  MRSDIAG("Trying current working directory: " + name);
  if (from1.good() == false)
    {
      attempts++;
    }
  else
    from1 >> l;
  
  if (attempts == 2) 
    {
      MRSWARN("Can not read collection from default mf directory or current working directory");
    }
  
}

void 
initOptions()
{
  cmd_options.addBoolOption("help", "h", false);
  cmd_options.addBoolOption("usage", "u", false);
  cmd_options.addBoolOption("verbose", "v", false);
  cmd_options.addRealOption("start", "s", 0.0f);
  cmd_options.addRealOption("length", "l", 1000.0f);
  cmd_options.addRealOption("gain", "g", 1.0);
  cmd_options.addNaturalOption("port", "p", 0);
  cmd_options.addStringOption("host", "h", EMPTYSTRING);
  cmd_options.addRealOption("repetitions", "r", 1.0);
}


void 
loadOptions()
{
  helpopt = cmd_options.getBoolOption("help");
  usageopt = cmd_options.getBoolOption("usage");
  start = cmd_options.getRealOption("start");
  length = cmd_options.getRealOption("length");
  repetitions = cmd_options.getRealOption("repetitions");
  gain = cmd_options.getRealOption("gain");
  port = cmd_options.getNaturalOption("port");
  host = cmd_options.getStringOption("host");
}



int
main(int argc, const char **argv)
{
  MRSDIAG("sendUDP.cpp - main");

  string progName = argv[0];  

  
  if (argc == 1)
    printUsage(progName);

  initOptions();
  cmd_options.readOptions(argc, argv);
  loadOptions();

  vector<string> soundfiles = cmd_options.getRemaining();
  vector<string>::iterator sfi;



  if (helpopt) 
    printHelp(progName);
  
  if (usageopt)
    printUsage(progName);

  int i;
  

  i = 0;
  Collection l;  
  
  for (sfi = soundfiles.begin(); sfi != soundfiles.end(); ++sfi) 
    {  
	string fname = *sfi;
	string ext = fname.substr(fname.rfind(".", fname.length()), fname.length());
	if (ext == ".mf") 
	  readCollection(l,fname);
	else 
	  l.add(fname);
	i++;
      }
  
  // check for hostname and port...
  sfplay(l, offset, duration, start, length, gain, repetitions );
  exit(1);
}






