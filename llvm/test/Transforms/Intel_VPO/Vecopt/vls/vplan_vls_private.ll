; RUN: opt -S -passes=vplan-vec -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; Check that VLS doesn't crash in presence of privates.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@buffer = external global [8192 x i32], align 16

define void @foo() {
; #pragma omp ... private(var)
; for (i = 0; i < 4096; i += 1) {
;   buffer[2*i+0] = 8;
;   var = 42;
;   buffer[2*i+1] = 31;
; }
;
; No VLS optimization happens in the loop due to unknown store to the private
; variable between stores to the buffer:
;
; CHECK-LABEL: vector.body
; CHECK:      call void @llvm.masked.scatter.v2i32.v2p0
; CHECK:      call void @llvm.masked.scatter.v2i32.v2p0
; CHECK-NOT:  call void @llvm.masked.scatter.v2i32.v2p0
; CHECK-LABEL: VPlannedBB4:
;
entry:
  %var = alloca i32, align 4
  br label %omp.entry

omp.entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %var, i32 0, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.entry ]
  %1 = shl nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [8192 x i32], ptr @buffer, i64 0, i64 %1
  store i32 8, ptr %arrayidx, align 8
  store i32 42, ptr %var, align 4
  %2 = or i64 %1, 1
  %arrayidx11 = getelementptr inbounds [8192 x i32], ptr @buffer, i64 0, i64 %2
  store i32 31, ptr %arrayidx11, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ne i64 %indvars.iv.next, 4096
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() #0
declare void @llvm.directive.region.exit(token) #0
