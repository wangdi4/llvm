// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -emit-llvm %s -o - | FileCheck %s

typedef float float4 __attribute__((ext_vector_type(4)));

#pragma OPENCL EXTENSION cl_altera_channels : enable

struct st {
  int i1;
  int i2;
};

channel int ich;
channel long lch;
channel struct st sch;
channel float4 fch;

channel int ich1;
channel long lch1;
channel struct st sch1;
channel float4 fch1;

// Declarations documented in 'Intel FPGA SDK for OpenCL Programming Guide'
// You can find it at https://www.altera.com/products/design-software/embedded-software-developers/opencl/documentation.html

// void write_channel_altera(channel <type> channel_id, const <type> data);
// bool write_channel_nb_altera(channel <type> channel_id, const <type> data);
//
// <type> read_channel_altera(channel <type> channel_id);
// <type> read_channel_nb_altera(channel <type> channel_id, bool * valid);

// CHECK: declare void @_Z20write_channel_altera11ocl_channelii(%opencl.channel_t {{.*}}*, i32)
// CHECK: declare void @_Z20write_channel_altera11ocl_channelll(%opencl.channel_t {{.*}}*, i64)
// CHECK: declare void @_Z20write_channel_altera11ocl_channel2stS_(%opencl.channel_t {{.*}}*, %struct.st* byval align 4)
// CHECK: declare void @_Z20write_channel_altera11ocl_channelDv4_fS_(%opencl.channel_t {{.*}}*, <4 x float>)
//
// CHECK: declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channelii(%opencl.channel_t {{.*}}*, i32)
// CHECK: declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channelll(%opencl.channel_t {{.*}}*, i64)
// CHECK: declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channel2stS_(%opencl.channel_t {{.*}}*, %struct.st* byval align 4)
// CHECK: declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channelDv4_fS_(%opencl.channel_t {{.*}}*, <4 x float>)

// CHECK: declare i32 @_Z19read_channel_altera11ocl_channeli(%opencl.channel_t {{.*}}*)
// CHECK: declare i64 @_Z19read_channel_altera11ocl_channell(%opencl.channel_t {{.*}}*)
// CHECK: declare void @_Z19read_channel_altera11ocl_channel2st(%struct.st* sret, %opencl.channel_t {{.*}}*)
// CHECK: declare <4 x float> @_Z19read_channel_altera11ocl_channelDv4_f(%opencl.channel_t {{.*}}*)
//
// CHECK: declare i32 @_Z22read_channel_nb_altera11ocl_channeliPU3AS4b(%opencl.channel_t {{.*}}*, i8 addrspace(4)*)
// CHECK: declare i64 @_Z22read_channel_nb_altera11ocl_channellPU3AS4b(%opencl.channel_t {{.*}}*, i8 addrspace(4)*)
// CHECK: declare void @_Z22read_channel_nb_altera11ocl_channel2stPU3AS4b(%struct.st* sret, %opencl.channel_t {{.*}}*, i8 addrspace(4)*)
// CHECK: declare <4 x float> @_Z22read_channel_nb_altera11ocl_channelDv4_fPU3AS4b(%opencl.channel_t {{.*}}*, i8 addrspace(4)*)
//
// CHECK-NOT: declare i32 @_Z22read_channel_nb_altera11ocl_channeliPb(%opencl.channel_t {{.*}}*, i8*)
// CHECK-NOT: declare i64 @_Z22read_channel_nb_altera11ocl_channellPb(%opencl.channel_t {{.*}}*, i8*)
// CHECK-NOT: declare void @_Z22read_channel_nb_altera11ocl_channel2stPb(%struct.st* sret, %opencl.channel_t {{.*}}*, i8*)
// CHECK-NOT: declare <4 x float> @_Z22read_channel_nb_altera11ocl_channelDv4_fPb(%opencl.channel_t {{.*}}*, i8*)
//
// CHECK-NOT: declare i32 @_Z22read_channel_nb_altera11ocl_channeliPU3AS3b(%opencl.channel_t {{.*}}*, i8 addrspace(3)*)
// CHECK-NOT: declare i64 @_Z22read_channel_nb_altera11ocl_channellPU3AS3b(%opencl.channel_t {{.*}}*, i8 addrspace(3)*)
// CHECK-NOT: declare void @_Z22read_channel_nb_altera11ocl_channel2stPU3AS3b(%struct.st* sret, %opencl.channel_t {{.*}}*, i8 addrspace(3)*)
// CHECK-NOT: declare <4 x float> @_Z22read_channel_nb_altera11ocl_channelDv4_fPU3AS3b(%opencl.channel_t {{.*}}*, i8 addrspace(3)*)
//
// CHECK-NOT: declare i32 @_Z22read_channel_nb_altera11ocl_channeliPU3AS1b(%opencl.channel_t {{.*}}*, i8 addrspace(1)*)
// CHECK-NOT: declare i64 @_Z22read_channel_nb_altera11ocl_channellPU3AS1b(%opencl.channel_t {{.*}}*, i8 addrspace(1)*)
// CHECK-NOT: declare void @_Z22read_channel_nb_altera11ocl_channel2stPU3AS1b(%struct.st* sret, %opencl.channel_t {{.*}}*, i8 addrspace(1)*)
// CHECK-NOT: declare <4 x float> @_Z22read_channel_nb_altera11ocl_channelDv4_fPU3AS1b(%opencl.channel_t {{.*}}*, i8 addrspace(1)*)

__global bool g_valid;

__kernel void k1() {
    int i = 5;
    long l = 100500;
    struct st s = { 1, 2 };
    float4 f = { 0.1, 0.2, 0.3, 0.4 };

    bool valid;
    __local bool l_valid;
    __private bool p_valid;

    bool *p_to_valid = &valid;
    __global bool *p_to_g_valid = &g_valid;
    __local bool *p_to_l_valid = &l_valid;
    __private bool *p_to_p_valid = &p_valid;

    write_channel_altera(ich, i);
    write_channel_altera(lch, l);
    write_channel_altera(sch, s);
    write_channel_altera(fch, f);

    valid = write_channel_nb_altera(ich, i);
    valid = write_channel_nb_altera(lch, l);
    valid = write_channel_nb_altera(sch, s);
    valid = write_channel_nb_altera(fch, f);

    i = read_channel_altera(ich);
    l = read_channel_altera(lch);
    s = read_channel_altera(sch);
    f = read_channel_altera(fch);

    i = read_channel_nb_altera(ich, p_to_g_valid);
    l = read_channel_nb_altera(lch, p_to_l_valid);
    s = read_channel_nb_altera(sch, p_to_p_valid);
    f = read_channel_nb_altera(fch, p_to_valid);
    f = read_channel_nb_altera(fch1, &valid);
}
