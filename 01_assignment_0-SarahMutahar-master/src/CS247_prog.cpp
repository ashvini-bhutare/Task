// CS 247 - Scientific Visualization, KAUST
//
// Programming Assignment #0

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <iomanip>

#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

// framework includes
#include "glslprogram.h"

// glm header files
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define ROTATION_SPEED 1.0f
#define TRANSLATE_SPEED 0.1f
#define SCALE_FACTOR 0.1f


// window size
const unsigned int gWindowWidth = 512;
const unsigned int gWindowHeight = 512;

// callbacks
// TODO: add callbacks definitions (e.g., framebuffer callback, mouse callback, keyboard callback)

GLuint shaderProgramObject;
GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;
GLuint textureSamplerUniform;

glm::mat4 perspectiveProjectionMatrix;
glm::mat4 orthographicProjectionMatrix;

GLuint cubeVAO;
GLuint cubePositionVBO;
GLuint cubeTexcoordVBO;

vec3 cameraEye = vec3(0.0f, 0.0f, 6.0f);	// position
vec3 cameraLookat = vec3(0.0f, 0.0f, -1.0f); // front or lookat
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);		// up

// Checkerboard texture
void makeCheckerBoardImage();
const int checkImageHeight = 64;
const int checkImageWidth = 64;
unsigned char checkImage[checkImageHeight][checkImageWidth][3];
unsigned int checkImageTexture;

// cube rotation angle
GLfloat rotateAngle = 0.0f;
unsigned int currentTransform = 0; // default transform is set to translate

// default cube transformation values
glm::vec3 translateTranfromvector = glm::vec3(0.0f, 0.0f, -3.0f);
glm::vec3 scaleTranfromvector = glm::vec3(0.5f, 0.5f, 0.5f);
glm::vec3 rotateTranfromvector = glm::vec3(1.0f, 1.0f, 1.0f);

bool toggleProjection = true; // defualt perspective projection 

// scene configurations
// TODO: define glsl program, MVP matrices,
//  and any other needed variables and configurations

// glfw error callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mode);

// make checkerboard texture image
void makeCheckerBoardImage() {
    // TODO: make checkerboard texture
    // Hint:  make n consecutive 0s, then n consecutive 1s .. etc

    // variable declarations
    int c;

    //code
    for (int i = 0; i < checkImageHeight; i++)
    {
        for (int j = 0; j < checkImageWidth; j++)
        {
            c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;

            checkImage[i][j][0] = (GLubyte)c;
            checkImage[i][j][1] = (GLubyte)c;
            checkImage[i][j][2] = (GLubyte)c;
        }
    }
}

// Texture Parameterization
void initTexture() {
    makeCheckerBoardImage();
    glGenTextures(1, &checkImageTexture);
    glBindTexture(GL_TEXTURE_2D, checkImageTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // load texture to the generated checkerboard
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, checkImageWidth, checkImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
}
// init application
// - load application specific data
// - set application specific parameters
// - initialize stuff
bool initApplication(int argc, char **argv)
{

      // default initialization

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    // viewport
    glViewport(0, 0, gWindowWidth, gWindowHeight);
    perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 100.0f);

    return true;
}


// set up the scene: shaders, VAO, ..etc
void setup() {
    // vertex shader source code
    const GLchar* vertexShadersourcecode =
        "#version 330 core" \
        "\n" \
        "in vec4 a_position;" \
        "in vec2 a_texcoord;" \
        "uniform mat4 u_modelMatrix;" \
        "uniform mat4 u_viewMatrix;" \
        "uniform mat4 u_projectionMatrix;" \
        "out vec2 a_texcoord_out;" \
        "void main(void)" \
        "{" \
        "gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position;" \
        "a_texcoord_out = a_texcoord;" \
        "}";

    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShadersourcecode, NULL);

    //compile the code with inline compiler ...human understandable format to machine understandable format ..here machine means GPU
    glCompileShader(vertexShaderObject);

    //error checking in compilation
    GLint status;
    GLint infoLogLength;
    char* log = NULL;
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char*)malloc(infoLogLength);
            if (log != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, &written, log);
                std::cout << "vertex shader compilation log " << log << std::endl;
                free(log);
            }
        }
    }

    //fragment shader
    const GLchar* fragmentShadersourcecode =
        "#version 330 core" \
        "\n" \
        "in vec2 a_texcoord_out;" \
        "uniform sampler2D u_textureSampler;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
            "FragColor = texture(u_textureSampler,a_texcoord_out);" \
        "}";


    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShadersourcecode, NULL);
    glCompileShader(fragmentShaderObject);

    status = 0;
    infoLogLength = 0;
    log = NULL;

    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char*)malloc(infoLogLength);
            if (log != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, log);
                std::cout << "fragment shader compilation log " << log << std::endl;
                free(log);
            }
        }
    }

    //Shader program object
    shaderProgramObject = glCreateProgram();

    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    glBindAttribLocation(shaderProgramObject, 0, "a_position");
    glBindAttribLocation(shaderProgramObject, 1, "a_texcoord");

    glLinkProgram(shaderProgramObject);

    status = 0;
    infoLogLength = 0;
    log = NULL;

    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0)
        {
            log = (char*)malloc(infoLogLength);

            if (log != NULL)
            {
                GLsizei written;

                glGetProgramInfoLog(shaderProgramObject, infoLogLength, &written, log);
                std::cout << "shader program link log " << log << std::endl;
                free(log);
            }
        }
    }

    //post link - getting uniform location from shader program object
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "u_textureSampler");

    // make a cube
    const GLfloat cubePosition[] =
    {
        // top
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        // bottom
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,

         // front
         1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, 1.0f,

         // back
         1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

         // right
         1.0f, 1.0f, -1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,

         // left
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
    };


    const GLfloat cubeTexcoords[] =
    {
       0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    // set up a Vertex Buffer Object (VBO) and a Vertex Array Object (VAO)
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    //vbo for position
    glGenBuffers(1, &cubePositionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubePosition), cubePosition, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //vbo for texture
    glGenBuffers(1, &cubeTexcoordVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeTexcoordVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTexcoords), cubeTexcoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // set up the texture
    initTexture();

}

// render a frame
void render() {
    // code

    // clear buffer with given color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the shader program
    glUseProgram(shaderProgramObject);

    // update MVP matrices
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, translateTranfromvector);
    modelMatrix = glm::scale(modelMatrix, scaleTranfromvector);
    modelMatrix = glm::rotate(modelMatrix, rotateAngle, rotateTranfromvector);

    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    if (toggleProjection)
    {
        projectionMatrix = glm::perspective(45.0f, (float)gWindowWidth / (float)gWindowHeight, 0.1f, 100.0f);
    }      
    else
    {    
        // this codition is added to work code if size of window gets changed
        if (gWindowWidth <= gWindowHeight)
        {
            projectionMatrix = glm::ortho(-1.0f, 1.0f, (-1.0f * ((GLfloat)gWindowHeight / (GLfloat)gWindowWidth)), (1.0f * ((GLfloat)gWindowHeight / (GLfloat)gWindowWidth)), -5.0f, 5.0f);
        }
        else
        {
            projectionMatrix = glm::ortho((-1.0f * ((GLfloat)gWindowWidth / (GLfloat)gWindowHeight)), (1.0f * ((GLfloat)gWindowWidth / (GLfloat)gWindowHeight)), -1.0f, 1.0f, -5.0f, 5.0f);
        }
    } 

    // pass uniforms to shaders
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, checkImageTexture);
    glUniform1i(textureSamplerUniform, 0);

    // render the cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
    glBindVertexArray(0);
    
    glUseProgram(0);
}

// entry point
int main(int argc, char** argv)
{
     // init glfw
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // init glfw window
    GLFWwindow* window = glfwCreateWindow(gWindowWidth, gWindowHeight, "CS247 - Scientific Visualization", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // set GLFW callback functions
    // =============================================================================
    // TODO: read up on certain GLFW callbacks which we will need in the future.
    // Get an understanding for what a 'callback' is. Questions you should be able to answer include:
    // What is a callback? When is a callback called? How do you use a callback in your application?
    // What are typical examples for callbacks in the context of graphics applications?
    // Have a look at the following examples:
    //
    // glfwSetKeyCallback(window, YOUR_KEY_CALLBACK);
    // glfwSetFramebufferSizeCallback(window, YOUR_FRAMEBUFFER_CALLBACK);
    // glfwSetMouseButtonCallback(window, YOUR_MOUSEBUTTON_CALLBACK);
    // glfwSetCursorPosCallback(window, YOUR_CURSORPOSCALL_BACK);
    // glfwSetScrollCallback(window, YOUR_SCROLL_CALLBACK);
    // ...

    // Implement mouse and keyboard callbacks!
    // Print information about the events on std::cout
    // =============================================================================

    // make context current (once is sufficient)
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetKeyCallback(window, keyCallBack);

    // init the OpenGL API (we need to do this once before any calls to the OpenGL API)
    gladLoadGL();


    //init our application
    if (!initApplication(argc, argv)) {
        glfwTerminate();
         exit(EXIT_FAILURE);
    }


    // set up the scene
    setup();

    // start traversing the main loop
    // loop until the user closes the window
  

    while (!glfwWindowShouldClose(window))
    {
        // input
        //processInput(window);

        // render one frame
        render();

        // swap front and back buffers
        glfwSwapBuffers(window);

        // poll and process input events (keyboard, mouse, window, ...)
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

// keyboard call back function
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // translate
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        currentTransform = 0;
    }

    // scale
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        currentTransform = 1;
    }

    // rotation
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        currentTransform = 2;
    }
       
    // for x axis operation
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        if (currentTransform == 0)
        {
            if(mode == GLFW_MOD_SHIFT) // upper case
            {
                translateTranfromvector.x += TRANSLATE_SPEED;
                std::cout << "translation in x direction : " << translateTranfromvector.x << std::endl;
            }
            else // lower case
            {
                translateTranfromvector.x -= TRANSLATE_SPEED;
                std::cout << "translation in x direction : " << translateTranfromvector.x << std::endl;
            }
        }
        else if (currentTransform == 1)
        {
            if (mode == GLFW_MOD_SHIFT) // upper case
            {
                scaleTranfromvector.x += SCALE_FACTOR;
                std::cout << "scale in x direction : " << scaleTranfromvector.x << std::endl;
            }
            else
            {
                scaleTranfromvector.x -= SCALE_FACTOR;
                std::cout << "scale in x direction : " << scaleTranfromvector.x << std::endl;
            }
        }
        else if (currentTransform == 2)
        {
            if (mode == GLFW_MOD_SHIFT) // upper case
            {
                rotateAngle += ROTATION_SPEED;
                rotateTranfromvector.x = 1.0f;
                std::cout << "rotation in x direction : " <<std::fixed << std::setprecision(6)<< rotateAngle << std::endl;
            }
            else
            {
                rotateAngle -= ROTATION_SPEED;
                rotateTranfromvector.x = 1.0f;
                std::cout << "rotation in x direction : " << std::fixed << std::setprecision(6) << rotateAngle << std::endl;
            }
        }
        
    }
      
    // for y axis operation
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        if (currentTransform == 0)
        {
            if (mode == GLFW_MOD_SHIFT) // upper case
            {
                translateTranfromvector.y += TRANSLATE_SPEED;
                std::cout << "translation in y direction : " << translateTranfromvector.y << std::endl;
            }
            else
            {
                translateTranfromvector.y -= TRANSLATE_SPEED;
                std::cout << "translation in y direction : " << translateTranfromvector.y << std::endl;
            }
        }
        else if (currentTransform == 1)
        {
            if (mode == GLFW_MOD_SHIFT) // upper case
            {
                scaleTranfromvector.y += SCALE_FACTOR;
                std::cout << "scale in y direction : " << scaleTranfromvector.y << std::endl;
            }
            else
            {
                scaleTranfromvector.y -= SCALE_FACTOR;
                std::cout << "scale in y direction : " << scaleTranfromvector.y << std::endl;
            }
        }
        else if (currentTransform == 2)
        {
            if (mode == GLFW_MOD_SHIFT) // upper case
            {
                rotateAngle += ROTATION_SPEED;
                rotateTranfromvector.y = 1.0f;
                std::cout << "rotation in y direction : " << std::fixed << std::setprecision(6) << rotateAngle << std::endl;
            }
            else
            {
                rotateAngle -= ROTATION_SPEED;
                rotateTranfromvector.y = 1.0f;
                std::cout << "rotation in y direction : " << std::fixed << std::setprecision(6) << rotateAngle << std::endl;
            }
        }       
    }

    // for z axis operation
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        if (currentTransform == 0)
        {
            if (mode == GLFW_MOD_SHIFT) // upper case
            {
                translateTranfromvector.z += TRANSLATE_SPEED;
                std::cout << "translation in z direction : " << translateTranfromvector.z << std::endl;
            }
            else
            {
                translateTranfromvector.z -= TRANSLATE_SPEED;
                std::cout << "translation in x direction : " << translateTranfromvector.z << std::endl;
            }
        }
        else if (currentTransform == 1)
        {
            if (mode == GLFW_MOD_SHIFT) // upper case
            {
                scaleTranfromvector.z += SCALE_FACTOR;
                std::cout << "scale in z direction : " << scaleTranfromvector.z << std::endl;
            }
            else
            {
                scaleTranfromvector.z -= SCALE_FACTOR;
                std::cout << "scale in z direction : " << scaleTranfromvector.z << std::endl;
            }
        }
        else if (currentTransform == 2)
        {
            if (mode == GLFW_MOD_SHIFT) // upper case
            {
                rotateAngle += ROTATION_SPEED;
                rotateTranfromvector.z = 1.0f;
                std::cout << "rotation in z direction : " << std::fixed << std::setprecision(6) << rotateAngle << std::endl;
            }
            else
            {
                rotateAngle -= ROTATION_SPEED;
                rotateTranfromvector.z = 1.0f;
                std::cout << "rotation in z direction : " << std::fixed << std::setprecision(6) << rotateAngle << std::endl;
            }
        }        
    }

    // for perspective projection 
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        toggleProjection = true;
        std::cout << "persepctive projection" << std::endl;
    }
        
    // for orthographic projection 
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        toggleProjection = false;
        std::cout << "orthographic projection" << std::endl;
           
    }        
}

// framebuffer size call back function
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);

}