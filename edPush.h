#ifndef EDPUSH_H
#define EDPUSH_H

#include <maya/MPxDeformerNode.h>

#include <vector>


class EdPush : public MPxDeformerNode {
    public:
        EdPush();
        virtual ~EdPush();
        virtual MStatus deform(
            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
            unsigned int MIndex
        );

        static void* creator();
        static MStatus initialize();
}
