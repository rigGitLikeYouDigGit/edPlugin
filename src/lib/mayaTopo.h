
#ifndef _MAYA_TOPO_H
#define _MAYA_TOPO_H 1

#include "topo.h"
#include "api.h"


// conversions between pure topo types and maya types
// it was just easier to manage this way



namespace ed {

	//// TOPO TYPE FUNCTIONS
	OffsetBuffer<int> faceBufferFromMfnMesh(MFnMesh& mfn);

	static int HalfEdgeMeshNeedsRebuild(HalfEdgeMesh& hedgeMesh, MFnMesh& meshFn);

	void HalfEdgeMeshFromMObject(HalfEdgeMesh& hedgeMesh, MObject meshObj, int build);
	void HalfEdgeMeshFromMObject(HalfEdgeMesh& hedgeMesh, MFnMesh& meshFn, int build);

	void meshFnFromHalfEdgeMesh(HalfEdgeMesh& hedgeMesh, MFnMesh& meshFn);

	static int ed::HalfEdgeMeshNeedsRebuild(HalfEdgeMesh& hedgeMesh, MFnMesh& meshFn) {
		// check if mfn mesh needs to be rebuilt, or positions can be updated normally
		return !((hedgeMesh.nPoints == meshFn.numVertices()) && (hedgeMesh.nFaces == meshFn.numPolygons()));
	}


	static std::map<std::pair<int, int>, int> pointFaceVertexMap(MFnMesh& meshFn) {
		// return map of {(point, face) : faceVertex}
		std::map<std::pair<int, int>, int> pfvMap;
		int n = 0;
		for (int f = 0; f < meshFn.numPolygons(); f++) {
			MIntArray facePoints;
			meshFn.getPolygonVertices(f, facePoints);
			for (int v = 0; v < facePoints.length(); v++)
			{
				pfvMap.insert({ {facePoints[v], f}, n });
				n++;
			}
		}
		return pfvMap;
	}

	static std::vector<int> faceVertexUvIds(MFnMesh& meshFn, const char* name) {
		// return vector of [faceVertexId : uvId]
		// maps each unique face vertex to its uv
		std::vector<int> faceVertexUVs(meshFn.numFaceVertices(), -1);
		MString newName(name);
		int n = 0;
		for (int face = 0; face < meshFn.numPolygons(); face++) {
			MIntArray faceVertices;
			meshFn.getPolygonVertices(face, faceVertices);
			for (int vertex = 0; vertex < faceVertices.length(); vertex++) {
				int uvid;
				/*meshFn.getPolygonUVid(
					face, vertex, uvid, newName);*/
				meshFn.getPolygonUVid(
					face, vertex, uvid, &newName);
				faceVertexUVs[n] = uvid;
				n++;
			}
		}
		return faceVertexUVs;
	}

	// trimesh conversions and maps

	static void triMeshData(MFnMesh& meshFn,
		std::map<std::pair<int, int>, int>& pointFaceVertexMap,

		// bound args
		MIntArray& triCounts,
		MIntArray& triVertIndices,

		std::vector<int> &triFaceMap,
		// map triangle corners to original face vertices
		std::vector<int> &triFaceVertexMap
		) {
		// return map of [tri index : original polygon index]
		
		int nPoints = meshFn.numVertices();
		//MIntArray triCounts; // number of triangles per polygon
		//MIntArray triVertIndices; // vertex indices per triangle
		meshFn.getTriangles(triCounts, triVertIndices);
		int nTris = triVertIndices.length() / 3;

		triFaceMap.resize(nTris);
		triFaceVertexMap.resize(triVertIndices.length());

		int n = 0;
		for (int face=0; face < triCounts.length(); face++) {
			for (int triIndex = 0; triIndex < triCounts[face]; triIndex++) {
				triFaceMap[n] = face;

				// set individual vertex information
				for (int triPoint = 0; triPoint < 3; triPoint++) {
					int faceVertex = pointFaceVertexMap[{ n * 3 + triPoint , face }];
					triFaceVertexMap[n * 3 + triPoint] = faceVertex;
				}
				n++;
			}
		}

	}

	static std::vector<int> triFaceVertexToOrigFaceVertex;


	// igl mesh generation

	inline static Eigen::MatrixXd coordMatFromMFnMesh(MFnMesh& meshFn, MStatus &s) 
	{
		// take the raw positions and return a matrix of
		// Nx3, with coordinates in rows
		int nPoints = meshFn.numVertices();
		const float* ptr = meshFn.getRawPoints(&s);
		Eigen::MatrixXd mat(nPoints, 3);
		for (int n = 0; n < nPoints; n++) {
			mat(n, 0) = ptr[n * 3];
			mat(n, 1) = ptr[n * 3 + 1];
			mat(n, 2) = ptr[n * 3 + 2];
		}
		return mat;
	}

	inline static Eigen::MatrixXi faceVertexMatFromMFnMesh(
		MFnMesh& meshFn, MStatus& s) 
	{
		/* return matrix tri mesh, with rows being vertices on each triangle
		*/
		int nPoints = meshFn.numVertices();
		MIntArray triCounts; // number of triangles per polygon
		MIntArray triVertIndices; // vertex indices per triangle
		meshFn.getTriangles(triCounts, triVertIndices);

		int nTris = triVertIndices.length() / 3;
		Eigen::MatrixXi mat(nTris / 3, 3);
		for (int n = 0; n < nTris; n++) {
			mat(n, 0) = triVertIndices[n * 3];
			mat(n, 1) = triVertIndices[n * 3 + 1];
			mat(n, 2) = triVertIndices[n * 3 + 2];
		}
		return mat;
	}



	// ---- EIGEN STUFF -----
	typedef Eigen::SparseMatrix<double> Sparse;
	typedef Eigen::Matrix<double, 4, 4> Mat4;
	typedef Eigen::Matrix<double, 4, 1> Vec4;
	typedef Eigen::Matrix<double, 3, 3> Mat3;
	typedef Eigen::Matrix<double, 3, 1> Vec3;
	typedef Eigen::Matrix<double, -1, -1> MatX;


	static void mmatrix_to_eigen(const MMatrix& M, Mat4& E)
	{
		DEBUGS("topo mmatrix_to_eigen");
		const double *matData = reinterpret_cast<const double*>(&M);
		//D << matData[0], matData[1], matData[2], matData[3],
		//	matData[4], matData[5], matData[6], matData[7],
		//	matData[8], matData[9], matData[10], matData[11],
		//	matData[12], matData[13], matData[14], matData[15];

		std::copy(matData, matData + 16, E.data());

	}

	static MMatrix eigen_to_mmatrix(const Mat4& E)
	{
		DEBUGS("topo eigen_to_mmatrix");
		MMatrix result;
		double *matData = reinterpret_cast<double*>(&result);
		std::copy(E.data(), E.data() + 16, matData);
		return result;
	}


} // namespace ed
#endif
