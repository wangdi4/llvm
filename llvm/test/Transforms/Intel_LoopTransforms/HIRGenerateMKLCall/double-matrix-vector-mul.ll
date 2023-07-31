; Test for generating mkl call for matrix multiplication with a vector

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,hir-generate-mkl-call,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;

; matmul code embedded inside another loop-
; for (int l = 0; l < 100; l++) {
;   for (int i = 0; i < 1000; i++) {
;     for (int j = 0; j < 1000; j++) {
;       c[i] += a[i][j] * b[j];
;     }
;   }
; }

; Before HIR Generate MKL Call-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   + DO i2 = 0, 999, 1   <DO_LOOP>
; |   |   %add130 = (@c)[0][i2];
; |   |
; |   |   + DO i3 = 0, 999, 1   <DO_LOOP>
; |   |   |   %mul = (@a)[0][i2][i3]  *  (@b)[0][i3];
; |   |   |   %add130 = %add130  +  %mul;
; |   |   + END LOOP
; |   |
; |   |   (@c)[0][i2] = %add130;
; |   + END LOOP
; + END LOOP


; After HIR Generate MKL Call-
; CHECK: BEGIN REGION { modified }
; CHECK: 0 = &((i8*)(@c)[0][0]);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 1;
; CHECK: 5 = 0;
; CHECK: 6 = 1000;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 0 = &((i8*)(@b)[0][0]);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 1;
; CHECK: 5 = 0;
; CHECK: 6 = 1000;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 0 = &((i8*)(@a)[0][0][0]);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 1000;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 1000;
; CHECK: 10 = 8000;
; CHECK: 11 = 1;
; CHECK: @matmul_mkl_f64_
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'double-matrix-vector-mul.cpp'
source_filename = "double-matrix-vector-mul.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }

@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@a = dso_local local_unnamed_addr global [1000 x [1000 x double]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [1000 x double] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [1000 x double] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_double_matrix_vector_mul.cpp, ptr null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #2

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %l.034 = phi i32 [ 0, %entry ], [ %inc19, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret i32 0

for.cond5.preheader:                              ; preds = %for.cond.cleanup7, %for.cond1.preheader
  %indvars.iv35 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next36, %for.cond.cleanup7 ]
  %arrayidx14 = getelementptr inbounds [1000 x double], ptr @c, i64 0, i64 %indvars.iv35, !intel-tbaa !2
  %arrayidx14.promoted = load double, ptr %arrayidx14, align 8, !tbaa !2
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %inc19 = add nuw nsw i32 %l.034, 1
  %exitcond38 = icmp eq i32 %inc19, 100
  br i1 %exitcond38, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.body8
  %add.lcssa = phi double [ %add, %for.body8 ]
  store double %add.lcssa, ptr %arrayidx14, align 8, !tbaa !2
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next36, 1000
  br i1 %exitcond37, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.body8, %for.cond5.preheader
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next, %for.body8 ]
  %add32 = phi double [ %arrayidx14.promoted, %for.cond5.preheader ], [ %add, %for.body8 ]
  %arrayidx10 = getelementptr inbounds [1000 x [1000 x double]], ptr @a, i64 0, i64 %indvars.iv35, i64 %indvars.iv, !intel-tbaa !7
  %0 = load double, ptr %arrayidx10, align 8, !tbaa !7
  %arrayidx12 = getelementptr inbounds [1000 x double], ptr @b, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load double, ptr %arrayidx12, align 8, !tbaa !2
  %mul = fmul double %0, %1
  %add = fadd double %add32, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup7, label %for.body8
}

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_double_matrix_vector_mul.cpp() #4 section ".text.startup" {
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
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA1000_A1000_d", !3, i64 0}

