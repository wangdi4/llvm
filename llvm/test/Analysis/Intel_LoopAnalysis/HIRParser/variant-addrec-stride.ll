; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; The stride %t696 of phi %t687 was deduced to be '-4' due to control flow
; dependence of the select on the loop exit condition (%t694).
; Parser tried to create an AddRec using a stride of %t696 and hit an assert.
; We now parse the phi conservatively as a blob rather than an AddRec.

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %t690 = (%t687)[0];
; CHECK: |   (%t20)[0] = %t690;
; CHECK: |   %t696 = (-4 * i1 + 4 == 0) ? 4 * i1 + -4 : -4;
; CHECK: |   %t687 = &((%t687)[%t696]);
; CHECK: + END LOOP

define void @foo(ptr %t684, ptr %t20) {
entry:
  br label %loop

loop:                                              ; preds = %loop, %entry
  %t686 = phi i32 [ 8, %entry ], [ %t693, %loop ]
  %t687 = phi ptr [ %t684, %entry ], [ %t697, %loop ]
  %t688 = phi i32 [ 8, %entry ], [ %t692, %loop ]
  %t689 = bitcast ptr %t687 to ptr
  %t690 = load i32, ptr %t689, align 1
  store i32 %t690, ptr %t20, align 4
  %t692 = add nuw nsw i32 %t688, 1
  %t693 = add nsw i32 %t686, -4
  %t694 = icmp eq i32 %t693, 0
  %t695 = sub nsw i32 4, %t686
  %t696 = select i1 %t694, i32 %t695, i32 -4
  %t697 = getelementptr inbounds i8, ptr %t687, i32 %t696
  br i1 %t694, label %exit, label %loop

exit:
  ret void
}
