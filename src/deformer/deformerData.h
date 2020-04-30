
#ifndef DEFORMERDATA_H
#define DEFORMERDATA_H

#include "lib/api.h"
#include "lib/topo.h"

#include <vector>
#include <set>

// deformer data structs for notion system

/* values that will change with every iteration,
to be updated by the deformation functions */
struct MeshLiveData {

	std::vector<float> positions; // vertex positions
	std::vector<float> normals; // vertex normals

	void clear(){
		// reset arrays
		positions.clear();
		normals.clear();
	}

	MeshLiveData(int nPoints) {
		// initialise arrays
		positions.assign(nPoints * 3, 0.0);
		normals.assign(nPoints * 3, 0.0);
	}

};

/* values like curvature which are generated from the mesh,
which may inform deformation functions, but will not
be affected by the functions*/
struct MeshAuxData;

/* values specific to deformation function, effectively
function arguments*/
struct DeformParametres;





#endif