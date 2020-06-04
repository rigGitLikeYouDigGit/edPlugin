

/*

deformer module for wrapping one geometry to another

*/

#include "deformerNotion.h"
#include "wrapNotion.h"
#include "containers.h"
#include "lib/topo.h"
//
// MTypeId DeformerNotion::kNODE_ID(0x00122C10);
// MString DeformerNotion::kNODE_NAME( "deformerNotion" );

// attribute MObjects
MObject WrapNotion::aDriverMesh;
MObject WrapNotion::aDriverBaseMesh;

// driven mesh and driven base mesh are passed in from deformerNotion at runtime


// bind closest point buffer
void bindClosestPoints(OffsetBuffer &closestPoints, HalfEdgeMesh &deformGeo, HalfEdgeMesh &driverGeo,
 	int mode){
	}

void bindBarycentric(){

}


MStatus DeformerNotion::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnTypedAttribute tFn;

	// boolean plug to connect to master deformer
	aMasterConnection = nFn.create("masterConnection", "masterConnection", MFnNumericData::kBoolean, 0);
	addAttribute(aMasterConnection);

	// check whether to use per-point weights for deformation
	aUseWeights = nFn.create("useWeights", "useWeights", MFnNumericData::kBoolean, 0);
	addAttribute(aMasterConnection);

	// weight array attribute
	aWeights = tFn.create("weights", "weights", MFnData::kDoubleArray);
	addAttribute(aWeights);

	// local iterations, how many times to loop this operation on each cycle
	aLocalIterations = nFn.create("localIterations", "localIteration", MFnNumericData::kInt, 1);
	addAttribute(aLocalIterations);

	// set affects
	std::vector<MObject> drivers = { aUseWeights, aWeights, aLocalIterations };
	for (auto &it : drivers) {
		attributeAffects(it, aMasterConnection);
	}

    return MStatus::kSuccess;
}




MStatus DeformerNotion::compute(
				const MPlug& plug, MDataBlock& data) {

	// "balance wheel" mechanism to mark node dirty to uberDeformer
	// thanks Matt
	bool old = data.outputValue(aMasterConnection).asBool();
	data.outputValue(aMasterConnection).setBool( !old );


	data.setClean(plug);
    return MS::kSuccess;
}

void* DeformerNotion::creator(){

    return new DeformerNotion;

}

DeformerNotion::DeformerNotion() {};
DeformerNotion::~DeformerNotion() {};
