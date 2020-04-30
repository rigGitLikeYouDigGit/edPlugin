

#ifndef TOPO_LIB
#define TOPO_LIB

#include <vector>
#include <set>

#include <Eigen/Core>
#include <Eigen/Sparse>

using namespace std;

// none of this is elegant
// please forgive


/* more complex operations done with eigen types,
to be converted to and from raw buffers at interfaces between software*/


// --- building buffers ---

inline int entryRealLength(vector<int> &buffer, int entryIndex) {
	// gives number of indices in entry other than -1
	// assumes entry length of 4
	int output = 0;
	for (int i = 0; i < 4; i++) {
		int val = buffer[entryIndex * 4 + i];
		output = (val < 0) ? output : output + 1;
	}
	return output;
}

inline vector<int> buildBufferOffsets(vector<int> &baseBuffer) {
	// returns new vector containing offsets into original
	// each offset corresponds to number of real indices in base entry
	int n = static_cast<int>(baseBuffer.size());
	vector<int> output(n);
	for (int i = 0; i < n; i++) {
		output[i] = entryRealLength(baseBuffer, i);
	}
	return output;
}

inline vector<int> pointBufferFromFaceBuffer( vector<int> &faceBuffer ) 
{
	/* expects face buffer with entries 4-long, -1 denoting no connection 
	not particularly performant, do not use online 
	for now gives points in unordered entries, with no consistent winding order
	*/

	//int nPoints = *max_element(faceBuffer.begin(), faceBuffer.end());
	int nPoints = static_cast<int>(faceBuffer.size());
	int nFaces = nPoints / 4;

	vector<int> output(nPoints, -1);
	//vector<int> output(nPoints);

	// build set representing each point
	vector< set<int> > pointSets(nPoints);

	// iterate over faces in buffer
	for (int i = 0; i < nFaces; i++) {
		// iterate over points in face entry


		set<int> faceSet;
		for (int n = 0; n < 4; n++) {
			faceSet.insert(faceBuffer[i * 4 + n]);
		}

		// if triangle, add all other points
		int entryLength = 4;
		if (faceSet.count(-1) > 0) {
			entryLength = 3;
		}

		for (int n = 0; n < 4; n++) {

			int pointIndex = faceBuffer[i * 4 + n];
			// skip index if it doesn't exist
			if (pointIndex < 0) {
				continue;
			}

			int left = (n - 1 + entryLength) % entryLength;
			int right = (n + 1) % entryLength;

			pointSets[pointIndex].insert(faceBuffer[i * 4 + left]);
			pointSets[pointIndex].insert(faceBuffer[i * 4 + right]);
		}
	}
	// flatten pointSets to vector
	set<int>::iterator it;
	for (int i = 0; i < nPoints; i++) 
	{
		int n = 0;
		for (it = pointSets[i].begin(); it != pointSets[i].end(); ++it) {
			output[i * 4 + n] = *it;
			n++;
		}
	}
	/* output point buffers also contain -1
	this is for ease, it may be that building corresponding offset buffers is more efficient
	than checking n > 0 when processing points at low level
	*/
	return output;
}


// --- LAPLACIAN AND CHILL ---

inline Eigen::SparseMatrix<int> buildValenceMatrix( vector<int> &pointOffsets) {
	// aka degree matrix
	int n = static_cast<int>(pointOffsets.size());
	Eigen::SparseMatrix<int> output(n, n);
	for (int i = 0; i < n; i++) {
		output.insert(i, i) = pointOffsets[i];
	}
	return output;
}

inline Eigen::SparseMatrix<int> buildAdjacencyMatrix(vector<int> &pointBuffer) {
	int n = static_cast<int>(pointBuffer.size());
	Eigen::SparseMatrix<int> output(n, n);

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < 4; j++) {
			int foundIndex = pointBuffer[i * 4 + j];
			// check if real
			if (foundIndex < 0) {
				continue;
			}
			// insert value of one at row i, column foundIndex
			output.insert(i, foundIndex) = 1;
		}
	}
	return output;
}


inline Eigen::SparseMatrix<int> buildLaplaceMatrix(
	Eigen::SparseMatrix<int> &adjacencyMatrix,
	vector<int> &pointValences,
	int nPoints)
{
	Eigen::SparseMatrix<int> output(nPoints, nPoints);

	output = adjacencyMatrix;
	// negate adjacency
	output *= -1;
	// add valences in diagonal
	for (int i = 0; i < nPoints; i++) {
		output.insert(i, i) = pointValences[i];
	}
    return output;
}

// struct Topo to represent mesh
// either builds buffers or accepts prebuilt
// assumes max vertex valence of 4

struct Topo {
	int nPoints;
	vector<int> pointConnects;
	vector<int> faceConnects;

	Topo(int nPointsIn) {
		// initialise arrays to -1
		pointConnects.assign(nPointsIn * 4, -1);
		faceConnects.assign(nPointsIn * 4, -1);
		nPoints = nPointsIn;
	}

	Topo(vector<int> pointConnectsIn, vector<int> faceConnectsIn) {
		// copy input arrays, Topo struct owns its values in memory
		pointConnects = pointConnectsIn;
		faceConnects = faceConnectsIn;
		nPoints = int(pointConnects.size());
	}


};


#endif