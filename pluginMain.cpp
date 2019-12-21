/*
register all plugins
*/

#include "edPush.h"
#include "meshAnalysis.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

//#define CHECK_MSTATUS_AND_RETURN_IT(STATUS, msg)
//        if (!STATUS){
//        STATUS.perror( msg );
//        return STATUS;
//    }

//MTypeId EdPush::kNODE_ID = 0x00126b05;
//MString EdPush::kNODE_NAME = "edPush";

#define REGISTER_NODE(NODE) \
    status = fnPlugin.registerNode( \
        NODE::kNODE_NAME, \
        NODE::kNODE_ID, \
        NODE::creator, \
        NODE::initialize \
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

    REGISTER_NODE(EdPush);

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
