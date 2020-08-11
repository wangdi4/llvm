; RUN: opt -mattr=+avx2 -enable-intel-advanced-opts -hir-ssa-deconstruction -hir-cost-model-throttling=0 -hir-temp-cleanup -tbaa -hir-pm-symbolic-tripcount-completeunroll -print-after=hir-pm-symbolic-tripcount-completeunroll -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -mattr=+avx2 -enable-intel-advanced-opts -hir-cost-model-throttling=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pm-symbolic-tripcount-completeunroll,print<hir>" -aa-pipeline="basic-aa,tbaa" -disable-output < %s 2>&1 | FileCheck %s


; The input is very similar to pattern-match-1.ll, but slightly different in dealing with copy instructions (see copy inst 1 and 2 below).
; Make sure this alternative pattern is also matched by HIRSymbolicTripCountCompleteUnroll.

; Function: _ZN9FastBoard13add_neighbourEii
;
; <0>          BEGIN REGION { }
; <62>               + DO i1 = 0, 3, 1   <DO_LOOP>
; <4>                |   %15 = (%0)[0].12.0[i1];
; <8>                |   %19 = (%0)[0].10.0[%15 + %1];
; <10>               |   (%0)[0].10.0[%15 + %1] = %19 + trunc.i32.i16(%7) + -256;
; <13>               |   %23 = (%0)[0].7.0[%15 + %1];
; <15>               |   if (%13 > 0)
; <15>               |   {
; <63>               |      + DO i2 = 0, zext.i32.i64(%13) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4>
; <31>               |      |   if ((%4)[0].0[i2] == %23)
; <31>               |      |   {
; <32>               |      |      goto %46;
; <31>               |      |   }
; <63>               |      + END LOOP
; <63>               |
; <41>               |      %39 = %13; // copy inst 1
; <42>               |      goto %38;
; <43>               |      %46:
; <44>               |      goto %47;
; <15>               |   }
; <20>               |   %39 = %13; // copy inst 2
; <45>               |   %38:
; <48>               |   %42 = (%0)[0].8.0[%23];
; <50>               |   (%0)[0].8.0[%23] = %42 + -1;
; <51>               |   %13 = %13  +  1;
; <53>               |   (%4)[0].0[%39] = %23;
; <55>               |   %47:
; <62>               + END LOOP
; <0>          END REGION

; Function: _ZN9FastBoard13add_neighbourEii
;
;CHECK:         BEGIN REGION { modified }
;CHECK:               %15 = (%0)[0].12.0[0];
;CHECK:               %19 = (%0)[0].10.0[%15 + %1];
;CHECK:               (%0)[0].10.0[%15 + %1] = %19 + trunc.i32.i16(%7) + -256;
;CHECK:               %23 = (%0)[0].7.0[%15 + %1];
;CHECK:               %42 = (%0)[0].8.0[%23];
;CHECK:               %mv = (%0)[0].12.0[1];
;CHECK:               %mv3 = (%0)[0].10.0[%1 + %mv];
;CHECK:               (%0)[0].10.0[%1 + %mv] = trunc.i32.i16(%7) + %mv3 + -256;
;CHECK:               %mv4 = (%0)[0].7.0[%1 + %mv];
;CHECK:               %mv5 = (%0)[0].8.0[%mv4];
;CHECK:               %mv6 = (%0)[0].12.0[2];
;CHECK:               %mv7 = (%0)[0].10.0[%1 + %mv6];
;CHECK:               (%0)[0].10.0[%1 + %mv6] = trunc.i32.i16(%7) + %mv7 + -256;
;CHECK:               %mv8 = (%0)[0].7.0[%1 + %mv6];
;CHECK:               %mv9 = (%0)[0].8.0[%mv8];
;CHECK:               %mv10 = (%0)[0].12.0[3];
;CHECK:               %mv11 = (%0)[0].10.0[%1 + %mv10];
;CHECK:               (%0)[0].10.0[%1 + %mv10] = trunc.i32.i16(%7) + %mv11 + -256;
;CHECK:               %mv12 = (%0)[0].7.0[%1 + %mv10];
;CHECK:               %mv13 = (%0)[0].8.0[%mv12];
;CHECK:               (%0)[0].8.0[%23] = %42 + -1;
;CHECK:               (%0)[0].8.0[%mv4] = %mv5 + -1;
;CHECK:               (%0)[0].8.0[%mv8] = %mv9 + -1;
;CHECK:               (%0)[0].8.0[%mv12] = %mv13 + -1;
;CHECK:         END REGION

; ModuleID = 'input.ll'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.FastBoard = type <{ %"class.boost::array", %"class.boost::array", i32, i32, i32, %"class.boost::array.0", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array", [2 x i8], %"class.boost::array.2", %"class.boost::array.3", %"class.boost::array.4", %"class.boost::array.4", %"class.std::vector", i32, [4 x i8] }>
%"class.boost::array.0" = type { [441 x i32] }
%"class.boost::array.1" = type { [442 x i16] }
%"class.boost::array" = type { [441 x i16] }
%"class.boost::array.2" = type { [4 x i32] }
%"class.boost::array.3" = type { [8 x i32] }
%"class.boost::array.4" = type { [2 x i32] }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" = type { i32*, i32*, i32* }

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: nounwind uwtable
define hidden fastcc void @_ZN9FastBoard13add_neighbourEii(%class.FastBoard* nocapture %0, i32 %1, i32 %2) unnamed_addr #1 align 2 {
  %4 = alloca %"class.boost::array.2", align 4
  %5 = bitcast %"class.boost::array.2"* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %5) #2
  %6 = shl nsw i32 %2, 2
  %7 = shl nuw i32 1, %6
  %8 = trunc i32 %7 to i16
  %9 = add i16 %8, -256
  br label %11

10:                                               ; preds = %47
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %5) #2
  ret void

11:                                               ; preds = %47, %3
  %12 = phi i64 [ 0, %3 ], [ %49, %47 ]
  %13 = phi i32 [ 0, %3 ], [ %48, %47 ]
  %14 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 12, i32 0, i64 %12, !intel-tbaa !5
  %15 = load i32, i32* %14, align 4, !tbaa !5
  %16 = add nsw i32 %15, %1
  %17 = sext i32 %16 to i64
  %18 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 10, i32 0, i64 %17, !intel-tbaa !25
  %19 = load i16, i16* %18, align 2, !tbaa !25
  %20 = add i16 %9, %19
  store i16 %20, i16* %18, align 2, !tbaa !25
  %21 = icmp sgt i32 %13, 0
  %22 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 7, i32 0, i64 %17
  %23 = load i16, i16* %22, align 2, !tbaa !26
  %24 = zext i16 %23 to i32
  br i1 %21, label %27, label %25

25:                                               ; preds = %11
  %26 = sext i32 %13 to i64
  br label %38

27:                                               ; preds = %11
  %28 = zext i32 %13 to i64
  br label %31

29:                                               ; preds = %31
  %30 = icmp eq i64 %36, %28
  br i1 %30, label %37, label %31

31:                                               ; preds = %29, %27
  %32 = phi i64 [ 0, %27 ], [ %36, %29 ]
  %33 = getelementptr inbounds %"class.boost::array.2", %"class.boost::array.2"* %4, i64 0, i32 0, i64 %32
  %34 = load i32, i32* %33, align 4, !tbaa !27
  %35 = icmp eq i32 %34, %24
  %36 = add nuw nsw i64 %32, 1
  br i1 %35, label %46, label %29

37:                                               ; preds = %29
  br label %38

38:                                               ; preds = %37, %25
  %39 = phi i64 [ %26, %25 ], [ %28, %37 ]
  %40 = zext i16 %23 to i64
  %41 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 8, i32 0, i64 %40
  %42 = load i16, i16* %41, align 2, !tbaa !28
  %43 = add i16 %42, -1
  store i16 %43, i16* %41, align 2, !tbaa !28
  %44 = add nsw i32 %13, 1
  %45 = getelementptr inbounds %"class.boost::array.2", %"class.boost::array.2"* %4, i64 0, i32 0, i64 %39
  store i32 %24, i32* %45, align 4, !tbaa !27
  br label %47

46:                                               ; preds = %31
  br label %47

47:                                               ; preds = %46, %38
  %48 = phi i32 [ %44, %38 ], [ %13, %46 ]
  %49 = add nuw nsw i64 %12, 1
  %50 = icmp eq i64 %49, 4
  br i1 %50, label %10, label %11
}

attributes #0 = { argmemonly nounwind willreturn }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{i32 1, !"LTOPostLink", i32 1}
!5 = !{!6, !12, i64 7960}
!6 = !{!"struct@_ZTS9FastBoard", !7, i64 0, !7, i64 882, !12, i64 1764, !12, i64 1768, !12, i64 1772, !13, i64 1776, !16, i64 3540, !16, i64 4424, !16, i64 5308, !16, i64 6192, !7, i64 7076, !18, i64 7960, !20, i64 7976, !22, i64 8008, !22, i64 8016, !24, i64 8024, !12, i64 8048}
!7 = !{!"struct@_ZTSN5boost5arrayItLm441EEE", !8, i64 0}
!8 = !{!"array@_ZTSA441_t", !9, i64 0}
!9 = !{!"short", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!"int", !10, i64 0}
!13 = !{!"struct@_ZTSN5boost5arrayIN9FastBoard8square_tELm441EEE", !14, i64 0}
!14 = !{!"array@_ZTSA441_N9FastBoard8square_tE", !15, i64 0}
!15 = !{!"_ZTSN9FastBoard8square_tE", !10, i64 0}
!16 = !{!"struct@_ZTSN5boost5arrayItLm442EEE", !17, i64 0}
!17 = !{!"array@_ZTSA442_t", !9, i64 0}
!18 = !{!"struct@_ZTSN5boost5arrayIiLm4EEE", !19, i64 0}
!19 = !{!"array@_ZTSA4_i", !12, i64 0}
!20 = !{!"struct@_ZTSN5boost5arrayIiLm8EEE", !21, i64 0}
!21 = !{!"array@_ZTSA8_i", !12, i64 0}
!22 = !{!"struct@_ZTSN5boost5arrayIiLm2EEE", !23, i64 0}
!23 = !{!"array@_ZTSA2_i", !12, i64 0}
!24 = !{!"struct@_ZTSSt6vectorIiSaIiEE"}
!25 = !{!6, !9, i64 7076}
!26 = !{!6, !9, i64 4424}
!27 = !{!18, !12, i64 0}
!28 = !{!6, !9, i64 5308}
