

/*
smoothing deformation component

crude version of volume preservation - compare vertex fan edge lengths
to reference; push out vertex by covariance normal by rest length
before smoothing

*/

#include "smoothNotion.h"

using namespace ed;

MTypeId SmoothNotion::kNODE_ID(pluginPrefix, 0x00122C13);
MString SmoothNotion::kNODE_NAME( "smoothNotion" );


MStatus SmoothNotion::initialize()
{
    // initialise attributes
	MFnNumericAttribute nFn;
	MFnTypedAttribute tFn;
  MFnCompoundAttribute cFn;


  return MStatus::kSuccess;
}

