#ifndef EDPUSH_H
#define EDPUSH_H

#include "lib/api.h"


class EdPush : public MPxDeformerNode {
    public:
        EdPush();
        virtual ~EdPush();
        virtual MStatus deform(
            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
            unsigned int MIndex
        );

        virtual MStatus connectionMade(const MPlug& nodePlug, const MPlug& foreignPlug,
            bool asSrc );

        static void* creator();
        static MStatus initialize();

private:
    //void initialiseMasks(int length, MDataBlock& data);
    // MDoubleArray initialiseMasks(int length, MDataBlock& data);
    MDoubleArray initialiseMasks(int length, MDataHandle& maskHandle);

    //void initialiseMasks(int length, MObject maskAttributes[]);
    //void initialiseMasks(int length, std::vector maskAttributes);

    public:
        static MObject aOffset;
        static MObject aMask;
        static MTypeId kNODE_ID;
//        static MString nodeName;
        static MString kNODE_NAME;

};
#endif
