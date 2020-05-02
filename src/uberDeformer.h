

#ifndef UBERDEFORMER_H
#define UBERDEFORMER_H

#include "lib/api.h"

class UberDeformer : public MPxDeformerNode {
    public:
        UberDeformer();
        virtual ~UberDeformer();

        virtual MStatus deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex);

        static void* creator();
        static MStatus initialize();

		// function to find connected deformerNotions
		std::vector<MObject> findConnectedNotions();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
	static MObject aBind;
	static MObject aGlobalIterations;
	static MObject aNotions;
    
    

};
#endif
	