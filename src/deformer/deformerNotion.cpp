

/*

	individual component deformation of uberDeformer

*/

#include "deformerNotion.h"

using namespace ed;
using namespace stl;

MTypeId DeformerNotion::kNODE_ID(pluginPrefix, 0x00122C11);
MString DeformerNotion::kNODE_NAME( "deformerNotion" );

MObject DeformerNotion::aMasterWeights;
MObject DeformerNotion::aLocalIterations;
MObject DeformerNotion::aUberDeformer;


MStatus DeformerNotion::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnTypedAttribute tFn;

	// boolean plug to connect to master deformer
	aUberDeformer = nFn.create("uberDeformer", "uberDeformer", MFnNumericData::kBoolean, 0);
	// addAttribute(aUberDeformer);

	aEnvelope = nFn.create("envelope", "envelope", MFnNumericData::kFloat, 1.0);
	// addAttribute(aEnvelope);

	// weight array attribute
	aMasterWeights = tFn.create("weights", "weights", MFnData::kDoubleArray);
	// addAttribute(aMasterWeights);

	// local iterations, how many times to loop this operation on each cycle
	aLocalIterations = nFn.create("localIterations", "localIteration", MFnNumericData::kInt, 1);
	// addAttribute(aLocalIterations);

	// set affects
	vector<MObject> drivers = { aMasterWeights, aLocalIterations, aEnvelope };
	vector<MObject> driven = { aUberDeformer };

	addAttributes( drivers );
	addAttributes( driven );

	setAttributeAffectsAll(drivers, driven);

    return MStatus::kSuccess;
}

virtual int DeformerNotion::extractParametres(
	MDataBlock &data, DeformerParametres &params
){
	params.envelope = data.inputValue(aEnvelope).asFloat();
	params.localIterations = data.inputValue(aLocalIterations).asInt();
	params.masterWeights = accessDoubleArrayAttr(data.inputValue(aMasterWeights));
	return 1;

}


MStatus DeformerNotion::compute(
				const MPlug& plug, MDataBlock& data) {

	extractParametres(data, this->params);

	// "balance wheel" mechanism to mark node dirty to uberDeformer
	// thanks Matt
	bool old = data.outputValue(aUberDeformer).asBool();
	data.outputValue(aUberDeformer).setBool( !old );


	data.setClean(plug);
    return MS::kSuccess;
}

virtual int bind( MDataBlock &data, DeformerParametres &params, HalfEdgeMesh &hedgeMesh ){
	// runs precomputation for deformerNotion, saves results into params

	// DO EXPENSIVE PRECOMPUTATION HERE
	return 1;
}


void* DeformerNotion::creator(){

    return new DeformerNotion;

}

DeformerNotion::DeformerNotion() {};
DeformerNotion::~DeformerNotion() {};
