; REQUIRES: asserts
; This test checks that a function's argument is always aligned by a specific
; constant value and uses that to simplify tests in that function based on the
; alignment.

; This test is the same as argument_alignment_1.ll, but checks that the debug
; messages are printed correctly.

; RUN: opt < %s -intel-argument-alignment -whole-program-assume -debug-only=intel-argument-alignment -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=intel-argument-alignment -whole-program-assume -debug-only=intel-argument-alignment  -disable-output  2>&1

; CHECK: Candidates for argument alignment: 1
; CHECK-NEXT: Aligning by: 8
; CHECK-NEXT:  Function: foo
; CHECK-NEXT:    Instructions:
; CHECK-NEXT:        %9 = ptrtoint i8* %8 to i64
; CHECK-NEXT:        %2 = ptrtoint i8* %0 to i64
; CHECK-NEXT:    Argument: i8* %0

declare noalias i8* @calloc(i64, i64) #0

define internal fastcc void @foo(i8*, i64) {
entry:
  %2 = ptrtoint i8* %0 to i64
  %3 = and i64 %2, 7
  %4 = icmp eq i64 %3, 0
  br i1 %4, label %if_bb, label %end

if_bb:
  %5 = phi i8* [ %0, %entry ], [ %8, %else_bb ]
  %6 = phi i1  [ %4, %entry ], [ %11, %else_bb ]
  br i1 %6, label %else_bb, label %end

else_bb:
  %7 = sub i64 0, %1
  %8 = getelementptr inbounds i8, i8* %5, i64 %7
  %9 = ptrtoint i8* %8 to i64
  %10 = and i64 %9, 7
  %11 = icmp eq i64 %10, 0
  %12 = lshr i64 %7, 3
  %13 = icmp ult i64 %12, 7
  br i1 %13, label %if_bb, label %end

end:
  ret void
}

define void @bar(i64, i64) {
entry:
  %2 = tail call noalias i8* @calloc(i64 %0, i64 8)
  tail call fastcc void @foo(i8* %2, i64 %1)
  ret void
}

attributes #0 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
