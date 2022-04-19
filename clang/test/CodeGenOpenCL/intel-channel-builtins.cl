// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK,CL20
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -emit-llvm -opaque-pointers -D USE_ARRAYS %s -o - | FileCheck %s -check-prefixes=CHECK,CL20
// RUN: %clang_cc1 -x cl -triple spir -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK,CL12
// RUN: %clang_cc1 -x cl -triple spir -emit-llvm -opaque-pointers -D USE_ARRAYS %s -o - | FileCheck %s -check-prefixes=CHECK,CL12
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple x86_64-pc-windows-intelfpga -emit-llvm -opaque-pointers -fopencl-force-vector-abi %s -o - | FileCheck %s --check-prefix CHECKWIN

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

// CHECK: declare void @_Z19write_channel_intel11ocl_channelii(ptr {{.*}}, i32 noundef)
// CHECK: declare void @_Z19write_channel_intel11ocl_channelll(ptr {{.*}}, i64 noundef)
// CHECK: declare void @_Z19write_channel_intel11ocl_channel2stS_(ptr {{.*}}, ptr noundef byval(%struct.st) align 4)
// CHECK: declare void @_Z19write_channel_intel11ocl_channelDv4_fS_(ptr {{.*}}, <4 x float> noundef)
//
// CHECKWIN: declare dso_local void @"?write_channel_intel@@$$J0YAXU?$ocl_channel@H@__clang@@H@Z"(ptr, i32 noundef)
// CHECKWIN: declare dso_local void @"?write_channel_intel@@$$J0YAXU?$ocl_channel@J@__clang@@J@Z"(ptr, i64 noundef)
// CHECKWIN: declare dso_local void @"?write_channel_intel@@$$J0YAXU?$ocl_channel@Ust@@@__clang@@Ust@@@Z"(ptr, ptr noundef)
// CHECKWIN: declare dso_local void @"?write_channel_intel@@$$J0YAXU?$ocl_channel@T?$__vector@M$03@__clang@@@__clang@@T?$__vector@M$03@2@@Z"(ptr, <4 x float> noundef)
//
// CHECK: declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(ptr {{.*}}, i32 noundef)
// CHECK: declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelll(ptr {{.*}}, i64 noundef)
// CHECK: declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channel2stS_(ptr {{.*}}, ptr noundef byval(%struct.st) align 4)
// CHECK: declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelDv4_fS_(ptr {{.*}}, <4 x float> noundef)
//
// CHECKWIN: declare dso_local zeroext i1 @"?write_channel_nb_intel@@$$J0YA_NU?$ocl_channel@H@__clang@@H@Z"(ptr, i32 noundef)
// CHECKWIN: declare dso_local zeroext i1 @"?write_channel_nb_intel@@$$J0YA_NU?$ocl_channel@J@__clang@@J@Z"(ptr, i64 noundef)
// CHECKWIN: declare dso_local zeroext i1 @"?write_channel_nb_intel@@$$J0YA_NU?$ocl_channel@Ust@@@__clang@@Ust@@@Z"(ptr, ptr noundef)
// CHECKWIN: declare dso_local zeroext i1 @"?write_channel_nb_intel@@$$J0YA_NU?$ocl_channel@T?$__vector@M$03@__clang@@@__clang@@T?$__vector@M$03@2@@Z"(ptr, <4 x float> noundef)

// CHECK: declare i32 @_Z18read_channel_intel11ocl_channeli(ptr {{.*}})
// CHECK: declare i64 @_Z18read_channel_intel11ocl_channell(ptr {{.*}})
// CHECK: declare void @_Z18read_channel_intel11ocl_channel2st(ptr sret(%struct.st){{( align 4)?}}, ptr {{.*}})
// CHECK: declare <4 x float> @_Z18read_channel_intel11ocl_channelDv4_f(ptr {{.*}})
//
// CHECKWIN: declare dso_local i32 @"?read_channel_intel@@$$J0YAHU?$ocl_channel@H@__clang@@@Z"(ptr)
// CHECKWIN: declare dso_local i64 @"?read_channel_intel@@$$J0YAJU?$ocl_channel@J@__clang@@@Z"(ptr)
// CHECKWIN: declare dso_local void @"?read_channel_intel@@$$J0YA?AUst@@U?$ocl_channel@Ust@@@__clang@@@Z"(ptr sret(%struct.st){{( align 4)?}}, ptr)
// CHECKWIN: declare dso_local <4 x float> @"?read_channel_intel@@$$J0YAT?$__vector@M$03@__clang@@U?$ocl_channel@T?$__vector@M$03@__clang@@@2@@Z"(ptr)
//
// CL12: declare i64 @_Z21read_channel_nb_intel11ocl_channellPU3AS3b(ptr {{.*}}, ptr addrspace(3) noundef)
// CL12: declare void @_Z21read_channel_nb_intel11ocl_channel2stPb(ptr sret(%struct.st){{( align 4)?}}, ptr {{.*}}, ptr noundef)
// CL12: declare <4 x float> @_Z21read_channel_nb_intel11ocl_channelDv4_fPb(ptr {{.*}}, ptr noundef)
//
// CL20: declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(ptr {{.*}}, ptr addrspace(4) noundef)
// CL20: declare i64 @_Z21read_channel_nb_intel11ocl_channellPU3AS4b(ptr {{.*}}, ptr addrspace(4) noundef)
// CL20: declare void @_Z21read_channel_nb_intel11ocl_channel2stPU3AS4b(ptr sret(%struct.st){{( align 4)?}}, ptr{{.*}}, ptr addrspace(4) noundef)
// CL20: declare <4 x float> @_Z21read_channel_nb_intel11ocl_channelDv4_fPU3AS4b(ptr {{.*}}, ptr addrspace(4) noundef)
//
// CHECKWIN: declare dso_local i32 @"?read_channel_nb_intel@@$$J0YAHU?$ocl_channel@H@__clang@@PEAU?$_ASCLgeneric@$$CA_N@2@@Z"(ptr, ptr noundef)
// CHECKWIN: declare dso_local i64 @"?read_channel_nb_intel@@$$J0YAJU?$ocl_channel@J@__clang@@PEAU?$_ASCLgeneric@$$CA_N@2@@Z"(ptr, ptr noundef)
// CHECKWIN: declare dso_local void @"?read_channel_nb_intel@@$$J0YA?AUst@@U?$ocl_channel@Ust@@@__clang@@PEAU?$_ASCLgeneric@$$CA_N@3@@Z"(ptr sret(%struct.st){{( align 4)?}}, ptr, ptr noundef)
// CHECKWIN: declare dso_local <4 x float> @"?read_channel_nb_intel@@$$J0YAT?$__vector@M$03@__clang@@U?$ocl_channel@T?$__vector@M$03@__clang@@@2@PEAU?$_ASCLgeneric@$$CA_N@2@@Z"(ptr, ptr noundef)
//
// CL12-NOT: declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(ptr {{.*}}, ptr addrspace(4) noundef
// CL12-NOT: declare i64 @_Z21read_channel_nb_intel11ocl_channellPU3AS4b(ptr {{.*}}, ptr addrspace(4) noundef)
// CL12-NOT: declare void @_Z21read_channel_nb_intel11ocl_channel2stPU3AS4b(ptr sret(%struct.st), ptr {{.*}}, ptr addrspace(4) noundef)
// CL12-NOT: declare <4 x float> @_Z21read_channel_nb_intel11ocl_channelDv4_fPU3AS4b(ptr {{.*}}, ptr addrspace(4) noundef)
//
// CHECK-NOT: declare i32 @_Z21read_channel_nb_intel11ocl_channeliPb(ptr {{.*}}, ptr noundef)
// CHECK-NOT: declare i64 @_Z21read_channel_nb_intel11ocl_channellPb(ptr {{.*}}, ptr noundef)
// CHECK-NOT: declare void @_Z21read_channel_nb_intel11ocl_channel2stPb(ptr sret(%struct.st){{( align 4)?}}, ptr {{.*}}, ptr noundef)
// CHECK-NOT: declare <4 x float> @_Z21read_channel_nb_intel11ocl_channelDv4_fPb(ptr {{.*}}, ptr noundef)
//
// CHECK-NOT: declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS3b(ptr {{.*}}, ptr addrspace(3) noundef)
// CHECK-NOT: declare i64 @_Z21read_channel_nb_intel11ocl_channellPU3AS3b(ptr {{.*}}, ptr addrspace(3) noundef)
// CHECK-NOT: declare void @_Z21read_channel_nb_intel11ocl_channel2stPU3AS3b(ptr sret(%struct.st), ptr {{.*}}, ptr addrspace(3) noundef)
// CHECK-NOT: declare <4 x float> @_Z21read_channel_nb_intel11ocl_channelDv4_fPU3AS3b(ptr {{.*}}, ptr addrspace(3) noundef)
//
// CHECK-NOT: declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS1b(ptr {{.*}}, ptr addrspace(1) noundef)
// CHECK-NOT: declare i64 @_Z21read_channel_nb_intel11ocl_channellPU3AS1b(ptr {{.*}}, ptr addrspace(1) noundef)
// CHECK-NOT: declare void @_Z21read_channel_nb_intel11ocl_channel2stPU3AS1b(ptr sret(%struct.st), ptr {{.*}}, ptr addrspace(1) noundef)
// CHECK-NOT: declare <4 x float> @_Z21read_channel_nb_intel11ocl_channelDv4_fPU3AS1b(ptr {{.*}}, ptr addrspace(1) noundef)

#if __OPENCL_C_VERSION__ == 200
__global bool g_valid;
#endif

__kernel void k1() {
    int i = 5;
    long l = 100500;
    struct st s = { 1, 2 };
    float4 f = { 0.1, 0.2, 0.3, 0.4 };

    bool valid;
    __local bool l_valid;
    __private bool p_valid;

    bool *p_to_valid = &valid;
#if __OPENCL_C_VERSION__ == 200
    __global bool *p_to_g_valid = &g_valid;
#endif
    __local bool *p_to_l_valid = &l_valid;
    __private bool *p_to_p_valid = &p_valid;

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

#if __OPENCL_C_VERSION__ == 200
    i = read_channel_nb_intel(ich_arr[3], p_to_g_valid);
#endif
    l = read_channel_nb_intel(lch_arr[3][2], p_to_l_valid);
    s = read_channel_nb_intel(sch_arr[3][2][1], p_to_p_valid);
    f = read_channel_nb_intel(fch_arr[0], p_to_valid);
    f = read_channel_nb_intel(fch_arr1[0], &valid);
#else
    i = read_channel_intel(ich);
    l = read_channel_intel(lch);
    s = read_channel_intel(sch);
    f = read_channel_intel(fch);

#if __OPENCL_C_VERSION__ == 200
    i = read_channel_nb_intel(ich, p_to_g_valid);
#endif
    l = read_channel_nb_intel(lch, p_to_l_valid);
    s = read_channel_nb_intel(sch, p_to_p_valid);
    f = read_channel_nb_intel(fch, p_to_valid);
    f = read_channel_nb_intel(fch1, &valid);
#endif
}
