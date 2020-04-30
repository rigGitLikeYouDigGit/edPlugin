/*
register all plugins
*/

#include "edPush.h"

#include "meshAnalysis.h"

// BEGIN PROCEDURAL CONTROL INCLUDE
#include "meshToBuffers.h"
#include "uberDeformer.h"
#include "deformer/deformerNotion.h"
// END PROCEDURAL CONTROL INCLUDE

#include <maya/MFnPlugin.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>

const char* kAUTHOR = "ed";
const char* kVERSION = "1.0";
const char* kREQUIRED_API_VERSION = "Any";

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

/*
macros shamelessly lifted from yantor3d
thanks mate
*/

// node IDs in one place for ease of use
//MTypeId MeshAnalysis::kNODE_ID(0x00122C04);
//MTypeId EdPush::kNODE_ID(0x00122C05);


//MTypeId MeshToBuffers::kNODE_ID(0x00122C08);
//MTypeId UberDeformer::kNODE_ID(0x00122C09);
//MTypeId DeformerNotion::kNODE_ID(0x00122C10);


MStatus initializePlugin( MObject obj ){

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
    // END PROCEDURAL CONTROL DEREGISTER

    return status;

}
