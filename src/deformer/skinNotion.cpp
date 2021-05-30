

/*
skincluster deformation component
*/

#include "skinNotion.h"

using namespace ed;
using namespace std;


MTypeId SkinNotion::kNODE_ID(pluginPrefix, 0x00122C13);
MString SkinNotion::kNODE_NAME( "skinNotion" );

MObject SkinNotion::aWeightList;
MObject SkinNotion::aWeights;
MObject SkinNotion::aWeightMode;

MObject SkinNotion::aTransformMatrices;
MObject SkinNotion::aBindMatrices;


MStatus SkinNotion::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnTypedAttribute tFn;
	MFnCompoundAttribute cFn;
	MFnEnumAttribute eFn;

	// multi attributes mirroring skincluster weights
  // weight entry for single vertex
	aWeights = nFn.create("weights", "weights", MFnNumericData::kFloat, 0.0);
  nFn.setArray(true);
  nFn.setUsesArrayDataBuilder(true);
	//addAttribute(aWeights);

  // list of weight entries, one per vertex
  aWeightList = cFn.create("weightList", "weightList");
  cFn.setArray(true);
  cFn.setUsesArrayDataBuilder(true);
  cFn.addChild(aWeights);

	// should weights be looked up live or only on bind?
	aWeightMode = eFn.create("weightMode", "weightMode", 0);
	eFn.addField("live", SkinWeightMode::live);
	eFn.addField("onBind", SkinWeightMode::onBind);
	eFn.setHidden(false);

  // transform joint matrices
  aTransformMatrices = tFn.create("matrix", "matrix", MFnData::kMatrix);
  tFn.setArray(true);
  tFn.setUsesArrayDataBuilder(true);

  aBindMatrices = tFn.create("bindPreMatrix", "bindPreMatrix", MFnData::kMatrix);
  tFn.setArray(true);
  tFn.setUsesArrayDataBuilder(true);


	// set affects
	std::vector<MObject> drivers = {aWeightList, aWeightMode,
    aTransformMatrices, aBindMatrices };
  addAttributes<SkinNotion>( drivers );
  setAttributesAffect<SkinNotion>(drivers, aUberDeformer);
  return MStatus::kSuccess;
}


int SkinNotion::deformPoint(int index, SkinNotionParametres &params,
	ed::HalfEdgeMesh &hedgeMesh){

		//SmallList<float> baseList = mesh.pointPositions.entry(index);
		const float * baseList = hedgeMesh.pointPositions.rawEntry(index);
		MPoint basePos(
			baseList[0], baseList[1], baseList[2], 1.0);

		// get skin indices and weights
		SmallList<int> weightIndices = entryFromBuffer(
			params.weightIndices, params.vertexOffsets, index);
		SmallList<float> weightValues = entryFromBuffer(
			params.weightValues, params.vertexOffsets, index);

		// output position
		MPoint result;
		MPoint tfPos(0, 0, 0, 1.0);
		MMatrix diffMat;
		int infIndex;
		float weightVal;

		for (int i = 0; i < weightValues.size(); i++) {
			infIndex = weightIndices[i];
			weightVal = weightValues[i];
			if (infIndex == -1) { // no valid transform index
				continue;
			}
			diffMat = params.refMats[i] * params.transformMats[i];
			tfPos += (basePos * diffMat) * weightVal;
		}

		// blend output by envelope
		result = basePos * (1 - params.localEnvelope) + tfPos * params.localEnvelope;
		hedgeMesh.deltaPointPositions.setEntry<MPoint>(index, result);
		return 1;

	}


/* traverse basic skincluster weight system to build more efficient weight
buffers, then transfer those into array attributes
refresher:
	weightList[ vtxId ].weights[ influenceIndex ] = influence weight
*/
void SkinNotion::extractSkinWeights(MArrayDataHandle& weightRoot,
  SkinNotionParametres& params) {
	// extracts skin data to buffers
  int nPoints = weightRoot.elementCount();
	vector<int> vertexOffsets(nPoints);
	vector<int> influenceIndices;
	vector<float> influenceWeights;
	// eps value to discount negligible influences
	static const float e = 0.001f;

	int offsetIndex = 0;
	int entryCreated = 0;
	for (unsigned int i = 0; i < nPoints; i++) {
		entryCreated = 0;
		vertexOffsets[i] = offsetIndex;
		jumpToElement(weightRoot, i);
		MArrayDataHandle vtxEntry = weightRoot.inputValue().child(
			aWeights);
		unsigned int nWeights = vtxEntry.elementCount();

		for (unsigned int n = 0; n < nWeights; n++) {
			float weightVal = vtxEntry.inputValue().asDouble();
			if (weightVal > e) {
				influenceIndices.push_back(vtxEntry.elementIndex());
				influenceWeights.push_back(weightVal);
				offsetIndex++;
				entryCreated = 1;
			}
			vtxEntry.next();
		}

		// no entry for vertex created yet
		if (nWeights == 0 || entryCreated == 0) {
			influenceIndices.push_back(-1); //
			influenceWeights.push_back(0.0);
			offsetIndex++;
			continue;
		}
	}
	params.vertexOffsets = vertexOffsets;
	params.weightIndices = influenceIndices;
	params.weightValues = influenceWeights;
}

void SkinNotion::extractSkinWeights(MPlug& weightRoot,
  SkinNotionParametres& params) {
	// alt implementation for ansynchronous access with MPlugs
  int nPoints = weightRoot.numElements();
	vector<int> vertexOffsets(nPoints);
	vector<int> influenceIndices;
	vector<float> influenceWeights;
	// eps value to discount negligible influences
	static const float e = 0.001f;
	MPlug weightIndexPlug;
	MPlug weightChildArrayPlug;
	MPlug weightValPlug;
	float weightVal;

	int offsetIndex = 0;
	int entryCreated = 0;
	for (unsigned int i = 0; i < nPoints; i++) {
		entryCreated = 0;
		vertexOffsets[i] = offsetIndex;
		weightIndexPlug = weightRoot.elementByPhysicalIndex(i);
		// MArrayDataHandle vtxEntry = weightRoot.inputValue().child(
		// 	aWeights);
		weightChildArrayPlug = weightIndexPlug.child(aWeights);
		unsigned int nWeights = weightChildArrayPlug.numElements();

		for (unsigned int n = 0; n < nWeights; n++) {
			weightValPlug = weightChildArrayPlug.elementByPhysicalIndex(n);
			weightVal = weightValPlug.asFloat();
			if (weightVal > e) {
				influenceIndices.push_back(weightValPlug.logicalIndex());
				influenceWeights.push_back(weightVal);
				offsetIndex++;
				entryCreated = 1;
			}
			//vtxEntry.next();
		}

		// no entry for vertex created yet
		if (nWeights == 0 || entryCreated == 0) {
			influenceIndices.push_back(-1); //
			influenceWeights.push_back(0.0);
			offsetIndex++;
			continue;
		}
	}
	params.vertexOffsets = vertexOffsets;
	params.weightIndices = influenceIndices;
	params.weightValues = influenceWeights;
}

int SkinNotion::extractParametres(
	MDataBlock &data, SkinNotionParametres &params){
	// retrieve skin data and joint matrices
	DeformerNotion::extractParametres(data, params);

	int weightModeVal = data.inputValue(aWeightMode).asInt();
	if (weightModeVal == SkinWeightMode::live){ // live weights
		extractSkinWeights(
			data.inputArrayValue(aWeightList), params);
	}
	params.refMats = extractMMatrixArray(
		data.inputArrayValue(aBindMatrices));
	params.transformMats = extractMMatrixArray(
		data.inputArrayValue(aTransformMatrices));
	return 1;
}

int SkinNotion::bind(
	MFnDependencyNode &mfn, SkinNotionParametres &params,
	ed::HalfEdgeMesh &hedgeMesh){
		MPlug plugRoot = mfn.findPlug(aWeightList, false);
		extractSkinWeights(plugRoot, params);
		return 1;
	}
