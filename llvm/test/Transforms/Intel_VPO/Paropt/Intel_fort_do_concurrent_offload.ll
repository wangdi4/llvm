; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; This test checks that the "genericloop qual.ext.do.concurrent" construct is 
; mapped to "distribute parallel do"
; after prepare pass.

; integer, parameter :: n=64
; real a(n)
; real::b(n)=17.
; do concurrent(i=1:n)
;   a(i) = b(i)+i
; enddo
;
; print *,a
; end

; ModuleID = '/tmp/ifxkjeOa1.bc'
source_filename = "/tmp/ifxkjeOa1.bc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@"_unnamed_main$$_$B" = internal global [64 x float] [float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01], align 16
@0 = internal unnamed_addr constant i32 65536
@1 = internal unnamed_addr constant i32 2
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define void @MAIN__() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"_unnamed_main$$_$I$_1" = alloca i32, align 8
  %"_unnamed_main$$_$A" = alloca [64 x float], align 16
  %"var$1" = alloca i32, align 4
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca <{ i64, i8* }>, align 8
  %func_result = call i32 @for_set_fpe_(i32* @0)
  %func_result2 = call i32 @for_set_reentrancy(i32* @1)
  %do.start = alloca i32, align 4
  store i32 1, i32* %do.start, align 1, !tbaa !2
  %do.end = alloca i32, align 4
  store i32 64, i32* %do.end, align 1, !tbaa !2
  %do.step = alloca i32, align 4
  store i32 1, i32* %do.step, align 1, !tbaa !2
  %do.norm.lb = alloca i64, align 8
  store i64 0, i64* %do.norm.lb, align 1, !tbaa !2
  %do.norm.ub = alloca i64, align 8
  %do.end_fetch.1 = load i32, i32* %do.end, align 1, !tbaa !2
  %do.start_fetch.2 = load i32, i32* %do.start, align 1, !tbaa !2
  %sub.1 = sub nsw i32 %do.end_fetch.1, %do.start_fetch.2
  %do.step_fetch.3 = load i32, i32* %do.step, align 1, !tbaa !2
  %div.1 = sdiv i32 %sub.1, %do.step_fetch.3
  %int_sext = sext i32 %div.1 to i64
  store i64 %int_sext, i64* %do.norm.ub, align 1, !tbaa !2
  %do.norm.iv = alloca i64, align 8
  br label %bb_new6

do.cond10:                                        ; preds = %DIR.OMP.GENERICLOOP.4, %do.body11
  %do.norm.iv_fetch.5 = load i64, i64* %do.norm.iv, align 1, !tbaa !2
  %do.norm.ub_fetch.6 = load i64, i64* %do.norm.ub, align 1, !tbaa !2
  %rel.1 = icmp sle i64 %do.norm.iv_fetch.5, %do.norm.ub_fetch.6
  br i1 %rel.1, label %do.body11, label %do.epilog12

do.body11:                                        ; preds = %do.cond10
  %do.norm.iv_fetch.7 = load i64, i64* %do.norm.iv, align 1, !tbaa !2
  %int_sext4 = trunc i64 %do.norm.iv_fetch.7 to i32
  %do.step_fetch.8 = load i32, i32* %do.step, align 1, !tbaa !2
  %mul.1 = mul nsw i32 %int_sext4, %do.step_fetch.8
  %do.start_fetch.9 = load i32, i32* %do.start, align 1, !tbaa !2
  %add.1 = add nsw i32 %mul.1, %do.start_fetch.9
  store i32 %add.1, i32* %"_unnamed_main$$_$I$_1", align 1, !tbaa !5
  %"_unnamed_main$$_$I$_1_fetch.10" = load i32, i32* %"_unnamed_main$$_$I$_1", align 1, !tbaa !5
  %int_sext5 = sext i32 %"_unnamed_main$$_$I$_1_fetch.10" to i64
  %0 = sub nsw i64 %int_sext5, 1
  %1 = getelementptr inbounds float, float* getelementptr inbounds ([64 x float], [64 x float]* @"_unnamed_main$$_$B", i32 0, i32 0), i64 %0
  %"_unnamed_main$$_$B[]_fetch.11" = load float, float* %1, align 1, !tbaa !7
  %"_unnamed_main$$_$I$_1_fetch.12" = load i32, i32* %"_unnamed_main$$_$I$_1", align 1, !tbaa !5
  %"(float)_unnamed_main$$_$I$_1_fetch.12$" = sitofp i32 %"_unnamed_main$$_$I$_1_fetch.12" to float
  %add.2 = fadd reassoc ninf nsz arcp contract afn float %"_unnamed_main$$_$B[]_fetch.11", %"(float)_unnamed_main$$_$I$_1_fetch.12$"
  %"_unnamed_main$$_$I$_1_fetch.13" = load i32, i32* %"_unnamed_main$$_$I$_1", align 1, !tbaa !5
  %int_sext6 = sext i32 %"_unnamed_main$$_$I$_1_fetch.13" to i64
  %"(float*)_unnamed_main$$_$A$" = bitcast [64 x float]* %"_unnamed_main$$_$A" to float*
  %2 = sub nsw i64 %int_sext6, 1
  %3 = getelementptr inbounds float, float* %"(float*)_unnamed_main$$_$A$", i64 %2
  store float %add.2, float* %3, align 1, !tbaa !9
  %do.norm.iv_fetch.14 = load i64, i64* %do.norm.iv, align 1, !tbaa !2
  %add.3 = add nsw i64 %do.norm.iv_fetch.14, 1
  store i64 %add.3, i64* %do.norm.iv, align 1, !tbaa !2
  br label %do.cond10

do.epilog12:                                      ; preds = %do.cond10
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %DIR.OMP.END.GENERICLOOP.1

DIR.OMP.END.GENERICLOOP.1:                        ; preds = %do.epilog12
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.2

DIR.OMP.END.TEAMS.2:                              ; preds = %DIR.OMP.END.GENERICLOOP.1
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.END.TEAMS.2
  store [4 x i8] c"\1A\05\01\00", [4 x i8]* %"(&)val$", align 1, !tbaa !2
  %BLKFIELD_i64_ = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock, i32 0, i32 0
  store i64 256, i64* %BLKFIELD_i64_, align 1, !tbaa !11
  %"(i8*)_unnamed_main$$_$A$" = bitcast [64 x float]* %"_unnamed_main$$_$A" to i8*
  %"BLKFIELD_i8*_" = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock, i32 0, i32 1
  store i8* %"(i8*)_unnamed_main$$_$A$", i8** %"BLKFIELD_i8*_", align 1, !tbaa !13
  %"(i8*)$io_ctx$" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)(&)val$$" = bitcast [4 x i8]* %"(&)val$" to i8*
  %"(i8*)argblock$" = bitcast <{ i64, i8* }>* %argblock to i8*
  %func_result8 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %"(i8*)$io_ctx$", i32 -1, i64 1239157112576, i8* %"(i8*)(&)val$$", i8* %"(i8*)argblock$")
  ret void

bb_new8:                                          ; preds = %DIR.OMP.TEAMS.2

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.DISTRIBUTE.PARLOOP
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.EXT.DO.CONCURRENT"(),

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.EXT.DO.CONCURRENT"(), "QUAL.OMP.COLLAPSE"(i32 1), "QUAL.OMP.SHARED"([64 x float]* %"_unnamed_main$$_$A"), "QUAL.OMP.SHARED"([64 x float]* @"_unnamed_main$$_$B"), "QUAL.OMP.SHARED"(i32* %do.step), "QUAL.OMP.SHARED"(i32* %do.start), "QUAL.OMP.PRIVATE"(i32* %"_unnamed_main$$_$I$_1"), "QUAL.OMP.FIRSTPRIVATE"(i64* %do.norm.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %do.norm.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %do.norm.ub), "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"() ]
  br label %DIR.OMP.GENERICLOOP.4

DIR.OMP.GENERICLOOP.4:                            ; preds = %bb_new8
  %do.norm.lb_fetch.4 = load i64, i64* %do.norm.lb, align 1, !tbaa !2
  store i64 %do.norm.lb_fetch.4, i64* %do.norm.iv, align 1, !tbaa !2
  br label %do.cond10

bb_new6:                                          ; preds = %alloca_0
  br label %DIR.OMP.TARGET.5

DIR.OMP.TARGET.5:                                 ; preds = %bb_new6
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %DIR.OMP.TARGET.5
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.EXT.DO.CONCURRENT"(), "QUAL.OMP.MAP.TOFROM"([64 x float]* @"_unnamed_main$$_$B", [64 x float]* @"_unnamed_main$$_$B", i64 256, i64 39, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"([64 x float]* %"_unnamed_main$$_$A", [64 x float]* %"_unnamed_main$$_$A", i64 256, i64 39, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i64* %do.norm.iv), "QUAL.OMP.PRIVATE"(i32* %"_unnamed_main$$_$I$_1"), "QUAL.OMP.FIRSTPRIVATE"(i64* %do.norm.ub), "QUAL.OMP.FIRSTPRIVATE"(i64* %do.norm.lb), "QUAL.OMP.FIRSTPRIVATE"(i32* %do.step), "QUAL.OMP.FIRSTPRIVATE"(i32* %do.start), "QUAL.OMP.OFFLOAD.NDRANGE"(i64* %do.norm.ub, i64 0) ]
  br label %bb_new7

bb_new7:                                          ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.TEAMS.6

DIR.OMP.TEAMS.6:                                  ; preds = %bb_new7
  br label %DIR.OMP.TEAMS.2

DIR.OMP.TEAMS.2:                                  ; preds = %DIR.OMP.TEAMS.6
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.EXT.DO.CONCURRENT"(), "QUAL.OMP.SHARED"([64 x float]* %"_unnamed_main$$_$A"), "QUAL.OMP.SHARED"([64 x float]* @"_unnamed_main$$_$B"), "QUAL.OMP.SHARED"(i64* %do.norm.ub), "QUAL.OMP.SHARED"(i64* %do.norm.lb), "QUAL.OMP.SHARED"(i32* %do.step), "QUAL.OMP.SHARED"(i32* %do.start), "QUAL.OMP.PRIVATE"(i64* %do.norm.iv), "QUAL.OMP.PRIVATE"(i32* %"_unnamed_main$$_$I$_1") ]
  br label %bb_new8
}

declare i32 @for_set_fpe_(i32* nocapture readonly)

declare i32 @for_set_reentrancy(i32* nocapture readonly)

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!omp_offload.info = !{!1}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i32 0, i32 2050, i32 56484600, !"MAIN__", i32 4, i32 0, i32 0}
!2 = !{!3, !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$1$MAIN__"}
!5 = !{!6, !6, i64 0}
!6 = !{!"ifx$unique_sym$1", !3, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"ifx$unique_sym$2", !3, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$3", !3, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"ifx$unique_sym$4", !3, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"ifx$unique_sym$5", !3, i64 0}
; end INTEL_CUSTOMIZATION
