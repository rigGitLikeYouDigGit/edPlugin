
#ifndef SKINNOTION_H
#define SKINNOTION_H

#include "../lib/api.h"
#include "../lib/topo.h"
#include "deformerNotion.h"

struct SkinNotionParametres : DeformerParametres{
  // skin weights
  std::vector<int> vertexOffsets; // offsets to each vertex entry
  std::vector<int> weightIndices; // joint indices per vertex
  std::vector<float> weightValues; // joint values per vertex


};


// skincluster deformerNotion
class SkinNotion : public DeformerNotion {
    public:
        SkinNotion();
        virtual ~SkinNotion();

        // virtual MStatus compute(
				// const MPlug& plug, MDataBlock& data);

				SkinNotionParametres params;

				// EXECUTION FUNCTIONS
				// extractParametres is run every evaluation,
				// transfers datablock values to params struct
				virtual int extractParametres(
					MDataBlock &data, DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

				// bind is run once on bind
				virtual int bind( MDataBlock &data, DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

				// deform
				// virtual int deform( DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

				virtual int deformPoint(int index, DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

        // specific functions
        void extractSkinWeights(MArrayDataHandle& weightRoot,
          SkinNotionParametres& skinInfo, int nPoints) {

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

    // attribute MObjects
  	static MObject aWeightList;
    static MObject aWeights;

    static MObject aTransformMatrices;
    static MObject aBindMatrices;

};
#endif
