; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that the select instruction with IV operand (%0) is reverse engineered from min() operation into a blob.

; CHECK: + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK: |   %4 = -256;
; CHECK: |   if (i1 + -512 >= -256)
; CHECK: |   {
; CHECK: |      %3 = (i1 + -512 > 255) ? 255 : i1 + -512;
; CHECK: |      %4 = %3;
; CHECK: |   }
; CHECK: |   (@iclip)[0][i1] = %4;
; CHECK: + END LOOP


@iclip = internal global [1024 x i16] zeroinitializer, align 2

define void @foo() {
entry:
  br label %for.body

for.body:                                    ; preds = %entry, %for.inc
  %0 = phi i32 [ -512, %entry ], [ %7, %for.inc ]
  %1 = icmp slt i32 %0, -256
  br i1 %1, label %for.inc, label %if.then

if.then:                                    ; preds = %for.body
  %2 = icmp sgt i32 %0, 255
  %3 = select i1 %2, i32 255, i32 %0
  br label %for.inc

for.inc:                                    ; preds = %for.body, %if.then
  %4 = phi i32 [ -256, %for.body ], [ %3, %if.then ]
  %5 = trunc i32 %4 to i16
  %6 = getelementptr inbounds i16, ptr getelementptr inbounds ([1024 x i16], ptr @iclip, i32 0, i32 512), i32 %0
  store i16 %5, ptr %6, align 2
  %7 = add nsw i32 %0, 1
  %8 = icmp eq i32 %7, 512
  br i1 %8, label %exit, label %for.body

exit:
  ret void
}


