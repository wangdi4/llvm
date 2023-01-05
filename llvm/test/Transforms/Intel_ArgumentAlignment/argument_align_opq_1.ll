; RUN: opt < %s -opaque-pointers -passes='module(intel-argument-alignment)' -whole-program-assume -S 2>&1

; This test checks that the argument alignment runs correctly. It is the same
; test case as argument_align_1.ll, but it checks that the argument alignment
; works for opaque pointers.

; Check that the constants were removed
; CHECK: void @foo
; CHECK-NEXT: entry:
; CHECK-NEXT: br label %if_bb
; CHECK-EMPTY:
; CHECK-NEXT: if_bb:
; CHECK-NEXT:  %2 = phi ptr [ %0, %entry ], [ %4, %if_bb ]
; CHECK-NEXT:  %3 = sub i64 0, %1
; CHECK-NEXT:  %4 = getelementptr inbounds i8, ptr %2, i64 %3
; CHECK-NEXT:  %5 = lshr i64 %3, 3
; CHECK-NEXT:  %6 = icmp ult i64 %5, 7
; CHECK-NEXT:  br i1 %6, label %if_bb, label %end
; CHECK-EMPTY:
; CHECK-NEXT: end:
; CHECK-NEXT:  ret void
; CHECK-NEXT: }

; ModuleID = 'argument_align_1.ll'
source_filename = "argument_align_1.ll"

declare noalias ptr @calloc(i64, i64) #0

define internal fastcc void @foo(ptr %0, i64 %1) {
entry:
  %2 = ptrtoint ptr %0 to i64
  %3 = and i64 %2, 7
  %4 = icmp eq i64 %3, 0
  br i1 %4, label %if_bb, label %end

if_bb:                                            ; preds = %else_bb, %entry
  %5 = phi ptr [ %0, %entry ], [ %8, %else_bb ]
  %6 = phi i1 [ %4, %entry ], [ %11, %else_bb ]
  br i1 %6, label %else_bb, label %end

else_bb:                                          ; preds = %if_bb
  %7 = sub i64 0, %1
  %8 = getelementptr inbounds i8, ptr %5, i64 %7
  %9 = ptrtoint ptr %8 to i64
  %10 = and i64 %9, 7
  %11 = icmp eq i64 %10, 0
  %12 = lshr i64 %7, 3
  %13 = icmp ult i64 %12, 7
  br i1 %13, label %if_bb, label %end

end:                                              ; preds = %else_bb, %if_bb, %entry
  ret void
}

define void @bar(i64 %0, i64 %1) {
entry:
  %2 = tail call noalias ptr @calloc(i64 %0, i64 8)
  tail call fastcc void @foo(ptr %2, i64 %1)
  ret void
}

attributes #0 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
