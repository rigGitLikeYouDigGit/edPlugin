/*
simple inflate deformer with dynamic weights
focus on the most efficient boilerplate

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
    MDoubleArray defaultArr;
    MObject defaultObj;
    MFnDoubleArrayData defaultData;

    aOffset = nAttr.create( "offset", "off", MFnNumericData::kDouble, 0.0);
    nAttr.setStorable(true);
    nAttr.setWritable(true);
    nAttr.setKeyable(true);
    status = addAttribute(aOffset);

    // mask default
    // defaultArr = MDoubleArray(100, 1.0); // length, count
    // defaultObj = defaultData.create(defaultArr);

    aMask = tAttr.create( "mask", "msk", MFnData::kDoubleArray);
    tAttr.setStorable(true);
    tAttr.setWritable(true);
    tAttr.setHidden(false);
    //tAttr.setDefault( defaultObj );
    //tAttr.setArray(true);
    // only works for weights on one mesh input for now
    status = addAttribute(aMask);

    status = attributeAffects(aOffset, outputGeom);
    status = attributeAffects(aMask, outputGeom);

    // make mask and weights paintable
    MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer edPush weights");
    MGlobal::executeCommand("makePaintable -attrType doubleArray edPush mask");
    // how best to initialise mask to 1.0?

    return MStatus::kSuccess;
}


/// ON CONNECTION MADE
// crashes with total consistency
MStatus EdPush::connectionMade( const MPlug& nodePlug, const MPlug& foreignPlug,
    bool asSrc){


    // // MItGeometry iter;
    // MFnDependencyNode mfnDep;
    // // initialise mask properly
    // if (nodePlug.asMObject() == inputGeom){
    //
    //
    //     MObject meshAttr = nodePlug.asMObject();
    //     MItGeometry iter = MItGeometry( meshAttr );
    //     int length = iter.count();
    //
    //     MObject node = nodePlug.node();
    //     MFnDependencyNode mfnDep( node );
    //     MPlug maskPlug = mfnDep.findPlug( aMask, false);
    //     MDataHandle maskHandle = maskPlug.asMDataHandle();
    //
    //     initialiseMasks( length, maskHandle);
    // }
    MStatus result = MPxDeformerNode::connectionMade( nodePlug, foreignPlug, asSrc);
    return result;

}

//void EdPush::initialiseMasks(int length, MObject maskAttributes[]){
//void EdPush::initialiseMasks(int length, std::vector maskAttributes){
// how would I construct and pass a vector of MObjects procedurally like this?

// MDoubleArray EdPush::initialiseMasks(int length, MDataBlock &data){
MDoubleArray EdPush::initialiseMasks(int length, MDataHandle &maskHandle){

    //MDataHandle maskHandle = data.inputValue( aMask );
    MFnDoubleArrayData maskData( maskHandle.data());
    //MFnDoubleArrayData maskData( data.inputValue( aMask) );
    MDoubleArray mask = maskData.array();
    if (mask.length() < length){
        mask.setLength(length);
        for( int n; n < length; n++){
            mask[n] = 1.0;
        }
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
    MDataHandle maskHandle = data.inputValue(aMask);
    MDoubleArray mask = initialiseMasks(length, maskHandle);

    // grab input parametres
    double offset = data.inputValue( aOffset ).asDouble();
    double envelopeVal = data.inputValue( envelope ).asFloat();
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
    // if ( mask.length() < length ){
    //     MGlobal::displayError( "Weight array too short for edPush \n");
    //     return MS::kFailure;
    // }

    //itGeo.reset();
    // deform points
    for ( int i=0; !itGeo.isDone(); itGeo.next() ){
        float weight = weightValue(data, mIndex, i);
        MPoint origPos = itGeo.position();
        MVector normal = itGeo.normal();
        // double maskValue = mData[ i ];
        // MPoint newPos = origPos + normal * maskValue * envelopeVal;
        double distance = offset * envelopeVal * weight * mask[i];
        //float distance = offset * envelopeVal * weight;
        MPoint newPos = origPos + normal * distance;
        itGeo.setPosition( newPos );
        i++;
    }
    return MS::kSuccess;
}

void* EdPush::creator(){
    // stuff to initialise mask to 1
    // EdPush created = new EdPush;
    // MObject obj = created.thisMObject();
    // return created;
    return new EdPush;

}

EdPush::EdPush() {};
EdPush::~EdPush() {};
