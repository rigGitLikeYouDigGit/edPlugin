

#ifndef DEFORMERNOTION_H
#define DEFORMERNOTION_H

#include "../lib/topo.h"
#include "../lib/mayaTopo.h"
#include "../lib/api.h"



class DeformerNotion : public MPxNode {
    public:
        DeformerNotion();
        virtual ~DeformerNotion();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

		// deformation function for each notion scheme
		void notionDeform(int maxGlobalIterations, int currentGlobalIteration,
			int maxLocalIterations, int currentLocalIteration, MeshData &meshData);

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
	static MObject aWeights;
	static MObject aUseWeights;
	static MObject aLocalIterations;
	static MObject aMasterConnection;

};
#endif
	