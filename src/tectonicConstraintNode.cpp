

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
//using namespace std;

MTypeId TectonicConstraintNode::kNODE_ID(0x00000000);
//MTypeId TectonicConstraintNode::kNODE_ID(0x00122C10);
MString TectonicConstraintNode::kNODE_NAME( "tectonicConstraint" );

// Balance wheel signal to solver, to cast in the ConstraintData
MObject TectonicConstraintNode::aSolver;

// goal point data - each has a tie of
// goal vector, base point vector
MObject TectonicConstraintNode::aGoals;
	MObject TectonicConstraintNode::aBasePos;
		MObject TectonicConstraintNode::aBasePosX;
		MObject TectonicConstraintNode::aBasePosY;
		MObject TectonicConstraintNode::aBasePosZ;
	MObject TectonicConstraintNode::aGoalPos;
		MObject TectonicConstraintNode::aGoalPosX;
		MObject TectonicConstraintNode::aGoalPosY;
		MObject TectonicConstraintNode::aGoalPosZ;

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
	MFnCompoundAttribute cFn;
    MFnTypedAttribute tFn;
    MFnNumericAttribute nFn;

	aSolver = nFn.create("solver", "solver",
		MFnNumericData::kBoolean);
	nFn.setWritable(false);

	// goal attributes
	aGoals = cFn.create("goal", "goal");
	aBasePos = makeXYZVectorAttr("basePos",
		aBasePos, aBasePosX, aBasePosY, aBasePosZ, false);
	aGoalPos = makeXYZVectorAttr("endPos",
		aGoalPos, aGoalPosX, aGoalPosY, aGoalPosZ, false);
	aPointIndex = nFn.create("pointIndex", "pointIndex",
		MFnNumericData::kInt);
	aPlateIndex = nFn.create("plateIndex", "plateIndex",
		MFnNumericData::kInt);
	cFn.setArray(true);
	cFn.addChild(aBasePos);
	cFn.addChild(aGoalPos);
	cFn.addChild(aPointIndex);
	cFn.addChild(aPlateIndex);

	// dynamic control attributes
	aStrength = nFn.create("strength", "strength",
		MFnNumericData::kFloat);
	aDamping = nFn.create("damping", "damping",
		MFnNumericData::kFloat);

	std::vector<MObject> drivers { 
		aGoals,
		aStrength, aDamping };

	std::vector<MObject> driven{ aSolver };

	setAttributesAffect<TectonicConstraintNode>(drivers, driven);
	addAttributes<TectonicConstraintNode>(drivers);
	addAttributes<TectonicConstraintNode>(driven);

    return MStatus::kSuccess;
}


MStatus TectonicConstraintNode::compute(
				const MPlug& plug, MDataBlock& data) {
	//DEBUGS("TectonicConstraintNode compute")
	MStatus s = MS::kSuccess;

	// flip balance wheel switch
	bool old = data.outputValue(aSolver).asBool();
	data.outputValue(aSolver).setBool( !old );

	


	data.setClean(plug);


    return s;
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
