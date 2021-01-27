// RUN: %clang_cc1 -triple spir64 -fopenmp -fintel-openmp-region -fopenmp-targets=spir64 %s -emit-llvm -o - | FileCheck %s

// Check that the addrspacecasts are generated as specified by UseAutoOpenCLAddrSpaceForOpenMP LangOpt:
//   "For address space agnostic languages deduce address space as:"
//   " - use target AS 0 for alloca objects;"
//   " - use LangAS::opencl_global for constant and global variables;"
//   " - use LangAS::Default for all other pointer types;"
//   " - map LangAS::Default to target AS 4."

// CHECK-DAG: %struct.user_struct = type { i32 addrspace(4)* }
// CHECK-DAG: @user_constant_global = {{.*}}target_declare addrspace(1) global i32
// CHECK-DAG: @user_nonconstant_global = {{.*}}target_declare addrspace(1) global i32
// CHECK-DAG: define{{.*}}spir_func void @user_function(i32 addrspace(4)* [[USER_ARG:[^)]*]])
// CHECK-DAG: [[USER_ARG_ADDR:%.*]] = alloca i32 addrspace(4)*
// CHECK-DAG: [[USER_ARG_ADDR_CAST:%.*]] = addrspacecast i32 addrspace(4)** [[USER_ARG_ADDR]] to i32 addrspace(4)* addrspace(4)*
// CHECK-DAG: [[USER_STRUCT_INST:%.*]] = alloca %struct.user_struct
// CHECK-DAG: [[USER_STRUCT_INST_CAST:%.*]] = addrspacecast %struct.user_struct* [[USER_STRUCT_INST]] to %struct.user_struct addrspace(4)
// CHECK-DAG: store i32 addrspace(4)* [[USER_ARG]], i32 addrspace(4)* addrspace(4)* [[USER_ARG_ADDR_CAST]]
// CHECK-DAG: [[USER_ARG_VAL:%.*]] = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* [[USER_ARG_ADDR_CAST]]
// CHECK-DAG: [[MEMBER_P:%.*]] = getelementptr inbounds %struct.user_struct, %struct.user_struct addrspace(4)* [[USER_STRUCT_INST_CAST]], i32 0, i32 0
// CHECK-DAG: store i32 addrspace(4)* [[USER_ARG_VAL]], i32 addrspace(4)* addrspace(4)* [[MEMBER_P]]
#pragma omp declare target
int user_constant_global = 0;
int user_nonconstant_global;
#pragma omp end declare target

struct user_struct {
  int *p;
};

void user_function(int *user_arg) {
  struct user_struct user_struct_inst;
  user_struct_inst.p = user_arg;
}
