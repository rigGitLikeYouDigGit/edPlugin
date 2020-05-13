

#ifndef MESHTOBUFFERS_H
#define MESHTOBUFFERS_H

#include "lib/api.h"
#include "lib/topo.h"

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

	static MObject aFaceConnects;
	static MObject aFaceOffsets;

    static MObject aPointPositions;
	static MObject aPointConnects;
	static MObject aPointOffsets;

	static MObject aFaceCentres;
	static MObject aUvCoords;
	static MObject aNormals;
    static MObject aBind;
    


};
#endif
	