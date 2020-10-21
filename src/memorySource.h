

#ifndef MEMORYSOURCE_H
#define MEMORYSOURCE_H

#include "lib/api.h"
#include "lib/containers.h"
#include "memorySink.h"

class MemorySource : public MPxNode {
    public:
        MemorySource();
        virtual ~MemorySource();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

        static void* creator();
        static MStatus initialize();
		MStatus getConnectedSink(MObject &nodeObj);
		MStatus connectionMade(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);
		MStatus connectionBroken(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);
		void setSinkObj(MObject &obj);
		void clearSinkObj(MObject &obj);
		MStatus getSinkData(MObject &sinkObj, MDataHandle &sinkDH);
		MemorySink * getSinkInstance(MObject &sinkObj, MStatus &s);
		MStatus setOutputSourceData(MArrayDataHandle &sourceArrayDH, MArrayDataHandle &sinkArrayDH);

		virtual void postConstructor();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
	static MObject aSinkConnection;
	static MObject aTime;
	static MObject aResetFrame;
	static MObject aData;
	static MObject aPrevTime;

	// time delay to use - this corresponds to length of buffer,
	// so don't make it crazy long
	static MObject aTimeOffset;

	// frequency of update
	static MObject aStepSize;
	// number of frames to cover
	static MObject aNFrames;

	// constantly incrementing counter, might be useful for something
	static MObject aIncrement;

	// outer buffer array
	static MObject aFrameBuffer;
	// single frame entry
	static MObject aValueBuffer;

	// single value entry, holds all solver data for one frame
	static MObject aInnerData;

	// initial data for solver behaviour
	static MObject aInitData;

	// array of float time deltas, might be useful for live time stuff
	static MObject aDeltas;


	// internal sink objects
	MObject sinkObj;

	// pointer to sink instance
	MemorySink * sinkPtr;

	// ToDo: refactor to use observing pointers
	std::set<MObject> sinkObjs;
	// check for disconnection
	bool sinkConnected;

	// compare previous value
	float previousTime;
    
	// data arrays
	std::vector<std::vector<MObject>> dataArrays;

};

/*
There is some inelegance in using a bool connection, as one source may be connected
to multiple sinks, which is wrong
for now, only the first connected sink will be considered


*/


#endif
	