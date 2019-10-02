/*
register all plugins
*/

#include "edPush.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

MStatus initializePlugin( MObject obj ){

    MFnPlugin plugin( obj, "edPush_by_ed", "1.0", "any");
    MStatus status = MStatus::kSuccess;
    MString errorString;

    status = plugin.registerNode(
        "edPush",
        EdPush::id,
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

    status = plugin.deregisterNode( EdPush::id );
    if (!status){
        status.perror( "deregisterNode" );
        return status;
    }
    return status;

}
