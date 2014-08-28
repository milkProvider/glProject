#pragma once

#include <glm\mat4x4.hpp>

class BShader {
public:
    BShader(); // Default constructor
    BShader(const char *vsFile, const char *fsFile); // Constructor for creating a shader from two shader filenames
    ~BShader(); // Deconstructor for cleaning up
    
    void init(const char *vsFile, const char *fsFile); // Initialize our shader file if we have to
    
    void bind(); // Bind our GLSL shader program
    void unbind(); // Unbind our GLSL shader program
    
    unsigned int id(); // Get the identifier for our program

	void setBoneMatrix(glm::mat4x4 pMatrix[61]);
	void setMVPMatrix(glm::mat4x4 pMVPMatrix);
private:
    unsigned int shader_id; // The shader program identifier
    unsigned int shader_vp; // The vertex shader identifier
    unsigned int shader_fp; // The fragment shader identifier

	unsigned int MVP_loc;
	unsigned int bones_loc;

    bool inited; // Whether or not we have initialized the shader


	void updateBoneMatrix(int pIndex, glm::mat4x4 pMatrix);
};
