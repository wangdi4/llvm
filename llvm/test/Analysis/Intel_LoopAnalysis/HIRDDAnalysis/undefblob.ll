;  There was a compfail to get BlobValue.
;  It's sufficient to check if HIR is produced at the end
; RUN: opt -scoped-noalias-aa -hir-ssa-deconstruction -hir-runtime-dd -hir-vec-dir-insert -VPlanDriverHIR  -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s
;
; CHECK: After
; CHECK: DO i1
; CHECK: END LOOP
; CHECK: DO i1
; CHECK: END LOOP

; ModuleID = 'module.ll'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #0

declare i32 @undef(i32**) local_unnamed_addr #1

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #0

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32, i8** nocapture readnone) local_unnamed_addr #3 {
  %3 = alloca i32*, align 8
  %4 = bitcast i32** %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %4) #4
  store i32* null, i32** %3, align 8, !tbaa !2
  %5 = call i32 @undef(i32** nonnull %3) #4
  %6 = bitcast i32** %3 to i8**
  %7 = load i8*, i8** %6, align 8, !tbaa !6
  %8 = load i8, i8* %7, align 1, !tbaa !8
  %9 = getelementptr inbounds i8, i8* %7, i64 1
  store i8* %9, i8** %6, align 8, !tbaa !6
  %10 = call i8* @malloc(i64 0) #4
  %11 = bitcast i8* %10 to i32**
  %12 = zext i8 %8 to i32
  %13 = icmp sgt i32 %12, 0
  br i1 %13, label %14, label %24

; <label>:14:                                     ; preds = %2
  br label %15

; <label>:15:                                     ; preds = %15, %14
  %16 = phi i8* [ %9, %14 ], [ %19, %15 ]
  %17 = phi i32 [ 0, %14 ], [ %20, %15 ]
  %18 = load i8, i8* %16, align 1, !tbaa !8
  store i8 %18, i8* undef, align 1, !tbaa !8
  %19 = getelementptr inbounds i8, i8* %16, i64 1
  store i8* %19, i8** %6, align 8, !tbaa !6
  %20 = add nuw nsw i32 %17, 1
  %21 = icmp ne i32 %20, %12
  br i1 %21, label %15, label %22

; <label>:22:                                     ; preds = %15
  %23 = bitcast i8* %10 to i8**
  br label %24

; <label>:24:                                     ; preds = %22, %2
  %25 = call i32 @undef(i32** %11) #4
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %4) #4
  ret i32 0
}

attributes #0 = { argmemonly nounwind }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.ident = !{!0, !0}
!llvm.module.flags = !{!1}

!0 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 60aeb1dd4bb3bd88a3376e685f3db9c1e0207916)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSPh", !4, i64 0}
!8 = !{!4, !4, i64 0}
