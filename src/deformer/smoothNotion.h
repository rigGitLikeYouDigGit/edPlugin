
#ifndef SMOOTHNOTION_H
#define SMOOTHNOTION_H

#include "../lib/api.h"
#include "../lib/topo.h"
#include "deformerNotion.h"

struct SmoothNotionParametres : DeformerParametres{

};


// skincluster deformerNotion
class SmoothNotion : public DeformerNotion {
    public:
        SmoothNotion();
        virtual ~SmoothNotion();

        // virtual MStatus compute(
				// const MPlug& plug, MDataBlock& data);

				SmoothNotionParametres * params;

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


        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

    // attribute MObjects


};
#endif
