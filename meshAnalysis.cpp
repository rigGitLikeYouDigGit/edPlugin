
/*
all purpose node for deriving information from mesh surface and character -
curvature, tension, compression, motion vectors, etc,
storing information per point, outputting it as mask arrays

it would be AMAZING if this could somehow be made compatible with the gpu
deformation chain, but that is beyond my ability for now
*/

#include "meshAnalysis.h"

MTypeId MeshAnalysis::id(0x00122C05);
MString MeshAnalysis::nodeName( "edPush" );
MObject MeshAnalysis::aOffset;
MObject MeshAnalysis::aMask;
