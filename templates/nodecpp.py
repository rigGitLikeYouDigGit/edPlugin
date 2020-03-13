""" template strings for c++ """

baseCpp = """

/*
{nodeDescription}
*/

#include "{nodeName}.h"

MTypeId {nodeNameTitle}::kNODE_ID({MTypeId});
MString {nodeNameTitle}::kNODE_NAME( "{nodeName}" );

{MObjects}

MStatus {nodeNameTitle}::initialize()
{{
    // initialise attributes

    return MStatus::kSuccess;
}}


MStatus {nodeNameTitle}::{mainMethod} {{
    return MS::kSuccess;
}}

void* {nodeNameTitle}::creator(){{

    return new {nodeNameTitle};

}}

{nodeNameTitle}::{nodeNameTitle}() {{}};
{nodeNameTitle}::~{nodeNameTitle}() {{}};

"""

MObjectTemplate = """MObject {nodeNameTitle}::{MObjectName};
					"""


