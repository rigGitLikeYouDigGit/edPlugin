

/*

	no copyright or breach of contract intended, 
	it's just much more efficient to test this when I already have the environment set up
	will delete once project is concluded
	
*/

#include "directDeltaMush.h"

using namespace std;

MTypeId DirectDeltaMush::kNODE_ID(0x00122C1D);
MString DirectDeltaMush::kNODE_NAME( "directDeltaMush" );

MObject DirectDeltaMush::aBind;


MObject DirectDeltaMush::aVertexWeightIndices;
MObject DirectDeltaMush::aVertexWeightValues;
MObject DirectDeltaMush::aVertexWeightOffsets;


/*
objects inherited from deformer, skincluster etc

MObject input - input attribute, array
MObject inputGeom - input geometry attribute
MObject outputGeom - output geometry attribute
MObject envelope - envelope attribute

MObject weightList
MObject weights
MObject matrix - input matrix array
MObject bindPreMatrix - input prematrix array

ALSO INHERIT ALL ATTRIBUTES FROM NORMAL SKINCLUSTER enjoy that node bloat lol

*/


MStatus DirectDeltaMush::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;

	aBind = makeBindAttr();
	addAttribute(aBind);

	// vertex weight buffers
	// main skin matrix weights
	aVertexWeightOffsets = tFn.create("vertexWeightOffsets", "vertexWeightOffsets", MFnData::kIntArray);
	addAttribute(aVertexWeightOffsets);

	aVertexWeightIndices = tFn.create("vertexWeightIndices", "vertexWeightIndices", MFnData::kIntArray);
	addAttribute(aVertexWeightIndices);

	aVertexWeightValues = tFn.create("vertexWeightValues", "vertexWeightValues", MFnData::kIntArray);
	addAttribute(aVertexWeightValues);

	// attribute affects
	// crude for now
	vector<MObject> drivers = {
		aVertexWeightIndices, aVertexWeightOffsets, aVertexWeightValues,
		aBind,
		matrix, bindPreMatrix, weights, weightList,
		inputGeom, envelope
	};

	vector<MObject> driven = {
		outputGeom
	};

	setAttributesAffect<DirectDeltaMush>(drivers, driven);

    return MStatus::kSuccess;
}


//MStatus DirectDeltaMush::deform(
//	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
//	            unsigned int MIndex) {
//    return MS::kSuccess;
//}

MStatus DirectDeltaMush::compute(
	const MPlug& plug, MDataBlock& data) {

	if (plug != outputGeom) {
		return MS::kSuccess;	}
	MStatus m;
	MArrayDataHandle inputArray = data.outputArrayValue(input);
	jumpToElement(inputArray, 0);
	MDataHandle geoHandle = (inputArray.inputValue().child(inputGeom));
	MObject meshObj = geoHandle.asMesh();
	
	//MFnMeshData meshData = MFnMeshData::create();
	//MObject newMeshObj = MFnMeshData::create();

	// check envelope
	float envelopeValue = data.inputValue(envelope).asFloat();
	if (envelopeValue < 0.001) {
		//MObject mesh = MObject(data.inputValue(inputGeom).asMesh());
		//data.outputValue(outputGeom).copy( data.inputValue(inputGeom));
		return MS::kSuccess;
	}
	//MObject mesh = MObject(data.inputValue(inputGeom).asMesh());
	//data.outputValue(outputGeom).copy(data.inputValue(inputGeom));
	//data.outputValue(outputGeom).setMObject(meshDatas );

	// set output geometry plug
	MArrayDataHandle outputArray = data.outputArrayValue(outputGeom);
	jumpToElement(outputArray, 0);
	MDataHandle outputGeoHandle = outputArray.outputValue();
	outputGeoHandle.setMObject(meshObj);

	return MS::kSuccess;
}


/*
use this opportunity to test a more efficient way of working with per-vertex weights

buffers:

vertex index : influence1 weight, influence4 weight, influence5 weight    - influence weights
vertex index : 1, 4, 5   - influence indices
vertex index : 3  - offset buffer



*/


void* DirectDeltaMush::creator(){

    return new DirectDeltaMush;

}

DirectDeltaMush::DirectDeltaMush() {};
DirectDeltaMush::~DirectDeltaMush() {};

