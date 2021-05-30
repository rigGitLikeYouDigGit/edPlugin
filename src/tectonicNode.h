

#ifndef TECTONICNODE_H_H
#define TECTONICNODE_H_H

#include "lib/api.h"
#include "lib/topo.h"
#include "lib/mayaTopo.h"

#include "tectonicConstraintNode.h"

#include "tectonic/solver.h"
#include "tectonic/constraint.h"

#include <time.h>
#include <chrono>
#include <array>

class TectonicNode : public MPxNode {
    public:
        TectonicNode();
        virtual ~TectonicNode();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

        static void* creator();
        static MStatus initialize();
        virtual void postConstructor();


        MStatus connectionMade(
            const MPlug& plug, const MPlug& otherPlug, bool asSrc);
        MStatus connectionBroken(
            const MPlug& plug, const MPlug& otherPlug, bool asSrc);

        MStatus syncConnections(MStatus& s);

        MStatus bind(MDataBlock& data, MStatus& s);

        // connected deformerNotions
        //TectonicConstraintNode* getConnectedConstraints(
        //    MObject& sinkObj, MStatus& s);
        //std::vector<ed::TecConstraint*> connectedConstraints;
        ed::HalfEdgeMesh hedgeMesh;

        // time attributes
        unsigned int currentTime;
        unsigned int prevTime;

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
    static MObject aInMesh;
    static MObject aBaseMesh;

    static MObject aUVSet;
    static MObject aBind;
    static MObject aSplitMode;
    static MObject aMode;

    static MObject aConstraints;

    // time and dynamic attributes
    static MObject aGlobalIterations;
    static MObject aTimeStep;
    static MObject aTime;
    static MObject aPrevTime;
    static MObject aResetFrame;
 
    /* Query attributes, each takes vector query position
    * relative to neutral mesh
    * returns the index and matrix of closest plate
    */
    static MObject aQueryPlates;
        static MObject aQueryPos;
        static MObject aQueryIndex;
        static MObject aQueryMatrix;

    

    // outputs
    // output mesh
    static MObject aOutMesh;
    // output cut mesh (very cool)
    static MObject aOutCutMesh;
    // matrix array attribute
    static MObject aOutMatrices;
    


};
#endif
	