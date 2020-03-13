

#ifndef UBERDEFORMER_H
#define UBERDEFORMER_H

#include "lib.cpp"

class UberDeformer : public MPxDeformerNode {
    public:
        UberDeformer();
        virtual ~UberDeformer();

        virtual MStatus 
		 deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex)   ;

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

};
#endif
	