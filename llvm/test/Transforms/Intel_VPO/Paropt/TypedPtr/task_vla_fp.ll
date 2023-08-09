; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
; #include <stdio.h>
;
; void foo(int n) {
;   short a[n];
;   a[1] = 1;
; #pragma omp task firstprivate(a)
;   {
;     printf("%d\n", a[1]);
;     a[1] = 2;
;   }
;   printf("%d\n", a[1]);
; }
;
; // int main() {
; //   foo(10);
; // }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; Check for the space allocated for the private copy.
; CHECK: %__struct.kmp_privates.t = type { i16*, i64, i64 }

define dso_local void @foo(i32 %n) {
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
  %vla = alloca i16, i64 %1, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  %arrayidx = getelementptr inbounds i16, i16* %vla, i64 1
  store i16 1, i16* %arrayidx, align 2
  store i64 %1, i64* %omp.vla.tmp, align 8

; Check for computation of VLA size in bytes
; CHECK: %vla = alloca i16, i64 [[NUM_ELEMENTS:%[^, ]+]]
; CHECK: [[VLA_SIZE_IN_BYTES:%[^ ]+]] = mul i64 [[NUM_ELEMENTS]], 2

; Check that we call _task_alloc with total size of task_t_with_privates + vla_size
; CHECK: [[TOTAL_SIZE:%[^ ]+]] = add i64 96, [[VLA_SIZE_IN_BYTES]]
; CHECK: [[TASK_ALLOC:[^ ]+]] = call i8* @__kmpc_omp_task_alloc({{.*}}i64 [[TOTAL_SIZE]]{{.*}})

; Check that VLA size and offset are stored in the thunk
; CHECK: [[VLA_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^ ]+}}, i32 0, i32 1
; CHECK: store i64 [[VLA_SIZE_IN_BYTES]], i64* [[VLA_SIZE_GEP]]
; CHECK: [[VLA_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^ ]+}}, i32 0, i32 2
; CHECK: store i64 96, i64* [[VLA_OFFSET_GEP]]

; Check that the local buffer space for %vla, allocated with __kmpc_alloc, is initialized (as it is firstprivate).
; CHECK: [[VLA_BUFFER:%[^ ]+]] = getelementptr i8, i8* [[TASK_ALLOC]], i64 96
; CHECK: [[VLA_CAST:%[^ ]+]] = bitcast i16* %vla to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 [[VLA_BUFFER]], i8* align 2 [[VLA_CAST]], i64 [[VLA_SIZE_IN_BYTES]], i1 false)

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE"(i16* %vla),
    "QUAL.OMP.SHARED"(i64* %omp.vla.tmp) ]

; Inside the outlined function, check that the i16* in the privates thunk is linked to the array allocated in the buffer at the end.
; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}
; CHECK: [[VLA_PRIV_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^, ]+}}, i32 0, i32 0
; CHECK: [[VLA_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^, ]+}}, i32 0, i32 2
; CHECK: [[VLA_OFFSET:%[^ ]+]] = load i64, i64* [[VLA_OFFSET_GEP]]
; CHECK: [[THUNK_BASE_PTR:%[^ ]+]] = bitcast %__struct.kmp_task_t_with_privates* {{[^, ]+}} to i8*
; CHECK: [[VLA_DATA:%[^ ]+]]  = getelementptr i8, i8* [[THUNK_BASE_PTR]], i64 [[VLA_OFFSET]]
; CHECK: [[VLA_PRIV_GEP_CAST:[^ ]+]] = bitcast i16** [[VLA_PRIV_GEP]] to i8**
; CHECK: store i8* [[VLA_DATA]], i8** [[VLA_PRIV_GEP_CAST]]

; Check that a load from VLA_PRIV_GEP is done to get an i16*, which replaces %vla inside the region.
; CHECK: [[VLA_PRIV_GEP1:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^ ,]+}}, i32 0, i32 0
; CHECK: [[VLA_NEW:%[^ ]+]] = load i16*, i16** [[VLA_PRIV_GEP1]]
; CHECK: [[GEP1:%[^ ]+]] = getelementptr inbounds i16, i16* [[VLA_NEW]], i64 1
; CHECK: [[GEP2:%[^ ]+]] = getelementptr inbounds i16, i16* [[VLA_NEW]], i64 1
; CHECK: store i16 2, i16* [[GEP2]]


  %4 = load i64, i64* %omp.vla.tmp, align 8
  %arrayidx1 = getelementptr inbounds i16, i16* %vla, i64 1
  %5 = load i16, i16* %arrayidx1, align 2
  %conv = sext i16 %5 to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %conv)
  %arrayidx2 = getelementptr inbounds i16, i16* %vla, i64 1
  store i16 2, i16* %arrayidx2, align 2

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]

  %arrayidx3 = getelementptr inbounds i16, i16* %vla, i64 1
  %6 = load i16, i16* %arrayidx3, align 2
  %conv4 = sext i16 %6 to i32
  %call5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %conv4)
  %7 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %7)
  ret void
}

declare i8* @llvm.stacksave()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(i8*, ...)
declare void @llvm.stackrestore(i8*)
