#ifndef _SPACE_PARTITIONING_LIB
#define _SPACE_PARTITIONING_LIB


// various means of partitioning spatial data

#include <cstdint>
#include <vector>
#include "containers.h"

// technology as handed down by the inimitable DragonEnergy



struct QuadNode; // recursive quadNode piece
struct QuadTree; // entire quadTree
struct Element; // element lying within space partitioning
struct ElementNode; // representation of intersection of element with tree node


struct QuadNode
{
	// index of node
	int32_t index;

	// pointer to first child node if this is a branch,
	// or first elementNode if this is a leaf
	int32_t firstChild;

	// stores number of elements in leaf, or -1 if a branch
	int32_t count;
};
// contiguous memory layout - each quadNode is 12 bytes

// main class
struct QuadTree {
	std::vector<QuadNode> nodes; // one array for all nodes
	std::vector<Element> elements; // one array for all elements
};

// returns all leaf nodes in a quadTree
std::vector<QuadNode> findLeaves(
	const QuadTree& tree, 
	const QuadNode& root, 
	const int rect[4] // originX, originY, width, height
){
	std::vector<QuadNode> leaves, toProcess;
	toProcess.push_back(root);

	while (toProcess.size() > 0) // iterate over all nodes left to process
	{
		const QuadNode node = toProcess.back();
		toProcess.pop_back();

		if (tree.nodes[node.index].count != -1) { // if node has elements
			leaves.push_back(node);
		}
		else {

		}

	}


}







#endif // !_SPACE_PARTITIONING_LIB