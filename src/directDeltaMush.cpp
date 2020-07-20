

/*

	no copyright or breach of contract intended,
	it's just much more efficient to test this when I already have the environment set up
	will delete once project is concluded

*/

#include "directDeltaMush.h"

using namespace std;
using namespace ed;

MTypeId DirectDeltaMush::kNODE_ID(0x00122C1D);
MString DirectDeltaMush::kNODE_NAME( "directDeltaMush" );

MObject DirectDeltaMush::aBind;
MObject DirectDeltaMush::aBindSkinWeights;

MObject DirectDeltaMush::aTransSmooth;
MObject DirectDeltaMush::aRotSmooth;
MObject DirectDeltaMush::aAlpha;
MObject DirectDeltaMush::aIterations;


MObject DirectDeltaMush::aVertexWeightIndices;
MObject DirectDeltaMush::aVertexWeightValues;
MObject DirectDeltaMush::aVertexWeightOffsets;


/*
objects inherited from deformer, skincluster etc

MObject input - input attribute, array
MObject inputGeom - input geometry attribute
MObject outputGeom - output geometry attribute
MObject envelope - envelope attribute

MObject weightList
MObject weights
MObject matrix - input matrix array
MObject bindPreMatrix - input prematrix array

ALSO INHERIT ALL ATTRIBUTES FROM NORMAL SKINCLUSTER enjoy that node bloat lol

*/



MStatus DirectDeltaMush::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;

	MStatus status;

	aBind = makeBindAttr("bind");
	addAttribute(aBind);

	aBindSkinWeights = makeBindAttr("bindSkinWeights");
	addAttribute(aBindSkinWeights);

	// deltamush attributes
	aTransSmooth = nFn.create("smoothTranslation", "smoothTranslation", MFnNumericData::kDouble, 1.0, &status);
	nFn.setMin(0.0);
	nFn.setStorable(true);
	nFn.setKeyable(true);
	addAttribute(aTransSmooth);

	aRotSmooth = nFn.create("smoothRotation", "smoothRotation", MFnNumericData::kDouble, 1.0, &status);
	nFn.setMin(0.0);
	nFn.setStorable(true);
	nFn.setKeyable(true);
	addAttribute(aRotSmooth);

	aAlpha = nFn.create("alpha", "alpha", MFnNumericData::kDouble);
	nFn.setMin(0.0);
	nFn.setMax(1.0);
	nFn.setStorable(true);
	nFn.setKeyable(true);
	addAttribute(aAlpha);

	aIterations = nFn.create("iterations", "iterations", MFnNumericData::kInt, 1);
	nFn.setMin(0);
	nFn.setStorable(true);
	nFn.setKeyable(true);
	addAttribute(aIterations);

	// vertex weight buffers
	// main skin matrix weights
	aVertexWeightOffsets = tFn.create("vertexWeightOffsets", "vertexWeightOffsets", MFnData::kIntArray);
	addAttribute(aVertexWeightOffsets);

	aVertexWeightIndices = tFn.create("vertexWeightIndices", "vertexWeightIndices", MFnData::kIntArray);
	addAttribute(aVertexWeightIndices);

	aVertexWeightValues = tFn.create("vertexWeightValues", "vertexWeightValues", MFnData::kIntArray);
	addAttribute(aVertexWeightValues);

	// attribute affects
	// crude for now
	vector<MObject> drivers = {
		aVertexWeightIndices, aVertexWeightOffsets, aVertexWeightValues,
		aBind, aBindSkinWeights,
		matrix, bindPreMatrix, weights, weightList,
		inputGeom, envelope,
		aRotSmooth, aTransSmooth, aAlpha, aIterations
	};

	vector<MObject> driven = {
		outputGeom
	};

	setAttributesAffect<DirectDeltaMush>(drivers, driven);

    return status;
}


OffsetBuffer<int> faceBufferFromMfnMesh(MFnMesh& mfn) {
	// construct face buffer from mfn info
	int nPolys = mfn.numPolygons();
	vector<int> facePointConnects;
	vector<int> facePointOffsets(nPolys, -1);

	// vertex buffer from mfn
	MIntArray vertexCount; // number of vertices in face - NOT global offsets
	MIntArray vertexList; // list of vertices in each face
	mfn.getVertices(vertexCount, vertexList);
	vector<int> testv = MIntArrayToVector(vertexCount);
	//DEBUGS("vertexCount");
	//DEBUGVI(testv);

	int offsetIndex = 0;
	for (unsigned int i = 0; i < vertexCount.length(); i++) {

		// add offset to current index
		facePointOffsets[i] = offsetIndex;
		int nFacePoints = vertexCount[i];

		for (int n = 0; n < nFacePoints; n++) {
			facePointConnects.push_back(vertexList[offsetIndex]);
			offsetIndex++;
		}
	}
	return OffsetBuffer<int>(facePointConnects, facePointOffsets);
}

void HalfEdgeMeshFromMObject(HalfEdgeMesh& hedgeMesh, MObject meshObj, int build) {
	// updates target mesh struct from mesh MObject
	// if build, will rebuild topology buffers
	// if not, will only copy point positions
	//DEBUGS("ddm hedgeMesh from mobject")
	MStatus s = MS::kSuccess;
	MFnMesh meshFn(meshObj);

	int nPoints = meshFn.numVertices();
	int nPolys = meshFn.numPolygons();
	hedgeMesh.nPoints = nPoints;
	hedgeMesh.nFaces = nPolys;

	if (build > 0) {

		//DEBUGS("building hedgemesh");
		OffsetBuffer<int> faceBuffer = faceBufferFromMfnMesh(meshFn);
		//OffsetBuffer<int> faceBuffer(allFaceVertices, faceVertexOffsets);
		//DEBUGS("faceBuffer built");
		//DEBUGVI(faceBuffer.offsets);
		//DEBUGVI(faceBuffer.values);
		OffsetBuffer<int> pointBuffer = pointBufferFromFaceBuffer(
			faceBuffer);

		hedgeMesh.build(
			pointBuffer.values, pointBuffer.offsets,
			faceBuffer.values, faceBuffer.offsets
		);
	}

	// set mesh point positions
	//DEBUGS("set positions");
	const float* rawPositions = meshFn.getRawPoints(&s);
	//meshFn.getRawPoints(&s);
	//float test = rawPositions[7];
	vector<float> posVector(nPoints * 3, 0.0);
	for (int i = 0; i < nPoints; i++) {
		posVector[i*3] = rawPositions[i*3];
		posVector[i*3 + 1] = rawPositions[i*3 + 1];
		posVector[i*3 + 2] = rawPositions[i*3 + 2];

	}


	//DEBUGVI(posVector);
	hedgeMesh.setPositions(posVector);
}

/* traverse basic skincluster weight system to build more efficient weight
buffers, then transfer those into array attributes
refresher:
	weightList[ vtxId ].weights[ influenceIndex ] = influence weight
*/
void extractSkinWeights(MArrayDataHandle& weightRoot, SkinData& skinInfo, int nPoints) {
	// extracts skin data to buffers
	// restricted to nPoints
	vector<int> vertexOffsets(nPoints);
	vector<int> influenceIndices;
	vector<float> influenceWeights;
	// eps value to discount negligible influences
	static const float e = 0.001f;


	int offsetIndex = 0;
	int entryCreated = 0;
	for (unsigned int i = 0; i < nPoints; i++) {
		entryCreated = 0;
		vertexOffsets[i] = offsetIndex;
		jumpToElement(weightRoot, i);
		MArrayDataHandle vtxEntry = weightRoot.inputValue().child(
			DirectDeltaMush::weights);
		unsigned int nWeights = vtxEntry.elementCount();
		
		for (unsigned int n = 0; n < nWeights; n++) {

			float weightVal = vtxEntry.inputValue().asDouble();

			if (weightVal > e) {
				influenceIndices.push_back(vtxEntry.elementIndex());
				influenceWeights.push_back(weightVal);
				offsetIndex++;
				entryCreated = 1;
			}
			vtxEntry.next();
		}

		// no entry for vertex created yet
		if (nWeights == 0 || entryCreated == 0) {
			if (nWeights == 0) {
				//DEBUGS("nWeights zero for vertex " << i);
			}
			if (entryCreated == 0) {
				//DEBUGS("no entry created for vertex " << i);
			}
			
			influenceIndices.push_back(-1); // 
			influenceWeights.push_back(0.0);
			offsetIndex++;
			continue;
		}
		
	}

	skinInfo.vertexOffsets = vertexOffsets;
	skinInfo.influenceIndices = influenceIndices;
	skinInfo.influenceWeights = influenceWeights;

}


void DirectDeltaMush::runBind(MDataBlock& data, const MObject& meshObj) {
	// precompute node
	// first rebuild topological struct
	// DEBUGS("ddm runBind");
	HalfEdgeMeshFromMObject(*hedgeMesh, meshObj, 1);

	// still having serious trouble
	DEBUGS("begin precompute")
	precompute(data);

	//DEBUGS("skinIndices");
	//DEBUGVI(skinInfo.influenceIndices)
}


MMatrixArray extractMMatrixArray(MArrayDataHandle& matArray) {
	//DEBUGS("ddm extract mmatrices");
		// extract weight matrices from array
	// simple, right?

		MMatrixArray result(matArray.elementCount());
		for (unsigned int i = 0; i < matArray.elementCount(); i++) {
			result[i] = MFnMatrixData(matArray.inputValue().data()).matrix();
			// guess again
			matArray.next();
		}
		return result;
	}

void DirectDeltaMush::setOutputGeo(MDataBlock& data, const MObject& meshGeo) {
	// sets output plug to target mesh object
	// we deform only one piece of geometry
	//DEBUGS("setOutputGeo")
	MArrayDataHandle outputArray = data.outputArrayValue(outputGeom);
	jumpToElement(outputArray, 0);
	MDataHandle outputGeoHandle = outputArray.outputValue();
	outputGeoHandle.setMObject(meshGeo);
}

void DirectDeltaMush::deformGeo(HalfEdgeMesh& mesh,
	const DeformerParametres& params,
	vector<double> &outputPositions) {
	// kicks off full deformation process

	for (int i = 0; i < mesh.nPoints; i++) {
		deformPoint(mesh, params, outputPositions, i);
	}
}

void DirectDeltaMush::deformPoint(
	HalfEdgeMesh & mesh,
	const DeformerParametres &params,
	vector<double> &outPositions, int index) {
	// deform single vertex

	SmallList<float> baseList = mesh.pointPositions.entry(index);

	MPoint basePos(
		baseList[0], baseList[1], baseList[2], 1.0);

	// get skin indices and weights
	SmallList<int> weightIndices = entryFromBuffer(
		params.skinData.influenceIndices, params.skinData.vertexOffsets, index);
	SmallList<float> weightValues = entryFromBuffer(
		params.skinData.influenceWeights, params.skinData.vertexOffsets, index);

	// output position
	MPoint result(basePos);
	MPoint tfPos(0, 0, 0, 1.0);
	//MMatrix diffMat;

	// main deformation
	Mat4 qmat;
	qmat.setZero();

	//for (std::list<std::pair<int, Mat4>> : omegas[index]) {
	//for (std::pair<int, Mat4> e omegas[index]) {
	for (auto e : omegas[index]) { // e is pair<int, mat4>
		Mat4 Mi;
		mmatrix_to_eigen(params.diffMats[e.first], Mi);
		Mat4 tmp = getOmega(index, e.first) * Mi;
		qmat += tmp;
	}

	qmat.transposeInPlace();
	Mat3 Qi = qmat.block(0, 0, 3, 3); // sum rotation
	Vec3 qi = qmat.block(0, 3, 3, 1); // sum transform position
	Vec3 pi = qmat.block(3, 0, 1, 3).transpose(); // ?

	// rotation
	Mat3 M = Qi - (qi * pi.transpose());
	Eigen::JacobiSVD<Mat3> svd;
	svd.compute(M, Eigen::ComputeFullU | Eigen::ComputeFullV);
	Mat3 U = svd.matrixU(); // svd decomposition of difference between base pos and target pos
	Mat3 V = svd.matrixV().transpose();
	Mat3 R = (U * V); // rotation matrix

	// translation
	Vec3 ti = qi - (R * pi);
	Mat4 gamma;
	gamma << R, ti, 0, 0, 0, 1;

	/*Vec4 pt_h( basePos.x)*/

	//MMatrix mGamma = eigen_to_mmatrix(gamma);
	Vec4 ptH(basePos.x, basePos.y, basePos.z, 1);
	Vec4 finalPt = gamma * ptH;
	tfPos = MPoint(finalPt[0], finalPt[1], finalPt[2]);
	
	//tfPos = basePos * mGamma;



	//for (int i = 0; i < weightValues.size(); i++) {
	//	int infIndex = weightIndices[i];
	//	float weightVal = weightValues[i];
	//	if (infIndex == -1) { // no valid transform index
	//		continue;
	//	}



	//	diffMat = params.diffMats[ infIndex ];
	//	tfPos += (basePos * diffMat) * weightVal;

	//}
	//

	// blend output by envelope
	result = basePos * (1 - params.envelope) + tfPos * params.envelope;
	// later use envelope to blend matrices from base to transformed
	// seems at least slightly more useful

	outPositions[index * 3] = result.x;
	outPositions[index * 3 + 1] = result.y;
	outPositions[index * 3 + 2] = result.z;

	//DEBUGS("outPositions");
	//DEBUGVI(outPositions);

}


MStatus DirectDeltaMush::compute(
	const MPlug& plug, MDataBlock& data) {
	//DEBUGS("ddm compute");

	if (plug != outputGeom) {
		DEBUGS("plug is not outputGeom, skipping")
		data.setClean(plug);
		return MS::kSuccess;
	}

	MStatus m;
	MArrayDataHandle inputArray = data.outputArrayValue(input);
	jumpToElement(inputArray, 0);
	MDataHandle geoHandle = (inputArray.inputValue().child(inputGeom));
	MObject meshObj = geoHandle.asMesh();
	if (meshObj.isNull()) {
		DEBUGS("null mesh obj, skipping compute")
		data.setClean(plug);
		return MS::kSuccess;
	}


	MFnMesh meshFn(meshObj);
	int nPoints = meshFn.numVertices();

	//DEBUGS("nPoints " << nPoints);

	// ensure weight array is correct to avoid crashing
	int weightCount = data.outputArrayValue(weightList).elementCount();
	//DEBUGS("weightCount " << weightCount);



	// check envelope
	float envelopeValue = data.inputValue(envelope).asFloat();
	if (envelopeValue < 0.001 || weightCount == 0) {
		// set output data directly
		setOutputGeo(data, meshObj);
		data.setClean(plug);
		return MS::kSuccess;
	}

	// extract joint matrix arrays
	MMatrixArray transformMatrices = extractMMatrixArray(
		data.inputArrayValue(matrix));
	MMatrixArray bindPreMatrices = extractMMatrixArray(
		data.inputArrayValue(bindPreMatrix));
	deformParams->diffMats.setLength(transformMatrices.length());
	for (int i = 0; i < transformMatrices.length(); i++) {
		deformParams->diffMats[i] = bindPreMatrices[i] * transformMatrices[i];
	}


	deformParams->tfMats = transformMatrices;
	deformParams->refMats = bindPreMatrices;
	deformParams->envelope = envelopeValue;

	deformParams->smoothRotation = data.inputValue(aRotSmooth).asDouble();
	deformParams->smoothTranslation = data.inputValue(aTransSmooth).asDouble();
	deformParams->alpha = data.inputValue(aAlpha).asDouble();
	deformParams->iterations = data.inputValue(aIterations).asInt();

	// binding
	int bindSkin = data.inputValue(aBindSkinWeights).asInt();
	int bind = data.inputValue(aBind).asInt();

	//if (!hedgeMesh->hasBuilt) {
	//	bindSkin = 1;
	//	bind = 1;
	//}

	// bind skin data
	SkinData &skinInfo = deformParams->skinData;

	extractSkinWeights(data.inputArrayValue(weightList), skinInfo, nPoints);

	//if (bindSkin == BindState::bind || bindSkin == BindState::live) { // bind or live
	//	extractSkinWeights(data.inputArrayValue(weightList),
	//		skinInfo, nPoints);
	//	if (bindSkin == BindState::bind) { // set bind attr to bound
	//		data.inputValue(aBindSkinWeights).setInt(BindState::bound);
	//	}
	//}


	// bind mesh data
	if (bind == BindState::bind || bind == BindState::live) { // bind or live
		runBind(data, meshObj);
		if (bind == BindState::bind) { // set bind attr to bound
			data.inputValue(aBind).setInt(BindState::bound);
		}
		DEBUGS("bind complete")
	}

	if (!hedgeMesh->hasBuilt) {
		// don't compute without building mesh
		setOutputGeo(data, meshObj);
		data.setClean(plug);

		return MS::kFailure;
	}


	// deformation call - optimise this
	vector<double> outPositions(meshFn.numVertices()*3, 0.0);
	deformGeo(*hedgeMesh, *deformParams, outPositions);

	//DEBUGS("output")
	//DEBUGVI(outPositions);

	// convert to point array
	MPointArray outputPoints(meshFn.numVertices());
	for (int i = 0; i < meshFn.numVertices(); i++) {
		outputPoints[i] = MPoint(
			outPositions[i * 3],
			outPositions[i * 3 + 1],
			outPositions[i * 3 + 2],
			1.0);
	}

	meshFn.setPoints(outputPoints, MSpace::kObject);

	// set output geometry plug
	setOutputGeo(data, meshObj);
	data.setClean(plug);

	return MS::kSuccess;
}

Mat4 DirectDeltaMush::getOmega(int i, int j)
{
	//Get omega, or if it doesn't exist, return the zero matrix.
	auto l = omegas.at(i);
	for (auto e : l) {
		if (e.first == j)
			return e.second;
	}
	Mat4 z;
	z.setZero();
	return z;
}


MStatus DirectDeltaMush::precompute(MDataBlock& block)
{
	// Time the precomputation stage
	MTimer timer;
	timer.beginTimer();
	MGlobal::displayInfo("Starting Precomputation");

	// Get the geometry
	MArrayDataHandle hInput = block.inputArrayValue(input);
	jumpToElement(hInput, 0);
	MDataHandle hInputGeom = hInput.inputValue().child(inputGeom);
	MObject inmesh = hInputGeom.asMesh();
	MFnMesh mesh(inmesh);
	// Verts
	Eigen::MatrixX3d V; // n by 3 matrix of undeformed vertex positions
	MPointArray verts;
	mesh.getPoints(verts);
	V.resize(verts.length(), Eigen::NoChange);
	for (unsigned int i = 0; i < verts.length(); i++) {
		double vx = verts[i][0];
		double vy = verts[i][1];
		double vz = verts[i][2];
		V.row(i) = Eigen::RowVector3d(vx, vy, vz);
	}
	DEBUGS("vertMat bound")

	// Faces
	MIntArray tricount, triidx;
	Eigen::MatrixX3i F; // n by 3 matrix of undeformed triangle indices
	mesh.getTriangles(tricount, triidx);
	F.resize(triidx.length() / 3, Eigen::NoChange);
	for (unsigned int i = 0; i < triidx.length() / 3; i++) {
		F.row(i) = Eigen::RowVector3i(triidx[(3 * i)], triidx[(3 * i) + 1], triidx[(3 * i) + 2]);
	}
	DEBUGS("faceMat bound");
	DEBUGS("faceMat rows " << F.rows());

	std::ptrdiff_t n = V.rows(); // Number of vertices

	// Transform matrices

	MMatrixArray transforms = deformParams->tfMats;
	int numTransforms = transforms.length();


	//Eigen::VectorXd rotSmoothMap(n);
	//Eigen::VectorXd transSmoothMap(n);

	//MArrayDataHandle hWeightList = block.inputArrayValue(weightList);
	//MArrayDataHandle hRotSmoothMap = block.inputArrayValue(aRotSmoothMap);
	//MArrayDataHandle hTransSmoothMap = block.inputArrayValue(aTransSmoothMap);

	//long ii = 0;
	//for (iter.reset(); !iter.isDone(); iter.next(), ii++) {
	//	MArrayDataHandle weightsHandle = hWeightList.inputValue().child(weights);
	//	for (long widx = 0; widx < numTransforms; widx++) {
	//		jumpToElement(weightsHandle, widx);
	//		double w = weightsHandle.inputValue().asDouble();
	//		if (w != 0 && !isnan(w)) {
	//			W.insert(ii, widx) = w;
	//		}
	//	}
	//	//JumpToElement(hRotSmoothMap, iter.index());
	//	//rotSmoothMap[ii] = hRotSmoothMap.inputValue().asFloat();
	//	//JumpToElement(hTransSmoothMap, iter.index());
	//	//transSmoothMap[ii] = hTransSmoothMap.inputValue().asFloat();

	//	hWeightList.next();
	//}

		// Weights & other maps
	Sparse WeightMat(n, numTransforms);
	WeightMat.reserve(6); // Assuming approximately 4 weights per vertex
	DEBUGS("transforms bound")

	// iterate weights on each vertex, add weight of influence to matrix
	SkinData &skinInfo = deformParams->skinData;
	// for every vertex
	DEBUGS("bind weight matrix")
	DEBUGVI( skinInfo.vertexOffsets)
	for (int i = 0; i < INT(skinInfo.vertexOffsets.size()); i++) {

		SmallList<int> vertIndices = entryFromBuffer(
			skinInfo.influenceIndices, skinInfo.vertexOffsets, i);
		SmallList<float> vertWeights = entryFromBuffer(
			skinInfo.influenceWeights, skinInfo.vertexOffsets, i);

		// for every weight, add to matrix
		for (int n = 0; n < vertIndices.size(); n++) {
			double weightVal = vertWeights[n];
			WeightMat.insert(i, n) = weightVal;
		}
	}
	DEBUGS("weight bind complete")

	// Get precomputation parameters
	double rotSmooth = block.inputValue(aRotSmooth).asDouble();
	double transSmooth = block.inputValue(aTransSmooth).asDouble();

	double dmBlend = block.inputValue(aAlpha).asDouble();

	int steps = block.inputValue(aIterations).asInt();

	//Laplacian matrix
	Sparse lapl;
	igl::cotmatrix(V, F, lapl); // Compute standard laplacian matrix
	MatX lapl_diag_inv = lapl.diagonal().asDiagonal().inverse(); //Normalize
	Sparse L = (lapl * lapl_diag_inv).sparseView().eval(); // Normalized!

	// Vars needed for solver. Solver is used to calculate sparse inverses.
	Sparse I(n, n);
	I.setIdentity();

	Eigen::SparseLU<Sparse> solver_b;
	Eigen::SparseLU<Sparse> solver_c;

	// Implicitly solve.
	// This is a slight deviation from the original paper, which has parameters for
	// steps and the separate translation and rotation smoothing. For an artist, it's
	// easier to think in terms of the total desired amount of smoothing and the
	// number of steps as a precision parameter, rather than having to tune them in
	// tandem for each change.
	Sparse b(I + (transSmooth / (double)steps) * L /*transSmoothMap.asDiagonal()*/);
	Sparse c(I + (rotSmooth / (double)steps) * L /*rotSmoothMap.asDiagonal()*/);

	Sparse B(I);
	Sparse B_next;
	solver_b.compute(b.transpose());
	for (int i = 0; i < steps; i++) {
		B.makeCompressed();
		// This is the slow part
		B_next = solver_b.solve(B);
		B = B_next;
	}

	Sparse C(I);
	Sparse C_next;
	solver_c.compute(c.transpose());
	for (int i = 0; i < steps; i++) {
		C.makeCompressed();
		C_next = solver_c.solve(C);
		C = C_next;
	}

	auto psi = [&B, &V, &WeightMat, n, &transforms](const long i, const long j) -> Mat4 {
		Mat4 sum = Mat4::Zero();
		// Sum psi, a sum over all verts for a given weight.

		for (long k = 0; k < n; k++) {
			double w = WeightMat.coeff(k, j);
			double b = B.coeff(k, i);
			if (w != 0 && b != 0) {
				Vec3 r = V.row(k);
				Vec4 rw(V.coeff(k, 0), V.coeff(k, 1), V.coeff(k, 2), 1);
				Mat4 hh = (rw * rw.transpose());
				Mat4 h = b * w * hh;
				sum += h;
				if (h != h.transpose()) {
					h = h.transpose();
				}
			}
		}
		return sum;
	};

	// Helper lambdas for the precomputation process.

	auto p_i = [&psi, numTransforms](const int ii) -> Vec3 {
		Mat4 sum = Mat4::Zero();
		for (int j = 0; j < numTransforms; j++) {
			sum += psi(ii, j);
		}
		if (sum != sum.transpose()) {
			throw sum;
		}
		return sum.block(0, 3, 3, 1).eval();
	};

	auto w_prime = [&C, &WeightMat, n](const int i, const int j) -> double {
		double sum = 0;
		for (int k = 0; k < n; k++) {
			double w = WeightMat.coeff(k, j);
			double c = C.coeff(k, i);
			sum += w * c;
		}
		return sum;
	};

	auto omega = [dmBlend, &psi, &p_i, &w_prime, &WeightMat](const int i, const int j) -> Mat4 {
		Vec3 p_ii = p_i(i);
		Mat4 pimat;
		pimat << (p_ii * p_ii.transpose()), p_ii, p_ii.transpose(), 1;
		Mat4 psi_ij = psi(i, j);
		return ((1.0 - dmBlend) * (psi_ij)+(dmBlend * w_prime(i, j)) * pimat).eval();
	};

	omegas.clear();
	// Actually precompute omegas.
	for (int ii = 0; ii < n; ii++) {
		//omegas.push_back(std::list<std::pair<int, Mat4>>());
		omegas.push_back(std::vector<std::pair<int, Mat4>>());
		auto& e = omegas.at(ii);
		e.clear();
		for (int jj = 0; jj < numTransforms; jj++) {
			// This could be optimized more by not storing zero matrices
			//if(W.coeff(ii,jj) != 0){
			auto o = omega(ii, jj);
			e.push_back(std::pair<int, Mat4>(jj, o));
			//}
		}
	}

	timer.endTimer();

	std::stringstream ss;
	ss << "Precomputation complete (" << timer.elapsedTime() << ")";
	MGlobal::displayInfo(ss.str().c_str());
	return MS::kSuccess;
}




void* DirectDeltaMush::creator(){

     DirectDeltaMush *node = new DirectDeltaMush;
	 node->hedgeMesh = new HalfEdgeMesh;
	 node->deformParams = new DeformerParametres;

	 return node;

}

DirectDeltaMush::DirectDeltaMush() {};
DirectDeltaMush::~DirectDeltaMush() {};
