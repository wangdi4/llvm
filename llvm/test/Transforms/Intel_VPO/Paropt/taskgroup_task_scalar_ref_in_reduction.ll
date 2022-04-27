; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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

; ModuleID = 'taskgroup_task_scalar_ref_in_reduction.cpp'
source_filename = "taskgroup_task_scalar_ref_in_reduction.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that the shared struct has space for %a.addr, and
; reduction item struct type is defined.
; CHECK: %__struct.kmp_privates.t = type {}
; CHECK: %__struct.kmp_taskred_input_t = type { i8*, i8*, i64, i8*, i8*, i8*, i32 }
; CHECK: %__struct.shared.t = type { i16** }

; Check that the reduction struct is correctly initialized
; using the size of i16, and a dereference of the byref %a.addr
; CHECK: [[RED_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 0
; CHECK: [[A_BYREF_DREF:%[^ ]+]] = load i16*, i16** %a.addr
; CHECK: [[A_BYREF_DEREF_CAST:%[^ ]+]] = bitcast i16* [[A_BYREF_DREF]] to i8*
; CHECK: store i8* [[A_BYREF_DEREF_CAST]], i8** [[RED_ITEM_PTR_GEP]]
; CHECK: [[ORIG_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 1
; CHECK: [[A_BYREF_DEREF_CAST1:%[^ ]+]] = bitcast i16* [[A_BYREF_DREF]] to i8*
; CHECK: store i8* [[A_BYREF_DEREF_CAST1]], i8** [[ORIG_ITEM_PTR_GEP]]
; CHECK: [[RED_ITEM_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 2
; CHECK: store i64 2, i64* [[RED_ITEM_SIZE_GEP]]

; Check that the replacement var for the reduction operand is the correct type
; CHECK: [[A_RED:%[^ ]+]] = call i8* @__kmpc_task_reduction_get_th_data{{.+}}
; CHECK: [[A_RED_CAST:%[^ ]+]] = bitcast i8* [[A_RED]] to i16*
; CHECK: store i16* [[A_RED_CAST]], i16** {{%[^ ]+}}

; Check that the initializer/combiner functions are as expected.
; CHECK: define internal void @{{.*}}task_red_init{{.*}}(i16* [[PRIV_ARG:%[^ ]+]], i16* [[ORIG_ARG:%[^ ]+]]) {
; CHECK: store i16 0, i16* [[PRIV_ARG]]

; CHECK: define internal void @{{.*}}task_red_comb{{.*}}(i16* [[RHS:%[^ ]+]], i16* [[LHS:%[^ ]+]]) {
; CHECK: [[LHS_VAL:%[^ ]]] = load i16, i16* [[LHS]]
; CHECK: [[RHS_VAL:%[^ ]]] = load i16, i16* [[RHS]]
; CHECK: [[RES:%[^ ]+]] = add i16 [[RHS_VAL]], [[LHS_VAL]]
; CHECK: store i16 [[RES]], i16* [[RHS]]

@a = dso_local global i16 0, align 2
; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z3fooRs(i16* dereferenceable(2) %a) #0 {
entry:
  %a.addr = alloca i16*, align 8
  store i16* %a, i16** %a.addr, align 8
  %0 = load i16*, i16** %a.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(), "QUAL.OMP.REDUCTION.ADD:BYREF"(i16** %a.addr) ]
  %2 = load i16*, i16** %a.addr, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.INREDUCTION.ADD:BYREF"(i16** %a.addr) ]
  %4 = load i16*, i16** %a.addr, align 8
  %5 = load i16, i16* %4, align 2
  %conv = sext i16 %5 to i32
  %add = add nsw i32 %conv, 2
  %conv1 = trunc i32 %add to i16
  store i16 %conv1, i16* %4, align 2
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKGROUP"() ]
  ret void
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
