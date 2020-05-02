

/*

	memory cell passing on end result of evaluation
	
*/

#include "memorySink.h"
#include "memorySource.h"

MTypeId MemorySink::kNODE_ID(0x00122C1A);
MString MemorySink::kNODE_NAME( "memorySink" );

MObject MemorySink::aTime;
MObject MemorySink::aData;
MObject MemorySink::aSourceConnection;


MStatus MemorySink::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;


	// attribute used to update cell - time is most convenient
	aTime = uFn.create("time", "time", MFnUnitAttribute::kTime, 0.0);
	addAttribute(aTime);

	// untyped attribute can be used to pass on whatever you want
	//aData = gFn.create("data", "data");
	aData = tFn.create("data", "data", MFnData::kAny);
	tFn.setReadable(false);
	tFn.setWritable(true);
	addAttribute(aData);

	aSourceConnection = nFn.create("source", "source", MFnNumericData::kBoolean, 0.0);
	nFn.setReadable(false);
	nFn.setWritable(true);
	addAttribute(aSourceConnection);

    return MStatus::kSuccess;
}


MStatus MemorySink::compute(
				const MPlug& plug, MDataBlock& data) {

	data.setClean(plug);

    return MS::kSuccess;
}

void* MemorySink::creator(){

    return new MemorySink;

}

MemorySink::MemorySink() {};
MemorySink::~MemorySink() {};

