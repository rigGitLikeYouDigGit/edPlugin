

/*

your description here

*/

#include "uberDeformer.h"

MTypeId UberDeformer::kNODE_ID(0x00122C05);
MString UberDeformer::kNODE_NAME( "uberDeformer" );

MStatus UberDeformer::initialize()
{
    // initialise attributes

    return MStatus::kSuccess;
}


MStatus UberDeformer::
		 deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex)    {
    return MS::kSuccess;
}

void* UberDeformer::creator(){

    return new UberDeformer;

}

UberDeformer::UberDeformer() {};
UberDeformer::~UberDeformer() {};

