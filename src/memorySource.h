

#ifndef MEMORYSOURCE_H
#define MEMORYSOURCE_H

#include "lib/api.h"

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
		MStatus setOutputSourceData(MArrayDataHandle &sourceArrayDH, MArrayDataHandle &sinkArrayDH);

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
	static MObject aSinkConnection;
	static MObject aTime;
	static MObject aResetFrame;
	static MObject aData;

	// time delay to use - this corresponds to length of buffer,
	// so don't make it crazy long
	static MObject aTimeOffset;

	// frequency of update
	static MObject aStepSize;

	// constantly incrementing counter, might be useful for something
	static MObject aIncrement;

	// outer buffer array - each entry will hold array of all values 
	// at corresponding frame
	static MObject aValueBuffer;

	// single value entry, holds all solver data for one frame
	static MObject aInnerData;


	// internal sink objects
	MObject sinkObj;
	// ToDo: refactor to use observing pointers
	std::set<MObject> sinkObjs;
	// check for disconnection
	bool sinkConnected;

	// compare previous value
	float previousTime;
    

};

/*
There is some inelegance in using a bool connection, as one source may be connected
to multiple sinks, which is wrong
for now, only the first connected sink will be considered


*/


#endif
	