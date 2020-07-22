

#ifndef DEFORMERNOTION_H
#define DEFORMERNOTION_H

#include "../lib/api.h"
#include "../lib/topo.h"
//#include "deformerData.h"

/*
base class for individual components of deformer system
each deformer node specifies functions:
		 bind(), extractParametres(), and deform()

each deformer node specifies struct DeformerParametres
no hard distinction between precomputed information and keyable -
everything is placed in deformer params. This may become an issue if
uploading cost grows too serious.

*/

struct DeformerParametres {
  int localIterations;
  float envelope;
  MDoubleArray masterWeights;
}

class DeformerNotion : public MPxNode {
    public:
        DeformerNotion();
        virtual ~DeformerNotion();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

				DeformerParametres * params;

				// EXECUTION FUNCTIONS
				// extractParametres is run every evaluation,
				// transfers datablock values to params struct
				virtual int extractParametres(
					MDataBlock &data, DeformerParametres &params );

				// bind is run once on bind
				virtual int bind( MDataBlock &data, DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

				// deform
				//virtual int deform( DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

        virtual int deformPoint( int index,
          DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

    // attribute MObjects
  static MObject aEnvelope;
	static MObject aMasterWeights;
	static MObject aLocalIterations;
	static MObject aUberDeformer;

};
#endif
