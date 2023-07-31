; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-dead-store-elimination,print<hir>" -hir-details -disable-output 2>&1 < %s | FileCheck %s

; Verify that we eliminate the store to (%A)[0] by replacing it and the
; intermediate load with temp.

; Store becomes a copy and but the copy's rval isn't propagated to the use in
; the load instruction by the copy propagation utility because rval canon expr
; sext.i16.i32(i1 + %t) src and dst type do not match and we cannot create a
; blob out of it.

; Note that it is still possible to propagate the rval in the use without
; forming the blob in this case as the use is in a self-blob but this is left
; as a TODO for now. 

; Print Before-

; CHECK: + DO i16 i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%A)[0] = i1 + %t;
; CHECK: |       <RVAL-REG> LINEAR sext.i16.i32(i1 + %t)
; CHECK: |
; CHECK: |   %ld = (%A)[0];
; CHECK: |   %t.02 = %t.02  +  %ld;
; CHECK: |   (%A)[0] = 5;
; CHECK: + END LOOP

; Print After-

; CHECK:      modified
; CHECK:     + DO i16 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   %temp = i1 + %t;
; CHECK-NOT: |   %ld = %temp;
; CHECK:     |   %t.02 = %t.02  +  %temp;
; CHECK:     |   (%A)[0] = 5;
; CHECK:     + END LOOP


define dso_local i32 @foo(ptr nocapture %A, i16 %t) {
entry:
  %t1 = sext i16 %t to i32
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %t.02 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %i.01 = phi i16 [ 0, %entry ], [ %inc, %for.body ]
  %add16 = add i16 %i.01, %t
  %sxt = sext i16 %add16 to i32
  store i32 %sxt, ptr %A, align 4
  %ld = load i32, ptr %A, align 4
  %add = add nuw nsw i32 %t.02, %ld
  store i32 5, ptr %A, align 4
  %inc = add nuw nsw i16 %i.01, 1
  %cmp = icmp ult i16 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  %t.0.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %t.0.lcssa
}

