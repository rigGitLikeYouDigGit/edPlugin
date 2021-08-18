
#ifndef _ED_STL_LIB
#define _ED_STL_LIB 1

#include <vector>

namespace ed {

	template < typename T>
	inline void joinVectors(std::vector<T> & toExtend, std::vector<T> & toAdd) {
		toExtend.reserve(toExtend.size() + std::distance(toAdd.begin(), toAdd.end()));
		toExtend.insert(toExtend.end(), toAdd.begin(), toAdd.end());
	}


}


#endif