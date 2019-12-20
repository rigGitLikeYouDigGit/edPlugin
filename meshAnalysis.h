
#ifndef MESHANALYSIS_H
#define MESHANALYSIS_H

#include <vector>
#include <iostream>


#include <maya/MPxDeformerNode.h>
#include <maya/MTypeId.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnDependencyNode.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MPlug.h>
#include <maya/MItGeometry.h>


class MeshAnalysis : public MPxDeformerNode {
    public:
        MeshAnalysis();
        virtual ~MeshAnalysis();
        virtual MStatus compute( )

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
        static MTypeId id;
        static MString nodeName;
};
#endif