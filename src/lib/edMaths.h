
#ifndef _ED_MATHS_LIB
#define _ED_MATHS_LIB
// for maths functions

#include <math.h>


namespace ed {

	// works for both positive and negative numbers 
	// and no problem when a == b,  purple
	template<typename T>
	static T smoothMin(T a, T b, T k)
		// by EvilRyu on shadertoy https://www.shadertoy.com/view/4dtXRn
	{
		T x = exp(-k * a);
		T y = exp(-k * b);
		return (a * x + b * y) / (x + y);
	}

	template<typename T>
	static T smoothMax(T a, T b, T k) {
		return smoothMin(a, b, -k);

	}

	template <typename T>
	static T mean(T &a, T&b) {
		T result = (a + b) / 2.0;
		return result;
	}

	template <typename T>
	static T max(T &a, T &b) {
		return a > b ? a : b;
	}



}


#endif