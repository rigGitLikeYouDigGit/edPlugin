

/*

	combined curve framing and nearest-point node
	
*/

#include "lib/api.h"
#include "lib/containers.h"

#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\

class MultiMod : public MPxNode {
	public:
		MultiMod();
		virtual ~MultiMod();

		virtual MStatus compute(
			const MPlug& plug, MDataBlock& data);

		static void* creator();
		static MStatus initialize();
		virtual void postConstructor();

		/*MTypeId kNODE_ID(0x00122C1C);
		MString kNODE_NAME("curveFrame");*/


public:
	// internal objects
	enum FalloffModes {
		volume, surface
	};
	enum CombinationModes {
		mean, sum, max, smoothMax, min, smoothMin
	};

public:
	static MTypeId kNODE_ID;
	static MString kNODE_NAME;

	// attribute MObjects
	static MObject aInputMesh;

	static MObject aHandles;
		static MObject aMatrix;
		static MObject aRefMatrix;
		static MObject aFalloff;
		static MObject aFalloffMode;
		static MObject aFalloffRadius;
		static MObject aWeights;
	static MObject aCombinationMode;
	static MObject aMasterWeights;

	static MObject aOutputMesh;

		

};
