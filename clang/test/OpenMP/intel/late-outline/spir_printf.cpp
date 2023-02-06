// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc \
//RUN:  -disable-llvm-passes -fopenmp -fopenmp-targets=spir64    \
//RUN:  -fopenmp-late-outline -fintel-compatibility -Werror      \
//RUN:  -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 -emit-llvm -disable-llvm-passes   \
//RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline     \
//RUN:  -fintel-compatibility -fopenmp-is-device         -o - %s   \
//RUN:  -fopenmp-host-ir-file-path %t_host.bc -Wsource-uses-openmp \
//RUN:  | FileCheck %s

extern "C" int printf(const char*,...);
void foo(const unsigned char *) {};
int x;

// CHECK: [[C:@__const._Z19do_char_string_testv.C]] = private unnamed_addr addrspace(1) constant [6 x i8] c"Hello\00", align 1
// CHECK: [[STR:@.str]] = private unnamed_addr addrspace(2) constant [12 x i8] c"%s, %s, %d\0A\00", align 1
// CHECK: [[STR1:@.str.1]] = private unnamed_addr addrspace(2) constant [6 x i8] c"Hello\00", align 1
// CHECK: [[STR2:@.str.2]] = private unnamed_addr addrspace(2) constant [7 x i8] c"World!\00", align 1
// CHECK: [[X:@x]] = external addrspace(1) global i32, align 4
// CHECK: [[STR3:@.str.3]] = private unnamed_addr addrspace(1) constant [5 x i8] c"foo\0A\00"
// CHECK: [[STR4:@.str.4]] = private unnamed_addr addrspace(2) constant [5 x i8] c"foo\0A\00"

// CHECK-LABEL: @_Z19do_char_string_testv(
// CHECK:    "DIR.OMP.TARGET"()
// CHECK:    [[TMP2:%.*]] = call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) [[STR]], ptr addrspace(2) [[STR1]], ptr addrspace(2) [[STR2]], i32 [[TMP1:%.*]])
// CHECK:    "DIR.OMP.END.TARGET"()
// CHECK:    "DIR.OMP.TARGET"()
// CHECK:    [[TMP4:%.*]] = call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) [[STR1]])
// CHECK:    "DIR.OMP.END.TARGET"()
// CHECK:    [[ARRAYDECAY:%.*]] = getelementptr inbounds [6 x i8], ptr addrspace(4) [[C_ASCAST:%.*]], i64 0, i64 0
// CHECK:    call spir_func void @_Z3fooPKh(ptr addrspace(4) noundef [[ARRAYDECAY]])
// CHECK:    "DIR.OMP.TARGET"()
// CHECK:    [[TMP6:%.*]] = load i32, ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), align 4
// CHECK:    [[TMP7:%.*]] = call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) [[STR]], ptr addrspace(2) [[STR1]], ptr addrspace(2) [[STR2]], i32 [[TMP6]])
// CHECK:    "DIR.OMP.END.TARGET"()
// CHECK:     call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) [[STR3]] to ptr addrspace(4)))
// CHECK:    "DIR.OMP.TARGET"()
// CHECK:    call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) [[STR4]])
// CHECK:    "DIR.OMP.END.TARGET"()
// CHECK:    ret void
//
// CHECK: declare spir_func i32 @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2), ...)
void do_char_string_test() {
const unsigned char C[] = "Hello";
#pragma omp target
  printf("%s, %s, %d\n", "Hello", "\x57\x6F\x72\x6C\x64\x21", x);
#pragma omp target
  printf("Hello");
foo(C);  // Const char used here
#pragma omp target
  printf("%s, %s, %d\n", "Hello", "\x57\x6F\x72\x6C\x64\x21", x);
  printf("foo\n"); // same function, outside target region addrspace(1)
#pragma omp target
  printf("foo\n"); //addrspace(2)
}
// end INTEL_COLLAB
