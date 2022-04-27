; Test for generating mkl call for matrix multiplication with double data type

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-generate-mkl-call -print-after=hir-generate-mkl-call -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,hir-generate-mkl-call,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;

; Before HIR Generate MKL Call-
; + DO i1 = 0, 1023, 1   <DO_LOOP>
; |   + DO i2 = 0, 1023, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 1023, 1   <DO_LOOP>
; |   |   |   %0 = (@c)[0][i1][i3];
; |   |   |   %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3];
; |   |   |   %0 = %0  +  %mul;
; |   |   |   (@c)[0][i1][i3] = %0;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP

; After HIR Generate MKL Call-
; CHECK: BEGIN REGION { modified }
; CHECK: 0 = &((i8*)(@c)[0][0][0]);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 1024;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 1024;
; CHECK: 10 = 8192;
; CHECK: 11 = 1;
; CHECK: 0 = &((i8*)(@b)[0][0][0]);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 1024;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 1024;
; CHECK: 10 = 8192;
; CHECK: 11 = 1;
; CHECK: 0 = &((i8*)(@a)[0][0][0]);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 1024;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 1024;
; CHECK: 10 = 8192;
; CHECK: 11 = 1;
; CHECK: @matmul_mkl_f64_
; CHECK: END REGION

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-generate-mkl-call -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter -disable-output < %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,hir-generate-mkl-call,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low -disable-output < %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT
;

; OPTREPORT: LOOP BEGIN
; OPTREPORT:     remark #25459: Loopnest replaced by matmul intrinsic
; OPTREPORT:     LOOP BEGIN
; OPTREPORT:         LOOP BEGIN
; OPTREPORT:         LOOP END
; OPTREPORT:     LOOP END
; OPTREPORT: LOOP END


;Module Before HIR
; ModuleID = 'double-matmul-only.cpp'
source_filename = "double-matmul-only.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }

@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@a = dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_double_matmul_only.cpp, i8* null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #2

; Function Attrs: norecurse nounwind uwtable
define dso_local void @_Z8multiplyv() local_unnamed_addr #3 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc20, %entry
  %indvars.iv41 = phi i64 [ 0, %entry ], [ %indvars.iv.next42, %for.inc20 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc17, %for.cond1.preheader
  %indvars.iv38 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next39, %for.inc17 ]
  %arrayidx16 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %indvars.iv41, i64 %indvars.iv38, !intel-tbaa !2
  %arrayidx16.promoted = load double, double* %arrayidx16, align 8, !tbaa !2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %0 = phi double [ %arrayidx16.promoted, %for.cond4.preheader ], [ %add, %for.body6 ]
  %arrayidx8 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %indvars.iv41, i64 %indvars.iv, !intel-tbaa !2
  %1 = load double, double* %arrayidx8, align 8, !tbaa !2
  %arrayidx12 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %indvars.iv, i64 %indvars.iv38, !intel-tbaa !2
  %2 = load double, double* %arrayidx12, align 8, !tbaa !2
  %mul = fmul double %1, %2
  %add = fadd double %0, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.inc17, label %for.body6

for.inc17:                                        ; preds = %for.body6
  %add.lcssa = phi double [ %add, %for.body6 ]
  store double %add.lcssa, double* %arrayidx16, align 8, !tbaa !2
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond40 = icmp eq i64 %indvars.iv.next39, 1024
  br i1 %exitcond40, label %for.inc20, label %for.cond4.preheader

for.inc20:                                        ; preds = %for.inc17
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43 = icmp eq i64 %indvars.iv.next42, 1024
  br i1 %exitcond43, label %for.end22, label %for.cond1.preheader

for.end22:                                        ; preds = %for.inc20
  ret void
}

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_double_matmul_only.cpp() #4 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZStL8__ioinit)
  %0 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZStL8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #2
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
!3 = !{!"array@_ZTSA1024_A1024_d", !4, i64 0}
!4 = !{!"array@_ZTSA1024_d", !5, i64 0}
!5 = !{!"double", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
