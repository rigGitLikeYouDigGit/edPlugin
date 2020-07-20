

#ifndef UBERDEFORMER_H
#define UBERDEFORMER_H

#include "lib/api.h"
#include "lib/topo.h"

// master node managing chain of DeformerNotions


class UberDeformer : public MPxDeformerNode {
    public:
        UberDeformer();
        virtual ~UberDeformer();

        virtual MStatus deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex);

        static void* creator();
        static MStatus initialize();

		MStatus connectionMade(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);
		MStatus connectionBroken(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);

		// vector of MObjects for each connected deformerNotion
		std::vector<MObject> getConnectedNotions();
		std::vector<MObject> connectedNotions;

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

    // attribute MObjects
	static MObject aBind;
	static MObject aGlobalIterations;
	static MObject aNotions;



};
#endif
