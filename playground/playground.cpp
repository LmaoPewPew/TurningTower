#include "playground.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
# define PI 3.14159265358979323846  /* pi */

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

int main(void)
{
    //Initialize window
    bool windowInitialized = initializeWindow();
    if (!windowInitialized) return -1;

    //Initialize vertex buffer
    bool vertexbufferInitialized = initializeVertexbuffer();
    if (!vertexbufferInitialized) return -1;

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

    MatrixIDMV = glGetUniformLocation(programID, "MV");
    MatrixIDMV2 = glGetUniformLocation(programID, "MV2");
    MatrixID = glGetUniformLocation(programID, "MVP");
    MatrixID2 = glGetUniformLocation(programID, "MVP2");
    LightID = glGetUniformLocation(programID, "Lightsource");
    updateMVPTransformation();

    //initialize verte variables
    curr_x = 0;
    curr_y = 0;
    curr_angle = 0.0007;

    Lightsource = vec4(8.0, 8.0, 8.0, 1.0);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    //start animation loop until escape key is pressed
    do {

        updateAnimationLoop();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    //Cleanup and close window
    cleanupVertexbuffer();
    glDeleteProgram(programID);
    closeWindow();

    return 0;
}

void updateAnimationLoop()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    curr_x = 0.0f; curr_y = 0.0f;
    
    // Update the MVP transformation with the new values
    updateMVPTransformation();

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP2[0][0]);
    glUniformMatrix4fv(MatrixIDMV, 1, GL_FALSE, &MV[0][0]);
    glUniformMatrix4fv(MatrixIDMV2, 1, GL_FALSE, &MV2[0][0]);
    glUniform4fv(LightID, 1, &Lightsource[0]);


    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
    glVertexAttribPointer(
        0,               
        3,                
        GL_FLOAT,           
        GL_FALSE,          
        0,                  
        (void*)0          
    );

    // 2nd attribute buffer : normals
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
    glVertexAttribPointer(
        1,                 
        3,                 
        GL_FLOAT,         
        GL_FALSE,          
        0,                 
        (void*)0            
    );

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, vertexbuffer_size * 3); // 3 indices starting at 0 -> 1 triangle

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool initializeWindow()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Tower", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return false;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
    return true;
}

bool updateMVPTransformation()
{
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 50.0f);

    static glm::vec3 eye = glm::vec3(0, 3, 10); // Camera is at (0,0,0), in World Space
    static glm::vec3 look = glm::vec3(0, 3, 0); // and looks at the origin
    static glm::vec3 up = glm::vec3(0, 1, 0); // Head is up (set to 0,-1,0 to look upside-down)

    // Apply movement restrictions to the eye position
    if (!((eye[1] < -5 && curr_y < 0) || (eye[1] > 11 && curr_y > 0))) {
        eye += vec3(0.0, curr_y * eye[2], -curr_y * (eye[1] - 3));
    }
    eye += vec3(curr_x * eye[2], 0.0, -curr_x * eye[0]);

    // Camera matrix (modify this to let the camera move)
    glm::mat4 View = glm::lookAt(eye, look, up);

    // Model matrix, add translation for movement
    static glm::mat4 Model = glm::mat4(1.0f); // start with identity matrix
    Model = glm::rotate(Model, curr_angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Apply rotation (last parameter: axis)
    Model = glm::translate(Model, glm::vec3(curr_x, 0.0f, curr_y)); // Apply translation (move along x and y)

    // Model2 matrix with similar transformation (optional for another object or instance)
    static glm::mat4 Model2 = glm::mat4(1.0f); // start with identity matrix
    Model2 = glm::rotate(Model2, -curr_angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Apply rotation
    Model2 = glm::translate(Model2, glm::vec3(curr_x, 0.0f, curr_y)); // Apply translation

    // Our ModelViewProjection: multiplication of our 3 matrices
    MVP = Projection * View * Model;  // Model -> View -> Projection
    MVP2 = Projection * View * Model2; // Model2 -> View -> Projection
    MV = View * Model; // Also need MV for light transformation
    MV2 = View * Model2;

    return true;
}

bool initializeVertexbuffer()
{
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    vertexbuffer_size = 36; 
   
    double vertX = 2.0f;
    double vertY = 5.0f;
    double vertZ = 2.0f;

    GLfloat vertex_data[] = {
        // Front face
        -vertX, -vertY,  vertZ,
         vertX, -vertY,  vertZ,
         vertX,  vertY,  vertZ,

         vertX,  vertY,  vertZ,
        -vertX,  vertY,  vertZ,
        -vertX, -vertY,  vertZ,

        // Back face
        -vertX, -vertY, -vertZ,
        -vertX,  vertY, -vertZ,
         vertX,  vertY, -vertZ,

         vertX,  vertY, -vertZ,
         vertX, -vertY, -vertZ,
        -vertX, -vertY, -vertZ,

        // Left face
        -vertX, -vertY, -vertZ,
        -vertX, -vertY,  vertZ,
        -vertX,  vertY,  vertZ,

        -vertX,  vertY,  vertZ,
        -vertX,  vertY, -vertZ,
        -vertX, -vertY, -vertZ,

        // Right face
         vertX, -vertY, -vertZ,
         vertX, -vertY,  vertZ,
         vertX,  vertY,  vertZ,

         vertX,  vertY,  vertZ,
         vertX,  vertY, -vertZ,
         vertX, -vertY, -vertZ,

         // Top face
         -vertX,  vertY, -vertZ,
          vertX,  vertY, -vertZ,
          vertX,  vertY,  vertZ,

          vertX,  vertY,  vertZ,
         -vertX,  vertY,  vertZ,
         -vertX,  vertY, -vertZ,

         // Bottom face
         -vertX, -vertY, -vertZ,
         -vertX, -vertY,  vertZ,
          vertX, -vertY,  vertZ,

          vertX, -vertY,  vertZ,
          vertX, -vertY, -vertZ,
         -vertX, -vertY, -vertZ,
    };

    // Define normal vectors for each face
    GLfloat normal_data[] = {
        // Front face
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,

        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,

        // Back face
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,

        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,

        // Left face
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,

        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,

        // Right face
        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,

        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,

        // Top face
        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,

        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,

        // Bottom face
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,

        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
    };

    glGenBuffers(2, vertexbuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normal_data), normal_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    return true;
}

bool cleanupVertexbuffer()
{
    // Cleanup VBO
    glDeleteVertexArrays(1, &VertexArrayID);
    return true;
}

bool closeWindow()
{
    glfwTerminate();
    return true;
}