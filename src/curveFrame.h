

/*

	combined curve framing and nearest-point node
	
*/

#include "lib/api.h"
#include "lib/containers.h"

#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\

class CurveFrame : public MPxNode {
	public:
		CurveFrame();
		virtual ~CurveFrame();

		virtual MStatus compute(
			const MPlug& plug, MDataBlock& data);

		static void* creator();
		static MStatus initialize();
		virtual void postConstructor();
		virtual MStatus connectionMade(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);
		virtual MStatus connectionBroken(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);

		CurveFrame * getFrameInstance(MObject &nodeObj, MStatus &s);

		/*MTypeId kNODE_ID(0x00122C1C);
		MString kNODE_NAME("curveFrame");*/


public:
	// internal objects
	enum CurvePointModes {
		u, uNorm, arcLength, arcLengthNorm, chordLengthFromPrev, closestPoint
	};
	enum CurveSolvers {
		doubleReflection, upCurve, frameNode
	};

	MVectorArray tangents;
	MVectorArray normals;

	CurveFrame * framePtr;
	bool frameNodeConnected;


public:
	static MTypeId kNODE_ID;
	static MString kNODE_NAME;

	// attribute MObjects
	static MObject aInCurve;
	static MObject aInUpCurve;
	static MObject aInPoints;
		static MObject aInParam;
		static MObject aInPosition;
		static MObject aPointMode;
	

	static MObject aFrameResolution;
	static MObject aSolver;
	static MObject aFrameNode;

	static MObject aOutUpCurve;
	static MObject aOutTangents;
	static MObject aOutNormals;
	static MObject aOutPoints;
		static MObject aOutPosition;
		static MObject aOutMatrix;

		

};
