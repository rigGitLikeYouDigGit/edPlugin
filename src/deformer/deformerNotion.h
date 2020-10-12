

#ifndef DEFORMERNOTION_H
#define DEFORMERNOTION_H 1

#include "../lib/api.h"
#include "../lib/topo.h"
#include "../lib/mayaTopo.h"

/*
base class for individual components of deformer system
each deformer node specifies functions:
		 bind(), extractParametres(), and deform() and upload*()

each deformer node specifies struct DeformerParametres
no hard distinction between precomputed information and keyable -
everything is placed in deformer params.
upload() lets node micromanage what gets passed to gpu and when

there is no stable way to avoid collapsing parallel kernels between
deformer steps, as otherwise it would be possible to have patches of the
deformation outrun each other between steps - cool, but not what we want
*/

struct DeformerParametres {
  int globalIterations;
  int globalIteration;
  float globalEnvelope; // packing global stuff in here seems fine

  int localIterations;
  int localIteration;
  float localEnvelope;
  MDoubleArray masterWeights;
};

class DeformerNotion : public MPxNode {
    public:
        DeformerNotion();
        virtual ~DeformerNotion();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

		DeformerParametres params;

		// EXECUTION FUNCTIONS
		// extractParametres is run every evaluation,
		// transfers datablock values to params struct
		virtual int extractParametres(
			MDataBlock &data, DeformerParametres &params );

		//		// bind is run once on bind
		//virtual int bind( MDataBlock &data, DeformerParametres &params,
		//	ed::HalfEdgeMesh &hedgeMesh );

       // bind actually has to be run asynchronously by uberDeformer,
       // so must be passed an MFnDependencyNode instead of a datablock
       virtual int bind( MFnDependencyNode &mfn, DeformerParametres &params,
          ed::HalfEdgeMesh &hedgeMesh );

		// deform
		virtual int deformGeo( DeformerParametres &params, 
			ed::HalfEdgeMesh &hedgeMesh );

        // deform individual point
        // this method will be executed from threads, so must be entirely threadsafe
        virtual int deformPoint( int index,
          DeformerParametres &params, ed::HalfEdgeMesh &hedgeMesh );

        // GPU methods
        virtual int uploadParametres(
          DeformerParametres &params);

        int uploadMesh(
          ed::HalfEdgeMesh &hedgeMesh );

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

    // attribute MObjects
  static MObject aLocalEnvelope;
	static MObject aMasterWeights;
	static MObject aLocalIterations;
  static MObject aDeformationMode;
	static MObject aUberDeformer;

};
#endif
