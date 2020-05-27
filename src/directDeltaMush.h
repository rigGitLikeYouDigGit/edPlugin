

#ifndef DIRECTDELTAMUSH_H
#define DIRECTDELTAMUSH_H

#include "lib/api.h"

class DirectDeltaMush : public MPxSkinCluster {
    public:
        DirectDeltaMush();
        virtual ~DirectDeltaMush();

		virtual MStatus compute(
			const MPlug& plug, MDataBlock& data);

        /*virtual MStatus deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex);
*/
        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
	static MObject aBind;

	// weight buffers
	static MObject aVertexWeightOffsets;
	static MObject aVertexWeightIndices;
	static MObject aVertexWeightValues;
	

    
    

};
#endif
	