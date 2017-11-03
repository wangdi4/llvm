// RUN: %clang_cc1 -x cl -cl-std=CL1.2 -triple spir -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -x cl -cl-std=CL1.2 -triple spir -emit-llvm -D USE_ARRAYS %s -o - | FileCheck %s
// XFAIL: *
// Test disabled due to https://jira01.devtools.intel.com/browse/CORC-1903
// [FPGA][Clang] Allow channels declaration for -cl-std=1.x

typedef float float4 __attribute__((ext_vector_type(4)));

#pragma OPENCL EXTENSION cl_intel_channels : enable

struct st {
  int i1;
  int i2;
};

#ifdef USE_ARRAYS
channel int ich_arr[5];
channel long lch_arr[5][4];
channel struct st sch_arr[5][4][3];
channel float4 fch_arr[1];

channel int ich_arr1[5];
channel long lch_arr1[5][4];
channel struct st sch_arr1[5][4][3];
channel float4 fch_arr1[1];
#else
channel int ich;
channel long lch;
channel struct st sch;
channel float4 fch;

channel int ich1;
channel long lch1;
channel struct st sch1;
channel float4 fch1;
#endif

// Declarations documented in 'Intel FPGA SDK for OpenCL Programming Guide'
// You can find it at https://www.altera.com/products/design-software/embedded-software-developers/opencl/documentation.html

// void write_channel_intel(channel <type> channel_id, const <type> data);
// bool write_channel_nb_intel(channel <type> channel_id, const <type> data);
//
// <type> read_channel_intel(channel <type> channel_id);
// <type> read_channel_nb_intel(channel <type> channel_id, bool * valid);

// CHECK: declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t {{.*}}*, i32)
// CHECK: declare void @_Z19write_channel_intel11ocl_channelll(%opencl.channel_t {{.*}}*, i64)
// CHECK: declare void @_Z19write_channel_intel11ocl_channel2stS_(%opencl.channel_t {{.*}}*, %struct.st* byval align 4)
// CHECK: declare void @_Z19write_channel_intel11ocl_channelDv4_fS_(%opencl.channel_t {{.*}}*, <4 x float>)
//
// CHECK: declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t {{.*}}*, i32)
// CHECK: declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelll(%opencl.channel_t {{.*}}*, i64)
// CHECK: declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channel2stS_(%opencl.channel_t {{.*}}*, %struct.st* byval align 4)
// CHECK: declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelDv4_fS_(%opencl.channel_t {{.*}}*, <4 x float>)

// CHECK: declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t {{.*}}*)
// CHECK: declare i64 @_Z18read_channel_intel11ocl_channell(%opencl.channel_t {{.*}}*)
// CHECK: declare void @_Z18read_channel_intel11ocl_channel2st(%struct.st* sret, %opencl.channel_t {{.*}}*)
// CHECK: declare <4 x float> @_Z18read_channel_intel11ocl_channelDv4_f(%opencl.channel_t {{.*}}*)
//
// CHECK: declare i32 @_Z21read_channel_nb_intel11ocl_channeliPb(%opencl.channel_t {{.*}}*, i8*)
// CHECK: declare i64 @_Z21read_channel_nb_intel11ocl_channellPb(%opencl.channel_t {{.*}}*, i8*)
// CHECK: declare void @_Z21read_channel_nb_intel11ocl_channel2stPb(%struct.st* sret, %opencl.channel_t {{.*}}*, i8*)
// CHECK: declare <4 x float> @_Z21read_channel_nb_intel11ocl_channelDv4_fPb(%opencl.channel_t {{.*}}*, i8*)
//
// CHECK: declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS3b(%opencl.channel_t {{.*}}*, i8 addrspace(3)*)
// CHECK: declare i64 @_Z21read_channel_nb_intel11ocl_channellPU3AS3b(%opencl.channel_t {{.*}}*, i8 addrspace(3)*)
// CHECK: declare void @_Z21read_channel_nb_intel11ocl_channel2stPU3AS3b(%struct.st* sret, %opencl.channel_t {{.*}}*, i8 addrspace(3)*)
// CHECK: declare <4 x float> @_Z21read_channel_nb_intel11ocl_channelDv4_fPU3AS3b(%opencl.channel_t {{.*}}*, i8 addrspace(3)*)
//
// CHECK: declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS1b(%opencl.channel_t {{.*}}*, i8 addrspace(1)*)
// CHECK: declare i64 @_Z21read_channel_nb_intel11ocl_channellPU3AS1b(%opencl.channel_t {{.*}}*, i8 addrspace(1)*)
// CHECK: declare void @_Z21read_channel_nb_intel11ocl_channel2stPU3AS1b(%struct.st* sret, %opencl.channel_t {{.*}}*, i8 addrspace(1)*)
// CHECK: declare <4 x float> @_Z21read_channel_nb_intel11ocl_channelDv4_fPU3AS1b(%opencl.channel_t {{.*}}*, i8 addrspace(1)*)

__kernel void k1(__global bool *p_to_g_valid) {
    int i = 5;
    long l = 100500;
    struct st s = { 1, 2 };
    float4 f = { 0.1, 0.2, 0.3, 0.4 };
    bool valid;
    __private bool p_valid;
    __local bool l_valid;

    __private bool *p_to_p_valid = &p_valid;
    __local bool *p_to_l_valid = &l_valid;

#ifdef USE_ARRAYS
    write_channel_intel(ich_arr[3], i);
    write_channel_intel(lch_arr[3][2], l);
    write_channel_intel(sch_arr[3][2][1], s);
    write_channel_intel(fch_arr[0], f);

    valid = write_channel_nb_intel(ich_arr[3], i);
    valid = write_channel_nb_intel(lch_arr[3][2], l);
    valid = write_channel_nb_intel(sch_arr[3][2][1], s);
    valid = write_channel_nb_intel(fch_arr[0], f);
#else
    write_channel_intel(ich, i);
    write_channel_intel(lch, l);
    write_channel_intel(sch, s);
    write_channel_intel(fch, f);

    valid = write_channel_nb_intel(ich, i);
    valid = write_channel_nb_intel(lch, l);
    valid = write_channel_nb_intel(sch, s);
    valid = write_channel_nb_intel(fch, f);
#endif

#ifdef USE_ARRAYS
    i = read_channel_intel(ich_arr[3]);
    l = read_channel_intel(lch_arr[3][2]);
    s = read_channel_intel(sch_arr[3][2][1]);
    f = read_channel_intel(fch_arr[0]);

    i = read_channel_nb_intel(ich_arr[3], p_to_g_valid);
    l = read_channel_nb_intel(lch_arr[3][2], p_to_l_valid);
    s = read_channel_nb_intel(sch_arr[3][2][1], p_to_p_valid);
    f = read_channel_nb_intel(fch_arr[0], &valid);
#else
    i = read_channel_intel(ich);
    l = read_channel_intel(lch);
    s = read_channel_intel(sch);
    f = read_channel_intel(fch);

    i = read_channel_nb_intel(ich, p_to_g_valid);
    l = read_channel_nb_intel(lch, p_to_l_valid);
    s = read_channel_nb_intel(sch, p_to_p_valid);
    f = read_channel_nb_intel(fch, &valid);
#endif
}

