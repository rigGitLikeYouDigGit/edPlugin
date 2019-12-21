""" template strings for c++ """

baseCpp = """

/*

your description here

*/

#include "{nodeName}.h"

MTypeId {nodeNameTitle}::kNODE_ID(0x00122C05);
MString {nodeNameTitle}::kNODE_NAME( "{nodeName}" );

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

