; RUN: opt -S -passes=vplan-vec -debug-only=ovls -disable-output -print-after=vplan-vec < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; Check that we don't optimize atomic nor volatile accesses.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test_1(ptr nocapture %ary) {
;  for (i = 0; i < 2048; i += 2) {
;    t0 = ary[i + 0] + 7;
;    t1 = ary[i + 1] + 11; // <- volatile load
;    ary[i + 0] = t0;
;    ary[i + 1] = t1; // <- atomic store
;  }
;
; CHECK:       Received a vector of memrefs (2):
; CHECK-NEXT:    #1 <4 x 32> SLoad:
; CHECK-NEXT:    #2 <4 x 32> SStore:
;
; CHECK:       call <4 x i32> @llvm.masked.gather.v4i32.v4p0
; CHECK:       load volatile i32
; CHECK:       load volatile i32
; CHECK:       load volatile i32
; CHECK:       load volatile i32
;
; CHECK:       call void @llvm.masked.scatter.v4i32.v4p0
; CHECK:       store atomic i32 {{.*}} unordered, align 4
; CHECK:       store atomic i32 {{.*}} unordered, align 4
; CHECK:       store atomic i32 {{.*}} unordered, align 4
; CHECK:       store atomic i32 {{.*}} unordered, align 4
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx.0 = getelementptr inbounds i32, ptr %ary, i64 %indvars.iv
  %t0 = load i32, ptr %arrayidx.0, align 4
  %add7 = add nsw i32 %t0, 7
  %indvars.iv.1 = add nsw i64 %indvars.iv, 1
  %arrayidx.1 = getelementptr inbounds i32, ptr %ary, i64 %indvars.iv.1
  %t1 = load volatile i32, ptr %arrayidx.1, align 4
  %add11 = add nsw i32 %t1, 11
  store i32 %add7, ptr %arrayidx.0, align 4
  store atomic i32 %add11, ptr %arrayidx.1 unordered, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 2048
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define void @test_2(ptr nocapture %ary) {
;  for (i = 0; i < 2048; i += 2) {
;    t0 = ary[i + 0] + 7;  // <- atomic load
;    t1 = ary[i + 1] + 11;
;    ary[i + 0] = t0; // <- volatile store
;    ary[i + 1] = t1;
;  }
;
; CHECK:       Received a vector of memrefs (2):
; CHECK-NEXT:    #3 <4 x 32> SLoad:
; CHECK-NEXT:    #4 <4 x 32> SStore:
;
; CHECK:       load atomic i32, {{.*}} unordered, align 4
; CHECK:       load atomic i32, {{.*}} unordered, align 4
; CHECK:       load atomic i32, {{.*}} unordered, align 4
; CHECK:       load atomic i32, {{.*}} unordered, align 4
; CHECK:       call <4 x i32> @llvm.masked.gather.v4i32.v4p0
;
; CHECK:       store volatile i32
; CHECK:       store volatile i32
; CHECK:       store volatile i32
; CHECK:       store volatile i32
; CHECK:       call void @llvm.masked.scatter.v4i32.v4p0
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx.0 = getelementptr inbounds i32, ptr %ary, i64 %indvars.iv
  %t0 = load atomic i32, ptr %arrayidx.0 unordered, align 4
  %add7 = add nsw i32 %t0, 7
  %indvars.iv.1 = add nsw i64 %indvars.iv, 1
  %arrayidx.1 = getelementptr inbounds i32, ptr %ary, i64 %indvars.iv.1
  %t1 = load i32, ptr %arrayidx.1, align 4
  %add11 = add nsw i32 %t1, 11
  store volatile i32 %add7, ptr %arrayidx.0, align 4
  store i32 %add11, ptr %arrayidx.1, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 2048
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
