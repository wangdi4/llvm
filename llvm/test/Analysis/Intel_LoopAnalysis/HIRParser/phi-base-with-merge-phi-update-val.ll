; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 -disable-output | FileCheck %s

; Check parsing output for the loop verifying that store with linear phi base is parsed correctly.
; CHECK: DO i1 = 0, 999
; CHECK-NEXT: (%p)[i1] = i1
; CHECK-NEXT: END LOOP


; Function Attrs: nounwind uwtable
define i32 @sub(ptr nocapture %p, i1 %cond) {
entry:
  br label %for.body

for.body:                                         ; preds = %latch, %entry
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %latch ]
  %p.addr.05 = phi ptr [ %p, %entry ], [ %p.inc.merge, %latch ]
  store i32 %i.06, ptr %p.addr.05, align 4
  br i1 %cond, label %then, label %else

then:
  %p.inc1 = getelementptr inbounds i32, ptr %p.addr.05, i64 1
  br label %latch

else:
  %p.inc2 = getelementptr inbounds i32, ptr %p.addr.05, i64 1
  br label %latch

latch:
  %p.inc.merge = phi ptr [ %p.inc1, %then ], [  %p.inc2, %else ]
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

