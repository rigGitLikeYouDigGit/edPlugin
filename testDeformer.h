

#ifndef TESTDEFORMER_H
#define TESTDEFORMER_H

#include <vector>
#include <iostream>

#include <maya/MPxDeformerNode.h>


class TestDeformer : public MPxDeformerNode {
    public:
        TestDeformer();
        virtual ~TestDeformer();

        virtual MStatus 
		 deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex)   ;

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId id;
    static MString kNODE_NAME;

};
#endif
	