

#ifndef DIRECTDELTAMUSH_H
#define DIRECTDELTAMUSH_H

#include "lib/api.h"
#include "lib/topo.h"

/*
use this opportunity to test a more efficient way of working with per-vertex weights

buffers:

vertex index : influence1 weight, influence4 weight, influence5 weight    - influence weights
vertex index : 1, 4, 5   - influence indices
vertex index : 3  - offset buffer

*/
struct SkinData {
	// struct to store and query skincluster data
	std::vector<int> vertexOffsets;
	std::vector<int> influenceIndices;
	std::vector<float> influenceWeights;
};

// argument struct for deformation
struct DeformerParametres {
	MMatrixArray tfMats;
	MMatrixArray refMats;
	MMatrixArray diffMats; // product matrices
	float envelope;

	double smoothTranslation;
	double smoothRotation;
	double alpha;
	int iterations;

	SkinData skinData; // stores vertex joint influences
};


class DirectDeltaMush : public MPxSkinCluster {
    public:
        DirectDeltaMush();
        virtual ~DirectDeltaMush();


		virtual MStatus compute(
			const MPlug& plug, MDataBlock& data);

        /*virtual MStatus deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex);
*/
		void runBind( MDataBlock& data, const MObject& meshObj);
		void setOutputGeo(MDataBlock& data, const MObject& meshObj);
		void deformGeo(
			ed::HalfEdgeMesh& mesh, const DeformerParametres & params,
			std::vector<double>& outPositions);
		void deformPoint(
			ed::HalfEdgeMesh& mesh, const DeformerParametres & params,
			std::vector<double>& outPositions, int index);
        static void* creator();
        static MStatus initialize();

		ed::HalfEdgeMesh * hedgeMesh; // pointer to halfEdge mesh for deformed geo
		DeformerParametres * deformParams; // deformation arguments

		ed::Mat4 getOmega(int i, int j);
		MStatus precompute(MDataBlock& block);
		// bit crazy but working on it
		//std::vector<std::list<std::pair<int, ed::Mat4>>> omegas; 
		std::vector<std::vector<std::pair<int, ed::Mat4>>> omegas; 


public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
	static MObject aBind;
	static MObject aBindSkinWeights;

	static MObject aTransSmooth;
	static MObject aRotSmooth;
	static MObject aAlpha;
	static MObject aIterations;

	// weight buffers
	static MObject aVertexWeightOffsets;
	static MObject aVertexWeightIndices;
	static MObject aVertexWeightValues;
	
	

    
    

};
#endif
	