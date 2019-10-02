/*
simple inflate deformer with dynamic weights
check out how long it takes to get to actual deformation

point1 = point0 + normal * weight * distance


*/

#include "edPush.h"

MTypeId EdPush::id(0x00122C05);
MString EdPush::nodeName( "edPush" );
MObject EdPush::aOffset;
MObject EdPush::aWeights;



MStatus EdPush::initialize()
{
    // initialise attributes
    MStatus status;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;

    aOffset = nAttr.create( "offset", "off", MFnNumericData::kDouble, 0.0);
    nAttr.setStorable(true);
    nAttr.setWritable(true);
    status = addAttribute(aOffset);

    aWeights = tAttr.create( "mask", "msk", MFnData::kDoubleArray);
    tAttr.setStorable(true);
    tAttr.setWritable(true);
    tAttr.setHidden(false);
    // only works for weights on one mesh input for now
    status = addAttribute(aWeights);

    status = attributeAffects(aOffset, outputGeom);
    status = attributeAffects(aWeights, outputGeom);

    return MStatus::kSuccess;
}

MStatus EdPush::deform(
    MDataBlock& data, MItGeometry& itGeo,
    const MMatrix &localToWorldMatrix,
    unsigned int mIndex
){

    MObject thisNode = thisMObject();
    MStatus status;

    // grab input parametres
    double offset = data.inputValue( aOffset ).asDouble();
    double envelopeVal = data.inputValue( envelope ).asFloat();

    // check input weights are valid
    MDataHandle weightsHandle = data.inputValue( aWeights, &status);
    MFnDoubleArrayData weightsData( weightsHandle.data(), &status );
    if (status != MS::kSuccess){
        MGlobal::displayError( "Invalid weights data for edPush \n");
        return MS::kFailure;
    }
    MDoubleArray wData = weightsData.array();

    // check that current index can be found in weights
    if ( wData.length() < itGeo.count() ){
        MGlobal::displayError( "Weight array too short for edPush \n");
        return MS::kFailure;
    }

    // deform points
    for ( int i=0; !itGeo.isDone(); itGeo.next() ){
        MPoint origPos = itGeo.position();
        MVector normal = itGeo.normal();
        double weightValue = wData[ i++ ];
        MPoint newPos = origPos + normal * weightValue * envelopeVal;
        itGeo.setPosition( newPos );
    }
    return MS::kSuccess;
}

void* EdPush::creator(){
    return new EdPush;
}

EdPush::EdPush() {};
EdPush::~EdPush() {};
