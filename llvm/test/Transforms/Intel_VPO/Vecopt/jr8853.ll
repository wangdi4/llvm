; Regression test from CMPLRLLVM-8853. Test that VPlanDriver does not cause a compilation fail.
; RUN: opt -S -VPlanDriver %s | FileCheck %s
; RUN: opt -S -passes="vplan-driver" %s | FileCheck %s
; CHECK-LABEL: vector.body
; ModuleID = 'main'
%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare double @_Z4sqrtd(double) local_unnamed_addr #2

; Function Attrs: nounwind
declare double @_Z3expd(double) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #3

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: nounwind
define void @_ZGVdN8uuuuuuuuuuuuuuuuuuu_TSZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_E9binominal(double addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, double addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, double addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, i32, double, double, double addrspace(1)*, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval, %"class.cl::sycl::range"* byval) local_unnamed_addr #0 {
  %20 = alloca [2520 x double], align 8
  %21 = call i64 @_Z13get_global_idj(i32 0) #3
  br label %simd.begin.region

simd.begin.region:                                ; preds = %19
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(double addrspace(1)* %0, %"class.cl::sycl::range"* %1, %"class.cl::sycl::range"* %2, %"class.cl::sycl::range"* %3, double addrspace(1)* %4, %"class.cl::sycl::range"* %5, %"class.cl::sycl::range"* %6, %"class.cl::sycl::range"* %7, double addrspace(1)* %8, %"class.cl::sycl::range"* %9, %"class.cl::sycl::range"* %10, %"class.cl::sycl::range"* %11, i32 %12, double %13, double %14, double addrspace(1)* %15, %"class.cl::sycl::range"* %16, %"class.cl::sycl::range"* %17, %"class.cl::sycl::range"* %18), "QUAL.OMP.PRIVATE"([2520 x double]* %20), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %22 = sext i32 %index to i64
  %add = add nuw i64 %22, %21
  %23 = bitcast [2520 x double]* %20 to i8*
  call void @llvm.lifetime.start.p0i8(i64 20160, i8* nonnull %23) #2
  %24 = getelementptr inbounds double, double addrspace(1)* %0, i64 %add
  %25 = load double, double addrspace(1)* %24, align 8
  %26 = getelementptr inbounds double, double addrspace(1)* %4, i64 %add
  %27 = load double, double addrspace(1)* %26, align 8
  %28 = getelementptr inbounds double, double addrspace(1)* %8, i64 %add
  %29 = load double, double addrspace(1)* %28, align 8
  %30 = sitofp i32 %12 to double
  %31 = fdiv double %29, %30
  %32 = call double @_Z4sqrtd(double %31) #2
  %33 = fmul double %32, %13
  %34 = fmul double %31, %14
  %35 = call double @_Z3expd(double %34) #2
  %36 = fsub double -0.000000e+00, %34
  %37 = call double @_Z3expd(double %36) #2
  %38 = call double @_Z3expd(double %33) #2
  %39 = fsub double -0.000000e+00, %33
  %40 = call double @_Z3expd(double %39) #2
  %41 = fsub double %35, %40
  %42 = fsub double %38, %40
  %43 = fdiv double %41, %42
  %44 = fsub double 1.000000e+00, %43
  %45 = fmul double %37, %43
  %46 = fmul double %37, %44
  %47 = fmul double %33, 2.000000e+00
  %48 = icmp slt i32 %12, 0
  br i1 %48, label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit, label %.lr.ph10.i

.lr.ph10.i:                                       ; preds = %simd.loop
  %49 = sub nsw i32 0, %12
  %50 = sitofp i32 %49 to double
  %51 = fmul double %33, %50
  br label %55

.preheader1.i:                                    ; preds = %55
  %.lcssa = phi i64 [ %62, %55 ]
  %52 = icmp sgt i32 %12, 0
  br i1 %52, label %.preheader.lr.ph.i, label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit

.preheader.lr.ph.i:                               ; preds = %.preheader1.i
  %53 = add nsw i64 %.lcssa, -1
  %xtraiter16.i = and i64 %.lcssa, 3
  %54 = icmp ult i64 %53, 3
  br i1 %54, label %._crit_edge6.unr-lcssa.i, label %.preheader.lr.ph.new.i

.preheader.lr.ph.new.i:                           ; preds = %.preheader.lr.ph.i
  %unroll_iter21.i = sub nsw i64 %.lcssa, %xtraiter16.i
  br label %.lr.ph.i

55:                                               ; preds = %55, %.lr.ph10.i
  %indvars.iv14.i = phi i64 [ %indvars.iv.next15.i, %55 ], [ 0, %.lr.ph10.i ]
  %.037.i = phi double [ %61, %55 ], [ %51, %.lr.ph10.i ]
  %56 = call double @_Z3expd(double %.037.i) #2
  %57 = fmul double %25, %56
  %58 = fsub double %57, %27
  %59 = fcmp ogt double %58, 0.000000e+00
  %..i = select i1 %59, double %58, double 0.000000e+00
  %60 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv14.i
  store double %..i, double* %60, align 8
  %61 = fadd double %47, %.037.i
  %indvars.iv.next15.i = add nuw nsw i64 %indvars.iv14.i, 1
  %62 = sext i32 %12 to i64
  %63 = icmp slt i64 %indvars.iv14.i, %62
  br i1 %63, label %55, label %.preheader1.i

.lr.ph.i:                                         ; preds = %._crit_edge.3.i, %.preheader.lr.ph.new.i
  %indvar.i = phi i64 [ 0, %.preheader.lr.ph.new.i ], [ %indvar.next.3.i, %._crit_edge.3.i ]
  %niter22.i = phi i64 [ %unroll_iter21.i, %.preheader.lr.ph.new.i ], [ %niter22.nsub.3.i, %._crit_edge.3.i ]
  %64 = sub i64 %.lcssa, %indvar.i
  %65 = sub i64 %53, %indvar.i
  %xtraiter.i = and i64 %64, 7
  %66 = icmp ult i64 %65, 7
  br i1 %66, label %._crit_edge.unr-lcssa.i, label %.lr.ph.new.i

.lr.ph.new.i:                                     ; preds = %.lr.ph.i
  %unroll_iter.i = sub i64 %64, %xtraiter.i
  %.phi.trans.insert.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 0
  %.pre.i = load double, double* %.phi.trans.insert.i, align 8
  br label %67

67:                                               ; preds = %67, %.lr.ph.new.i
  %68 = phi double [ %.pre.i, %.lr.ph.new.i ], [ %106, %67 ]
  %indvars.iv.i = phi i64 [ 0, %.lr.ph.new.i ], [ %indvars.iv.next.7.i, %67 ]
  %niter.i = phi i64 [ %unroll_iter.i, %.lr.ph.new.i ], [ %niter.nsub.7.i, %67 ]
  %indvars.iv.next.i = or i64 %indvars.iv.i, 1
  %69 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.i
  %70 = load double, double* %69, align 8
  %71 = fmul double %45, %70
  %72 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.i
  %73 = fmul double %46, %68
  %74 = fadd double %71, %73
  store double %74, double* %72, align 8
  %indvars.iv.next.1.i = or i64 %indvars.iv.i, 2
  %75 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.1.i
  %76 = load double, double* %75, align 8
  %77 = fmul double %45, %76
  %78 = fmul double %46, %70
  %79 = fadd double %77, %78
  store double %79, double* %69, align 8
  %indvars.iv.next.2.i = or i64 %indvars.iv.i, 3
  %80 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.2.i
  %81 = load double, double* %80, align 8
  %82 = fmul double %45, %81
  %83 = fmul double %46, %76
  %84 = fadd double %82, %83
  store double %84, double* %75, align 8
  %indvars.iv.next.3.i = or i64 %indvars.iv.i, 4
  %85 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.3.i
  %86 = load double, double* %85, align 8
  %87 = fmul double %45, %86
  %88 = fmul double %46, %81
  %89 = fadd double %87, %88
  store double %89, double* %80, align 8
  %indvars.iv.next.4.i = or i64 %indvars.iv.i, 5
  %90 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.4.i
  %91 = load double, double* %90, align 8
  %92 = fmul double %45, %91
  %93 = fmul double %46, %86
  %94 = fadd double %92, %93
  store double %94, double* %85, align 8
  %indvars.iv.next.5.i = or i64 %indvars.iv.i, 6
  %95 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.5.i
  %96 = load double, double* %95, align 8
  %97 = fmul double %45, %96
  %98 = fmul double %46, %91
  %99 = fadd double %97, %98
  store double %99, double* %90, align 8
  %indvars.iv.next.6.i = or i64 %indvars.iv.i, 7
  %100 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.6.i
  %101 = load double, double* %100, align 8
  %102 = fmul double %45, %101
  %103 = fmul double %46, %96
  %104 = fadd double %102, %103
  store double %104, double* %95, align 8
  %indvars.iv.next.7.i = add nuw nsw i64 %indvars.iv.i, 8
  %105 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.7.i
  %106 = load double, double* %105, align 8
  %107 = fmul double %45, %106
  %108 = fmul double %46, %101
  %109 = fadd double %107, %108
  store double %109, double* %100, align 8
  %niter.nsub.7.i = add i64 %niter.i, -8
  %niter.ncmp.7.i = icmp eq i64 %niter.nsub.7.i, 0
  br i1 %niter.ncmp.7.i, label %._crit_edge.unr-lcssa.i.loopexit, label %67

._crit_edge.unr-lcssa.i.loopexit:                 ; preds = %67
  %indvars.iv.next.7.i.lcssa = phi i64 [ %indvars.iv.next.7.i, %67 ]
  br label %._crit_edge.unr-lcssa.i

._crit_edge.unr-lcssa.i:                          ; preds = %._crit_edge.unr-lcssa.i.loopexit, %.lr.ph.i
  %indvars.iv.unr.i = phi i64 [ 0, %.lr.ph.i ], [ %indvars.iv.next.7.i.lcssa, %._crit_edge.unr-lcssa.i.loopexit ]
  %lcmp.mod.i = icmp eq i64 %xtraiter.i, 0
  br i1 %lcmp.mod.i, label %.lr.ph.1.i, label %.epil.preheader.i

.epil.preheader.i:                                ; preds = %._crit_edge.unr-lcssa.i
  %.phi.trans.insert32.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.unr.i
  %.pre33.i = load double, double* %.phi.trans.insert32.i, align 8
  br label %110

110:                                              ; preds = %110, %.epil.preheader.i
  %111 = phi double [ %.pre33.i, %.epil.preheader.i ], [ %113, %110 ]
  %indvars.iv.epil.i = phi i64 [ %indvars.iv.unr.i, %.epil.preheader.i ], [ %indvars.iv.next.epil.i, %110 ]
  %epil.iter.i = phi i64 [ %xtraiter.i, %.epil.preheader.i ], [ %epil.iter.sub.i, %110 ]
  %indvars.iv.next.epil.i = add nuw nsw i64 %indvars.iv.epil.i, 1
  %112 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.epil.i
  %113 = load double, double* %112, align 8
  %114 = fmul double %45, %113
  %115 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.epil.i
  %116 = fmul double %46, %111
  %117 = fadd double %114, %116
  store double %117, double* %115, align 8
  %epil.iter.sub.i = add i64 %epil.iter.i, -1
  %epil.iter.cmp.i = icmp eq i64 %epil.iter.sub.i, 0
  br i1 %epil.iter.cmp.i, label %.lr.ph.1.i.loopexit, label %110

._crit_edge6.unr-lcssa.i.loopexit:                ; preds = %._crit_edge.3.i
  %indvar.next.3.i.lcssa = phi i64 [ %indvar.next.3.i, %._crit_edge.3.i ]
  br label %._crit_edge6.unr-lcssa.i

._crit_edge6.unr-lcssa.i:                         ; preds = %._crit_edge6.unr-lcssa.i.loopexit, %.preheader.lr.ph.i
  %indvar.unr.i = phi i64 [ 0, %.preheader.lr.ph.i ], [ %indvar.next.3.i.lcssa, %._crit_edge6.unr-lcssa.i.loopexit ]
  %lcmp.mod20.i = icmp eq i64 %xtraiter16.i, 0
  br i1 %lcmp.mod20.i, label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit, label %.lr.ph.epil.i.preheader

.lr.ph.epil.i.preheader:                          ; preds = %._crit_edge6.unr-lcssa.i
  br label %.lr.ph.epil.i

.lr.ph.epil.i:                                    ; preds = %.lr.ph.epil.i.preheader, %._crit_edge.epil.i
  %indvar.epil.i = phi i64 [ %indvar.next.epil.i, %._crit_edge.epil.i ], [ %indvar.unr.i, %.lr.ph.epil.i.preheader ]
  %epil.iter19.i = phi i64 [ %epil.iter19.sub.i, %._crit_edge.epil.i ], [ %xtraiter16.i, %.lr.ph.epil.i.preheader ]
  %118 = sub i64 %.lcssa, %indvar.epil.i
  %119 = sub i64 %53, %indvar.epil.i
  %xtraiter.epil.i = and i64 %118, 7
  %120 = icmp ult i64 %119, 7
  br i1 %120, label %._crit_edge.unr-lcssa.epil.i, label %.lr.ph.new.epil.i

.lr.ph.new.epil.i:                                ; preds = %.lr.ph.epil.i
  %unroll_iter.epil.i = sub i64 %118, %xtraiter.epil.i
  %.phi.trans.insert46.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 0
  %.pre47.i = load double, double* %.phi.trans.insert46.i, align 8
  br label %121

121:                                              ; preds = %121, %.lr.ph.new.epil.i
  %122 = phi double [ %.pre47.i, %.lr.ph.new.epil.i ], [ %160, %121 ]
  %indvars.iv.epil17.i = phi i64 [ 0, %.lr.ph.new.epil.i ], [ %indvars.iv.next.7.epil.i, %121 ]
  %niter.epil.i = phi i64 [ %unroll_iter.epil.i, %.lr.ph.new.epil.i ], [ %niter.nsub.7.epil.i, %121 ]
  %indvars.iv.next.epil18.i = or i64 %indvars.iv.epil17.i, 1
  %123 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.epil18.i
  %124 = load double, double* %123, align 8
  %125 = fmul double %45, %124
  %126 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.epil17.i
  %127 = fmul double %46, %122
  %128 = fadd double %125, %127
  store double %128, double* %126, align 8
  %indvars.iv.next.1.epil.i = or i64 %indvars.iv.epil17.i, 2
  %129 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.1.epil.i
  %130 = load double, double* %129, align 8
  %131 = fmul double %45, %130
  %132 = fmul double %46, %124
  %133 = fadd double %131, %132
  store double %133, double* %123, align 8
  %indvars.iv.next.2.epil.i = or i64 %indvars.iv.epil17.i, 3
  %134 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.2.epil.i
  %135 = load double, double* %134, align 8
  %136 = fmul double %45, %135
  %137 = fmul double %46, %130
  %138 = fadd double %136, %137
  store double %138, double* %129, align 8
  %indvars.iv.next.3.epil.i = or i64 %indvars.iv.epil17.i, 4
  %139 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.3.epil.i
  %140 = load double, double* %139, align 8
  %141 = fmul double %45, %140
  %142 = fmul double %46, %135
  %143 = fadd double %141, %142
  store double %143, double* %134, align 8
  %indvars.iv.next.4.epil.i = or i64 %indvars.iv.epil17.i, 5
  %144 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.4.epil.i
  %145 = load double, double* %144, align 8
  %146 = fmul double %45, %145
  %147 = fmul double %46, %140
  %148 = fadd double %146, %147
  store double %148, double* %139, align 8
  %indvars.iv.next.5.epil.i = or i64 %indvars.iv.epil17.i, 6
  %149 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.5.epil.i
  %150 = load double, double* %149, align 8
  %151 = fmul double %45, %150
  %152 = fmul double %46, %145
  %153 = fadd double %151, %152
  store double %153, double* %144, align 8
  %indvars.iv.next.6.epil.i = or i64 %indvars.iv.epil17.i, 7
  %154 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.6.epil.i
  %155 = load double, double* %154, align 8
  %156 = fmul double %45, %155
  %157 = fmul double %46, %150
  %158 = fadd double %156, %157
  store double %158, double* %149, align 8
  %indvars.iv.next.7.epil.i = add nuw nsw i64 %indvars.iv.epil17.i, 8
  %159 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.7.epil.i
  %160 = load double, double* %159, align 8
  %161 = fmul double %45, %160
  %162 = fmul double %46, %155
  %163 = fadd double %161, %162
  store double %163, double* %154, align 8
  %niter.nsub.7.epil.i = add i64 %niter.epil.i, -8
  %niter.ncmp.7.epil.i = icmp eq i64 %niter.nsub.7.epil.i, 0
  br i1 %niter.ncmp.7.epil.i, label %._crit_edge.unr-lcssa.epil.i.loopexit, label %121

._crit_edge.unr-lcssa.epil.i.loopexit:            ; preds = %121
  %indvars.iv.next.7.epil.i.lcssa = phi i64 [ %indvars.iv.next.7.epil.i, %121 ]
  br label %._crit_edge.unr-lcssa.epil.i

._crit_edge.unr-lcssa.epil.i:                     ; preds = %._crit_edge.unr-lcssa.epil.i.loopexit, %.lr.ph.epil.i
  %indvars.iv.unr.epil.i = phi i64 [ 0, %.lr.ph.epil.i ], [ %indvars.iv.next.7.epil.i.lcssa, %._crit_edge.unr-lcssa.epil.i.loopexit ]
  %lcmp.mod.epil.i = icmp eq i64 %xtraiter.epil.i, 0
  br i1 %lcmp.mod.epil.i, label %._crit_edge.epil.i, label %.epil.preheader.epil.i

.epil.preheader.epil.i:                           ; preds = %._crit_edge.unr-lcssa.epil.i
  %.phi.trans.insert48.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.unr.epil.i
  %.pre49.i = load double, double* %.phi.trans.insert48.i, align 8
  br label %164

164:                                              ; preds = %164, %.epil.preheader.epil.i
  %165 = phi double [ %.pre49.i, %.epil.preheader.epil.i ], [ %167, %164 ]
  %indvars.iv.epil.epil.i = phi i64 [ %indvars.iv.unr.epil.i, %.epil.preheader.epil.i ], [ %indvars.iv.next.epil.epil.i, %164 ]
  %epil.iter.epil.i = phi i64 [ %xtraiter.epil.i, %.epil.preheader.epil.i ], [ %epil.iter.sub.epil.i, %164 ]
  %indvars.iv.next.epil.epil.i = add nuw nsw i64 %indvars.iv.epil.epil.i, 1
  %166 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.epil.epil.i
  %167 = load double, double* %166, align 8
  %168 = fmul double %45, %167
  %169 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.epil.epil.i
  %170 = fmul double %46, %165
  %171 = fadd double %168, %170
  store double %171, double* %169, align 8
  %epil.iter.sub.epil.i = add i64 %epil.iter.epil.i, -1
  %epil.iter.cmp.epil.i = icmp eq i64 %epil.iter.sub.epil.i, 0
  br i1 %epil.iter.cmp.epil.i, label %._crit_edge.epil.i.loopexit, label %164

._crit_edge.epil.i.loopexit:                      ; preds = %164
  br label %._crit_edge.epil.i

._crit_edge.epil.i:                               ; preds = %._crit_edge.epil.i.loopexit, %._crit_edge.unr-lcssa.epil.i
  %indvar.next.epil.i = add i64 %indvar.epil.i, 1
  %epil.iter19.sub.i = add i64 %epil.iter19.i, -1
  %epil.iter19.cmp.i = icmp eq i64 %epil.iter19.sub.i, 0
  br i1 %epil.iter19.cmp.i, label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit.loopexit, label %.lr.ph.epil.i

.lr.ph.1.i.loopexit:                              ; preds = %110
  br label %.lr.ph.1.i

.lr.ph.1.i:                                       ; preds = %.lr.ph.1.i.loopexit, %._crit_edge.unr-lcssa.i
  %172 = xor i64 %indvar.i, -1
  %173 = add i64 %.lcssa, %172
  %174 = add i64 %53, %172
  %xtraiter.1.i = and i64 %173, 7
  %175 = icmp ult i64 %174, 7
  br i1 %175, label %._crit_edge.unr-lcssa.1.i, label %.lr.ph.new.1.i

.lr.ph.new.1.i:                                   ; preds = %.lr.ph.1.i
  %unroll_iter.1.i = sub i64 %173, %xtraiter.1.i
  %.phi.trans.insert34.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 0
  %.pre35.i = load double, double* %.phi.trans.insert34.i, align 8
  br label %176

176:                                              ; preds = %176, %.lr.ph.new.1.i
  %177 = phi double [ %.pre35.i, %.lr.ph.new.1.i ], [ %215, %176 ]
  %indvars.iv.1.i = phi i64 [ 0, %.lr.ph.new.1.i ], [ %indvars.iv.next.7.1.i, %176 ]
  %niter.1.i = phi i64 [ %unroll_iter.1.i, %.lr.ph.new.1.i ], [ %niter.nsub.7.1.i, %176 ]
  %indvars.iv.next.123.i = or i64 %indvars.iv.1.i, 1
  %178 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.123.i
  %179 = load double, double* %178, align 8
  %180 = fmul double %45, %179
  %181 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.1.i
  %182 = fmul double %46, %177
  %183 = fadd double %180, %182
  store double %183, double* %181, align 8
  %indvars.iv.next.1.1.i = or i64 %indvars.iv.1.i, 2
  %184 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.1.1.i
  %185 = load double, double* %184, align 8
  %186 = fmul double %45, %185
  %187 = fmul double %46, %179
  %188 = fadd double %186, %187
  store double %188, double* %178, align 8
  %indvars.iv.next.2.1.i = or i64 %indvars.iv.1.i, 3
  %189 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.2.1.i
  %190 = load double, double* %189, align 8
  %191 = fmul double %45, %190
  %192 = fmul double %46, %185
  %193 = fadd double %191, %192
  store double %193, double* %184, align 8
  %indvars.iv.next.3.1.i = or i64 %indvars.iv.1.i, 4
  %194 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.3.1.i
  %195 = load double, double* %194, align 8
  %196 = fmul double %45, %195
  %197 = fmul double %46, %190
  %198 = fadd double %196, %197
  store double %198, double* %189, align 8
  %indvars.iv.next.4.1.i = or i64 %indvars.iv.1.i, 5
  %199 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.4.1.i
  %200 = load double, double* %199, align 8
  %201 = fmul double %45, %200
  %202 = fmul double %46, %195
  %203 = fadd double %201, %202
  store double %203, double* %194, align 8
  %indvars.iv.next.5.1.i = or i64 %indvars.iv.1.i, 6
  %204 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.5.1.i
  %205 = load double, double* %204, align 8
  %206 = fmul double %45, %205
  %207 = fmul double %46, %200
  %208 = fadd double %206, %207
  store double %208, double* %199, align 8
  %indvars.iv.next.6.1.i = or i64 %indvars.iv.1.i, 7
  %209 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.6.1.i
  %210 = load double, double* %209, align 8
  %211 = fmul double %45, %210
  %212 = fmul double %46, %205
  %213 = fadd double %211, %212
  store double %213, double* %204, align 8
  %indvars.iv.next.7.1.i = add nuw nsw i64 %indvars.iv.1.i, 8
  %214 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.7.1.i
  %215 = load double, double* %214, align 8
  %216 = fmul double %45, %215
  %217 = fmul double %46, %210
  %218 = fadd double %216, %217
  store double %218, double* %209, align 8
  %niter.nsub.7.1.i = add i64 %niter.1.i, -8
  %niter.ncmp.7.1.i = icmp eq i64 %niter.nsub.7.1.i, 0
  br i1 %niter.ncmp.7.1.i, label %._crit_edge.unr-lcssa.1.i.loopexit, label %176

._crit_edge.unr-lcssa.1.i.loopexit:               ; preds = %176
  %indvars.iv.next.7.1.i.lcssa = phi i64 [ %indvars.iv.next.7.1.i, %176 ]
  br label %._crit_edge.unr-lcssa.1.i

._crit_edge.unr-lcssa.1.i:                        ; preds = %._crit_edge.unr-lcssa.1.i.loopexit, %.lr.ph.1.i
  %indvars.iv.unr.1.i = phi i64 [ 0, %.lr.ph.1.i ], [ %indvars.iv.next.7.1.i.lcssa, %._crit_edge.unr-lcssa.1.i.loopexit ]
  %lcmp.mod.1.i = icmp eq i64 %xtraiter.1.i, 0
  br i1 %lcmp.mod.1.i, label %.lr.ph.2.i, label %.epil.preheader.1.i

.epil.preheader.1.i:                              ; preds = %._crit_edge.unr-lcssa.1.i
  %.phi.trans.insert36.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.unr.1.i
  %.pre37.i = load double, double* %.phi.trans.insert36.i, align 8
  br label %219

219:                                              ; preds = %219, %.epil.preheader.1.i
  %220 = phi double [ %.pre37.i, %.epil.preheader.1.i ], [ %222, %219 ]
  %indvars.iv.epil.1.i = phi i64 [ %indvars.iv.unr.1.i, %.epil.preheader.1.i ], [ %indvars.iv.next.epil.1.i, %219 ]
  %epil.iter.1.i = phi i64 [ %xtraiter.1.i, %.epil.preheader.1.i ], [ %epil.iter.sub.1.i, %219 ]
  %indvars.iv.next.epil.1.i = add nuw nsw i64 %indvars.iv.epil.1.i, 1
  %221 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.epil.1.i
  %222 = load double, double* %221, align 8
  %223 = fmul double %45, %222
  %224 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.epil.1.i
  %225 = fmul double %46, %220
  %226 = fadd double %223, %225
  store double %226, double* %224, align 8
  %epil.iter.sub.1.i = add i64 %epil.iter.1.i, -1
  %epil.iter.cmp.1.i = icmp eq i64 %epil.iter.sub.1.i, 0
  br i1 %epil.iter.cmp.1.i, label %.lr.ph.2.i.loopexit, label %219

.lr.ph.2.i.loopexit:                              ; preds = %219
  br label %.lr.ph.2.i

.lr.ph.2.i:                                       ; preds = %.lr.ph.2.i.loopexit, %._crit_edge.unr-lcssa.1.i
  %227 = sub nuw nsw i64 -2, %indvar.i
  %228 = add i64 %227, %.lcssa
  %229 = add i64 %53, %227
  %xtraiter.2.i = and i64 %228, 7
  %230 = icmp ult i64 %229, 7
  br i1 %230, label %._crit_edge.unr-lcssa.2.i, label %.lr.ph.new.2.i

.lr.ph.new.2.i:                                   ; preds = %.lr.ph.2.i
  %unroll_iter.2.i = sub i64 %228, %xtraiter.2.i
  %.phi.trans.insert38.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 0
  %.pre39.i = load double, double* %.phi.trans.insert38.i, align 8
  br label %231

231:                                              ; preds = %231, %.lr.ph.new.2.i
  %232 = phi double [ %.pre39.i, %.lr.ph.new.2.i ], [ %270, %231 ]
  %indvars.iv.2.i = phi i64 [ 0, %.lr.ph.new.2.i ], [ %indvars.iv.next.7.2.i, %231 ]
  %niter.2.i = phi i64 [ %unroll_iter.2.i, %.lr.ph.new.2.i ], [ %niter.nsub.7.2.i, %231 ]
  %indvars.iv.next.225.i = or i64 %indvars.iv.2.i, 1
  %233 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.225.i
  %234 = load double, double* %233, align 8
  %235 = fmul double %45, %234
  %236 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.2.i
  %237 = fmul double %46, %232
  %238 = fadd double %235, %237
  store double %238, double* %236, align 8
  %indvars.iv.next.1.2.i = or i64 %indvars.iv.2.i, 2
  %239 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.1.2.i
  %240 = load double, double* %239, align 8
  %241 = fmul double %45, %240
  %242 = fmul double %46, %234
  %243 = fadd double %241, %242
  store double %243, double* %233, align 8
  %indvars.iv.next.2.2.i = or i64 %indvars.iv.2.i, 3
  %244 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.2.2.i
  %245 = load double, double* %244, align 8
  %246 = fmul double %45, %245
  %247 = fmul double %46, %240
  %248 = fadd double %246, %247
  store double %248, double* %239, align 8
  %indvars.iv.next.3.2.i = or i64 %indvars.iv.2.i, 4
  %249 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.3.2.i
  %250 = load double, double* %249, align 8
  %251 = fmul double %45, %250
  %252 = fmul double %46, %245
  %253 = fadd double %251, %252
  store double %253, double* %244, align 8
  %indvars.iv.next.4.2.i = or i64 %indvars.iv.2.i, 5
  %254 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.4.2.i
  %255 = load double, double* %254, align 8
  %256 = fmul double %45, %255
  %257 = fmul double %46, %250
  %258 = fadd double %256, %257
  store double %258, double* %249, align 8
  %indvars.iv.next.5.2.i = or i64 %indvars.iv.2.i, 6
  %259 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.5.2.i
  %260 = load double, double* %259, align 8
  %261 = fmul double %45, %260
  %262 = fmul double %46, %255
  %263 = fadd double %261, %262
  store double %263, double* %254, align 8
  %indvars.iv.next.6.2.i = or i64 %indvars.iv.2.i, 7
  %264 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.6.2.i
  %265 = load double, double* %264, align 8
  %266 = fmul double %45, %265
  %267 = fmul double %46, %260
  %268 = fadd double %266, %267
  store double %268, double* %259, align 8
  %indvars.iv.next.7.2.i = add nuw nsw i64 %indvars.iv.2.i, 8
  %269 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.7.2.i
  %270 = load double, double* %269, align 8
  %271 = fmul double %45, %270
  %272 = fmul double %46, %265
  %273 = fadd double %271, %272
  store double %273, double* %264, align 8
  %niter.nsub.7.2.i = add i64 %niter.2.i, -8
  %niter.ncmp.7.2.i = icmp eq i64 %niter.nsub.7.2.i, 0
  br i1 %niter.ncmp.7.2.i, label %._crit_edge.unr-lcssa.2.i.loopexit, label %231

._crit_edge.unr-lcssa.2.i.loopexit:               ; preds = %231
  %indvars.iv.next.7.2.i.lcssa = phi i64 [ %indvars.iv.next.7.2.i, %231 ]
  br label %._crit_edge.unr-lcssa.2.i

._crit_edge.unr-lcssa.2.i:                        ; preds = %._crit_edge.unr-lcssa.2.i.loopexit, %.lr.ph.2.i
  %indvars.iv.unr.2.i = phi i64 [ 0, %.lr.ph.2.i ], [ %indvars.iv.next.7.2.i.lcssa, %._crit_edge.unr-lcssa.2.i.loopexit ]
  %lcmp.mod.2.i = icmp eq i64 %xtraiter.2.i, 0
  br i1 %lcmp.mod.2.i, label %.lr.ph.3.i, label %.epil.preheader.2.i

.epil.preheader.2.i:                              ; preds = %._crit_edge.unr-lcssa.2.i
  %.phi.trans.insert40.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.unr.2.i
  %.pre41.i = load double, double* %.phi.trans.insert40.i, align 8
  br label %274

274:                                              ; preds = %274, %.epil.preheader.2.i
  %275 = phi double [ %.pre41.i, %.epil.preheader.2.i ], [ %277, %274 ]
  %indvars.iv.epil.2.i = phi i64 [ %indvars.iv.unr.2.i, %.epil.preheader.2.i ], [ %indvars.iv.next.epil.2.i, %274 ]
  %epil.iter.2.i = phi i64 [ %xtraiter.2.i, %.epil.preheader.2.i ], [ %epil.iter.sub.2.i, %274 ]
  %indvars.iv.next.epil.2.i = add nuw nsw i64 %indvars.iv.epil.2.i, 1
  %276 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.epil.2.i
  %277 = load double, double* %276, align 8
  %278 = fmul double %45, %277
  %279 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.epil.2.i
  %280 = fmul double %46, %275
  %281 = fadd double %278, %280
  store double %281, double* %279, align 8
  %epil.iter.sub.2.i = add i64 %epil.iter.2.i, -1
  %epil.iter.cmp.2.i = icmp eq i64 %epil.iter.sub.2.i, 0
  br i1 %epil.iter.cmp.2.i, label %.lr.ph.3.i.loopexit, label %274

.lr.ph.3.i.loopexit:                              ; preds = %274
  br label %.lr.ph.3.i

.lr.ph.3.i:                                       ; preds = %.lr.ph.3.i.loopexit, %._crit_edge.unr-lcssa.2.i
  %282 = sub nuw nsw i64 -3, %indvar.i
  %283 = add i64 %282, %.lcssa
  %284 = add i64 %53, %282
  %xtraiter.3.i = and i64 %283, 7
  %285 = icmp ult i64 %284, 7
  br i1 %285, label %._crit_edge.unr-lcssa.3.i, label %.lr.ph.new.3.i

.lr.ph.new.3.i:                                   ; preds = %.lr.ph.3.i
  %unroll_iter.3.i = sub i64 %283, %xtraiter.3.i
  %.phi.trans.insert42.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 0
  %.pre43.i = load double, double* %.phi.trans.insert42.i, align 8
  br label %286

286:                                              ; preds = %286, %.lr.ph.new.3.i
  %287 = phi double [ %.pre43.i, %.lr.ph.new.3.i ], [ %325, %286 ]
  %indvars.iv.3.i = phi i64 [ 0, %.lr.ph.new.3.i ], [ %indvars.iv.next.7.3.i, %286 ]
  %niter.3.i = phi i64 [ %unroll_iter.3.i, %.lr.ph.new.3.i ], [ %niter.nsub.7.3.i, %286 ]
  %indvars.iv.next.327.i = or i64 %indvars.iv.3.i, 1
  %288 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.327.i
  %289 = load double, double* %288, align 8
  %290 = fmul double %45, %289
  %291 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.3.i
  %292 = fmul double %46, %287
  %293 = fadd double %290, %292
  store double %293, double* %291, align 8
  %indvars.iv.next.1.3.i = or i64 %indvars.iv.3.i, 2
  %294 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.1.3.i
  %295 = load double, double* %294, align 8
  %296 = fmul double %45, %295
  %297 = fmul double %46, %289
  %298 = fadd double %296, %297
  store double %298, double* %288, align 8
  %indvars.iv.next.2.3.i = or i64 %indvars.iv.3.i, 3
  %299 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.2.3.i
  %300 = load double, double* %299, align 8
  %301 = fmul double %45, %300
  %302 = fmul double %46, %295
  %303 = fadd double %301, %302
  store double %303, double* %294, align 8
  %indvars.iv.next.3.3.i = or i64 %indvars.iv.3.i, 4
  %304 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.3.3.i
  %305 = load double, double* %304, align 8
  %306 = fmul double %45, %305
  %307 = fmul double %46, %300
  %308 = fadd double %306, %307
  store double %308, double* %299, align 8
  %indvars.iv.next.4.3.i = or i64 %indvars.iv.3.i, 5
  %309 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.4.3.i
  %310 = load double, double* %309, align 8
  %311 = fmul double %45, %310
  %312 = fmul double %46, %305
  %313 = fadd double %311, %312
  store double %313, double* %304, align 8
  %indvars.iv.next.5.3.i = or i64 %indvars.iv.3.i, 6
  %314 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.5.3.i
  %315 = load double, double* %314, align 8
  %316 = fmul double %45, %315
  %317 = fmul double %46, %310
  %318 = fadd double %316, %317
  store double %318, double* %309, align 8
  %indvars.iv.next.6.3.i = or i64 %indvars.iv.3.i, 7
  %319 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.6.3.i
  %320 = load double, double* %319, align 8
  %321 = fmul double %45, %320
  %322 = fmul double %46, %315
  %323 = fadd double %321, %322
  store double %323, double* %314, align 8
  %indvars.iv.next.7.3.i = add nuw nsw i64 %indvars.iv.3.i, 8
  %324 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.7.3.i
  %325 = load double, double* %324, align 8
  %326 = fmul double %45, %325
  %327 = fmul double %46, %320
  %328 = fadd double %326, %327
  store double %328, double* %319, align 8
  %niter.nsub.7.3.i = add i64 %niter.3.i, -8
  %niter.ncmp.7.3.i = icmp eq i64 %niter.nsub.7.3.i, 0
  br i1 %niter.ncmp.7.3.i, label %._crit_edge.unr-lcssa.3.i.loopexit, label %286

._crit_edge.unr-lcssa.3.i.loopexit:               ; preds = %286
  %indvars.iv.next.7.3.i.lcssa = phi i64 [ %indvars.iv.next.7.3.i, %286 ]
  br label %._crit_edge.unr-lcssa.3.i

._crit_edge.unr-lcssa.3.i:                        ; preds = %._crit_edge.unr-lcssa.3.i.loopexit, %.lr.ph.3.i
  %indvars.iv.unr.3.i = phi i64 [ 0, %.lr.ph.3.i ], [ %indvars.iv.next.7.3.i.lcssa, %._crit_edge.unr-lcssa.3.i.loopexit ]
  %lcmp.mod.3.i = icmp eq i64 %xtraiter.3.i, 0
  br i1 %lcmp.mod.3.i, label %._crit_edge.3.i, label %.epil.preheader.3.i

.epil.preheader.3.i:                              ; preds = %._crit_edge.unr-lcssa.3.i
  %.phi.trans.insert44.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.unr.3.i
  %.pre45.i = load double, double* %.phi.trans.insert44.i, align 8
  br label %329

329:                                              ; preds = %329, %.epil.preheader.3.i
  %330 = phi double [ %.pre45.i, %.epil.preheader.3.i ], [ %332, %329 ]
  %indvars.iv.epil.3.i = phi i64 [ %indvars.iv.unr.3.i, %.epil.preheader.3.i ], [ %indvars.iv.next.epil.3.i, %329 ]
  %epil.iter.3.i = phi i64 [ %xtraiter.3.i, %.epil.preheader.3.i ], [ %epil.iter.sub.3.i, %329 ]
  %indvars.iv.next.epil.3.i = add nuw nsw i64 %indvars.iv.epil.3.i, 1
  %331 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.epil.3.i
  %332 = load double, double* %331, align 8
  %333 = fmul double %45, %332
  %334 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.epil.3.i
  %335 = fmul double %46, %330
  %336 = fadd double %333, %335
  store double %336, double* %334, align 8
  %epil.iter.sub.3.i = add i64 %epil.iter.3.i, -1
  %epil.iter.cmp.3.i = icmp eq i64 %epil.iter.sub.3.i, 0
  br i1 %epil.iter.cmp.3.i, label %._crit_edge.3.i.loopexit, label %329

._crit_edge.3.i.loopexit:                         ; preds = %329
  br label %._crit_edge.3.i

._crit_edge.3.i:                                  ; preds = %._crit_edge.3.i.loopexit, %._crit_edge.unr-lcssa.3.i
  %indvar.next.3.i = add i64 %indvar.i, 4
  %niter22.nsub.3.i = add i64 %niter22.i, -4
  %niter22.ncmp.3.i = icmp eq i64 %niter22.nsub.3.i, 0
  br i1 %niter22.ncmp.3.i, label %._crit_edge6.unr-lcssa.i.loopexit, label %.lr.ph.i

_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit.loopexit: ; preds = %._crit_edge.epil.i
  br label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit

_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit: ; preds = %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit.loopexit, %._crit_edge6.unr-lcssa.i, %.preheader1.i, %simd.loop
  %337 = bitcast [2520 x double]* %20 to i64*
  %338 = load i64, i64* %337, align 8
  %339 = getelementptr inbounds double, double addrspace(1)* %15, i64 %add
  %340 = bitcast double addrspace(1)* %339 to i64 addrspace(1)*
  store i64 %338, i64 addrspace(1)* %340, align 8
  call void @llvm.lifetime.end.p0i8(i64 20160, i8* nonnull %23) #2
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2
