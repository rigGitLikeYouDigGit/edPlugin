

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

OffsetBuffer<int> faceBufferFromMfnMesh(MFnMesh mfn) {
	// construct face buffer from mfn info
	int nPolys = mfn.numPolygons();
	vector<int> facePointConnects;
	vector<int> facePointOffsets(nPolys, -1);
	int offsetIndex = 0;
	for (int i = 0; i < nPolys; i++) {

		// add offset to current index
		facePointOffsets[i] = offsetIndex;

		// get face vertices
		MIntArray faceVertices;
		mfn.getPolygonVertices(i, faceVertices);

		for (unsigned int n = 0; n < faceVertices.length(); n++) {
			//allFaceVertices.append(faceVertices[n]);
			facePointConnects.push_back(faceVertices[n]);
			offsetIndex += 1;
		}
	}
	return OffsetBuffer<int>(facePointConnects, facePointOffsets);
}

void HalfEdgeMeshFromMObject(HalfEdgeMesh& hedgeMesh, MObject meshObj, int build) {
	// updates target mesh struct from mesh MObject
	// if build, will rebuild topology buffers
	// if not, will only copy point positions
	//MFnMesh meshFn = MFnMesh(meshObj);
	MStatus s = MS::kSuccess;
	MFnMesh meshFn(meshObj);

	int nPoints = meshFn.numVertices();
	int nPolys = meshFn.numPolygons();

	if (build > 0) {
		// face buffers
		//MIntArray allFaceVertices;
		vector<int> allFaceVertices;
		//MIntArray faceVertexOffsets = MIntArray(nPolys); // offsets into allFaceVertices
		vector<int> faceVertexOffsets(nPolys, -1);
		int offsetIndex = 0;
		for (int i = 0; i < nPolys; i++) {

				// add offset to current index
			faceVertexOffsets[i] = offsetIndex;

			// get face vertices
			MIntArray faceVertices;
			meshFn.getPolygonVertices(i, faceVertices);

			for (unsigned int n = 0; n < faceVertices.length(); n++) {
				//allFaceVertices.append(faceVertices[n]);
				allFaceVertices.push_back(n);
				offsetIndex += 1;
			}
		}
		OffsetBuffer<int> faceBuffer(allFaceVertices, faceVertexOffsets);
		OffsetBuffer<int> pointBuffer = pointBufferFromFaceBuffer(
			faceBuffer);
		hedgeMesh.build(
			pointBuffer.values, pointBuffer.offsets,
			faceBuffer.values, faceBuffer.offsets
		);
	}

	// set mesh point positions
	const float * rawPositions = meshFn.getRawPoints(&s);
	vector<float> posVector(nPoints * 3, 0.0);
	for (int i = 0; i < nPoints; i++) {
		posVector[i] = rawPositions[i];
	}
	hedgeMesh.setPositions(posVector);
}


void DirectDeltaMush::runBind(MDataBlock& data, const MObject& meshObj) {
	// precompute node
	// first rebuild topological struct
	HalfEdgeMeshFromMObject(*hedgeMesh, meshObj, 1);

	/* traverse basic skincluster weight system to build more efficient weight
buffers, then transfer those into array attributes
refresher:
	weightList[ vtxId ].weights[ influenceIndex ] = influence weight
*/
	vector<int> vertexWeightIndices;
	vector<float> vertexWeightValues;
	vector<int> vertexWeightOffsets(hedgeMesh->nPoints, -1);

	MArrayDataHandle weightHdl = data.outputArrayValue(weightList);
	int offsetIndex = 0;
	for (int i = 0; i < hedgeMesh->nPoints; i++) {
		vertexWeightOffsets[i] = offsetIndex;
		jumpToElement(weightHdl, i);
		MArrayDataHandle vtxEntry = weightHdl.outputArrayValue();
		unsigned int nWeights = vtxEntry.elementCount();
		for (unsigned int n = 0; n < nWeights; n++) {
			vertexWeightIndices.push_back(vtxEntry.elementIndex());
			vertexWeightValues.push_back(vtxEntry.outputValue().asFloat());
			offsetIndex++;
		}

	}


}


MMatrixArray extractMMatrixArray( const MArrayDataHandle& matArray){
	// extract weight matrices from array
	MMatrixArray result( matArray.elementCount());
	for(int i=0; i < matArray.elementCount(); i++){
		jumpToElement(matArray, i);
		result[i] = matArray.outputValue().asMatrix();
	}
	return result;
}


MStatus DirectDeltaMush::compute(
	const MPlug& plug, MDataBlock& data) {

	if (plug != outputGeom) {
		return MS::kSuccess;	}
	MStatus m;
	MArrayDataHandle inputArray = data.outputArrayValue(input);
	jumpToElement(inputArray, 0);
	MDataHandle geoHandle = (inputArray.inputValue().child(inputGeom));
	MObject meshObj = geoHandle.asMesh();

	int bind = data.inputValue(aBind).asInt();
	if (bind == 1 || bind == 3) { // bind or live

		runBind(data, meshObj);

		if (bind == 1) { // set bind attr to bound
			data.inputValue(aBind).setInt(2);
		}
	}

	// check envelope
	float envelopeValue = data.inputValue(envelope).asFloat();
	if (envelopeValue < 0.001) {

		// set output data directly
		setOutputGeo(data, meshObj);
		data.setClean(plug);
		return MS::kSuccess;
	}

	// extract joint matrix arrays
	MMatrixArray transformMatrices = extractMMatrixArray(
		data.outputArrayValue(matrix));
	MMatrixArray bindPreMatrices = extractMMatrixArray(
			data.outputArrayValue(bindPreMatrix));

	// set output geometry plug
	setOutputGeo(data, meshObj);
	data.setClean(plug);

	return MS::kSuccess;
}

void DirectDeltaMush::setOutputGeo(MDataBlock& data, const MObject& meshGeo) {
	// sets output plug to target mesh object
	// we deform only one piece of geometry
	MArrayDataHandle outputArray = data.outputArrayValue(outputGeom);
	jumpToElement(outputArray, 0);
	MDataHandle outputGeoHandle = outputArray.outputValue();
	outputGeoHandle.setMObject(meshGeo);
}

/*
use this opportunity to test a more efficient way of working with per-vertex weights

buffers:

vertex index : influence1 weight, influence4 weight, influence5 weight    - influence weights
vertex index : 1, 4, 5   - influence indices
vertex index : 3  - offset buffer

*/
struct SkinData{
	// struct to store and query skincluster data
	vector<int> vertexOffsets;
	vector<int> influenceIndices;
	vector<float> influenceWeights;
};

void* DirectDeltaMush::creator(){

     DirectDeltaMush *node = new DirectDeltaMush;
	 node->hedgeMesh = new HalfEdgeMesh;

	 return node;

}

DirectDeltaMush::DirectDeltaMush() {};
DirectDeltaMush::~DirectDeltaMush() {};
