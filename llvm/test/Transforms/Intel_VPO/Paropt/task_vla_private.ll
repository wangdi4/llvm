; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; void foo(int n) {
;   int a[n];
;   a[1] = 1;
; #pragma omp task private(a)
;   {
;     a[1] = 2;
;     printf("%d\n", a[1]);
;   }
;   printf("%d\n", a[1]);
; }
;
; // int main() {
; //   foo(10);
; // }

; ModuleID = 'task_vla_private.c'
source_filename = "task_vla_private.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; Check for the space allocated for the private copy.
; CHECK: %__struct.kmp_privates.t = type { i32*, i64, i64 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  %arrayidx = getelementptr inbounds i32, i32* %vla, i64 1
  store i32 1, i32* %arrayidx, align 4
  store i64 %1, i64* %omp.vla.tmp, align 8

; Check for computation of VLA size in bytes
; CHECK: %vla = alloca i32, i64 [[NUM_ELEMENTS:%[^, ]+]]
; CHECK: [[VLA_SIZE_IN_BYTES:%[^ ]+]] = mul i64 [[NUM_ELEMENTS]], 4

; Check that we call _task_alloc with total size of task_t_with_privates + vla_size
; CHECK: [[TOTAL_SIZE:%[^ ]+]] = add i64 96, [[VLA_SIZE_IN_BYTES]]
; CHECK: {{[^ ]+}} = call i8* @__kmpc_omp_task_alloc({{.*}}i64 [[TOTAL_SIZE]]{{.*}})

; Check that VLA size and offset are stored in the thunk
; CHECK: [[VLA_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^ ]+}}, i32 0, i32 1
; CHECK: store i64 [[VLA_SIZE_IN_BYTES]], i64* [[VLA_SIZE_GEP]]
; CHECK: [[VLA_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^ ]+}}, i32 0, i32 2
; CHECK: store i64 96, i64* [[VLA_OFFSET_GEP]]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.PRIVATE"(i32* %vla), "QUAL.OMP.SHARED"(i64* %omp.vla.tmp) ]

; Inside the outlined function, check that the i32* in the privates thunk, is linked to the array allocated in the buffer at the end.
; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}
; CHECK: [[VLA_PRIV_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^, ]+}}, i32 0, i32 0
; CHECK: [[VLA_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^, ]+}}, i32 0, i32 2
; CHECK: [[VLA_OFFSET:%[^ ]+]] = load i64, i64* [[VLA_OFFSET_GEP]]
; CHECK: [[THUNK_BASE_PTR:%[^ ]+]] = bitcast %__struct.kmp_task_t_with_privates* {{[^, ]+}} to i8*
; CHECK: [[VLA_DATA:%[^ ]+]]  = getelementptr i8, i8* [[THUNK_BASE_PTR]], i64 [[VLA_OFFSET]]
; CHECK: [[VLA_PRIV_GEP_CAST:[^ ]+]] = bitcast i32** [[VLA_PRIV_GEP]] to i8**
; CHECK: store i8* [[VLA_DATA]], i8** [[VLA_PRIV_GEP_CAST]]

; Check that a load from VLA_PRIV_GEP is done to get an i32*, which replaces %vla inside the region.
; CHECK: [[VLA_PRIV_GEP1:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^ ,]+}}, i32 0, i32 0
; CHECK: [[VLA_NEW:%[^ ]+]] = load i32*, i32** [[VLA_PRIV_GEP1]]
; CHECK: [[GEP:%[^ ]+]] = getelementptr inbounds i32, i32* [[VLA_NEW]], i64 1
; CHECK: store i32 2, i32* [[GEP]]

  %4 = load i64, i64* %omp.vla.tmp, align 8
  %arrayidx1 = getelementptr inbounds i32, i32* %vla, i64 1
  store i32 2, i32* %arrayidx1, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %vla, i64 1
  %5 = load i32, i32* %arrayidx2, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %5)

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]


  %arrayidx3 = getelementptr inbounds i32, i32* %vla, i64 1
  %6 = load i32, i32* %arrayidx3, align 4
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %6)
  %7 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %7)
  ret void
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
