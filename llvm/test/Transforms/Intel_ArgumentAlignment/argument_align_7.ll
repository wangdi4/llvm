; REQUIRES: assert
; This test checks that the argument alignment identifies that there is no
; no whole program safe and prints the proper debug messages.

; This test is the same as argument_align_3.ll.

; RUN: opt < %s -intel-argument-alignment -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=intel-argument-alignment -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes=intel-argument-alignment -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=intel-argument-alignment  -S 2>&1

; CHECK: Candidates for argument alignment: 0
; CHECK-NEXT: Reason: Whole program not safe

declare noalias i8* @calloc(i64, i64)

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
