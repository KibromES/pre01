/**
 * Benchmark for the hash table map implementation.
 *
 * Measures average time per operation (insert, get, remove) for
 * increasing values of N, and prints results as CSV for plotting.
 *
 * Build with:  make (included via Makefile target 'bench')
 * Run with:    ./bin/debug/benchmark
 */

#include "map.h"
#include "common.h"
#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

/* Returns wall-clock time in seconds */
static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* Generate a key string from index */
static void make_key(char *buf, size_t buf_len, size_t i) {
    snprintf(buf, buf_len, "key_%zu", i);
}

typedef struct bench_result {
    size_t n;
    double ns_insert;
    double ns_get;
    double ns_remove;
} bench_result_t;

static bench_result_t run_benchmark(size_t n) {
    bench_result_t r = { .n = n };
    char keybuf[64];

    /* --- INSERT --- */
    map_t *map = map_create((cmp_fn)strcmp, hash_string_fnv1a64);

    double t0 = now_sec();
    for (size_t i = 0; i < n; i++) {
        make_key(keybuf, sizeof(keybuf), i);
        int *val = malloc(sizeof(int));
        *val = (int)i;
        map_insert(map, keybuf, strlen(keybuf) + 1, val);
    }
    double t1 = now_sec();
    r.ns_insert = (t1 - t0) / (double)n * 1e9;

    /* --- GET (hit) --- */
    t0 = now_sec();
    for (size_t i = 0; i < n; i++) {
        make_key(keybuf, sizeof(keybuf), i);
        volatile void *v = map_get(map, keybuf);
        (void)v;
    }
    t1 = now_sec();
    r.ns_get = (t1 - t0) / (double)n * 1e9;

    /* --- REMOVE --- */
    t0 = now_sec();
    for (size_t i = 0; i < n; i++) {
        make_key(keybuf, sizeof(keybuf), i);
        void *val = map_remove(map, keybuf);
        free(val);
    }
    t1 = now_sec();
    r.ns_remove = (t1 - t0) / (double)n * 1e9;

    map_destroy(map, NULL);
    return r;
}

int main(void) {
    /* Sizes to benchmark */
    size_t sizes[] = {
        100, 500, 1000, 5000, 10000, 50000,
        100000, 250000, 500000, 1000000
    };
    size_t n_sizes = sizeof(sizes) / sizeof(sizes[0]);

    printf("n,ns_per_insert,ns_per_get,ns_per_remove\n");

    for (size_t i = 0; i < n_sizes; i++) {
        bench_result_t r = run_benchmark(sizes[i]);
        printf("%zu,%.2f,%.2f,%.2f\n",
               r.n, r.ns_insert, r.ns_get, r.ns_remove);
        fflush(stdout);
    }

    return 0;
}
