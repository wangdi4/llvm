; RUN: opt < %s -opaque-pointers -passes='module(call-tree-clone)' -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -call-tree-clone-do-mv=false -call-tree-clone-seed=a:0 -call-tree-clone-seed=b:0,1 -call-tree-clone-max-depth=2 -S | FileCheck %s

; CHECK:source_filename
; CHECK-NOT:@"a|3"()
; CHECK-NOT:@"b|19.-15"()
; CHECK-NOT:@"x|3.5"()
; CHECK-NOT:@"y|17.2"()
; CHECK:@"y|_.-2"(i32 %0)
; CHECK:@"a|-1"()

; This test checks is "call-tree-clone" optimization works correctly on the
; following call graph below in presence of clone depth restriction to 2 levels:
; main(1) main(2)
;    |   /
;    x  /
;    | /
;    y
;   / \
;  a   b
;
; It is specifically checked that only two clones are created, and that other
; clones,  which would have been created if not for the depth restriction, are
; not created.

; This is the same test case as call_tree_cloning_3.ll, but it checks for
; opaque pointers.

; ModuleID = 'call_tree_cloning_3-opaque-ptr.ll'
source_filename = "call_tree_cloning_3-opaque-ptr.ll"

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal i32 @a(i32 returned %0) unnamed_addr #0 {
  ret i32 %0
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal i32 @b(i32 %0, i32 %1) unnamed_addr #0 {
  %3 = add nsw i32 %1, %0
  ret i32 %3
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal i32 @y(i32 %0, i32 %1) unnamed_addr #0 {
  %3 = add nsw i32 %1, 1
  %4 = call i32 @a(i32 %3)
  %5 = sub nsw i32 %1, %0
  %6 = add nsw i32 %1, %0
  %7 = call i32 @b(i32 %6, i32 %5)
  %8 = add nsw i32 %7, %4
  ret i32 %8
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal i32 @x(i32 %0, i32 %1) unnamed_addr #0 {
  %3 = sdiv i32 %1, 2
  %4 = add nsw i32 %1, %0
  %5 = shl nsw i32 %4, 1
  %6 = or i32 %5, 1
  %7 = call i32 @y(i32 %6, i32 %3)
  ret i32 %7
}

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @main(i32 %0, ptr nocapture readnone %1) local_unnamed_addr #1 {
  %3 = add nsw i32 %0, 1
  %4 = call i32 @x(i32 %0, i32 %3)
  %5 = call i32 @x(i32 3, i32 5)
  %6 = add nsw i32 %5, %4
  %7 = call i32 @y(i32 %0, i32 -2)
  %8 = add nsw i32 %6, %7
  ret i32 %8
}

attributes #0 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
