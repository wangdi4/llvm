; RUN: opt < %s -passes='module(call-tree-clone)' -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -call-tree-clone-do-mv=false -call-tree-clone-seed=a:0 -call-tree-clone-seed=b:0,1 -S | FileCheck %s
; RUN: opt < %s -call-tree-clone -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2  -call-tree-clone-do-mv=false -call-tree-clone-seed=a:0 -call-tree-clone-seed=b:0,1 -S | FileCheck %s

; CHECK:@"a|3"()
; CHECK:@"b|19.-15"()
; CHECK:@"x|3.5"()
; CHECK:@"y|17.2"()
; CHECK:@"y|_.-2"(i32 %0)
; CHECK:@"a|-1"()

; This test checks is "call-tree-clone" optimization works correctly on the
; following call graph below:
; main(1) main(2)
;    |   /
;    x  /
;    | /
;    y
;   / \
;  a   b
;
; It is specifically checked that:
; 1) expected clones of y are created, when y is called from multiple
;    levels in the call graph
; 2) negative integer constants are equally accepted for cloning
; 3) two types of clones for y are generated - y|_.x and y|x.x
; 4) -call-tree-clone-seed option works as expected


; *** IR Dump After IP Cloning ***; ModuleID = 'ld-temp.o'

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal i32 @a(i32 returned) unnamed_addr #0 {
  ret i32 %0
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal i32 @b(i32, i32) unnamed_addr #0 {
  %3 = add nsw i32 %1, %0
  ret i32 %3
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal i32 @y(i32, i32) unnamed_addr #0 {
  %3 = add nsw i32 %1, 1
  %4 = call i32 @a(i32 %3)
  %5 = sub nsw i32 %1, %0
  %6 = add nsw i32 %1, %0
  %7 = call i32 @b(i32 %6, i32 %5)
  %8 = add nsw i32 %7, %4
  ret i32 %8
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define internal i32 @x(i32, i32) unnamed_addr #0 {
  %3 = sdiv i32 %1, 2
  %4 = add nsw i32 %1, %0
  %5 = shl nsw i32 %4, 1
  %6 = or i32 %5, 1
  %7 = call i32 @y(i32 %6, i32 %3)
  ret i32 %7
}

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @main(i32, i8** nocapture readnone) local_unnamed_addr #1 {
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

; Original test case:
;
; #define NOI __declspec(noinline)
;
; NOI int a(int p1) {
;   return p1;
; }
;
; NOI int b(int p1, int p2) {
;   return p1+p2;
; }
;
; // y.17.2
; // y._.-2
; NOI int y(int p1, int p2) {
;   // 1) a(2+1) + b(2+17,2-17) =>
;   //    a.3
;   //    b.19.-15
;   // 2) a(-2+1) + b(-2+p1, -2-p1) =>
;   //    a.-1
;   return a(p2+1) + b(p2+p1,p2-p1);
; }
;
; // x.3.5
; NOI int x(int p1, int p2) {
;   // y((3+5)*2+1, 5/2) =>
;   // y.17.2
;   return y((p1+p2)*2 + 1, p2/2);
; }
;
; int main(int argc, const char** argv) {
;   // x.3.5
;   // y._.-2
;   return x(argc,argc+1) + x(3,5) + y(argc,-2);
; }
