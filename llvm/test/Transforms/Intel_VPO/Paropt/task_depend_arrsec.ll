; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; #include <stdio.h>
; #include <omp.h>
;
; void foo(int n) {
;
;     int i = 10;
;     char*a, *b, *c, d;
;     a = &d; b = &d; c = &d;
; #pragma omp task depend(in:a[1:2], b[0:n]) depend(out:c[:0]) firstprivate(i)
;     {
;         printf("%d\n", i);
;     }
; }
;
; // int main() {
; //     foo(2);
; //     return 0;
; // }

; ModuleID = 'task_depend_arrsec.c'
source_filename = "task_depend_arrsec.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %a = alloca i8*, align 8
  %b = alloca i8*, align 8
  %c = alloca i8*, align 8
  %d = alloca i8, align 1
  store i32 %n, i32* %n.addr, align 4
  store i32 10, i32* %i, align 4
  store i8* %d, i8** %a, align 8
  store i8* %d, i8** %b, align 8
  store i8* %d, i8** %c, align 8
  %0 = load i32, i32* %n.addr, align 4
  %conv = sext i32 %0 to i64

; Check that private thunk has space for firstprivate %i,
; CHECK: %__struct.kmp_privates.t = type { i32 }

; Check that the depend struct is populated correctly for %a
; CHECK: [[A_LOAD:%[^ ]+]] = load i8*, i8** %a
; CHECK: [[A_LOAD_PLUS_OFFSET:%[^ ]+]]  = getelementptr i8, i8* [[A_LOAD]], i64 1
; CHECK: [[DEP_VEC_A_PTR:%[^ ]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* {{[^ ,]+}}, i32 0, i32 0
; CHECK: [[A_LOAD_PLUS_OFFSET_CAST:%[^ ]+]] = ptrtoint i8* [[A_LOAD_PLUS_OFFSET]] to i64
; CHECK: store i64 [[A_LOAD_PLUS_OFFSET_CAST]], i64* [[DEP_VEC_A_PTR]]
; CHECK: [[DEP_VEC_A_NUM_BYTES:%[^ ]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* {{[^ ,]+}}, i32 0, i32 1
; CHECK: store i64 2, i64* [[DEP_VEC_A_NUM_BYTES]]
; CHECK: [[DEP_VEC_A_FLAGS:%[^ ]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* {{[^ ,]+}}, i32 0, i32 2
; CHECK: store i8 1, i8* [[DEP_VEC_A_FLAGS]]

; Check that the depend struct is populated correctly for %b
; CHECK: [[B_SIZE:%[^ ]+]] = mul i64 1, %conv
; CHECK: [[B_LOAD:%[^ ]+]] = load i8*, i8** %b
; CHECK: [[B_LOAD_PLUS_OFFSET:%[^ ]+]]  = getelementptr i8, i8* [[B_LOAD]], i64 0
; CHECK: [[B_SIZE_IN_BYTES:[^ ]+]] = mul i64 [[B_SIZE]], 1
; CHECK: [[DEP_VEC_B_PTR:%[^ ]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* {{[^ ,]+}}, i32 0, i32 0
; CHECK: [[B_LOAD_PLUS_OFFSET_CAST:%[^ ]+]] = ptrtoint i8* [[B_LOAD_PLUS_OFFSET]] to i64
; CHECK: store i64 [[B_LOAD_PLUS_OFFSET_CAST]], i64* [[DEP_VEC_B_PTR]]
; CHECK: [[DEP_VEC_B_NUM_BYTES:%[^ ]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* {{[^ ,]+}}, i32 0, i32 1
; CHECK: store i64 [[B_SIZE_IN_BYTES]], i64* [[DEP_VEC_B_NUM_BYTES]]
; CHECK: [[DEP_VEC_B_FLAGS:%[^ ]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* {{[^ ,]+}}, i32 0, i32 2
; CHECK: store i8 1, i8* [[DEP_VEC_B_FLAGS]]

; Check that the depend struct is populated correctly for %c
; CHECK: [[C_LOAD:%[^ ]+]] = load i8*, i8** %c
; CHECK: [[C_LOAD_PLUS_OFFSET:%[^ ]+]]  = getelementptr i8, i8* [[C_LOAD]], i64 0
; CHECK: [[DEP_VEC_C_PTR:%[^ ]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* {{[^ ,]+}}, i32 0, i32 0
; CHECK: [[C_LOAD_PLUS_OFFSET_CAST:%[^ ]+]] = ptrtoint i8* [[C_LOAD_PLUS_OFFSET]] to i64
; CHECK: store i64 [[C_LOAD_PLUS_OFFSET_CAST]], i64* [[DEP_VEC_C_PTR]]
; CHECK: [[DEP_VEC_C_NUM_BYTES:%[^ ]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* {{[^ ,]+}}, i32 0, i32 1
; CHECK: store i64 0, i64* [[DEP_VEC_C_NUM_BYTES]]
; CHECK: [[DEP_VEC_C_FLAGS:%[^ ]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* {{[^ ,]+}}, i32 0, i32 2
; CHECK: store i8 3, i8* [[DEP_VEC_C_FLAGS]]

; Check that a call to task_with_deps is emitted
; CHECK: call void @__kmpc_omp_task_with_deps{{.*}}

; Check that the outlined function only takes two arguments, one for thread ID, and one for task thunk.
; CHECK: define internal void @foo.DIR.OMP.TASK{{.*}}(i32 %{{[^ ,]+}}, %__struct.kmp_task_t_with_privates* %{{[^ ]+}})

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.DEPEND.IN:ARRSECT"(i8** %a, i64 1, i64 1, i64 2, i64 1), "QUAL.OMP.DEPEND.IN:ARRSECT"(i8** %b, i64 1, i64 0, i64 %conv, i64 1), "QUAL.OMP.DEPEND.OUT:ARRSECT"(i8** %c, i64 1, i64 0, i64 0, i64 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %i) ]

  %2 = load i32, i32* %i, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %2)

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  ret void
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
