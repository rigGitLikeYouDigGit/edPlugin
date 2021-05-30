

/*

master constraint node for use with tectonic
painting any kind of value on edges in maya is super painful,
so this is a way to assign polarity values to many edges at once

will also be used to set collision, attraction etc

be as topo independent as possible - closest point as much as possible



*/

#include "lib/topo.h"
#include "tectonic/constraint.h"
#include "tectonicConstraintNode.h"


using namespace ed;
using namespace std;

MTypeId TectonicConstraintNode::kNODE_ID(0x00122C08);
MString TectonicConstraintNode::kNODE_NAME( "tectonicConstraint" );

// Balance wheel signal to solver, to cast in the ConstraintData
MObject TectonicConstraintNode::aSolver;

// goal point data - each has a tie of
// goal vector, base point vector
MObject TectonicConstraintNode::aGoals;
	MObject TectonicConstraintNode::aBasePos;
	MObject TectonicConstraintNode::aGoalPos;

	// int attributes showing which point and plate connected to?
	MObject TectonicConstraintNode::aPointIndex;
	MObject TectonicConstraintNode::aPlateIndex;

// all goals in single constraint share settings

// spring settings
MObject TectonicConstraintNode::aStrength;
MObject TectonicConstraintNode::aDamping;

// could use these as pistons
MObject TectonicConstraintNode::aTargetLength;

MStatus TectonicConstraintNode::initialize()
{

    // initialise attributes
    MFnTypedAttribute tFn;
    MFnNumericAttribute nFn;

	aSolver = nFn.create("solver", "solver",
		MFnNumericData::kBoolean);

    return MStatus::kSuccess;
}


MStatus TectonicConstraintNode::compute(
				const MPlug& plug, MDataBlock& data) {
    // going with floats for now, can easily switch to doubles if needed
	// initialise MFnMesh
	//DEBUGS("TectonicConstraintNode compute")
	MStatus s = MS::kSuccess;

	data.setClean(plug);


    return MS::kSuccess;
}

void TectonicConstraintNode::postConstructor() {
	this->setExistWithoutInConnections(true);
	this->setExistWithoutOutConnections(true);
}


void* TectonicConstraintNode::creator(){

    return new TectonicConstraintNode;

}


TectonicConstraintNode::TectonicConstraintNode() {};
TectonicConstraintNode::~TectonicConstraintNode() {};
