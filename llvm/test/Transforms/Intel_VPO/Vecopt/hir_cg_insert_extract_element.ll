; Check that VPlan HIR vectorizer codegen generates correct sequence of instructions
; to revectorize insert/extractelement HLInsts seen in incoming HIR.

; Incoming HIR
;   BEGIN REGION { }
;         %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;         + DO i1 = 0, 99, 1   <DO_LOOP> <ivdep>
;         |   %1 = (<2 x float>*)(%src1)[i1];
;         |   %_M_value.real.i.i.i = (%src2)[i1].0.0;
;         |   %_M_value.imag.i.i.i = (%src2)[i1].0.1;
;         |   %retval.sroa.0.0.vec.extract.i = extractelement %1,  0;
;         |   %retval.sroa.0.4.vec.extract.i = extractelement %1,  1;
;         |   %2 = %retval.sroa.0.0.vec.extract.i  *  %_M_value.real.i.i.i;
;         |   %3 = %_M_value.imag.i.i.i  *  %retval.sroa.0.4.vec.extract.i;
;         |   %4 = %3  +  %2;
;         |   %5 = %_M_value.real.i.i.i  *  %_M_value.real.i.i.i;
;         |   %6 = %_M_value.imag.i.i.i  *  %_M_value.imag.i.i.i;
;         |   %7 = %6  +  %5;
;         |   %8 = %retval.sroa.0.4.vec.extract.i  *  %_M_value.real.i.i.i;
;         |   %9 = %_M_value.imag.i.i.i  *  %retval.sroa.0.0.vec.extract.i;
;         |   %10 = %8  -  %9;
;         |   %11 = %4  /  %7;
;         |   %12 = %10  /  %7;
;         |   %retval.sroa.0.0.vec.insert.i = insertelement poison,  %11,  0;
;         |   %retval.sroa.0.4.vec.insert.i = insertelement %retval.sroa.0.0.vec.insert.i,  %12,  1;
;         |   (<2 x float>*)(%dest)[i1] = %retval.sroa.0.4.vec.insert.i;
;         + END LOOP
;
;         @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;   END REGION

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

; CHECK-LABEL: Function: foo
; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:       + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize> <ivdep>
; CHECK-NEXT:       |   %.vec = (<8 x float>*)(%src1)[i1];
; CHECK-NEXT:       |   %.vec1 = (<4 x float>*)(%src2)[i1 + <i64 0, i64 1, i64 2, i64 3>].0.0;
; CHECK-NEXT:       |   %.vec2 = (<4 x float>*)(%src2)[i1 + <i64 0, i64 1, i64 2, i64 3>].0.1;
; CHECK-NEXT:       |   %wide.extract = shufflevector %.vec,  undef,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:       |   %wide.extract3 = shufflevector %.vec,  undef,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:       |   %.vec4 = %wide.extract  *  %.vec1;
; CHECK-NEXT:       |   %.vec5 = %.vec2  *  %wide.extract3;
; CHECK-NEXT:       |   %.vec6 = %.vec5  +  %.vec4;
; CHECK-NEXT:       |   %.vec7 = %.vec1  *  %.vec1;
; CHECK-NEXT:       |   %.vec8 = %.vec2  *  %.vec2;
; CHECK-NEXT:       |   %.vec9 = %.vec8  +  %.vec7;
; CHECK-NEXT:       |   %.vec10 = %wide.extract3  *  %.vec1;
; CHECK-NEXT:       |   %.vec11 = %.vec2  *  %wide.extract;
; CHECK-NEXT:       |   %.vec12 = %.vec10  -  %.vec11;
; CHECK-NEXT:       |   %.vec13 = %.vec6  /  %.vec9;
; CHECK-NEXT:       |   %.vec14 = %.vec12  /  %.vec9;
; CHECK-NEXT:       |   %wide.insert = shufflevector %.vec13,  undef,  <i32 0, i32 undef, i32 1, i32 undef, i32 2, i32 undef, i32 3, i32 undef>;
; CHECK-NEXT:       |   %.extended = shufflevector %.vec14,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>;
; CHECK-NEXT:       |   %wide.insert15 = shufflevector %wide.insert,  %.extended,  <i32 0, i32 8, i32 2, i32 9, i32 4, i32 10, i32 6, i32 11>;
; CHECK-NEXT:       |   (<8 x float>*)(%dest)[i1] = %wide.insert15;
; CHECK-NEXT:       + END LOOP
; CHECK:      END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.std::complex" = type { { float, float } }

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @foo(%"struct.std::complex"* nocapture %dest, %"struct.std::complex"* nocapture readonly %src1, %"struct.std::complex"* nocapture readonly %src2) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %ptridx = getelementptr inbounds %"struct.std::complex", %"struct.std::complex"* %src1, i64 %l1.010
  %0 = bitcast %"struct.std::complex"* %ptridx to <2 x float>*
  %1 = load <2 x float>, <2 x float>* %0, align 4
  %_M_value.realp.i.i.i = getelementptr inbounds %"struct.std::complex", %"struct.std::complex"* %src2, i64 %l1.010, i32 0, i32 0
  %_M_value.real.i.i.i = load float, float* %_M_value.realp.i.i.i, align 4
  %_M_value.imagp.i.i.i = getelementptr inbounds %"struct.std::complex", %"struct.std::complex"* %src2, i64 %l1.010, i32 0, i32 1
  %_M_value.imag.i.i.i = load float, float* %_M_value.imagp.i.i.i, align 4
  %retval.sroa.0.0.vec.extract.i = extractelement <2 x float> %1, i32 0
  %retval.sroa.0.4.vec.extract.i = extractelement <2 x float> %1, i32 1
  %2 = fmul fast float %retval.sroa.0.0.vec.extract.i, %_M_value.real.i.i.i
  %3 = fmul fast float %_M_value.imag.i.i.i, %retval.sroa.0.4.vec.extract.i
  %4 = fadd fast float %3, %2
  %5 = fmul fast float %_M_value.real.i.i.i, %_M_value.real.i.i.i
  %6 = fmul fast float %_M_value.imag.i.i.i, %_M_value.imag.i.i.i
  %7 = fadd fast float %6, %5
  %8 = fmul fast float %retval.sroa.0.4.vec.extract.i, %_M_value.real.i.i.i
  %9 = fmul fast float %_M_value.imag.i.i.i, %retval.sroa.0.0.vec.extract.i
  %10 = fsub fast float %8, %9
  %11 = fdiv fast float %4, %7
  %12 = fdiv fast float %10, %7
  %retval.sroa.0.0.vec.insert.i = insertelement <2 x float> poison, float %11, i32 0
  %retval.sroa.0.4.vec.insert.i = insertelement <2 x float> %retval.sroa.0.0.vec.insert.i, float %12, i32 1
  %ptridx2 = getelementptr inbounds %"struct.std::complex", %"struct.std::complex"* %dest, i64 %l1.010
  %ref.tmp.sroa.0.0..sroa_cast3 = bitcast %"struct.std::complex"* %ptridx2 to <2 x float>*
  store <2 x float> %retval.sroa.0.4.vec.insert.i, <2 x float>* %ref.tmp.sroa.0.0..sroa_cast3, align 4
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="64" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

!6 = distinct !{!6, !7, !8}
!7 = !{!"llvm.loop.mustprogress"}
!8 = !{!"llvm.loop.vectorize.ivdep_back"}
