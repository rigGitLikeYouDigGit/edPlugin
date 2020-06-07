

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
	std::vector<double> influenceWeights;
};

// argument struct for deformation
struct DeformerParametres {
	MMatrixArray tfMats;
	MMatrixArray refMats;
	float envelope;

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


public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
	static MObject aBind;

	// weight buffers
	static MObject aVertexWeightOffsets;
	static MObject aVertexWeightIndices;
	static MObject aVertexWeightValues;
	

    
    

};
#endif
	