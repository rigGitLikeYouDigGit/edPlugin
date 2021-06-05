

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
	MObject MeshToBuffers::aPointVectorsX;
	MObject MeshToBuffers::aPointVectorsY;
	MObject MeshToBuffers::aPointVectorsZ;
MObject MeshToBuffers::aFaceVectors;
	MObject MeshToBuffers::aFaceVectorsX;
	MObject MeshToBuffers::aFaceVectorsY;
	MObject MeshToBuffers::aFaceVectorsZ;
MObject MeshToBuffers::aPointNormalVectors;
	MObject MeshToBuffers::aPointNormalVectorsX;
	MObject MeshToBuffers::aPointNormalVectorsY;
	MObject MeshToBuffers::aPointNormalVectorsZ;
MObject MeshToBuffers::aFaceNormalVectors;
	MObject MeshToBuffers::aFaceNormalVectorsX;
	MObject MeshToBuffers::aFaceNormalVectorsY;
	MObject MeshToBuffers::aFaceNormalVectorsZ;

MObject MeshToBuffers::aHedgeMesh;

MStatus MeshToBuffers::initialize()
{
    // initialise attributes
    MFnTypedAttribute tFn;
    MFnNumericAttribute nFn;
	//MFnEnumAttribute fn;
	MFnUnitAttribute uFn;
	MFnCompoundAttribute cFn;


    // main inputs
    aInMesh = tFn.create("inMesh", "inMesh", MFnData::kMesh);
    tFn.setReadable(false);
    tFn.setWritable(true);

    // outputs
	aFaceConnects = tFn.create("faceConnects", "faceConnects", MFnData::kIntArray);

    aFaceOffsets = tFn.create("faceOffsets", "faceOffsets", MFnData::kIntArray);

    aPointPositions = tFn.create("pointPositionsRaw", "pointPositionsRaw", MFnData::kFloatArray);

	aPointConnects = tFn.create("pointConnects", "pointConnects", MFnData::kIntArray);

	aPointOffsets = tFn.create("pointOffsets", "pointOffsets", MFnData::kIntArray);

	aNormals = tFn.create("normalsRaw", "normalsRaw", MFnData::kFloatArray);

	// uv buffers are arrays, one for each uv set of mesh
	aUvCoords = tFn.create("uvCoords", "uvCoords", MFnData::kFloatArray);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);
	//addAttribute(aUvCoords);


    // bind
	aBind = makeEnumAttr<BindState>("bind");

	// vector values

	aPointVectors = makeXYZVectorAttr("point", aPointVectors,
		aPointVectorsX, aPointVectorsY, aPointVectorsZ,
		true);


	aFaceVectors = makeXYZVectorAttr("face", aFaceVectors,
		aFaceVectorsX, aFaceVectorsY, aFaceVectorsZ, true);

	aPointNormalVectors = makeXYZVectorAttr("pointNormal", aPointNormalVectors,
		aPointNormalVectorsX, aPointNormalVectorsY, aPointNormalVectorsZ, true);

	aFaceNormalVectors = makeXYZVectorAttr("faceNormal", aFaceNormalVectors,
		aFaceNormalVectorsX, aFaceNormalVectorsY, aFaceNormalVectorsZ, true);

	vector<MObject> drivers = {
		aInMesh, aBind
	};
	vector<MObject> driven = {
		aFaceConnects, aFaceOffsets, aPointPositions, aPointConnects,
		aPointOffsets, aFaceCentres, aNormals, aUvCoords,
		aPointVectors, aFaceVectors,
		aPointNormalVectors, aFaceNormalVectors
	};



	addAttributes<MeshToBuffers>(drivers);
	addAttributes<MeshToBuffers>(driven);
	setAttributesAffect<MeshToBuffers>(drivers, driven);
	


    return MStatus::kSuccess;
}


MStatus MeshToBuffers::bind(MDataBlock& data, MFnMesh& meshFn, MStatus& s) {
	// bind topology information
			// do binding with topology buffers
	//MStatus s = MStatus::kSuccess;
		// face buffers
	//DEBUGS("MTB bind")
	int nPolys = meshFn.numPolygons();
	int nPoints = meshFn.numVertices();
	MIntArray allFaceVertices;
	MIntArray faceVertexOffsets = MIntArray(nPolys); // offsets into allFaceVertices
	int offsetIndex = 0;
	//DEBUGS("nPolys " << nPolys);
	for (int i = 0; i < nPolys; i++) {

		//DEBUGS("offsetIndex " << offsetIndex);

			// add offset to current index
		faceVertexOffsets[i] = offsetIndex;

		// get face vertices
		MIntArray faceVertices;
		meshFn.getPolygonVertices(i, faceVertices);

		for (unsigned int n = 0; n < faceVertices.length(); n++) {
			allFaceVertices.append(faceVertices[n]);
			offsetIndex += 1;
		}
	}

	/* currently gives an offset buffer with 0 as first value - this is redundant,
	but allows direct indexing into main values, and I think it's convention
	*/
	//DEBUGS("meshToBuffers face buffer done")

		MFnIntArrayData faceData;
	MObject faceObj = faceData.create(allFaceVertices);
	MDataHandle faceDH = data.outputValue(aFaceConnects);
	faceDH.setMObject(faceObj);

	MObject faceOffsetObj = faceData.create(faceVertexOffsets);
	MDataHandle faceOffsetDH = data.outputValue(aFaceOffsets);
	faceOffsetDH.setMObject(faceOffsetObj);

	// find point connections
	std::vector<int> faceVector = MayaContainerToVector<int, MIntArray>(allFaceVertices);
	
;	std::vector<int> faceOffsetVector = MIntArrayToVector(faceVertexOffsets);

	OffsetBuffer<int> result = ed::pointBufferFromFaceVectors(faceVector, faceOffsetVector);

	std::vector<int> pointConnects = result.values, pointOffsets = result.offsets;

	MIntArray pointConnectsArray(pointConnects.data(), static_cast<int>(pointConnects.size()));
	MObject pointObj = faceData.create(pointConnectsArray);
	MDataHandle pointConnectsDH = data.outputValue(aPointConnects);
	pointConnectsDH.setMObject(pointObj);

	//MIntArray pointOffsetArray = vectorToMIntArray(pointOffsets);
	MIntArray pointOffsetArray(pointOffsets.data(), static_cast<int>(pointOffsets.size()));
	MObject pointOffsetObj = faceData.create(pointOffsetArray);
	MDataHandle pointOffsetDH = data.outputValue(aPointOffsets);
	pointOffsetDH.setMObject(pointOffsetObj);


	// set length of varying plugs
	for (int i = 0; i < nPoints; i++) {
		for (MObject obj : {aPointVectors, aPointNormalVectors}) {
			jumpToElement(data.outputArrayValue(obj), i);
		}
	}
	for (int i = 0; i < nPolys; i++) {
		for (MObject obj : {aFaceVectors, aFaceNormalVectors}) {
			jumpToElement(data.outputArrayValue(obj), i);
		}
	}

	return s;
}


MStatus MeshToBuffers::compute(
				const MPlug& plug, MDataBlock& data) {
    // going with floats for now, can easily switch to doubles if needed
	// initialise MFnMesh
	MStatus s = MS::kSuccess;

	//DEBUGS("MTB compute");

	// if no mesh is connected, return immediately
	if (!isConnected) {
		DEBUGS("MTB mesh not connected, passing")
		data.setClean(plug);
		return MS::kSuccess;
	}

	MObject meshObj = data.inputValue( aInMesh ).asMesh() ;
	MFnMesh meshFn(meshObj);
	//meshFn.setObject(meshObj);
	int nPoints = meshFn.numVertices();
	int nPolys = meshFn.numPolygons();

	// check if input mesh connection is empty


	// positions first
	const float * rawPoints = meshFn.getRawPoints(&s);
	MCHECK(s, "Error in float point extraction");

	// getRawDoublePoints flat out does not work
	/*const double * rawDoublePoints = meshFn.getRawDoublePoints(&s);
	MCHECK(s, "Error in double point extraction");*/

	MFloatArray positions = MFloatArray(rawPoints, nPoints * 3);
	
	
	///MVectorArray positionsVec = MVectorArrayFromPtr(rawDoublePoints, nPoints);
	MVectorArray positionsVec = MVectorArrayFromPtr(rawPoints, nPoints);
	MFnFloatArrayData floatData;
	MObject positionsData = floatData.create( positions );

	MFnVectorArrayData vecData;
	MObject positionsVecData = vecData.create(positionsVec);

	// check bind
	int bindVal = data.inputValue(aBind).asInt();

	// check bind status
	switch (bindVal)
	{
	case BindState::off: // do literally nothing
		data.setClean(plug);
		return MS::kSuccess;
		break;
	case BindState::bind: // bind and set to bound, then continue
		bind(data, meshFn, s);
		data.outputValue(aBind).setInt(BindState::bound);
		break;
	case BindState::bound: // continue
		break;
	case BindState::live: // bind and continue, leave on live
		bind(data, meshFn, s);
		break;
	}

	// DEBUGS("After MTB bind")
	// set outputs
	MDataHandle positionsDH = data.outputValue( aPointPositions, &s );
	MCHECK(s, "Error extracting float positions output handle");
	positionsDH.setMObject( positionsData);

	// vector outputs
	MArrayDataHandle positionsVecDH = data.outputArrayValue(aPointVectors, &s);
	MCHECK(s, "Error extracting vector positions output handle");

	for (int i = 0; i < nPoints; i++) {
		jumpToElement(positionsVecDH, i);
		//positionsVecDH.outputValue().set3Float(
		positionsVecDH.outputValue().set3Double(
			rawPoints[i * 3], rawPoints[i * 3 + 1], rawPoints[i * 3 + 2]
		);
	}

	// optional outputs
	// this could probably be more efficient by using the existing raw buffers
	// but it's not a huge issue

	

	if (connectedOutputs.count(&aFaceVectors)) {
		DEBUGS("Face vectors connected, setting outputs");
		for (int i = 0; i < nPolys; i++) {
			
		}
		// set clean
		for (MObject* obj : (attrParentToChild[&aFaceNormalVectors])) {
			data.setClean(*obj);
		}
	}

	if (connectedOutputs.count(&aPointNormalVectors)) {
		MArrayDataHandle normalsVecDH = data.outputArrayValue(aPointNormalVectors, &s);
		MCHECK(s, "Error extracting point normal vector output handle");
		for (int i = 0; i < nPoints; i++) {
			jumpToElement(normalsVecDH, i);
			MVector normal;
			meshFn.getVertexNormal(i, false, // angle weighted
				normal, MSpace::kObject);
			normalsVecDH.outputValue().setMVector(normal);
		}
		// set clean
		for (MObject* obj : (attrParentToChild[&aPointNormalVectors])) {
			data.setClean(*obj);
		}

	}

	if (connectedOutputs.count(&aFaceNormalVectors)) {
		MArrayDataHandle faceNormalsVecDH = data.outputArrayValue(aPointNormalVectors, &s);
		MCHECK(s, "Error extracting face normal vector output handle");
		for (int i = 0; i < nPolys; i++) {
			jumpToElement(faceNormalsVecDH, i);
			MVector normal;
			meshFn.getPolygonNormal(i, // angle weighted
				normal, MSpace::kObject);
			faceNormalsVecDH.outputValue().setMVector(normal);
		}
		// set clean
		for (MObject* obj : (attrParentToChild[&aFaceNormalVectors])) {
			data.setClean(*obj);
		}
	}

	data.setClean(plug);

    return MS::kSuccess;
}

void MeshToBuffers::syncConnections() {
	//DEBUGS("Sync connections")
	attrParentToChild.clear();
	attrParentToChild.insert(
		{ &aPointVectors, { &aPointVectors, &aPointVectorsX, &aPointVectorsY, &aPointVectorsZ } });
	attrParentToChild.insert(
		{ &aFaceVectors, { &aFaceVectors, &aFaceVectorsX, &aFaceVectorsY, &aFaceVectorsZ } });
	attrParentToChild.insert(
		{ &aPointNormalVectors, { &aPointNormalVectors, &aPointNormalVectorsX, &aPointNormalVectorsY, &aPointNormalVectorsZ } });
	attrParentToChild.insert(
		{ &aFaceNormalVectors, { &aFaceNormalVectors, &aFaceNormalVectorsX, &aFaceNormalVectorsY, &aFaceNormalVectorsZ } });

	MFnDependencyNode nFn(thisMObject());
	//DEBUGS("node name " << nFn.name());

	for (auto i : attrParentToChild) {
		MPlug parentPlug = nFn.findPlug(*(i.first), true);
		//DEBUGS("testing parentPlug" << parentPlug.info());
		std::vector<MPlug> childPlugs = getAllChildPlugs(parentPlug);

		//for (MObject* childObj : i.second) {
			//MPlug plug = nFn.findPlug(*childObj, true);
		for (MPlug plug: childPlugs){
			//DEBUGS("check plug " << plug.name());
			//DEBUGS(plug.isNull());
			//DEBUGS(plug.info());
			//
			if (plug.isConnected()) {
				//DEBUGS("is connected")
				connectedOutputs.insert(i.first);
			}
		}
	}

}


/// boiler plate

void MeshToBuffers::postConstructor() {
	this->setExistWithoutInConnections(true);
	this->setExistWithoutOutConnections(true);
}
MStatus MeshToBuffers::connectionMade(
	const MPlug& plug, const MPlug& otherPlug, bool asSrc) {
	// asSrc - is THIS NODE'S PLUG the source of a connection

	if (plug.attribute() == aInMesh) {
		isConnected = true;
	}
	
	auto result = MPxNode::connectionMade(plug, otherPlug, asSrc);
	syncConnections();
	return result;

}

MStatus MeshToBuffers::connectionBroken(
	const MPlug& plug, const MPlug& otherPlug, bool asSrc) {


	if (plug.attribute() == aInMesh) {
		isConnected = false;
	}

	auto result = MPxNode::connectionBroken(plug, otherPlug, asSrc);
	syncConnections();
	return result;
}


void* MeshToBuffers::creator(){

    return new MeshToBuffers;

}


MeshToBuffers::MeshToBuffers() {};
MeshToBuffers::~MeshToBuffers() {};
