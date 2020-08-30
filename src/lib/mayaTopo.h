
#ifndef _MAYA_TOPO_H
#define _MAYA_TOPO_H 1

#include "topo.h"
#include "api.h"

// conversions between pure topo types and maya types
// it was just easier to manage this way

namespace ed{

//// TOPO TYPE FUNCTIONS
OffsetBuffer<int> faceBufferFromMfnMesh(MFnMesh& mfn) {
	// construct face buffer from mfn info
	int nPolys = mfn.numPolygons();
	std::vector<int> facePointConnects;
	std::vector<int> facePointOffsets(nPolys, -1);

	// vertex buffer from mfn
	MIntArray vertexCount; // number of vertices in face - NOT global offsets
	MIntArray vertexList; // list of vertices in each face
	mfn.getVertices(vertexCount, vertexList);

	int offsetIndex = 0;
	for (unsigned int i = 0; i < vertexCount.length(); i++) {
		// add offset to current index
		facePointOffsets[i] = offsetIndex;
		int nFacePoints = vertexCount[i];

		for (int n = 0; n < nFacePoints; n++) {
			facePointConnects.push_back(vertexList[offsetIndex]);
			offsetIndex++;
		}
	}
	return OffsetBuffer<int>(facePointConnects, facePointOffsets);
}


void HalfEdgeMeshFromMObject(HalfEdgeMesh& hedgeMesh, MObject meshObj, int build) {
	// updates target mesh struct from mesh MObject
	// if build, will rebuild topology buffers
	// if not, will only copy point positions
	MStatus s = MS::kSuccess;
	MFnMesh meshFn(meshObj);

	int nPoints = meshFn.numVertices();
	int nPolys = meshFn.numPolygons();
	hedgeMesh.nPoints = nPoints;
	hedgeMesh.nFaces = nPolys;

	if (build > 0) {
		OffsetBuffer<int> faceBuffer = faceBufferFromMfnMesh(meshFn);
		OffsetBuffer<int> pointBuffer = pointBufferFromFaceBuffer(faceBuffer);
		hedgeMesh.build(
			pointBuffer.values, pointBuffer.offsets,
			faceBuffer.values, faceBuffer.offsets
		);
	}
	// set mesh point positions
	const float* rawPositions = meshFn.getRawPoints(&s);
	//meshFn.getRawPoints(&s);
	//float test = rawPositions[7];
	std::vector<float> posVector(nPoints * 3, 0.0);
	for (int i = 0; i < nPoints; i++) {
		posVector[i*3] = rawPositions[i*3];
		posVector[i*3 + 1] = rawPositions[i*3 + 1];
		posVector[i*3 + 2] = rawPositions[i*3 + 2];

	}
	hedgeMesh.setPositions(posVector);
}

void meshFnFromHalfEdgeMesh(HalfEdgeMesh &hedgeMesh, MFnMesh &meshFn){
	// convert to point array
	MPointArray outputPoints(meshFn.numVertices());
	for (int i = 0; i < meshFn.numVertices(); i++) {
		outputPoints[i].x = hedgeMesh.pointPositions.values[i * 3];
		outputPoints[i].y = hedgeMesh.pointPositions.values[i * 3 + 1];
		outputPoints[i].z = hedgeMesh.pointPositions.values[i * 3 + 2];
	}
	meshFn.setPoints(outputPoints, MSpace::kObject);
}

} // namespace ed
#endif
