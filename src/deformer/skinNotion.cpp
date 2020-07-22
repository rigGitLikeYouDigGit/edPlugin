

/*
skincluster deformation component
*/

#include "skinNotion.h"

using namespace ed;

MTypeId SkinNotion::kNODE_ID(pluginPrefix, 0x00122C13);
MString SkinNotion::kNODE_NAME( "skinNotion" );

MObject SkinNotion::aWeightList;
MObject SkinNotion::aWeights;


MStatus SkinNotion::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnTypedAttribute tFn;
  MFnCompoundAttribute cFn;

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


  // transform joint matrices
  aTransformMatrices = tFn.create("matrix", "matrix", MFnData::kMatrix);
  tFn.setArray(true);
  tFn.setUsesArrayDataBuilder(true);

  aBindMatrices = tFn.create("bindPreMatrix", "bindPreMatrix", MFnData::kMatrix);
  tFn.setArray(true);
  tFn.setUsesArrayDataBuilder(true);


	// set affects
	std::vector<MObject> drivers = {aWeightList, aWeights,
    aTransformMatrices, aBindMatrices };
  addAttributes( drivers );
  setAttributesAffect(drivers, aUberDeformer);
  return MStatus::kSuccess;
}


/* traverse basic skincluster weight system to build more efficient weight
buffers, then transfer those into array attributes
refresher:
	weightList[ vtxId ].weights[ influenceIndex ] = influence weight
*/
void SkinNotion::extractSkinWeights(MArrayDataHandle& weightRoot,
  SkinNotionParametres& params) {
	// extracts skin data to buffers
	// restricted to nPoints
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



// MStatus DeformerNotion::compute(
// 				const MPlug& plug, MDataBlock& data) {
//
// 	// "balance wheel" mechanism to mark node dirty to uberDeformer
// 	// thanks Matt
// 	bool old = data.outputValue(aUberDeformer).asBool();
// 	data.outputValue(aUberDeformer).setBool( !old );
//
//
// 	data.setClean(plug);
//     return MS::kSuccess;
// }
//
// void* DeformerNotion::creator(){
//
//     return new DeformerNotion;
//
// }
//
// DeformerNotion::DeformerNotion() {};
// DeformerNotion::~DeformerNotion() {};
