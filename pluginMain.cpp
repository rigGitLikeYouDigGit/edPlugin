/*
register all plugins
*/

#include "edPush.h"
#include "meshAnalysis.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>


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


MStatus initializePlugin( MObject obj ){

    MFnPlugin fnPlugin( obj, "edPush_by_ed", "1.0", "any");
    MStatus status;

    REGISTER_DEFORMER(EdPush);

    REGISTER_NODE(MeshAnalysis);


    return MS::kSuccess;

}

MStatus uninitializePlugin( MObject obj ){

    MStatus status;
    MFnPlugin fnPlugin(obj);

    DEREGISTER_NODE( EdPush );
    DEREGISTER_NODE( MeshAnalysis );

//    status = plugin.deregisterNode( EdPush::id );
//    if (!status){
//        status.perror( "deregisterNode" );
//        return status;
//    }
//    return status;
    return MS::kSuccess;
}
