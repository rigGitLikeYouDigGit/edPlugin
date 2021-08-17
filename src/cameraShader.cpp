

/*

	test for rendering a view from specific camera to a render target,
	then displaying that in maya itself as a texture for software shading


	
*/

#include "cameraShader.h"

using namespace ed;
using namespace std;

MTypeId CameraShader::kNODE_ID(0x00122C1B);
MString CameraShader::kNODE_NAME( "memorySource" );

MObject CameraShader::aTime;
MObject CameraShader::aResetFrame;
MObject CameraShader::aTimeOffset;
MObject CameraShader::aStepSize;
MObject CameraShader::aNFrames;
MObject CameraShader::aData;
MObject CameraShader::aValueBuffer;
MObject CameraShader::aFrameBuffer;
MObject CameraShader::aInnerData;
MObject CameraShader::aInitData;
MObject CameraShader::aSinkConnection;
MObject CameraShader::aIncrement;
MObject CameraShader::aDeltas;
MObject CameraShader::aPrevTime;

#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\

void CameraShader::postConstructor() {
	this->setExistWithoutInConnections(true);
	this->setExistWithoutOutConnections(true);

	MStatus stat;
	// connect time1 by default
	MSelectionList sel;
	sel.add("time1");
	MFnDependencyNode thisFn(thisMObject());
	MObject timeObj;
	sel.getDependNode(0, timeObj);
	MFnDependencyNode timeFn(timeObj);
	
	MPlug outTimePlug = timeFn.findPlug("outTime");
	MPlug inTimePlug = thisFn.findPlug("time");
	MDGModifier dgMod;
	dgMod.connect(outTimePlug, inTimePlug);
	dgMod.doIt();
}

MStatus CameraShader::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnGenericAttribute gFn;
	MFnTypedAttribute tFn;
	MFnUnitAttribute uFn;
	MFnCompoundAttribute cFn;
	MFnAttribute aFn;


	// attribute used to update cell - time is most convenient
	aTime = uFn.create("time", "time", MFnUnitAttribute::kTime, 0.0);
	tFn.setReadable(true);
	tFn.setWritable(true);

	// previous time value, stored to figure out timestep
	aPrevTime = uFn.create("prevTime", "prevTime", MFnUnitAttribute::kTime, 0.0);
	tFn.setReadable(true);

	// attribute used to update cell - time is most convenient
	aResetFrame = uFn.create("resetFrame", "resetFrame", MFnUnitAttribute::kTime, 1.0);

	// number of frames to track
	aNFrames = nFn.create("nFrames", "nFrames", MFnNumericData::kInt, 3);
	nFn.setReadable(true);
	nFn.setWritable(true);

	// untyped attribute can be used to pass on whatever you want
	aData = tFn.create("data", "data", MFnData::kAny);
	tFn.setReadable(true);
	tFn.setWritable(false);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);
	addAttribute(aData);

	// outer array of frame data
	aFrameBuffer = cFn.create("frame", "frame");
	cFn.setArray(true);
	cFn.setUsesArrayDataBuilder(true);
	cFn.setWritable(false);

	cFn.addChild(aData);

	// internal cache array mirroring main interface
	aInnerData = tFn.create("innerData", "innerData", MFnData::kAny);
	tFn.setReadable(true);
	tFn.setWritable(true);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);

	aInitData = tFn.create("initData", "initData", MFnData::kAny);
	tFn.setReadable(false);
	tFn.setWritable(true);
	tFn.setArray(true);
	tFn.setUsesArrayDataBuilder(true);

	

	// array of time deltas
	aDeltas = nFn.create("delta", "delta", MFnNumericData::kFloat);
	nFn.setWritable(false);
	nFn.setReadable(true);
	nFn.setArray(true);
	nFn.setUsesArrayDataBuilder(true);

	// boolean connection to sink
	aSinkConnection = nFn.create("sink", "sink", MFnNumericData::kBoolean, 0.0);
	nFn.setReadable(true);
	nFn.setWritable(false);

	// can be handy for some uses to have an exclusively positive counter
	aIncrement = nFn.create("increment", "increment", MFnNumericData::kInt, 0.0);
	nFn.setReadable(true);
	nFn.setWritable(false);

	vector<MObject> drivers = { aInitData, aTime, aResetFrame, aTimeOffset, aNFrames };
	vector<MObject> driven = { aFrameBuffer, aInnerData, aSinkConnection, aIncrement,
		aDeltas, aPrevTime,
		aData,
	};
	
	addAttributes<CameraShader>(drivers);
	addAttributes<CameraShader>(driven);

	setAttributesAffect<CameraShader>(drivers, driven);

    return MStatus::kSuccess;
}


MStatus CameraShader::compute(
				const MPlug& plug, MDataBlock& data) {
	MStatus s;
	DEBUGS("memorySource compute");
	//DEBUGS("memorySource sinkConnected " << sinkConnected);

	if (!sinkConnected || sinkObj == MObject::kNullObj) {
		// do literally nothing
		data.setClean(plug);
		return MS::kSuccess;
	}

	// get source data handles
	MArrayDataHandle sourceFramesDH = data.outputArrayValue(aFrameBuffer);
	MArrayDataHandle sourceArrayDH = data.outputArrayValue(aData);
	MArrayDataHandle sourceInnerArrayDH = data.outputArrayValue(aInnerData);
	MArrayDataHandle sourceDeltasDH = data.outputArrayValue(aDeltas);


	// retrieve vector from sink
	std::vector<MObject> sinkData;// = camPtr->dataObjs;
	unsigned int nDataElements = static_cast<int>(sinkData.size());

	DEBUGS("nDataElements " << nDataElements);

	// extract sink data
	MDataHandle sinkDH;
	s = getSinkData(sinkObj, sinkDH);
	MArrayDataHandle sinkArrayDH = MArrayDataHandle(sinkDH);

	//data.setClean(plug);
	//return MS::kSuccess;

	int nFrames = data.inputValue(aNFrames).asInt();
	MTime newMTime = data.inputValue(aTime).asTime();
	MTime prevMTime = data.outputValue(aPrevTime).asTime();
	double newTime = newMTime.as(MTime::k24FPS);
	double prevTime = prevMTime.as(MTime::k24FPS);


	//data.setClean(plug);
	//return MS::kSuccess;
		
	// check if time is reset frame - reset to current values if so
	if EQ(newTime, data.inputValue(aResetFrame).asTime().as(MTime::k24FPS)) {
		DEBUGS("memorySource time reset");
		/*
		data.setClean(aFrameBuffer);
		data.setClean(aData);
		data.setClean(aDeltas);
		data.setClean(aIncrement);
		data.setClean(plug);
		*/

		// reset vectors
		dataArrays.clear();

		for (int i = 0; i < nFrames; i++) {
			jumpToElement(sourceDeltasDH, i);
			jumpToElement(sourceFramesDH, i);

			dataArrays.push_back(std::vector<MObject>(sinkData));
		}

		// look up data through plug since it was crashing otherwise
		MPlug aDataPlug(thisMObject(), aData);


		// reset all frame data
		// for (int i = 0; i < sourceFramesDH.elementCount(); i++) {
		for (int i = 0; i < nFrames; i++) {
			s = aDataPlug.selectAncestorLogicalIndex(i, aFrameBuffer);
			MDataHandle aDataDH = aDataPlug.constructHandle(data);
			MArrayDataHandle aDataArrayDH(aDataDH, &s);
			MCHECK(s, "reset data arrayHandle construction failed");
			CHECK_MSTATUS_AND_RETURN_IT(s );
			MArrayDataBuilder aDataArrayBuilder = aDataArrayDH.builder(&s);
			MCHECK(s, "reset data arrayHandle builder extraction failed")
			CHECK_MSTATUS_AND_RETURN_IT(s);

			for (unsigned int n = 0; n < nDataElements; n++) {
				MDataHandle aResetDataDH = aDataArrayBuilder.addElement(n, &s);
				MCHECK(s, "reset data add data entry failed");
				CHECK_MSTATUS_AND_RETURN_IT(s);
				s = aResetDataDH.setMObject( MObject(dataArrays[i][n]));
				MCHECK(s, "reset data inner data entry " << n << "copy failed");
				CHECK_MSTATUS_AND_RETURN_IT(s);
			}
			s = aDataArrayDH.set(aDataArrayBuilder);
			MCHECK(s, "reset data set array builder failed");
			CHECK_MSTATUS_AND_RETURN_IT(s);
			aDataPlug.setValue(aDataDH);
			aDataPlug.destructHandle(aDataDH);

			// reset deltas
			jumpToElement(sourceDeltasDH, i);
			MDataHandle deltaDH = sourceDeltasDH.outputValue();
			deltaDH.setFloat(0.0);
		}
		
		// reset counter
		data.outputValue(aIncrement).setInt(0);

		// set plugs clean and return
		vector<MObject> driven = { aFrameBuffer, aInnerData, aSinkConnection, aIncrement,
	aDeltas, aPrevTime,
	aData,
		};
		data.setClean(aFrameBuffer);
		data.setClean(aData);
		data.setClean(aDeltas);
		data.setClean(aIncrement);
		data.setClean(plug);
		return MS::kSuccess;
	}

	


	// for some reason input time plug was never reading as dirty
	DEBUGS("prevTime " << prevTime << " newTime " << newTime);
	//if(!EQ( prevTime, newTime)){
	if(!EQ( prevTime, newTime)){
		DEBUGS("memorySource time dirty");

		float delta = newTime - prevTime;
		float deltaA = delta, deltaB = delta;
		float deltas[2] = { delta, delta };

		// update delta array
		for (int i = 0; i < nFrames; i++) {
			jumpToElement(sourceDeltasDH, i);
			deltas[i % 2] = sourceDeltasDH.outputValue().asFloat();
			sourceDeltasDH.outputValue().setFloat(deltas[!(i % 2)]);
		}

		// update data arrays
		dataArrays.insert(dataArrays.begin(), sinkData);
		dataArrays.pop_back();


		for ( int i = 0; i < nFrames; i++) {
			jumpToElement(sourceFramesDH, i);
			MDataHandle frameDH = sourceFramesDH.outputValue();
			MArrayDataHandle frameArrayDH(frameDH);

			
			MPlug targetPlug(thisMObject(), aData);
			s = targetPlug.selectAncestorLogicalIndex(i, aFrameBuffer);
			MCHECK(s, "failed to set parent index for data plug")


			for (unsigned int n = 0; n < nDataElements; n++) {
				MPlug targetLeafPlug = targetPlug.elementByPhysicalIndex(n);

				DEBUGS("targetPlug " << targetLeafPlug.info());

				if (targetLeafPlug.isNull()) {
					DEBUGS("targetLeafPlug is not valid, continuing");
					continue;
				}
				if (dataArrays[i][n].isNull()) {
					DEBUGS("Mobject is null, continuing");
					continue;
				}

				targetLeafPlug.setMObject(dataArrays[i][n]);
			}
		}

		data.setClean(aFrameBuffer);
		data.setClean(aData);

		// add to increment
		data.outputValue(aIncrement).setInt(data.outputValue(aIncrement).asInt() + 1);

		previousTime = newTime;
		data.outputValue(aPrevTime).setMTime(newMTime);
		data.setClean(aTime);
		data.setClean(aPrevTime);

		// dirty sink plug
		bool switchStatus = !data.outputValue(aSinkConnection).asBool();
		data.outputValue(aSinkConnection).setBool(switchStatus);

	}
	

	

	data.setClean(plug);

    return MS::kSuccess;
}

MStatus CameraShader::getSinkData(MObject &sinkObj, MDataHandle &sinkDH) {
	//DEBUGS("memorySource getSinkData")
	MStatus s;
	//MPlug sinkPlug = MPlug(sinkObj, MemorySink::aData);

	//sinkDH = MDataHandle(sinkPlug.asMDataHandle());

	return MStatus::kSuccess;
}

MObject * CameraShader::getSinkInstance(MObject &sinkObj, MStatus &s) {
	// retrieve and cast full user-defined node for given MObject
	// thanks Matt
	MFnDependencyNode nodeFn(sinkObj);
	
	// retrieve MPxNode pointer
	MPxNode *mpxPtr = nodeFn.userNode(&s);
	//MCHECK(s, "failed to extract mpxNode pointer, object is invalid");

	//// black science
	//MemorySink * sinkPtr = dynamic_cast<MemorySink *>(mpxPtr);
	//if (sinkPtr == NULL) {
	//	cerr << "failed dynamic cast to sink instance " << endl;
	//	s = MS::kFailure;
	//}
	//s = MS::kSuccess;
	return camPtr;

}

MStatus CameraShader::setOutputSourceData(MArrayDataHandle &sourceArrayDH, MArrayDataHandle &sinkArrayDH) {
	DEBUGS("memorySource setOutputData")
	MStatus s;

	mirrorArrayDataHandle(sinkArrayDH, sourceArrayDH);
	return MStatus::kSuccess;
}


MStatus CameraShader::connectionMade(
	const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// check if sink has been connected 
	//DEBUGS("memorySource connectionMade");

	//DEBUGS("plug " << plug.name() << " otherPlug " << otherPlug.name());

	// we only care about sink plug
	if (plug.attribute() != aSinkConnection) {
		return MPxNode::connectionMade(plug, otherPlug, asSrc);
	}

	return MPxNode::connectionMade(plug, otherPlug, asSrc);
}

MStatus CameraShader::connectionBroken(
	const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	// clear connected sink node
	DEBUGS("memorySource connectionBroken");

	if (plug.attribute() != aSinkConnection) {
		return MPxNode::connectionBroken(plug, otherPlug, asSrc);
	}
	clearSinkObj(MObject::kNullObj);

	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
}

void CameraShader::setSinkObj(MObject &obj) {
	// sets internal sink reference
	sinkObj = obj;
	MStatus s;
	camPtr = getSinkInstance(obj, s);
	sinkConnected = true;
}

void CameraShader::clearSinkObj(MObject &obj) {
	// is this enough to clear reference to connected node?
	sinkObj = MObject::kNullObj;
	camPtr = NULL;
	sinkConnected = false;
}

void* CameraShader::creator(){
	// sink is not connected on creation
	CameraShader *newObj = new CameraShader;
	newObj->sinkConnected = false;
	newObj->sinkObj = MObject::kNullObj;
	newObj->camPtr = NULL;
	newObj->previousTime = 0.0;
    return newObj;

}

CameraShader::CameraShader() {};
CameraShader::~CameraShader() {};

