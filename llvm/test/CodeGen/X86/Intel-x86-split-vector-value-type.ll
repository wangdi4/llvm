; RUN: opt --mcpu=skylake-avx512 -S --x86-split-vector-value-type --verify < %s | FileCheck %s

; Tests for x86-split-vector-value-type pass

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z10foo_doublePi(i32* noalias nocapture %result) #0 {
omp.inner.for.body.lr.ph:
  br label %hir.L.1

; CHECK:      {{phi <16 x i32>}}
; CHECK:      {{phi <16 x double>}}
; CHECK:      {{phi <16 x i1>}}
; CHECK:      {{fcmp fast olt <16 x double>}}
; CHECK:      {{and <16 x i1>}}
; CHECK:      {{icmp sgt <16 x i32>}}
; CHECK:      {{select <16 x i1>.*<16 x i32>}}
; CHECK:      {{and <16 x i1>}}
; CHECK:      {{select <16 x i1>.*<16 x i32>}}
; CHECK:      {{bitcast <32 x i1>}}

hir.L.1:                                          ; preds = %then.38, %omp.inner.for.body.lr.ph
  %t3.0 = phi <32 x i32> [ undef, %omp.inner.for.body.lr.ph ], [ %22, %then.38 ]
  %t6.0 = phi <32 x double> [ <double 1.000000e+00, double 2.000000e+00, double 3.000000e+00, double 4.000000e+00, double 5.000000e+00, double 6.000000e+00, double 7.000000e+00, double 8.000000e+00, double 9.000000e+00, double 1.000000e+01, double 1.100000e+01, double 1.200000e+01, double 1.300000e+01, double 1.400000e+01, double 1.500000e+01, double 1.600000e+01, double 1.700000e+01, double 1.800000e+01, double 1.900000e+01, double 2.000000e+01, double 2.100000e+01, double 2.200000e+01, double 2.300000e+01, double 2.400000e+01, double 2.500000e+01, double 2.600000e+01, double 2.700000e+01, double 2.800000e+01, double 2.900000e+01, double 3.000000e+01, double 3.100000e+01, double 3.200000e+01>, %omp.inner.for.body.lr.ph ], [ %18, %then.38 ]
  %t5.0 = phi <32 x double> [ zeroinitializer, %omp.inner.for.body.lr.ph ], [ %25, %then.38 ]
  %t4.0 = phi <32 x i32> [ <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>, %omp.inner.for.body.lr.ph ], [ %broadcast.splat91, %then.38 ]
  %t7.0 = phi <32 x i1> [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %omp.inner.for.body.lr.ph ], [ %21, %then.38 ]
  %vec.phi31.extract.0.45 = extractelement <32 x i32> %t4.0, i32 0
  %0 = fadd fast <32 x double> %t5.0, %t6.0
  %hir.cmp.72 = fcmp fast olt <32 x double> %0, <double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01>
  %1 = and <32 x i1> %t7.0, %hir.cmp.72
  %2 = fmul fast <32 x double> %t6.0, %t6.0
  %3 = fadd fast <32 x double> %2, %t5.0
  %4 = fmul fast <32 x double> %t5.0, %t5.0
  %5 = fsub fast <32 x double> %4, %3
  %6 = add i32 %vec.phi31.extract.0.45, -1
  %broadcast.splatinsert53 = insertelement <32 x i32> undef, i32 %6, i32 0
  %broadcast.splat54 = shufflevector <32 x i32> %broadcast.splatinsert53, <32 x i32> undef, <32 x i32> zeroinitializer
  %hir.cmp.82 = icmp sgt <32 x i32> %t4.0, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %7 = select <32 x i1> %1, <32 x i32> %broadcast.splat54, <32 x i32> %t4.0
  %8 = and <32 x i1> %1, %hir.cmp.82
  %9 = select <32 x i1> %t7.0, <32 x i32> %7, <32 x i32> %t3.0
  %10 = bitcast <32 x i1> %8 to i32
  %hir.cmp.92 = icmp eq i32 %10, 0
  br i1 %hir.cmp.92, label %hir.L.68, label %ifmerge.101

hir.L.68:                                         ; preds = %ifmerge.101, %hir.L.1
  %t3.1 = phi <32 x i32> [ %9, %hir.L.1 ], [ %22, %ifmerge.101 ]
  %11 = icmp sgt <32 x i32> %t3.1, zeroinitializer
  %12 = sub nsw <32 x i32> <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>, %t3.1
  %13 = select <32 x i1> %11, <32 x i32> %12, <32 x i32> zeroinitializer
  %14 = bitcast i32* %result to <32 x i32>*
  %wide.load = load <32 x i32>, <32 x i32>* %14, align 4
  %15 = add nsw <32 x i32> %wide.load, %13
  store <32 x i32> %15, <32 x i32>* %14, align 4
  ret void

ifmerge.101:                                      ; preds = %hir.L.1
  %hir.cmp.8 = fcmp fast olt <32 x double> %4, <double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01>
  %16 = and <32 x i1> %8, %hir.cmp.8
  %17 = fmul fast <32 x double> %3, %3
  %18 = fadd fast <32 x double> %17, %5
  %19 = add i32 %vec.phi31.extract.0.45, -2
  %broadcast.splatinsert89 = insertelement <32 x i32> undef, i32 %19, i32 0
  %broadcast.splat91 = shufflevector <32 x i32> %broadcast.splatinsert89, <32 x i32> undef, <32 x i32> zeroinitializer
  %hir.cmp.19 = icmp sgt <32 x i32> %broadcast.splat54, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %20 = select <32 x i1> %16, <32 x i32> %broadcast.splat91, <32 x i32> %broadcast.splat54
  %21 = and <32 x i1> %16, %hir.cmp.19
  %22 = select <32 x i1> %8, <32 x i32> %20, <32 x i32> %9
  %23 = bitcast <32 x i1> %21 to i32
  %hir.cmp.29 = icmp eq i32 %23, 0
  br i1 %hir.cmp.29, label %hir.L.68, label %then.38

then.38:                                          ; preds = %ifmerge.101
  %24 = fmul fast <32 x double> %5, %5
  %25 = fsub fast <32 x double> %24, %18
  br label %hir.L.1, !llvm.loop !2
}

define void @_Z9foo_floatPi(i32* noalias nocapture %result) #0 {
omp.inner.for.body.lr.ph:
  br label %hir.L.1

; CHECK:      {{phi <16 x i32>}}
; CHECK:      {{phi <16 x float>}}
; CHECK:      {{phi <16 x i1>}}
; CHECK:      {{fcmp fast olt <16 x float>}}
; CHECK:      {{and <16 x i1>}}
; CHECK:      {{icmp sgt <16 x i32>}}
; CHECK:      {{select <16 x i1>.*<16 x i32>}}
; CHECK:      {{and <16 x i1>}}
; CHECK:      {{select <16 x i1>.*<16 x i32>}}
; CHECK:      {{bitcast <32 x i1>}}

hir.L.1:                                          ; preds = %then.38, %omp.inner.for.body.lr.ph
  %t3.0 = phi <32 x i32> [ undef, %omp.inner.for.body.lr.ph ], [ %22, %then.38 ]
  %t6.0 = phi <32 x float> [ <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00, float 9.000000e+00, float 1.000000e+01, float 1.100000e+01, float 1.200000e+01, float 1.300000e+01, float 1.400000e+01, float 1.500000e+01, float 1.600000e+01, float 1.700000e+01, float 1.800000e+01, float 1.900000e+01, float 2.000000e+01, float 2.100000e+01, float 2.200000e+01, float 2.300000e+01, float 2.400000e+01, float 2.500000e+01, float 2.600000e+01, float 2.700000e+01, float 2.800000e+01, float 2.900000e+01, float 3.000000e+01, float 3.100000e+01, float 3.200000e+01>, %omp.inner.for.body.lr.ph ], [ %18, %then.38 ]
  %t5.0 = phi <32 x float> [ zeroinitializer, %omp.inner.for.body.lr.ph ], [ %25, %then.38 ]
  %t4.0 = phi <32 x i32> [ <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>, %omp.inner.for.body.lr.ph ], [ %broadcast.splat93, %then.38 ]
  %t7.0 = phi <32 x i1> [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %omp.inner.for.body.lr.ph ], [ %21, %then.38 ]
  %vec.phi33.extract.0.47 = extractelement <32 x i32> %t4.0, i32 0
  %0 = fadd fast <32 x float> %t5.0, %t6.0
  %hir.cmp.72 = fcmp fast olt <32 x float> %0, <float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01>
  %1 = and <32 x i1> %t7.0, %hir.cmp.72
  %2 = fmul fast <32 x float> %t6.0, %t6.0
  %3 = fadd fast <32 x float> %2, %t5.0
  %4 = fmul fast <32 x float> %t5.0, %t5.0
  %5 = fsub fast <32 x float> %4, %3
  %6 = add i32 %vec.phi33.extract.0.47, -1
  %broadcast.splatinsert55 = insertelement <32 x i32> undef, i32 %6, i32 0
  %broadcast.splat56 = shufflevector <32 x i32> %broadcast.splatinsert55, <32 x i32> undef, <32 x i32> zeroinitializer
  %hir.cmp.82 = icmp sgt <32 x i32> %t4.0, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %7 = select <32 x i1> %1, <32 x i32> %broadcast.splat56, <32 x i32> %t4.0
  %8 = and <32 x i1> %1, %hir.cmp.82
  %9 = select <32 x i1> %t7.0, <32 x i32> %7, <32 x i32> %t3.0
  %10 = bitcast <32 x i1> %8 to i32
  %hir.cmp.92 = icmp eq i32 %10, 0
  br i1 %hir.cmp.92, label %hir.L.68, label %ifmerge.101

hir.L.68:                                         ; preds = %ifmerge.101, %hir.L.1
  %t3.1 = phi <32 x i32> [ %9, %hir.L.1 ], [ %22, %ifmerge.101 ]
  %11 = icmp sgt <32 x i32> %t3.1, zeroinitializer
  %12 = sub nsw <32 x i32> <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>, %t3.1
  %13 = select <32 x i1> %11, <32 x i32> %12, <32 x i32> zeroinitializer
  %14 = bitcast i32* %result to <32 x i32>*
  %wide.load = load <32 x i32>, <32 x i32>* %14, align 4
  %15 = add nsw <32 x i32> %wide.load, %13
  store <32 x i32> %15, <32 x i32>* %14, align 4
  ret void

ifmerge.101:                                      ; preds = %hir.L.1
  %hir.cmp.8 = fcmp fast olt <32 x float> %4, <float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01>
  %16 = and <32 x i1> %8, %hir.cmp.8
  %17 = fmul fast <32 x float> %3, %3
  %18 = fadd fast <32 x float> %17, %5
  %19 = add i32 %vec.phi33.extract.0.47, -2
  %broadcast.splatinsert91 = insertelement <32 x i32> undef, i32 %19, i32 0
  %broadcast.splat93 = shufflevector <32 x i32> %broadcast.splatinsert91, <32 x i32> undef, <32 x i32> zeroinitializer
  %hir.cmp.19 = icmp sgt <32 x i32> %broadcast.splat56, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %20 = select <32 x i1> %16, <32 x i32> %broadcast.splat93, <32 x i32> %broadcast.splat56
  %21 = and <32 x i1> %16, %hir.cmp.19
  %22 = select <32 x i1> %8, <32 x i32> %20, <32 x i32> %9
  %23 = bitcast <32 x i1> %21 to i32
  %hir.cmp.29 = icmp eq i32 %23, 0
  br i1 %hir.cmp.29, label %hir.L.68, label %then.38

then.38:                                          ; preds = %ifmerge.101
  %24 = fmul fast <32 x float> %5, %5
  %25 = fsub fast <32 x float> %24, %18
  br label %hir.L.1, !llvm.loop !4
}

attributes #0 = { nounwind uwtable }

!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.unroll.disable"}
!4 = distinct !{!4, !3}
