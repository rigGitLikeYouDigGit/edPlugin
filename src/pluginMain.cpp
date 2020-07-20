/*
register all plugins
*/

#include "edPush.h"

#include "meshAnalysis.h"

// BEGIN PROCEDURAL CONTROL INCLUDE
#include "meshToBuffers.h"
#include "uberDeformer.h"
#include "deformer/deformerNotion.h"
#include "memorySource.h"
#include "memorySink.h"
#include "directDeltaMush.h"

// END PROCEDURAL CONTROL INCLUDE

#include "refDDM.h"

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>

const char* kAUTHOR = "ed";
const char* kVERSION = "1.0";
const char* kREQUIRED_API_VERSION = "Any";

// define vector for nodes to register and deregister
//std::vector<MObject> nodes = {/*
//	MeshToBuffers::thisMObject,
//};*/

#define REGISTER_NODE(NODE) \
    status = fnPlugin.registerNode( \
        NODE::kNODE_NAME, \
        NODE::kNODE_ID, \
        NODE::creator, \
        NODE::initialize, \
        MPxNode::kDependNode \
    ); \
    CHECK_MSTATUS_AND_RETURN_IT(status); \

#define REGISTER_DEFORMER(NODE) \
    status = fnPlugin.registerNode( \
        NODE::kNODE_NAME, \
        NODE::kNODE_ID, \
        NODE::creator, \
        NODE::initialize, \
        MPxNode::kDeformerNode \
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


MStatus initializePlugin( MObject obj ){
	DEBUGS("");
	DEBUGS(" - - - ");
	DEBUGS("");
	DEBUGS("initializePlugin");

    MFnPlugin fnPlugin( obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);
    MStatus status = MStatus::kSuccess;

    // deformers
    status = REGISTER_DEFORMER(EdPush);

    // gpu gubs

    // normal nodes 
    status = REGISTER_NODE(MeshAnalysis);

    // BEGIN PROCEDURAL CONTROL REGISTER
    status = REGISTER_NODE(MeshToBuffers);
	status = REGISTER_DEFORMER(UberDeformer);
	status = REGISTER_NODE(DeformerNotion);
	status = REGISTER_NODE(MemorySource);
	status = REGISTER_NODE(MemorySink);
	status = REGISTER_NODE_TYPE(DirectDeltaMush, MPxNode::kSkinCluster);
	//status = REGISTER_NODE_TYPE(RefDDM, MPxNode::kSkinCluster);

	status = fnPlugin.registerNode( // reference implementation, remove once we have a better solution
		"refDDM",
		//directDeltaMush::id,
		RefDDM::kNODE_ID,
		RefDDM::creator,
		RefDDM::initialize,
		MPxNode::kSkinCluster);

    // END PROCEDURAL CONTROL REGISTER


    return status;
}

MStatus uninitializePlugin( MObject obj ){

    MStatus status;
    MFnPlugin fnPlugin(obj);

    status = DEREGISTER_NODE( EdPush );
    status = DEREGISTER_NODE( MeshAnalysis );
    // BEGIN PROCEDURAL CONTROL DEREGISTER
    status = DEREGISTER_NODE( MeshToBuffers );
	status = DEREGISTER_NODE(UberDeformer);
	status = DEREGISTER_NODE(DeformerNotion);
	status = DEREGISTER_NODE(MemorySource);
	status = DEREGISTER_NODE(MemorySink);
	status = DEREGISTER_NODE(DirectDeltaMush);
	status = fnPlugin.deregisterNode(RefDDM::kNODE_ID);
    // END PROCEDURAL CONTROL DEREGISTER

    return status;

}
