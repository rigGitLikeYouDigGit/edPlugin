""" holds templates for .h files for nodes """

baseH = """

#ifndef {nodeNameCaps}_H
#define {nodeNameCaps}_H

#include "lib/api.cpp"

class {nodeNameTitle} : public {nodeParent} {{
    public:
        {nodeNameTitle}();
        virtual ~{nodeNameTitle}();

        virtual MStatus {mainMethod};

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
    {MObjects}
    

}};
#endif
	"""

deformMethod = """deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex)"""

computeMethod = """compute(
				const MPlug& plug, MDataBlock& data)"""

MObjectTemplate = """static MObject {MObjectName};
				"""


MTypeId = """0x00122C05"""