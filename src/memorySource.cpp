

/*

	memory cell receiving result of previous evaluation

	source pulls from sink ahead of it, before sink is evaluated,
	instead of sink updating source -
	this seems less likely to cause dirty problems
	
*/

#include "memorySource.h"
#include "memorySink.h"

MTypeId MemorySource::kNODE_ID(0x00122C1B);
MString MemorySource::kNODE_NAME( "memorySource" );

MObject MemorySource::aTime;
MObject MemorySource::aResetFrame;
MObject MemorySource::aData;
MObject MemorySource::aInnerData;
MObject MemorySource::aSinkConnection;
MObject MemorySource::aIncrement;


MStatus MemorySource::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;


	// attribute used to update cell - time is most convenient
	aTime = uFn.create("time", "time", MFnUnitAttribute::kTime, 0.0);
	tFn.setReadable(true);
	tFn.setWritable(true);
	addAttribute(aTime);

	// attribute used to update cell - time is most convenient
	aResetFrame = uFn.create("resetFrame", "resetFrame", MFnUnitAttribute::kTime, 1.0);
	addAttribute(aResetFrame);

	// untyped attribute can be used to pass on whatever you want
	aData = tFn.create("data", "data", MFnData::kAny);
	tFn.setReadable(true);
	tFn.setWritable(false);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);
	addAttribute(aData);

	// internal cache array mirroring main interface
	aInnerData = tFn.create("innerData", "innerData", MFnData::kAny);
	tFn.setReadable(true);
	tFn.setWritable(true);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);
	addAttribute(aInnerData);

	// boolean connection to sink
	aSinkConnection = nFn.create("sink", "sink", MFnNumericData::kBoolean, 0.0);
	nFn.setReadable(true);
	nFn.setWritable(false);
	addAttribute(aSinkConnection);

	// can be handy for some uses to have an exclusively positive counter
	aIncrement = nFn.create("increment", "increment", MFnNumericData::kInt, 0.0);
	nFn.setReadable(true);
	nFn.setWritable(false);
	addAttribute(aIncrement);

	attributeAffects(aTime, aData);
	attributeAffects(aTime, aInnerData);
	attributeAffects(aTime, aIncrement);

	attributeAffects(aResetFrame, aData);
	attributeAffects(aResetFrame, aInnerData);
	attributeAffects(aResetFrame, aIncrement);

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

	// get source data handles
	MArrayDataHandle sourceArrayDH = data.outputArrayValue(aData);
	MArrayDataHandle sourceInnerArrayDH = data.outputArrayValue(aInnerData);

	// extract sink data
	MDataHandle sinkDH;
	s = getSinkData(sinkObj, sinkDH);
	MArrayDataHandle sinkArrayDH = MArrayDataHandle(sinkDH);
	
	// check if time is reset frame - reset to current values if so
	if (data.inputValue(aTime).asFloat() == data.inputValue(aResetFrame).asFloat()) {
		DEBUGS("memorySource time reset")
		mirrorArrayDataHandle(sinkArrayDH, sourceInnerArrayDH);
		mirrorArrayDataHandle(sinkArrayDH, sourceArrayDH);

		// reset counter
		data.outputValue(aIncrement).setInt(0);

		data.setClean(plug);
		return MS::kSuccess;
	}


	// only if time has changed, update current values from cached
	//MPlug timePlug = MPlug(thisMObject(), MemorySource::aTime);
	//if (!data.isClean(aTime)) {
	//if (!data.isClean(timePlug)) {

	// for some reason input time plug was never reading as dirty
	if( previousTime != data.inputValue(aTime).asFloat()){
		DEBUGS("memorySource time dirty");
		mirrorArrayDataHandle(sourceInnerArrayDH, sourceArrayDH);
		mirrorArrayDataHandle(sinkArrayDH, sourceInnerArrayDH);

		previousTime = data.inputValue(aTime).asFloat();

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

