

#ifndef DEFORMERNOTION_H
#define DEFORMERNOTION_H

#include "../lib/api.h"
#include "../lib/topo.h"
#include "deformerData.h"

// base class for individual components of deformer system
// each deformer node specifies functions:
//		 bind(), extractParametres(), and deform()
//
// each deformer node specifies struct DeformerParametres

class DeformerNotion : public MPxNode {
    public:
        DeformerNotion();
        virtual ~DeformerNotion();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

				ed::DeformerParametres * params;

				// EXECUTION FUNCTIONS
				// extractParametres is run every evaluation,
				// transfers datablock values to params struct
				virtual int extractParametres(
					MDataBlock &data, ed::DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

				// bind is run once on bind
				virtual int bind( MDataBlock &data, ed::DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

				// deform 
				virtual int deform( ed::DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

    // attribute MObjects
	static MObject aWeights;
	static MObject aUseWeights;
	static MObject aLocalIterations;
	static MObject aUberDeformer;

};
#endif
