#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pulse/simple.h>
#include <pulse/error.h>

static volatile int running = 1;

void handle_signal(int sig) {
    (void)sig;
    running = 0;
}

int main() {
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    pa_sample_spec ss = {
        .format   = PA_SAMPLE_S16LE,
        .rate     = 48000,
        .channels = 2
    };

    int pa_err;
    pa_simple *pa = pa_simple_new(
        NULL,               /* default server */
        "keep-audio-awake", /* app name */
        PA_STREAM_PLAYBACK,
        NULL,               /* default device — whatever system audio uses */
        "silence",          /* stream description */
        &ss,
        NULL,               /* default channel map */
        NULL,               /* default buffering */
        &pa_err
    );

    if (!pa) {
        fprintf(stderr, "Cannot connect to PulseAudio/PipeWire: %s\n", pa_strerror(pa_err));
        return 1;
    }

    printf("Keeping audio awake via PipeWire/PulseAudio (silent stream).\n");
    printf("Other apps can play audio normally. Press Ctrl+C to stop.\n");

    /* 100ms of silence per write at 48000Hz stereo S16LE */
    const size_t frame_bytes = 2 * 2; /* channels * bytes per sample */
    const size_t frames      = 4800;  /* 100ms */
    const size_t buf_bytes   = frames * frame_bytes;
    void *buf = calloc(buf_bytes, 1); /* zeroed = silence */
    if (!buf) {
        fprintf(stderr, "Out of memory\n");
        pa_simple_free(pa);
        return 1;
    }

    while (running) {
        if (pa_simple_write(pa, buf, buf_bytes, &pa_err) < 0) {
            fprintf(stderr, "Write error: %s\n", pa_strerror(pa_err));
            break;
        }
    }

    free(buf);
    pa_simple_drain(pa, NULL);
    pa_simple_free(pa);
    printf("\nDone.\n");
    return 0;
}
