

#ifndef DIRECTDELTAMUSH_H
#define DIRECTDELTAMUSH_H

#include "lib/api.h"
#include "lib/topo.h"

// argument struct for deformation
struct DeformerParametres {
	MMatrixArray* tfMats;
	MMatrixArray* refMats;

	// avoid NxM length arrays with buffers here too
	std::vector<int> vertexInfluenceIndices; // transform indices affecting vertex
	std::vector<float> vertexInfluenceValues; // scalar weights for each transform
	std::vector<int> vertexInfluenceOffsets; // per-vertex offsets into both
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

        static void* creator();
        static MStatus initialize();

		ed::HalfEdgeMesh * hedgeMesh; // pointer to halfEdge mesh for deformed geo

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
	