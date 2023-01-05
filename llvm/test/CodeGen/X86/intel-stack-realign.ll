; RUN: llc < %s -mtriple=x86_64-- -mattr=avx512f -enable-intel-advanced-opts -O3 -debug-only=x86-stack-realign -o /dev/null 2>&1 | FileCheck %s --check-prefix=COST
; RUN: llc < %s -mtriple=x86_64-- -mattr=avx512f -enable-intel-advanced-opts -O3 --x86-stack-realign-avg-cost-threshold=0.7 | FileCheck %s --check-prefix=REALIGN
; RUN: llc < %s -mtriple=x86_64-- -mattr=avx512f -enable-intel-advanced-opts -O3 --x86-stack-realign-avg-cost-threshold=1   | FileCheck %s --check-prefix=NOREALIGN

define dso_local void @foo(double* %a0, double* %a1, double* %a2, double* %a3, double* %a4, double* %a5, double* %a6, double* %a7, double* %a8, double* %a9, double* %a10, double* %a11, double* %a12, double* %a13, double* %a14, double* %a15, double* %a16, double* %a17, double* %a18, double* %a19, double* %a20, double* %a21, double* %a22, double* %a23, double* %a24, double* %a25, double* %a26, double* %a27, double* %a28, double* %a29, double* %a30, double* %a31, double* %b, double* %c, i32 %n) {
; COST:     Unalign cost info of foo:
; COST-NEXT:  Total cost = 9.8{{[0-9]*}}e+08
; COST-NEXT:  InstRetired = 1.3{{[0-9]*}}e+09
; COST-NEXT:  Avg cost = 7.2{{[0-9]*}}e-01
;
; REALIGN:        andq $-64, %rsp
; NOREALIGN-NOT:  andq $-64, %rsp
entry:
  %cmp410 = icmp sgt i32 %n, 0
  %dd = alloca [33 x { double*, double* }], align 8
  br i1 %cmp410, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %0 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 0, i32 0
  store double* %a0, double** %0, align 8
  %1 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 0, i32 1
  %2 = add nsw i64 %wide.trip.count, -1
  %3 = getelementptr inbounds double, double* %a0, i64 %2
  store double* %3, double** %1, align 8
  %4 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 1, i32 0
  store double* %a1, double** %4, align 8
  %5 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 1, i32 1
  %6 = getelementptr inbounds double, double* %a1, i64 %2
  store double* %6, double** %5, align 8
  %7 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 2, i32 0
  store double* %a2, double** %7, align 8
  %8 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 2, i32 1
  %9 = getelementptr inbounds double, double* %a2, i64 %2
  store double* %9, double** %8, align 8
  %10 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 3, i32 0
  store double* %a3, double** %10, align 8
  %11 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 3, i32 1
  %12 = getelementptr inbounds double, double* %a3, i64 %2
  store double* %12, double** %11, align 8
  %13 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 4, i32 0
  store double* %a4, double** %13, align 8
  %14 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 4, i32 1
  %15 = getelementptr inbounds double, double* %a4, i64 %2
  store double* %15, double** %14, align 8
  %16 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 5, i32 0
  store double* %a5, double** %16, align 8
  %17 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 5, i32 1
  %18 = getelementptr inbounds double, double* %a5, i64 %2
  store double* %18, double** %17, align 8
  %19 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 6, i32 0
  store double* %a6, double** %19, align 8
  %20 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 6, i32 1
  %21 = getelementptr inbounds double, double* %a6, i64 %2
  store double* %21, double** %20, align 8
  %22 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 7, i32 0
  store double* %a7, double** %22, align 8
  %23 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 7, i32 1
  %24 = getelementptr inbounds double, double* %a7, i64 %2
  store double* %24, double** %23, align 8
  %25 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 8, i32 0
  store double* %a8, double** %25, align 8
  %26 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 8, i32 1
  %27 = getelementptr inbounds double, double* %a8, i64 %2
  store double* %27, double** %26, align 8
  %28 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 9, i32 0
  store double* %a9, double** %28, align 8
  %29 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 9, i32 1
  %30 = getelementptr inbounds double, double* %a9, i64 %2
  store double* %30, double** %29, align 8
  %31 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 10, i32 0
  store double* %a10, double** %31, align 8
  %32 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 10, i32 1
  %33 = getelementptr inbounds double, double* %a10, i64 %2
  store double* %33, double** %32, align 8
  %34 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 11, i32 0
  store double* %a11, double** %34, align 8
  %35 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 11, i32 1
  %36 = getelementptr inbounds double, double* %a11, i64 %2
  store double* %36, double** %35, align 8
  %37 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 12, i32 0
  store double* %a12, double** %37, align 8
  %38 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 12, i32 1
  %39 = getelementptr inbounds double, double* %a12, i64 %2
  store double* %39, double** %38, align 8
  %40 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 13, i32 0
  store double* %a13, double** %40, align 8
  %41 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 13, i32 1
  %42 = getelementptr inbounds double, double* %a13, i64 %2
  store double* %42, double** %41, align 8
  %43 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 14, i32 0
  store double* %a14, double** %43, align 8
  %44 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 14, i32 1
  %45 = getelementptr inbounds double, double* %a14, i64 %2
  store double* %45, double** %44, align 8
  %46 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 15, i32 0
  store double* %a15, double** %46, align 8
  %47 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 15, i32 1
  %48 = getelementptr inbounds double, double* %a15, i64 %2
  store double* %48, double** %47, align 8
  %49 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 16, i32 0
  store double* %a16, double** %49, align 8
  %50 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 16, i32 1
  %51 = getelementptr inbounds double, double* %a16, i64 %2
  store double* %51, double** %50, align 8
  %52 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 17, i32 0
  store double* %a17, double** %52, align 8
  %53 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 17, i32 1
  %54 = getelementptr inbounds double, double* %a17, i64 %2
  store double* %54, double** %53, align 8
  %55 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 18, i32 0
  store double* %a18, double** %55, align 8
  %56 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 18, i32 1
  %57 = getelementptr inbounds double, double* %a18, i64 %2
  store double* %57, double** %56, align 8
  %58 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 19, i32 0
  store double* %a19, double** %58, align 8
  %59 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 19, i32 1
  %60 = getelementptr inbounds double, double* %a19, i64 %2
  store double* %60, double** %59, align 8
  %61 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 20, i32 0
  store double* %a20, double** %61, align 8
  %62 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 20, i32 1
  %63 = getelementptr inbounds double, double* %a20, i64 %2
  store double* %63, double** %62, align 8
  %64 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 21, i32 0
  store double* %a21, double** %64, align 8
  %65 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 21, i32 1
  %66 = getelementptr inbounds double, double* %a21, i64 %2
  store double* %66, double** %65, align 8
  %67 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 22, i32 0
  store double* %a22, double** %67, align 8
  %68 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 22, i32 1
  %69 = getelementptr inbounds double, double* %a22, i64 %2
  store double* %69, double** %68, align 8
  %70 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 23, i32 0
  store double* %a23, double** %70, align 8
  %71 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 23, i32 1
  %72 = getelementptr inbounds double, double* %a23, i64 %2
  store double* %72, double** %71, align 8
  %73 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 24, i32 0
  store double* %a24, double** %73, align 8
  %74 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 24, i32 1
  %75 = getelementptr inbounds double, double* %a24, i64 %2
  store double* %75, double** %74, align 8
  %76 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 25, i32 0
  store double* %a25, double** %76, align 8
  %77 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 25, i32 1
  %78 = getelementptr inbounds double, double* %a25, i64 %2
  store double* %78, double** %77, align 8
  %79 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 26, i32 0
  store double* %a26, double** %79, align 8
  %80 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 26, i32 1
  %81 = getelementptr inbounds double, double* %a26, i64 %2
  store double* %81, double** %80, align 8
  %82 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 27, i32 0
  store double* %a27, double** %82, align 8
  %83 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 27, i32 1
  %84 = getelementptr inbounds double, double* %a27, i64 %2
  store double* %84, double** %83, align 8
  %85 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 28, i32 0
  store double* %a28, double** %85, align 8
  %86 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 28, i32 1
  %87 = getelementptr inbounds double, double* %a28, i64 %2
  store double* %87, double** %86, align 8
  %88 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 29, i32 0
  store double* %a29, double** %88, align 8
  %89 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 29, i32 1
  %90 = getelementptr inbounds double, double* %a29, i64 %2
  store double* %90, double** %89, align 8
  %91 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 30, i32 0
  store double* %a30, double** %91, align 8
  %92 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 30, i32 1
  %93 = getelementptr inbounds double, double* %a30, i64 %2
  store double* %93, double** %92, align 8
  %94 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 31, i32 0
  store double* %a31, double** %94, align 8
  %95 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 31, i32 1
  %96 = getelementptr inbounds double, double* %a31, i64 %2
  store double* %96, double** %95, align 8
  %97 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 32, i32 0
  store double* %c, double** %97, align 8
  %98 = getelementptr inbounds [33 x { double*, double* }], [33 x { double*, double* }]* %dd, i64 0, i64 32, i32 1
  %99 = getelementptr inbounds double, double* %c, i64 %2
  store double* %99, double** %98, align 8
  %100 = bitcast [33 x { double*, double* }]* %dd to i8*
  %101 = call i64 @__intel_rtdd_indep(i8* nonnull %100, i64 33)
  %hir.cmp.388 = icmp eq i64 %101, 0
  %102 = add i32 %n, -1
  br i1 %hir.cmp.388, label %loop.188.preheader, label %loop.191

loop.188.preheader:                               ; preds = %for.cond1.preheader.lr.ph
  %103 = lshr i32 %102, 6
  %104 = lshr i64 %2, 6
  br label %loop.188

for.cond.cleanup:                                 ; preds = %afterloop.192, %afterloop.389, %entry
  ret void

loop.188:                                         ; preds = %loop.188.preheader, %afterloop.389
  %i1.i32.0 = phi i32 [ %nextivloop.188, %afterloop.389 ], [ 0, %loop.188.preheader ]
  br label %loop.389

loop.389:                                         ; preds = %afterloop.189, %loop.188
  %i2.i32.0 = phi i32 [ 0, %loop.188 ], [ %nextivloop.389, %afterloop.189 ]
  %105 = shl i32 %i2.i32.0, 6
  %106 = sub i32 %102, %105
  %107 = call i32 @llvm.smin.i32(i32 %106, i32 63)
  br label %loop.189

loop.189:                                         ; preds = %afterloop.391, %loop.389
  %i3.i64.0 = phi i64 [ 0, %loop.389 ], [ %nextivloop.189, %afterloop.391 ]
  %108 = shl i64 %i3.i64.0, 6
  %109 = sub i64 %2, %108
  %110 = call i64 @llvm.smin.i64(i64 %109, i64 63)
  %111 = add nsw i64 %110, 1
  %112 = and i64 %111, -8
  %extract.0.585 = icmp eq i64 %112, 0
  %113 = add i64 %112, -1
  %extract.0.517689 = icmp eq i64 %111, %112
  br label %loop.391

loop.391:                                         ; preds = %hir.L.555, %loop.189
  %i4.i32.0 = phi i32 [ 0, %loop.189 ], [ %nextivloop.391, %hir.L.555 ]
  br i1 %extract.0.585, label %loop.560.preheader, label %loop.399

loop.560.preheader:                               ; preds = %afterloop.399, %loop.391
  %i5.i64.1.ph = phi i64 [ %112, %afterloop.399 ], [ 0, %loop.391 ]
  br label %loop.560

loop.399:                                         ; preds = %loop.391, %loop.399
  %i5.i64.0 = phi i64 [ %nextivloop.399, %loop.399 ], [ 0, %loop.391 ]
  %114 = add i64 %108, %i5.i64.0
  %115 = getelementptr inbounds double, double* %a0, i64 %114
  %116 = bitcast double* %115 to <8 x double>*
  %gepload = load <8 x double>, <8 x double>* %116, align 8
  %117 = getelementptr inbounds double, double* %a1, i64 %114
  %118 = bitcast double* %117 to <8 x double>*
  %gepload587 = load <8 x double>, <8 x double>* %118, align 8
  %119 = getelementptr inbounds double, double* %a2, i64 %114
  %120 = bitcast double* %119 to <8 x double>*
  %gepload588 = load <8 x double>, <8 x double>* %120, align 8
  %121 = getelementptr inbounds double, double* %a3, i64 %114
  %122 = bitcast double* %121 to <8 x double>*
  %gepload589 = load <8 x double>, <8 x double>* %122, align 8
  %123 = getelementptr inbounds double, double* %a4, i64 %114
  %124 = bitcast double* %123 to <8 x double>*
  %gepload590 = load <8 x double>, <8 x double>* %124, align 8
  %125 = getelementptr inbounds double, double* %a5, i64 %114
  %126 = bitcast double* %125 to <8 x double>*
  %gepload591 = load <8 x double>, <8 x double>* %126, align 8
  %127 = getelementptr inbounds double, double* %a6, i64 %114
  %128 = bitcast double* %127 to <8 x double>*
  %gepload592 = load <8 x double>, <8 x double>* %128, align 8
  %129 = getelementptr inbounds double, double* %a7, i64 %114
  %130 = bitcast double* %129 to <8 x double>*
  %gepload593 = load <8 x double>, <8 x double>* %130, align 8
  %131 = getelementptr inbounds double, double* %a8, i64 %114
  %132 = bitcast double* %131 to <8 x double>*
  %gepload594 = load <8 x double>, <8 x double>* %132, align 8
  %133 = getelementptr inbounds double, double* %a9, i64 %114
  %134 = bitcast double* %133 to <8 x double>*
  %gepload595 = load <8 x double>, <8 x double>* %134, align 8
  %135 = getelementptr inbounds double, double* %a10, i64 %114
  %136 = bitcast double* %135 to <8 x double>*
  %gepload596 = load <8 x double>, <8 x double>* %136, align 8
  %137 = getelementptr inbounds double, double* %a11, i64 %114
  %138 = bitcast double* %137 to <8 x double>*
  %gepload597 = load <8 x double>, <8 x double>* %138, align 8
  %139 = getelementptr inbounds double, double* %a12, i64 %114
  %140 = bitcast double* %139 to <8 x double>*
  %gepload598 = load <8 x double>, <8 x double>* %140, align 8
  %141 = getelementptr inbounds double, double* %a13, i64 %114
  %142 = bitcast double* %141 to <8 x double>*
  %gepload599 = load <8 x double>, <8 x double>* %142, align 8
  %143 = getelementptr inbounds double, double* %a14, i64 %114
  %144 = bitcast double* %143 to <8 x double>*
  %gepload600 = load <8 x double>, <8 x double>* %144, align 8
  %145 = getelementptr inbounds double, double* %a15, i64 %114
  %146 = bitcast double* %145 to <8 x double>*
  %gepload601 = load <8 x double>, <8 x double>* %146, align 8
  %147 = getelementptr inbounds double, double* %a16, i64 %114
  %148 = bitcast double* %147 to <8 x double>*
  %gepload602 = load <8 x double>, <8 x double>* %148, align 8
  %149 = getelementptr inbounds double, double* %a17, i64 %114
  %150 = bitcast double* %149 to <8 x double>*
  %gepload603 = load <8 x double>, <8 x double>* %150, align 8
  %151 = getelementptr inbounds double, double* %a18, i64 %114
  %152 = bitcast double* %151 to <8 x double>*
  %gepload604 = load <8 x double>, <8 x double>* %152, align 8
  %153 = getelementptr inbounds double, double* %a19, i64 %114
  %154 = bitcast double* %153 to <8 x double>*
  %gepload605 = load <8 x double>, <8 x double>* %154, align 8
  %155 = getelementptr inbounds double, double* %a20, i64 %114
  %156 = bitcast double* %155 to <8 x double>*
  %gepload606 = load <8 x double>, <8 x double>* %156, align 8
  %157 = getelementptr inbounds double, double* %a21, i64 %114
  %158 = bitcast double* %157 to <8 x double>*
  %gepload607 = load <8 x double>, <8 x double>* %158, align 8
  %159 = getelementptr inbounds double, double* %a22, i64 %114
  %160 = bitcast double* %159 to <8 x double>*
  %gepload608 = load <8 x double>, <8 x double>* %160, align 8
  %161 = getelementptr inbounds double, double* %a23, i64 %114
  %162 = bitcast double* %161 to <8 x double>*
  %gepload609 = load <8 x double>, <8 x double>* %162, align 8
  %163 = getelementptr inbounds double, double* %a24, i64 %114
  %164 = bitcast double* %163 to <8 x double>*
  %gepload610 = load <8 x double>, <8 x double>* %164, align 8
  %165 = getelementptr inbounds double, double* %a25, i64 %114
  %166 = bitcast double* %165 to <8 x double>*
  %gepload611 = load <8 x double>, <8 x double>* %166, align 8
  %167 = getelementptr inbounds double, double* %a26, i64 %114
  %168 = bitcast double* %167 to <8 x double>*
  %gepload612 = load <8 x double>, <8 x double>* %168, align 8
  %169 = getelementptr inbounds double, double* %a27, i64 %114
  %170 = bitcast double* %169 to <8 x double>*
  %gepload613 = load <8 x double>, <8 x double>* %170, align 8
  %171 = getelementptr inbounds double, double* %a28, i64 %114
  %172 = bitcast double* %171 to <8 x double>*
  %gepload614 = load <8 x double>, <8 x double>* %172, align 8
  %173 = getelementptr inbounds double, double* %a29, i64 %114
  %174 = bitcast double* %173 to <8 x double>*
  %gepload615 = load <8 x double>, <8 x double>* %174, align 8
  %175 = getelementptr inbounds double, double* %a30, i64 %114
  %176 = bitcast double* %175 to <8 x double>*
  %gepload616 = load <8 x double>, <8 x double>* %176, align 8
  %177 = getelementptr inbounds double, double* %a31, i64 %114
  %178 = bitcast double* %177 to <8 x double>*
  %gepload617 = load <8 x double>, <8 x double>* %178, align 8
  %179 = fadd fast <8 x double> %gepload587, %gepload
  %180 = fadd fast <8 x double> %179, %gepload588
  %181 = fadd fast <8 x double> %180, %gepload589
  %182 = fadd fast <8 x double> %181, %gepload590
  %183 = fadd fast <8 x double> %182, %gepload591
  %184 = fadd fast <8 x double> %183, %gepload592
  %185 = fadd fast <8 x double> %184, %gepload593
  %186 = fadd fast <8 x double> %185, %gepload594
  %187 = fadd fast <8 x double> %186, %gepload595
  %188 = fadd fast <8 x double> %187, %gepload596
  %189 = fadd fast <8 x double> %188, %gepload597
  %190 = fadd fast <8 x double> %189, %gepload598
  %191 = fadd fast <8 x double> %190, %gepload599
  %192 = fadd fast <8 x double> %191, %gepload600
  %193 = fadd fast <8 x double> %192, %gepload601
  %194 = fadd fast <8 x double> %193, %gepload602
  %195 = fadd fast <8 x double> %194, %gepload603
  %196 = fadd fast <8 x double> %195, %gepload604
  %197 = fadd fast <8 x double> %196, %gepload605
  %198 = fadd fast <8 x double> %197, %gepload606
  %199 = fadd fast <8 x double> %198, %gepload607
  %200 = fadd fast <8 x double> %199, %gepload608
  %201 = fadd fast <8 x double> %200, %gepload609
  %202 = fadd fast <8 x double> %201, %gepload610
  %203 = fadd fast <8 x double> %202, %gepload611
  %204 = fadd fast <8 x double> %203, %gepload612
  %205 = fadd fast <8 x double> %204, %gepload613
  %206 = fadd fast <8 x double> %205, %gepload614
  %207 = fadd fast <8 x double> %206, %gepload615
  %208 = fadd fast <8 x double> %207, %gepload616
  %209 = fadd fast <8 x double> %208, %gepload617
  %210 = getelementptr inbounds double, double* %c, i64 %114
  %211 = bitcast double* %210 to <8 x double>*
  store <8 x double> %209, <8 x double>* %211, align 8
  store <8 x double> %179, <8 x double>* %116, align 8
  %212 = fadd fast <8 x double> %gepload588, %gepload587
  store <8 x double> %212, <8 x double>* %118, align 8
  %213 = fadd fast <8 x double> %gepload589, %gepload588
  store <8 x double> %213, <8 x double>* %120, align 8
  %214 = fadd fast <8 x double> %gepload590, %gepload589
  store <8 x double> %214, <8 x double>* %122, align 8
  %215 = fadd fast <8 x double> %gepload591, %gepload590
  store <8 x double> %215, <8 x double>* %124, align 8
  %216 = fadd fast <8 x double> %gepload592, %gepload591
  store <8 x double> %216, <8 x double>* %126, align 8
  %217 = fadd fast <8 x double> %gepload593, %gepload592
  store <8 x double> %217, <8 x double>* %128, align 8
  %218 = fadd fast <8 x double> %gepload594, %gepload593
  store <8 x double> %218, <8 x double>* %130, align 8
  %219 = fadd fast <8 x double> %gepload595, %gepload594
  store <8 x double> %219, <8 x double>* %132, align 8
  %220 = fadd fast <8 x double> %gepload596, %gepload595
  store <8 x double> %220, <8 x double>* %134, align 8
  %221 = fadd fast <8 x double> %gepload597, %gepload596
  store <8 x double> %221, <8 x double>* %136, align 8
  %222 = fadd fast <8 x double> %gepload598, %gepload597
  store <8 x double> %222, <8 x double>* %138, align 8
  %223 = fadd fast <8 x double> %gepload599, %gepload598
  store <8 x double> %223, <8 x double>* %140, align 8
  %224 = fadd fast <8 x double> %gepload600, %gepload599
  store <8 x double> %224, <8 x double>* %142, align 8
  %225 = fadd fast <8 x double> %gepload601, %gepload600
  store <8 x double> %225, <8 x double>* %144, align 8
  %226 = fadd fast <8 x double> %gepload602, %gepload601
  store <8 x double> %226, <8 x double>* %146, align 8
  %227 = fadd fast <8 x double> %gepload603, %gepload602
  store <8 x double> %227, <8 x double>* %148, align 8
  %228 = fadd fast <8 x double> %gepload604, %gepload603
  store <8 x double> %228, <8 x double>* %150, align 8
  %229 = fadd fast <8 x double> %gepload605, %gepload604
  store <8 x double> %229, <8 x double>* %152, align 8
  %230 = fadd fast <8 x double> %gepload606, %gepload605
  store <8 x double> %230, <8 x double>* %154, align 8
  %231 = fadd fast <8 x double> %gepload607, %gepload606
  store <8 x double> %231, <8 x double>* %156, align 8
  %232 = fadd fast <8 x double> %gepload608, %gepload607
  store <8 x double> %232, <8 x double>* %158, align 8
  %233 = fadd fast <8 x double> %gepload609, %gepload608
  store <8 x double> %233, <8 x double>* %160, align 8
  %234 = fadd fast <8 x double> %gepload610, %gepload609
  store <8 x double> %234, <8 x double>* %162, align 8
  %235 = fadd fast <8 x double> %gepload611, %gepload610
  store <8 x double> %235, <8 x double>* %164, align 8
  %236 = fadd fast <8 x double> %gepload612, %gepload611
  store <8 x double> %236, <8 x double>* %166, align 8
  %237 = fadd fast <8 x double> %gepload613, %gepload612
  store <8 x double> %237, <8 x double>* %168, align 8
  %238 = fadd fast <8 x double> %gepload614, %gepload613
  store <8 x double> %238, <8 x double>* %170, align 8
  %239 = fadd fast <8 x double> %gepload615, %gepload614
  store <8 x double> %239, <8 x double>* %172, align 8
  %240 = fadd fast <8 x double> %gepload616, %gepload615
  store <8 x double> %240, <8 x double>* %174, align 8
  %241 = fadd fast <8 x double> %gepload617, %gepload616
  store <8 x double> %241, <8 x double>* %176, align 8
  %242 = fadd fast <8 x double> %gepload617, %gepload
  store <8 x double> %242, <8 x double>* %178, align 8
  %nextivloop.399 = add nuw nsw i64 %i5.i64.0, 8
  %condloop.399.not = icmp sgt i64 %nextivloop.399, %113
  br i1 %condloop.399.not, label %afterloop.399, label %loop.399

afterloop.399:                                    ; preds = %loop.399
  br i1 %extract.0.517689, label %hir.L.555, label %loop.560.preheader

hir.L.555:                                        ; preds = %loop.560, %afterloop.399
  %nextivloop.391 = add nuw nsw i32 %i4.i32.0, 1
  %condloop.391.not = icmp eq i32 %i4.i32.0, %107
  br i1 %condloop.391.not, label %afterloop.391, label %loop.391

loop.560:                                         ; preds = %loop.560.preheader, %loop.560
  %i5.i64.1 = phi i64 [ %nextivloop.560, %loop.560 ], [ %i5.i64.1.ph, %loop.560.preheader ]
  %243 = add i64 %108, %i5.i64.1
  %244 = getelementptr inbounds double, double* %a0, i64 %243
  %gepload691 = load double, double* %244, align 8
  %245 = getelementptr inbounds double, double* %a1, i64 %243
  %gepload692 = load double, double* %245, align 8
  %246 = getelementptr inbounds double, double* %a2, i64 %243
  %gepload693 = load double, double* %246, align 8
  %247 = getelementptr inbounds double, double* %a3, i64 %243
  %gepload694 = load double, double* %247, align 8
  %248 = getelementptr inbounds double, double* %a4, i64 %243
  %gepload695 = load double, double* %248, align 8
  %249 = getelementptr inbounds double, double* %a5, i64 %243
  %gepload696 = load double, double* %249, align 8
  %250 = getelementptr inbounds double, double* %a6, i64 %243
  %gepload697 = load double, double* %250, align 8
  %251 = getelementptr inbounds double, double* %a7, i64 %243
  %gepload698 = load double, double* %251, align 8
  %252 = getelementptr inbounds double, double* %a8, i64 %243
  %gepload699 = load double, double* %252, align 8
  %253 = getelementptr inbounds double, double* %a9, i64 %243
  %gepload700 = load double, double* %253, align 8
  %254 = getelementptr inbounds double, double* %a10, i64 %243
  %gepload701 = load double, double* %254, align 8
  %255 = getelementptr inbounds double, double* %a11, i64 %243
  %gepload702 = load double, double* %255, align 8
  %256 = getelementptr inbounds double, double* %a12, i64 %243
  %gepload703 = load double, double* %256, align 8
  %257 = getelementptr inbounds double, double* %a13, i64 %243
  %gepload704 = load double, double* %257, align 8
  %258 = getelementptr inbounds double, double* %a14, i64 %243
  %gepload705 = load double, double* %258, align 8
  %259 = getelementptr inbounds double, double* %a15, i64 %243
  %gepload706 = load double, double* %259, align 8
  %260 = getelementptr inbounds double, double* %a16, i64 %243
  %gepload707 = load double, double* %260, align 8
  %261 = getelementptr inbounds double, double* %a17, i64 %243
  %gepload708 = load double, double* %261, align 8
  %262 = getelementptr inbounds double, double* %a18, i64 %243
  %gepload709 = load double, double* %262, align 8
  %263 = getelementptr inbounds double, double* %a19, i64 %243
  %gepload710 = load double, double* %263, align 8
  %264 = getelementptr inbounds double, double* %a20, i64 %243
  %gepload711 = load double, double* %264, align 8
  %265 = getelementptr inbounds double, double* %a21, i64 %243
  %gepload712 = load double, double* %265, align 8
  %266 = getelementptr inbounds double, double* %a22, i64 %243
  %gepload713 = load double, double* %266, align 8
  %267 = getelementptr inbounds double, double* %a23, i64 %243
  %gepload714 = load double, double* %267, align 8
  %268 = getelementptr inbounds double, double* %a24, i64 %243
  %gepload715 = load double, double* %268, align 8
  %269 = getelementptr inbounds double, double* %a25, i64 %243
  %gepload716 = load double, double* %269, align 8
  %270 = getelementptr inbounds double, double* %a26, i64 %243
  %gepload717 = load double, double* %270, align 8
  %271 = getelementptr inbounds double, double* %a27, i64 %243
  %gepload718 = load double, double* %271, align 8
  %272 = getelementptr inbounds double, double* %a28, i64 %243
  %gepload719 = load double, double* %272, align 8
  %273 = getelementptr inbounds double, double* %a29, i64 %243
  %gepload720 = load double, double* %273, align 8
  %274 = getelementptr inbounds double, double* %a30, i64 %243
  %gepload721 = load double, double* %274, align 8
  %275 = getelementptr inbounds double, double* %a31, i64 %243
  %gepload722 = load double, double* %275, align 8
  %276 = fadd fast double %gepload692, %gepload691
  %277 = fadd fast double %276, %gepload693
  %278 = fadd fast double %277, %gepload694
  %279 = fadd fast double %278, %gepload695
  %280 = fadd fast double %279, %gepload696
  %281 = fadd fast double %280, %gepload697
  %282 = fadd fast double %281, %gepload698
  %283 = fadd fast double %282, %gepload699
  %284 = fadd fast double %283, %gepload700
  %285 = fadd fast double %284, %gepload701
  %286 = fadd fast double %285, %gepload702
  %287 = fadd fast double %286, %gepload703
  %288 = fadd fast double %287, %gepload704
  %289 = fadd fast double %288, %gepload705
  %290 = fadd fast double %289, %gepload706
  %291 = fadd fast double %290, %gepload707
  %292 = fadd fast double %291, %gepload708
  %293 = fadd fast double %292, %gepload709
  %294 = fadd fast double %293, %gepload710
  %295 = fadd fast double %294, %gepload711
  %296 = fadd fast double %295, %gepload712
  %297 = fadd fast double %296, %gepload713
  %298 = fadd fast double %297, %gepload714
  %299 = fadd fast double %298, %gepload715
  %300 = fadd fast double %299, %gepload716
  %301 = fadd fast double %300, %gepload717
  %302 = fadd fast double %301, %gepload718
  %303 = fadd fast double %302, %gepload719
  %304 = fadd fast double %303, %gepload720
  %305 = fadd fast double %304, %gepload721
  %306 = fadd fast double %305, %gepload722
  %307 = getelementptr inbounds double, double* %c, i64 %243
  store double %306, double* %307, align 8
  store double %276, double* %244, align 8
  %308 = fadd fast double %gepload693, %gepload692
  store double %308, double* %245, align 8
  %309 = fadd fast double %gepload694, %gepload693
  store double %309, double* %246, align 8
  %310 = fadd fast double %gepload695, %gepload694
  store double %310, double* %247, align 8
  %311 = fadd fast double %gepload696, %gepload695
  store double %311, double* %248, align 8
  %312 = fadd fast double %gepload697, %gepload696
  store double %312, double* %249, align 8
  %313 = fadd fast double %gepload698, %gepload697
  store double %313, double* %250, align 8
  %314 = fadd fast double %gepload699, %gepload698
  store double %314, double* %251, align 8
  %315 = fadd fast double %gepload700, %gepload699
  store double %315, double* %252, align 8
  %316 = fadd fast double %gepload701, %gepload700
  store double %316, double* %253, align 8
  %317 = fadd fast double %gepload702, %gepload701
  store double %317, double* %254, align 8
  %318 = fadd fast double %gepload703, %gepload702
  store double %318, double* %255, align 8
  %319 = fadd fast double %gepload704, %gepload703
  store double %319, double* %256, align 8
  %320 = fadd fast double %gepload705, %gepload704
  store double %320, double* %257, align 8
  %321 = fadd fast double %gepload706, %gepload705
  store double %321, double* %258, align 8
  %322 = fadd fast double %gepload707, %gepload706
  store double %322, double* %259, align 8
  %323 = fadd fast double %gepload708, %gepload707
  store double %323, double* %260, align 8
  %324 = fadd fast double %gepload709, %gepload708
  store double %324, double* %261, align 8
  %325 = fadd fast double %gepload710, %gepload709
  store double %325, double* %262, align 8
  %326 = fadd fast double %gepload711, %gepload710
  store double %326, double* %263, align 8
  %327 = fadd fast double %gepload712, %gepload711
  store double %327, double* %264, align 8
  %328 = fadd fast double %gepload713, %gepload712
  store double %328, double* %265, align 8
  %329 = fadd fast double %gepload714, %gepload713
  store double %329, double* %266, align 8
  %330 = fadd fast double %gepload715, %gepload714
  store double %330, double* %267, align 8
  %331 = fadd fast double %gepload716, %gepload715
  store double %331, double* %268, align 8
  %332 = fadd fast double %gepload717, %gepload716
  store double %332, double* %269, align 8
  %333 = fadd fast double %gepload718, %gepload717
  store double %333, double* %270, align 8
  %334 = fadd fast double %gepload719, %gepload718
  store double %334, double* %271, align 8
  %335 = fadd fast double %gepload720, %gepload719
  store double %335, double* %272, align 8
  %336 = fadd fast double %gepload721, %gepload720
  store double %336, double* %273, align 8
  %337 = fadd fast double %gepload722, %gepload721
  store double %337, double* %274, align 8
  %338 = fadd fast double %gepload722, %gepload691
  store double %338, double* %275, align 8
  %nextivloop.560 = add nuw nsw i64 %i5.i64.1, 1
  %condloop.560.not = icmp eq i64 %i5.i64.1, %110
  br i1 %condloop.560.not, label %hir.L.555, label %loop.560

afterloop.391:                                    ; preds = %hir.L.555
  %nextivloop.189 = add nuw nsw i64 %i3.i64.0, 1
  %condloop.189.not = icmp eq i64 %i3.i64.0, %104
  br i1 %condloop.189.not, label %afterloop.189, label %loop.189

afterloop.189:                                    ; preds = %afterloop.391
  %nextivloop.389 = add nuw nsw i32 %i2.i32.0, 1
  %condloop.389.not = icmp eq i32 %i2.i32.0, %103
  br i1 %condloop.389.not, label %afterloop.389, label %loop.389

afterloop.389:                                    ; preds = %afterloop.189
  %nextivloop.188 = add nuw nsw i32 %i1.i32.0, 1
  %condloop.188.not = icmp eq i32 %i1.i32.0, %102
  br i1 %condloop.188.not, label %for.cond.cleanup, label %loop.188

loop.191:                                         ; preds = %for.cond1.preheader.lr.ph, %afterloop.192
  %i1.i32.1 = phi i32 [ %nextivloop.191, %afterloop.192 ], [ 0, %for.cond1.preheader.lr.ph ]
  br label %loop.192

loop.192:                                         ; preds = %afterloop.193, %loop.191
  %i2.i32.1 = phi i32 [ 0, %loop.191 ], [ %nextivloop.192, %afterloop.193 ]
  br label %loop.193

loop.193:                                         ; preds = %loop.193, %loop.192
  %i3.i64.1 = phi i64 [ 0, %loop.192 ], [ %nextivloop.193, %loop.193 ]
  %339 = getelementptr inbounds double, double* %a0, i64 %i3.i64.1
  %gepload787 = load double, double* %339, align 8
  %340 = getelementptr inbounds double, double* %a1, i64 %i3.i64.1
  %gepload788 = load double, double* %340, align 8
  %341 = getelementptr inbounds double, double* %a2, i64 %i3.i64.1
  %gepload789 = load double, double* %341, align 8
  %342 = getelementptr inbounds double, double* %a3, i64 %i3.i64.1
  %gepload790 = load double, double* %342, align 8
  %343 = getelementptr inbounds double, double* %a4, i64 %i3.i64.1
  %gepload791 = load double, double* %343, align 8
  %344 = getelementptr inbounds double, double* %a5, i64 %i3.i64.1
  %gepload792 = load double, double* %344, align 8
  %345 = getelementptr inbounds double, double* %a6, i64 %i3.i64.1
  %gepload793 = load double, double* %345, align 8
  %346 = getelementptr inbounds double, double* %a7, i64 %i3.i64.1
  %gepload794 = load double, double* %346, align 8
  %347 = getelementptr inbounds double, double* %a8, i64 %i3.i64.1
  %gepload795 = load double, double* %347, align 8
  %348 = getelementptr inbounds double, double* %a9, i64 %i3.i64.1
  %gepload796 = load double, double* %348, align 8
  %349 = getelementptr inbounds double, double* %a10, i64 %i3.i64.1
  %gepload797 = load double, double* %349, align 8
  %350 = getelementptr inbounds double, double* %a11, i64 %i3.i64.1
  %gepload798 = load double, double* %350, align 8
  %351 = getelementptr inbounds double, double* %a12, i64 %i3.i64.1
  %gepload799 = load double, double* %351, align 8
  %352 = getelementptr inbounds double, double* %a13, i64 %i3.i64.1
  %gepload800 = load double, double* %352, align 8
  %353 = getelementptr inbounds double, double* %a14, i64 %i3.i64.1
  %gepload801 = load double, double* %353, align 8
  %354 = getelementptr inbounds double, double* %a15, i64 %i3.i64.1
  %gepload802 = load double, double* %354, align 8
  %355 = getelementptr inbounds double, double* %a16, i64 %i3.i64.1
  %gepload803 = load double, double* %355, align 8
  %356 = getelementptr inbounds double, double* %a17, i64 %i3.i64.1
  %gepload804 = load double, double* %356, align 8
  %357 = getelementptr inbounds double, double* %a18, i64 %i3.i64.1
  %gepload805 = load double, double* %357, align 8
  %358 = getelementptr inbounds double, double* %a19, i64 %i3.i64.1
  %gepload806 = load double, double* %358, align 8
  %359 = getelementptr inbounds double, double* %a20, i64 %i3.i64.1
  %gepload807 = load double, double* %359, align 8
  %360 = getelementptr inbounds double, double* %a21, i64 %i3.i64.1
  %gepload808 = load double, double* %360, align 8
  %361 = getelementptr inbounds double, double* %a22, i64 %i3.i64.1
  %gepload809 = load double, double* %361, align 8
  %362 = getelementptr inbounds double, double* %a23, i64 %i3.i64.1
  %gepload810 = load double, double* %362, align 8
  %363 = getelementptr inbounds double, double* %a24, i64 %i3.i64.1
  %gepload811 = load double, double* %363, align 8
  %364 = getelementptr inbounds double, double* %a25, i64 %i3.i64.1
  %gepload812 = load double, double* %364, align 8
  %365 = getelementptr inbounds double, double* %a26, i64 %i3.i64.1
  %gepload813 = load double, double* %365, align 8
  %366 = getelementptr inbounds double, double* %a27, i64 %i3.i64.1
  %gepload814 = load double, double* %366, align 8
  %367 = getelementptr inbounds double, double* %a28, i64 %i3.i64.1
  %gepload815 = load double, double* %367, align 8
  %368 = getelementptr inbounds double, double* %a29, i64 %i3.i64.1
  %gepload816 = load double, double* %368, align 8
  %369 = getelementptr inbounds double, double* %a30, i64 %i3.i64.1
  %gepload817 = load double, double* %369, align 8
  %370 = getelementptr inbounds double, double* %a31, i64 %i3.i64.1
  %gepload818 = load double, double* %370, align 8
  %371 = fadd fast double %gepload788, %gepload787
  %372 = fadd fast double %371, %gepload789
  %373 = fadd fast double %372, %gepload790
  %374 = fadd fast double %373, %gepload791
  %375 = fadd fast double %374, %gepload792
  %376 = fadd fast double %375, %gepload793
  %377 = fadd fast double %376, %gepload794
  %378 = fadd fast double %377, %gepload795
  %379 = fadd fast double %378, %gepload796
  %380 = fadd fast double %379, %gepload797
  %381 = fadd fast double %380, %gepload798
  %382 = fadd fast double %381, %gepload799
  %383 = fadd fast double %382, %gepload800
  %384 = fadd fast double %383, %gepload801
  %385 = fadd fast double %384, %gepload802
  %386 = fadd fast double %385, %gepload803
  %387 = fadd fast double %386, %gepload804
  %388 = fadd fast double %387, %gepload805
  %389 = fadd fast double %388, %gepload806
  %390 = fadd fast double %389, %gepload807
  %391 = fadd fast double %390, %gepload808
  %392 = fadd fast double %391, %gepload809
  %393 = fadd fast double %392, %gepload810
  %394 = fadd fast double %393, %gepload811
  %395 = fadd fast double %394, %gepload812
  %396 = fadd fast double %395, %gepload813
  %397 = fadd fast double %396, %gepload814
  %398 = fadd fast double %397, %gepload815
  %399 = fadd fast double %398, %gepload816
  %400 = fadd fast double %399, %gepload817
  %401 = fadd fast double %400, %gepload818
  %402 = getelementptr inbounds double, double* %c, i64 %i3.i64.1
  store double %401, double* %402, align 8
  store double %371, double* %339, align 8
  %403 = fadd fast double %gepload789, %gepload788
  store double %403, double* %340, align 8
  %404 = fadd fast double %gepload790, %gepload789
  store double %404, double* %341, align 8
  %405 = fadd fast double %gepload791, %gepload790
  store double %405, double* %342, align 8
  %406 = fadd fast double %gepload792, %gepload791
  store double %406, double* %343, align 8
  %407 = fadd fast double %gepload793, %gepload792
  store double %407, double* %344, align 8
  %408 = fadd fast double %gepload794, %gepload793
  store double %408, double* %345, align 8
  %409 = fadd fast double %gepload795, %gepload794
  store double %409, double* %346, align 8
  %410 = fadd fast double %gepload796, %gepload795
  store double %410, double* %347, align 8
  %411 = fadd fast double %gepload797, %gepload796
  store double %411, double* %348, align 8
  %412 = fadd fast double %gepload798, %gepload797
  store double %412, double* %349, align 8
  %413 = fadd fast double %gepload799, %gepload798
  store double %413, double* %350, align 8
  %414 = fadd fast double %gepload800, %gepload799
  store double %414, double* %351, align 8
  %415 = fadd fast double %gepload801, %gepload800
  store double %415, double* %352, align 8
  %416 = fadd fast double %gepload802, %gepload801
  store double %416, double* %353, align 8
  %417 = fadd fast double %gepload803, %gepload802
  store double %417, double* %354, align 8
  %418 = fadd fast double %gepload804, %gepload803
  store double %418, double* %355, align 8
  %419 = fadd fast double %gepload805, %gepload804
  store double %419, double* %356, align 8
  %420 = fadd fast double %gepload806, %gepload805
  store double %420, double* %357, align 8
  %421 = fadd fast double %gepload807, %gepload806
  store double %421, double* %358, align 8
  %422 = fadd fast double %gepload808, %gepload807
  store double %422, double* %359, align 8
  %423 = fadd fast double %gepload809, %gepload808
  store double %423, double* %360, align 8
  %424 = fadd fast double %gepload810, %gepload809
  store double %424, double* %361, align 8
  %425 = fadd fast double %gepload811, %gepload810
  store double %425, double* %362, align 8
  %426 = fadd fast double %gepload812, %gepload811
  store double %426, double* %363, align 8
  %427 = fadd fast double %gepload813, %gepload812
  store double %427, double* %364, align 8
  %428 = fadd fast double %gepload814, %gepload813
  store double %428, double* %365, align 8
  %429 = fadd fast double %gepload815, %gepload814
  store double %429, double* %366, align 8
  %430 = fadd fast double %gepload816, %gepload815
  store double %430, double* %367, align 8
  %431 = fadd fast double %gepload817, %gepload816
  store double %431, double* %368, align 8
  %432 = fadd fast double %gepload818, %gepload817
  store double %432, double* %369, align 8
  %433 = fadd fast double %gepload818, %gepload787
  store double %433, double* %370, align 8
  %nextivloop.193 = add nuw nsw i64 %i3.i64.1, 1
  %condloop.193.not = icmp eq i64 %i3.i64.1, %2
  br i1 %condloop.193.not, label %afterloop.193, label %loop.193

afterloop.193:                                    ; preds = %loop.193
  %nextivloop.192 = add nuw nsw i32 %i2.i32.1, 1
  %condloop.192.not = icmp eq i32 %i2.i32.1, %102
  br i1 %condloop.192.not, label %afterloop.192, label %loop.192

afterloop.192:                                    ; preds = %afterloop.193
  %nextivloop.191 = add nuw nsw i32 %i1.i32.1, 1
  %condloop.191.not = icmp eq i32 %i1.i32.1, %102
  br i1 %condloop.191.not, label %for.cond.cleanup, label %loop.191
}

declare i64 @__intel_rtdd_indep(i8*, i64)
declare i32 @llvm.smin.i32(i32, i32)
declare i64 @llvm.smin.i64(i64, i64)
