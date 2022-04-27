; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; This file checks the support of the firstprivate which is a reference.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%struct.my_struct = type { [12 x i16] }
%"class._ZTSZN9my_struct4workERA12_sE3$_0" = type { %struct.my_struct*, [12 x i16]* }

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@__num_instances = dso_local global i32 0, align 4
@__num_initializer_calls = dso_local global i32 0, align 4
@_Z1x = internal global [12 x i16] zeroinitializer, align 16
@_Z10y_original = internal global [12 x i16] zeroinitializer, align 16
@_Z10y_expected = internal global [12 x i16] zeroinitializer, align 16
@_Z1z = internal global [12 x i16] zeroinitializer, align 16
@_Z10z_expected = internal global [12 x i16] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_a.cpp, i8* null }]
@"@tid.addr" = external global i32

; Function Attrs: uwtable
define internal void @__cxx_global_var_init() #0 section ".text.startup" {
entry:
  call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* @_ZSt8__ioinit)
  %0 = call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZSt8__ioinit, i32 0, i32 0), i8* @__dso_handle) #3
  ret void
}

declare dso_local void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #2

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) #3

; Function Attrs: nounwind uwtable
define dso_local void @_Z4initPs(i16* %y) #4 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp ult i32 %i.0, 12
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %conv = trunc i32 %i.0 to i16
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [12 x i16], [12 x i16]* @_Z1x, i64 0, i64 %idxprom
  store i16 %conv, i16* %arrayidx, align 2, !tbaa !2
  %mul = mul i32 2, %i.0
  %conv1 = trunc i32 %mul to i16
  %arrayidx3 = getelementptr inbounds i16, i16* %y, i64 %idxprom
  store i16 %conv1, i16* %arrayidx3, align 2, !tbaa !7
  %arrayidx7 = getelementptr inbounds [12 x i16], [12 x i16]* @_Z10y_original, i64 0, i64 %idxprom
  store i16 %conv1, i16* %arrayidx7, align 2, !tbaa !2
  %0 = load i16, i16* %arrayidx3, align 2, !tbaa !7
  %arrayidx11 = getelementptr inbounds [12 x i16], [12 x i16]* @_Z10y_expected, i64 0, i64 %idxprom
  store i16 %0, i16* %arrayidx11, align 2, !tbaa !2
  %arrayidx13 = getelementptr inbounds [12 x i16], [12 x i16]* @_Z1z, i64 0, i64 %idxprom
  store i16 1, i16* %arrayidx13, align 2, !tbaa !2
  %arrayidx17 = getelementptr inbounds [12 x i16], [12 x i16]* @_Z10z_expected, i64 0, i64 %idxprom
  store i16 1, i16* %arrayidx17, align 2, !tbaa !2
  %inc = add i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %for.cond19

for.cond19:                                       ; preds = %for.body21, %for.end
  %i18.0 = phi i32 [ 3, %for.end ], [ %inc27, %for.body21 ]
  %cmp20 = icmp slt i32 %i18.0, 6
  br i1 %cmp20, label %for.body21, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond19
  ret void

for.body21:                                       ; preds = %for.cond19
  %idxprom22 = sext i32 %i18.0 to i64
  %arrayidx23 = getelementptr inbounds i16, i16* %y, i64 %idxprom22
  %1 = load i16, i16* %arrayidx23, align 2, !tbaa !7
  %arrayidx25 = getelementptr inbounds [12 x i16], [12 x i16]* @_Z10z_expected, i64 0, i64 %idxprom22
  store i16 %1, i16* %arrayidx25, align 2, !tbaa !2
  %inc27 = add nsw i32 %i18.0, 1
  br label %for.cond19
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #5

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #5

; Function Attrs: uwtable
define dso_local void @_ZN9my_struct4workERA12_s(%struct.my_struct* %this, [12 x i16]* dereferenceable(24) %x) #0 align 2 {
entry:
  %foo = alloca %"class._ZTSZN9my_struct4workERA12_sE3$_0", align 8
  %0 = bitcast %"class._ZTSZN9my_struct4workERA12_sE3$_0"* %foo to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %0) #3
  %1 = getelementptr inbounds %"class._ZTSZN9my_struct4workERA12_sE3$_0", %"class._ZTSZN9my_struct4workERA12_sE3$_0"* %foo, i32 0, i32 0
  store %struct.my_struct* %this, %struct.my_struct** %1, align 8, !tbaa !8
  %2 = getelementptr inbounds %"class._ZTSZN9my_struct4workERA12_sE3$_0", %"class._ZTSZN9my_struct4workERA12_sE3$_0"* %foo, i32 0, i32 1
  store [12 x i16]* %x, [12 x i16]** %2, align 8, !tbaa !12
  call void @"_ZZN9my_struct4workERA12_sENK3$_0clEv"(%"class._ZTSZN9my_struct4workERA12_sE3$_0"* %foo)
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %0) #3
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define internal void @"_ZZN9my_struct4workERA12_sENK3$_0clEv"(%"class._ZTSZN9my_struct4workERA12_sE3$_0"* %this) #6 align 2 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = getelementptr inbounds %"class._ZTSZN9my_struct4workERA12_sE3$_0", %"class._ZTSZN9my_struct4workERA12_sE3$_0"* %this, i32 0, i32 0
  %1 = load %struct.my_struct*, %struct.my_struct** %0, align 8, !tbaa !8
  %y2 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 0
  %2 = getelementptr inbounds %"class._ZTSZN9my_struct4workERA12_sE3$_0", %"class._ZTSZN9my_struct4workERA12_sE3$_0"* %this, i32 0, i32 1
  %3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #3
  %4 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #3
  store i32 0, i32* %.omp.lb, align 4, !tbaa !13
  %5 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #3
  store i32 2, i32* %.omp.ub, align 4, !tbaa !13
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %entry
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"([12 x i16]* %y2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  br label %DIR.OMP.LOOP.11

DIR.OMP.LOOP.11:                                  ; preds = %DIR.OMP.LOOP.1
  %7 = load i32, i32* %.omp.lb, align 4, !tbaa !13
  store volatile i32 %7, i32* %.omp.iv, align 4, !tbaa !13
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.11
  %8 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !13
  %9 = load i32, i32* %.omp.ub, align 4, !tbaa !13
  %cmp = icmp sle i32 %8, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #3
  %11 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !13
  %add = add nsw i32 3, %11
  store i32 %add, i32* %i, align 4, !tbaa !13
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds [12 x i16], [12 x i16]* %y2, i64 0, i64 %idxprom
  %12 = load i16, i16* %arrayidx, align 2, !tbaa !2
  %arrayidx6 = getelementptr inbounds [12 x i16], [12 x i16]* @_Z1z, i64 0, i64 %idxprom
  store i16 %12, i16* %arrayidx6, align 2, !tbaa !2
  %13 = load [12 x i16]*, [12 x i16]** %2, align 8, !tbaa !15
  %14 = load i32, i32* %i, align 4, !tbaa !13
  %idxprom7 = sext i32 %14 to i64
  %arrayidx8 = getelementptr inbounds [12 x i16], [12 x i16]* %13, i64 0, i64 %idxprom7
  %15 = load i16, i16* %arrayidx8, align 2, !tbaa !2
  %arrayidx10 = getelementptr inbounds [12 x i16], [12 x i16]* %y2, i64 0, i64 %idxprom7
  store i16 %15, i16* %arrayidx10, align 2, !tbaa !2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #3
  %16 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !13
  %add11 = add nsw i32 %16, 1
  store volatile i32 %add11, i32* %.omp.iv, align 4, !tbaa !13
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.2

DIR.OMP.END.LOOP.2:                               ; preds = %omp.loop.exit
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %5) #3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3) #3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_a.cpp() #0 section ".text.startup" {
entry:
  call void @__cxx_global_var_init()
  ret void
}

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { argmemonly nounwind }
attributes #6 = { inlinehint nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA12_s", !4, i64 0}
!4 = !{!"short", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!4, !4, i64 0}
!8 = !{!9, !10, i64 0}
!9 = !{!"struct@_ZTSZN9my_struct4workERA12_sE3$_0", !10, i64 0, !11, i64 8}
!10 = !{!"unspecified pointer", !5, i64 0}
!11 = !{!"any pointer", !5, i64 0}
!12 = !{!11, !11, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !5, i64 0}
!15 = !{!9, !11, i64 8}

; CHECK-LABEL: define internal void @"_ZZN9my_struct4workERA12_sENK3$_0clEv"
; CHECK: %{{[a-zA-Z._0-9]+}} = alloca [12 x i16]
