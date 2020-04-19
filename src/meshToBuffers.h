

#ifndef MESHTOBUFFERS_H
#define MESHTOBUFFERS_H

#include "lib/api.cpp"
#include "lib/topo.cpp"

class MeshToBuffers : public MPxNode {
    public:
        MeshToBuffers();
        virtual ~MeshToBuffers();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
    static MObject aTest;
    static MObject aInMesh;
    static MObject aPointPositions;
    static MObject aFaceCounts;
    static MObject aFaceConnects;
	static MObject aPointConnects;
	static MObject aFaceCentres;
    static MObject aBind;
    


};
#endif
	