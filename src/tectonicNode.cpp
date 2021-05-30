

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

MTypeId TectonicNode::kNODE_ID(0x00122C08);
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
MObject TectonicNode::aMode;

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
MObject TectonicNode::aQueryIndex;
MObject TectonicNode::aQueryMatrix;



// outputs
// output mesh
MObject TectonicNode::aOutMesh;
// output cut mesh (very cool)
MObject TectonicNode::aOutCutMesh;
// matrix array attribute
MObject TectonicNode::aOutMatrices;


////////////// begin actual code ////////////////
#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\

// splitMode enum
BETTER_ENUM(SplitMode, int,
	polygonShell, // raw poly connectivity, superposed vertices mean welds
	uvShell // uv connectivity, uv islands are plates
)

// topo split mode
std::vector<TecPlate> tecMeshFromMFnMeshTopo(
	HalfEdgeMesh& hedgeMesh, MFnMesh& mfn, MStatus &s) {
	std::vector<TecPlate> plates;
	/*pseudocode*/


	return plates;
}

// uv split mode
std::vector<TecPlate> tecMeshFromMFnMeshUV(
	HalfEdgeMesh& hedgeMesh, MFnMesh& meshFn, MStatus& s) {
	std::vector<TecPlate> plates;
	/*pseudocode
	assume conversion to triangle mesh for easier adjacency
	convert main mesh nFaces to uv mesh nFaces
	*/

	auto pfvMap = pointFaceVertexMap(meshFn);

	// mesh data for spatial mesh
	Eigen::MatrixXi coordMat = coordMatFromMFnMesh(meshFn, s);
	Eigen::MatrixXi triIndexMat = faceVertexMatFromMFnMesh(meshFn, s);
	Eigen::SparseMatrix<int> triMeshAdjMat;
	igl::adjacency_matrix(triIndexMat, triMeshAdjMat);

	// get tri data
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


	// mesh data for uv mesh


	// from end

	// map of [tri index : face index]

	// map of [tri index : orig face vertex indices]

	// need triangle UV map of {tri index : [uvIndices]}


	Eigen::MatrixXi triUVIndexMat = faceVertexMatFrom
	Eigen::SparseMatrix<int> triUVAdjMat;
	igl::adjacency_matrix(triUVIndexMat, triUVAdjMat);

	


	// islands
	std::vector<int> visited(hedgeMesh.nFaces, 0);
	//std::set<int> toVisit(visited.begin(), visited.end());
	std::set<int> toVisit;
	for (int i = 0; i < hedgeMesh.nFaces; i++) {
		toVisit.insert(i);
	}
	int islandIndex = 0;
	while (toVisit.size()) {
		int seedFace = *toVisit.begin();
		toVisit.erase(toVisit.begin());

		
	}

	


	return plates;

}


MStatus TectonicNode::initialize()
{
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
	MObject defaultMesh = mData.create();

	// mesh inputs
	aInMesh = tFn.create("inMesh", "inMesh", MFnData::kMesh, defaultMesh);
	defaultMesh = mData.create();
	aBaseMesh = tFn.create("inMesh", "inMesh", MFnData::kMesh, defaultMesh);

	// string data for uv set
	MFnStringData sFn;
	MObject defaultString;
	defaultString = sFn.create("map1");
	aUVSet = tFn.create("uvSet", "uvSet", MFnData::kString, defaultString);

	aBind = makeEnumAttr<BindState>("bind");
	aSplitMode = makeEnumAttr<SplitMode>("splitMode");
	aMode = makeEnumAttr<LiveState>("mode");

	// constraint array
	aConstraints = nFn.create("constraints", "constraints",
		MFnNumericData::kBoolean);
	nFn.setArray(true);
	nFn.setUsesArrayDataBuilder(true);

	// dynamics attributes
	// global solver iterations
	aGlobalIterations = nFn.create("iterations", "iterations",
		MFnNumericData::kInt, 1);
	// scale timestep by this value
	aTimeStep = nFn.create("iterations", "iterations",
		MFnNumericData::kFloat, 1.0);

	// tracks current time during playback
	aTime = uFn.create("time", "time", MFnUnitAttribute::kTime, 0.0);
	tFn.setReadable(true);
	tFn.setWritable(true);

	// previous time value, stored to figure out timestep
	aPrevTime = uFn.create("prevTime", "prevTime", MFnUnitAttribute::kTime, 0.0);
	tFn.setReadable(true);
	tFn.setWritable(false);

	// attribute used to update cell - time is most convenient
	aResetFrame = uFn.create("resetFrame", "resetFrame", MFnUnitAttribute::kTime, 1.0);

	// query attributes
	aQueryPlates = cFn.create("queryPlates", "queryPlates");
		aQueryPos = nFn.create("pos", "pos", MFnNumericData::k3Double);
		aQueryIndex = nFn.create("index", "index", MFnNumericData::k3Double);
		aQueryMatrix = tFn.create("matrix", "matrix", MFnMatrixData::kMatrix);

	cFn.addChild(aQueryPos);
	cFn.addChild(aQueryIndex);
	cFn.addChild(aQueryMatrix);


	// outputs
	defaultMesh = mData.create();
	aOutMesh = tFn.create("outMesh", "outMesh", MFnData::kMesh, defaultMesh);
	defaultMesh = mData.create();
	aOutCutMesh = tFn.create("outCutMesh", "outCutMesh", MFnData::kMesh, defaultMesh);
	aOutMatrices = tFn.create("outMatrices", "outMatrices",
		MFnData::kMatrixArray);


	std::vector<MObject> drivers = {
		//aInMesh,
		aInMesh, aBind, aMode, aConstraints, aGlobalIterations,
		aUVSet,
	};
	std::vector<MObject> driven = {
		aOutMesh
	};

	addAttributes<TectonicNode>(drivers);
	addAttributes<TectonicNode>(driven);

	setAttributesAffect<TectonicNode>(drivers, driven);

    return MStatus::kSuccess;
}


MStatus TectonicNode::bind(MDataBlock& data, MStatus& s) {
	/* run bind operations for Tectonic node
	* build tectonic plates from mesh, sync with constraints etc
	*/

	// extract params
	// doubling up some because it's more readable to have in one place
	int splitModeVal = data.inputValue(aSplitMode).asInt();
	MObject meshObj = data.inputValue(aInMesh).asMesh();
	MFnMesh meshFn(meshObj);

	//HalfEdgeMesh hedgeMesh;
	HalfEdgeMeshFromMObject(hedgeMesh, meshFn, 1);


	// build plates
	std::vector<TecPlate> plates;
	switch (splitModeVal) {
	case SplitMode::polygonShell : // only one supported for now
		plates = tecMeshFromMFnMeshTopo(hedgeMesh, meshFn, s);
		break;
	case SplitMode::uvShell:
		plates = tecMeshFromMFnMeshUV(hedgeMesh, meshFn, s);
		break;
	}
	MCHECK(s, "error in building tecMeshFromMFnMesh")

	return s;
}

MStatus TectonicNode::compute(
				const MPlug& plug, MDataBlock& data) {

	MStatus s;

	// extract data values
	int bindVal = data.inputValue(aBind).asInt();

	// check bind status
	switch (bindVal)
	{
	case BindState::off : // do literally nothing
		data.setClean(plug);
		return MS::kSuccess;
		break;
	case BindState::bind: // bind and set to bound, then continue
		bind(data, s);
		data.outputValue(aBind).setInt(BindState::bound);
		break;
	case BindState::bound: // continue
		break;
	case BindState::live: // bind and continue, leave on live
		bind(data, s);
		break;
	}



	// realtime switch
	currentTime = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
	int dt = currentTime - prevTime;
	prevTime = currentTime;

	data.setClean(plug);


    return MS::kSuccess;
}


MStatus TectonicNode::syncConnections(MStatus& s) {
	// ensures node's pointers are synced to connected elements
	return s;
}

MStatus TectonicNode::connectionMade(
	const MPlug& plug, const MPlug& otherPlug, bool asSrc) {

	// we only care about sink plug
	if (plug.attribute() != aConstraints) {
		return MPxNode::connectionMade(plug, otherPlug, asSrc);
	}
	MStatus s;
	s = syncConnections(s);
	MCHECK(s, "failure in tecNode connectionMade")
	//getConnectedConstraints();
	return MPxNode::connectionMade(plug, otherPlug, asSrc);
}

MStatus TectonicNode::connectionBroken(
	const MPlug& plug, const MPlug& otherPlug, bool asSrc) {

	if (plug.attribute() != aConstraints) {
		return MPxNode::connectionBroken(plug, otherPlug, asSrc);
	}
	MStatus s;
	s = syncConnections(s);
	MCHECK(s, "failure in tecNode connectionBroken")
	//getConnectedConstraints();
	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
}


void* TectonicNode::creator(){

    return new TectonicNode;

}

void TectonicNode::postConstructor() {
	this->setExistWithoutInConnections(true);
	this->setExistWithoutOutConnections(true);

	MStatus stat;
	// connect time1 by default
	MSelectionList sel;
	sel.add("time1");
	MFnDependencyNode thisFn(thisMObject());
	MObject timeObj;
	sel.getDependNode(0, timeObj);
	MFnDependencyNode timeFn(timeObj);

	MPlug outTimePlug = timeFn.findPlug("outTime");
	MPlug inTimePlug = thisFn.findPlug("time");
	MDGModifier dgMod;
	dgMod.connect(outTimePlug, inTimePlug);
	dgMod.doIt();
}

TectonicNode::TectonicNode() {};
TectonicNode::~TectonicNode() {};
