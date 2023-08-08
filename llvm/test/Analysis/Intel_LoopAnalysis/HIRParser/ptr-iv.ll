; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that the pointer IV is handled correctly.
; CHECK: DO i1 = 0, (-1 * ptrtoint.ptr.i64(%p) + ptrtoint.ptr.i64(%q) + -4)/u4, 1
; CHECK-NEXT: (%p)[2 * i1] = i1;
; CHECK-NEXT:  END LOOP

; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output -hir-details 2>&1 | FileCheck %s -check-prefix=DETAIL

; Verify that we are able to detect no signed wrap for pointer IV loops.
; DETAIL: HasSignedIV: Yes

; ModuleID = 'ptr-iv.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %p, ptr readnone %q) {
entry:
  %cmp.6 = icmp eq ptr %p, %q
  br i1 %cmp.6, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %p.addr.07 = phi ptr [ %incdec.ptr, %for.body ], [ %p, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %p.addr.07, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4
  %incdec.ptr = getelementptr inbounds i32, ptr %p.addr.07, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq ptr %incdec.ptr, %q
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
