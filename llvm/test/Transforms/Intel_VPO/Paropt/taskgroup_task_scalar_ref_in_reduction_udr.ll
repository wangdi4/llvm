; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
; #include <stdio.h>
;
; void my_init(short* in, short* orig) {*in = 10; printf("init\n");}
; #pragma omp declare reduction (my_add: short : omp_out = omp_out + omp_in) initializer(my_init(&omp_priv, &omp_orig))
;
; short a;
; void foo(short &a) {
; //#pragma omp parallel num_threads(10)
; //#pragma omp single
; #pragma omp taskgroup task_reduction(my_add:a)
;   {
; #pragma omp task in_reduction(my_add:a)
;     {
; //      printf("task\n");
;       a += 1;
;     }
;   }
; }
;
; //int main() {
; //    a = 0;
; //    foo(a);
; //    printf("%d\n", a);
; //    return 0;
; //}

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
; CHECK: [[RED_INIT_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %{{[^ ,]+}}, i32 0, i32 3
; CHECK: store ptr @[[RED_INIT_FUNC:[^ ]+task_red_init[^ ]+]], ptr [[RED_INIT_GEP]], align 8
; CHECK: [[RED_FINI_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %{{[^ ,]+}}, i32 0, i32 4
; CHECK: store ptr null, ptr [[RED_FINI_GEP]], align 8
; CHECK: [[RED_COMB_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %{{[^ ,]+}}, i32 0, i32 5
; CHECK: store ptr @[[RED_COMB_FUNC:[^ ]+task_red_comb[^ ]+]], ptr [[RED_COMB_GEP]], align 8

; Check that the replacement var for the reduction operand is the correct type
; CHECK: [[A_RED:%[^ ]+]] = call ptr @__kmpc_task_reduction_get_th_data{{.+}}
; CHECK: store ptr [[A_RED]], ptr {{%[^ ]+}}

; Check that the initializer/combiner functions are as expected.
; CHECK: define internal void @[[RED_INIT_FUNC]](ptr [[PRIV_ARG:%[^ ]+]], ptr [[ORIG_ARG:%[^ ]+]]) {
; CHECK: call void @.omp_initializer.(ptr [[PRIV_ARG]], ptr [[ORIG_ARG]])

; CHECK: define internal void @[[RED_COMB_FUNC]](ptr [[RHS:%[^ ]+]], ptr [[LHS:%[^ ]+]]) {
; CHECK: call void @.omp_combiner.(ptr [[RHS]], ptr [[LHS]])

; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"init\0A\00", align 1
@a = dso_local global i16 0, align 2

declare dso_local void @_Z7my_initPsS_(ptr noundef %in, ptr noundef %orig)

declare dso_local i32 @printf(ptr noundef, ...)

define dso_local void @_Z3fooRs(ptr noundef nonnull align 2 dereferenceable(2) %a) {
entry:
  %a.addr = alloca ptr, align 8
  store ptr %a, ptr %a.addr, align 8
  %0 = load ptr, ptr %a.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
     "QUAL.OMP.REDUCTION.UDR:BYREF.TYPED"(ptr %a.addr, i16 0, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer.) ]

  %2 = load ptr, ptr %a.addr, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.INREDUCTION.UDR:BYREF.TYPED"(ptr %a.addr, i16 0, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer.) ]

  %4 = load ptr, ptr %a.addr, align 8
  %5 = load i16, ptr %4, align 2
  %conv = sext i16 %5 to i32
  %add = add nsw i32 %conv, 1
  %conv1 = trunc i32 %add to i16
  store i16 %conv1, ptr %4, align 2
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKGROUP"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @.omp_combiner.(ptr noalias noundef %0, ptr noalias noundef %1)

declare void @.omp_initializer.(ptr noalias noundef %0, ptr noalias noundef %1)
