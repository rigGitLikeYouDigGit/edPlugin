/*
register all plugins
*/

#include "edPush.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

#define CHECK_MSTATUS_AND_RETURN_IT(STATUS, msg)
        if (!STATUS){
        STATUS.perror( msg );
        return STATUS;
    }

#define REGISTER_NODE(NODE)
    status = fnPlugin.registerNode(
        NODE::kNODE_NAME,
        NODE::kNODE_ID,
        NODE::creator,
        NODE::initialize
    );
    CHECK_MSTATUS_AND_RETURN_IT(status, "error registering node");

#define DEREGISTER_NODE(NODE)
    status = fnPlugin.deregisterNode(
        NODE::kNODE_ID
    );
    CHECK_MSTATUS_AND_RETURN_IT(status);

/*
macros shamelessly lifted from yantor3d
thanks mate
*/


MStatus initializePlugin( MObject obj ){

    MFnPlugin plugin( obj, "edPush_by_ed", "1.0", "any");
    MStatus status = MStatus::kSuccess;
    MString errorString;

//    status = plugin.registerNode(
//        "edPush",
//        EdPush::id,
//        EdPush::creator,
//        EdPush::initialize,
//        MPxNode::kDeformerNode    );
        
//    if( status != MStatus::kSuccess ){
//        status.perror( "registerNode" );
//        return status;
//    }
//    return status;

    REGISTER_NODE(EdPush);
    return MS::kSuccess;

}

MStatus uninitializePlugin( MObject obj ){

    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterNode( EdPush::id );
    if (!status){
        status.perror( "deregisterNode" );
        return status;
    }
    return status;

}
