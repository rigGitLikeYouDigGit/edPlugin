// simple lines

#include "maya/MStreamUtils.h"
#ifndef API_MACROS
#define API_MACROS 1

// debug macros
#define COUT MStreamUtils::stdOutStream()
#define CERR MStreamUtils::stdErrorStream()

 // as in "debugString"
#define DEBUGS(info) \
COUT << info << std::endl;

// as in "debugVectorInt"
#define DEBUGVI(vec) \
for(auto const& i: vec){ \
	COUT << i << " "; \
} COUT << "length " << vec.size() << std::endl;

// as in "debugMVector"
#define DEBUGMV(vec) \
COUT << vec[0] << ", " << vec[1] << ", " << vec[2] << std::endl;

#define DEBUGVF(vec) \
copy( vec.begin(), vec.end(), ostream_iterator<float>(MStreamUtils::stdOutStream, " "));




#endif
