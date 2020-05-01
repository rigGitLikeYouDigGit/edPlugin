

/*

	memory cell receiving result of previous evaluation
	
*/

#include "memorySource.h"

MTypeId MemorySource::kNODE_ID(0x00122C1B);
MString MemorySource::kNODE_NAME( "memorySource" );



MStatus MemorySource::initialize()
{
    // initialise attributes

    return MStatus::kSuccess;
}


MStatus MemorySource::compute(
				const MPlug& plug, MDataBlock& data) {
    return MS::kSuccess;
}

void* MemorySource::creator(){

    return new MemorySource;

}

MemorySource::MemorySource() {};
MemorySource::~MemorySource() {};

