#pragma once

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

/// This is an object that manages drawing to a file stream (example - stdout).
///
/// This opens up two child threads: One thread is responsible for rendering a
/// state onto a render buffer. While this happens, the other thread reads the
/// previous render buffer, and writes it to the screen. When they are both
/// done, they switch buffers.
typedef struct ConsoleGraphics {
    pthread_t render_thread;
    pthread_t screen_thread;
    void* allocation;
} ConsoleGraphics;

typedef void ConsoleGraphicsDrawFunc(
    wchar_t* buf, uint16_t w, uint16_t h, void* arg
);

ConsoleGraphics console_graphics_init(
    FILE* f, ConsoleGraphicsDrawFunc draw, void* draw_arg, float targe_fps
);
