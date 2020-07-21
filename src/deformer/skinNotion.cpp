

/*
skincluster deformation component
*/

#include "skinNotion.h"

using namespace ed;

MTypeId SkinNotion::kNODE_ID(pluginPrefix, 0x00122C13);
MString SkinNotion::kNODE_NAME( "skinNotion" );

MObject SkinNotion::aWeightList;
MObject DeformerNotion::aWeights;


MStatus DeformerNotion::initialize()
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


	// set affects
	std::vector<MObject> drivers = {aWeightList, aWeights };
  addAttributes( drivers );
  setAttributesAffect(drivers, aUberDeformer);
  return MStatus::kSuccess;
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
