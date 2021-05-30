
/*
base lib containing all maya imports,
as well as common plugin functions

maya notes

many nodes need a bind() method to precompute geometry, 
build objects etc.
For ease of use, follow one of 2 signatures - if bind() may be called asynchronously, by a DIFFERENT NODE, use
MStatus bind(MFnDependencyNode& mfn, *params, MStatus&s);
else, use
	MStatus bind(MDatablock& data, *params, MStatus&s);

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
#include <algorithm>
#include <numeric>

#include <maya/MStreamUtils.h>
#include <maya/MPxNode.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MPxSkinCluster.h>
#include <maya/MTypeId.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>
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
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MRampAttribute.h>

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
#include <maya/MFnNurbsSurfaceData.h>
#include <maya/MFnData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnStringArrayData.h>

#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MItGeometry.h>

#include <maya/MTimer.h>
#include <maya/MTime.h>


#include "macro.h"
#include "enum.h"




namespace ed{

// registering IDs of plugin nodes
const unsigned int pluginPrefix = 101997;

#define MCHECK(stat,msg)             \
        if ( MS::kSuccess != stat ) {   \
                cerr << __LINE__ << msg;            \
                return MS::kFailure;    \
        }


// common functions
//enum BindState { off, bind, bound, live };
BETTER_ENUM(BindState, int, off, bind, bound, live);
BETTER_ENUM(LiveState, int, stop, playback, realtime);

template<typename T>
static MObject makeEnumAttr(char* name) {
	MObject newBind;
	MFnEnumAttribute fn;
	newBind = fn.create(name, name, 0);
	for (size_t index = 0; index < T::_size(); ++index) {
		fn.addField(T::_names()[index],
			T::_values()[index]);
	}
	fn.setKeyable(true);
	fn.setHidden(false);
	return newBind;
}



static MObject makeBindAttr( char* name ){
    MObject newBind;
    MFnEnumAttribute fn;
	newBind = fn.create( name, name, 0 );
	for (size_t index = 0; index < BindState::_size(); ++index) {
		fn.addField(BindState::_names()[index],
			BindState::_values()[index]);
	}
    fn.setKeyable(true);
    fn.setHidden(false);
    return newBind;
}


//enum SpaceMethod { naive, partitioning };
BETTER_ENUM(SpaceMethod, int, naive, partitioning)
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

//enum LiveState {stop, playback, realtime};
// off, playing back when time input changes,
// or when ANY input changes
static MObject makeLiveAttr(char* name) {
	// signifying 
	MObject newLive;
	MFnEnumAttribute fn;
	newLive = fn.create(name, name, 1);
	for (size_t index = 0; index < LiveState::_size(); ++index) {
		fn.addField(LiveState::_names()[index],
			LiveState::_values()[index]);
	}
	fn.addField("off", 0);
	fn.addField("playback", 1);
	fn.addField("realtime", 2);
	fn.setKeyable(false);
	fn.setHidden(false);
	return newLive;
}

// ramp attribute interface
// order ramp points by position
// array of structs - maximum of ~10 points

struct RampPoint {
	float position;
	float value;
	int interp;
	bool operator< (const RampPoint &other) const {
		return position < other.position;
	}
};

struct RampInterface {
	RampPoint *pointArray;
	unsigned int nPoints = 0;
	MIntArray indices, interps;
	MFloatArray positions, values;
	RampInterface(MRampAttribute &ramp) {
		if (nPoints < ramp.getNumEntries()) {
			nPoints = ramp.getNumEntries();
			if (pointArray) delete[] pointArray;
			pointArray = new RampPoint[nPoints];
		}
		ramp.getEntries(indices, positions, values, interps);

		for (unsigned int i = 0; i < nPoints; i++)
		{
			pointArray[i].position = positions[i];
			pointArray[i].value = values[i];
			pointArray[i].interp = interps[i];
		}
		std::sort(pointArray, pointArray + nPoints);
	}

	inline void getValueAtPosition(float position, float &value) {
		// stub for now, can be redone for better spline interpolation
	}

	~RampInterface() { // deallocate point array
		if (pointArray) delete[] pointArray;
		pointArray = nullptr;
	}
};

template <typename T>
static T* castToUserNode(MObject& nodeObj, MStatus& s) {
	// retrieve and cast full user-defined node for given MObject
	// thanks Matt
	MFnDependencyNode nodeFn(nodeObj);

	// retrieve MPxNode pointer
	MPxNode* mpxPtr = nodeFn.userNode(&s);
	MCHECK(s, "failed to extract mpxNode pointer, object is invalid");

	// black science
	TectonicConstraintNode* sinkPtr = dynamic_cast<TectonicConstraintNode*>(mpxPtr);
	if (sinkPtr == NULL) {
		cerr << "failed dynamic cast to sink instance " << endl;
		s = MS::kFailure;
	}
	s = MS::kSuccess;
	return sinkPtr;
}


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
	unsigned int index = 0;
	unsigned int n = masterArrayDH.elementCount();
	DEBUGS(" master array handle length n " << n);
	for (index; index < n; index++) {
		//DEBUGS("index " << index);
		jumpToElement(masterArrayDH, index);
		jumpToElement(slaveArrayDH, index);

		slaveArrayDH.outputValue().copy(
			masterArrayDH.outputValue());
		/*slaveArrayDH.outputValue().set(
			MObject(masterArrayDH.outputValue().data()));*/
	}
	DEBUGS("slave array handle length " << slaveArrayDH.elementCount());
	return MStatus::kSuccess;
}

inline MStatus childDataHandles(MArrayDataHandle &parentArrayDH, std::vector<MDataHandle> &result) {
	// return all data handles that are children of parent
	MStatus s;
	for (unsigned int i = 0; i < parentArrayDH.builder().elementCount(); i++) {
		s = parentArrayDH.jumpToArrayElement(i);
		MCHECK(s, "failed to gather child data handles");
		result.push_back(parentArrayDH.outputValue());
	}
	return s;
}

inline std::vector<MObject> arrayHandleData(MArrayDataHandle &parentArrayDH, MStatus &stat) {
	// copy all data MObjects in array - only valid for flat arrays
	std::vector<MObject> result;
	for (unsigned int i = 0; i < parentArrayDH.builder().elementCount(); i++) {
		stat = jumpToElement(parentArrayDH, i);
		result.push_back(MObject(parentArrayDH.outputValue().data()));
	}
	return result;
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
