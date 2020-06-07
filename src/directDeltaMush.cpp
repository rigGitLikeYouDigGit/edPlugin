

/*

	no copyright or breach of contract intended,
	it's just much more efficient to test this when I already have the environment set up
	will delete once project is concluded

*/

#include "directDeltaMush.h"

using namespace std;
using namespace ed;

MTypeId DirectDeltaMush::kNODE_ID(0x00122C1D);
MString DirectDeltaMush::kNODE_NAME( "directDeltaMush" );

MObject DirectDeltaMush::aBind;

MObject DirectDeltaMush::aVertexWeightIndices;
MObject DirectDeltaMush::aVertexWeightValues;
MObject DirectDeltaMush::aVertexWeightOffsets;


/*
objects inherited from deformer, skincluster etc

MObject input - input attribute, array
MObject inputGeom - input geometry attribute
MObject outputGeom - output geometry attribute
MObject envelope - envelope attribute

MObject weightList
MObject weights
MObject matrix - input matrix array
MObject bindPreMatrix - input prematrix array

ALSO INHERIT ALL ATTRIBUTES FROM NORMAL SKINCLUSTER enjoy that node bloat lol

*/



MStatus DirectDeltaMush::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;

	aBind = makeBindAttr();
	addAttribute(aBind);

	// vertex weight buffers
	// main skin matrix weights
	aVertexWeightOffsets = tFn.create("vertexWeightOffsets", "vertexWeightOffsets", MFnData::kIntArray);
	addAttribute(aVertexWeightOffsets);

	aVertexWeightIndices = tFn.create("vertexWeightIndices", "vertexWeightIndices", MFnData::kIntArray);
	addAttribute(aVertexWeightIndices);

	aVertexWeightValues = tFn.create("vertexWeightValues", "vertexWeightValues", MFnData::kIntArray);
	addAttribute(aVertexWeightValues);

	// attribute affects
	// crude for now
	vector<MObject> drivers = {
		aVertexWeightIndices, aVertexWeightOffsets, aVertexWeightValues,
		aBind,
		matrix, bindPreMatrix, weights, weightList,
		inputGeom, envelope
	};

	vector<MObject> driven = {
		outputGeom
	};

	setAttributesAffect<DirectDeltaMush>(drivers, driven);

    return MStatus::kSuccess;
}


//MStatus DirectDeltaMush::deform(
//	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
//	            unsigned int MIndex) {
//    return MS::kSuccess;
//}

OffsetBuffer<int> faceBufferFromMfnMesh(MFnMesh& mfn) {
	// construct face buffer from mfn info
	DEBUGS("ddm faceBufferFromMfnMesh")
	int nPolys = mfn.numPolygons();
	vector<int> facePointConnects;
	vector<int> facePointOffsets(nPolys, -1);

	// vertex buffer from mfn
	MIntArray vertexCount; // number of vertices in face - NOT global offsets
	MIntArray vertexList; // list of vertices in each face
	mfn.getVertices(vertexCount, vertexList);
	vector<int> testv = MIntArrayToVector(vertexCount);
	DEBUGS("vertexCount");
	DEBUGVI(testv);

	int offsetIndex = 0;
	for (unsigned int i = 0; i < vertexCount.length(); i++) {

		// add offset to current index
		facePointOffsets[i] = offsetIndex;
		int nFacePoints = vertexCount[i];

		//// get face vertices
		//MIntArray faceVertices;
		//mfn.getPolygonVertices(i, faceVertices);

		//for (unsigned int n = 0; n < faceVertices.length(); n++) {
		//	//allFaceVertices.append(faceVertices[n]);
		//	facePointConnects.push_back(faceVertices[n]);
		//	offsetIndex += 1;
		//}
		for (int n = 0; n < nFacePoints; n++) {
			facePointConnects.push_back(vertexList[offsetIndex]);
			offsetIndex++;
		}
	}
	return OffsetBuffer<int>(facePointConnects, facePointOffsets);
}

void HalfEdgeMeshFromMObject(HalfEdgeMesh& hedgeMesh, MObject meshObj, int build) {
	// updates target mesh struct from mesh MObject
	// if build, will rebuild topology buffers
	// if not, will only copy point positions
	DEBUGS("ddm hedgeMesh from mobject")
	MStatus s = MS::kSuccess;
	MFnMesh meshFn(meshObj);

	int nPoints = meshFn.numVertices();
	int nPolys = meshFn.numPolygons();
	hedgeMesh.nPoints = nPoints;
	hedgeMesh.nFaces = nPolys;

	if (build > 0) {

		DEBUGS("building hedgemesh");
		OffsetBuffer<int> faceBuffer = faceBufferFromMfnMesh(meshFn);
		//OffsetBuffer<int> faceBuffer(allFaceVertices, faceVertexOffsets);
		DEBUGS("faceBuffer built");
		DEBUGVI(faceBuffer.offsets);
		DEBUGVI(faceBuffer.values);
		OffsetBuffer<int> pointBuffer = pointBufferFromFaceBuffer(
			faceBuffer);
		
		hedgeMesh.build(
			pointBuffer.values, pointBuffer.offsets,
			faceBuffer.values, faceBuffer.offsets
		);
	}

	// set mesh point positions
	DEBUGS("set positions")

	const float * rawPositions = meshFn.getRawPoints(&s);	
	float test = rawPositions[7];
	vector<double> posVector(nPoints * 3, 0.0);
	for (int i = 0; i < nPoints; i++) {
		posVector[i*3] = rawPositions[i*3];
		posVector[i*3 + 1] = rawPositions[i*3 + 1];
		posVector[i*3 + 2] = rawPositions[i*3 + 2];

	}


	//DEBUGVI(posVector);
	hedgeMesh.setPositions(posVector);
}

/* traverse basic skincluster weight system to build more efficient weight
buffers, then transfer those into array attributes
refresher:
	weightList[ vtxId ].weights[ influenceIndex ] = influence weight
*/
void extractSkinWeights(MArrayDataHandle& weightRoot, SkinData& skinInfo) {
	// extracts skin data to buffers
	//SkinData result;
	DEBUGS("extractSkinWeights");
	vector<int> vertexOffsets(weightRoot.elementCount());
	vector<int> influenceIndices;
	vector<double> influenceWeights;
	// eps value to discount negligible influences
	static double e = 0.0001;

	//DEBUGS("elemcount");
	//DEBUGS(weightRoot.elementCount());

	int offsetIndex = 0;
	for (unsigned int i = 0; i < weightRoot.elementCount(); i++) {
		vertexOffsets[i] = offsetIndex;
		jumpToElement(weightRoot, i);
		MArrayDataHandle vtxEntry = weightRoot.inputValue().child(
			DirectDeltaMush::weights);
		unsigned int nWeights = vtxEntry.elementCount();
		//DEBUGS("nWeights");
		//DEBUGS(nWeights);
		if (nWeights == 0) {
			influenceIndices.push_back(0);
			influenceWeights.push_back(0.0);
			offsetIndex++;
			continue;
		}
		for (unsigned int n = 0; n < nWeights; n++) {
			jumpToElement(vtxEntry, n);
			influenceIndices.push_back(vtxEntry.elementIndex());
			influenceWeights.push_back(vtxEntry.inputValue().asDouble());
			offsetIndex++;
		}
	}
	DEBUGVI(influenceIndices)
	DEBUGVI(vertexOffsets)
	skinInfo.vertexOffsets = vertexOffsets;
	skinInfo.influenceIndices = influenceIndices;
	skinInfo.influenceWeights = influenceWeights;
	DEBUGS("extractSkinIndices");
	DEBUGVI(skinInfo.influenceIndices);

}


void DirectDeltaMush::runBind(MDataBlock& data, const MObject& meshObj) {
	// precompute node
	// first rebuild topological struct
	DEBUGS("ddm runBind");
	HalfEdgeMeshFromMObject(*hedgeMesh, meshObj, 1);

	// still having serious trouble
	
	//DEBUGS("skinIndices");
	//DEBUGVI(skinInfo.influenceIndices)
}


MMatrixArray extractMMatrixArray(MArrayDataHandle& matArray) {
	DEBUGS("ddm extract mmatrices");
		// extract weight matrices from array
	// simple, right?

		MMatrixArray result(matArray.elementCount());
		for (unsigned int i = 0; i < matArray.elementCount(); i++) {
			result[i] = MFnMatrixData(matArray.inputValue().data()).matrix();
			// guess again
			matArray.next();
		}
		return result;
	}

void DirectDeltaMush::setOutputGeo(MDataBlock& data, const MObject& meshGeo) {
	// sets output plug to target mesh object
	// we deform only one piece of geometry
	DEBUGS("setOutputGeo")
	MArrayDataHandle outputArray = data.outputArrayValue(outputGeom);
	jumpToElement(outputArray, 0);
	MDataHandle outputGeoHandle = outputArray.outputValue();
	outputGeoHandle.setMObject(meshGeo);
}

void DirectDeltaMush::deformGeo(HalfEdgeMesh& mesh, 
	const DeformerParametres& params,
	vector<double> &outputPositions) {
	// kicks off full deformation process
	DEBUGS("deformGeo");
	DEBUGS("nPoints");
	DEBUGS(mesh.nPoints);

	for (int i = 0; i < mesh.nPoints; i++) {
		deformPoint(mesh, params, outputPositions, i);
	}
}

void DirectDeltaMush::deformPoint(
	HalfEdgeMesh & mesh, 
	const DeformerParametres &params, 
	vector<double> &outPositions, int index) {
	// deform single vertex

	SmallList<double> baseList = mesh.pointPositions.entry(index);


	MVector baseVector(
		baseList[0], baseList[1], baseList[2]);

	// get skin indices and weights
	SmallList<int> weightIndices = entryFromBuffer(
		params.skinData.influenceIndices, params.skinData.vertexOffsets, index);
	//DEBUGVI(weightIndices);
	SmallList<double> weightValues = entryFromBuffer(
		params.skinData.influenceWeights, params.skinData.vertexOffsets, index);
	//DEBUGVI(weightValues);

	// output position
	MVector result(baseVector);
	//DEBUGS("baseVector");

	MVector tfPos(baseVector);
	//MMatrix tfMat;
	//MMatrix baseMat;
	MMatrix diffMat;
	
	//DEBUGS("iterate weight values")
	for (int i = 0; i < weightValues.size(); i++) {
		//DEBUGS(i);
		int infIndex = weightIndices[i];
		//DEBUGS(infIndex);
		MMatrix baseMat = (params.refMats)[ infIndex ];

		MMatrix tfMat = params.tfMats[ infIndex ];

		double weightVal = static_cast<double>(weightValues[i]);
		//DEBUGS(weightVal);
		diffMat = baseMat.inverse() * tfMat;
		tfPos = tfPos + diffMat * baseVector * weightVal;

	}

	// blend output by envelope
	result = baseVector * (1 - params.envelope) + tfPos * params.envelope;

	outPositions[index * 3] = result.x;
	outPositions[index * 3 + 1] = result.y;
	outPositions[index * 3 + 2] = result.z;

	//DEBUGS("outPositions");
	//DEBUGVI(outPositions);

}

void initialiseWeightArray(MArrayDataHandle& weightRoot, int nElements) {
	// make sure we have corresponding array values
	DEBUGS("initWeightArray")
	for (int i = 0; i < nElements; i++) {
		jumpToElement(weightRoot, i);
		MArrayDataHandle childDH = weightRoot.inputArrayValue();
		if (childDH.elementCount() == 0) {
			/*childDH.builder().addElement(0);
			jumpToElement(childDH, 0);*/
			//childDH.inputValue().setFloat(0.0);
		}
		//DEBUGS(childDH.elementCount());
	}
}

MStatus DirectDeltaMush::compute(
	const MPlug& plug, MDataBlock& data) {
	DEBUGS("ddm compute");
	if (plug != outputGeom) {
		return MS::kSuccess;	}
	MStatus m;
	MArrayDataHandle inputArray = data.outputArrayValue(input);
	jumpToElement(inputArray, 0);
	MDataHandle geoHandle = (inputArray.inputValue().child(inputGeom));
	MObject meshObj = geoHandle.asMesh();
	MFnMesh meshFn(meshObj);

	// ensure weight array is correct to avoid crashing
	int weightCount = data.outputArrayValue(weightList).elementCount();
	DEBUGS("weightCount");
	DEBUGS(weightCount);
	//data.inputArrayValue(weights);

	initialiseWeightArray(data.inputArrayValue(weightList), meshFn.numVertices());

	SkinData &skinInfo = deformParams->skinData;
	extractSkinWeights(data.inputArrayValue(weightList),
		skinInfo);


	//if (weightCount != meshFn.numVertices()) {
	//	initialiseWeightArray(data.inputArrayValue(weightList), meshFn.numVertices());
	//}

	int bind = data.inputValue(aBind).asInt();
	if (bind == 1 || bind == 3) { // bind or live

		runBind(data, meshObj);

		if (bind == 1) { // set bind attr to bound
			data.inputValue(aBind).setInt(2);
		}
	}


	// check envelope
	float envelopeValue = data.inputValue(envelope).asFloat();
	if (envelopeValue < 0.001 || weightCount==0) {

		// set output data directly
		setOutputGeo(data, meshObj);
		data.setClean(plug);
		return MS::kSuccess;
	}

	// extract joint matrix arrays
	MMatrixArray transformMatrices = extractMMatrixArray(
		data.inputArrayValue(matrix));
	MMatrixArray bindPreMatrices = extractMMatrixArray(
			data.inputArrayValue(bindPreMatrix));

	deformParams->tfMats = transformMatrices;
	deformParams->refMats = bindPreMatrices;
	deformParams->envelope = data.inputValue(envelope).asFloat();

	// deformation call - optimise this
	vector<double> outPositions(meshFn.numVertices()*3, 0.0);
	deformGeo(*hedgeMesh, *deformParams, outPositions);

	DEBUGS("output")
	DEBUGVI(outPositions);

	// convert to point array
	MPointArray outputPoints(meshFn.numVertices());
	for (int i = 0; i < meshFn.numVertices(); i++) {
		outputPoints[i] = MPoint(
			outPositions[i * 3],
			outPositions[i * 3 + 1],
			outPositions[i * 3 + 2],
			0.5);
	}

	meshFn.setPoints(outputPoints, MSpace::kObject);

	// set output geometry plug
	setOutputGeo(data, meshObj);
	data.setClean(plug);

	return MS::kSuccess;
}




void* DirectDeltaMush::creator(){

     DirectDeltaMush *node = new DirectDeltaMush;
	 node->hedgeMesh = new HalfEdgeMesh;
	 node->deformParams = new DeformerParametres;

	 return node;

}

DirectDeltaMush::DirectDeltaMush() {};
DirectDeltaMush::~DirectDeltaMush() {};
