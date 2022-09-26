; The only use of bitcasted private alloca is lifetime intrinsic, which we consiider as a safe use.
; With enhanced memory escape analysis, this code is considered safe for SOA,
; and profitable for SOA.

; RUN: opt %s -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -vplan-vec -vplan-force-vf=4 -vplan-dump-soa-info 2>&1 | FileCheck %s

; CHECK: SOA profitability:
; CHECK: SOASafe =  Profitable = 1
; CHECK: SOA profitability:
; CHECK: SOASafe =  Profitable = 1

; CHECK: [[DOTSOA_VEC0:%.*]] = alloca [2520 x <4 x double>], align 32
; CHECK: [[TMP24:%.*]] = bitcast [2520 x <4 x double>]* [[DOTSOA_VEC0]] to <4 x i8>*
; CHECK: [[SOA_SCALAR_GEP0:%.*]] = getelementptr inbounds [2520 x <4 x double>], [2520 x <4 x double>]* [[DOTSOA_VEC0]], i64 0, i64 [[UNI_PHI350:%.*]]
; CHECK: store <4 x double> [[TMP96:%.*]], <4 x double>* [[SOA_SCALAR_GEP0]], align 8

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)
declare double @_Z4sqrtd(double)
declare double @_Z3expd(double)
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)
declare i64 @_Z13get_global_idj(i32)

define void @main(double addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), double addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), double addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), i32, double, double, double addrspace(1)*, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range"), %"class.cl::sycl::range"* byval(%"class.cl::sycl::range")) local_unnamed_addr {
  %20 = alloca [2520 x double], align 8
  %21 = call i64 @_Z13get_global_idj(i32 0)
  br label %simd.begin.region

simd.begin.region:                                ; preds = %19
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM:TYPED"(double addrspace(1)* %0, double zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %1, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %2, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %3, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(double addrspace(1)* %4, double zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %5, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %6, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %7, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(double addrspace(1)* %8, double zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %9, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %10, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %11, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM"(i32 %12, double %13, double %14), "QUAL.OMP.UNIFORM:TYPED"(double addrspace(1)* %15, double zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %16, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %17, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.UNIFORM:TYPED"(%"class.cl::sycl::range"* %18, %"class.cl::sycl::range" zeroinitializer, i32 1), "QUAL.OMP.PRIVATE:TYPED"([2520 x double]* %20, double 0.000000e+00, i32 2520), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %22 = sext i32 %index to i64
  %add = add nuw i64 %22, %21
  %23 = bitcast [2520 x double]* %20 to i8*
  call void @llvm.lifetime.start.p0i8(i64 20160, i8* nonnull %23)
  %24 = getelementptr inbounds double, double addrspace(1)* %0, i64 %add
  %25 = load double, double addrspace(1)* %24, align 8
  %26 = getelementptr inbounds double, double addrspace(1)* %4, i64 %add
  %27 = load double, double addrspace(1)* %26, align 8
  %28 = getelementptr inbounds double, double addrspace(1)* %8, i64 %add
  %29 = load double, double addrspace(1)* %28, align 8
  %30 = sitofp i32 %12 to double
  %31 = fdiv double %29, %30
  %32 = call double @_Z4sqrtd(double %31)
  %33 = fmul double %32, %13
  %34 = fmul double %31, %14
  %35 = call double @_Z3expd(double %34)
  %36 = fsub double -0.000000e+00, %34
  %37 = call double @_Z3expd(double %36)
  %38 = call double @_Z3expd(double %33)
  %39 = fsub double -0.000000e+00, %33
  %40 = call double @_Z3expd(double %39)
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
  br label %53

.preheader1.i:                                    ; preds = %53
  %52 = icmp sgt i32 %12, 0
  br i1 %52, label %.lr.ph.i, label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit

; <label>:53:                                     ; preds = %53, %.lr.ph10.i
  %indvars.iv14.i = phi i64 [ %indvars.iv.next15.i, %53 ], [ 0, %.lr.ph10.i ]
  %.037.i = phi double [ %59, %53 ], [ %51, %.lr.ph10.i ]
  %54 = call double @_Z3expd(double %.037.i)
  %55 = fmul double %25, %54
  %56 = fsub double %55, %27
  %57 = fcmp ogt double %56, 0.000000e+00
  %..i = select i1 %57, double %56, double 0.000000e+00
  %58 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv14.i
  store double %..i, double* %58, align 8
  %59 = fadd double %47, %.037.i
  %indvars.iv.next15.i = add nuw nsw i64 %indvars.iv14.i, 1
  %60 = sext i32 %12 to i64
  %61 = icmp slt i64 %indvars.iv14.i, %60
  br i1 %61, label %53, label %.preheader1.i

.lr.ph.i:                                         ; preds = %._crit_edge.i, %.preheader1.i
  %indvars.iv12.i = phi i64 [ %indvars.iv.next13.i, %._crit_edge.i ], [ %60, %.preheader1.i ]
  %.phi.trans.insert.i = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 0
  %.pre.i = load double, double* %.phi.trans.insert.i, align 8
  br label %62

; <label>:62:                                     ; preds = %62, %.lr.ph.i
  %63 = phi double [ %.pre.i, %.lr.ph.i ], [ %65, %62 ]
  %indvars.iv.i = phi i64 [ 0, %.lr.ph.i ], [ %indvars.iv.next.i, %62 ]
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %64 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.next.i
  %65 = load double, double* %64, align 8
  %66 = fmul double %45, %65
  %67 = getelementptr inbounds [2520 x double], [2520 x double]* %20, i64 0, i64 %indvars.iv.i
  %68 = fmul double %46, %63
  %69 = fadd double %66, %68
  store double %69, double* %67, align 8
  %exitcond.i = icmp eq i64 %indvars.iv.next.i, %indvars.iv12.i
  br i1 %exitcond.i, label %._crit_edge.i, label %62

._crit_edge.i:                                    ; preds = %62
  %indvars.iv.next13.i = add nsw i64 %indvars.iv12.i, -1
  %70 = icmp sgt i64 %indvars.iv.next13.i, 0
  br i1 %70, label %.lr.ph.i, label %_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit

_ZZZN8binomialIdE3runEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi1EEEE_clES7_.exit: ; preds = %._crit_edge.i, %.preheader1.i, %simd.loop
  %71 = bitcast [2520 x double]* %20 to i64*
  %72 = load i64, i64* %71, align 8
  %73 = getelementptr inbounds double, double addrspace(1)* %15, i64 %add
  %74 = bitcast double addrspace(1)* %73 to i64 addrspace(1)*
  store i64 %72, i64 addrspace(1)* %74, align 8
  call void @llvm.lifetime.end.p0i8(i64 20160, i8* nonnull %23)
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

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
