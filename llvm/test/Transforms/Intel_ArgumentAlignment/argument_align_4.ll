; This test checks that the argument alignment runs correctly.
; This test is same as argument_align_1.ll except return address
; of calloc is passed to "foo" as first argument indirectly.

; RUN: opt < %s -intel-argument-alignment -whole-program-assume -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(intel-argument-alignment)' -whole-program-assume -S 2>&1

; Check that the constants were removed
; CHECK: void @foo
; CHECK-NEXT: entry:
; CHECK-NEXT: br label %if_bb
; CHECK-EMPTY:
; CHECK-NEXT: if_bb:
; CHECK-NEXT:  %2 = phi i8* [ %0, %entry ], [ %4, %if_bb ]
; CHECK-NEXT:  %3 = sub i64 0, %1
; CHECK-NEXT:  %4 = getelementptr inbounds i8, i8* %2, i64 %3
; CHECK-NEXT:  %5 = lshr i64 %3, 3
; CHECK-NEXT:  %6 = icmp ult i64 %5, 7
; CHECK-NEXT:  br i1 %6, label %if_bb, label %end
; CHECK-EMPTY:
; CHECK-NEXT: end:
; CHECK-NEXT:  ret void
; CHECK-NEXT: }

declare noalias i8* @calloc(i64, i64) #0
declare noalias i8* @malloc(i64 noundef) #1

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
  %call0 = tail call noalias i8* @calloc(i64 %0, i64 8)
  %call1 = tail call noalias i8* @malloc(i64 8)
  %bc1 = bitcast i8* %call1 to i64**
  %bc2 = bitcast i8* %call0 to i64*
  store i64* %bc2, i64** %bc1, align 8
  %val = load i64*, i64** %bc1, align 8
  %bc3 = bitcast i64* %val to i8*
  tail call fastcc void @foo(i8* %bc3, i64 %1)
  ret void
}

attributes #0 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
