

#ifndef TECTONICSOLVER_H
#define TECTONICSOLVER_H

/*
Tectonic - a plate solver

Idea is a single physics environment for rigid mechanical systems,
relying on simple point positions where possible, and more advanced
constraints and connections where needed.


A Plate is a fundamental rigid unit, consisting of an Orient matrix
and vector Points around it in its space. (No mat hierarchy,
all matrices in world space).
Each Point may be paired with Point of an adjacent Plate.

In solve:
 - Each Point P_0 may have a different, independent goal P_G set
 - Best fitting rigid transform from Orient_0 to Orient_1 is found
 - Transform applied to base P_0 Points to give set of P_1
 - repeat

If you're wondering if this is worth it, consider that a proper system
such as this is a superset of literally every IK solver.

This sounds simple enough
also considered "Tetris - a block solver" but copyright


*/

#include <chrono>

#include "../lib/topo.h"
#include "../lib/containers.h"
#include <Eigen/Core>
#include "tecTypes.h"
#include "constraint.h"

namespace ed {

	struct TecSolver {
		// main solver object
	};



}




#endif
	