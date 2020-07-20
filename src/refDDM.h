//#pragma once
#ifndef REFDDM_H
#define REFDDM_H

#include <list>
#include <vector>

#include "lib/api.h"
#include "lib/topo.h"

//#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h>

#include <maya/MFnMesh.h>
#include <maya/MPxSkinCluster.h>
#include <maya/MStatus.h>

//#include <Eigen/Dense>
//#include <Eigen/SVD>
//#include <Eigen/Sparse>

#include <maya/MArrayDataBuilder.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrixArray.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MTimer.h>
#include <maya/MObject.h>

//#include <igl/cotmatrix.h>
//#include <igl/massmatrix.h>
//
//typedef double NUMTYPE;
//
//typedef Eigen::SparseMatrix<NUMTYPE> Sparse;
//typedef Eigen::Matrix<NUMTYPE, 4, 4> Mat4;
//typedef Eigen::Matrix<NUMTYPE, 4, 1> Vec4;
//typedef Eigen::Matrix<NUMTYPE, 3, 3> Mat3;
//typedef Eigen::Matrix<NUMTYPE, 3, 1> Vec3;
//typedef Eigen::Matrix<NUMTYPE, -1, -1> MatX;


//class directDeltaMush : public MPxSkinCluster {
class RefDDM : public MPxSkinCluster {
	public:


		static void* creator();
		static MStatus initialize();
		static MObject aTransSmooth;
		// static MObject aTransSmoothMap;
		static MObject aRotSmooth;
		// static MObject aRotSmoothMap;
		static MObject aAlpha;
		static MObject aSteps;
		static MObject aRecalc;
		// Deformation function
		//
		MStatus deform(MDataBlock& block,
			MItGeometry& iter,
			const MMatrix& mat,
			unsigned int multiIndex) override;

		//static const MTypeId id;

	private:
		MStatus JumpToElement(MArrayDataHandle& hArray, unsigned int index);
		ed::Mat4 getOmega(int i, int j);
		MStatus precompute(MDataBlock& block, MItGeometry& iter);
		std::vector<std::list<std::pair<int, ed::Mat4>>> omegas;

	public:

		static MTypeId kNODE_ID;
		static MString kNODE_NAME;


};

#endif // !REFDDM_H