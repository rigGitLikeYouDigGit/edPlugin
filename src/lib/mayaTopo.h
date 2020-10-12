
#ifndef _MAYA_TOPO_H
#define _MAYA_TOPO_H 1

#include "topo.h"
#include "api.h"

// conversions between pure topo types and maya types
// it was just easier to manage this way



namespace ed{

//// TOPO TYPE FUNCTIONS
	OffsetBuffer<int> faceBufferFromMfnMesh(MFnMesh& mfn);


	void HalfEdgeMeshFromMObject(HalfEdgeMesh& hedgeMesh, MObject meshObj, int build);

	void meshFnFromHalfEdgeMesh(HalfEdgeMesh &hedgeMesh, MFnMesh &meshFn);

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
