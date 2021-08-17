

/*
Builds Tectonic dynamic system from Maya meshes and objects.
Collision treated separately to articulation - 

by default all edges of a mesh are treated as articulated joins,
with each mesh face treated as a plate.

Nonmanifold and many-edged faces are supported. To mark an edge as rigid
(ie both of its faces forming the same plate), set it to a crease value of 1.

MAYBE instead we use UV connectivity to show plates

Query points are suggested to map out individual plates
for richer data array attributes are used, and the assumption is
retrieval will be done with Yantor's array nodes if needed

this file also serves as a lib file for maya-specific conversions
to tectonic types - didn't seem too necessary to add another one

*/

#include "tectonicNode.h"

using namespace ed;
//using namespace std;

//MTypeId TectonicNode::kNODE_ID(0x00122C08);
MTypeId TectonicNode::kNODE_ID(0x00000000);
MString TectonicNode::kNODE_NAME( "tectonic" );


// mesh to process as tectonics
MObject TectonicNode::aInMesh;

// base neutral mesh - used for building system and querying
// actually lets this solver act as deformer
MObject TectonicNode::aBaseMesh;

// uv set name to use
MObject TectonicNode::aUVSet;

// bind signal - realistically live should not be used here
MObject TectonicNode::aBind;
// how to split the maya mesh to Tectonic plates 
MObject TectonicNode::aSplitMode;

// mode - either off, frame dependent or real time
MObject TectonicNode::aPlaybackMode;

// connections to Constraint nodes 
MObject TectonicNode::aConstraints;

// time and dynamic attributes
MObject TectonicNode::aGlobalIterations;
MObject TectonicNode::aTimeStep;
MObject TectonicNode::aTime;
MObject TectonicNode::aPrevTime;
MObject TectonicNode::aResetFrame;

/* Query attributes, each takes vector query position
* returns the index and matrix of closest plate
*/
MObject TectonicNode::aQueryPlates;
	MObject TectonicNode::aQueryPos;
		MObject TectonicNode::aQueryPosX;
		MObject TectonicNode::aQueryPosY;
		MObject TectonicNode::aQueryPosZ;
	MObject TectonicNode::aQueryIndex;
	MObject TectonicNode::aQueryResultMatrix;


// outputs
// output mesh
MObject TectonicNode::aOutMesh;
// output cut mesh (very cool)
MObject TectonicNode::aOutCutMesh;
// matrix array attribute
MObject TectonicNode::aOutMatrices;
MObject TectonicNode::aComplete;

std::vector<MObject> TectonicNode::driverMObjects;
std::vector<MObject> TectonicNode::drivenMObjects;


////////////// begin actual code ////////////////
#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\




MStatus TectonicNode::initialize()
{
	MStatus s = MS::kSuccess;

	// initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;
	MFnCompoundAttribute cFn;
	MFnEnumAttribute eFn;
	MFnAttribute aFn;
	MFnMessageAttribute mFn;

	MFnMeshData mData;
	MObject defaultMesh = mData.create(&s);
	MCHECK(s, "could not create default mesh object")

	// mesh inputs
	//aInMesh = tFn.create("inMesh", "inMesh", MFnData::kMesh, defaultMesh);
	aInMesh = tFn.create("inMesh", "inMesh", MFnData::kMesh);
	defaultMesh = mData.create();
	//aBaseMesh = tFn.create("baseMesh", "baseMesh", MFnData::kMesh, defaultMesh);
	aBaseMesh = tFn.create("baseMesh", "baseMesh", MFnData::kMesh);

	// string data for uv set
	MFnStringData sFn;
	MObject defaultString;
	defaultString = sFn.create("map1");
	aUVSet = tFn.create("uvSet", "uvSet", MFnData::kString, defaultString);

	aBind = makeEnumAttr<BindState>("bind");
	aSplitMode = makeEnumAttr<SplitMode>("splitMode", SplitMode::uvShell);
	//aMode = makeEnumAttr<LiveState>("mode");
	aPlaybackMode = makeEnumAttr<LiveState>("mode", LiveState::realtime);

	// constraint array
	aConstraints = nFn.create("constraint", "constraints",
		MFnNumericData::kBoolean);
	nFn.setArray(true);
	nFn.setUsesArrayDataBuilder(true);

	// dynamics attributes
	// global solver iterations
	aGlobalIterations = nFn.create("iterations", "iterations",
		MFnNumericData::kInt, 1);
	nFn.setKeyable(true);
	// scale timestep by this value
	aTimeStep = nFn.create("timeStep", "timeStep",
		MFnNumericData::kFloat, 1.0);
	nFn.setKeyable(true);

	//// tracks current time during playback
	aTime = uFn.create("time", "time", MFnUnitAttribute::kTime, 1.0);
	tFn.setReadable(true);
	tFn.setWritable(true);

	//// previous time value, stored to figure out timestep
	aPrevTime = uFn.create("prevTime", "prevTime", MFnUnitAttribute::kTime, 0.0);
	tFn.setReadable(true);
	tFn.setWritable(false);

	// frame at which to reset
	aResetFrame = uFn.create("resetFrame", "resetFrame", MFnUnitAttribute::kTime, 1.0);
	uFn.setKeyable(true);

	// query attributes
	aQueryPlates = cFn.create("queryPlates", "queryPlates");
	aQueryPos = makeXYZVectorAttr("queryPos", 
		aQueryPos, aQueryPosX, aQueryPosY, aQueryPosZ, 
		false);
	aQueryIndex = nFn.create("queryIndex", "index", MFnNumericData::kInt);
	aQueryResultMatrix = tFn.create("queryResultMatrix", "matrix", MFnMatrixData::kMatrix);

	cFn.addChild(aQueryPos);
	cFn.addChild(aQueryIndex);
	cFn.addChild(aQueryResultMatrix);

	cFn.setArray(true);
	cFn.setReadable(true);
	cFn.setWritable(true);


	//// outputs
	defaultMesh = mData.create();
	//aOutMesh = tFn.create("outMesh", "outMesh", MFnData::kMesh, defaultMesh);
	aOutMesh = tFn.create("outMesh", "outMesh", MFnData::kMesh);
	defaultMesh = mData.create();
	//aOutCutMesh = tFn.create("outCutMesh", "outCutMesh", MFnData::kMesh, defaultMesh);
	aOutCutMesh = tFn.create("outCutMesh", "outCutMesh", MFnData::kMesh);
	aOutMatrices = tFn.create("outMatrices", "outMatrices",
		MFnData::kMatrixArray);

	aComplete = nFn.create("complete", "complete", MFnNumericData::kInt);

	// populate static object vectors

	driverMObjects = {
		aInMesh, aBaseMesh,
		aUVSet, aBind, aSplitMode, aPlaybackMode,
		aConstraints,
		aGlobalIterations, aTime, aPrevTime, aResetFrame, aTimeStep,

		aQueryPlates, aQueryIndex, aQueryPos, aQueryResultMatrix,

	};

	drivenMObjects = {
	aOutMesh, aOutCutMesh, aOutMatrices, aComplete,
	};

	addAttributes<TectonicNode>(driverMObjects);
	addAttributes<TectonicNode>(drivenMObjects);

	setAttributesAffect<TectonicNode>(driverMObjects, drivenMObjects);

	return MStatus::kSuccess;
}



// topo split mode
std::vector<TecPlate> tecMeshFromMFnMeshTopo(
	HalfEdgeMesh& hedgeMesh, MFnMesh& mfn, MStatus &s) {
	DEBUGSL("tecMeshFromTopo")
	std::vector<TecPlate> plates;
	/*pseudocode*/


	return plates;
}

// uv split mode
std::vector<TecPlate> tecMeshFromMFnMeshUV(
	HalfEdgeMesh& hedgeMesh, MFnMesh& meshFn,
	const char* uvSetName,
	MStatus& s) {
	DEBUGSL("tecMeshFromUV")
	std::vector<TecPlate> plates;
	/*pseudocode
	assume conversion to triangle mesh for easier adjacency
	convert main mesh nFaces to uv mesh nFaces
	*/

	auto pfvMap = pointFaceVertexMap(meshFn);

	// get uv index map for main mesh
	std::vector<int> faceVertexUVs = faceVertexUVIds(
		meshFn, uvSetName
	);


	//// mesh data for spatial mesh
	Eigen::MatrixXd coordMat = coordMatFromMFnMesh(meshFn, s);
	Eigen::MatrixXi triIndexMat = triVertexMatFromMFnMesh(meshFn, s);
	Eigen::SparseMatrix<int> triMeshAdjMat;
	igl::adjacency_matrix(triIndexMat, triMeshAdjMat);

	//// get tri data
	MIntArray triPerFaceCounts;
	MIntArray triPoints;
	std::vector<int> triFaceMap;
	std::vector<int> triFaceVertexMap;
	triMeshData(
		meshFn,
		pfvMap,
		triPerFaceCounts,
		triPoints,
		triFaceMap,
		triFaceVertexMap);

	//// update tri face vertex map to tri uv index map
	std::vector<int> triUVMap(triFaceVertexMap.size(), -1);
	// may not need a whole separate vector for this

	for (int i = 0; i < INT(triFaceVertexMap.size()); i++)
	{ // find uv id for each tri face vertex on original mesh
		triUVMap[i] = faceVertexUVs[triFaceVertexMap[i]];
	}

	int nTris = INT(triUVMap.size()) / 3;

	// mesh data for uv mesh
	//Eigen::MatrixXi triUVIndexMat = triVertexMatFromVector(triUVMap);
	Eigen::Map<Eigen::MatrixXi> triUVIndexMat(&triUVMap[0], nTris, 3);
	Eigen::SparseMatrix<int> triUVAdjMat;
	igl::adjacency_matrix(triUVIndexMat, triUVAdjMat);

	//// islands
	Eigen::VectorXi islandComponents; 
	Eigen::VectorXi islandSizes;
	igl::connected_components(triUVAdjMat, islandComponents, islandSizes);

	DEBUGS("tecMeshFromUV islands");
		DEBUGS("components:");
	EIGEN_DEBUG_VAR(islandComponents);
	DEBUGS("sizes:");
	EIGEN_DEBUG_VAR(islandSizes);

	

	//DEBUGVI(islandComponents.array());

	//std::vector<int> visited(hedgeMesh.nFaces, 0);
	////std::set<int> toVisit(visited.begin(), visited.end());
	//std::set<int> toVisit;
	//for (int i = 0; i < hedgeMesh.nFaces; i++) {
	//	toVisit.insert(i);
	//}
	//int islandIndex = 0;
	//while (toVisit.size()) {
	//	int seedFace = *toVisit.begin();
	//	toVisit.erase(toVisit.begin());

	//	
	//}

	


	return plates;

}



MStatus TectonicNode::bind(MDataBlock& data, MStatus& s) {
	/* run bind operations for Tectonic node
	* build tectonic plates from mesh, sync with constraints etc
	*/
	DEBUGSL("Tectonic bind")

	// extract params
	// doubling up some because it's more readable to have in one place
	//int splitModeVal = data.inputValue(aSplitMode).asInt();

	int splitModeVal = data.inputValue(aSplitMode).asShort();
	DEBUGS("SplitModeVal " << splitModeVal);
	MObject meshObj = data.inputValue(aInMesh).asMesh();
	if (meshObj.isNull()) {
		DEBUGS("input mesh object invalid, returning");
		return MS::kFailure;
	}
	MFnMesh meshFn(meshObj, &s);
	MCHECK(s, "cannot initialise meshFn on meshObj");

	MObject baseMeshObj = data.inputValue(aBaseMesh).asMesh();
	MFnMesh baseMeshFn(baseMeshObj, &s);
	MCHECK(s, "cannot initialise meshFn on baseMeshObj");

	MString uvSetName = data.inputValue(aUVSet).asString();

	DEBUGS("bind attributes extracted")

	//HalfEdgeMesh hedgeMesh;
	HalfEdgeMeshFromMObject(hedgeMesh, meshFn, 1);

	DEBUGS("built half edge mesh")


	// build plates
	std::vector<TecPlate> plates;
	switch (splitModeVal) {
	case SplitMode::polygonShell : // only one supported for now
		plates = tecMeshFromMFnMeshTopo(hedgeMesh, meshFn, s);
		break;
	case SplitMode::uvShell:
		plates = tecMeshFromMFnMeshUV(hedgeMesh, meshFn, uvSetName.asChar(), s);
		break;
	}
	MCHECK(s, "error in building tecMeshFromMFnMesh")

	return s;
}

MStatus TectonicNode::compute(
				const MPlug& plug, MDataBlock& data) {

	MStatus s;

	// check for 
	if (data.isClean(plug)) {
		return MS::kSuccess;}

	DEBUGSL("tectonic compute");
	MObject meshObj = data.inputValue(aInMesh).asMesh();
	MFnMesh meshFn(meshObj);
	if (meshObj.isNull()) {
		DEBUGS("input mesh object invalid, returning");
	}
	MObject baseMeshObj = data.inputValue(aInMesh).asMesh();
	MFnMesh baseMeshFn(baseMeshObj);
	if (baseMeshObj.isNull()) {
		DEBUGS("input base mesh object invalid, returning");
	}

	// extract data values
	//int bindVal = data.inputValue(aBind).asInt();
	//int bindVal = data.inputValue(aBind).asLong();
	int bindVal = data.inputValue(aBind).asShort();
	DEBUGS("bind val" << bindVal);

	// check bind status
	switch (bindVal)
	{
	case BindState::off : // do literally nothing
		DEBUGS("bind off");
		data.setClean(plug);
		return MS::kSuccess;
		break;
	case BindState::bind: // bind and set to bound, then continue
		//DEBUGS("bind on, running bind");
		bind(data, s);
		MCHECK(s, "immediate error in tectonic bind, aborting");
		//DEBUGS("setting bound value" << BindState::bound);
		//data.outputValue(aBind).setInt(BindState::bound);
		data.outputValue(aBind).setShort(BindState::bound);
		//data.outputValue(aBind).setInt(static_cast<int>(BindState::bound));
		//data.outputValue(aBind).setInt(2);
		DEBUGS("set bound plug");
		break;
	case BindState::bound: // continue
		//DEBUGS("bind bound");
		break;
	case BindState::live: // bind and continue, leave on live
		//DEBUGS("bind live");
		bind(data, s);
		break;
	}
	MCHECK(s, "error in tectonic bind, aborting");



	// realtime switch
	//currentTime = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
	//int dt = currentTime - prevTime;
	//prevTime = currentTime;

	// set clean all plugs after computation
	//DEBUGS("set plugs clean")
	for (MObject obj : drivenMObjects) {
		data.setClean(obj);	}
	//DEBUGS("driven set clean")
	for (MObject obj : driverMObjects) {
		MFnDependencyNode thisFn(thisMObject());
		MPlug cleanPlug = thisFn.findPlug(obj, true, &s);
		if (s != MS::kSuccess) {
			DEBUGS("could not extract plug, skipping");
			continue; }
		data.setClean(obj);

	}
	//DEBUGS("set drivers clean")

	DEBUGS("set plug " << plug.name() << " clean");
	data.setClean(plug);


    return MS::kSuccess;
}


MStatus TectonicNode::syncConnections(MStatus& s) {
	// ensures node's pointers are synced to connected elements
	return s;
}

MStatus TectonicNode::connectionMade(
	const MPlug& plug, const MPlug& otherPlug, bool asSrc) {

	//// we only care about sink plug
	//if (plug.attribute() != aConstraints) {
	//	return MPxNode::connectionMade(plug, otherPlug, asSrc);
	//}
	//MStatus s;
	////s = syncConnections(s);
	//MCHECK(s, "failure in tecNode connectionMade")
	////getConnectedConstraints();
	return MPxNode::connectionMade(plug, otherPlug, asSrc);
}

MStatus TectonicNode::connectionBroken(
	const MPlug& plug, const MPlug& otherPlug, bool asSrc) {

	//if (plug.attribute() != aConstraints) {
	//	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
	//}
	//MStatus s;
	////s = syncConnections(s);
	//MCHECK(s, "failure in tecNode connectionBroken")
	////getConnectedConstraints();
	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
}


void* TectonicNode::creator(){

    return new TectonicNode;

}

void TectonicNode::postConstructor() {
	this->setExistWithoutInConnections(true);
	this->setExistWithoutOutConnections(true);

	MStatus stat;
	MFnDependencyNode thisFn(thisMObject(), &stat);
	//MCHECK(stat, "tecNode postConstructor unable to initialise thisFn");
	if (stat != MStatus::kSuccess) {
		DEBUGSL("tecNode postConstructor unable to initialise thisFn");
		return;
	}


	// connect time1 by default
	MSelectionList sel;
	sel.add("time1");
	MObject timeObj;
	sel.getDependNode(0, timeObj);
	MFnDependencyNode timeFn(timeObj);

	MPlug outTimePlug = timeFn.findPlug("outTime", true, &stat);
	if (stat != MStatus::kSuccess) {
		DEBUGSL("tecNode postConstructor unable to find time1.outTime");
		return;
	}
	MPlug inTimePlug = thisFn.findPlug(aTime, true, &stat);
	if (stat != MStatus::kSuccess) {
		DEBUGSL("tecNode postConstructor unable to find this node aTime plug");
		return;
	}
	MDGModifier dgMod;
	dgMod.connect(outTimePlug, inTimePlug);
	stat = dgMod.doIt();
	if (stat != MStatus::kSuccess) {
		DEBUGSL("tecNode postConstructor unable to make outTime connection");
		return;
	}
}

TectonicNode::TectonicNode() {};
TectonicNode::~TectonicNode() {};
