#include "pipes-internal.h"
#include "pipes.h"

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define ASSERT(cond)                                                           \
  {                                                                            \
    if (!(cond)) {                                                             \
      printf(">> ASSERT at " STRINGIFY(__FILE__) ":" STRINGIFY(                \
          __LINE__) ": " #cond "\n");                                          \
      *((volatile int *)NULL) = 0xdead;                                        \
    }                                                                          \
  }

__global struct __pipe_t *buf2pipe(__global char *buf) {
  return (__global struct __pipe_t *)buf;
}

// --- SinglePacket kernels ---
char single_packet_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void single_packet_init() {
  __pipe_init_intel(buf2pipe(single_packet_pipe), 4, 128);
}

__kernel void single_packet_reader() {
  __global struct __pipe_t *p = buf2pipe(single_packet_pipe);
  int dst = 0;
  __read_pipe_2_bl_intel(p, &dst);
  ASSERT(dst == 42);
  __flush_read_pipe(p);
}

__kernel void single_packet_writer() {
  __global struct __pipe_t *p = buf2pipe(single_packet_pipe);
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
  __global struct __pipe_t *p = buf2pipe(multiple_packets_pipe);
  for (int i = 0; i < 1024; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void multiple_packets_writer() {
  __global struct __pipe_t *p = buf2pipe(multiple_packets_pipe);
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
  __global struct __pipe_t *p = buf2pipe(non_uniform_pipe);
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
  __global struct __pipe_t *p = buf2pipe(non_uniform_pipe);
  for (int i = 0; i < 8256; ++i) {
    int src = i;
    int result = __write_pipe_2_bl_intel(p, &src);
  }
  __flush_write_pipe(p);
}

__kernel void non_uniform_reader2() {
  __global struct __pipe_t *p = buf2pipe(non_uniform_pipe);
  for (int i = 0; i < 8256; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void non_uniform_writer2() {
  __global struct __pipe_t *p = buf2pipe(non_uniform_pipe);
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
  __global struct __pipe_t *p = buf2pipe(one_slow_pipe);
  for (int i = 0; i < 1024 * 1024; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
    wait(1000);
  }
  __flush_read_pipe(p);
}

__kernel void one_slow_writer1() {
  __global struct __pipe_t *p = buf2pipe(one_slow_pipe);
  for (int i = 0; i < 1024 * 1024; ++i) {
    int src = i;
    int result = __write_pipe_2_bl_intel(p, &src);
  }
  __flush_write_pipe(p);
}

__kernel void one_slow_reader2() {
  __global struct __pipe_t *p = buf2pipe(one_slow_pipe);
  for (int i = 0; i < 1024 * 1024; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void one_slow_writer2() {
  __global struct __pipe_t *p = buf2pipe(one_slow_pipe);
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
  __global struct __pipe_t *p = buf2pipe(billion_packets_pipe);
  for (long i = 0; i < 1000 * 1000 * 1000; ++i) {
    long dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void billion_packets_writer() {
  __global struct __pipe_t *p = buf2pipe(billion_packets_pipe);
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
  __global struct __pipe_t *p = buf2pipe(max_packets_pipe);
  int num = 4 * max(1024, p->max_packets);
  for (int i = 0; i < num; ++i) {
    int dst = 0;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == i);
  }
  __flush_read_pipe(p);
}

__kernel void max_packets_writer() {
  __global struct __pipe_t *p = buf2pipe(max_packets_pipe);
  ASSERT(p->max_packets > p->write_buf.limit);
  int num = 4 * max(1024, p->max_packets);
  for (int i = 0; i < num; ++i) {
    int src = i;
    __write_pipe_2_bl_intel(p, &src);
  }
  __flush_write_pipe(p);
}

int __write_pipe_2_bl_intel_v2f32(__global struct __pipe_t *, float2);
float2 __read_pipe_2_bl_intel_v2f32(__global struct __pipe_t *);
int __write_pipe_2_bl_intel_v4f32(__global struct __pipe_t *, float4);
float4 __read_pipe_2_bl_intel_v4f32(__global struct __pipe_t *);
int __write_pipe_2_bl_intel_v8f32(__global struct __pipe_t *, float8);
float8 __read_pipe_2_bl_intel_v8f32(__global struct __pipe_t *);
int __write_pipe_2_bl_intel_v16f32(__global struct __pipe_t *, float16);
float16 __read_pipe_2_bl_intel_v16f32(__global struct __pipe_t *);

// --- VectorSinglePacket kernels ---
char vector_single_packet_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void vector_single_packet_init() {
  __pipe_init_intel(buf2pipe(vector_single_packet_pipe), 4, 128);
}

__kernel void vector_single_packet_reader() {
  __global struct __pipe_t *p = buf2pipe(vector_single_packet_pipe);
  float4 dst = __read_pipe_2_bl_intel_v4f32(p);
  ASSERT(all(isequal(dst, (float4)(42.0f))));
  __flush_read_pipe(p);
}

__kernel void vector_single_packet_writer() {
  __global struct __pipe_t *p = buf2pipe(vector_single_packet_pipe);
  float4 src = (float4)42.0f;
  int result = __write_pipe_2_bl_intel_v4f32(p, src);
  ASSERT(result == 0);
  __flush_write_pipe(p);
}
// --- VectorMultiplePackets kernels ---
char vector_mult_packets_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void vector_mult_packets_init() {
  __pipe_init_intel(buf2pipe(vector_mult_packets_pipe), 4, 128);
}

__kernel void vector_mult_packets_reader() {
  __global struct __pipe_t *p = buf2pipe(vector_mult_packets_pipe);
  for (int i = 0; i < 256; ++i) {
    float4 dst = __read_pipe_2_bl_intel_v4f32(p);
    ASSERT(all(isequal(dst, (float4)(0.1f * i))));
  }
  __flush_read_pipe(p);
}

__kernel void vector_mult_packets_writer() {
  __global struct __pipe_t *p = buf2pipe(vector_mult_packets_pipe);
  for (int i = 0; i < 256; ++i) {
    float4 src = (float4)(0.1f * i);
    __write_pipe_2_bl_intel_v4f32(p, src);
  }
  __flush_write_pipe(p);
}
// --- VectorReader/ScalarWriter kernels ---
char vector_vr_sw_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void vector_vr_sw_init() {
  __pipe_init_intel(buf2pipe(vector_vr_sw_pipe), 4, 128);
}

__kernel void vector_vr_sw_reader() {
  __global struct __pipe_t *p = buf2pipe(vector_vr_sw_pipe);
  for (int i = 0; i < 256; ++i) {
    float4 dst = __read_pipe_2_bl_intel_v4f32(p);
    ASSERT(all(isequal(dst, (float4)(0.1f * i))));
  }
  __flush_read_pipe(p);
}

__kernel void vector_vr_sw_writer() {
  __global struct __pipe_t *p = buf2pipe(vector_vr_sw_pipe);
  for (int i = 0; i < 1024; ++i) {
    // write 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.1f, 0.1f,...
    float src = (float)(0.1f * (i / 4));
    __write_pipe_2_bl_intel(p, &src);
  }
  __flush_write_pipe(p);
}
// --- ScalarReader/VectorWriter kernels ---
char vector_sr_vw_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void vector_sr_vw_init() {
  __pipe_init_intel(buf2pipe(vector_sr_vw_pipe), 4, 128);
}

__kernel void vector_sr_vw_reader() {
  __global struct __pipe_t *p = buf2pipe(vector_sr_vw_pipe);
  for (int i = 0; i < 1024; ++i) {
    float dst = 0.0f;
    __read_pipe_2_bl_intel(p, &dst);
    ASSERT(dst == (float)((i / 4) * 0.1f));
  }
  __flush_read_pipe(p);
}

__kernel void vector_sr_vw_writer() {
  __global struct __pipe_t *p = buf2pipe(vector_sr_vw_pipe);
  for (int i = 0; i < 256; ++i) {
    float4 src = (float4)(0.1f * i);
    __write_pipe_2_bl_intel_v4f32(p, src);
  }
  __flush_write_pipe(p);
}

// --- CheckWrapping kernels ---

char vector_wrapping_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void vector_wrapping_init() {
  __pipe_init_intel(buf2pipe(vector_wrapping_pipe), 4, 128);
}

__kernel void vector_wrapping_reader1() {
  __global struct __pipe_t *p = buf2pipe(vector_wrapping_pipe);

  float dst_scalar = 0.0f;
  __read_pipe_2_bl_intel(p, &dst_scalar);
  ASSERT(dst_scalar == (float)42.0f);

  for (int i = 0; i < 256; ++i) {
    float4 dst_vector = __read_pipe_2_bl_intel_v4f32(p);
    ASSERT(all(isequal(dst_vector, (float4)(0.1f * i))));
  }
  __flush_read_pipe(p);
}

__kernel void vector_wrapping_writer1() {
  __global struct __pipe_t *p = buf2pipe(vector_wrapping_pipe);

  float src_scalar = (float)42.0f;
  __write_pipe_2_bl_intel(p, &src_scalar);

  for (int i = 0; i < 256; ++i) {
    float4 src_vector = (float4)(0.1f * i);
    __write_pipe_2_bl_intel_v4f32(p, src_vector);
  }
  __flush_write_pipe(p);
}
__kernel void vector_wrapping_reader2() {
  __global struct __pipe_t *p = buf2pipe(vector_wrapping_pipe);

  for (int i = 0; i < 2; i++) {
    float dst_scalar = 0.0f;
    __read_pipe_2_bl_intel(p, &dst_scalar);
    ASSERT(dst_scalar == (float)42.0f);
  }
  for (int i = 0; i < 256; ++i) {
    float4 dst_vector = __read_pipe_2_bl_intel_v4f32(p);
    ASSERT(all(isequal(dst_vector, (float4)(0.1f * i))));
  }
  __flush_read_pipe(p);
}

__kernel void vector_wrapping_writer2() {
  __global struct __pipe_t *p = buf2pipe(vector_wrapping_pipe);

  for (int i = 0; i < 2; i++) {
    float src_scalar = (float)42.0f;
    __write_pipe_2_bl_intel(p, &src_scalar);
  }
  for (int i = 0; i < 256; ++i) {
    float4 src_vector = (float4)(0.1f * i);
    __write_pipe_2_bl_intel_v4f32(p, src_vector);
  }
  __flush_write_pipe(p);
}
__kernel void vector_wrapping_reader3() {
  __global struct __pipe_t *p = buf2pipe(vector_wrapping_pipe);

  for (int i = 0; i < 3; i++) {
    float dst_scalar = 0.0f;
    __read_pipe_2_bl_intel(p, &dst_scalar);
    ASSERT(dst_scalar == (float)42.0f);
  }
  for (int i = 0; i < 256; ++i) {
    float4 dst_vector = __read_pipe_2_bl_intel_v4f32(p);
    ASSERT(all(isequal(dst_vector, (float4)(0.1f * i))));
  }
  __flush_read_pipe(p);
}

__kernel void vector_wrapping_writer3() {
  __global struct __pipe_t *p = buf2pipe(vector_wrapping_pipe);

  for (int i = 0; i < 3; i++) {
    float src_scalar = (float)42.0f;
    __write_pipe_2_bl_intel(p, &src_scalar);
  }
  for (int i = 0; i < 256; ++i) {
    float4 src_vector = (float4)(0.1f * i);
    __write_pipe_2_bl_intel_v4f32(p, src_vector);
  }
  __flush_write_pipe(p);
}

// --- VectorBigMaxPackets/VectorSmallMaxPackets kernels ---

char vector_max_packets_pipe[sizeof(struct __pipe_t) + 4 * 1024 * 64];

__kernel void vector_max_packets_init(int vector_max_packets) {
  __pipe_init_intel(buf2pipe(vector_max_packets_pipe), 4, vector_max_packets);
}

__kernel void vector_max_packets_reader() {
  __global struct __pipe_t *p = buf2pipe(vector_max_packets_pipe);
  int num = 4 * max(256, p->max_packets);
  for (int i = 0; i < num; ++i) {
    float4 dst = __read_pipe_2_bl_intel_v4f32(p);
    ASSERT(all(isequal(dst, (float4)(0.1f * i))));
  }
  __flush_read_pipe(p);
}

__kernel void vector_max_packets_writer() {
  __global struct __pipe_t *p = buf2pipe(vector_max_packets_pipe);
  ASSERT(p->max_packets > p->write_buf.limit);
  int num = 4 * max(256, p->max_packets);
  for (int i = 0; i < num; ++i) {
    float4 src = (float4)(0.1f * i);
    __write_pipe_2_bl_intel_v4f32(p, src);
  }
  __flush_write_pipe(p);
}

// --- VectorBruteForce kernels ---

char vector_bruteforce_pipe[sizeof(struct __pipe_t) + 4 * 128];

__kernel void vector_bruteforce_init() {
  __pipe_init_intel(buf2pipe(vector_bruteforce_pipe), 4, 128);
}

__kernel void vector_bruteforce_reader() {
  __global struct __pipe_t *p = buf2pipe(vector_bruteforce_pipe);

  for (int i = 0; i < 256; ++i) {
    float dst_scalar = 0.0f;
    __read_pipe_2_bl_intel(p, &dst_scalar);
    ASSERT(dst_scalar == (float)42.0f);

    float2 dst_vector2 = __read_pipe_2_bl_intel_v2f32(p);
    ASSERT(all(isequal(dst_vector2, (float2)(0.1f * i))));

    float4 dst_vector4 = __read_pipe_2_bl_intel_v4f32(p);
    ASSERT(all(isequal(dst_vector4, (float4)(0.1f * i))));

    float8 dst_vector8 = __read_pipe_2_bl_intel_v8f32(p);
    ASSERT(all(isequal(dst_vector8, (float8)(0.1f * i))));

    float16 dst_vector16 = __read_pipe_2_bl_intel_v16f32(p);
    ASSERT(all(isequal(dst_vector16, (float16)(0.1f * i))));
  }

  __flush_read_pipe(p);
}

__kernel void vector_bruteforce_writer() {
  __global struct __pipe_t *p = buf2pipe(vector_bruteforce_pipe);

  for (int i = 0; i < 256; ++i) {
    float src_scalar = (float)42.0f;
    __write_pipe_2_bl_intel(p, &src_scalar);

    float2 src_vector2 = (float2)(0.1f * i);
    __write_pipe_2_bl_intel_v2f32(p, src_vector2);

    float4 src_vector4 = (float4)(0.1f * i);
    __write_pipe_2_bl_intel_v4f32(p, src_vector4);

    float8 src_vector8 = (float8)(0.1f * i);
    __write_pipe_2_bl_intel_v8f32(p, src_vector8);

    float16 src_vector16 = (float16)(0.1f * i);
    __write_pipe_2_bl_intel_v16f32(p, src_vector16);
  }

  __flush_write_pipe(p);
}
