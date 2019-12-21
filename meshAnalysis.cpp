

/*

your description here

*/

#include "meshAnalysis.h"

MTypeId MeshAnalysis::kNODE_ID(0x00122C04);
MString MeshAnalysis::kNODE_NAME( "meshAnalysis" );

MObject MeshAnalysis::aInMesh;


MStatus MeshAnalysis::initialize()
{
    // initialise attributes
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;

    // main inputs
    aInMesh = tAttr.create("inMesh", "inMesh", MFnData::kMesh);
    tAttr.setReadable(true);
    tAttr.setWritable(true);
    addAttribute(aInMesh);

    // cache attributes
//        aMask = tAttr.create( "mask", "msk", MFnData::kDoubleArray);
//    tAttr.setStorable(true);
//    tAttr.setWritable(true);
//    tAttr.setHidden(false);
//    //tAttr.setDefault( defaultObj );
//    //tAttr.setArray(true);
//    // only works for weights on one mesh input for now
//    status = addAttribute(aMask);

    return MStatus::kSuccess;
}



MStatus MeshAnalysis::
	    compute(const MPlug& plug, MDataBlock& data)
		 {
    return MS::kSuccess;
}

void* MeshAnalysis::creator(){

    return new MeshAnalysis;

}

MeshAnalysis::MeshAnalysis() {};
MeshAnalysis::~MeshAnalysis() {};

