

/*

node for extracting information from mesh surface and character -
density, tension, gradient, etc
other values like velocity may be found from further operations
use separate mesh time process node to access previous values

*/

#include "meshAnalysis.h"

MTypeId MeshAnalysis::kNODE_ID(0x00122C04);
MString MeshAnalysis::kNODE_NAME( "meshAnalysis" );

MObject MeshAnalysis::aInMesh;
MObject MeshAnalysis::aRefMesh;
MObject MeshAnalysis::aTension;
MObject MeshAnalysis::aCurvature;
MObject MeshAnalysis::aBind;
MObject MeshAnalysis::aDeltas;


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

    aCurvature = tAttr.create( "curvature", "curvature", MFnData::kDoubleArray);
    addAttribute(aCurvature);

    aTension = tAttr.create( "tension", "tension", MFnData::kDoubleArray);
    addAttribute(aTension);

    MFnVectorArrayData vArrayData;
    aDeltas = tAttr.create( "deltas", "deltas", MFnData::kVectorArray,
        vArrayData.create());
    addAttribute( aDeltas );

    // caching
    aRefMesh = tAttr.create("refMesh", "refMesh", MFnData::kMesh);
    tAttr.setReadable(true);
    tAttr.setWritable(true);
    addAttribute(aRefMesh);

    // if refMesh is connected, compute values relative to that
    // else use the cached bound mesh
    // ELSE compute the absolute values live
    aBind = makeBindAttr();
    addAttribute( aBind );


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

