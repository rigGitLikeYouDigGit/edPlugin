
/*
base lib containing all maya imports,
as well as common plugin functions
*/

#pragma once
#ifndef _PLUGIN_LIB
#define _PLUGIN_LIB 1

#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <memory>
#include <list>

#include <maya/MStreamUtils.h>
#include <maya/MPxNode.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MPxSkinCluster.h>
#include <maya/MTypeId.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
//#include <maya/MSpace.h>
#include <maya/MString.h>
#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MVector.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatMatrix.h>
#include <maya/MVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MMatrixArray.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/MFloatPointArray.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnAttribute.h>

#include <maya/MFnMesh.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MArrayDataBuilder.h>

#include <maya/MFnMatrixArrayData.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnFloatArrayData.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MFnData.h>

#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MItGeometry.h>

#include <maya/MTimer.h>

// including topo lib to build topo types from api types
#include "macro.h"


namespace ed{

// registering IDs of plugin nodes
const unsigned int pluginPrefix = 101997;

// common functions
static MObject makeBindAttr( char* name ){
    MObject newBind;
    MFnEnumAttribute fn;
	newBind = fn.create( name, name, 0 );
    fn.addField("off", 0);
    fn.addField("bind", 1);
    fn.addField("bound", 2);
    fn.addField("live", 3);
    fn.setKeyable(true);
    fn.setHidden(false);
    return newBind;
}
enum BindState {off, bind, bound, live};

static MObject makeSpaceAttr( char* name ){
    MObject newSpace;
    MFnEnumAttribute fn;
	newSpace = fn.create( name, name, 0 );
    fn.addField("naive", 0);
    fn.addField("partitioning", 1);
    fn.setKeyable(false);
    fn.setHidden(false);
    return newSpace;
}
enum SpaceMethod {naive, partitioning};

//static MObject makeDoubleArrayAttr( std::string &name ){
static MObject makeDoubleArrayAttr( char * name ){
    MObject aArray;
    MFnTypedAttribute fn;
	aArray = fn.create(name, name, MFnData::kDoubleArray);
    return aArray;
}

static MDoubleArray accessDoubleArrayAttr( MDataHandle &dh ){
	// returns live double array of attribute
	MFnDoubleArrayData fn( dh.data() );
	return fn.array();
}

static MMatrixArray extractMMatrixArray(MArrayDataHandle& matArray) {
	// extracts matrices from array plug
	// simple, right?

		MMatrixArray result(matArray.elementCount());
		for (unsigned int i = 0; i < matArray.elementCount(); i++) {
			result[i] = MFnMatrixData(matArray.inputValue().data()).matrix();
			// guess again
			matArray.next();
		}
		return result;
	}

static MPlugArray getAllConnectedPlugs(MObject &mainNode, MObject &plugAttr,
		bool asSource, bool asSink) {
	// returns nodes connected to attribute
	DEBUGS("api.h getAllConnectedPlugs")
	MStatus s;
	MPlugArray output;
	MFnDependencyNode dFn(mainNode);
	MPlug queryPlug(mainNode, plugAttr);
	if (queryPlug.isArray()) {

	}
	else {
		queryPlug.connectedTo(output, asSink, asSource);
	}
	/*CHECK_MSTATUS_AND_RETURN_IT(s);*/
	return output;
}


inline MStatus jumpToElement(MArrayDataHandle &hArray, int index) {
	// safer system for array plugs
	// creates index if it doesn't exist
	MStatus s;
	s = hArray.jumpToElement(index);
	if (MFAIL(s)) {
		MArrayDataBuilder builder = hArray.builder(&s);
		builder.addElement(index);
		s = hArray.set(builder);
		s = hArray.jumpToElement(index);
	}
	CHECK_MSTATUS_AND_RETURN_IT(s);
	return s;
}

inline MStatus mirrorArrayDataHandle(MArrayDataHandle &masterArrayDH, MArrayDataHandle &slaveArrayDH) {
	// transfer exact structure of array handle from master to slave
	int index = 0;
	int n = masterArrayDH.elementCount();
	//DEBUGS("n " << n);
	for (index; index < n; index++) {
		//DEBUGS("index " << index);
		jumpToElement(masterArrayDH, index);
		jumpToElement(slaveArrayDH, index);

		slaveArrayDH.outputValue().copy(
			masterArrayDH.outputValue()
		);
	}
	return MStatus::kSuccess;
}

// converting between maya types and vectors
inline std::vector<int> MIntArrayToVector(MIntArray &arr) {
	// constructs stl vector from int array
	DEBUGS("api.h MIntArrayToVector");
	std::vector<int> output(arr.length(), 1);
	for (unsigned int i = 0; i < arr.length(); i++) {
		output[i] = arr[i];
	}
	return output;
}

inline std::vector<float> MFloatArrayToVector(MFloatArray &arr) {
	// constructs stl vector from float array
	// sorry if there's a more elegant way to template these
	DEBUGS("api.h MFloatArrayToVector")
	std::vector<float> output( static_cast<int>( arr.length() ), 1);
	for (unsigned int i = 0; i < arr.length(); i++) {
		output[i] = arr[i];
	}
	return output;
}

inline std::vector<float> MVectorArrayToVector(MVectorArray &arr) {
	// constructs stl vector from MVectorArray
	DEBUGS("api.h MVectorArrayToVector");
	std::vector<float> output(static_cast<int>(arr.length()) * 3, 1);
	for (unsigned int i = 0; i < arr.length(); i++) {
		output[i*3] = static_cast<float>(arr[i].x);
		output[i*3 + 1] = static_cast<float>(arr[i].y);
		output[i*3 + 2] = static_cast<float>(arr[i].z);
	}
	return output;
}

inline MIntArray vectorToMIntArray(std::vector<int> &v) {
	// constructs MIntArray from stl float vector
	DEBUGS("api.h vectorToMIntArray");
	// MIntArray output( static_cast<int>(v.size()) );
	// for (unsigned int i = 0; i < v.size(); i++) {
	// 	output[i] = v[i];
	// }
	MIntArray output( v.data(), static_cast<int>(v.size()));
	return output;
}

inline MFloatArray vectorToMFloatArray(std::vector<float> &v) {
	// constructs MFloatArray from stl float vector
	DEBUGS("api.h vectorToMFloatArray");
	// MFloatArray output( static_cast<int>(v.size()) );
	// // static casting size_t is among the most annoying c++ I've found
	// for (unsigned int i = 0; i < v.size(); i++) {
	// 	output[i] = v[i];
	// }
	MFloatArray output( v.data(), static_cast<int>(v.size()));
	return output;
}

inline MVectorArray vectorToMVectorArray(std::vector<float> &v) {
	// constructs MFloatArray from stl float vector
	DEBUGS("api.h vectorToMVectorArray");
	MVectorArray output(static_cast<int>(v.size()) / 3);
	// static casting size_t is among the most annoying c++ I've found
	for (unsigned int i = 0; i < v.size(); i++) {
		output[i].x = v[i*3];
		output[i].y = v[i*3 + 1];
		output[i].z = v[i*3 + 2];
	}
	return output;
}

inline MVector MVectorFromArray(float arr[]){
	return MVector( arr[0], arr[1], arr[2]);
}

inline MPoint MPointFromArray(float arr[]){
	return MPoint( arr[0], arr[1], arr[2]);
}


// unable to find a good way to do this
template <typename T>
inline void setAttributeAffectsAll(MObject &driver, std::vector<MObject> &driven) {
	// sets driver to affect all driven
	for (auto &i : driven) {
		T::attributeAffects(driver, i);
	}
}

template <typename T>
inline void setAttributesAffect(std::vector<MObject> &drivers, std::vector<MObject> &driven) {
	// sets driver to affect all driven
	for (auto &i : drivers) {
		for (auto &j : driven) {
			T::attributeAffects(i, j);
		}
	}
}

template <typename T>
inline void setAttributesAffect(std::vector<MObject> &drivers, MObject &driven) {
	// sets driver to affect all driven
	for (auto &i : drivers) {
		T::attributeAffects(i, driven);
	}
}

template <typename T>
inline void addAttributes(std::vector<MObject> &attrs){
	// adds all attributes at once
	for( auto &attrObj : attrs ){
		T::addAttribute(attrObj);
	}
}


} // namespace ed

#endif
