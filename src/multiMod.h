

/*

	combined curve framing and nearest-point node
	
*/

#include "lib/api.h"
#include "lib/containers.h"
#include "lib/topo.h"
#include "lib/mayaTopo.h"
#include "lib/edMaths.h"

struct MultiModParametres {
	MMatrixArray deltaMats;
	MMatrixArray captureMats;
	/*std::vector<float> captureRadii;
	std::vector<float> weightValues;*/
	MFloatArray captureRadii;
	MFloatArray weightValues;
	ed::SmallList<MRampAttribute>captureRamps;
	ed::SmallList<MFloatArray>handlePaintedWeights;
	
	// global parametres
	MFloatArray globalWeights;
	float envelope;

	// function pointer to execute on positions
	float *fnPtr = nullptr;
};

class MultiMod : public MPxDeformerNode {
	public:
		MultiMod();
		virtual ~MultiMod();

		virtual MStatus compute(
			const MPlug& plug, MDataBlock& data);

		virtual int gatherParams( MDataBlock& data);

		// deform functions
		virtual int deformGeo(MultiModParametres &params,
			ed::HalfEdgeMesh &hedgeMesh, int meshIndex);

		virtual int deformPoint(MultiModParametres &params,
			ed::HalfEdgeMesh &hedgeMesh,
			//const float *deltaPositions, 
			/*float * deltaPositions,
			int nPoints,*/
			std::vector<float> &deltaPositions,
			int index);

		static void* creator();
		static MStatus initialize();
		virtual void postConstructor();


public:
	// internal objects
	enum FalloffModes {
		volume, surface
	};
	enum CombinationModes {
		mean, sum, max, smoothMax, 
	};



	ed::SmallList<ed::HalfEdgeMesh> hedgeMeshes;
	MultiModParametres params;

public:
	static MTypeId kNODE_ID;
	static MString kNODE_NAME;

	// attribute MObjects
	// static MObject aInMesh;

	static MObject aHandles;
		static MObject aMat;
		static MObject aCaptureMat;
		static MObject aFalloffRamp;
		static MObject aFalloffMode;
		static MObject aFalloffRadius;
		static MObject aWeightArray;
		static MObject aWeight;
	static MObject aCombinationMode;
	static MObject aCombinationSmoothness;

	static MObject aWeights;

	//static MObject aOutMesh;

		

};
