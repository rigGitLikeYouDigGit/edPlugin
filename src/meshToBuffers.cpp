

/*

	converts maya mesh to raw float and int buffers of position and topo data

*/

#include "meshToBuffers.h"


using namespace ed;
using namespace std;

MTypeId MeshToBuffers::kNODE_ID(0x00122C08);
MString MeshToBuffers::kNODE_NAME( "meshToBuffers" );

MObject MeshToBuffers::aInMesh;
MObject MeshToBuffers::aPointPositions;
MObject MeshToBuffers::aFaceOffsets;
MObject MeshToBuffers::aFaceConnects;
MObject MeshToBuffers::aPointConnects;
MObject MeshToBuffers::aPointOffsets;
MObject MeshToBuffers::aFaceCentres;
MObject MeshToBuffers::aNormals;
MObject MeshToBuffers::aUvCoords;
// connects actually pointless with constant vertices per face
MObject MeshToBuffers::aBind;

MObject MeshToBuffers::aPointVectors;
MObject MeshToBuffers::aFaceVectors;
MObject MeshToBuffers::aPointNormalVectors;
MObject MeshToBuffers::aFaceNormalVectors;

MObject MeshToBuffers::aHedgeMesh;

MStatus MeshToBuffers::initialize()
{
    // initialise attributes
    MFnTypedAttribute tFn;
    MFnNumericAttribute nFn;
	//MFnEnumAttribute fn;


    // main inputs
    aInMesh = tFn.create("inMesh", "inMesh", MFnData::kMesh);
    tFn.setReadable(true);
    tFn.setWritable(true);
    //addAttribute(aInMesh);

    // outputs
	aFaceConnects = tFn.create("faceConnects", "faceConnects", MFnData::kIntArray);
	//addAttribute(aFaceConnects);

    aFaceOffsets = tFn.create("faceOffsets", "faceOffsets", MFnData::kIntArray);
    //addAttribute( aFaceOffsets );


    aPointPositions = tFn.create("pointPositions", "pointPositions", MFnData::kFloatArray);
    //addAttribute( aPointPositions );

	aPointConnects = tFn.create("pointConnects", "pointConnects", MFnData::kIntArray);
	//addAttribute(aPointConnects);

	aPointOffsets = tFn.create("pointOffsets", "pointOffsets", MFnData::kIntArray);
	//addAttribute(aPointOffsets);

	aNormals = tFn.create("normals", "normals", MFnData::kFloatArray);
	//addAttribute(aNormals);

	// uv buffers are arrays, one for each uv set of mesh
	aUvCoords = tFn.create("uvCoords", "uvCoords", MFnData::kFloatArray);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);
	//addAttribute(aUvCoords);


    // bind
	/*aBind = makeBindAttr("bind");*/
	aBind = makeEnumAttr<BindState>("bind");

	// vector values
	aPointVectors = nFn.create("pointVectors", "pointVectors",
		MFnNumericData::k3Double);
	aFaceVectors = nFn.create("pointVectors", "pointVectors",
		MFnNumericData::k3Double);

	vector<MObject> drivers = {
		aInMesh, aBind
	};
	vector<MObject> driven = {
		aFaceConnects, aFaceOffsets, aPointPositions, aPointConnects,
		aPointOffsets, aFaceCentres, aNormals, aUvCoords,
	};

	addAttributes<MeshToBuffers>(drivers);
	addAttributes<MeshToBuffers>(driven);
	setAttributesAffect<MeshToBuffers>(drivers, driven);
	


    return MStatus::kSuccess;
}


MStatus MeshToBuffers::compute(
				const MPlug& plug, MDataBlock& data) {
    // going with floats for now, can easily switch to doubles if needed
	// initialise MFnMesh
	//DEBUGS("MeshToBuffers compute")
	MStatus s = MS::kSuccess;
	MObject meshObj = data.inputValue( aInMesh ).asMesh() ;
	MFnMesh meshFn;
	meshFn.setObject(meshObj);
	int nPoints = meshFn.numVertices();
	int nPolys = meshFn.numPolygons();

	// positions first
	const float * rawPoints = meshFn.getRawPoints(&s);

	//MFloatArray positions = MFloatArray( nPoints * 3, 0.0);
	MFloatArray positions = MFloatArray( rawPoints, nPoints * 3);
	// don't know how to do rawPoints yet
	// MFloatPointArray points;
	// meshFn.getPoints( points );
	// for( int i=0; i < nPoints; i++){
	//     positions.set( points[ i ].x, i*3 );
	//     positions.set( points[ i ].y, i*3+1 );
	//     positions.set( points[ i ].z, i*3+2 );
	// }
	MFnFloatArrayData floatData;
	MObject positionsData = floatData.create( positions );

	// check bind
	int bind = data.inputValue( aBind ).asInt() ;
	if( bind == 1 || bind == 3 ){ // bind or live
	    // do binding with topology buffers

		// face buffers
	    MIntArray allFaceVertices;
		MIntArray faceVertexOffsets = MIntArray(nPolys); // offsets into allFaceVertices
		int offsetIndex = 0;
		//DEBUGS("nPolys " << nPolys);
	    for( int i = 0; i < nPolys; i++){

			//DEBUGS("offsetIndex " << offsetIndex);

				// add offset to current index
				faceVertexOffsets[i] = offsetIndex;

			// get face vertices
			MIntArray faceVertices;
	        meshFn.getPolygonVertices( i, faceVertices );

	        for( unsigned int n = 0; n < faceVertices.length(); n++ ){
				allFaceVertices.append(faceVertices[n]);
				offsetIndex += 1;
	        }
	    }

		/* currently gives an offset buffer with 0 as first value - this is redundant,
		but allows direct indexing into main values, and I think it's convention
		*/
		DEBUGS("meshToBuffers face buffer done")

	    MFnIntArrayData faceData;
	    MObject faceObj = faceData.create( allFaceVertices );
        MDataHandle faceDH = data.outputValue( aFaceConnects );
        faceDH.setMObject( faceObj );

		MObject faceOffsetObj = faceData.create(faceVertexOffsets);
		MDataHandle faceOffsetDH = data.outputValue(aFaceOffsets);
		faceOffsetDH.setMObject(faceOffsetObj);

		// find point connections
		std::vector<int> faceVector = MIntArrayToVector(allFaceVertices);
		std::vector<int> faceOffsetVector = MIntArrayToVector(faceVertexOffsets);
		DEBUGS("faceVector" );
		DEBUGVI(faceVector);
		DEBUGS("faceOffsets");
		DEBUGVI(faceOffsetVector);


		//tie(pointConnects, pointOffsets) = ed::pointBufferFromFaceBuffer(faceVector, faceOffsetVector);
		OffsetBuffer<int> result = ed::pointBufferFromFaceVectors(faceVector, faceOffsetVector);
		std::vector<int> pointConnects = result.values, pointOffsets = result.offsets;

		DEBUGS("point buffer");
		DEBUGVI(pointConnects);
		DEBUGVI(pointOffsets);

		MIntArray pointConnectsArray = vectorToMIntArray(pointConnects);
		MObject pointObj = faceData.create(pointConnectsArray);
		MDataHandle pointConnectsDH = data.outputValue(aPointConnects);
		pointConnectsDH.setMObject(pointObj);

		MIntArray pointOffsetArray = vectorToMIntArray(pointOffsets);
		MObject pointOffsetObj = faceData.create(pointOffsetArray);
		MDataHandle pointOffsetDH = data.outputValue(aPointOffsets);
		pointOffsetDH.setMObject(pointOffsetObj);

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
