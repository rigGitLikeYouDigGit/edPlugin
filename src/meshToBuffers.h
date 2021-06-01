

#ifndef MESHTOBUFFERS_H
#define MESHTOBUFFERS_H

#include "lib/api.h"
#include "lib/topo.h"
#include "lib/mayaTopo.h"


class MeshToBuffers : public MPxNode {
    public:
        MeshToBuffers();
        virtual ~MeshToBuffers();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

        MStatus bind(MDataBlock& data, MFnMesh& meshFn, MStatus& s);

        virtual void postConstructor();
        MStatus connectionMade(
            const MPlug& plug, const MPlug& otherPlug, bool asSrc);
        MStatus connectionBroken(
            const MPlug& plug, const MPlug& otherPlug, bool asSrc);

        static void* creator();
        static MStatus initialize();

        // track state of input mesh connection
        bool isConnected = false;

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
    static MObject aInMesh;

	static MObject aFaceConnects;
	static MObject aFaceOffsets;

    static MObject aPointPositions;
	static MObject aPointConnects;
	static MObject aPointOffsets;

	static MObject aFaceCentres;
	static MObject aUvCoords;
	static MObject aNormals;
    static MObject aBind;
    
    // vector values for the above
    static MObject aPointVectors;
    static MObject aFaceVectors;
    static MObject aPointNormalVectors;
    static MObject aFaceNormalVectors;

    // custom mesh data struct ooh it's very spooky
    static MObject aHedgeMesh;


};
#endif
	