

#ifndef _ED_TECTONICNODE_H
#define _ED_TECTONICNODE_H


#include <time.h>
#include <chrono>
#include <array>

#include "lib/api.h"
#include "lib/topo.h"
#include "lib/mayaTopo.h"

#include "tectonic/solver.h"
#include "tectonic/constraint.h"
#include "tectonic/tectonicMaya.h"

#include "tectonicConstraintNode.h"


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

        // time functions
        MStatus reset(MDataBlock& data, MStatus& s);

        // iteration functions
        MStatus runSolve(MDataBlock& data, float dt, MStatus& s);



public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
    static MObject aInMesh; // active input mesh
    static MObject aBaseMesh; // base input mesh that tectonic system will be built from

    static MObject aUVSet; // UV set to use when building from UVs
    static MObject aBind; // normal bind mode of tectonic system - bind bound live etc
    static MObject aSplitMode; // Split mode to use for extracting plates - mesh or UV connectivity
    static MObject aPlaybackMode; // off, frame-dependent or real-time

    static MObject aConstraints;

    // time and dynamic attributes
    static MObject aGlobalIterations; // how many iterations to perform for each time step
    static MObject aTimeStep; // length of each timestep
    static MObject aTime; // current scene time frame
    static MObject aPrevTime; // internal attr to store previous time frame
    static MObject aResetFrame; // frame on which reset function triggers

    /*realtime mode will ALWAYS evaluate a single timestep from neutral - this allows 
    the semi-end result of a solve to be seen live while modelling the base geometry
    */


 
    /* Query attributes, each takes vector query position
    * relative to neutral mesh
    * returns the index and matrix of closest plate
    */
    static MObject aQueryPlates;
        static MObject aQueryPos;
            static MObject aQueryPosX;
            static MObject aQueryPosY;
            static MObject aQueryPosZ;
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
	