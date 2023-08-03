; RUN: opt < %s -passes='module(call-tree-clone)' -call-tree-clone-mv-bypass-coll-for-littest=1 -S | FileCheck %s

; Check that the compilation does not segfault because @foo is selected as
; a possible call-tree cloning candidate, but no actual clones are generated.

; CHECK: define internal void @foo(
; CHECK: define internal void @goo(
; CHECK: define internal void @bar(
; CHECK: define internal void @gee(
; CHECK: define dso_local i32 @main(
; CHECK: define internal void @"goo|13.7.13.2"(
; CHECK: define internal void @"bar|7._.6"(
; CHECK: define internal void @"gee|2.4._"(
; CHECK: define internal void @"bar|_._._"(
; CHECK: define internal void @"bar|7._._"(
; CHECK: define internal void @"bar|_._.6"(
; CHECK: define internal void @"goo|_._._._"(
; CHECK: define internal void @"goo|13._._._"(
; CHECK: define internal void @"goo|_.7._._"(
; CHECK: define internal void @"goo|_._.13._"(
; CHECK: define internal void @"goo|_._._.2"(

@.str = private unnamed_addr constant [27 x i8] c"      foo(%d, %d, %d, %d)\0A\00", align 1
@.str.1 = private unnamed_addr constant [18 x i8] c"        loop1:%d\0A\00", align 1
@.str.2 = private unnamed_addr constant [18 x i8] c"        loop2:%d\0A\00", align 1
@.str.3 = private unnamed_addr constant [21 x i8] c"    bar(%d, %d, %d)\0A\00", align 1
@.str.4 = private unnamed_addr constant [19 x i8] c"  gee(%d, %d, %d)\0A\00", align 1
@glob = common dso_local global i32 0, align 4
@.str.5 = private unnamed_addr constant [6 x i8] c"main\0A\00", align 1
@GV = internal global [15 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define internal void @foo(i32 %arg, i32 %arg1, i32 %arg2, i32 %arg3) #0 {
bb:
  tail call void @printf4(ptr @.str, i32 %arg, i32 %arg1, i32 %arg2, i32 %arg3) #3
  %i = add i32 %arg1, %arg
  %i4 = icmp sgt i32 %i, 0
  br i1 %i4, label %bb12, label %bb5

bb5:                                              ; preds = %bb12, %bb
  %i6 = add nsw i32 %arg2, %arg1
  %i7 = add nsw i32 %i6, %arg3
  %i8 = icmp sgt i32 %i7, 0
  br i1 %i8, label %bb9, label %bb20

bb9:                                              ; preds = %bb5
  %i10 = add i32 %arg3, %arg2
  %i11 = add i32 %i10, %arg1
  br label %bb16

bb12:                                             ; preds = %bb12, %bb
  %i13 = phi i32 [ %i14, %bb12 ], [ 0, %bb ]
  tail call void @printf1(ptr @.str.1, i32 %i13) #3
  %i14 = add nuw nsw i32 %i13, 1
  %i15 = icmp eq i32 %i14, %i
  br i1 %i15, label %bb5, label %bb12

bb16:                                             ; preds = %bb16, %bb9
  %i17 = phi i32 [ %i18, %bb16 ], [ 0, %bb9 ]
  tail call void @printf1(ptr @.str.2, i32 %i17) #3
  %i18 = add nuw nsw i32 %i17, 1
  %i19 = icmp eq i32 %i18, %i11
  br i1 %i19, label %bb20, label %bb16

bb20:                                             ; preds = %bb16, %bb5
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @goo(i32 %arg, i32 %arg1, i32 %arg2, i32 %arg3) #0 {
bb:
  tail call void @printf4(ptr @.str, i32 %arg, i32 %arg1, i32 %arg2, i32 %arg3) #3
  %i = add i32 %arg1, %arg
  %i4 = icmp sgt i32 %i, 0
  br i1 %i4, label %bb12, label %bb5

bb5:                                              ; preds = %bb12, %bb
  %i6 = add nsw i32 %arg2, %arg1
  %i7 = add nsw i32 %i6, %arg3
  %i8 = icmp sgt i32 %i7, 0
  br i1 %i8, label %bb9, label %bb20

bb9:                                              ; preds = %bb5
  %i10 = add i32 %arg3, %arg2
  %i11 = add i32 %i10, %arg1
  br label %bb16

bb12:                                             ; preds = %bb12, %bb
  %i13 = phi i32 [ %i14, %bb12 ], [ 0, %bb ]
  tail call void @printf1(ptr @.str.1, i32 %i13) #3
  %i14 = add nuw nsw i32 %i13, 1
  %i15 = icmp eq i32 %i14, %i
  br i1 %i15, label %bb5, label %bb12

bb16:                                             ; preds = %bb16, %bb9
  %i17 = phi i32 [ %i18, %bb16 ], [ 0, %bb9 ]
  tail call void @printf1(ptr @.str.2, i32 %i17) #3
  %i18 = add nuw nsw i32 %i17, 1
  %i19 = icmp eq i32 %i18, %i11
  br i1 %i19, label %bb20, label %bb16

bb20:                                             ; preds = %bb16, %bb5
  ret void
}

declare dso_local void @printf4(ptr, i32, i32, i32, i32) local_unnamed_addr #1

declare dso_local void @printf1(ptr, i32) local_unnamed_addr #1

; Function Attrs: noinline nounwind uwtable
define internal void @bar(i32 %arg, i32 %arg1, i32 %arg2) #0 {
bb:
  tail call void @printf3(ptr @.str.3, i32 %arg, i32 %arg1, i32 %arg2) #3
  %i = add nsw i32 %arg1, 1
  %i3 = add nsw i32 %arg2, %arg
  %t1 = getelementptr [15 x i32], ptr @GV, i64 0, i32 %i
  %t0 = load i32, ptr %t1, align 8
  tail call void @foo(i32 %t0, i32 %t0, i32 %t0, i32 %t0)
  %i4 = add nsw i32 %arg1, %arg
  %i5 = add nsw i32 %arg2, %arg1
  tail call void @foo(i32 %t0, i32 %t0, i32 %t0, i32 %t0)
  tail call void @goo(i32 %i3, i32 %arg, i32 %i3, i32 2)
  ret void
}

declare dso_local void @printf3(ptr, i32, i32, i32) local_unnamed_addr #1

; Function Attrs: noinline nounwind uwtable
define internal void @gee(i32 %arg, i32 %arg1, i32 %arg2) #0 {
bb:
  tail call void @printf3(ptr @.str.4, i32 %arg, i32 %arg1, i32 %arg2) #3
  %i = load i32, ptr @glob, align 4, !tbaa !1
  %i3 = add nsw i32 %arg1, 1
  %i4 = add nsw i32 %arg2, 1
  tail call void @bar(i32 %i, i32 %i3, i32 %i4)
  %i5 = add nsw i32 %arg1, %arg
  tail call void @bar(i32 7, i32 8, i32 %i5)
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 %arg, ptr nocapture readnone %arg1) #2 {
bb:
  store i32 %arg, ptr @glob, align 4, !tbaa !1
  tail call void @printf0(ptr @.str.5) #3
  tail call void @gee(i32 2, i32 4, i32 5)
  ret i32 0
}

declare dso_local void @printf0(ptr) local_unnamed_addr #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
