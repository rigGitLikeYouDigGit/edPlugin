

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
MObject MemorySource::aFloatData;
MObject MemorySource::aSinkConnection;


MStatus MemorySource::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;


	// attribute used to update cell - time is most convenient
	aTime = uFn.create("time", "time", MFnUnitAttribute::kTime, 0.0);
	addAttribute(aTime);

	// attribute used to update cell - time is most convenient
	aResetFrame = uFn.create("resetFrame", "resetFrame", MFnUnitAttribute::kTime, 0.0);
	addAttribute(aResetFrame);

	// untyped attribute can be used to pass on whatever you want
	aData = tFn.create("data", "data", MFnData::kAny);
	tFn.setReadable(true);
	tFn.setWritable(false);
	addAttribute(aData);

	// test
	aFloatData = nFn.create("floatData", "floatData", MFnNumericData::kFloat);
	addAttribute(aFloatData);

	aSinkConnection = nFn.create("sink", "sink", MFnNumericData::kBoolean, 0.0);
	nFn.setReadable(true);
	nFn.setWritable(false);
	addAttribute(aSinkConnection);

	attributeAffects(aTime, aData);
	attributeAffects(aTime, aFloatData);

	attributeAffects(aResetFrame, aData);
	attributeAffects(aResetFrame, aFloatData);

    return MStatus::kSuccess;
}


MStatus MemorySource::compute(
				const MPlug& plug, MDataBlock& data) {
	MStatus s;
	DEBUGS("memorySource compute");
	DEBUGS("memorySource sinkConnected " << sinkConnected);

	if (!sinkConnected || sinkObj == MObject::kNullObj) {
		// do literally nothing
		data.setClean(plug);
		return MS::kSuccess;
	}

	// check if time is reset frame - reset to zero if so
	if (data.inputValue(aTime).asFloat() == data.inputValue(aResetFrame).asFloat()) {
		data.outputValue(aFloatData).setFloat(0.0);
		data.setClean(plug);
		return MS::kSuccess;
	}

	DEBUGS("memorySource connected sink " << MFnDependencyNode(sinkObj).name());
	data.setClean(plug);

	// extract sink data
	MObject sinkData;
	MObject sinkFloatData;
	float sinkFloatValue;
	s = getSinkData(sinkObj, sinkData, sinkFloatData, sinkFloatValue);
	CHECK_MSTATUS_AND_RETURN_IT(s);
	
	// set source data plug
	//s = setOutputSourceData(sinkData, data);
	//data.outputValue(aFloatData).set(sinkFloatData);
	//data.outputValue(aFloatData).setMObject(MObject(sinkFloatData));
	data.outputValue(aFloatData).setFloat(sinkFloatValue);
	//data.outputValue(aData).set(sinkData);
	CHECK_MSTATUS_AND_RETURN_IT(s);



	data.setClean(plug);

    return MS::kSuccess;
}

MStatus MemorySource::getSinkData(MObject &sinkObj, MObject &sinkData, MObject &sinkFloatData, float &floatValue) {
	DEBUGS("memorySource getSinkData")
	MStatus s;
	MPlug sinkPlug = MPlug(sinkObj, MemorySink::aData);
	//sinkData = sinkPlug.asMObject();

	// check float data
	MPlug floatPlug = MPlug(sinkObj, MemorySink::aFloatData);
	float testFloat = floatPlug.asFloat();
	floatValue = testFloat;
	//sinkFloatData = floatPlug.asMDataHandle().data();
	

	DEBUGS("found float value " << testFloat);

	sinkData = sinkPlug.asMDataHandle().data();

	return MStatus::kSuccess;
}

MStatus MemorySource::setOutputSourceData(MObject &sinkData, MDataBlock &data) {
	DEBUGS("memorySource setOutputData")
	MStatus s;
	//DEBUGS("obj value "<<)
	//MPlug sourcePlug = MPlug(thisMObject(), aData);
	//sourcePlug.setMObject(sinkData);
	//sourcePlug.asMDataHandle().set(sinkData);

	data.outputValue(aData).set(sinkData);
	return MStatus::kSuccess;
}



MStatus MemorySource::connectionMade(
	const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// check if sink has been connected 
	DEBUGS("memorySource connectionMade");

	//DEBUGS("plug " << plug.name() << " otherPlug " << otherPlug.name());

	// we only care about sink plug
	if (plug.attribute() != aSinkConnection) {
		return MPxNode::connectionMade(plug, otherPlug, asSrc);
	}

	DEBUGS("connection to sink plug");

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
	DEBUGS("memorySource connectionBroken");

	if (plug.attribute() != aSinkConnection) {
		return MPxNode::connectionBroken(plug, otherPlug, asSrc);
	}
	clearSinkObj(MObject::kNullObj);

	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
}

void MemorySource::setSinkObj(MObject &obj) {
	sinkObj = obj;
	sinkConnected = true;
}

void MemorySource::clearSinkObj(MObject &obj) {
	sinkObj = MObject::kNullObj;
	sinkConnected = false;
}

void* MemorySource::creator(){
	// sink is not connected on creation
	MemorySource *newObj = new MemorySource;
	newObj->sinkConnected = false;
	newObj->sinkObj = MObject::kNullObj;
    return newObj;

}



MemorySource::MemorySource() {};
MemorySource::~MemorySource() {};

