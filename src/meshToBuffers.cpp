

/*

	converts maya mesh to raw float and int buffers of position and topo data

*/

#include "meshToBuffers.h"


using namespace ed;

MTypeId MeshToBuffers::kNODE_ID(0x00122C08);
MString MeshToBuffers::kNODE_NAME( "meshToBuffers" );

MObject MeshToBuffers::aTest;
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
	aFaceConnects = tAttr.create("faceConnects", "faceConnects", MFnData::kIntArray);
	addAttribute(aFaceConnects);

    aFaceOffsets = tAttr.create("faceOffsets", "faceOffsets", MFnData::kIntArray);
    addAttribute( aFaceOffsets );

	//aFaceCentres = tAttr.create("faceCentres", "faceCentres", MFnData::kFloatArray);
	//addAttribute(aFaceCentres ); // more efficient to find online


    aPointPositions = tAttr.create("pointPositions", "pointPositions", MFnData::kFloatArray);
    addAttribute( aPointPositions );

	aPointConnects = tAttr.create("pointConnects", "pointConnects", MFnData::kIntArray);
	addAttribute(aPointConnects);

	aPointOffsets = tAttr.create("pointOffsets", "pointOffsets", MFnData::kIntArray);
	addAttribute(aPointOffsets);

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
	attributeAffects(aBind, aFaceOffsets);

	attributeAffects(aInMesh, aPointPositions);
	attributeAffects(aInMesh, aPointConnects);
	attributeAffects(aInMesh, aFaceOffsets);

/*
	std::vector<MObject> topoAttrs = { aFaceOffsets, aPointConnects };
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
