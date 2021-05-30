

/*
	multiple softmod handles in one
*/

#include "multiMod.h"

using namespace ed;
using namespace std;

MTypeId EdBlendShape::kNODE_ID(0x00122C1D);
MString EdBlendShape::kNODE_NAME("multiMod");

//MObject EdBlendShape::aInMesh;

MObject EdBlendShape::aHandles;
MObject EdBlendShape::aMat;
MObject EdBlendShape::aCaptureMat;
MObject EdBlendShape::aWeight;
MObject EdBlendShape::aFalloffRamp;
MObject EdBlendShape::aFalloffRadius;

MObject EdBlendShape::aCombinationMode;
MObject EdBlendShape::aCombinationSmoothness;
MObject EdBlendShape::aWeights;

//MObject EdBlendShape::aOutMesh;



#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\

void EdBlendShape::postConstructor() {
	this->setExistWithoutInConnections(true);
	this->setExistWithoutOutConnections(true);
}

MStatus EdBlendShape::initialize()
{
	// initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;
	MFnCompoundAttribute cFn;
	MFnEnumAttribute eFn;
	MFnAttribute aFn;

	// input mesh to deform
	//aInMesh = tFn.create("inputMesh", "inputMesh", MFnMeshData::kMesh);
	//tFn.setReadable(false);

	// handle array
	aHandles = cFn.create("handles", "handles");
	//cFn.setReadable(false);

		// delta matrix for deformation
	aMat = tFn.create("matrix", "matrix", MFnMatrixData::kMatrix);
	cFn.addChild(aMat);

	// object-space matrix to capture points in spherical falloff
	aCaptureMat = tFn.create("captureMatrix", "captureMatrix", MFnMatrixData::kMatrix);
	cFn.addChild(aCaptureMat);

	// linear scaling of captureregion
	aFalloffRadius = tFn.create("falloffRadius", "falloffRadius", MFnMatrixData::kMatrix);
	cFn.addChild(aFalloffRadius);

	// falloff ramp
	aFalloffRamp = MRampAttribute::createCurveRamp("falloffRamp", "falloffRamp");
	cFn.addChild(aFalloffRamp);

	// contribution of this handle
	aWeight = nFn.create("weight", "weight", MFnNumericData::kFloat, 1.0);
	nFn.setMin(0.0);
	nFn.setSoftMax(1.0);
	cFn.addChild(aWeight);

	// combination modes
	aCombinationMode = eFn.create("operation", "operation", 0);
	eFn.addField("mean", CombinationModes::mean);
	eFn.addField("sum", CombinationModes::sum);
	eFn.addField("max", CombinationModes::max);
	eFn.addField("smoothMax", CombinationModes::smoothMax);
	eFn.setReadable(false);

	aWeights = tFn.create("weights", "weights", MFnNumericData::kFloatArray);


	//aOutMesh = tFn.create("outputMesh", "outputMesh", MFnMeshData::kMesh);
	//tFn.setWritable(false);


	vector<MObject> drivers = {
		//aInMesh,
		input, inputGeom, envelope,
		aHandles, aMat, aCaptureMat, aWeight, aFalloffRamp, aFalloffRadius,
		aCombinationMode
	};
	vector<MObject> driven = {
		//aOutMesh
		outputGeom
	};

	addAttributes<EdBlendShape>(drivers);
	addAttributes<EdBlendShape>(driven);

	setAttributesAffect<EdBlendShape>(drivers, driven);

	return MStatus::kSuccess;
}


MStatus EdBlendShape::compute(
	const MPlug& plug, MDataBlock& data) {

	MStatus s;
	float envelopeValue = data.inputValue(envelope).asFloat();
	if ((plug != outputGeom) | (envelopeValue < EPS)) {
		data.setClean(plug);
		return;
	}

	gatherParams(data);

	// set param function pointer
	int mode = data.inputValue(aCombinationMode).asInt();



	MArrayDataHandle inputArray = data.inputArrayValue(input);
	MArrayDataHandle outputArray = data.outputArrayValue(outputGeom);
	for (int i = 0; i < inputArray.elementCount(); i++) {
		s = jumpToElement(inputArray, i);
		MCHECK(s, "compute input jte failed");
		s = jumpToElement(outputArray, i);
		MCHECK(s, "compute output jte failed")

			// get input mesh data
			MObject meshObj = inputArray.inputValue().asMesh();
		MFnMesh meshFn(meshObj);

		// set length of mesh array
		if (hedgeMeshes.size() < (i - 1)) {
			hedgeMeshes.push_back(HalfEdgeMesh());
		}

		// refresh hedgemesh object
		HalfEdgeMesh hedgeMesh = hedgeMeshes[i];
		int build = HalfEdgeMeshNeedsRebuild(hedgeMesh, meshFn);
		HalfEdgeMeshFromMObject(hedgeMesh, meshFn, build);

		// deform mesh
		deformGeo(params, hedgeMesh, i);

		// set output geo
		meshFnFromHalfEdgeMesh(hedgeMesh, meshFn);
		MDataHandle outputGeoHandle = outputArray.outputValue();
		outputGeoHandle.setMObject(meshObj);

	}



	data.setClean(plug);
	return MS::kSuccess;
}

int EdBlendShape::deformGeo(EdBlendShapeParametres &params, HalfEdgeMesh &hedgeMesh, int meshIndex) {
	// deform whole mesh
	// multithread this sick filth

	/* copy mesh position buffer for each handle
	then combine them according to the selected method

	iteration structure is thus:
	for every mesh:
		for every point
			check every handle
		combine

	points are totally independent, we don't need a collapse stage

	*/

	//SmallList<std::vector<float>> deltaBuffers;
	for (int i = 0; i < params.deltaMats.length(); i++) {
		//std::vector<float> deltaPositions(hedgeMesh.pointPositions.values);

		for (int n = 0; n < hedgeMesh.nPoints; i++) {
			deformPoint(params, hedgeMesh,
				/*deltaPositions.data(), hedgeMesh.nPoints,*/
				//deltaPositions,
				hedgeMesh.deltaPointPositions.values,
				i);
		}
	}

	hedgeMesh.swapDeltaBuffers();

	return 0;
}

int EdBlendShape::deformPoint(EdBlendShapeParametres &params, HalfEdgeMesh &hedgeMesh,
	/*float *deltaPositions,
	int nPoints,*/
	std::vector<float> &deltaPositions,
	int index) {
	// deform individual point

	// check first if point is active
	if (params.envelope * params.globalWeights[index] < EPS) {
		return MStatus::kSuccess;
	}

	int nHandles = params.deltaMats.length();
	/*float *weights = new float[nHandles];
	MVector *resultPositions = new MVector[nHandles];*/
	MFloatArray weights(nHandles, 0.0);
	MVectorArray resultPositions(nHandles);
	MVector pos(hedgeMesh.pointPositions.rawEntry(index));


	MVector result;


	// brute force search across all handle captures, don't know if there is a better way
	for (int i = 0; i < nHandles; i++) {
		MVector capturePos = params.captureMats[i].inverse() * pos / params.captureRadii[i];

		float r = capturePos.length();
		// in inverse space, capture region is a sphere radius 1
		if (r > 1 || (params.weightValues[i] < EPS)) {
			// point lies outside capture sphere
			continue;
		}
		float rampVal;
		params.captureRamps[i].getValueAtPosition(r, rampVal);

		float weight = r * params.weightValues[i] * rampVal;
		weights[i] = weight;
		resultPositions[i] = params.deltaMats[i] * pos - pos;

		/*MVector deltaPos = pos + params.deltaMats[i] * pos *
			weight * params.envelope * params.globalWeights[i];*/
	}

	// weighted mean of all positions
	float weightSum = 0;
	//weightSum = std::accumulate(weights, weights + nHandles, weightSum);
	for (int i = 0; i < nHandles; i++) {
		weightSum += weights[i];
	}

	for (int i = 0; i < nHandles; i++) {
		weights[i] = weights[i] / weightSum;
		/*resultPositions[i] = resultPositions[i] * weights[i];
		result += resultPositions[i];*/
		result += resultPositions[i] * weights[i];
	}

	// scale by envelope
	result = pos + result * params.envelope * params.globalWeights[index];

	// set output delta positions
	for (int i = 0; i < 3; i++) {
		deltaPositions[index * 3 + i] = result[i];
	}

	return 1;
}

int EdBlendShape::gatherParams(MDataBlock &data) {
	// iterate over handle plugs, and gather handle data in parametre struct
	MStatus s;
	MArrayDataHandle hdlArray = data.outputArrayValue(aHandles);
	int hdlCount = hdlArray.elementCount();

	// conform param arrays' sizzes
	params.deltaMats.setLength(hdlCount);
	params.captureMats.setLength(hdlCount);
	params.weightValues.setLength(hdlCount);
	params.captureRadii.setLength(hdlCount);

	params.captureRamps.clear();
	params.captureRamps.reserve(hdlCount);

	// apparently needed for ramps
	MObject nodeObj = thisMObject();

	for (int i = 0; i < hdlArray.elementCount(); i++) {
		s = jumpToElement(hdlArray, i);
		MCHECK(s, "gatherHandles jte failed");
		params.deltaMats[i] = hdlArray.outputValue().child(aMat).asMatrix();
		params.captureMats[i] = hdlArray.outputValue().child(aCaptureMat).asMatrix();
		params.weightValues[i] = hdlArray.outputValue().child(aWeight).asFloat();
		params.captureRadii[i] = hdlArray.outputValue().child(aFalloffRadius).asFloat();

		/*MRampAttribute ramp(MPlug(hdlArray.outputValue().child(aFalloffRamp)));*/
		MRampAttribute ramp(nodeObj, aFalloffRamp);
	}

	// gather global parametres
	params.envelope = data.inputValue(envelope).asFloat();

	MFnFloatArrayData fn(data.inputValue(aWeights).data());
	params.globalWeights = fn.array();

	return 1;
}



void* EdBlendShape::creator() {
	// sink is not connected on creation
	EdBlendShape *newObj = new EdBlendShape;
	return newObj;

}

EdBlendShape::EdBlendShape() {};
EdBlendShape::~EdBlendShape() {};

