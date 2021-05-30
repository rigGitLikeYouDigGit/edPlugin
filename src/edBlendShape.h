

/*

	ed-based blendshape 
	
*/

#include "lib/api.h"
#include "lib/containers.h"

#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\

class EdBlendShape : public MPxNode {
	public:
		EdBlendShape();
		virtual ~EdBlendShape();

		virtual MStatus compute(
			const MPlug& plug, MDataBlock& data);

		static void* creator();
		static MStatus initialize();
		virtual void postConstructor();
		virtual MStatus connectionMade(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);
		virtual MStatus connectionBroken(
			const MPlug &plug, const MPlug &otherPlug, bool asSrc);

		EdBlendShape * getFrameInstance(MObject &nodeObj, MStatus &s);

		/*MTypeId kNODE_ID(0x00122C1C);
		MString kNODE_NAME("curveFrame");*/


public:


	MVectorArray tangents;
	MVectorArray normals;

	EdBlendShape * framePtr;
	bool frameNodeConnected;


public:
	static MTypeId kNODE_ID;
	static MString kNODE_NAME;

	// attribute MObjects
	static MObject aBind;
	static MObject aTargets;


		

};
