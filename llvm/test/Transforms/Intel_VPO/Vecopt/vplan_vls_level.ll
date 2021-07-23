; RUN: opt -S -mattr=avx -vplan-vec  < %s | FileCheck %s --check-prefix=ENABLED
; RUN: opt -S -mattr=avx -vplan-vls-level=never  -vplan-vec  < %s | FileCheck %s --check-prefix=DISABLED
; RUN: opt -S -mattr=avx -vplan-vls-level=always -vplan-vec  < %s | FileCheck %s --check-prefix=ENABLED
; RUN: opt -S -mattr=avx -vplan-vls-level=auto   -vplan-vec  < %s | FileCheck %s --check-prefix=ENABLED

; RUN: opt -S -mattr=avx2 -vplan-vec  < %s | FileCheck %s --check-prefix=DISABLED
; RUN: opt -S -mattr=avx2 -vplan-vls-level=never  -vplan-vec  < %s | FileCheck %s --check-prefix=DISABLED
; RUN: opt -S -mattr=avx2 -vplan-vls-level=always -vplan-vec  < %s | FileCheck %s --check-prefix=DISABLED
; RUN: opt -S -mattr=avx2 -vplan-vls-level=auto   -vplan-vec  < %s | FileCheck %s --check-prefix=DISABLED
; RUN: opt -S -mattr=avx2 -enable-intel-advanced-opts -vplan-vls-level=auto -vplan-vec  < %s | FileCheck %s --check-prefix=ENABLED

; RUN: opt -S -mattr=avx512f -vplan-vec  < %s | FileCheck %s --check-prefix=DISABLED
; RUN: opt -S -mattr=avx512f -vplan-vls-level=never  -vplan-vec  < %s | FileCheck %s --check-prefix=DISABLED
; RUN: opt -S -mattr=avx512f -vplan-vls-level=always -vplan-vec  < %s | FileCheck %s --check-prefix=DISABLED
; RUN: opt -S -mattr=avx512f -vplan-vls-level=auto   -vplan-vec  < %s | FileCheck %s --check-prefix=DISABLED
; RUN: opt -S -mattr=avx512f -enable-intel-advanced-opts -vplan-vls-level=always -vplan-vec  < %s | FileCheck %s --check-prefix=ENABLED

; ENABLED:      @llvm.masked.load.v16i32
; ENABLED:      @llvm.masked.store.v16i32

; DISABLED-NOT: <16 x i32>

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ary, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add7 = add nsw i32 %0, 7
  %1 = add nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds i32, i32* %ary, i64 %1
  %2 = load i32, i32* %arrayidx4, align 4
  %add11 = add nsw i32 %2, 11
  %3 = add nsw i64 %indvars.iv, 2
  %arrayidx8 = getelementptr inbounds i32, i32* %ary, i64 %3
  %4 = load i32, i32* %arrayidx8, align 4
  %add12 = add nsw i32 %4, 12
  store i32 %add7, i32* %arrayidx, align 4
  store i32 %add11, i32* %arrayidx4, align 4
  store i32 %add12, i32* %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp = icmp ult i64 %indvars.iv.next, 3072
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
