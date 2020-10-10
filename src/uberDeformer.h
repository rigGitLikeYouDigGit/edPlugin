

#ifndef UBERDEFORMER_H
#define UBERDEFORMER_H 1

#include "lib/api.h"
#include "lib/topo.h"
#include "lib/mayaTopo.h"


#include "deformer/deformerNotion.h" // satellite nodes to track
//#include "deformer/deformerData.h"


// master node managing chain of DeformerNotions




class UberDeformer : public MPxDeformerNode {
    public:
        UberDeformer();
        virtual ~UberDeformer();

        virtual MStatus compute(
              MDataBlock& data, const MPlug& plug
        );

        // virtual MStatus deform(
	      //       MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	      //       unsigned int MIndex);


        // binding system
        void bindDeformerNetwork();
        void bindUberDeformer();
        void bindDeformerNotions();

        void globalDeform(int globalIterations, float globalEnvelope);

        void setOutputGeo(MDataBlock &data, MObject &meshObj);

        static void* creator();
        static MStatus initialize();

		MStatus connectionMade(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);
		MStatus connectionBroken(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);

		// connected deformerNotions
    void getConnectedNotions();
		std::vector<DeformerNotion*> connectedNotions;

    ed::HalfEdgeMesh hedgeMesh;

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

    // attribute MObjects
	static MObject aBind;
	static MObject aGlobalIterations;
  //static MObject aGlobalEnvelope;
	static MObject aNotions;

	static MObject aOutputGeo;


};
#endif
