; This test verifies that we correctly import loop-privates as VPLoopEntities
; as well as through Converter->Descriptor mechanism


; RUN: opt %s -S -passes="mem2reg,loop-simplify,lcssa,vpo-cfg-restructuring,vplan-vec" \
; RUN: -vplan-force-vf=4 -disable-vplan-codegen \
; RUN: -vplan-entities-dump -disable-output \
; RUN: -debug-only=VPlanDriver 2>&1 | FileCheck %s

; REQUIRES:asserts


; CHECK: ptr [[PRIV_PTR:%.*]] = allocate-priv [2520 x double]
; CHECK: ptr {{.*}} = getelementptr inbounds [2520 x double], ptr [[PRIV_PTR]] i64 0 i64 {{.*}}

; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }


declare void @llvm.lifetime.start.p0(i64, ptr nocapture)
declare double @_Z4sqrtd(double)
declare double @_Z3expd(double)
declare void @llvm.lifetime.end.p0(i64, ptr nocapture)
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr


; Function Attrs: nounwind
define void @main(ptr addrspace(1), ptr byval(%"class.cl::sycl::range"), ptr byval(%"class.cl::sycl::range"), ptr byval(%"class.cl::sycl::range"), ptr addrspace(1), ptr byval(%"class.cl::sycl::range"), ptr byval(%"class.cl::sycl::range"), ptr byval(%"class.cl::sycl::range"), ptr addrspace(1), ptr byval(%"class.cl::sycl::range"), ptr byval(%"class.cl::sycl::range"), ptr byval(%"class.cl::sycl::range"), i32, double, double, ptr addrspace(1), ptr byval(%"class.cl::sycl::range"), ptr byval(%"class.cl::sycl::range"), ptr byval(%"class.cl::sycl::range")) local_unnamed_addr {
  %20 = alloca [2520 x double], align 8
  %21 = call i64 @_Z13get_global_idj(i32 0)
  br label %simd.begin.region

simd.begin.region:                                ; preds = %19
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM:TYPED"(ptr addrspace(1) %0, double zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %1, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %2, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %3, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr addrspace(1) %4, double zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %5, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %6, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %7, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr addrspace(1) %8, double zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %9, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %10, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %11, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM"(i32 %12, double %13, double %14), "QUAL.OMP.UNIFORM:TYPED"(ptr addrspace(1) %15, double zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %16, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %17, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(ptr %18, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %20, double 0.000000e+00, i32 2520), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %22 = sext i32 %index to i64
  %add = add nuw i64 %22, %21
  call void @llvm.lifetime.start.p0(i64 20160, ptr nonnull %20)
  %23 = getelementptr inbounds double, ptr addrspace(1) %0, i64 %add
  %24 = load double, ptr addrspace(1) %23, align 8
  %25 = getelementptr inbounds double, ptr addrspace(1) %4, i64 %add
  %26 = load double, ptr addrspace(1) %25, align 8
  %27 = getelementptr inbounds double, ptr addrspace(1) %8, i64 %add
  %28 = load double, ptr addrspace(1) %27, align 8
  %29 = sitofp i32 %12 to double
  %30 = fdiv double %28, %29
  %31 = call double @_Z4sqrtd(double %30)
  %32 = fmul double %31, %13
  %33 = fmul double %30, %14
  %34 = call double @_Z3expd(double %33)
  %35 = fsub double -0.000000e+00, %33
  %36 = call double @_Z3expd(double %35)
  %37 = call double @_Z3expd(double %32)
  %38 = fsub double -0.000000e+00, %32
  %39 = call double @_Z3expd(double %38)
  %40 = fsub double %34, %39
  %41 = fsub double %37, %39
  %42 = fdiv double %40, %41
  %43 = fsub double 1.000000e+00, %42
  %44 = fmul double %36, %42
  %45 = fmul double %36, %43
  %46 = fmul double %32, 2.000000e+00
  %47 = icmp slt i32 %12, 0
  br i1 %47, label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit, label %.lr.ph10.i

.lr.ph10.i:                                       ; preds = %simd.loop
  %48 = sub nsw i32 0, %12
  %49 = sitofp i32 %48 to double
  %50 = fmul double %32, %49
  br label %52

.preheader1.i:                                    ; preds = %52
  %51 = icmp sgt i32 %12, 0
  br i1 %51, label %.lr.ph.i, label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit

; <label>:53:                                     ; preds = %52, %.lr.ph10.i
  %indvars.iv14.i = phi i64 [ %indvars.iv.next15.i, %52 ], [ 0, %.lr.ph10.i ]
  %.037.i = phi double [ %58, %52 ], [ %50, %.lr.ph10.i ]
  %53 = call double @_Z3expd(double %.037.i)
  %54 = fmul double %24, %53
  %55 = fsub double %54, %26
  %56 = fcmp ogt double %55, 0.000000e+00
  %..i = select i1 %56, double %55, double 0.000000e+00
  %57 = getelementptr inbounds [2520 x double], ptr %20, i64 0, i64 %indvars.iv14.i
  store double %..i, ptr %57, align 8
  %58 = fadd double %46, %.037.i
  %indvars.iv.next15.i = add nuw nsw i64 %indvars.iv14.i, 1
  %59 = sext i32 %12 to i64
  %60 = icmp slt i64 %indvars.iv14.i, %59
  br i1 %60, label %52, label %.preheader1.i

.lr.ph.i:                                         ; preds = %._crit_edge.i, %.preheader1.i
  %indvars.iv12.i = phi i64 [ %indvars.iv.next13.i, %._crit_edge.i ], [ %59, %.preheader1.i ]
  %.pre.i = load double, ptr %20, align 8
  br label %61

; <label>:62:                                     ; preds = %61, %.lr.ph.i
  %62 = phi double [ %.pre.i, %.lr.ph.i ], [ %64, %61 ]
  %indvars.iv.i = phi i64 [ 0, %.lr.ph.i ], [ %indvars.iv.next.i, %61 ]
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %63 = getelementptr inbounds [2520 x double], ptr %20, i64 0, i64 %indvars.iv.next.i
  %64 = load double, ptr %63, align 8
  %65 = fmul double %44, %64
  %66 = getelementptr inbounds [2520 x double], ptr %20, i64 0, i64 %indvars.iv.i
  %67 = fmul double %45, %62
  %68 = fadd double %65, %67
  store double %68, ptr %66, align 8
  %exitcond.i = icmp eq i64 %indvars.iv.next.i, %indvars.iv12.i
  br i1 %exitcond.i, label %._crit_edge.i, label %61

._crit_edge.i:                                    ; preds = %61
  %indvars.iv.next13.i = add nsw i64 %indvars.iv12.i, -1
  %69 = icmp sgt i64 %indvars.iv.next13.i, 0
  br i1 %69, label %.lr.ph.i, label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit

_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit: ; preds = %._crit_edge.i, %.preheader1.i, %simd.loop
  %70 = load i64, ptr %20, align 8
  %71 = getelementptr inbounds double, ptr addrspace(1) %15, i64 %add
  store i64 %70, ptr addrspace(1) %71, align 8
  call void @llvm.lifetime.end.p0(i64 20160, ptr nonnull %20)
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !19

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!19 = distinct !{!19, !20}
!20 = !{!"llvm.loop.unroll.disable"}
