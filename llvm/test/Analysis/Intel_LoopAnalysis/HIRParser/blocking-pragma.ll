; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-details -hir-framework-debug=parser 2>&1 | FileCheck %s

; Src-
;  float s;
;  #pragma block_loop factor (16) level (1)
;  #pragma block_loop factor (32) level (2) private(s)
;  for (i=0; i<256; i++) {
;    for (j=0; j<256; j++) {
;      s = 0.0;
;
;      for (k=0; k<256; k++) {
;        s += B[i][k] * C[k][j];
;      }
;
;      A[i][j] += s;
;    }
;  }

; Verify that DIR.PRAGMA.BLOCK_LOOP is handled by loopopt framework.
; Note that 's' is a local variable which is registerized by SROA. This test is
; only checking that information from the blocking pragma is successfully
; captured in HLLoop structure by the framework.


; CHECK: BEGIN REGION

; region entry/exit intrinsics are eliminated by parser
; CHECK-NOT: llvm.directive.region.entry

; CHECK: + Blocking levels and factors:(1,16) (2,32)
; CHECK: + Blocking privates:&((%s)[0])
; CHECK: + DO i64 i1

; CHECK-NOT: llvm.directive.region.exit

; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [256 x [256 x float]] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [256 x [256 x float]] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [256 x [256 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() {
entry:
  %s = alloca float, align 4
  %t1 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.PRIVATE"(ptr %s), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 16), "QUAL.PRAGMA.LEVEL"(i32 2), "QUAL.PRAGMA.FACTOR"(i32 32) ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc21, %entry
  %indvars.iv44 = phi i64 [ 0, %entry ], [ %indvars.iv.next45, %for.inc21 ]
  br label %for.body3

for.body3:                                        ; preds = %for.end, %for.cond1.preheader
  %indvars.iv41 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next42, %for.end ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3 ], [ %indvars.iv.next, %for.body6 ]
  %add37 = phi float [ 0.000000e+00, %for.body3 ], [ %add, %for.body6 ]
  %arrayidx8 = getelementptr inbounds [256 x [256 x float]], ptr @B, i64 0, i64 %indvars.iv44, i64 %indvars.iv
  %t2 = load float, ptr %arrayidx8, align 4
  %arrayidx12 = getelementptr inbounds [256 x [256 x float]], ptr @C, i64 0, i64 %indvars.iv, i64 %indvars.iv41
  %t3 = load float, ptr %arrayidx12, align 4
  %mul = fmul float %t2, %t3
  %add = fadd float %add37, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond, label %for.end, label %for.body6

for.end:                                          ; preds = %for.body6
  %add.lcssa = phi float [ %add, %for.body6 ]
  %arrayidx16 = getelementptr inbounds [256 x [256 x float]], ptr @A, i64 0, i64 %indvars.iv44, i64 %indvars.iv41
  %t4 = load float, ptr %arrayidx16, align 4
  %add17 = fadd float %add.lcssa, %t4
  store float %add17, ptr %arrayidx16, align 4
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43 = icmp eq i64 %indvars.iv.next42, 256
  br i1 %exitcond43, label %for.inc21, label %for.body3

for.inc21:                                        ; preds = %for.end
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next45, 256
  br i1 %exitcond46, label %for.end23, label %for.cond1.preheader

for.end23:                                        ; preds = %for.inc21
  call void @llvm.directive.region.exit(token %t1) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)


