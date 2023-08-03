; Test for generating mkl call for matrix multiplication with float data type and zero initialization

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-dead-store-elimination,hir-loop-interchange,hir-generate-mkl-call,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;

; Before HIR Generate MKL Call-
; + DO i1 = 0, 499, 1   <DO_LOOP>
; |   + DO i2 = 0, 499, 1   <DO_LOOP>
; |   |   %add44 = 0.000000e+00;
; |   |
; |   |   + DO i3 = 0, 499, 1   <DO_LOOP>
; |   |   |   %mul = (@a)[0][i1][i3]  *  (@b)[0][i3][i2];
; |   |   |   %add44 = %add44  +  %mul;
; |   |   + END LOOP
; |   |
; |   |   (@c)[0][i1][i2] = %add44;
; |   + END LOOP
; + END LOOP

; After HIR Generate MKL Call-
; CHECK: BEGIN REGION { modified }
; CHECK: 0 = &((i8*)(@c)[0][0][0]);
; CHECK: 1 = 4;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 500;
; CHECK: 7 = 4;
; CHECK: 8 = 1;
; CHECK: 9 = 500;
; CHECK: 10 = 2000;
; CHECK: 11 = 1;
; CHECK: 0 = &((i8*)(@b)[0][0][0]);
; CHECK: 1 = 4;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 500;
; CHECK: 7 = 4;
; CHECK: 8 = 1;
; CHECK: 9 = 500;
; CHECK: 10 = 2000;
; CHECK: 11 = 1;
; CHECK: 0 = &((i8*)(@a)[0][0][0]);
; CHECK: 1 = 4;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 500;
; CHECK: 7 = 4;
; CHECK: 8 = 1;
; CHECK: 9 = 500;
; CHECK: 10 = 2000;
; CHECK: 11 = 1;
; CHECK: @matmul_mkl_f32_
; CHECK: 9,  0
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'float-matmul-with-reset.cpp'
source_filename = "float-matmul-with-reset.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }

@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@a = dso_local local_unnamed_addr global [500 x [500 x float]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [500 x [500 x float]] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [500 x [500 x float]] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_float_matmul_with_reset.cpp, ptr null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #2

; Function Attrs: norecurse nounwind uwtable
define dso_local void @_Z8multiplyv() local_unnamed_addr #3 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.cond.cleanup9
  %indvar.next = add nuw nsw i64 %indvar, 1
  %exitcond51 = icmp eq i64 %indvar.next, 500
  br i1 %exitcond51, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond.cleanup9, %for.cond1.preheader
  %indvars.iv47 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next48, %for.cond.cleanup9 ]
  %arrayidx6 = getelementptr inbounds [500 x [500 x float]], ptr @c, i64 0, i64 %indvar, i64 %indvars.iv47, !intel-tbaa !2
  store float 0.000000e+00, ptr %arrayidx6, align 4, !tbaa !2
  br label %for.body10

for.cond.cleanup9:                                ; preds = %for.body10
  %add.lcssa = phi float [ %add, %for.body10 ]
  store float %add.lcssa, ptr %arrayidx6, align 4, !tbaa !2
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 500
  br i1 %exitcond49, label %for.cond.cleanup3, label %for.body4

for.body10:                                       ; preds = %for.body10, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body4 ], [ %indvars.iv.next, %for.body10 ]
  %add44 = phi float [ 0.000000e+00, %for.body4 ], [ %add, %for.body10 ]
  %arrayidx14 = getelementptr inbounds [500 x [500 x float]], ptr @a, i64 0, i64 %indvar, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, ptr %arrayidx14, align 4, !tbaa !2
  %arrayidx18 = getelementptr inbounds [500 x [500 x float]], ptr @b, i64 0, i64 %indvars.iv, i64 %indvars.iv47, !intel-tbaa !2
  %1 = load float, ptr %arrayidx18, align 4, !tbaa !2
  %mul = fmul float %0, %1
  %add = fadd float %add44, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 500
  br i1 %exitcond, label %for.cond.cleanup9, label %for.body10
}

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  br label %for.cond1.preheader.i

for.cond1.preheader.i:                            ; preds = %for.cond.cleanup3.i, %entry
  %indvar.i = phi i64 [ 0, %entry ], [ %indvar.next.i, %for.cond.cleanup3.i ]
  br label %for.body4.i

for.cond.cleanup3.i:                              ; preds = %for.cond.cleanup9.i
  %indvar.next.i = add nuw nsw i64 %indvar.i, 1
  %exitcond51.i = icmp eq i64 %indvar.next.i, 500
  br i1 %exitcond51.i, label %_Z8multiplyv.exit, label %for.cond1.preheader.i

for.body4.i:                                      ; preds = %for.cond.cleanup9.i, %for.cond1.preheader.i
  %indvars.iv47.i = phi i64 [ 0, %for.cond1.preheader.i ], [ %indvars.iv.next48.i, %for.cond.cleanup9.i ]
  %arrayidx6.i = getelementptr inbounds [500 x [500 x float]], ptr @c, i64 0, i64 %indvar.i, i64 %indvars.iv47.i, !intel-tbaa !2
  store float 0.000000e+00, ptr %arrayidx6.i, align 4, !tbaa !2
  br label %for.body10.i

for.cond.cleanup9.i:                              ; preds = %for.body10.i
  %add.i.lcssa = phi float [ %add.i, %for.body10.i ]
  store float %add.i.lcssa, ptr %arrayidx6.i, align 4, !tbaa !2
  %indvars.iv.next48.i = add nuw nsw i64 %indvars.iv47.i, 1
  %exitcond49.i = icmp eq i64 %indvars.iv.next48.i, 500
  br i1 %exitcond49.i, label %for.cond.cleanup3.i, label %for.body4.i

for.body10.i:                                     ; preds = %for.body10.i, %for.body4.i
  %indvars.iv.i = phi i64 [ 0, %for.body4.i ], [ %indvars.iv.next.i, %for.body10.i ]
  %add44.i = phi float [ 0.000000e+00, %for.body4.i ], [ %add.i, %for.body10.i ]
  %arrayidx14.i = getelementptr inbounds [500 x [500 x float]], ptr @a, i64 0, i64 %indvar.i, i64 %indvars.iv.i, !intel-tbaa !2
  %0 = load float, ptr %arrayidx14.i, align 4, !tbaa !2
  %arrayidx18.i = getelementptr inbounds [500 x [500 x float]], ptr @b, i64 0, i64 %indvars.iv.i, i64 %indvars.iv47.i, !intel-tbaa !2
  %1 = load float, ptr %arrayidx18.i, align 4, !tbaa !2
  %mul.i = fmul float %0, %1
  %add.i = fadd float %add44.i, %mul.i
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.i = icmp eq i64 %indvars.iv.next.i, 500
  br i1 %exitcond.i, label %for.cond.cleanup9.i, label %for.body10.i

_Z8multiplyv.exit:                                ; preds = %for.cond.cleanup3.i
  ret i32 0
}

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_float_matmul_with_reset.cpp() #4 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(ptr nonnull @_ZStL8__ioinit)
  %0 = tail call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr @_ZStL8__ioinit, ptr nonnull @__dso_handle) #2
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA500_A500_f", !4, i64 0}
!4 = !{!"array@_ZTSA500_f", !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}

