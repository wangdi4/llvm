; Same as sin-cos-double-scalar-01.ll, but we run it through the O2 pipeline,
; to ensure that the pass is running properly.
; Also check that it's disabled with -O0.

; RUN: opt -passes="default<O2>" -enable-transform-sin-cos-double -S %s | FileCheck %s

; RUN: opt -passes="default<O0>" -enable-transform-sin-cos-double -S %s | FileCheck %s --check-prefix=NOOPT

; NOOPT-NOT: sinpi
; NOOPT-NOT: cospi
; CHECK-DAG: call fast double @sinpi(double %mul2.overpi)
; CHECK-DAG: call fast double @cospi(double %mul3.overpi)

; Function Attrs: nounwind readnone uwtable willreturn mustprogress
define dso_local double @_Z7Computed(double %fArg) local_unnamed_addr #0 {
entry:
  %mul = fmul fast double %fArg, 8.000000e+00
  %0 = tail call fast double @llvm.sin.f64(double %mul) #2
  %1 = tail call fast double @llvm.cos.f64(double %mul) #2
  %mul2 = fmul fast double %fArg, 0x403163996FA82E88
  %2 = tail call fast double @llvm.sin.f64(double %mul2) #2
  %mul3 = fmul fast double %fArg, 1.025000e+01
  %3 = tail call fast double @llvm.cos.f64(double %mul3) #2
  %sub = fadd fast double %fArg, -2.000000e+01
  %4 = tail call fast double @llvm.sin.f64(double %sub) #2
  %mul4 = fmul fast double %fArg, %fArg
  %5 = tail call fast double @llvm.cos.f64(double %mul4) #2
  %6 = fmul fast double %fArg, 0x400921FB54442D11
  %mul5 = fadd fast double %6, 0x403736A2789CF9B1
  %7 = tail call fast double @llvm.sin.f64(double %mul5) #2
  %8 = tail call fast double @llvm.cos.f64(double %mul5) #2
  %add6 = fadd fast double %0, %1
  %add7 = fadd fast double %add6, %2
  %add8 = fadd fast double %add7, %3
  %add9 = fadd fast double %add8, %4
  %add10 = fadd fast double %add9, %5
  %add11 = fadd fast double %add10, %7
  %add12 = fadd fast double %add11, %8
  ret double %add12
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.sin.f64(double) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.cos.f64(double) #1

attributes #0 = { "imf-precision"="low" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { "imf-precision"="low" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (2021.2.0.YYYYMMDD)"}
