/*
simple inflate deformer with dynamic weights
check out how long it takes to get to actual deformation

point1 = point0 + normal * weight * distance

deformer -type "edPush"

*/

#include "edPush.h"

MTypeId EdPush::id(0x00122C05);
MString EdPush::nodeName( "edPush" );
MObject EdPush::aOffset;
MObject EdPush::aMask;



MStatus EdPush::initialize()
{
    // initialise attributes
    MStatus status;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;

    aOffset = nAttr.create( "offset", "off", MFnNumericData::kDouble, 0.0);
    nAttr.setStorable(true);
    nAttr.setWritable(true);
    nAttr.setKeyable(true);
    status = addAttribute(aOffset);

    aMask = tAttr.create( "mask", "msk", MFnData::kDoubleArray);
    tAttr.setStorable(true);
    tAttr.setWritable(true);
    tAttr.setHidden(false);
    //tAttr.setArray(true);
    // only works for weights on one mesh input for now
    status = addAttribute(aMask);

    status = attributeAffects(aOffset, outputGeom);
    status = attributeAffects(aMask, outputGeom);

    // make mask and weights paintable
    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer edPush weights");
    MGlobal::executeCommand("makePaintable -attrType doubleArray edPush mask");

    return MStatus::kSuccess;
}

//void EdPush::initialiseMasks(int length, MObject maskAttributes[]){
//void EdPush::initialiseMasks(int length, std::vector maskAttributes){
// how would I construct and pass a vector of MObjects procedurally like this?

MDoubleArray EdPush::initialiseMasks(int length, MDataBlock &data){

    MDataHandle maskHandle = data.inputValue( aMask );
    MFnDoubleArrayData maskData( maskHandle.data());
    //MFnDoubleArrayData maskData( data.inputValue( aMask) );
    MDoubleArray mask = maskData.array();
    if (mask.length() < length){
        mask.setLength(length);
    }

    return mask;

}

MStatus EdPush::deform(
    MDataBlock& data, MItGeometry& itGeo,
    const MMatrix& localToWorldMatrix,
    unsigned int mIndex
){

    MObject thisNode = thisMObject();
    MStatus status;

    // set length of mask array
    int length = itGeo.count();
    MDoubleArray mask = initialiseMasks(length, data);

    // grab input parametres
    double offset = data.inputValue( aOffset ).asDouble();
    double envelopeVal = data.inputValue( envelope ).asDouble();
    //
    // // check input weights are valid
    // MDataHandle maskHandle = data.inputValue( aMask, &status);
    // MFnDoubleArrayData maskData( maskHandle.data(), &status );
    // if (status != MS::kSuccess){
    //     MGlobal::displayError( "Invalid mask data for edPush \n");
    //     return MS::kFailure;
    // }
    // MDoubleArray mData = maskData.array();
    //
    // check that current index can be found in weights
    if ( mask.length() < length ){
        MGlobal::displayError( "Weight array too short for edPush \n");
        return MS::kFailure;
    }

    itGeo.reset();
    // deform points
    for ( int i=0; !itGeo.isDone(); itGeo.next() ){
        float weight = weightValue(data, mIndex, i);
        MPoint origPos = itGeo.position();
        MVector normal = itGeo.normal();
        // double maskValue = mData[ i ];
        // MPoint newPos = origPos + normal * maskValue * envelopeVal;
        MPoint newPos = origPos + normal * offset * envelopeVal * weight * mask[i];
        itGeo.setPosition( newPos );
        i++;
    }
    return MS::kSuccess;
}

void* EdPush::creator(){
    return new EdPush;
}

EdPush::EdPush() {};
EdPush::~EdPush() {};
