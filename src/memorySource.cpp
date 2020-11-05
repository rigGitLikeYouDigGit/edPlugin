

/*

	memory cell receiving result of previous evaluation

	source pulls from sink ahead of it, before sink is evaluated,
	instead of sink updating source -
	this seems less likely to cause dirty problems

	UPDATE : this kind of works, but the test case of a simple 3 body simulation
	threw up some problems
	currently no way to only specify start state - I'm having to do janky choice node things
	to get it to work

	let the memory source data be writable too - if 


	
*/

#include "memorySource.h"
#include "memorySink.h"

using namespace ed;
using namespace std;

MTypeId MemorySource::kNODE_ID(0x00122C1B);
MString MemorySource::kNODE_NAME( "memorySource" );

MObject MemorySource::aTime;
MObject MemorySource::aResetFrame;
MObject MemorySource::aTimeOffset;
MObject MemorySource::aStepSize;
MObject MemorySource::aNFrames;
MObject MemorySource::aData;
MObject MemorySource::aValueBuffer;
MObject MemorySource::aFrameBuffer;
MObject MemorySource::aInnerData;
MObject MemorySource::aInitData;
MObject MemorySource::aSinkConnection;
MObject MemorySource::aIncrement;
MObject MemorySource::aDeltas;
MObject MemorySource::aPrevTime;

#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\

void MemorySource::postConstructor() {
	this->setExistWithoutInConnections(true);
	this->setExistWithoutOutConnections(true);

	MStatus stat;
	// connect time1 by default
	MSelectionList sel;
	sel.add("time1");
	MFnDependencyNode thisFn(thisMObject());
	MObject timeObj;
	sel.getDependNode(0, timeObj);
	MFnDependencyNode timeFn(timeObj);
	
	MPlug outTimePlug = timeFn.findPlug("outTime");
	MPlug inTimePlug = thisFn.findPlug("time");
	MDGModifier dgMod;
	dgMod.connect(outTimePlug, inTimePlug);
	dgMod.doIt();
}

MStatus MemorySource::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;
	MFnCompoundAttribute cFn;
	MFnAttribute aFn;


	// attribute used to update cell - time is most convenient
	aTime = uFn.create("time", "time", MFnUnitAttribute::kTime, 0.0);
	tFn.setReadable(true);
	tFn.setWritable(true);

	// attribute used to update cell - time is most convenient
	aPrevTime = uFn.create("prevTime", "prevTime", MFnUnitAttribute::kTime, 0.0);
	tFn.setReadable(true);

	// attribute used to update cell - time is most convenient
	aResetFrame = uFn.create("resetFrame", "resetFrame", MFnUnitAttribute::kTime, 1.0);

	// number of frames to track
	aNFrames = nFn.create("nFrames", "nFrames", MFnNumericData::kInt, 3);
	nFn.setReadable(true);
	nFn.setWritable(true);

	// untyped attribute can be used to pass on whatever you want
	aData = tFn.create("data", "data", MFnData::kAny);
	tFn.setReadable(true);
	tFn.setWritable(false);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);
	addAttribute(aData);

	// outer array of frame data
	aFrameBuffer = cFn.create("frame", "frame");
	cFn.setArray(true);
	cFn.setUsesArrayDataBuilder(true);
	cFn.setWritable(false);

	cFn.addChild(aData);

	// internal cache array mirroring main interface
	aInnerData = tFn.create("innerData", "innerData", MFnData::kAny);
	tFn.setReadable(true);
	tFn.setWritable(true);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);

	aInitData = tFn.create("initData", "initData", MFnData::kAny);
	tFn.setReadable(false);
	tFn.setWritable(true);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);

	

	// array of time deltas
	aDeltas = nFn.create("delta", "delta", MFnNumericData::kFloat);
	nFn.setWritable(false);
	nFn.setReadable(true);
	nFn.setArray(true);
	nFn.setUsesArrayDataBuilder(true);

	// boolean connection to sink
	aSinkConnection = nFn.create("sink", "sink", MFnNumericData::kBoolean, 0.0);
	nFn.setReadable(true);
	nFn.setWritable(false);

	// can be handy for some uses to have an exclusively positive counter
	aIncrement = nFn.create("increment", "increment", MFnNumericData::kInt, 0.0);
	nFn.setReadable(true);
	nFn.setWritable(false);

	vector<MObject> drivers = { aInitData, aTime, aResetFrame, aTimeOffset, aNFrames };
	vector<MObject> driven = { aFrameBuffer, aInnerData, aSinkConnection, aIncrement,
		aDeltas, aPrevTime,
		aData,
	};
	
	addAttributes<MemorySource>(drivers);
	addAttributes<MemorySource>(driven);

	setAttributesAffect<MemorySource>(drivers, driven);

    return MStatus::kSuccess;
}


MStatus MemorySource::compute(
				const MPlug& plug, MDataBlock& data) {
	MStatus s;
	DEBUGS("memorySource compute");
	//DEBUGS("memorySource sinkConnected " << sinkConnected);

	if (!sinkConnected || sinkObj == MObject::kNullObj) {
		// do literally nothing
		data.setClean(plug);
		return MS::kSuccess;
	}

	// get source data handles
	MArrayDataHandle sourceFramesDH = data.outputArrayValue(aFrameBuffer);
	MArrayDataHandle sourceArrayDH = data.outputArrayValue(aData);
	MArrayDataHandle sourceInnerArrayDH = data.outputArrayValue(aInnerData);
	MArrayDataHandle sourceDeltasDH = data.outputArrayValue(aDeltas);


	// retrieve vector from sink
	std::vector<MObject> sinkData = sinkPtr->dataObjs;
	unsigned int nDataElements = static_cast<int>(sinkData.size());

	DEBUGS("nDataElements " << nDataElements);

	// extract sink data
	MDataHandle sinkDH;
	s = getSinkData(sinkObj, sinkDH);
	MArrayDataHandle sinkArrayDH = MArrayDataHandle(sinkDH);

	//data.setClean(plug);
	//return MS::kSuccess;

	int nFrames = data.inputValue(aNFrames).asInt();
	MTime newMTime = data.inputValue(aTime).asTime();
	MTime prevMTime = data.outputValue(aPrevTime).asTime();
	float newTime = newMTime.as(MTime::k24FPS);
	float prevTime = prevMTime.as(MTime::k24FPS);


	//data.setClean(plug);
	//return MS::kSuccess;
		
	// check if time is reset frame - reset to current values if so
	if EQ(newTime, data.inputValue(aResetFrame).asTime().as(MTime::k24FPS)) {
		DEBUGS("memorySource time reset");
		/*
		data.setClean(aFrameBuffer);
		data.setClean(aData);
		data.setClean(aDeltas);
		data.setClean(aIncrement);
		data.setClean(plug);
		*/

		// reset vectors
		dataArrays.clear();

		for (unsigned int i = 0; i < nFrames; i++) {
			jumpToElement(sourceDeltasDH, i);
			jumpToElement(sourceFramesDH, i);

			dataArrays.push_back(std::vector<MObject>(sinkData));
		}

		// look up data through plug since it was crashing otherwise
		MPlug aDataPlug(thisMObject(), aData);


		// reset all frame data
		// for (int i = 0; i < sourceFramesDH.elementCount(); i++) {
		for (unsigned int i = 0; i < nFrames; i++) {
			s = aDataPlug.selectAncestorLogicalIndex(i, aFrameBuffer);
			MDataHandle aDataDH = aDataPlug.constructHandle(data);
			MArrayDataHandle aDataArrayDH(aDataDH, &s);
			CHECK_MSTATUS_AND_RETURN_IT(s, "reset data arrayHandle construction failed");
			MArrayDataBuilder aDataArrayBuilder = aDataArrayDH.builder(&s);
			CHECK_MSTATUS_AND_RETURN_IT(s, "reset data arrayHandle builder extraction failed");

			for (unsigned int n = 0; n < nDataElements; n++) {
				MDataHandle aResetDataDH = aDataArrayBuilder.addElement(n, &s);
				CHECK_MSTATUS_AND_RETURN_IT(s, "reset data add data entry failed");
				s = aResetDataDH.setMObject( MObject(dataArrays[i][n]));
				CHECK_MSTATUS_AND_RETURN_IT(s, "reset data inner data entry " << n << "copy failed");
			}
			s = aDataArrayDH.set(aDataArrayBuilder);
			CHECK_MSTATUS_AND_RETURN_IT(s, "reset data set array builder failed");
			aDataPlug.setValue(aDataDH);
			aDataPlug.destructHandle(aDataDH);

			// reset deltas
			jumpToElement(sourceDeltasDH, i);
			MDataHandle deltaDH = sourceDeltasDH.outputValue();
			deltaDH.setFloat(0.0);
		}
		
		// reset counter
		data.outputValue(aIncrement).setInt(0);

		// set plugs clean and return
		vector<MObject> driven = { aFrameBuffer, aInnerData, aSinkConnection, aIncrement,
	aDeltas, aPrevTime,
	aData,
		};
		data.setClean(aFrameBuffer);
		data.setClean(aData);
		data.setClean(aDeltas);
		data.setClean(aIncrement);
		data.setClean(plug);
		return MS::kSuccess;
	}

	


	// for some reason input time plug was never reading as dirty
	DEBUGS("prevTime " << prevTime << " newTime " << newTime);
	//if(!EQ( prevTime, newTime)){
	if(!EQ( prevTime, newTime)){
		DEBUGS("memorySource time dirty");

		float delta = newTime - prevTime;
		float deltaA = delta, deltaB = delta;
		float deltas[2] = { delta, delta };

		// update delta array
		for (unsigned int i = 0; i < nFrames; i++) {
			jumpToElement(sourceDeltasDH, i);
			deltas[i % 2] = sourceDeltasDH.outputValue().asFloat();
			sourceDeltasDH.outputValue().setFloat(deltas[!(i % 2)]);
		}

		// update data arrays
		dataArrays.insert(dataArrays.begin(), sinkData);
		dataArrays.pop_back();


		for (unsigned int i = 0; i < nFrames; i++) {
			jumpToElement(sourceFramesDH, i);
			MDataHandle frameDH = sourceFramesDH.outputValue();
			MArrayDataHandle frameArrayDH(frameDH);

			
			MPlug targetPlug(thisMObject(), aData);
			s = targetPlug.selectAncestorLogicalIndex(i, aFrameBuffer);
			MCHECK(s, "failed to set parent index for data plug")


			for (unsigned int n = 0; n < nDataElements; n++) {
				MPlug targetLeafPlug = targetPlug.elementByPhysicalIndex(n);

				DEBUGS("targetPlug " << targetLeafPlug.info());

				if (targetLeafPlug.isNull()) {
					DEBUGS("targetLeafPlug is not valid, continuing");
					continue;
				}
				if (dataArrays[i][n].isNull()) {
					DEBUGS("Mobject is null, continuing");
					continue;
				}

				targetLeafPlug.setMObject(dataArrays[i][n]);
			}
		}

		data.setClean(aFrameBuffer);
		data.setClean(aData);

		// add to increment
		data.outputValue(aIncrement).setInt(data.outputValue(aIncrement).asInt() + 1);

		previousTime = newTime;
		data.outputValue(aPrevTime).setMTime(newMTime);
		data.setClean(aTime);
		data.setClean(aPrevTime);

		// dirty sink plug
		bool switchStatus = !data.outputValue(aSinkConnection).asBool();
		data.outputValue(aSinkConnection).setBool(switchStatus);

	}
	

	

	data.setClean(plug);

    return MS::kSuccess;
}

MStatus MemorySource::getSinkData(MObject &sinkObj, MDataHandle &sinkDH) {
	//DEBUGS("memorySource getSinkData")
	MStatus s;
	MPlug sinkPlug = MPlug(sinkObj, MemorySink::aData);

	sinkDH = MDataHandle(sinkPlug.asMDataHandle());

	return MStatus::kSuccess;
}

MemorySink * MemorySource::getSinkInstance(MObject &sinkObj, MStatus &s) {
	// retrieve and cast full user-defined node for given MObject
	// thanks Matt
	MFnDependencyNode nodeFn(sinkObj);
	
	// retrieve MPxNode pointer
	MPxNode *mpxPtr = nodeFn.userNode(&s);
	//MCHECK(s, "failed to extract mpxNode pointer, object is invalid");

	// black science
	MemorySink * sinkPtr = dynamic_cast<MemorySink *>(mpxPtr);
	if (sinkPtr == NULL) {
		cerr << "failed dynamic cast to sink instance " << endl;
		s = MS::kFailure;
	}
	s = MS::kSuccess;
	return sinkPtr;

}

MStatus MemorySource::setOutputSourceData(MArrayDataHandle &sourceArrayDH, MArrayDataHandle &sinkArrayDH) {
	DEBUGS("memorySource setOutputData")
	MStatus s;

	mirrorArrayDataHandle(sinkArrayDH, sourceArrayDH);
	return MStatus::kSuccess;
}


MStatus MemorySource::connectionMade(
	const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// check if sink has been connected 
	//DEBUGS("memorySource connectionMade");

	//DEBUGS("plug " << plug.name() << " otherPlug " << otherPlug.name());

	// we only care about sink plug
	if (plug.attribute() != aSinkConnection) {
		return MPxNode::connectionMade(plug, otherPlug, asSrc);
	}

	if (otherPlug.attribute() == MemorySink::aSourceConnection) {
		if (sinkConnected) {
			// cannot connect two sinks to one source
			DEBUGS("memorySource already has sink connected");
			return MStatus::kFailure;
		}
		setSinkObj(otherPlug.node());
	}

	return MPxNode::connectionMade(plug, otherPlug, asSrc);
}

MStatus MemorySource::connectionBroken(
	const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// clear connected sink node
	DEBUGS("memorySource connectionBroken");

	if (plug.attribute() != aSinkConnection) {
		return MPxNode::connectionBroken(plug, otherPlug, asSrc);
	}
	clearSinkObj(MObject::kNullObj);

	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
}

void MemorySource::setSinkObj(MObject &obj) {
	// sets internal sink reference
	sinkObj = obj;
	MStatus s;
	sinkPtr = getSinkInstance(obj, s);
	sinkConnected = true;
}

void MemorySource::clearSinkObj(MObject &obj) {
	// is this enough to clear reference to connected node?
	sinkObj = MObject::kNullObj;
	sinkPtr = NULL;
	sinkConnected = false;
}

void* MemorySource::creator(){
	// sink is not connected on creation
	MemorySource *newObj = new MemorySource;
	newObj->sinkConnected = false;
	newObj->sinkObj = MObject::kNullObj;
	newObj->sinkPtr = NULL;
	newObj->previousTime = 0.0;
    return newObj;

}

MemorySource::MemorySource() {};
MemorySource::~MemorySource() {};

