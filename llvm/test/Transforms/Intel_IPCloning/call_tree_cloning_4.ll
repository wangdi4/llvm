; Checks that the Call Tree Cloning transformation clones expected functions,

; RUN: opt < %s -passes='module(call-tree-clone)'  -S | FileCheck %s

; Expect the call on foo(4) is turned into a specialized call as foo.1(4)
; CHECK:"foo.1|4"

; Expect function specialization happened on foo(4)
; CHECK: %1 = shl nsw i32 4, 1

source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [11 x i8] c"loop1: %d\0A\00", align 1
@GUB = internal global i32 2, align 4

; Function Attrs: noinline nounwind uwtable
define internal fastcc void @foo(i32) unnamed_addr #0 {
  %2 = shl nsw i32 %0, 1
  %3 = icmp sgt i32 %0, 0
  br i1 %3, label %4, label %9

; <label>:4:                                      ; preds = %4, %1
  %5 = phi i32 [ %7, %4 ], [ 0, %1 ]
  %6 = tail call i32 (i8*, i32, ...) bitcast (i32 (...)* @printf1 to i32 (i8*, i32, ...)*)(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 %5) #3
  %7 = add nuw nsw i32 %5, 1
  %8 = icmp slt i32 %7, %2
  br i1 %8, label %4, label %9

; <label>:9:                                      ; preds = %4, %1
  ret void
}

declare dso_local i32 @printf1(...) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
  tail call fastcc void @foo.1(i32 4)
  %1 = load volatile i32, i32* @GUB, align 4, !tbaa !3
  tail call fastcc void @foo(i32 %1)
  ret i32 0
}

; Function Attrs: noinline nounwind uwtable
define internal fastcc void @foo.1(i32) unnamed_addr #0 {
  %2 = shl nsw i32 %0, 1
  %3 = icmp sgt i32 %0, 0
  br i1 %3, label %4, label %9

; <label>:4:                                      ; preds = %4, %1
  %5 = phi i32 [ %7, %4 ], [ 0, %1 ]
  %6 = tail call i32 (i8*, i32, ...) bitcast (i32 (...)* @printf1 to i32 (i8*, i32, ...)*)(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 %5) #3
  %7 = add nuw nsw i32 %5, 1
  %8 = icmp slt i32 %7, %2
  br i1 %8, label %4, label %9

; <label>:9:                                      ; preds = %4, %1
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 8917aed201bae2133195342c218e4ac7e137d59c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 3e48843cc0723500f45157895f5d26cb27d81c91)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

; Original source a.c is below.
;volatile int GUB = 2;
;
;void __attribute__((noinline)) foo(int UB) {
;  int i;
;  for (i = 0; i < 2 * UB; ++i) {
;    printf1("loop1: %d\n", i);
;  }
;}
;
;int main(void) {
;  foo(4);
;  foo(GUB);
;  return 0;
;}
;
