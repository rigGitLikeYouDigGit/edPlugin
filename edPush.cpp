/*
simple inflate deformer with dynamic weights
check out how long it takes to get to actual deformation

point1 = point0 + normal * weight * distance


*/

#include <maya/MFnPlugin.h>
#include <MTypeId.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include "edPush.h"

MTypeId EdPush::id(0x00122C05);
Mstring EdPush::nodeName( "edPush" );
MObject EdPush::aOffset;
MObject EdPush::aWeights;


MStatus EdPush::initialize()
{
    // initialise attributes
    MStatus status;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;

    aOffset = nAttr.create( "offset", "off", MFnNumericData::kDouble, 0.0);
    aOffset.setStorable(true);
    aOffset.setWritable(true);

    aWeights = tAttr.create( "weights", "wt", MFnData::kDoubleArray);
    aWeights.setStorable(true);
    aWeights.setWritable(true);
    // only works for weights on one mesh input for now

    status = addAttribute(aOffset);
    status = addAttribute(aWeights);

    status = attributeAffects(aOffset, outputGeom);
    status = attributeAffects(aWeights, outputGeom);

    MCheckStatus(status, "ERROR in setting up attributes on edPush \n"):
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



}
