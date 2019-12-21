

#ifndef TESTNODE_H
#define TESTNODE_H

#include <vector>
#include <iostream>

#include <maya/MPxNode.h>
#include <maya/MTypeId.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MPlug.h>
#include <maya/MItGeometry.h>

class Testnode : public MPxNode {
    public:
        Testnode();
        virtual ~Testnode();

        
	    virtual MStatus compute(const MPlug& plug, MDataBlock& data);
		

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

};
#endif
	