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

#ifdef WINDOWS
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

#define SDL_DISABLE_IMMINTRIN_H 1
#define SDL_MAIN_HANDLED 1
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

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

const float SQUARE[] = {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f,  1.0f,
     1.0f, -1.0f
};

static void
print_usage(const char* arg0)
{
    printf("usage: %s [options]\n", arg0);
    printf("\n");
    printf("Options:\n");
    printf("  -h --help        print this help\n");
    printf("  -f --fullscreen  fullscreen window\n");
    printf("  -v --vsync       enable vsync\n");
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

int
main(int argc, char* argv[])
{
    bool fullscreen = false;
    bool vsync = false;

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

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "failed to init SDL2: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    printf("Platform:        %s\n", SDL_GetPlatform());
    printf("CPU Count:       %d\n", SDL_GetCPUCount());
    printf("System RAM:      %d MB\n", SDL_GetSystemRAM());
    printf("Supports SSE:    %s\n", SDL_HasSSE() ? "true" : "false");
    printf("Supports SSE2:   %s\n", SDL_HasSSE2() ? "true" : "false");
    printf("Supports SSE3:   %s\n", SDL_HasSSE3() ? "true" : "false");
    printf("Supports SSE4.1: %s\n", SDL_HasSSE41() ? "true" : "false");
    printf("Supports SSE4.2: %s\n", SDL_HasSSE42() ? "true" : "false");

    // Request at least 32-bit color
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    // Request a double-buffered, OpenGL 3.3 (or higher) core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    unsigned long flags = SDL_WINDOW_OPENGL;
    if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    SDL_Window* window = SDL_CreateWindow(
        "SDL2 OpenGL Demo",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        640,
        flags);

    if (window == NULL) {
        fprintf(stderr, "failed to create SDL2 window: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // SDL_GLContext is an alias for "void*"
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        fprintf(stderr, "failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    printf("OpenGL Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL Version:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Enable v-sync (set 1 to enable, 0 to disable)
    SDL_GL_SetSwapInterval(vsync ? 1 : 0);

    // Load the modern OpenGL funcs
    opengl_load_functions();


    // Do modern OpenGL stuff
    const GLchar *vs_source =
        "#version 330\n"
        "layout(location = 0) in vec2 point;\n"
        "uniform float angle;\n"
        "void main() {\n"
        "    mat2 rotate = mat2(cos(angle), -sin(angle),\n"
        "                       sin(angle), cos(angle));\n"
        "    gl_Position = vec4(0.75 * rotate * point, 0.0, 1.0);\n"
        "}\n";
    const GLchar *fs_source =
        "#version 330\n"
        "out vec4 color;\n"
        "void main() {\n"
        "    color = vec4(1, 0.15, 0.15, 0);\n"
        "}\n";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    opengl_shader_compile_source(vs, vs_source);
    opengl_shader_compile_source(fs, fs_source);

    GLuint prog = glCreateProgram();
    opengl_shader_link_program(prog, vs, fs);

    GLint uniform_angle = glGetUniformLocation(prog, "angle");

    glDeleteShader(fs);
    glDeleteShader(vs);


    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SQUARE), SQUARE, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    long frame_count = 0;
    unsigned long last_frame = 0;
    unsigned long last_second = 0;
    double angle = 0.0;

    bool running = true;
    while (running) {
        SDL_Event event = { 0 };
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYUP) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_q || key == SDLK_ESCAPE) running = false;
            }
        }

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glUniform1f(uniform_angle, angle);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(SQUARE) / sizeof(*SQUARE) / 2);
        glBindVertexArray(0);
        glUseProgram(0);

        unsigned long now = SDL_GetTicks();
        unsigned long diff = now - last_frame;
        last_frame = now;

        angle += diff / 1000.0;
        if (angle > 2 * M_PI) angle -= 2 * M_PI;

        if (now - last_second >= 1000) {
            printf("FPS: %ld\n", frame_count);
            frame_count = 0;
            last_second = now;
        }

        frame_count++;
        SDL_GL_SwapWindow(window);
    }

    // Cleanup OpenGL resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);

    // Cleanup SDL2 resources
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
