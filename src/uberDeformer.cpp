

/*

	build deformation scheme by iterating over deformation functions
	
*/

#include "uberDeformer.h"

MTypeId UberDeformer::kNODE_ID(0x00122C09);
MString UberDeformer::kNODE_NAME( "uberDeformer" );

MObject UberDeformer::aBind;
MObject UberDeformer::aGlobalIterations;
MObject UberDeformer::aNotions;


MStatus UberDeformer::initialize()
{
    // initialise attributes
	MFnEnumAttribute eFn;
	MFnNumericAttribute nFn;

	// standard bind system
	aBind = eFn.create("bind", "bind", 1);
	eFn.addField("off", 0);
	eFn.addField("bind", 1);
	eFn.addField("bound", 2);
	eFn.addField("live", 3);
	eFn.setKeyable(true);
	eFn.setHidden(false);
	addAttribute(aBind);

	// array of booleans to connect to deformerNotions
	aNotions = nFn.create("notions", "notions", MFnNumericData::kBoolean, 0);
	nFn.setArray(true);
	nFn.setWritable(true);
	nFn.setReadable(false);
	nFn.setKeyable(false);
	addAttribute(aNotions);

	// iterations to run over entire deformer system
	aGlobalIterations = nFn.create("iterations", "iterations", MFnNumericData::kInt, 1);
	addAttribute(aGlobalIterations);


    return MStatus::kSuccess;
}


MStatus UberDeformer::deform(
	            MDataBlock& data, MItGeometry& iter, const MMatrix& mat,
	            unsigned int MIndex) {


	// check bind
	int bind = data.inputValue(aBind).asInt();
	if (bind == 1 || bind == 3) { // bind or live

		if (bind == 1) {
			data.inputValue(aBind).setInt(2);
		}
	}
    return MS::kSuccess;
}

void* UberDeformer::creator(){

    return new UberDeformer;

}

UberDeformer::UberDeformer() {};
UberDeformer::~UberDeformer() {};

