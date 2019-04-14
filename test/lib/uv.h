/* Helpers around the libuv-based implementation of the raft_io interface. */

#ifndef TEST_UV_H
#define TEST_UV_H

#include "../../include/raft.h"
#include "../../include/raft/uv.h"

#include "../../src/uv.h"

#include "fs.h"
#include "heap.h"
#include "loop.h"
#include "munit.h"
#include "tcp.h"

#define FIXTURE_UV                      \
    struct raft_heap heap;              \
    struct test_tcp tcp;                \
    struct uv_loop_s loop;              \
    char *dir;                          \
    struct raft_uv_transport transport; \
    struct raft_io io;                  \
    struct uv *uv;                      \
    bool closed;

#define SETUP_UV                                                      \
    (void)user_data;                                                  \
    {                                                                 \
        int rv__;                                                     \
        test_heap_setup(params, &f->heap);                            \
        test_tcp_setup(params, &f->tcp);                              \
        SETUP_LOOP;                                                   \
        f->dir = test_dir_setup(params);                              \
        rv__ = raft_uv_tcp_init(&f->transport, &f->loop);             \
        munit_assert_int(rv__, ==, 0);                                \
        rv__ = raft_uv_init(&f->io, &f->loop, f->dir, &f->transport); \
        munit_assert_int(rv__, ==, 0);                                \
        f->io.data = f;                                               \
        rv__ = f->io.init(&f->io, 1, "127.0.0.1:9000");               \
        munit_assert_int(rv__, ==, 0);                                \
        f->uv = f->io.impl;                                           \
        f->closed = false;                                            \
    }

#define TEAR_DOWN_UV                  \
    if (!f->closed) {                 \
        UV_CLOSE;                     \
    }                                 \
    LOOP_STOP;                        \
    raft_uv_close(&f->io);            \
    raft_uv_tcp_close(&f->transport); \
    test_dir_tear_down(f->dir);       \
    TEAR_DOWN_LOOP;                   \
    test_tcp_tear_down(&f->tcp);      \
    test_heap_tear_down(&f->heap);

#define UV_CLOSE                         \
    {                                    \
        int rv_;                         \
        rv_ = f->io.close(&f->io, NULL); \
        munit_assert_int(rv_, ==, 0);    \
        f->closed = true;                \
    }

/**
 * Create a valid snapshot metadata file with the given @term, @index and
 * @timestamp. The snapshot configuration will contain @nconfiguration_n
 * servers. Return the size of the created file.
 */
size_t test_io_uv_write_snapshot_meta_file(const char *dir,
                                           raft_term term,
                                           raft_index index,
                                           unsigned long long timestamp,
                                           unsigned configuration_n,
                                           raft_index configuration_index);

/**
 * Create a valid snapshot file with the given @term, @index and @timestamp. The
 * content of the snapshot file will be @buf.
 */
size_t test_io_uv_write_snapshot_data_file(const char *dir,
                                           raft_term term,
                                           raft_index index,
                                           unsigned long long timestamp,
                                           void *buf,
                                           size_t size);

/**
 * Create a valid open segment file with counter @counter with @n batches each
 * containing one entry. @data should be an integer that will be used as base
 * value for the data of the firt entry, and will be incremented. Return the
 * size of the created file.
 */
size_t test_io_uv_write_open_segment_file(const char *dir,
                                          unsigned long long counter,
                                          int n,
                                          int data);

/**
 * Create a valid closed segment file with first index @first_index and @n
 * batches each containing one entry. @data should be an integer that will be
 * used as base value for the data of the firt entry, and will be incremented.
 * Return the size of the created file.
 */
size_t test_io_uv_write_closed_segment_file(const char *dir,
                                            raft_index first_index,
                                            int n,
                                            int data);

#endif /* TEST_IO_UV_H */
