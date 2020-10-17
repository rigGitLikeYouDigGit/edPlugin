

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

	// outer array of frame data
	aFrameBuffer = cFn.create("frame", "frame");
	cFn.setArray(true);
	cFn.setUsesArrayDataBuilder(true);

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
	aDeltas = nFn.create("deltas", "deltas", MFnNumericData::kFloat);
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
	vector<MObject> driven = { aFrameBuffer, aData, aInnerData, aValueBuffer, aSinkConnection, aIncrement,
		aDeltas};
	
	addAttributes<MemorySource>(drivers);
	addAttributes<MemorySource>(driven);

	setAttributesAffect<MemorySource>(drivers, driven);

	attributeAffects(aInnerData, aData);

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
/*
	data.setClean(plug);
	return MS::kSuccess;*/

	// get source data handles
	MArrayDataHandle sourceFramesDH = data.outputArrayValue(aFrameBuffer);
	MArrayDataHandle sourceArrayDH = data.outputArrayValue(aData);
	MArrayDataHandle sourceInnerArrayDH = data.outputArrayValue(aInnerData);
	MArrayDataHandle sourceDeltasDH = data.outputArrayValue(aDeltas);


	// extract sink data
	MDataHandle sinkDH;
	s = getSinkData(sinkObj, sinkDH);
	MArrayDataHandle sinkArrayDH = MArrayDataHandle(sinkDH);

	//data.setClean(plug);
	//return MS::kSuccess;

	int nFrames = data.inputValue(aNFrames).asInt();
	//int nFrames = data.outputValue(aNFrames).asInt();
	float newTime = data.inputValue(aTime).asFloat();

	data.setClean(plug);
	return MS::kSuccess;
		
	// check if time is reset frame - reset to current values if so
	if (newTime == data.inputValue(aResetFrame).asFloat()) {
		DEBUGS("memorySource time reset");

		// set up correct number of frames
		int currentFrames = sourceFramesDH.builder().elementCount();
		// removing element indices wiggles my jimmies, only add
		/*for (int i = 0; i < min(nFrames - currentFrames + 1, 0); i++) {
			sourceFramesDH.builder().addLast();
			sourceDeltasDH.builder().addLast();
		}*/
		for (int i = 0; i < nFrames; i++) {
			jumpToElement(sourceDeltasDH, i);
			jumpToElement(sourceFramesDH, i);
		}

		// reset all frame data
		//mirrorArrayDataHandle(sinkArrayDH, sourceInnerArrayDH);
		for (int i = 0; i < sourceFramesDH.elementCount(); i++) {
		//	//sourceArrayDH.jumpToElement(i);
		//	jumpToElement(sourceArrayDH, i);
		//	MArrayDataHandle nFrameDH = sourceArrayDH.inputArrayValue();
		//	mirrorArrayDataHandle(sinkArrayDH, nFrameDH);

			// reset deltas
			jumpToElement(sourceDeltasDH, i);
			MDataHandle deltaDH = sourceDeltasDH.outputValue();
			deltaDH.setFloat(1.0);
		}
		

		// reset counter
		data.outputValue(aIncrement).setInt(0);

		data.setClean(plug);
		return MS::kSuccess;
	}

	data.setClean(plug);
	return MS::kSuccess;


	// only if time has changed, update current values from cached
	//MPlug timePlug = MPlug(thisMObject(), MemorySource::aTime);
	//if (!data.isClean(aTime)) {
	//if (!data.isClean(timePlug)) {

	// for some reason input time plug was never reading as dirty
	if( previousTime != newTime){
		DEBUGS("memorySource time dirty");

		float delta = newTime - previousTime;
		float deltaA = delta, deltaB = delta;
		float deltas[2] = { delta, delta };

		// update delta array
		for (int i = 0; i < nFrames; i++) {
			jumpToElement(sourceDeltasDH, i);
			deltas[i % 2] = sourceDeltasDH.outputValue().asFloat();
			sourceDeltasDH.outputValue().setFloat(deltas[i % 2 + 1]);
		}

		// update frame array
		// MArrayDataHandle frameHandles[2] = { MArrayDataHandle handleA = sinkArrayDH, &sinkArrayDH };
		/*SmallList<MArrayDataHandle> frameHandles;
		frameHandles.push_back(sinkArrayDH);
		frameHandles.push_back(sinkArrayDH);*/
		MArrayDataHandle frameHandles[2] = { sinkArrayDH, sinkArrayDH };

//		for (int i = 0; i < nFrames; i++) {
//			jumpToElement(sourceFramesDH, i);
//			frameHandles[i % 2] = sourceFramesDH.outputArrayValue();
//			mirrorArrayDataHandle(frameHandles[i % 2 + 1], 
//				sourceFramesDH.outputArrayValue());
///*
//			mirrorArrayDataHandle(sourceInnerArrayDH, sourceArrayDH);
//			mirrorArrayDataHandle(sinkArrayDH, sourceInnerArrayDH);*/
//		}
		

		previousTime = newTime;

		// add to increment
		data.outputValue(aIncrement).setInt(data.outputValue(aIncrement).asInt() + 1);
	}


	// dirty sink plug
	bool switchStatus = !data.outputValue(aSinkConnection).asBool();
	data.outputValue(aSinkConnection).setBool(switchStatus);

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

	//DEBUGS("connection to sink plug");

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
	sinkConnected = true;
}

void MemorySource::clearSinkObj(MObject &obj) {
	// is this enough to clear reference to connected node?
	sinkObj = MObject::kNullObj;
	sinkConnected = false;
}

void* MemorySource::creator(){
	// sink is not connected on creation
	MemorySource *newObj = new MemorySource;
	newObj->sinkConnected = false;
	newObj->sinkObj = MObject::kNullObj;
	newObj->previousTime = 0.0;
    return newObj;

}

MemorySource::MemorySource() {};
MemorySource::~MemorySource() {};

