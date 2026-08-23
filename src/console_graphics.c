#include "console_graphics.h"
#include "array.h"
#include "basic.h"
#include "mac_semaphore.h"
#include <locale.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

DECLARE_ARRAY(wchar_t, WCharArray);

// The implementation consists of two child threads:
// 1. A render thread that writes to a buffer
// 2. A screen thread draws that buffer to the screen.

/// This is data used to pass messages and buffers between the two threads.
typedef struct MessageBox {
    /// Signal that render input is ready to be read.
    sem_t unread_render_input;
    /// Signal that render output is ready to be written to the screen.
    sem_t unread_render_output;
    /// The width/height of the screen for the next render.
    uint16_t width, height;
    /// The buffer to write the next render to.
    wchar_t* buf;
} MessageBox;

/// Arguments sent to the render thread when started.
typedef struct RenderThreadArgs {
    /// This is a pointer to where the screen thread writes.
    MessageBox* box;
    ConsoleGraphicsDrawFunc* draw;
    void* draw_arg;
    /// Are we done with reading these arguments?
    sem_t initialized;
} RenderThreadArgs;

/// Arguments sent to the screen thread when started.
typedef struct ScreenThreadArgs {
    /// This is a pointer to where to write the inputs to the render thread to.
    MessageBox* box;
    /// The stream to output to.
    FILE* f;
    /// Are we done with reading these arguments?
    sem_t initialized;
} ScreenThreadArgs;

static void* render_thread_main(RenderThreadArgs* args) {
    MessageBox* const box = args->box;
    ConsoleGraphicsDrawFunc* const draw = args->draw;
    void* const draw_arg = args->draw_arg;
    sem_post(&args->initialized);
    while (true) {
        // Start by reading the input cell. Wait for the screen thread to give
        // us the input.
        sem_wait(&box->unread_render_input);
        const uint16_t w = box->width, h = box->height;
        wchar_t* const buf = box->buf;
        // Now draw!
        draw(buf, w, h, draw_arg);
        // Tell bro we're done.
        sem_post(&box->unread_render_output);
    }
    return NULL;
}

void get_terminal_size(FILE* f, uint16_t* w, uint16_t* h) {
    int fd = fileno(f);
    struct winsize ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != 0) {
        fprintf(stderr, "ioctl() failed (%d): %s\n", errno, strerror(errno));
        return;
    }
    *w = ws.ws_col;
    *h = ws.ws_row;
}

static void draw_screen(FILE* f, const wchar_t* buf, uint16_t w, uint16_t h) {
    system("clear");
    for (size_t i = 0; i < h; i++) fwprintf(f, L"%.*ls\n", (int)w, &buf[i * w]);
    fflush(f);
    printf("%d %d\n", w, h);
    // sleep(1);
}

static void send_new_render_input(
    MessageBox* box,
    FILE* f,
    uint16_t* w,
    uint16_t* h,
    wchar_t* buf,
    wchar_t** buf_allocation
) {
    uint16_t prev_w = *w, prev_h = *h;
    get_terminal_size(f, w, h);
    if (*w == prev_w && *h == prev_h) {
        box->buf = buf;
    } else {
        // Need to allocate a new double buffer, and replace the old one.
        // We are allocating two times the space because we are going to do
        // something called buffer switching!
        free(*buf_allocation);
        wchar_t* new_buf = malloc((size_t)*w * *h * sizeof(wchar_t) * 2);
        wmemset(new_buf, L' ', *w * *h * 2);
        box->width = *w;
        box->height = *h;
        box->buf = new_buf;
        *buf_allocation = new_buf;
    }
    sem_post(&box->unread_render_input);
}

static void* screen_thread_main(ScreenThreadArgs* args) {
    MessageBox* const box = args->box;
    FILE* const f = args->f;
    sem_post(&args->initialized);
    bool render_first_buf = true;
    uint16_t w = 0, h = (uint16_t)~0;
    wchar_t* buf_allocation = NULL;
#define buf1 buf_allocation
#define buf2 (buf_allocation + (size_t)w * h)
    // Before entering the loop, ready the input for the first time.
    // The semaphore fields have been initialized already by the main thread.
    send_new_render_input(box, f, &w, &h, NULL, &buf_allocation);
    while (true) {
        render_first_buf = !render_first_buf;
        // Now wait for an output to write to the screen!
        sem_wait(&box->unread_render_output);
        // Before writing it, immediately start rendering the next one.
        wchar_t* buf_to_render_to = render_first_buf ? buf1 : buf2;
        wchar_t* buf_to_write = render_first_buf ? buf2 : buf1;

        uint16_t prev_w = w, prev_h = h;
        send_new_render_input(
            box, f, &w, &h, buf_to_render_to, &buf_allocation
        );
        // Now draw while the next frame is rendering!
        draw_screen(f, buf_to_write, prev_w, prev_h);
    }
    return NULL;
}

typedef void* ThreadMain(void*);

ConsoleGraphics console_graphics_init(
    FILE* f, ConsoleGraphicsDrawFunc draw, void* draw_arg
) {
    setlocale(LC_ALL, "");
    ConsoleGraphics ret;
    MessageBox* box = malloc(sizeof(MessageBox));
    sem_init(&box->unread_render_input, 0, 0);
    sem_init(&box->unread_render_output, 0, 0);
    RenderThreadArgs a1 = { box, draw, draw_arg, {} };
    sem_init(&a1.initialized, 0, 0);
    ScreenThreadArgs a2 = { box, f, {} };
    sem_init(&a2.initialized, 0, 0);
    pthread_create(
        &ret.render_thread, NULL, (ThreadMain*)render_thread_main, &a1
    );
    pthread_create(
        &ret.screen_thread, NULL, (ThreadMain*)screen_thread_main, &a2
    );
    sem_wait(&a1.initialized);
    sem_wait(&a2.initialized);
    return ret;
}
