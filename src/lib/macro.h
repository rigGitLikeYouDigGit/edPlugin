// simple lines

#include "maya/MStreamUtils.h"
#ifndef API_MACROS
#define API_MACROS 1

// debug macros
#define COUT MStreamUtils::stdOutStream()
#define CERR MStreamUtils::stdErrorStream()

 // as in "debugString"
#define DEBUGS(info) \
COUT << __FILE__ << " " << __LINE__ << " \n" << info << std::endl;

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


// maths macros
#define EPS 0.0001

#define EQ(a, b) \
	(abs(a - b) < EPS)\

// to tune of twinkle twinkle little star
#define PI 3.141592653589

#endif
