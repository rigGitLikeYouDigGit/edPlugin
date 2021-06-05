/*
register all plugins
*/
#include "lib/macro.h"

#include "edPush.h"

#include "meshAnalysis.h"

// BEGIN PROCEDURAL CONTROL INCLUDE
#include "meshToBuffers.h"
#include "uberDeformer.h"
#include "deformer/deformerNotion.h"
#include "memorySource.h"
#include "memorySink.h"
//#include "directDeltaMush.h"
#include "curveFrame.h"
#include "multiMod.h"
#include "tectonicNode.h"

// END PROCEDURAL CONTROL INCLUDE

//#include "refDDM.h"

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>

const char* kAUTHOR = "ed";
const char* kVERSION = "1.0";
const char* kREQUIRED_API_VERSION = "Any";

// define vector for nodes to register and deregister
//std::vector<MPxNode> nodes = {
//	MeshToBuffers*,
//	MemorySource,
//	MemorySink,
//	MeshAnalysis,
//};
//std::vector<MPxNode> pluginNodes(5);


#define REGISTER_NODE(NODE) \
    status = fnPlugin.registerNode( \
        NODE::kNODE_NAME, \
        NODE::kNODE_ID, \
        NODE::creator, \
        NODE::initialize \
    ); \
    CHECK_MSTATUS_AND_RETURN_IT(status); \

#define REGISTER_DEFORMER(NODE) \
    status = fnPlugin.registerNode( \
        NODE::kNODE_NAME, \
        NODE::kNODE_ID, \
        NODE::creator, \
        NODE::initialize, \
        MPxNode::Type::kDeformerNode \
    ); \
    CHECK_MSTATUS_AND_RETURN_IT(status); \


#define DEREGISTER_NODE(NODE) \
    status = fnPlugin.deregisterNode( \
        NODE::kNODE_ID ); \
    CHECK_MSTATUS_AND_RETURN_IT(status); \

#define REGISTER_NODE_TYPE(NODE, NODE_TYPE) \
    status = fnPlugin.registerNode( \
        NODE::kNODE_NAME, \
        NODE::kNODE_ID, \
        NODE::creator, \
        NODE::initialize, \
        NODE_TYPE \
    ); \
    CHECK_MSTATUS_AND_RETURN_IT(status); \
 // MPxNode::kDependNode

/*
macros shamelessly lifted from yantor3d
thanks mate
*/
//
//MTypeId TectonicNode::kNODE_ID(0x00122C08);
//MString TectonicNode::kNODE_NAME("tectonic");

MStatus initializePlugin( MObject obj ){
	DEBUGS("");
	DEBUGS(" - - - ");
	DEBUGS("");
	DEBUGS("initializePlugin");

    MFnPlugin fnPlugin( obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);
    MStatus status = MStatus::kSuccess;
    int s = MPxNode::kDependNode;
    /*s = MPxNode::schedulingType(;*/
    // deformers
    //status = REGISTER_DEFORMER(EdPush);

 //   // gpu gubs

 //   // normal nodes 
 //   //status = REGISTER_NODE(MeshAnalysis);

 //   TectonicNode::kNODE_ID = 0x00122C08;
 //   TectonicNode::kNODE_NAME = "tectonic";
 //   status = REGISTER_NODE(TectonicNode);

 //   // BEGIN PROCEDURAL CONTROL REGISTER
    status = REGISTER_NODE(MeshToBuffers);
	//status = REGISTER_DEFORMER(UberDeformer);
	status = REGISTER_DEFORMER(MultiMod);
	//status = REGISTER_NODE(DeformerNotion);
	//status = REGISTER_NODE(MemorySource);
	//status = REGISTER_NODE(MemorySink);
	//status = REGISTER_NODE(CurveFrame);
	//
    
    



    // END PROCEDURAL CONTROL REGISTER


    return status;
}

MStatus uninitializePlugin( MObject obj ){

    MStatus status;
    MFnPlugin fnPlugin(obj);

    //status = DEREGISTER_NODE( EdPush );
 //   status = DEREGISTER_NODE( MeshAnalysis );
 //   // BEGIN PROCEDURAL CONTROL DEREGISTER
    status = DEREGISTER_NODE( MeshToBuffers );
	//status = DEREGISTER_NODE(UberDeformer);
	//status = DEREGISTER_NODE(DeformerNotion);
	//status = DEREGISTER_NODE(MemorySource);
	//status = DEREGISTER_NODE(MemorySink);
	//status = DEREGISTER_NODE(CurveFrame);
	status = DEREGISTER_NODE(MultiMod);

 //   status = DEREGISTER_NODE(TectonicNode);
	////status = DEREGISTER_NODE(DirectDeltaMush);
	////status = fnPlugin.deregisterNode(RefDDM::kNODE_ID);
 //   // END PROCEDURAL CONTROL DEREGISTER

    return status;

}
