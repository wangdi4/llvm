; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; short a = 0;
;
; void foo(short &a) {
; #pragma omp taskgroup task_reduction(+:a)
;   {
; #pragma omp task in_reduction(+:a)
;     {
;       a += 2;
;     }
;   }
; }
;
; // int main() {
; //   a = 1;
; //   foo(a);
; //   printf("%d\n", a);
; //   return 0;
; // }

; Check that the shared struct has space for %a.addr, and
; reduction item struct type is defined.
; CHECK: %__struct.kmp_privates.t = type {}
; CHECK: %__struct.kmp_taskred_input_t = type { ptr, ptr, i64, ptr, ptr, ptr, i32 }
; CHECK: %__struct.shared.t = type { ptr }

; Check that the reduction struct is correctly initialized
; using the size of i16, and a dereference of the byref %a.addr
; CHECK: [[RED_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %{{[^ ,]+}}, i32 0, i32 0
; CHECK: [[A_BYREF_DREF:%[^ ]+]] = load ptr, ptr %a.addr
; CHECK: store ptr [[A_BYREF_DREF]], ptr [[RED_ITEM_PTR_GEP]]
; CHECK: [[ORIG_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %{{[^ ,]+}}, i32 0, i32 1
; CHECK: store ptr [[A_BYREF_DREF]], ptr [[ORIG_ITEM_PTR_GEP]]
; CHECK: [[RED_ITEM_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %{{[^ ,]+}}, i32 0, i32 2
; CHECK: store i64 2, ptr [[RED_ITEM_SIZE_GEP]]

; Check that the replacement var for the reduction operand is the correct type
; CHECK: [[A_RED:%[^ ]+]] = call ptr @__kmpc_task_reduction_get_th_data{{.+}}
; CHECK: store ptr [[A_RED]], ptr {{%[^ ]+}}

; Check that the initializer/combiner functions are as expected.
; CHECK: define internal void @{{.*}}task_red_init{{.*}}(ptr [[PRIV_ARG:%[^ ]+]], ptr [[ORIG_ARG:%[^ ]+]]) {
; CHECK: store i16 0, ptr [[PRIV_ARG]]

; CHECK: define internal void @{{.*}}task_red_comb{{.*}}(ptr [[RHS:%[^ ]+]], ptr [[LHS:%[^ ]+]]) {
; CHECK: [[LHS_VAL:%[^ ]]] = load i16, ptr [[LHS]]
; CHECK: [[RHS_VAL:%[^ ]]] = load i16, ptr [[RHS]]
; CHECK: [[RES:%[^ ]+]] = add i16 [[RHS_VAL]], [[LHS_VAL]]
; CHECK: store i16 [[RES]], ptr [[RHS]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global i16 0, align 2

define dso_local void @_Z3fooRs(ptr noundef nonnull align 2 dereferenceable(2) %a) {
entry:
  %a.addr = alloca ptr, align 8
  store ptr %a, ptr %a.addr, align 8
  %0 = load ptr, ptr %a.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
     "QUAL.OMP.REDUCTION.ADD:BYREF.TYPED"(ptr %a.addr, i16 0, i32 1) ]

  %2 = load ptr, ptr %a.addr, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.INREDUCTION.ADD:BYREF.TYPED"(ptr %a.addr, i16 0, i32 1) ]

  %4 = load ptr, ptr %a.addr, align 8
  %5 = load i16, ptr %4, align 2
  %conv = sext i16 %5 to i32
  %add = add nsw i32 %conv, 2
  %conv1 = trunc i32 %add to i16
  store i16 %conv1, ptr %4, align 2
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKGROUP"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
