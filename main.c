#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "esUtil.h"

typedef struct
{
    GLFWwindow *window;

    GLint width;

    GLint height;

    // Handle to a program object
    GLuint programObject;

    // Handle to a framebuffer object
    GLuint fbo;

    // Texture handle
    GLuint colorTexId[4];

    // Texture size
    GLsizei textureWidth;
    GLsizei textureHeight;
} Data;

///
// Initialize the framebuffer object and MRTs
//
int InitFBO(Data *data)
{
    int i;
    GLint defaultFramebuffer = 0;
    const GLenum attachments[4] =
        {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3};

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebuffer);

    // Setup fbo
    glGenFramebuffers(1, &data->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, data->fbo);

    // Setup four output buffers and attach to fbo
    data->textureHeight = data->textureWidth = 400;
    glGenTextures(4, &data->colorTexId[0]);
    for (i = 0; i < 4; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, data->colorTexId[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     data->textureWidth, data->textureHeight,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        // Set the filtering mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachments[i],
                               GL_TEXTURE_2D, data->colorTexId[i], 0);
    }

    glDrawBuffers(4, attachments);

    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
        return FALSE;
    }

    // Restore the original framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);

    return TRUE;
}

///
// Initialize the shader and program object
//
int Init(Data *data)
{
    char vShaderStr[] =
        "#version 300 es                            \n"
        "layout(location = 0) in vec4 a_position;   \n"
        "void main()                                \n"
        "{                                          \n"
        "   gl_Position = a_position;               \n"
        "}                                          \n";

    char fShaderStr[] =
        "#version 300 es                                     \n"
        "precision mediump float;                            \n"
        "layout(location = 0) out vec4 fragData0;            \n"
        "layout(location = 1) out vec4 fragData1;            \n"
        "layout(location = 2) out vec4 fragData2;            \n"
        "layout(location = 3) out vec4 fragData3;            \n"
        "void main()                                         \n"
        "{                                                   \n"
        "  // first buffer will contain red color            \n"
        "  fragData0 = vec4 ( 1, 0, 0, 1 );                  \n"
        "                                                    \n"
        "  // second buffer will contain green color         \n"
        "  fragData1 = vec4 ( 0, 1, 0, 1 );                  \n"
        "                                                    \n"
        "  // third buffer will contain blue color           \n"
        "  fragData2 = vec4 ( 0, 0, 1, 1 );                  \n"
        "                                                    \n"
        "  // fourth buffer will contain gray color          \n"
        "  fragData3 = vec4 ( 0.5, 0.5, 0.5, 1 );            \n"
        "}                                                   \n";

    // Load the shaders and get a linked program object
    data->programObject = esLoadProgram(vShaderStr, fShaderStr);

    InitFBO(data);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return TRUE;
}

///
// Draw a quad and output four colors per pixel
//
void DrawGeometry(Data *data)
{
    GLfloat vVertices[] = {
        -1.0f,
        1.0f,
        0.0f,
        -1.0f,
        -1.0f,
        0.0f,
        1.0f,
        -1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    // Set the viewport
    glViewport(0, 0, data->width, data->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(data->programObject);

    // Load the vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices);
    glEnableVertexAttribArray(0);

    // Draw a quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

///
// Copy MRT output buffers to screen
//
void BlitTextures(Data *data)
{
    // set the fbo for reading
    glBindFramebuffer(GL_READ_FRAMEBUFFER, data->fbo);

    // Copy the output red buffer to lower left quadrant
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, data->textureWidth, data->textureHeight,
                      0, 0, data->width / 2, data->height / 2,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Copy the output green buffer to lower right quadrant
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glBlitFramebuffer(0, 0, data->textureWidth, data->textureHeight,
                      data->width / 2, 0, data->width, data->height / 2,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Copy the output blue buffer to upper left quadrant
    glReadBuffer(GL_COLOR_ATTACHMENT2);
    glBlitFramebuffer(0, 0, data->textureWidth, data->textureHeight,
                      0, data->height / 2, data->width / 2, data->height,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Copy the output gray buffer to upper right quadrant
    glReadBuffer(GL_COLOR_ATTACHMENT3);
    glBlitFramebuffer(0, 0, data->textureWidth, data->textureHeight,
                      data->width / 2, data->height / 2, data->width, data->height,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

///
// Render to MRTs and screen
//
void Draw(Data *data)
{
    GLint defaultFramebuffer = 0;
    const GLenum attachments[4] =
        {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3};

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebuffer);

    // FIRST: use MRTs to output four colors to four buffers
    glBindFramebuffer(GL_FRAMEBUFFER, data->fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawBuffers(4, attachments);
    DrawGeometry(data);

    // SECOND: copy the four output buffers into four window quadrants
    // with framebuffer blits

    // Restore the default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebuffer);
    BlitTextures(data);
}

static void onError(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void OnUpdate(Data *data)
{
    Draw(data);
    glfwSwapBuffers(data->window);
    glfwPollEvents();
}

int main(void)
{
    glfwSetErrorCallback(onError);

    if (!glfwInit())
    {
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    Data *data = (Data *)malloc(sizeof(Data));
    data->width = 400;
    data->height = 400;
    data->window = glfwCreateWindow(data->width, data->height, "emwebgl2", NULL, NULL);
    if (!data->window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(data->window);

    Init(data);

#if defined(EMSCRIPTEN)
    emscripten_set_main_loop_arg((em_arg_callback_func)OnUpdate, data, 0, true);
#else
    while (!glfwWindowShouldClose(window))
    {
        OnUpdate(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
#endif

    exit(0);
}