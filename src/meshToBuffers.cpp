

/*

	converts maya mesh to raw float and int buffers of position and topo data
	
*/

#include "meshToBuffers.h"

MTypeId MeshToBuffers::kNODE_ID(0x00122C08);
MString MeshToBuffers::kNODE_NAME( "meshToBuffers" );

MObject MeshToBuffers::aTest;


MStatus MeshToBuffers::initialize()
{
    // initialise attributes

    return MStatus::kSuccess;
}


MStatus MeshToBuffers::compute(
				const MPlug& plug, MDataBlock& data) {
    return MS::kSuccess;
}


void* MeshToBuffers::creator(){

    return new MeshToBuffers;

}


MeshToBuffers::MeshToBuffers() {};
MeshToBuffers::~MeshToBuffers() {};

