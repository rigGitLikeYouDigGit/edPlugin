
#ifndef _ED_TREE_LIB
#define _ED_TREE_LIB

#include <vector>
#include <string>
#include <map>
#include <cstdarg>

/*
At this point purely for interest, a mirrored version of the 
Tree structure in Python
Syntax and principles should be as closely matched as possible
Serialisation of one should be compatible with the other,
through either json or protobuffers

PERFORMANCE NOT CRITICAL YET

First thing to figure out : 
how to store varied types within the same tree?


*/

namespace ed {

	template<typename T = int> // maybe
	class Tree {
	public:
		// core components
		std::string name;
		T * value;
		Tree * parent;
		map<std::string, Tree* > branchMap;

		// constructors
		Tree(char* name, value = T* ) : name{ name }, value{ value } {
		}
		Tree(char* name, value = T*, parent=Tree* ) : 
			name{ name }, value{ value } {
			parent->addBranch(this);
		}

		// tree topo functions
		std::string addBranch(Tree* newBranch) {
			// stub for now, adding the same name replaces it
			branchMap[newBranch.name] = newBranch;
			newBranch->parent = this;
		}

		// querying functions
		Tree* operator()(std::string * keys) {
			// index into the tree via sequence of keys

		}

	private:

	};

}

#endif