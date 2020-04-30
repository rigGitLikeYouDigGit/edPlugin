
/*
base lib containing all maya imports,
as well as common plugin functions
*/

#pragma once
#ifndef _PLUGIN_LIB
#define _PLUGIN_LIB 1

#include <vector>
#include <string>
#include <iostream>

#include <maya/MStreamUtils.h>
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

// debug macros
 // as in "debugString"
#define DEBUGS(info) \
MStreamUtils::stdOutStream() << info << std::endl;


using namespace std;

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

// converting between maya types and vectors
inline vector<int> MIntArrayToVector(MIntArray &arr) {
	// constructs stl vector from int array
	vector<int> output(arr.length(), 1);
	for (unsigned int i = 0; i < arr.length(); i++) {
		output[i] = arr[i];
	}
	return output;
}

inline vector<float> MFloatArrayToVector(MFloatArray &arr) {
	// constructs stl vector from float array
	// sorry if there's a more elegant way to template these
	vector<float> output( static_cast<int>( arr.length() ), 1);
	for (unsigned int i = 0; i < arr.length(); i++) {
		output[i] = arr[i];
	}
	return output;
}

inline vector<float> MVectorArrayToVector(MVectorArray &arr) {
	// constructs stl vector from MVectorArray
	vector<float> output(static_cast<int>(arr.length()) * 3, 1);
	for (unsigned int i = 0; i < arr.length(); i++) {
		output[i*3] = static_cast<float>(arr[i].x);
		output[i*3 + 1] = static_cast<float>(arr[i].y);
		output[i*3 + 2] = static_cast<float>(arr[i].z);
	}
	return output;
}

inline MIntArray vectorToMIntArray(vector<int> &v) {
	// constructs MIntArray from stl float vector
	MIntArray output( static_cast<int>(v.size()) );	
	for (unsigned int i = 0; i < v.size(); i++) {
		output[i] = v[i];
	}
	return output;
}

inline MFloatArray vectorToMFloatArray(vector<float> &v) {
	// constructs MFloatArray from stl float vector
	MFloatArray output( static_cast<int>(v.size()) );
	// static casting size_t is among the most annoying c++ I've found
	for (unsigned int i = 0; i < v.size(); i++) {
		output[i] = v[i];
	}
	return output;
}

inline MVectorArray vectorToMVectorArray(vector<float> &v) {
	// constructs MFloatArray from stl float vector
	MVectorArray output(static_cast<int>(v.size()) / 3);
	// static casting size_t is among the most annoying c++ I've found
	for (unsigned int i = 0; i < v.size(); i++) {
		output[i].x = v[i*3];
		output[i].y = v[i*3 + 1];
		output[i].z = v[i*3 + 2];
	}
	return output;
}


// unable to find a good way to do this
//void setAttributeAffectsAll(MPxNode &nodeType, MObject &driver, vector<MObject> &driven) {
//	// sets driver to affect all driven
//	for (auto &i : driven) {
//		nodeType::attributeAffects(driver, i);
//	}
//}
//


#endif