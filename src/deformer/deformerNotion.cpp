

/*

	individual component deformation of uberDeformer

*/

#include "deformerNotion.h"

using namespace ed;
using namespace std;

MTypeId DeformerNotion::kNODE_ID(pluginPrefix, 0x00122C11);
MString DeformerNotion::kNODE_NAME( "deformerNotion" );

MObject DeformerNotion::aMasterWeights;
MObject DeformerNotion::aLocalIterations;
MObject DeformerNotion::aLocalEnvelope;
MObject DeformerNotion::aUberDeformer;
MObject DeformerNotion::aDeformationMode;



MStatus DeformerNotion::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnTypedAttribute tFn;
	MFnEnumAttribute eFn;

	// boolean plug to connect to master deformer
	aUberDeformer = nFn.create("uberDeformer", "uberDeformer", MFnNumericData::kBoolean, 0);

	aLocalEnvelope = nFn.create("envelope", "envelope", MFnNumericData::kFloat, 1.0);
	nFn.setMin(0.0);
	//nFn.setMax(1.0);

	// weight array attribute
	aMasterWeights = tFn.create("weights", "weights", MFnData::kDoubleArray);

	// local iterations, how many times to loop this operation on each cycle
	aLocalIterations = nFn.create("localIterations", "localIteration", MFnNumericData::kInt, 1);
	nFn.setMin(1);

	// aDeformationMode = eFn.create("deformationMode", "deformationMode", 0);
	// eFn.addField("relative");
	// eFn.addField("absolute");
	// way too complex to include just yet

	// set affects
	vector<MObject> drivers = { aMasterWeights, aLocalIterations,
		aLocalEnvelope, //aDeformationMode
	};
	vector<MObject> driven = { aUberDeformer };

	addAttributes<DeformerNotion>( drivers );
	addAttributes<DeformerNotion>( driven );

	setAttributesAffect<DeformerNotion>(drivers, driven);

    return MStatus::kSuccess;
}

int DeformerNotion::extractParametres(
	MDataBlock &data, DeformerParametres &params
){
	params.localEnvelope = data.inputValue(aLocalEnvelope).asFloat();
	params.localIterations = data.inputValue(aLocalIterations).asInt();
	params.masterWeights = accessDoubleArrayAttr(data.inputValue(aMasterWeights));
	return 1;

}


MStatus DeformerNotion::compute(
				const MPlug& plug, MDataBlock& data) {
	extractParametres(data, this->params);
	// "balance wheel" mechanism to mark node dirty to uberDeformer
	// thanks Matt
	bool old = data.outputValue(aUberDeformer).asBool();
	data.outputValue(aUberDeformer).setBool( !old );

	data.setClean(plug);
    return MS::kSuccess;
}

///// DEFORMATION /////
int DeformerNotion::deformGeo( DeformerParametres &params, HalfEdgeMesh &hedgeMesh ){
	// if needed, override mesh-wide deformation system here
	for(int iteration=0; iteration < params.localIterations; iteration++){
		params.localIteration = iteration;
		// multithread this sick filth
		for(int i=0; i < hedgeMesh.nPoints; i++){
			deformPoint(i, params, hedgeMesh);
		}
		// swap new and old buffers to update mesh to result
		hedgeMesh.swapDeltaBuffers();
	}
	return 1;
}

int DeformerNotion::deformPoint( int index, DeformerParametres &params, HalfEdgeMesh &hedgeMesh ){
	// most should only override deformPoint, touching deformGeo should be unnecessary
	// example for how to interface with HalfEdgeMesh in parallel
	SmallList<float> oldPositions = hedgeMesh.pointPositions.entry(index);
	for(int i=0; i < 3; i++){
		// update new point positions from calculation
		hedgeMesh.deltaPointPositions.values[3*index + i] = oldPositions[i];
	}
}


void* DeformerNotion::creator(){
		// set masterWeights array to all 1.0 here
    return new DeformerNotion;
}

DeformerNotion::DeformerNotion() {};
DeformerNotion::~DeformerNotion() {};
