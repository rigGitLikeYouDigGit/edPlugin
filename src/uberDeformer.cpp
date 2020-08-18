

/*

	build deformation scheme by iterating over deformation functions

*/

#include "uberDeformer.h"



using namespace std;
using namespace ed;

MTypeId UberDeformer::kNODE_ID(0x00122C09);
MString UberDeformer::kNODE_NAME( "uberDeformer" );

MObject UberDeformer::aBind;
MObject UberDeformer::aGlobalIterations;
MObject UberDeformer::aGlobalEnvelope;
MObject UberDeformer::aNotions;

MObject UberDeformer::aOutputGeo;


MStatus UberDeformer::initialize()
{
    // initialise attributes
	MFnEnumAttribute eFn;
	MFnNumericAttribute nFn;

	// standard bind system
	aBind = makeBindAttr("bind");

	// array of booleans to connect to deformerNotions
	aNotions = nFn.create("notions", "notions", MFnNumericData::kBoolean, 0);
	nFn.setArray(true);
	nFn.setUsesArrayDataBuilder(true);
	nFn.setWritable(true);
	nFn.setReadable(false);
	nFn.setKeyable(false);

	// iterations to run over entire deformer system
	aGlobalIterations = nFn.create("iterations", "iterations", MFnNumericData::kInt, 1);
	nFn.setMin(0);
	addAttribute(aGlobalIterations);

	aGlobalEnvelope = nFn.create("envelope", "envelope", MFnNumericData::kFloat, 1);
	nFn.setMin(0);

	vector<MObject> drivers = {aGlobalIterations, aGlobalEnvelope, aBind, aNotions};
	setAttributesAffect(drivers, outputGeom);

    return MStatus::kSuccess;
}


MStatus UberDeformer::compute(
	MDataBlock& data, const MPlug& plug
){
	// bind first if needed, then compute
	int bindVal = data.inputValue(aBind).asInt();

	MArrayDataHandle inputArray = data.inputArrayValue(input);
	jumpToElement(inputArray, 0);
	MObject meshObj = (inputArray.inputValue().child(inputGeom)).asMesh();
	//MObject meshObj = geoHandle.asMesh();

	if(plug != outputGeom | bindVal == BindState::off | meshObj.isNull()){
		hedgeMesh.hasBuilt = 0;
		data.setClean(plug);
		return MStatus::kSuccess;
	}

	// build hedgemesh if not already built
	if(!meshObj.isNull() && !hedgeMesh.hasBuilt){
		HalfEdgeMeshFromMObject(hedgeMesh, meshObj, 1);
	}
	else{
		// only update positions
		HalfEdgeMeshFromMObject(hedgeMesh, meshObj, 0);
	}

	MFnMesh meshFn( meshObj );

	if( bindVal == BindState::bind | bindVal == BindState::live){
		bindDeformerNetwork();
		if( bindVal == BindState::bind){
			data.outputValue(aBind).setInt( BindState::bound);
		}
	}

	int globalIterations = data.inputValue(aGlobalIterations).asInt();
	float globalEnvelope = data.inputValue(aGlobalEnvelope).asFloat();
	globalDeform(globalIterations, globalEnvelope);


	// set output geometry points

}





void UberDeformer::bindDeformerNetwork(){
	bindUberDeformer();
	bindDeformerNotions();
}

void UberDeformer::bindUberDeformer(){
	// handled automatically right now
}

void UberDeformer::bindDeformerNotions(){
	// runs bind method on all connected components
	for(unsigned int i = 0; i < connectedNotions.size(); i++){
		DeformerNotion node = *connectedNotions[i];
		MFnDependencyNode mfnNode(node);
		node::bind(mfnNode, node->params, hedgeMesh);
	}
}

//vector<MObject*> UberDeformer::getConnectedNotions() {
void UberDeformer::getConnectedNotions() {
	// returns sequential vector of all deformerNotions connected to deformer
	connectedNotions.clear();
	MPlugArray connectedPlugs;
	MPlug notionsPlug(thisMObject(), aNotions);

	notionsPlug.connectedTo(connectedPlugs, true, false);
	for (unsigned int i = 0; i < connectedPlugs.length(); i++) {
		DEBUGS("plug " << connectedPlugs[i].name());
		connectedNotions.push_back(connectedPlugs[i].node());
	}
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
