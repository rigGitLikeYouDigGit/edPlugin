/*
register all plugins
*/

#include "edPush.h"

#include "meshAnalysis.h"
//#include "testDeformer.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

const char* kAUTHOR = "ed";
const char* kVERSION = "1.0";
const char* kREQUIRED_API_VERSION = "Any";

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

    MFnPlugin fnPlugin( obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);
    MStatus status = MStatus::kSuccess;

    status = REGISTER_DEFORMER(EdPush);

    status = REGISTER_NODE(MeshAnalysis);

    return status;
}

MStatus uninitializePlugin( MObject obj ){

    MStatus status;
    MFnPlugin fnPlugin(obj);

    //status = plugin.deregisterNode( EdPush::id );
//    status = plugin.deregisterNode( EdPush::kNODE_ID );
//    if (!status){
//        status.perror( "deregisterNode" );
//        return status;
//    }
    status = DEREGISTER_NODE( EdPush )
    return status;

}
