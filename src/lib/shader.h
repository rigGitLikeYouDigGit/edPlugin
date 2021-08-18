
#ifndef ED_LIBSHADER_H
#define ED_LIBSHADER_H

#include "api.h"

#include <string>

#include <gl/glew.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <math.h>
#include <io.h>



// maya stuff
#include <maya/MPxVertexBufferMutator.h>
#include <maya/MPxIndexBufferMutator.h>



namespace ed {

	
	//using namespace std;

	// test star shader data
	const float starData[] = { // right ascension, declination, magnitude
		0.5f, 0.5f, 4.3f, 0.1f, 0.4f, 5.2f,
		0.5f, 0.02f, 3.6f, 0.9f, 0.5f, 7.9f
	};
	

	static glm::mat4 mmatrixToMat4(const MMatrix& data) {
		glm::mat4 result;
		MFloatMatrix fData(data.matrix);
		memcpy(value_ptr(result), &fData.matrix, sizeof(fData));
		return result;
	}

	static std::string fileToChar(const char* filePath) {
		std::ifstream in(filePath, std::ifstream::in);
		std::string contents((std::istreambuf_iterator<char>(in)),
			std::istreambuf_iterator<char>());
		return contents;
	}
	//std::ifstream ifs("test.txt", std::ifstream::in)

	static std::string formatDefines(std::map<std::string, std::string> valueMap) {
		/* generates a block containing "#define" statements
		* able to be prepended to any GLSL scripts
		* allows easy access to constants defined in compiled plugin
		* 
		* numeric values should already be converted to strings
		*/
		std::string result("");
		for (auto const& it : valueMap) {
			std::string line("#define " + it.first + " " + it.second + "\n");
			result += line;
		}
		return result;

	}

}

#endif
