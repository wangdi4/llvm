; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that parser restructures the store ref A[1][-1 * i1 -1] into A[0][-1 * i1 + 8].

; CHECK: + DO i1 = 0, 5, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][-1 * i1 + 7];
; CHECK: |   (@A)[0][-1 * i1 + 8] = %0;
; CHECK: + END LOOP


; ModuleID = 'array-dd.c'
source_filename = "array-dd.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@A = common global [9 x i16] zeroinitializer, align 2

; Function Attrs: norecurse nounwind
define void @foo() local_unnamed_addr {
entry:
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %p.03.i = phi ptr [ getelementptr inbounds ([9 x i16], ptr @A, i32 1, i32 0), %entry ], [ %incdec.ptr2.i, %for.body.i ]
  %i.02.i = phi i32 [ 2, %entry ], [ %inc.i, %for.body.i ]
  %x.addr.01.i = phi ptr [ getelementptr inbounds ([9 x i16], ptr @A, i32 0, i32 8), %entry ], [ %incdec.ptr.i, %for.body.i ]
  %incdec.ptr.i = getelementptr inbounds i16, ptr %x.addr.01.i, i32 -1
  %0 = load i16, ptr %incdec.ptr.i, align 2
  %incdec.ptr2.i = getelementptr inbounds i16, ptr %p.03.i, i32 -1
  store i16 %0, ptr %incdec.ptr2.i, align 2
  %inc.i = add nuw nsw i32 %i.02.i, 1
  %exitcond.i = icmp eq i32 %inc.i, 8
  br i1 %exitcond.i, label %eshdn6.exit, label %for.body.i

eshdn6.exit:                                      ; preds = %for.body.i
  store i16 0, ptr getelementptr inbounds ([9 x i16], ptr @A, i32 0, i32 2), align 2
  ret void
}

