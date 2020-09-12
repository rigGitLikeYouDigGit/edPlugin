

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
//MObject UberDeformer::aGlobalEnvelope; // inherited
MObject UberDeformer::aNotions;
MObject UberDeformer::aOutputGeo;

/*
MObject input - input attribute, array
MObject inputGeom - input geometry attribute
MObject outputGeom - output geometry attribute, array
MObject envelope - envelope attribute
*/

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
	nFn.setMin(1);
	addAttribute(aGlobalIterations);

	vector<MObject> drivers = {aGlobalIterations,
		aBind, aNotions};
	setAttributesAffect<UberDeformer>(drivers, outputGeom);

    return MStatus::kSuccess;
}


MStatus UberDeformer::compute(
	MDataBlock& data, const MPlug& plug
){
	// bind first if needed, then compute
	int bindVal = data.inputValue(aBind).asInt();
	float envelopeValue = data.inputValue(envelope).asFloat();

	MArrayDataHandle inputArray = data.inputArrayValue(input);
	jumpToElement(inputArray, 0);
	MObject meshObj = (inputArray.inputValue().child(inputGeom)).asMesh();
	//MObject meshObj = geoHandle.asMesh();

	if( plug != outputGeom){
		data.setClean(plug);
		return MStatus::kSuccess;
	}

	if( envelopeValue < 0.001){
		// set output geo
		return MStatus::kSuccess;
	}

	if( (bindVal == BindState::off) | meshObj.isNull()){
		hedgeMesh.hasBuilt = 0;
		data.setClean(plug);
		return MStatus::kSuccess;
	}

	// build hedgemesh if not already built
	if(!meshObj.isNull() && !hedgeMesh.hasBuilt){
		HalfEdgeMeshFromMObject(hedgeMesh, meshObj, 1); // build topo
	}
	else{
		// only update positions
		HalfEdgeMeshFromMObject(hedgeMesh, meshObj, 0); // only set positions
	}

	MFnMesh meshFn( meshObj );

	if( (bindVal == BindState::bind) | (bindVal == BindState::live)){
		bindDeformerNetwork();
		if( bindVal == BindState::bind){
			data.outputValue(aBind).setInt( BindState::bound);
		}
	}

	int globalIterations = data.inputValue(aGlobalIterations).asInt();
	globalDeform(globalIterations, envelopeValue);

	// set output geometry points
	meshFnFromHalfEdgeMesh(hedgeMesh, meshFn);
	setOutputGeo(data, meshObj);
	// what up now swedes
}


void UberDeformer::globalDeform(int globalIterations, float globalEnvelope){
	for( int globalI = 0; globalI < globalIterations; globalI++){

		for( unsigned int deformerI = 0;
			deformerI < connectedNotions.size(); deformerI++){
				DeformerNotion* deformer = connectedNotions[deformerI];
				deformer->params.globalIterations = globalIterations;
				deformer->params.globalIteration = globalI;
				
				deformer->deformGeo( deformer->params, hedgeMesh);
		}
	}

}


void UberDeformer::setOutputGeo(MDataBlock& data, MObject& meshGeo) {
	// sets output plug to target mesh object
	// we deform only one piece of geometry
	MArrayDataHandle outputArray = data.outputArrayValue(outputGeom);
	jumpToElement(outputArray, 0);
	MDataHandle outputGeoHandle = outputArray.outputValue();
	outputGeoHandle.setMObject(meshGeo);
}


///// binding /////
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
		MObject nodeObj;
		DeformerNotion * node = connectedNotions[i];
		nodeObj = node->thisMObject();
		MFnDependencyNode mfnNode(nodeObj);
		node->bind(mfnNode, node->params, hedgeMesh);
	}
}


//// managing connected deformers
void UberDeformer::getConnectedNotions() {
	// returns sequential vector of all deformerNotions connected to deformer
	connectedNotions.clear();
	MPlugArray connectedPlugs;
	MPlug notionsPlug(thisMObject(), aNotions);

	notionsPlug.connectedTo(connectedPlugs, true, false);
	for (unsigned int i = 0; i < connectedPlugs.length(); i++) {
		DEBUGS("plug " << connectedPlugs[i].name());
		DeformerNotion * node = (DeformerNotion *)MFnDependencyNode(connectedPlugs[i].node()).userNode();
		connectedNotions.push_back( node );
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
