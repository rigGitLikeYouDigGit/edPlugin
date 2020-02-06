/*
base lib containing all maya imports,
as well as common plugin functions
*/

//#pragma once

#include <vector>
#include <string>

#include <maya/MPxNode.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MTypeId.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnDependencyNode.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MPlug.h>
#include <maya/MItGeometry.h>


// common functions
// how do you actually use strings though
static MObject makeBindAttr( ){
    MObject aBind;
    MFnEnumAttribute fn;
    aBind = fn.create( "bind", "bind", 1 );
    fn.addField("off", 0);
    fn.addField("bind", 1);
    fn.addField("bound", 2);
    fn.setKeyable(true);
    fn.setHidden(false);
    return aBind;
}
