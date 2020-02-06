

/*

your description here

*/

#include "testDeformer.h"

MTypeId TestDeformer::kNODE_ID(0x00122C01);
MString TestDeformer::kNODE_NAME( "testDeformer" );

MStatus TestDeformer::initialize()
{
    // initialise attributes

    return MStatus::kSuccess;
}


MStatus TestDeformer::
		 deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex)    {
    return MS::kSuccess;
}

void* TestDeformer::creator(){

    return new TestDeformer;

}

TestDeformer::TestDeformer() {};
TestDeformer::~TestDeformer() {};

