

#ifndef MESHANALYSIS_H
#define MESHANALYSIS_H

#include "lib/api.h"

class MeshAnalysis : public MPxNode {
    public:
        MeshAnalysis();
        virtual ~MeshAnalysis();

        virtual MStatus compute(
                const MPlug& plug, MDataBlock& data);

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

    // attributes
    static MObject aInMesh;
    static MObject aRefMesh;
    static MObject aCurvature;
    static MObject aDeltas;
    static MObject aTension;
    static MObject aBind;


};
#endif
	