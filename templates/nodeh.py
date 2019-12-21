""" holds templates for .h files for nodes """

baseH = """

#ifndef {nodeNameCaps}_H
#define {nodeNameCaps}_H

#include <vector>
#include <iostream>

#include <maya/{nodeParent}.h>
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

class {nodeNameTitle} : public {nodeParent} {{
    public:
        {nodeNameTitle}();
        virtual ~{nodeNameTitle}();

        {mainMethod}

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;

}};
#endif
	"""

deformMethod = """
		virtual MStatus deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex);   """

computeMethod = """
	    virtual MStatus compute(const MPlug& plug, MDataBlock& data);
		"""