

/*

	individual component deformation of uberDeformer
	
*/

#include "deformerNotion.h"

MTypeId DeformerNotion::kNODE_ID(0x00122C10);
MString DeformerNotion::kNODE_NAME( "deformerNotion" );

MObject DeformerNotion::aWeights;
MObject DeformerNotion::aUseWeights;
MObject DeformerNotion::aLocalIterations;
MObject DeformerNotion::aUberDeformer;


MStatus DeformerNotion::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnTypedAttribute tFn;

	// boolean plug to connect to master deformer
	aUberDeformer = nFn.create("uberDeformer", "uberDeformer", MFnNumericData::kBoolean, 0);
	addAttribute(aUberDeformer);
	
	// check whether to use per-point weights for deformation
	aUseWeights = nFn.create("useWeights", "useWeights", MFnNumericData::kBoolean, 0);
	addAttribute(aUberDeformer);

	// weight array attribute
	aWeights = tFn.create("weights", "weights", MFnData::kDoubleArray);
	addAttribute(aWeights);

	// local iterations, how many times to loop this operation on each cycle
	aLocalIterations = nFn.create("localIterations", "localIteration", MFnNumericData::kInt, 1);
	addAttribute(aLocalIterations);
	
	// set affects
	std::vector<MObject> drivers = { aUseWeights, aWeights, aLocalIterations };
	for (auto &it : drivers) {
		attributeAffects(it, aUberDeformer);
	}

    return MStatus::kSuccess;
}




MStatus DeformerNotion::compute(
				const MPlug& plug, MDataBlock& data) {

	// "balance wheel" mechanism to mark node dirty to uberDeformer
	// thanks Matt
	bool old = data.outputValue(aUberDeformer).asBool();
	data.outputValue(aUberDeformer).setBool( !old );
	

	data.setClean(plug);
    return MS::kSuccess;
}

void* DeformerNotion::creator(){

    return new DeformerNotion;

}

DeformerNotion::DeformerNotion() {};
DeformerNotion::~DeformerNotion() {};

