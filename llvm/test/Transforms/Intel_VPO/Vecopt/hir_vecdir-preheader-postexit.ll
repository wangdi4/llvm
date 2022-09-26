; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vec-dir-insert -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-force-vf=4 -print-after=hir-vec-dir-insert -disable-output < %s 2>&1 | FileCheck %s

;
; The test checks that HIRParVecAnalysis doesn't look into loop preheader or
; postexit blocks when checking legality of vectorization. The test code has
; non-vectorizable memset calls in preheader and postexit blocks.
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64* nocapture %ary, i64 %size.inner) {
; for (i = 0; i < 128; ++i) {
;   if (size_inner != 0) {
;     memset(ary, 0, 8);
;     for (j = 0; j != size_inner; ++j)
;       ary[j] = j;
;     memset(ary, 0, 8);
;   }
; }
;
; CHECK:   *** IR Dump After{{.+}}Vec{{.*}}Dir{{.*}}Insert{{.*}}Pass{{.*}} ***
; CHECK:       BEGIN REGION { }
; CHECK-NEXT:        + DO i1 = 0, 127, 1   <DO_LOOP>
; CHECK-NEXT:        |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK-NEXT:        |
; CHECK-NEXT:        |      @llvm.memset.p0i64.i64(&((%ary)[0]),  0,  8,  0);
; CHECK-NEXT:        |   + DO i2 = 0, %size.inner + -1, 1   <DO_LOOP>
; CHECK-NEXT:        |   |   (%ary)[i2] = i2;
; CHECK-NEXT:        |   + END LOOP
; CHECK-NEXT:        |      @llvm.memset.p0i64.i64(&((%ary)[0]),  0,  8,  0);
; CHECK-NEXT:        |
; CHECK-NEXT:        |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:  END REGION
;
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        + DO i1 = 0, 127, 1   <DO_LOOP>
; CHECK-NEXT:        |   if (%size.inner != 0)
; CHECK-NEXT:        |   {
; CHECK-NEXT:        |      @llvm.memset.p0i64.i64(&((%ary)[0]),  0,  8,  0);

; CHECK:             |         + DO i2 = 0, {{.*}}, 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:        |         |   (<4 x i64>*)(%ary)[i2] = i2 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:        |         + END LOOP

; CHECK:             |      + DO i2 = {{.*}}, %size.inner + -1, 1   <DO_LOOP>
; CHECK-NEXT:        |      |   (%ary)[i2] = i2;
; CHECK-NEXT:        |      + END LOOP

; CHECK:             |      @llvm.memset.p0i64.i64(&((%ary)[0]),  0,  8,  0);
; CHECK-NEXT:        |   }
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:  END REGION
;
entry:
  br label %loop.outer

loop.outer:
  %i = phi i64 [ 0, %entry ], [ %i.next, %loop.outer.latch ]
  %ztt = icmp eq i64 %size.inner, 0
  br i1 %ztt, label %loop.outer.latch, label %preheader

preheader:
  call void @llvm.memset.p0i64.i64(i64* %ary, i8 0, i64 8, i1 false)
  br label %for.body

for.body:
  %j = phi i64 [ 0, %preheader ], [ %j.next, %for.body ]
  %ptr = getelementptr inbounds i64, i64* %ary, i64 %j
  store i64 %j, i64* %ptr, align 8
  %j.next = add nsw i64 %j, 1
  %cmp.j = icmp ne i64 %j.next, %size.inner
  br i1 %cmp.j, label %for.body, label %postexit

postexit:
  call void @llvm.memset.p0i64.i64(i64* %ary, i8 0, i64 8, i1 false)
  br label %loop.outer.latch

loop.outer.latch:
  %i.next = add nsw i64 %i, 1
  %cmp.i = icmp slt i64 %i.next, 128
  br i1 %cmp.i, label %loop.outer, label %exit

exit:
  ret void
}

declare void @llvm.memset.p0i64.i64(i64*, i8, i64, i1)
