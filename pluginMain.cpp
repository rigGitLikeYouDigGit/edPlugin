/*
register all plugins
*/

#include "edPush.h"
//#include "meshAnalysis.h"
//#include "testDeformer.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
//
//#define CHECK_MSTATUS(STATUS) \
//        if (!STATUS){ \
//        STATUS.perror( "error" ); \
//        } \
//        return STATUS; \


#define REGISTER_NODE(NODE) \
    status = fnPlugin.registerNode( \
        NODE::node_name, \
        NODE::id, \
        NODE::creator, \
        NODE::initialize \
    ); \
    CHECK_MSTATUS(status); \

#define DEREGISTER_NODE(NODE) \
    status = fnPlugin.deregisterNode( \
        NODE::id ); \
    CHECK_MSTATUS(status); \

/*
macros shamelessly lifted from yantor3d
thanks mate
*/


MStatus initializePlugin( MObject obj ){

    MFnPlugin fnPlugin( obj, "edPlugin_by_ed", "1.0", "any");
    MStatus status;

    status = fnPlugin.registerNode(
        EdPush::node_name,
        EdPush::id,
        EdPush::creator,
        EdPush::initialize);

    //REGISTER_NODE(EdPush);

    //REGISTER_NODE(MeshAnalysis);
    //REGISTER_NODE(TestDeformer)

    return status;
    //return MS::kSuccess;

}

MStatus uninitializePlugin( MObject obj ){

    MStatus status;
    MFnPlugin fnPlugin(obj);

    DEREGISTER_NODE( EdPush );
    //DEREGISTER_NODE( MeshAnalysis );

//    status = fnPlugin.deregisterNode( EdPush::node_id );
//    if (!status){
//        status.perror( "deregisterNode" );
//        return status;
//    }
    //DEREGISTER_NODE(TestDeformer)
    return status;
    //return MS::kSuccess;
}
