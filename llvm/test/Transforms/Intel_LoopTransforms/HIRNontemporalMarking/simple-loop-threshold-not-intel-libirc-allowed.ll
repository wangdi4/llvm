; RUN: opt -enable-intel-advanced-opts -S -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-nontemporal-marking,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check to make sure that we do not convert to nontemporal for a simple loop
; when -intel-libirc-allowed is not passed, unless the store is already
; vector-aligned.

; Before:
; <0>          BEGIN REGION { }
; <10>               + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; <4>                |   (%dest)[i1] = i1;
; <10>               + END LOOP
; <0>          END REGION

define void @example(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: example
; CHECK-NOT: !nontemporal
; CHECK-NOT: @llvm.x86.sse.sfence();

entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds i64, ptr %dest, i64 %index
  store i64 %index, ptr %addr, align 8
  %cond = icmp eq i64 %index, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Before:
; <0>          BEGIN REGION { }
; <15>               + DO i1 = 0, 800000, 1   <DO_LOOP>
; <2>                |   %index.vecfirst = insertelement undef,  8 * i1,  0;
; <3>                |   %index.splat = shufflevector %index.vecfirst,  poison,  zeroinitializer;
; <4>                |   %index.vec = %index.splat  |  <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>;
; <5>                |   %index.vecfp = sitofp.<8 x i64>.<8 x double>(%index.vec);
; <9>                |   (<8 x double>*)(%dest)[8 * i1] = %index.vecfp; // {al:8}
; <15>               + END LOOP
; <0>          END REGION

define void @vec-unaligned(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: vec-unaligned
; CHECK-NOT: !nontemporal
; CHECK-NOT: @llvm.x86.sse.sfence();

entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.vecfirst = insertelement <8 x i64> undef, i64 %index, i64 0
  %index.splat = shufflevector <8 x i64> %index.vecfirst, <8 x i64> poison, <8 x i32> zeroinitializer
  %index.vec = or <8 x i64> %index.splat, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  %index.vecfp = sitofp <8 x i64> %index.vec to <8 x double>
  %index.next = add i64 %index, 8
  %addr = getelementptr inbounds double, ptr %dest, i64 %index
  %addr.vec = bitcast ptr %addr to ptr
  store <8 x double> %index.vecfp, ptr %addr.vec, align 8
  %cond = icmp eq i64 %index, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

; Before:
; <0>          BEGIN REGION { }
; <15>               + DO i1 = 0, 800000, 1   <DO_LOOP>
; <2>                |   %index.vecfirst = insertelement undef,  8 * i1,  0;
; <3>                |   %index.splat = shufflevector %index.vecfirst,  poison,  zeroinitializer;
; <4>                |   %index.vec = %index.splat  |  <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>;
; <5>                |   %index.vecfp = sitofp.<8 x i64>.<8 x double>(%index.vec);
; <9>                |   (<8 x double>*)(%dest)[8 * i1] = %index.vecfp; // {al:64}
; <15>               + END LOOP
; <0>          END REGION

define void @vec-aligned(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: vec-aligned
; CHECK:            |   <LVAL-REG> {al:64}(<8 x double>*)(LINEAR ptr %dest)[LINEAR i64 8 * i1] inbounds  !nontemporal
; CHECK:               @llvm.x86.sse.sfence();

entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.vecfirst = insertelement <8 x i64> undef, i64 %index, i64 0
  %index.splat = shufflevector <8 x i64> %index.vecfirst, <8 x i64> poison, <8 x i32> zeroinitializer
  %index.vec = or <8 x i64> %index.splat, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  %index.vecfp = sitofp <8 x i64> %index.vec to <8 x double>
  %index.next = add i64 %index, 8
  %addr = getelementptr inbounds double, ptr %dest, i64 %index
  %addr.vec = bitcast ptr %addr to ptr
  store <8 x double> %index.vecfp, ptr %addr.vec, align 64
  %cond = icmp eq i64 %index, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}
