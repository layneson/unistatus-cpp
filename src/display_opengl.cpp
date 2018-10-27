#include "display.hpp"

#include <SDL.h>
#include <GL/gl.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

namespace display {

const int SIZE = 500;

SDL_Window* window;
SDL_GLContext gl_context;

unsigned char pixels[8 * 8][3];

void init() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    window = SDL_CreateWindow("Unistatus", 0, 0, SIZE, SIZE, SDL_WINDOW_OPENGL);

    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);

    glViewport(0, 0, SIZE, SIZE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SIZE, SIZE, 0, 0, 0.5);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_MULTISAMPLE);

    memset(pixels, 0, sizeof(pixels));
}

void quit() {
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void update() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            exit(0);
        }
    }

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    const float PADDING = 15.0f;
    const int ANG_RES = 50;

    float draw_space = SIZE - PADDING*2.0f - PADDING*7.0f;

    float radius = draw_space / 16.0f;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
             glBegin(GL_TRIANGLE_FAN);
            {
                unsigned char* color = pixels[y * 8 + x];

                glColor4f((float)color[0]/255.0f, (float)color[1]/255.0f, (float)color[2]/255.0f, 1.0f);

                float cx = PADDING + (radius*2.0f + PADDING)*x + radius;
                float cy = PADDING + (radius*2.0f + PADDING)*y + radius;

                glVertex2f(cx, cy);

                for (int theta = 0; theta < ANG_RES; theta++) {
                    float angle_0 = (float)theta * 2.0f * M_PI / (float)ANG_RES;
                    float angle_1 = (float)(theta + 1) * 2.0f * M_PI / (float)ANG_RES;

                    glVertex2f(cx + radius*cosf(angle_0), cy + radius*sinf(angle_0));
                    glVertex2f(cx + radius*cosf(angle_1), cy + radius*sinf(angle_1));
                }
            }
            glEnd();
        }
    }

   

    SDL_GL_SwapWindow(window);
}

void set_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    assert(x >= 0 && x < 8 && y >= 0 && y < 8);

    pixels[y * 8 + x][0] = r;
    pixels[y * 8 + x][1] = g;
    pixels[y * 8 + x][2] = b;
}

void set_brightness(unsigned char v) {
    // Do nothing for now.
}

}