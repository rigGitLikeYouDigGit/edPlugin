

/*

	build deformation scheme by iterating over deformation functions
	
*/

#include "uberDeformer.h"

#include "deformer/deformerNotion.h" // satellite nodes to track

using namespace std;
using namespace ed;

MTypeId UberDeformer::kNODE_ID(0x00122C09);
MString UberDeformer::kNODE_NAME( "uberDeformer" );

MObject UberDeformer::aBind;
MObject UberDeformer::aGlobalIterations;
MObject UberDeformer::aNotions;


MStatus UberDeformer::initialize()
{
    // initialise attributes
	MFnEnumAttribute eFn;
	MFnNumericAttribute nFn;

	// standard bind system
	aBind = eFn.create("bind", "bind", 1);
	eFn.addField("off", 0);
	eFn.addField("bind", 1);
	eFn.addField("bound", 2);
	eFn.addField("live", 3);
	eFn.setKeyable(true);
	eFn.setHidden(false);
	addAttribute(aBind);

	// array of booleans to connect to deformerNotions
	aNotions = nFn.create("notions", "notions", MFnNumericData::kBoolean, 0);
	nFn.setArray(true);
	nFn.setUsesArrayDataBuilder(true);
	nFn.setWritable(true);
	nFn.setReadable(false);
	nFn.setKeyable(false);
	addAttribute(aNotions);

	// iterations to run over entire deformer system
	aGlobalIterations = nFn.create("iterations", "iterations", MFnNumericData::kInt, 1);
	addAttribute(aGlobalIterations);


    return MStatus::kSuccess;
}


MStatus UberDeformer::deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex) {
	
	// check bind
	int bind = data.inputValue(aBind).asInt();
	if (bind == 1 || bind == 3) { // bind or live

		if (bind == 1) {
			data.inputValue(aBind).setInt(2);
		}
	}
    return MS::kSuccess;
}

vector<MObject> UberDeformer::getConnectedNotions() {
	// returns sequential vector of all deformerNotions connected to deformer
	connectedNotions.clear();
	MPlugArray connectedPlugs;
	MPlug notionsPlug(thisMObject(), aNotions);

	notionsPlug.connectedTo(connectedPlugs, true, false);
	for (unsigned int i = 0; i < connectedPlugs.length(); i++) {
		DEBUGS("plug " << connectedPlugs[i].name());
		connectedNotions.push_back(connectedPlugs[i].node());
	}
	return connectedNotions;
}


MStatus UberDeformer::connectionMade(
	const MPlug &plug, const MPlug &otherPlug, bool asSrc) {

	// we only care about sink plug
	if (plug.attribute() != aNotions) {
		return MPxNode::connectionMade(plug, otherPlug, asSrc);
	}

	getConnectedNotions();
	return MPxNode::connectionMade(plug, otherPlug, asSrc);
}

MStatus UberDeformer::connectionBroken(
	const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// clear connected sink node
	DEBUGS("memorySource connectionBroken");

	if (plug.attribute() != aNotions) {
		return MPxNode::connectionBroken(plug, otherPlug, asSrc);
	}
	getConnectedNotions();
	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
}



void* UberDeformer::creator(){

     UberDeformer *node = new UberDeformer;

	 return node;

}

UberDeformer::UberDeformer() {};
UberDeformer::~UberDeformer() {};

