

#ifndef _ED_TECTONICCONSTRAINTNODE_H
#define _ED_TECTONICCONSTRAINTNODE_H

#include "lib/api.h"
#include "lib/topo.h"

#include "tectonic/solver.h"
#include "tectonic/constraint.h"

class TectonicConstraintNode : public MPxNode {
    public:
        TectonicConstraintNode();
        virtual ~TectonicConstraintNode();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

        static void* creator();
        static MStatus initialize();
        virtual void postConstructor();


public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
    static MObject aSolver;

    // goal point data - each has a tie of
    // goal vector, base point vector
    static MObject aGoals;
        static MObject aBasePos;
            static MObject aBasePosX;
            static MObject aBasePosY;
            static MObject aBasePosZ;
        static MObject aGoalPos;
            static MObject aGoalPosX;
            static MObject aGoalPosY;
            static MObject aGoalPosZ;

        // int attributes showing which point and plate connected to?
        static MObject aPointIndex;
        static MObject aPlateIndex;

    // all goals in single constraint share settings
 
    // spring settings
    static MObject aStrength;
    static MObject aDamping;

    // could use these as pistons
    static MObject aTargetLength;
   

};
#endif
	