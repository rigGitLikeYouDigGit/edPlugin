/*
register all plugins
*/

#include "edPush.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

const char* kAUTHOR = "ed";
const char* kVERSION = "1.0";
const char* kREQUIRED_API_VERSION = "Any";

#define REGISTER_NODE(NODE)                    \
    status = fnPlugin.registerNode(            \
        NODE::kNODE_NAME,                    \
        NODE::kNODE_ID,                        \
        NODE::creator,                        \
        NODE::initialize                    \
    );                                        \
    CHECK_MSTATUS_AND_RETURN_IT(status);    \

#define DEREGISTER_NODE(NODE)                \
    status = fnPlugin.deregisterNode(        \
        NODE::kNODE_ID                        \
    );                                        \
    CHECK_MSTATUS_AND_RETURN_IT(status);    \

MStatus initializePlugin( MObject obj ){

//    MFnPlugin fnPlugin( obj, "edPlugin_by_ed", "1.0", "any");
    MFnPlugin fnPlugin( obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);
    MStatus status = MStatus::kSuccess;
    MString errorString;

    //status = REGISTER_NODE(EdPush)

    status = fnPlugin.registerNode(
        //"edPush",
        EdPush::kNODE_NAME,
        EdPush::kNODE_ID,
//        EdPush::id,
        EdPush::creator,
        EdPush::initialize,
        MPxNode::kDeformerNode    );
        
    if( status != MStatus::kSuccess ){
        status.perror( "registerNode" );
        return status;
    }
    return status;

}

MStatus uninitializePlugin( MObject obj ){

    MStatus status;
    MFnPlugin plugin(obj);

    //status = plugin.deregisterNode( EdPush::id );
    status = plugin.deregisterNode( EdPush::kNODE_ID );
    if (!status){
        status.perror( "deregisterNode" );
        return status;
    }
    return status;

}
