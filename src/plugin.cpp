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
#include "tectonicConstraintNode.h"

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

//static std::unordered_set<MPxNode> pluginObjects;

#define REGISTER_NODE(NODE) \
    s = fnPlugin.registerNode( \
        NODE::kNODE_NAME, \
        NODE::kNODE_ID, \
        NODE::creator, \
        NODE::initialize \
    ); \
    MCHECK(s, "cannot register node " << NODE::kNODE_NAME); \
    /*CHECK_MSTATUS_AND_RETURN_IT(s); \*/
    //pluginObjects.insert(NODE);\

#define REGISTER_DEFORMER(NODE) \
    s = fnPlugin.registerNode( \
        NODE::kNODE_NAME, \
        NODE::kNODE_ID, \
        NODE::creator, \
        NODE::initialize, \
        MPxNode::Type::kDeformerNode \
    ); \
    MCHECK(s, "cannot register deformer " << NODE::kNODE_NAME); \


#define DEREGISTER_NODE(NODE) \
    s = fnPlugin.deregisterNode( \
        NODE::kNODE_ID ); \
    MCHECK(s, "failed to deregister " << NODE::kNODE_NAME); \

#define REGISTER_NODE_TYPE(NODE, NODE_TYPE) \
    s = fnPlugin.registerNode( \
        NODE::kNODE_NAME, \
        NODE::kNODE_ID, \
        NODE::creator, \
        NODE::initialize, \
        NODE_TYPE \
    ); \
    CHECK_MSTATUS_AND_RETURN_IT(s); \
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
    MStatus s = MStatus::kSuccess;
    //int s = MPxNode::kDependNode;
    /*s = MPxNode::schedulingType(;*/
    // deformers
    //s = REGISTER_DEFORMER(EdPush);

 //   // gpu gubs

 //   // normal nodes 
 //   //s = REGISTER_NODE(MeshAnalysis);



 //   // BEGIN PROCEDURAL CONTROL REGISTER

    EdPush::kNODE_ID = (0x00122C00);
    EdPush::kNODE_NAME = ("edPush");
    //s = REGISTER_DEFORMER(EdPush);

    MeshToBuffers::kNODE_ID = (0x00122C01); 
    MeshToBuffers::kNODE_NAME = ("meshToBuffers");
    s = REGISTER_NODE(MeshToBuffers);

	

    MultiMod::kNODE_ID = (0x00122C02);
	//status = REGISTER_DEFORMER(MultiMod);

    //// memory cell
    //MemorySource::kNODE_ID = (0x00122C04);
    //s = REGISTER_NODE(MemorySource);
    //MemorySink::kNODE_ID = (0x00122C05);
    //s = REGISTER_NODE(MemorySink);


    // tectonic
    TectonicNode::kNODE_ID = (0x00122C08);
    s = REGISTER_NODE(TectonicNode);
    CHECK_MSTATUS_AND_RETURN_IT(s);
    
    TectonicConstraintNode::kNODE_ID = (0x00122C10);
    s = REGISTER_NODE(TectonicConstraintNode);
    
    //s = REGISTER_DEFORMER(UberDeformer);
    //s = REGISTER_NODE(DeformerNotion);

    //s = REGISTER_NODE(CurveFrame);

    //

    // END PROCEDURAL CONTROL REGISTER


    return s;
}

MStatus uninitializePlugin( MObject obj ){

    MStatus s;
    MFnPlugin fnPlugin(obj);

    //status = DEREGISTER_NODE( EdPush );
    //status = DEREGISTER_NODE( MeshAnalysis );
    // BEGIN PROCEDURAL CONTROL DEREGISTER
    s = DEREGISTER_NODE( MeshToBuffers );
	//status = DEREGISTER_NODE(UberDeformer);
	//status = DEREGISTER_NODE(DeformerNotion);

        //status = DEREGISTER_NODE(CurveFrame);
    //status = DEREGISTER_NODE(MultiMod);


	//status = DEREGISTER_NODE(MemorySource);
	//status = DEREGISTER_NODE(MemorySink);


    s = DEREGISTER_NODE(TectonicNode);
    s = DEREGISTER_NODE(TectonicConstraintNode);
 // 
	//status = DEREGISTER_NODE(DirectDeltaMush);
	////status = fnPlugin.deregisterNode(RefDDM::kNODE_ID);
 //   // END PROCEDURAL CONTROL DEREGISTER

    return s;

}
