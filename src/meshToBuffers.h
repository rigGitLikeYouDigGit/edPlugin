

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

        // track status of output connections
        std::unordered_set<MObject *> connectedOutputs; // top level attrs
        std::map<MObject*, MObject*> attrChildToParent;
        std::map<MObject*, std::unordered_set<MObject*>> attrParentToChild;

        void syncConnections();



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
        static MObject aPointVectorsX;
        static MObject aPointVectorsY;
        static MObject aPointVectorsZ;
    static MObject aFaceVectors;
        static MObject aFaceVectorsX;
        static MObject aFaceVectorsY;
        static MObject aFaceVectorsZ;
    static MObject aPointNormalVectors;
        static MObject aPointNormalVectorsX;
        static MObject aPointNormalVectorsY;
        static MObject aPointNormalVectorsZ;
    static MObject aFaceNormalVectors;
        static MObject aFaceNormalVectorsX;
        static MObject aFaceNormalVectorsY;
        static MObject aFaceNormalVectorsZ;

    // custom mesh data struct ooh it's very spooky
    static MObject aHedgeMesh;


};
#endif
	