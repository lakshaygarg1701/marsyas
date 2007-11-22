#ifndef MEAWS_BACKEND_H
#define MEAWS_BACKEND_H

#include "defs.h"
#include <QObject>
#include "MarSystemManager.h"
#include "MarSystemQtWrapper.h"
#include "TranscriberExtract.h"
using namespace Marsyas;
using namespace MarsyasQt;
using namespace std;

class MarBackend: public QObject {
	Q_OBJECT

public:
	MarBackend();
	~MarBackend();

	// communication
	void setBackend(mrs_natural method, mrs_bool hasAudio,
		std::string filename);


	bool analyze();
	realvec getPitches();
	realvec getMidiPitches();
	realvec getAmplitudes();

	mrs_real getRate();
	void setup();

public slots:
	// communication
	void start();
	void stop();

signals:
	void setAttempt(bool running);
	void gotAudio();

private slots:
	void ctrlChanged(MarControlPtr changed);

private:
	MarSystemManager mng;
	MarSystem *sourceNet;
	MarSystem *allNet;
	MarSystem *pitchSink;
	MarSystem *ampSink;
	MarSystemQtWrapper *mrsWrapper;
	MarControlPtr isEmptyPtr;
	bool isEmptyState;

// constructor, destructor
	mrs_natural method_;
	MarSystem* makeSourceNet(bool fromFile);
	MarSystem* makePitchNet(mrs_real source_osrate);
	MarSystem* makeAmplitudeNet(mrs_real source_osrate);
	void setupAllNet();
	void delNet();

// for playback
	bool hasAudio_;
	bool setupChanged_;

// depending on usage, could be input OR output!
	string filename_;
// 	MarControlPtr filenamePtr;


	realvec pitchList;
	realvec ampList;
// pitch stuff
	realvec pitchOld;
	realvec pitchMidiList;
    realvec amplitudeOld;
};
#endif

