#ifndef _SPACE_PARTITIONING_LIB
#define _SPACE_PARTITIONING_LIB


// various means of partitioning spatial data

#include <cstdint>
#include <vector>


// technology as handed down by the inimitable DragonEnergy

/// Provides an indexed free list with constant-time removals from anywhere
/// in the list without invalidating indices. T must be trivially constructible 
/// and destructible.
template <class T>
class FreeList
{
public:
	/// Creates a new free list.
	FreeList();

	/// Inserts an element to the free list and returns an index to it.
	int insert(const T& element);

	// Removes the nth element from the free list.
	void erase(int n);

	// Removes all elements from the free list.
	void clear();

	// Returns the range of valid indices.
	int range() const;

	// Returns the nth element.
	T& operator[](int n);

	// Returns the nth element.
	const T& operator[](int n) const;

private:
	union FreeElement
	{
		T element;
		int next;
	};
	std::vector<FreeElement> data;
	int first_free;
};

template <class T>
FreeList<T>::FreeList() : first_free(-1)
{
}

template <class T>
int FreeList<T>::insert(const T& element)
{
	if (first_free != -1)
	{
		const int index = first_free;
		first_free = data[first_free].next;
		data[index].element = element;
		return index;
	}
	else
	{
		FreeElement fe;
		fe.element = element;
		data.push_back(fe);
		return static_cast<int>(data.size() - 1);
	}
}

template <class T>
void FreeList<T>::erase(int n)
{
	data[n].next = first_free;
	first_free = n;
}

template <class T>
void FreeList<T>::clear()
{
	data.clear();
	first_free = -1;
}

template <class T>
int FreeList<T>::range() const
{
	return static_cast<int>(data.size());
}

template <class T>
T& FreeList<T>::operator[](int n)
{
	return data[n].element;
}

template <class T>
const T& FreeList<T>::operator[](int n) const
{
	return data[n].element;
}





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