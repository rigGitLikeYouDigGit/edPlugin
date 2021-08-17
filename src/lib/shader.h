
#ifndef ED_LIBSHADER_H
#define ED_LIBSHADER_H

#include "api.h"

#include <gl/glew.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <math.h>



namespace ed {

	
	//using namespace std;
	

	static glm::mat4 mmatrixToMat4(const MMatrix& data) {
		using namespace glm;
		//MFloatMatrix fData(data);
		/*return mat4(
			data[0][0], data[0][1], data[0][2], data[0][3],
			data[1][0], data[1][1], data[1][2], data[1][3],
			data[2][0], data[2][1], data[2][2], data[2][3],
			data[3][0], data[3][1], data[3][2], data[3][3],
			);*/
		mat4 result;
		MFloatMatrix fData(data.matrix);
		memcpy(value_ptr(result), &fData.matrix, sizeof(fData));
		return result;
	}





}

#endif
