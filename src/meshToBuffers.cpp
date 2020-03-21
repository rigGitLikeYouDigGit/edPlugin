

/*

	converts maya mesh to raw float and int buffers of position and topo data
	
*/

#include "meshToBuffers.h"

MTypeId MeshToBuffers::kNODE_ID(0x00122C08);
MString MeshToBuffers::kNODE_NAME( "meshToBuffers" );

MObject MeshToBuffers::aTest;
MObject MeshToBuffers::aInMesh;
MObject MeshToBuffers::aPositions;
MObject MeshToBuffers::aFaceCounts;
MObject MeshToBuffers::aFaceConnects;
// connects actually pointless with constant vertices per face
MObject MeshToBuffers::aBind;

MStatus MeshToBuffers::initialize()
{
    // initialise attributes
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;

    // main inputs
    aInMesh = tAttr.create("inMesh", "inMesh", MFnData::kMesh);
    tAttr.setReadable(true);
    tAttr.setWritable(true);
    addAttribute(aInMesh);

    // outputs
    aFaceCounts = tAttr.create("faceCounts", "faceCounts", MFnData::kIntArray);
    addAttribute( aFaceCounts );

//    aFaceConnects = tAttr.create("faceConnects", "faceConnects", MFnData::kIntArray);
//    addAttribute( aFaceConnects );

    aPositions = tAttr.create("positions", "positions", MFnData::kFloatArray);
    addAttribute( aPositions );


    // bind
    aBind = makeBindAttr();
    addAttribute( aBind );

    MStatus status;
    status = attributeAffects(aInMesh, aFaceCounts);
    //status = attributeAffects(aInMesh, aFaceConnects);
    status = attributeAffects(aInMesh, aPositions);

    status = attributeAffects(aBind, aFaceCounts);
    //status = attributeAffects(aBind, aFaceConnects);
    status = attributeAffects(aBind, aPositions);

    return MStatus::kSuccess;
}


MStatus MeshToBuffers::compute(
				const MPlug& plug, MDataBlock& data) {
    // going with floats for now, can easily switch to doubles if needed
	// initialise MFnMesh
	MObject meshObj = data.inputValue( aInMesh ).asMesh() ;
	MFnMesh meshFn;
	meshFn.setObject(meshObj);
	int nPoints = meshFn.numVertices();
	int nPolys = meshFn.numPolygons();

	// positions first
	MFloatArray positions = MFloatArray( nPoints * 3, 0.0);
	// don't know how to do rawPoints yet
	MFloatPointArray points;
	meshFn.getPoints( points );
	for( int i=0; i < nPoints; i++){
	    positions.set( points[ i ].x, i*3 );
	    positions.set( points[ i ].y, i*3+1 );
	    positions.set( points[ i ].z, i*3+2 );
	}
	MFnFloatArrayData floatData;
	MObject positionsData = floatData.create( positions );

	// check bind
	int bind = data.inputValue( aBind ).asInt() ;
	if( bind == 1 || bind == 3 ){ // bind or live
	    // do binding with topology buffers
	    // we don't triangulate here, but assume count of 4 per face
	    MIntArray allFaceVertices = MIntArray( nPolys * 4, -1);
	    for( int i = 0; i < nPolys; i++){
	        MIntArray faceVertices;
	        meshFn.getPolygonVertices( i, faceVertices );

	        for( int n = 0; n < faceVertices.length(); n++ ){
	            allFaceVertices.set( faceVertices[ n ], i * 4 + n );
	        }
	    }

	    MFnIntArrayData faceData;
	    MObject faceObj = faceData.create( allFaceVertices );

        MDataHandle faceDH = data.outputValue( aFaceCounts );
        faceDH.setMObject( faceObj );

	    if( bind == 1){
	        data.inputValue( aBind ).setInt( 2 );
	    }

	}


	// set outputs
	MDataHandle positionsDH = data.outputValue( aPositions );
	positionsDH.setMObject( positionsData );


    return MS::kSuccess;
}


void* MeshToBuffers::creator(){

    return new MeshToBuffers;

}


MeshToBuffers::MeshToBuffers() {};
MeshToBuffers::~MeshToBuffers() {};

