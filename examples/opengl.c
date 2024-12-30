// Taken from https://github.com/theandrew168/sdl2-opengl-demo
//
// Copyright (c) 2020 Andrew Dailey
//
// Slightly modified to be one-file and work in CJIT by Jaromil
//
// MIT License
//
// Tutorial explanation:
// https://shallowbrooksoftware.com/posts/a-multi-platform-modern-opengl-demo-with-sdl2/

#pragma comment(lib, "SDL2")

#ifdef _WIN32
#pragma comment(lib, "opengl32")
#elif LINUX
#pragma comment(lib, "OpenGL")
#else
#pragma comment(lib, "OpenGL")
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// List of required OpenGL functions and their corresponding typedefs (defined
// somewhere under <SDL2/SDL_opengl.h>. The "OPENGL_FUNCTIONS" macro will do
// different things in different locations. Each of these functions requires
// three pieces of code: an initial declaration, an initial definition, and a
// dynamic load assignment.
//
// More info about dynamic loading can be found here:
// https://en.wikipedia.org/wiki/Dynamic_loading
#define OPENGL_FUNCTIONS                                                            \
    OPENGL_FUNCTION(glCreateShader, PFNGLCREATESHADERPROC)                          \
    OPENGL_FUNCTION(glDeleteShader, PFNGLDELETESHADERPROC)                          \
    OPENGL_FUNCTION(glAttachShader, PFNGLATTACHSHADERPROC)                          \
    OPENGL_FUNCTION(glDetachShader, PFNGLDETACHSHADERPROC)                          \
    OPENGL_FUNCTION(glShaderSource, PFNGLSHADERSOURCEPROC)                          \
    OPENGL_FUNCTION(glCompileShader, PFNGLCOMPILESHADERPROC)                        \
    OPENGL_FUNCTION(glGetShaderiv, PFNGLGETSHADERIVPROC)                            \
    OPENGL_FUNCTION(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC)                  \
    OPENGL_FUNCTION(glCreateProgram, PFNGLCREATEPROGRAMPROC)                        \
    OPENGL_FUNCTION(glDeleteProgram, PFNGLDELETEPROGRAMPROC)                        \
    OPENGL_FUNCTION(glUseProgram, PFNGLUSEPROGRAMPROC)                              \
    OPENGL_FUNCTION(glLinkProgram, PFNGLLINKPROGRAMPROC)                            \
    OPENGL_FUNCTION(glValidateProgram, PFNGLVALIDATEPROGRAMPROC)                    \
    OPENGL_FUNCTION(glGetProgramiv, PFNGLGETPROGRAMIVPROC)                          \
    OPENGL_FUNCTION(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC)                \
    OPENGL_FUNCTION(glUniform1i, PFNGLUNIFORM1IPROC)                                \
    OPENGL_FUNCTION(glUniform1f, PFNGLUNIFORM1FPROC)                                \
    OPENGL_FUNCTION(glUniform3f, PFNGLUNIFORM3FPROC)                                \
    OPENGL_FUNCTION(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC)                  \
    OPENGL_FUNCTION(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC)              \
    OPENGL_FUNCTION(glGenBuffers, PFNGLGENBUFFERSPROC)                              \
    OPENGL_FUNCTION(glDeleteBuffers, PFNGLDELETEBUFFERSPROC)                        \
    OPENGL_FUNCTION(glBindBuffer, PFNGLBINDBUFFERPROC)                              \
    OPENGL_FUNCTION(glBufferData, PFNGLBUFFERDATAPROC)                              \
    OPENGL_FUNCTION(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC)                    \
    OPENGL_FUNCTION(glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC)              \
    OPENGL_FUNCTION(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC)                    \
    OPENGL_FUNCTION(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC)            \
    OPENGL_FUNCTION(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC)    \
    OPENGL_FUNCTION(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC)

// Declare an OpenGL function. Other translation units that require
// calling OpenGL functions will link against these declarations.
//
// OPENGL_DECLARE(glCreateShader, PFNGLCREATESHADERPROC)
//
//   becomes
//
// extern PFNGLCREATESHADERPROC glCreateShader;
#define OPENGL_DECLARE(func_name, func_type)  \
    extern func_type func_name;

// Set the OPENGL_FUNCTION macro to OPENGL_DECLARE and then splat
// all of the declarations out here. Unset OPENGL_FUNCTION back
// to nothing afterwards just to be safe.
#define OPENGL_FUNCTION OPENGL_DECLARE
OPENGL_FUNCTIONS
#undef OPENGL_FUNCTION

// Call this function after obtaining an OpenGL context
// to dynamically load the modern functions.
bool opengl_load_functions(void);


// Define an OpenGL function. Until dynamically loaded, it will
// be set to NULL and should NOT be called. Doing so will cause
// a segfault.
//
// OPENGL_DEFINE(glCreateShader, PFNGLCREATESHADERPROC)
//
//   becomes
//
// PFNGLCREATESHADERPROC glCreateShader = NULL;
#define OPENGL_DEFINE(func_name, func_type)  \
    func_type func_name = NULL;

// Define all of the initally-NULL OpenGL functions.
#define OPENGL_FUNCTION OPENGL_DEFINE
OPENGL_FUNCTIONS
#undef OPENGL_FUNCTION

// Define a union that bridges the gap between object pointers
// and function pointers. This is needed because the C standard
// forbids assignment between function pointers and object pointers
// (void*, in our case). They are NOT guaranteed to be the same size.
// By pulling the void* from SDL_GL_GetProcAddress though this union,
// we ensure that the potential difference in pointer sizes is mitigated.
union bridge {
    void* object_ptr;
    void (*function_ptr)(void);
};

// Load an OpenGL function by passing it through the union. Check
// for errors and return from the load if something goes wrong. This
// expansion uses a C99 designated initializer to cleanly instantiate
// the union bridge. The OpenGL function pointer is then pulled out
// and assigned to the the definition that was initially NULL.
//
// In short, the void* goes in and the OpenGL function comes out.
// Using the union bridge is necesssary to keep the compilers happy
// (-std=c99 -Wall -Wextra -Wpedantic).
//
// OPENGL_LOAD(glCreateShader, PFNGLCREATESHADERPROC)
//
//   becomes
//
// glCreateShader = (PFNGLCREATESHADERPROC)(union bridge){
//     .object_ptr = SDL_GL_GetProcAddress("glCreateShader")
// }.function_ptr;
#define OPENGL_LOAD(func_name, func_type)                \
    func_name = (func_type)(union bridge){               \
        .object_ptr = SDL_GL_GetProcAddress(#func_name)  \
    }.function_ptr;

// Extra safety step to ensure that all the OpenGL functions were successfully
// dynamically loaded. If a function failed to load, print and error and
// return false back to the caller.
//
// OPENGL_VALIDATE(glCreateShader, PFNGLCREATESHADERPROC)
//
//   becomes
//
// if (glCreateShader == NULL) {
//     fprintf(stderr, "failed to load func: %s\n", "glCreateShader);
//     return false;
// }
#define OPENGL_VALIDATE(func_name, func_type)                      \
    if (func_name == NULL) {                                       \
        fprintf(stderr, "failed to load func: %s\n", #func_name);  \
        return false;                                              \
    }

bool
opengl_load_functions(void)
{
    // use SDL2's platform-agnostic loader to pull the "real" addresses
    //  out by name and assign to the definitions above
    //
    // WARN: ISO C forbids conversion of object pointer to function pointer type
    //
    // the C standard defines func ptrs and object ptrs as different types
    //  that are potentially different sizes (though in practice they tend
    //  to be the same)
    // glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");

    #define OPENGL_FUNCTION OPENGL_LOAD
    OPENGL_FUNCTIONS
    #undef OPENGL_FUNCTION

    #define OPENGL_FUNCTION OPENGL_VALIDATE
    OPENGL_FUNCTIONS
    #undef OPENGL_FUNCTION

    return true;
}

static bool
opengl_shader_compile_source(GLuint shader, const GLchar* source)
{
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        GLint info_log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);

        GLchar* info_log = malloc(info_log_length);
        glGetShaderInfoLog(shader, info_log_length, NULL, info_log);

        fprintf(stderr, "failed to compile shader:\n%s\n", info_log);
        free(info_log);

        return false;
    }

    return true;
}

static bool
opengl_shader_link_program(GLuint program, GLuint vertex_shader, GLuint fragment_shader)
{
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        GLint info_log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

        GLchar* info_log = malloc(info_log_length);
        glGetProgramInfoLog(program, info_log_length, NULL, info_log);

        fprintf(stderr, "failed to link program:\n%s\n", info_log);
        free(info_log);

        glDetachShader(program, vertex_shader);
        glDetachShader(program, fragment_shader);

        return false;
    }

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    return false;
}

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

const float CUBE_VERTICES[] = {
    // Front face
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    // Back face
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f
};

const unsigned int CUBE_INDICES[] = {
    // Front face
    0, 1, 2, 2, 3, 0,
    // Right face
    1, 5, 6, 6, 2, 1,
    // Back face
    5, 4, 7, 7, 6, 5,
    // Left face
    4, 0, 3, 3, 7, 4,
    // Top face
    3, 2, 6, 6, 7, 3,
    // Bottom face
    4, 5, 1, 1, 0, 4
};

// Matrix utility functions
void matrix_identity(float* matrix) {
    for (int i = 0; i < 16; i++) {
        matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

void matrix_multiply(float* result, const float* a, const float* b) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result[row * 4 + col] =
                a[row * 4 + 0] * b[0 * 4 + col] +
                a[row * 4 + 1] * b[1 * 4 + col] +
                a[row * 4 + 2] * b[2 * 4 + col] +
                a[row * 4 + 3] * b[3 * 4 + col];
        }
    }
}
void matrix_perspective(float* matrix, float fov, float aspect, float near_val, float far_val) {
    float f = 1.0f / tanf(fov / 2.0f);
    float depth_diff = far_val - near_val;

    matrix[0] = f / aspect;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -(far_val + near_val) / depth_diff;
    matrix[11] = -1.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = -(2.0f * far_val * near_val) / depth_diff;
    matrix[15] = 0.0f;
}

void matrix_rotate_x(float* matrix, float angle) {
    matrix_identity(matrix);
    matrix[5] = cosf(angle);
    matrix[6] = -sinf(angle);
    matrix[9] = sinf(angle);
    matrix[10] = cosf(angle);
}

void matrix_rotate_y(float* matrix, float angle) {
    matrix_identity(matrix);
    matrix[0] = cosf(angle);
    matrix[2] = sinf(angle);
    matrix[8] = -sinf(angle);
    matrix[10] = cosf(angle);
}

static void print_usage(const char* arg0) {
    printf("usage: %s [options]\n", arg0);
    printf("\n");
    printf("Options:\n");
    printf("  -h --help        print this help\n");
    printf("  -f --fullscreen  fullscreen window\n");
    printf("  -v --vsync       enable vsync\n");
}

int main(int argc, char* argv[]) {
    bool fullscreen = false;
    bool vsync = false;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fullscreen") == 0) {
            fullscreen = true;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--vsync") == 0) {
            vsync = true;
        }
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "failed to init SDL2: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // Create window
    unsigned long flags = SDL_WINDOW_OPENGL;
    if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    SDL_Window* window = SDL_CreateWindow(
        "Improved SDL2 OpenGL Pride Gradient Cube Demo",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        800, 600,
        flags);

    if (window == NULL) {
        fprintf(stderr, "failed to create SDL2 window: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Create OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        fprintf(stderr, "failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Set VSync
    SDL_GL_SetSwapInterval(vsync ? 1 : 0);

    // Load OpenGL functions (assuming this is defined elsewhere)
    opengl_load_functions();

    // Vertex Shader
    const GLchar *vs_source =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "out vec3 fragPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "    fragPos = aPos;\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}\n";

    // Fragment Shader
    const GLchar *fs_source =
        "#version 330 core\n"
        "in vec3 fragPos;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "    vec3 absPos = abs(fragPos);\n"
        "    float edgeFactor = 1.0 - max(max(absPos.x, absPos.y), absPos.z) * 2.0;\n"
        "    edgeFactor = clamp(edgeFactor, 0.0, 1.0);\n"
        "\n"
        "    vec3 baseColor;\n"
        "    if (fragPos.y > 0.4) {\n"
        "        baseColor = vec3(1.0, 0.0, 0.0);\n"  // Red (top red)\n"
        "    } else if (fragPos.y < -0.4) {\n"
        "        baseColor = vec3(1.0, 0.5, 0.0);\n"  // Orange (bottom orange)\n"
        "    } else if (fragPos.x > 0.4) {\n"
        "        baseColor = vec3(1.0, 1.0, 0.0);\n"  // Yellow (right yellow)\n"
        "    } else if (fragPos.x < -0.4) {\n"
        "        baseColor = vec3(0.0, 0.8, 0.0);\n"  // Green (left green)\n"
        "    } else if (fragPos.z > 0.4) {\n"
        "        baseColor = vec3(0.0, 0.0, 1.0);\n"  // Blue (front blue)\n"
        "    } else {\n"
        "        baseColor = vec3(0.5, 0.0, 0.5);\n"  // Purple (back purple)\n"
        "    }\n"
        "\n"
        "    color = vec4(mix(baseColor, vec3(1.0), edgeFactor), 1.0);\n"
        "}\n";

    // Compile shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    opengl_shader_compile_source(vs, vs_source);
    opengl_shader_compile_source(fs, fs_source);

    // Create and link shader program
    GLuint prog = glCreateProgram();
    opengl_shader_link_program(prog, vs, fs);

    // Get uniform locations
    GLint modelLoc = glGetUniformLocation(prog, "model");
    GLint viewLoc = glGetUniformLocation(prog, "view");
    GLint projectionLoc = glGetUniformLocation(prog, "projection");

    glDeleteShader(vs);
    glDeleteShader(fs);

    // Create and bind buffers
    GLuint vbo, ebo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CUBE_INDICES), CUBE_INDICES, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Enable depth testing and multisampling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glDepthFunc(GL_LESS);

    // Frame timing variables
    const int TARGET_FPS = 60;
    const int FRAME_DELAY = 1000 / TARGET_FPS;
    unsigned long last_frame = 0;
    long frame_count = 0;
    unsigned long last_second = 0;

    // Main rendering loop
    bool running = true;
    while (running) {
        unsigned long frameStart = SDL_GetTicks();

        // Event handling
        SDL_Event event = { 0 };
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYUP) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_q || key == SDLK_ESCAPE) running = false;
            }
        }

        // Calculate time
        unsigned long now = SDL_GetTicks();
        float time = now / 1000.0f;

        // Clear buffers
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        glUseProgram(prog);

        // Create matrices
        float projection[16], view[16], model[16], rotX[16], rotY[16], temp[16];

        // Projection matrix
        matrix_perspective(projection, M_PI/4, 800.0f/600.0f, 0.1f, 100.0f);

        // View matrix (move back a bit)
        matrix_identity(view);
        view[14] = -3.0f;

        // Model matrix (rotation)
        matrix_rotate_x(rotX, sinf(time) * M_PI);
        matrix_rotate_y(rotY, cosf(time) * M_PI);
        matrix_multiply(model, rotY, rotX);

        // Upload matrices to shader
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);

        // Draw cube
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, sizeof(CUBE_INDICES) / sizeof(*CUBE_INDICES), GL_UNSIGNED_INT, 0);

        // FPS tracking
        frame_count++;
        if (now - last_second >= 1000) {
            printf("FPS: %ld\n", frame_count);
            frame_count = 0;
            last_second = now;
        }

        // Swap buffers
        SDL_GL_SwapWindow(window);

        // Frame timing control
        unsigned long frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY > frameTime) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    // Cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(prog);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
