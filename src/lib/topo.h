

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

inline int entryRealLength(std::vector<int> &buffer, int entryIndex) {
	// gives number of indices in entry other than -1
	// assumes regular entry length of 4
	DEBUGS("topo.h entryRealLength");
	int output = 0;
	for (int i = 0; i < 4; i++) {
		int val = buffer[entryIndex * 4 + i];
		output = (val < 0) ? output : output + 1;
	}
	return output;
}

inline std::vector<int> buildBufferOffsets(std::vector<int> &baseBuffer) {
	// returns new vector containing offsets into original
	//DEBUGS("topo.h buildBufferOffsets");

	int n = static_cast<int>(baseBuffer.size());
	std::vector<int> output(n);
	for (int i = 0; i < n; i++) {
		output[i] = entryRealLength(baseBuffer, i);
	}
	return output;
}


inline std::vector<int> entryFromBuffer(
	std::vector<int> &values,
	std::vector<int> &offsets,
	int entryIndex) {
	// use buffer indices to retrieve main values in entry
	vector<int> result;
	int startIndex = offsets[entryIndex];
	int endIndex;

	// check if entry is last
	if (entryIndex == offsets.size() - 1) {
		endIndex = static_cast<int>(values.size());
	}
	else {
		endIndex = offsets[entryIndex + 1];
	}
	for (int i = startIndex; i < endIndex;) {
		result.push_back(values[i]);
		i++;
	}
	return result;
}



inline std::tuple<std::vector<int>, std::vector<int>> pointBufferFromFaceBuffer( 
	std::vector<int> &faceBuffer, std::vector<int> &faceOffsets)
{
	/*  
	for now gives points in unordered entries, with no consistent winding order

	buffer entries are points connected to that index
	buffer_entry[i] = iA, iB, iC, iD

	*/
	//DEBUGS("topo.h pointBufferFromFaceBuffer");
	//DEBUGVI(faceBuffer);

	// maximum point is number of points
	int nPoints = *max_element(faceBuffer.begin(), faceBuffer.end()) + 1;
	//DEBUGS("nPoints" << nPoints);

	int nFaces = static_cast<int>(faceOffsets.size());
	//DEBUGS("nFaces" << nFaces) // works

	std::vector<int> pointConnects;
	std::vector<int> pointOffsets(nPoints, -1);

	// build set representing each point
	std::vector< std::set<int> > pointSets(nPoints);

	// iterate over faces in buffer
	for (int i = 0; i < nFaces; i++) {
		// iterate over points in face entry
		//DEBUGS("i " << i)

		std::vector<int> facePoints = entryFromBuffer(faceBuffer, faceOffsets, i);
		//DEBUGS("facePoints");
		//DEBUGVI(facePoints);

		// if triangle, add all other points
		int entryLength = 4;
		entryLength = static_cast<int>(facePoints.size());

		// gather connected points
		for (int n = 0; n < entryLength; n++) {
			//DEBUGS("faceIndex" << (facePoints[n]));
			int pointIndex = facePoints[n];

			//DEBUGS("pointIndex" << pointIndex);

			int left = (n - 1 + entryLength) % entryLength;
			int right = (n + 1) % entryLength;

			//DEBUGS("modulo wrapping : n " << n << " left " << left << " right " << right)

			pointSets[pointIndex].insert(facePoints[left]);
			pointSets[pointIndex].insert(facePoints[right]);
		}
	}
	// flatten pointSets to vector
	//DEBUGS("iterating points")
	std::set<int>::iterator it;
	int n = 0;

	for (int i = 0; i < nPoints; i++) 
	{
		// set offset index
		pointOffsets[i] = n;
		
		// iterate over connected point indices in set
		for (it = pointSets[i].begin(); it != pointSets[i].end(); ++it) {
			pointConnects.push_back(*it);
			n++;
		}
	}

	//return output;
	return std::make_tuple(pointConnects, pointOffsets);
}


// --- LAPLACIAN AND CHILL ---

inline Eigen::SparseMatrix<int> buildValenceMatrix( std::vector<int> &pointOffsets) {
	// aka degree matrix
	DEBUGS("topo.h buildValenceMatrix");

	int n = static_cast<int>(pointOffsets.size());
	Eigen::SparseMatrix<int> output(n, n);
	for (int i = 0; i < n; i++) {
		output.insert(i, i) = pointOffsets[i];
	}
	return output;
}

inline Eigen::SparseMatrix<int> buildAdjacencyMatrix(std::vector<int> &pointBuffer) {
	DEBUGS("topo.h buildAdjacencyMatrix");

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
	std::vector<int> &pointValences,
	int nPoints)
{
	DEBUGS("topo.h buildLaplaceMatrix");

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
	std::vector<int> pointConnects;
	std::vector<int> faceConnects;

	Topo(int nPointsIn) {
		// initialise arrays to -1
		pointConnects.assign(nPointsIn * 4, -1);
		faceConnects.assign(nPointsIn * 4, -1);
		nPoints = nPointsIn;
	}

	Topo(std::vector<int> pointConnectsIn, std::vector<int> faceConnectsIn) {
		// copy input arrays, Topo struct owns its values in memory
		pointConnects = pointConnectsIn;
		faceConnects = faceConnectsIn;
		nPoints = int(pointConnects.size());
	}


};


#endif