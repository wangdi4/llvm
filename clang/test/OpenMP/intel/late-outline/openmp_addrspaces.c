//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu -fopenmp    \
//RUN:  -fopenmp-late-outline -fopenmp-targets=spir64,x86_64   \
//RUN:  %s -emit-llvm -o - | FileCheck %s -check-prefix=HOST

//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu -fopenmp    \
//RUN:  -fopenmp-late-outline -fopenmp-targets=spir64,x86_64   \
//RUN:  %s -emit-llvm-bc -o %t-host.bc

//RUN: %clang_cc1 -opaque-pointers -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64,x86_64 \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc \
//RUN:  %s -emit-llvm -o - | FileCheck %s -check-prefix=SPIR

//RUN: %clang_cc1 -opaque-pointers -triple x86_64 -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64,x86_64 \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc \
//RUN:  %s -emit-llvm -o - | FileCheck %s -check-prefix=HOST

// Check that the addrspacecasts are generated for OpenMP with SPIR target:
//   "For address space agnostic languages deduce address space as:"
//   " - use target AS 0 for alloca objects;"
//   " - use LangAS::opencl_global for constant and global variables;"
//   " - use LangAS::Default for all other pointer types;"
//   " - map LangAS::Default to target AS 4."

// SPIR-DAG: %struct.user_struct = type { ptr addrspace(4) }
// SPIR-DAG: @user_constant_global = {{.*}}target_declare addrspace(1) global i32
// SPIR-DAG: @user_nonconstant_global = {{.*}}target_declare addrspace(1) global i32
// SPIR-DAG: define{{.*}}spir_func void @user_function(ptr addrspace(4) noundef [[USER_ARG:[^)]*]])
// SPIR-DAG: [[USER_ARG_ADDR:%.*]] = alloca ptr addrspace(4)
// SPIR-DAG: [[USER_ARG_ADDR_CAST:%.*]] = addrspacecast ptr [[USER_ARG_ADDR]] to ptr addrspace(4)
// SPIR-DAG: [[USER_STRUCT_INST:%.*]] = alloca %struct.user_struct
// SPIR-DAG: [[USER_STRUCT_INST_CAST:%.*]] = addrspacecast ptr [[USER_STRUCT_INST]] to ptr addrspace(4)
// SPIR-DAG: store ptr addrspace(4) [[USER_ARG]], ptr addrspace(4) [[USER_ARG_ADDR_CAST]]
// SPIR-DAG: [[USER_ARG_VAL:%.*]] = load ptr addrspace(4), ptr addrspace(4) [[USER_ARG_ADDR_CAST]]
// SPIR-DAG: [[MEMBER_P:%.*]] = getelementptr inbounds %struct.user_struct, ptr addrspace(4) [[USER_STRUCT_INST_CAST]], i32 0, i32 0
// SPIR-DAG: store ptr addrspace(4) [[USER_ARG_VAL]], ptr addrspace(4) [[MEMBER_P]]
// HOST-DAG: %struct.user_struct = type { ptr }
// HOST-DAG: @user_constant_global = {{.*}}target_declare global i32
// HOST-DAG: @user_nonconstant_global = {{.*}}target_declare global i32
// HOST-DAG: define{{.*}} void @user_function(ptr noundef [[USER_ARG:[^)]*]])
// HOST-DAG: [[USER_ARG_ADDR:%.*]] = alloca ptr
// HOST-DAG: [[USER_STRUCT_INST:%.*]] = alloca %struct.user_struct
// HOST-DAG: store ptr [[USER_ARG]], ptr [[USER_ARG_ADDR]]
// HOST-DAG: [[USER_ARG_VAL:%.*]] = load ptr, ptr [[USER_ARG_ADDR]]
// HOST-DAG: [[MEMBER_P:%.*]] = getelementptr inbounds %struct.user_struct, ptr [[USER_STRUCT_INST]], i32 0, i32 0
// HOST-DAG: store ptr [[USER_ARG_VAL]], ptr [[MEMBER_P]]

#pragma omp declare target
int user_constant_global = 0;
int user_nonconstant_global;

struct user_struct {
  int *p;
};

void user_function(int *user_arg) {
  struct user_struct user_struct_inst;
  user_struct_inst.p = user_arg;
}
#pragma omp end declare target
