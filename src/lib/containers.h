
// small and efficient containers given by DragonEnergy on SO
// I don't know who you are, but I'm in your debt

// ************************************************************************************
// SmallList.hpp
// ************************************************************************************
#ifndef _CONTAINERS_LIB
#define _CONTAINERS_LIB

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>

namespace ed {

	// Stores a random-access sequence of elements similar to vector, but avoids
	// heap allocations for small lists. T must be trivially constructible and
	// destructible.
	template <class T>
	class SmallList
	{
	public:
		// Creates an empty list.
		SmallList();

		// Creates a copy of the specified list.
		SmallList(const SmallList& other);

		// Copies the specified list.
		SmallList& operator=(const SmallList& other);

		// Destroys the list.
		~SmallList();

		// Returns the number of agents in the list.
		int size() const;

		// Returns the nth element.
		T& operator[](int n);

		// Returns the nth element in the list.
		const T& operator[](int n) const;

		// Returns an index to a matching element in the list or -1
		// if the element is not found.
		int find_index(const T& element) const;

		// Clears the list.
		void clear();

		// Reserves space for n elements.
		void reserve(int n);

		// Inserts an element to the back of the list.
		void push_back(const T& element);

		/// Pops an element off the back of the list.
		T pop_back();

		// Swaps the contents of this list with the other.
		void swap(SmallList& other);

		// Returns a pointer to the underlying buffer.
		T* data();

		// Returns a pointer to the underlying buffer.
		const T* data() const;

	private:
		enum { fixed_cap = 256 };
		struct ListData
		{
			ListData();
			T buf[fixed_cap];
			T* data;
			int num;
			int cap;
		};
		ListData ld;
	};

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

		// Reserves space for n elements.
		void reserve(int n);

		// Swaps the contents of the two lists.
		void swap(FreeList& other);

	private:
		union FreeElement
		{
			T element;
			int next;
		};
		SmallList<FreeElement> data;
		int first_free;
	};

} // /ed

#endif _CONTAINERS_LIB