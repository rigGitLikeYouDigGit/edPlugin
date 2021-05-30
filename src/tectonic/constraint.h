

#ifndef TECTONICCONSTRAINT_H
#define TECTONICCONSTRAINT_H

/*
Specific constraint classes for Tectonic, used to define richer
relationships between points and plates.

Some like Collider are not actual constraints at all, just ways
of injecting extra information into the solve.

These should not do any computation outside of the actual solve

For MVP simple Goal constraint is enough.

Constraints may be static, exterior to solve, or also attached
to plates within the solve.

*/

#include "../lib/topo.h"
#include "tecTypes.h"

namespace ed {

	struct TecConstraint {

	};

	// Goal
	struct GoalConstraint : TecConstraint {
		/*
		Basic goal system for points, and main means of exterior
		forces interacting with	Tectonic system

		Either hard (weld) or soft (spring attraction)
		*/

	};

	// Collider
	struct ColliderConstraint : TecConstraint {
		/*
		Constraint to add polygon or SDF collider information to
		tectonic solve, in addition to the main plates' self collision
		*/
	};


	// Domain
	struct DomainConstraint : TecConstraint {
		/*
		Domain used to communicate runners on tracks,
		objects constrained to surfaces of others, etc

		Domain can itself be constrained to a plate
		*/
	};


}

#endif
	