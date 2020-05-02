

/*

	converts maya mesh to raw float and int buffers of position and topo data
	
*/

#include "meshToBuffers.h"



MTypeId MeshToBuffers::kNODE_ID(0x00122C08);
MString MeshToBuffers::kNODE_NAME( "meshToBuffers" );

MObject MeshToBuffers::aTest;
MObject MeshToBuffers::aInMesh;
MObject MeshToBuffers::aPointPositions;
MObject MeshToBuffers::aFaceCounts;
MObject MeshToBuffers::aFaceConnects;
MObject MeshToBuffers::aPointConnects;
MObject MeshToBuffers::aExtraPointConnects;
MObject MeshToBuffers::aFaceCentres;
MObject MeshToBuffers::aNormals;
MObject MeshToBuffers::aUvCoords;
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

	//aFaceCentres = tAttr.create("faceCentres", "faceCentres", MFnData::kFloatArray);
	//addAttribute(aFaceCentres ); // more efficient to find online


//    aFaceConnects = tAttr.create("faceConnects", "faceConnects", MFnData::kIntArray);
//    addAttribute( aFaceConnects );

    aPointPositions = tAttr.create("pointPositions", "pointPositions", MFnData::kFloatArray);
    addAttribute( aPointPositions );

	aPointConnects = tAttr.create("pointConnects", "pointConnects", MFnData::kIntArray);
	addAttribute(aPointConnects);

	// extra high valence point connects
	aExtraPointConnects = tAttr.create("extraPointConnects", "extraPointConnects", MFnData::kIntArray);
	addAttribute(aExtraPointConnects);

	aNormals = tAttr.create("normals", "normals", MFnData::kFloatArray);
	addAttribute(aNormals);

	// uv buffers are arrays, one for each uv set of mesh
	aUvCoords = tAttr.create("uvCoords", "uvCoords", MFnData::kFloatArray);
	tAttr.setArray(true);
	tAttr.setUsesArrayDataBuilder(true);
	addAttribute(aUvCoords);


    // bind
    // aBind = makeBindAttr(); // gives random errors
	MFnEnumAttribute fn;
	aBind = fn.create("bind", "bind", 1);
	fn.addField("off", 0);
	fn.addField("bind", 1);
	fn.addField("bound", 2);
	fn.addField("live", 3);
	fn.setKeyable(true);
	fn.setHidden(false);
    addAttribute( aBind );

    MStatus status;
	attributeAffects(aBind, aPointPositions);
	attributeAffects(aBind, aPointConnects);
	attributeAffects(aBind, aFaceCounts);

	attributeAffects(aInMesh, aPointPositions);
	attributeAffects(aInMesh, aPointConnects);
	attributeAffects(aInMesh, aFaceCounts);

/*
	std::vector<MObject> topoAttrs = { aFaceCounts, aPointConnects };
	std::vector<MObject> liveAttrs = { aPointPositions };
	setAttributeAffectsAll(aBind, topoAttrs);
	setAttributeAffectsAll(aBind, liveAttrs);
	setAttributeAffectsAll(aInMesh, topoAttrs);
	setAttributeAffectsAll(aInMesh, liveAttrs);*/

    return MStatus::kSuccess;
}


MStatus MeshToBuffers::compute(
				const MPlug& plug, MDataBlock& data) {
    // going with floats for now, can easily switch to doubles if needed
	// initialise MFnMesh
	DEBUGS("MeshToBuffers compute")
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

	        for( unsigned int n = 0; n < faceVertices.length(); n++ ){
	            allFaceVertices.set( faceVertices[ n ], i * 4 + n );
	        }
	    }

	    MFnIntArrayData faceData;
	    MObject faceObj = faceData.create( allFaceVertices );

        MDataHandle faceDH = data.outputValue( aFaceCounts );
        faceDH.setMObject( faceObj );

		// find point connections
		std::vector<int> faceVector = MIntArrayToVector(allFaceVertices);
		DEBUGS("faceVector");
		DEBUGVI(faceVector);

		std::vector<int> pointConnects, extraPointConnects;
		tie(pointConnects, extraPointConnects) = pointBufferFromFaceBuffer(faceVector);

		MIntArray pointConnectsArray = vectorToMIntArray(pointConnects);
		MObject pointObj = faceData.create(pointConnectsArray);
		MDataHandle pointConnectsDH = data.outputValue(aPointConnects);
		pointConnectsDH.setMObject(pointObj);

		MIntArray extraPointConnectsArray = vectorToMIntArray(extraPointConnects);
		MObject extraPointObj = faceData.create(pointConnectsArray);
		MDataHandle extraPointConnectsDH = data.outputValue(aExtraPointConnects);
		extraPointConnectsDH.setMObject(extraPointObj);

	    if( bind == 1){
	        data.inputValue( aBind ).setInt( 2 );
	    }

	}

	

	// set outputs
	MDataHandle positionsDH = data.outputValue( aPointPositions );
	positionsDH.setMObject( positionsData );

	data.setClean(plug);


    return MS::kSuccess;
}


void* MeshToBuffers::creator(){

    return new MeshToBuffers;

}


MeshToBuffers::MeshToBuffers() {};
MeshToBuffers::~MeshToBuffers() {};

