

/*
	in modes other than u, find u value on main curve and look up on upcurve

	framing points must be equally spaced in arc length
*/

#include "curveFrame.h"

using namespace ed;
using namespace std;

MTypeId CurveFrame::kNODE_ID(0x00122C1C);
MString CurveFrame::kNODE_NAME( "curveFrame" );

MObject CurveFrame::aInCurve;
MObject CurveFrame::aInUpCurve;
MObject CurveFrame::aInPoints;
MObject CurveFrame::aInParam;
MObject CurveFrame::aInPosition;


MObject CurveFrame::aFrameResolution;
MObject CurveFrame::aSolver;
MObject CurveFrame::aFrameNode;

MObject CurveFrame::aOutUpCurve;
MObject CurveFrame::aOutTangents;
MObject CurveFrame::aOutNormals;
MObject CurveFrame::aOutPoints;
MObject CurveFrame::aOutPosition;
MObject CurveFrame::aOutMatrix;



#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\

void CurveFrame::postConstructor() {
	this->setExistWithoutInConnections(true);
	this->setExistWithoutOutConnections(true);
}

MStatus CurveFrame::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;
	MFnCompoundAttribute cFn;
	MFnEnumAttribute eFn;
	MFnAttribute aFn;

	// curve to frame
	aInCurve = tFn.create("inputCurve", "inputCurve", MFnNurbsCurveData::kNurbsCurve);
	tFn.setReadable(false);

	// upcurve for upcurve solver
	aInUpCurve = tFn.create("inputUpCurve", "inputUpCurve", MFnNurbsCurveData::kNurbsCurve);
	tFn.setReadable(false);

	// solver enum
	aSolver = eFn.create("solver", "solver", 0);
	eFn.addField("rmf", CurveFrame::CurveSolvers::doubleReflection);
	eFn.addField("upCurve", CurveFrame::CurveSolvers::upCurve);
	eFn.addField("frameNode", CurveFrame::CurveSolvers::frameNode);

	// other frame node
	aFrameNode = nFn.create("frameNode", "frameNode", MFnNumericData::kBoolean, 0);




	vector<MObject> drivers = { 
		aInCurve, aInUpCurve, aInPoints, aInParam, aInPosition,
		aFrameResolution
	};
	vector<MObject> driven = {
		aOutUpCurve, aOutPoints, aOutPosition, aOutMatrix
	};
	
	addAttributes<CurveFrame>(drivers);
	addAttributes<CurveFrame>(driven);

	setAttributesAffect<CurveFrame>(drivers, driven);

    return MStatus::kSuccess;
}


MStatus CurveFrame::compute(
				const MPlug& plug, MDataBlock& data) {

	data.setClean(plug);
    return MS::kSuccess;
}


CurveFrame * CurveFrame::getFrameInstance(MObject &nodeObj, MStatus &s) {
	// retrieve and cast full user-defined node for given MObject
	// thanks Matt
	MFnDependencyNode nodeFn(nodeObj);
	
	// retrieve MPxNode pointer
	MPxNode *mpxPtr = nodeFn.userNode(&s);
	//MCHECK(s, "failed to extract mpxNode pointer, object is invalid");

	// black science
	CurveFrame * sinkPtr = dynamic_cast<CurveFrame *>(mpxPtr);
	if (sinkPtr == NULL) {
		cerr << "failed dynamic cast to sink instance " << endl;
		s = MS::kFailure;
	}
	frameNodeConnected = true;
	s = MS::kSuccess;
	return sinkPtr;

}




MStatus CurveFrame::connectionMade(
	const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// check if sink has been connected 
	//DEBUGS("memorySource connectionMade");

	//DEBUGS("plug " << plug.name() << " otherPlug " << otherPlug.name());
	MStatus s;

	// we only care about sink plug
	if ((plug.attribute() != aFrameNode) || asSrc ){
		return MPxNode::connectionMade(plug, otherPlug, asSrc);
	}

	//DEBUGS("connection to sink plug");

	if (otherPlug.attribute() == CurveFrame::aFrameNode) {
		if (frameNodeConnected) {
			// cannot connect two sinks to one source
			DEBUGS("curveFrame already has source solver connected");
			return MStatus::kFailure;
		}
		framePtr = getFrameInstance(otherPlug.node(), s);
		MCHECK(s, "failed to retrieve a pointer to other node")
	}

	return MPxNode::connectionMade(plug, otherPlug, asSrc);
}

MStatus CurveFrame::connectionBroken(
	const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// clear connected sink node

	if ((plug.attribute() != aFrameNode) || asSrc){
		return MPxNode::connectionBroken(plug, otherPlug, asSrc);
	}
	framePtr = NULL;
	frameNodeConnected = false;

	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
}

void* CurveFrame::creator(){
	// sink is not connected on creation
	CurveFrame *newObj = new CurveFrame;
	newObj->frameNodeConnected = false;
	newObj->framePtr = NULL;
    return newObj;

}

CurveFrame::CurveFrame() {};
CurveFrame::~CurveFrame() {};

