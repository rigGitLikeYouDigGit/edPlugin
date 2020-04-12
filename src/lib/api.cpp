
/*
base lib containing all maya imports,
as well as common plugin functions
*/

//#pragma once
#ifndef _PLUGIN_LIB
#define _PLUGIN_LIB

#include <vector>
#include <string>
#include <iostream>

#include <maya/MPxNode.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MTypeId.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MVector.h>
#include <maya/MFloatVector.h>
#include <maya/MVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/MFloatPointArray.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnFloatArrayData.h>
#include <maya/MFnData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnDependencyNode.h>



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
    fn.addField("live", 3);
    fn.setKeyable(true);
    fn.setHidden(false);
    return aBind;
}


#endif