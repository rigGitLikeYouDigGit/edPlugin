

/*

	memory cell passing on end result of evaluation
	
*/

#include "memorySink.h"
#include "memorySource.h"
#include "lib/api.h"



MTypeId MemorySink::kNODE_ID(0x00122C1A);
MString MemorySink::kNODE_NAME( "memorySink" );

//MObject MemorySink::aTime;
MObject MemorySink::aData;
MObject MemorySink::aSourceConnection;

using namespace ed;

MStatus MemorySink::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;


	//// attribute used to update cell - time is most convenient
	//aTime = uFn.create("time", "time", MFnUnitAttribute::kTime, 0.0);
	//addAttribute(aTime);

	// untyped attribute can be used to pass on whatever you want
	//aData = gFn.create("data", "data");
	aData = tFn.create("data", "data", MFnData::kAny);
	tFn.setReadable(true);
	tFn.setWritable(true);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);
	addAttribute(aData);

	// connection to memorySource node
	aSourceConnection = nFn.create("source", "source", MFnNumericData::kBoolean, 0.0);
	nFn.setReadable(false);
	nFn.setWritable(true);
	addAttribute(aSourceConnection);

	attributeAffects(aSourceConnection, aData);

    return MStatus::kSuccess;
}


MStatus MemorySink::compute(
				const MPlug& plug, MDataBlock& data) {

	MStatus s = MS::kSuccess;
	//if (data.isClean(aSourceConnection)) {
	//	// source has not updated
	//	// this break may not be correct
	//	data.setClean(plug);
	//	return MS::kSuccess;
	//}

	DEBUGS("memorySink compute ");

	dataObjs.clear();

	

	// retrieve MObjects from array attribute, store in vector
	// MArrayDataHandle aDataArrayDH = data.inputArrayValue(aData);
	MArrayDataHandle aDataArrayDH = data.outputArrayValue(aData);
	DEBUGS(" len aDataArrayDH " << aDataArrayDH.elementCount());
	for (int i = 0; i < aDataArrayDH.elementCount(); i++) {
		s = jumpToElement(aDataArrayDH, i);
		MCHECK(s, "failed jte in memory sink");
		MObject obj = MObject(aDataArrayDH.inputValue().data());
		dataObjs.push_back(obj);
	}
	DEBUGS("dataObjs length" << int(dataObjs.size()));

	data.setClean(plug);
    return MS::kSuccess;
}

void* MemorySink::creator(){

    return new MemorySink;

}

MemorySink::MemorySink() {};
MemorySink::~MemorySink() {};

