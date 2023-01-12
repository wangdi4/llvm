// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc \
//RUN:  -disable-llvm-passes -fopenmp -fopenmp-targets=spir64    \
//RUN:  -fopenmp-late-outline -fintel-compatibility -Werror      \
//RUN:  -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 -emit-llvm -disable-llvm-passes   \
//RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline     \
//RUN:  -fintel-compatibility -fopenmp-is-device -verify  -o - %s   \
//RUN:  -fopenmp-host-ir-file-path %t_host.bc -Wdevice-printf-literals\
//RUN:  | FileCheck %s

extern "C" int printf(const char*,...);
void foo(const unsigned char *) {};
const char * bar(const char * x) {return x;};
void do_char_string_test() {
const unsigned char C[] = "Hello";
#pragma omp target
// expected-warning@+1 {{non-string-literal argument used in printf is not supported for spir64}}
  printf("%s, %s\n", C, "\x57\x6F\x72\x6C\x64\x21");
#pragma omp target
  printf("Hello"); // no error
foo(C);  // Const char used here
#pragma omp target
// expected-warning@+1 {{non-string-literal argument used in printf is not supported for spir64}}
  printf("%s, %s\n", bar("Hello"), "\x57\x6F\x72\x6C\x64\x21");
}

// CHECK: [[C:@__const._Z19do_char_string_testv.C]] = private unnamed_addr addrspace(1) constant [6 x i8] c"Hello\00"
// CHECK: [[STR:@.str]] = private unnamed_addr addrspace(2) constant [8 x i8] c"%s, %s\0A\00",
// CHECK: [[STR1:@.str.1]] = private unnamed_addr addrspace(2) constant [7 x i8] c"World!\00", align 1
// CHECK: [[STR2:@.str.2]] = private unnamed_addr addrspace(2) constant [6 x i8] c"Hello\00", align 1
// CHECK: [[STR3:@.str.3]] = private unnamed_addr addrspace(1) constant [6 x i8] c"Hello\00", align 1
// CHECK-LABEL: @_Z19do_char_string_testv(
// CHECK:    "DIR.OMP.TARGET"()
// CHECK: call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) [[STR]], ptr addrspace(4) %arraydecay, ptr addrspace(2) [[STR1]])
// CHECK:    "DIR.OMP.END.TARGET"
// CHECK:    "DIR.OMP.TARGET"()
// CHECK: call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) [[STR2]])
// CHECK:    "DIR.OMP.END.TARGET"
// CHECK:    "DIR.OMP.TARGET"()
// CHECK:    call spir_func noundef ptr addrspace(4) @_Z3barPKc(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) [[STR3]] to ptr addrspace(4)))
// CHECK:    call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) [[STR]], ptr addrspace(4) %call, ptr addrspace(2) [[STR1]])
// CHECK:    "DIR.OMP.END.TARGET"()
//
// end INTEL_COLLAB
