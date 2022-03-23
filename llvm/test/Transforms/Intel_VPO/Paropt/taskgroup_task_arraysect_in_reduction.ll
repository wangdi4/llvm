; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; int a[5] = {2, 2, 2, 2, 2};
; int *b;
;
; void foo() {
; // #pragma omp parallel
; // #pragma omp master
; #pragma omp taskgroup task_reduction(*:b[1:4])
;   {
; #pragma omp task in_reduction(*:b[1:4])
;     {
; //      printf("  %d %d %d %d\n", b[1], b[2], b[3], b[4]);
;       b[1] *= 2; b[4] *= 3 ;
; //      printf("  %d %d %d %d\n",  b[1], b[2], b[3], b[4]);
;     }
;   }
; }
;
; // int main() {
; //   b = &a[0];
; //   foo();
; //   printf("%d %d %d %d %d\n", a[0], a[1], a[2], a[3], a[4]);
; //   return 0;
; // }

; ModuleID = 'taskgroup_task_arraysect_in_reduction.cpp'
source_filename = "taskgroup_task_arraysect_in_reduction.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global [5 x i32] [i32 2, i32 2, i32 2, i32 2, i32 2], align 16
@b = dso_local global i32* null, align 8

; Check that the shared struct has space for @b, and
; reduction item struct type is defined.
; CHECK: %__struct.kmp_privates.t = type {}
; CHECK: %__struct.kmp_taskred_input_t = type { i8*, i8*, i64, i8*, i8*, i8*, i32 }
; CHECK: %__struct.shared.t = type { i32** }

; Check that the reduction struct is correctly initialized
; using the size of i16, and a dereference of the byref %a.addr
; CHECK: [[RED_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 0
; CHECK: [[B_LOAD:%[^ ]+]] = load i32*, i32** @b
; CHECK: [[B_LOAD_PLUS_OFFSET:%[^ ]+]] = getelementptr i32, i32* [[B_LOAD]], i64 1
; CHECK: [[B_LOAD_PLUS_OFFSET_CAST:%[^ ]+]] = bitcast i32* [[B_LOAD_PLUS_OFFSET]] to i8*
; CHECK: store i8* [[B_LOAD_PLUS_OFFSET_CAST]], i8** [[RED_ITEM_PTR_GEP]]
; CHECK: [[ORIG_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 1
; CHECK: [[B_LOAD_PLUS_OFFSET_CAST1:%[^ ]+]] = bitcast i32* [[B_LOAD_PLUS_OFFSET]] to i8*
; CHECK: store i8* [[B_LOAD_PLUS_OFFSET_CAST1]], i8** [[ORIG_ITEM_PTR_GEP]]
; CHECK: [[RED_ITEM_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 2
; CHECK: store i64 16, i64* [[RED_ITEM_SIZE_GEP]]

; Check that the replacement var for the reduction operand is the correct type
; CHECK: [[B_RED:%[^ ]+]] = call i8* @__kmpc_task_reduction_get_th_data{{.+}}
; CHECK: [[B_RED_CAST:%[^ ]+]] = bitcast i8* [[B_RED]] to i32*
; CHECK: [[B_RED_CAST_MINUS_OFFSET:%[^ ]+]] = getelementptr i32, i32* [[B_RED_CAST]], i64 -1
; CHECK: store i32* [[B_RED_CAST_MINUS_OFFSET]], i32** {{%[^ ]+}}

; Check that the initializer/combiner functions are as expected.
; CHECK: define internal void @{{.*}}task_red_init{{.*}}(i32* [[PRIV_ARG:%[^ ]+]], i32* [[ORIG_ARG:%[^ ]+]]) {
; Check that the ending GEP for the array to be initialized is being computed
; CHECK: {{%[^ ]+}} = getelementptr i32, i32* [[PRIV_ARG]], i64 4
; CHECK: store i32 1, i32* {{.*}}

; CHECK: define internal void @{{.*}}task_red_comb{{.*}}(i32* [[RHS:%[^ ]+]], i32* [[LHS:%[^ ]+]]) {
; Check that the ending GEP for the array to be initialized is being computed
; CHECK: {{%[^ ]+}} = getelementptr i32, i32* [[RHS]], i64 4
; CHECK: [[LHS_VAL:%[^ ]]] = load i32, i32* {{.*}}
; CHECK: [[RHS_VAL:%[^ ]]] = load i32, i32* {{.*}}
; CHECK: [[RES:%[^ ]+]] = mul i32 [[RHS_VAL]], [[LHS_VAL]]
; CHECK: store i32 [[RES]], i32* {{.*}}

define internal void @_Z3foov_task_red_init_3(i32* %0) {
entry:
  %1 = getelementptr i32, i32* %0, i64 4
  %red.init.isempty = icmp eq i32* %0, %1
  br i1 %red.init.isempty, label %red.init.done, label %red.init.body

red.init.body:                                    ; preds = %red.init.body, %entry
  %red.cpy.dest.ptr = phi i32* [ %0, %entry ], [ %red.cpy.dest.inc, %red.init.body ]
  store i32 1, i32* %red.cpy.dest.ptr
  %red.cpy.dest.inc = getelementptr i32, i32* %red.cpy.dest.ptr, i32 1
  %red.cpy.done = icmp eq i32* %red.cpy.dest.inc, %1
  br i1 %red.cpy.done, label %red.init.done, label %red.init.body

red.init.done:                                    ; preds = %red.init.body, %entry
  ret void
}
define internal void @_Z3foov_task_red_comb_3(i32* %0, i32* %1) {
entry:
  %2 = getelementptr i32, i32* %0, i64 4
  %red.update.isempty = icmp eq i32* %0, %2
  br i1 %red.update.isempty, label %red.update.done, label %red.update.body

red.update.body:                                  ; preds = %red.update.body, %entry
  %red.cpy.dest.ptr = phi i32* [ %0, %entry ], [ %red.cpy.dest.inc, %red.update.body ]
  %red.cpy.src.ptr = phi i32* [ %1, %entry ], [ %red.cpy.src.inc, %red.update.body ]
  %3 = load i32, i32* %red.cpy.src.ptr
  %4 = load i32, i32* %red.cpy.dest.ptr
  %5 = mul i32 %4, %3
  store i32 %5, i32* %red.cpy.dest.ptr
  %red.cpy.dest.inc = getelementptr i32, i32* %red.cpy.dest.ptr, i32 1
  %red.cpy.src.inc = getelementptr i32, i32* %red.cpy.src.ptr, i32 1
  %red.cpy.done = icmp eq i32* %red.cpy.dest.inc, %2
  br i1 %red.cpy.done, label %red.update.done, label %red.update.body

red.update.done:                                  ; preds = %red.update.body, %entry
  ret void
}


; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(), "QUAL.OMP.REDUCTION.MUL:ARRSECT"(i32** @b, i64 1, i64 1, i64 4, i64 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.INREDUCTION.MUL:ARRSECT"(i32** @b, i64 1, i64 1, i64 4, i64 1) ]

  %2 = load i32*, i32** @b, align 8
  %arrayidx = getelementptr inbounds i32, i32* %2, i64 1
  %3 = load i32, i32* %arrayidx, align 4
  %mul = mul nsw i32 %3, 2
  store i32 %mul, i32* %arrayidx, align 4
  %4 = load i32*, i32** @b, align 8
  %arrayidx1 = getelementptr inbounds i32, i32* %4, i64 4
  %5 = load i32, i32* %arrayidx1, align 4
  %mul2 = mul nsw i32 %5, 3
  store i32 %mul2, i32* %arrayidx1, align 4

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]
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
