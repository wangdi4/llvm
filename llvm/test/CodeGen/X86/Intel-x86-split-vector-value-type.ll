; RUN: opt --mcpu=skylake-avx512 -S --x86-split-vector-value-type --verify < %s | FileCheck %s

; Tests for x86-split-vector-value-type pass.
; If an instruction has name, the split instructions will be name as [original inst name].(l|h)
; e.g. %x will be split into %x.l and %x.h.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define <32 x i32> @candidateTest1(<32 x i32>* %x0_ptr, <32 x i32>* %y0_ptr) {
; CHECK-LABEL: candidateTest1
; CHECK:       %cmp0 = icmp sgt <32 x i32>
entry:
  %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
  %y0 = load <32 x i32>, <32 x i32>* %y0_ptr
  %cmp0 = icmp sgt <32 x i32> %x0, %y0
  %cmp0.broadcast = shufflevector <32 x i1> %cmp0, <32 x i1> undef, <32 x i32> zeroinitializer
  %res = select <32 x i1> %cmp0.broadcast, <32 x i32> %x0, <32 x i32> %y0
  ret <32 x i32> %res
}

define <32 x i1> @insertelementSplitTest(<32 x i32>* %data_ptr, i32 %val1, i32 %val2) {
; CHECK-LABEL:  insertelementSplitTest
; CHECK:        %x0.h = insertelement <16 x i32> %data.h, i32 %val1, i32 4
; CHECK-NEXT:   %y0.l = insertelement <16 x i32> undef, i32 %val2, i32 15
entry:
  %data = load <32 x i32>, <32 x i32>* %data_ptr
  %x0 = insertelement <32 x i32> %data, i32 %val1, i32 20
  %y0 = insertelement <32 x i32> undef, i32 %val2, i32 15
  %cmp0 = icmp sgt <32 x i32> %x0, %y0
  %cmp1 = icmp sgt <32 x i32> %x0, zeroinitializer
  %res = and <32 x i1> %cmp0, %cmp1
  ret <32 x i1> %res
}

define <32 x i1> @shufflevectorSplitTest(<32 x i32>* %x0_ptr, <32 x i32>* %y0_ptr) {
; CHECK-LABEL:  shufflevectorSplitTest
; CHECK:        %x1.h = shufflevector <16 x i32> %x0.h, <16 x i32> undef, <16 x i32> <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
; CHECK-NEXT:   %y1.l = shufflevector <16 x i32> %y0.l, <16 x i32> undef, <16 x i32> zeroinitializer
entry:
  %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
  %y0 = load <32 x i32>, <32 x i32>* %y0_ptr
  %x1 = shufflevector <32 x i32> %x0, <32 x i32> undef, <32 x i32> <i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20>
  %y1 = shufflevector <32 x i32> %y0, <32 x i32> undef, <32 x i32> zeroinitializer
  %cmp0 = icmp sgt <32 x i32> %x1, %y1
  %cmp1 = icmp sgt <32 x i32> %x1, zeroinitializer
  %res = and <32 x i1> %cmp0, %cmp1
  ret <32 x i1> %res
}

define void @phiSplitTest(<32 x i32>* %x0_ptr, <32 x i1>* %cond0_ptr) {
; CHECK-LABEL:  phiSplitTest
; CHECK:        bb1:
; CHECK-NEXT:   %x1.l = phi <16 x i32> [ %x0.l, %entry ], [ %x3.l, %bb3 ]
; CHECK-NEXT:   %x1.h = phi <16 x i32> [ %x0.h, %entry ], [ %x3.h, %bb3 ]
; CHECK:        bb2:
; CHECK-NEXT:   %x2.l = phi <16 x i32> [ %x1.l, %bb1 ]
; CHECK-NEXT:   %x2.h = phi <16 x i32> [ %x1.h, %bb1 ]
; CHECK:        bb3:
; CHECK-NEXT:   %x3.l = phi <16 x i32> [ %x2.l, %bb2 ]
; CHECK-NEXT:   %x3.h = phi <16 x i32> [ %x2.h, %bb2 ]
entry:
  %cond0 = load <32 x i1>, <32 x i1>* %cond0_ptr
  %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
  br label %bb1

bb1:
  %x1 = phi <32 x i32> [ %x0, %entry ], [ %x3, %bb3 ]
  br label %bb2

bb2:
  %x2 = phi <32 x i32> [ %x1, %bb1 ]
  br label %bb3

bb3:
  %x3 = phi <32 x i32> [ %x2, %bb2 ]
  %cmp3 = icmp sgt <32 x i32> %x3, zeroinitializer
  %cond1 = and <32 x i1> %cmp3, %cond0
  %bc3 = bitcast <32 x i1> %cond1 to i32
  %zcmp3 = icmp eq i32 %bc3, 0
  br i1 %zcmp3, label %bb4, label %bb1

bb4:
  ret void
}

define i32 @foldFusedShufflevectorExtractElementTest(<32 x i32>* %x0_ptr, <32 x i32>* %y0_ptr) {
; CHECK-LABEL: foldFusedShufflevectorExtractElementTest
; CHECK:       %x1.h.extract.4{{.*}} = extractelement <16 x i32> %x1.h, i32 4
; CHECK-NEXT:  %y1.l.extract.15{{.*}} = extractelement <16 x i32> %y1.l, i32 15
entry:
  %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
  %y0 = load <32 x i32>, <32 x i32>* %y0_ptr
  %x1 = insertelement <32 x i32> %x0, i32 1, i32 20
  %y1 = insertelement <32 x i32> %y0, i32 1, i32 15
  %x1.extract.20 = extractelement <32 x i32> %x1, i32 20
  %y1.extract.15 = extractelement <32 x i32> %y1, i32 15
  %cmp0 = icmp sgt <32 x i32> %x1, %y1
  %cmp1 = icmp sgt <32 x i32> %x1, zeroinitializer
  %cond0 = and <32 x i1> %cmp0, %cmp1
  %bc = bitcast <32 x i1> %cond0 to i32
  %sum0 = add i32 %x1.extract.20, %y1.extract.15
  %sum1 = add i32 %sum0, %bc
  ret i32 %sum1
}

define <32 x i1> @foldSplattedCmpShuffleVectorTest(<32 x i32>* %x_ptr) {
; CHECK-LABEL: foldSplattedCmpShuffleVectorTest
; CHECK:       %{{.+}} = shufflevector <16 x i32> %x.h, <16 x i32> undef, <16 x i32> <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
; CHECK-NEXT:  %z.h = icmp sgt <16 x i32> %{{.+}}, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
entry:
  %x = load <32 x i32>, <32 x i32>* %x_ptr
  %cmp = icmp sgt <32 x i32> %x, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %z = shufflevector <32 x i1> %cmp, <32 x i1> undef, <32 x i32> <i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24>
  %res = and <32 x i1> %z, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  ret <32 x i1> %res
}

define void @foo_double(i32* noalias nocapture %result) {
; CHECK-LABEL: foo_double
; CHECK:       phi <16 x i32>
; CHECK:       phi <16 x double>
; CHECK:       phi <16 x i32>
; CHECK:       phi <16 x i1>
; CHECK:       extractelement <16 x i32>{{.*}} i32 0
; CHECK:       fcmp fast olt <16 x double>
; CHECK:       and <16 x i1>
; CHECK:       insertelement <16 x i32>{{.*}} i32 0
; CHECK-NEXT:  shufflevector <16 x i32>{{.*}} <16 x i32> zeroinitializer
; CHECK-NEXT:  icmp sgt <16 x i32>
; CHECK:       select <16 x i1>{{.*}}<16 x i32>
; CHECK:       and <16 x i1>
; CHECK:       select <16 x i1>{{.*}}<16 x i32>
; CHECK:       bitcast <32 x i1>

omp.inner.for.body.lr.ph:
  br label %hir.L.1

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
  br label %hir.L.1
}

define void @foo_float(i32* noalias nocapture %result) {
; CHECK-LABEL: foo_float
; CHECK:       phi <16 x i32>
; CHECK:       phi <16 x float>
; CHECK:       phi <16 x i32>
; CHECK:       phi <16 x i1>
; CHECK:       extractelement <16 x i32>{{.*}} i32 0
; CHECK:       fcmp fast olt <16 x float>
; CHECK:       and <16 x i1>
; CHECK:       insertelement <16 x i32>{{.*}} i32 0
; CHECK-NEXT:  shufflevector <16 x i32>{{.*}} <16 x i32> zeroinitializer
; CHECK-NEXT:  icmp sgt <16 x i32>
; CHECK:       select <16 x i1>{{.*}}<16 x i32>
; CHECK:       and <16 x i1>
; CHECK:       select <16 x i1>{{.*}}<16 x i32>
; CHECK:       bitcast <32 x i1>

omp.inner.for.body.lr.ph:
  br label %hir.L.1

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
  br label %hir.L.1
}
