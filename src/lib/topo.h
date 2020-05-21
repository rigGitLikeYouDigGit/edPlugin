

#ifndef TOPO_LIB
#define TOPO_LIB

#include <vector>
#include <set>
#include <algorithm>

#include <Eigen/Core>
#include <Eigen/Sparse>


// none of this is elegant
// please forgive


/* more complex operations done with eigen types,
to be converted to and from raw buffers at interfaces between software*/

// size_t sucks
#define INT(sizeVar) static_cast<int>(sizeVar)

template <typename T>
inline int index(const std::vector<T> &vec, const T &element) {
	// returns index in array or -1 if not found
	int result = -1;
	auto it = std::find(vec.begin(), vec.end(), element);
	if (it != vec.end()) {
		result = static_cast<int>(distance(vec.begin(), it));
	}
	return result;
}

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
	std::vector<int> result;
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


// --- HALF EDGE MESH STRUCTURE ---


struct IndexedComponent {
	// base class for index comparison
	int index;
	bool operator==(const IndexedComponent& other) {
		// common sense rules - don't compare face with edge
		return (index == other.index);
	}
};


// topology types, no spatial info
struct Point; // point in space
struct Vertex; // unique vertex, split by face
struct Face; // polygonal face
struct Edge; // undirected shared edge
struct HalfEdge; // directed unique edge


struct Point : IndexedComponent {
	std::vector<Vertex*> vertices; // vertices owned by this point, maybe ordered
	std::vector<Edge*> edges; // edges owned by this point, maybe ordered
	std::vector<Face*> faces; // faces to which this point belongs
};

struct Vertex : IndexedComponent {
	Point* point; // point to which this vertex belongs
	Face* face; // face to which this vertex belongs
	Edge* edges[2]; // previous and next edges
	HalfEdge* hedges[2]; // previous and next halfEdges
};

struct Face : IndexedComponent {
	std::vector<Point*> points; // points in this face
	std::vector<Edge*> edges; // edges shared by this face
	std::vector<HalfEdge*> hedges; // ordered half edges
	std::vector<Vertex*> vertices; // ordered vertices
};

struct Edge : IndexedComponent {
	Point* points[2]; // connected points, unordered
	std::vector<Face*> faces; // faces which share this edge
	std::vector<HalfEdge*> hedges; // hedges owned by this edge

};

struct HalfEdge : IndexedComponent {
	Vertex* vertices[2]; // unique source and sink vertices
	Point* points[2]; // source and sink points to which vertices belong
	HalfEdge* next; // next half edge in polygon
	HalfEdge* prev; // previous half edge in polygon
	Edge* edge; // edge to which this halfEdge belongs
	Face* face; // face to which this halfEdge belongs
	std::vector<HalfEdge*> equivalents; // equivalent half edges (lying along the same edge)
};

// topo operations
//int isAdjacent(IndexedComponent &a, IndexedComponent &b) {
//	// returns 1 if order is a -> b or no order, -1 if b <- a, 0 otherwise
//	return 0;
//}

struct HalfEdgeMesh {
	// half-edge data structure
	// also includes some extra features
	// we follow the pattern of points, vertices, faces, edges, half-edges
	// explored in Keenan Crane Discrete Differential Geometry
	// this also largely lines up with the Houdini model, except full edges exist

	// one point -> many vertices
	// one edge -> many half-edges

	// topo arrays - main mesh struct contains full object sequence
	// arrays of structs, rip
	std::vector<Point> points;
	std::vector<Vertex> vertices;
	std::vector<Face> faces;
	std::vector<Edge> edges;
	std::vector<HalfEdge> hedges;

	int nPoints;
	int nFaces;
	int nVertices;
	int nEdges;
	int nHalfEdges;

	// topo raw buffers
	std::vector<int> pointConnects; // points to points
	std::vector<int> pointOffsets;

	std::vector<int> faceConnects; // faces to points
	std::vector<int> faceVertexConnects; // faces to vertices
	std::vector<int> faceOffsets;

	// spatial information
	std::vector<float> pointPositions;
	std::vector<float> pointNormals;
	std::vector<float> faceNormals;


	HalfEdgeMesh() {
	}

	void build(
		std::vector<int> &pointConnects,
		std::vector<int> &pointOffsets,
		std::vector<int> &faceConnects,
		std::vector<int> &faceOffsets,
		std::vector<float> &pointPositions
	) {
		// main method to build half-edge representation from raw buffers
		nPoints = static_cast<int>(pointOffsets.size());
		nFaces = static_cast<int>(faceOffsets.size());
		nVertices = static_cast<int>(faceConnects.size());
		// build points
		for (int i = 0; i < nPoints; i++) {
			Point p = Point();
			p.index = i;
			points.push_back(p);
		}

		// build vertices
		for (int i = 0; i < nVertices; i++) {
			Vertex v = Vertex();
			v.index = i;
			vertices.push_back(v);
		}


		int n = 0;
		// loop over faces
		for (int i = 0; i < nFaces; i++) {

			Face f = Face();
			f.index = i;

			// need to guarantee order, set for vertices for now
			std::set<Vertex*> faceVertices;

			// get points in face
			std::vector<int> facePoints = entryFromBuffer(
				faceConnects, faceOffsets, i);

			// iterate over points in face
			for (int j = 0; j < static_cast<int>(facePoints.size()); j++) {
				// set references
				Point p = points[facePoints[j]];
				f.points.push_back( &p );
				p.faces.push_back(&f);

				// vertices
				Vertex v = vertices[n];
				p.vertices.push_back(&v);
				v.face = &f;
				v.point = &p;

				faceVertices.insert(&v);

				n++;
			}

			// ensure that vertices are ordered correctly
			// check for contiguous ring in pointConnects
			std::vector<Vertex*> orderedVertices;
			Vertex *vStart = *faceVertices.begin();
			faceVertices.erase(faceVertices.begin());
			orderedVertices.push_back(vStart);

			// found index must be in previous connects to be allowed
			int prevPointIndex = vStart->point->index;

			int nIterations = 0;

			while (!faceVertices.empty()) {
				// check if stuck in loop
				if (nIterations > INT(faceVertices.size())) {
					DEBUGS("ordering vertices loops indefinitely, buffers are non manifold");
					break;
				}
				Vertex *testV;
				int testPointIndex;
				for (auto &it : faceVertices) {

					std::vector<int> connectedIndices = entryFromBuffer(
						pointConnects, pointOffsets, prevPointIndex);

					testV = it;
					testPointIndex = testV->index;
					// check if test index is in connected points
					if (index(connectedIndices, testPointIndex) > -1) {
						break;
					}
				}
				orderedVertices.push_back(testV);
				prevPointIndex = testPointIndex;
				faceVertices.erase(testV);
			}

			// assign ordered vertices
			for (auto &v : orderedVertices) {
				f.vertices.push_back(v);
			}
			// assuming there is a better way than this abysmal effort

		}

		// for half edges a continuous ring of points around face is needed

		
	}

};




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