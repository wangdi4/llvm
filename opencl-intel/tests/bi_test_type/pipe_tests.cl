#include "pipes.h"

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define ASSERT(cond) { if (!(cond)) {printf(">> ASSERT at "             \
                                            STRINGIFY(__FILE__) ":"     \
                                            STRINGIFY(__LINE__) ": " #cond \
                                            "\n");                      \
                                     *((volatile int*)NULL) = 0xdead;}}

__global struct __pipe_t* buf2pipe(__global char* buf) {
  return (__global struct __pipe_t*)buf;
}

// --- SinglePacket kernels ---
char single_packet_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void single_packet_init() {
  __pipe_init_intel(buf2pipe(single_packet_pipe), 4, 128);
}

__kernel void single_packet_reader() {
  __global struct __pipe_t* p = buf2pipe(single_packet_pipe);
  int dst = 0;
  __read_pipe_2_bl_intel(p, &dst);
  ASSERT(dst == 42);
  __flush_read_pipe(p);
}

__kernel void single_packet_writer() {
  __global struct __pipe_t* p = buf2pipe(single_packet_pipe);
  int src = 42;
  int result = __write_pipe_2_bl_intel(p, &src);
  ASSERT(result == 0);
  __flush_write_pipe(p);
}

// --- MultiplePackets kernels ---
char multiple_packets_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void multiple_packets_init() {
  __pipe_init_intel(buf2pipe(multiple_packets_pipe), 4, 128);
}

__kernel void multiple_packets_reader() {
  __global struct __pipe_t* p = buf2pipe(multiple_packets_pipe);
  for (int i = 0; i < 1024; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void multiple_packets_writer() {
  __global struct __pipe_t* p = buf2pipe(multiple_packets_pipe);
  for (int i = 0; i < 1024; ++i) {
    int src = i;
    int result = __write_pipe_2_bl_intel(p, &src);
  }
  __flush_write_pipe(p);
}

// --- NonUniform kernels ---

char non_uniform_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void non_uniform_init() {
  __pipe_init_intel(buf2pipe(non_uniform_pipe), 4, 128);
}

__kernel void non_uniform_reader1() {
  __global struct __pipe_t* p = buf2pipe(non_uniform_pipe);
  int counter = 0;
  for (int i = 1; i < 129; ++i) {
    for (int j = 0; j < i; ++j) {
      int dst = 0;
      __read_pipe_2_bl_intel(p, &dst);
      ASSERT(dst == counter++);
    }
    __flush_read_pipe(p);
  }
}

__kernel void non_uniform_writer1() {
  __global struct __pipe_t* p = buf2pipe(non_uniform_pipe);
  for (int i = 0; i < 8256; ++i) {
    int src = i;
    int result = __write_pipe_2_bl_intel(p, &src);
  }
  __flush_write_pipe(p);
}

__kernel void non_uniform_reader2() {
  __global struct __pipe_t* p = buf2pipe(non_uniform_pipe);
  for (int i = 0; i < 8256; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void non_uniform_writer2() {
  __global struct __pipe_t* p = buf2pipe(non_uniform_pipe);
  int counter = 0;
  for (int i = 1; i < 129; ++i) {
    for (int j = 0; j < i; ++j) {
      int src = counter++;
      int result = __write_pipe_2_bl_intel(p, &src);
    }
    __flush_write_pipe(p);
  }
}

// --- OneSlow kernels ---

char one_slow_pipe[sizeof(struct __pipe_t) + 4 * 128];

static void wait(int amount) {
  for (volatile int i = 0; i < amount; ++i) {
    volatile float f = (float)i;
    volatile float s = sin(f);
  }
}

__kernel void one_slow_init() {
  __pipe_init_intel(buf2pipe(one_slow_pipe), 4, 128);
}

__kernel void one_slow_reader1() {
  __global struct __pipe_t* p = buf2pipe(one_slow_pipe);
  for (int i = 0; i < 1024 * 1024; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
    wait(1000);
  }
  __flush_read_pipe(p);
}

__kernel void one_slow_writer1() {
  __global struct __pipe_t* p = buf2pipe(one_slow_pipe);
  for (int i = 0; i < 1024 * 1024; ++i) {
    int src = i;
    int result = __write_pipe_2_bl_intel(p, &src);
  }
  __flush_write_pipe(p);
}

__kernel void one_slow_reader2() {
  __global struct __pipe_t* p = buf2pipe(one_slow_pipe);
  for (int i = 0; i < 1024 * 1024; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void one_slow_writer2() {
  __global struct __pipe_t* p = buf2pipe(one_slow_pipe);
  for (int i = 0; i < 1024 * 1024; ++i) {
    int src = i;
    int result = __write_pipe_2_bl_intel(p, &src);
    wait(1000);
  }
  __flush_write_pipe(p);
}

// --- BillionPackets kernels ---

char billion_packets_pipe[sizeof(struct __pipe_t) + 8 * 128];

__kernel void billion_packets_init() {
  __pipe_init_intel(buf2pipe(billion_packets_pipe), 8, 128);
}

__kernel void billion_packets_reader() {
  __global struct __pipe_t* p = buf2pipe(billion_packets_pipe);
  for (long i = 0; i < 1000 * 1000 * 1000; ++i) {
    long dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void billion_packets_writer() {
  __global struct __pipe_t* p = buf2pipe(billion_packets_pipe);
  for (long i = 0; i < 1000 * 1000 * 1000; ++i) {
    long src = i;
    __write_pipe_2_bl_intel(p, &src);
  }
  __flush_write_pipe(p);
}

// --- BigMaxPackets/SmallMaxPackets kernels ---

char max_packets_pipe[sizeof(struct __pipe_t) + 4 * 1024 * 64];

__kernel void max_packets_init(int max_packets) {
  __pipe_init_intel(buf2pipe(max_packets_pipe), 4, max_packets);
}

__kernel void max_packets_reader() {
  __global struct __pipe_t* p = buf2pipe(max_packets_pipe);
  int num = 4 * max(1024, p->max_packets);
  for (int i = 0; i < num; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void max_packets_writer() {
  __global struct __pipe_t* p = buf2pipe(max_packets_pipe);
  int num = 4 * max(1024, p->max_packets);
  for (int i = 0; i < num; ++i) {
    int src = i;
    __write_pipe_2_bl_intel(p, &src);
  }
  __flush_write_pipe(p);
}
