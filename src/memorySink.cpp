

/*

	memory cell passing on end result of evaluation
	
*/

#include "memorySink.h"

MTypeId MemorySink::kNODE_ID(0x00122C1A);
MString MemorySink::kNODE_NAME( "memorySink" );



MStatus MemorySink::initialize()
{
    // initialise attributes

    return MStatus::kSuccess;
}


MStatus MemorySink::compute(
				const MPlug& plug, MDataBlock& data) {
    return MS::kSuccess;
}

void* MemorySink::creator(){

    return new MemorySink;

}

MemorySink::MemorySink() {};
MemorySink::~MemorySink() {};

