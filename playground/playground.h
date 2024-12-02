#ifndef PLAYGROUND_H
#define PLAYGROUND_H

// Include GLEW
#include <GL/glew.h>

// Include GLM
#include <glm/glm.hpp>

//some global variables for handling the vertex buffer
GLuint vertexbuffer[2];
GLuint VertexArrayID;
GLuint vertexbuffer_size;

//program ID of the shaders, required for handling the shaders with OpenGL
GLuint programID;

//global variables to handle the MVP matrix
GLuint MatrixID;
glm::mat4 MVP;
GLuint MatrixID2;
glm::mat4 MVP2;
GLuint MatrixIDMV;
glm::mat4 MV;
GLuint MatrixIDMV2;
glm::mat4 MV2;
GLuint LightID;
glm::vec4 Lightsource;

float curr_x;
float curr_y;
float curr_angle;


int main( void ); //<<< main function, called at startup
void updateAnimationLoop(); //<<< updates the animation loop
bool initializeWindow(); //<<< initializes the window using GLFW and GLEW
bool updateMVPTransformation(); //<<< updates the MVP transform with the current pose
bool initializeVertexbuffer(); //<<< initializes the vertex buffer array and binds it OpenGL
bool cleanupVertexbuffer(); //<<< frees all recources from the vertex buffer
bool closeWindow(); //<<< Closes the OpenGL window and terminates GLFW


#endif