; INTEL_CUSTOMIZATION
; RUN: opt -passes="function(early-cse,vpo-cfg-restructuring),vpo-paropt" -S < %s 2>&1 | FileCheck %s

; CSE should not occur between the hypotf calls inside/outside the OMP region.

; CHECK-LABEL: define void @MAIN__
; CHECK: call reassoc ninf nsz arcp contract afn float @hypotf(float 0x40091EB860000000, float 0x40091EB860000000)

; CHECK-LABEL: define internal void @MAIN__.DIR.OMP.PARALLEL{{.*}}
; CHECK: [[ABS10:%.*]] = call reassoc ninf nsz arcp contract afn float @hypotf(float [[SUB3:%.*]], float [[SUB4:%.*]])
; CHECK: [[ABS13:%.*]] = call reassoc ninf nsz arcp contract afn float @hypotf(float [[FETCH8:%.*]], float [[FETCH9:%.*]])
; CHECK: [[ABS14:%.*]] = call reassoc ninf nsz arcp contract afn float @hypotf(float 0x40091EB860000000, float 0x40091EB860000000)
; CHECK: [[ADD2:%.*]] = fadd reassoc ninf nsz arcp contract afn float [[ABS13]], [[ABS14]]

; Test src:
; program test
;  implicit none
;  complex  :: A
;  complex init
;  init() = CMPLX(3.14, 3.14)
;  logical :: failed = .false.
;  if (abs(A - init()) .gt. (abs(A) + abs(init()))) then
;  end if
;
;  !$omp parallel
;  if (abs(A - init()) .gt. (abs(A) + abs(init()))) then
;     failed = .true.
;  end if
;  !$omp end parallel
; end program test

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%complex_64bit = type { float, float }

@"test_$FAILED" = internal global i32 0, align 4, !llfort.type_idx !0
@0 = internal unnamed_addr constant i32 65536, align 4
@1 = internal unnamed_addr constant i32 2, align 4
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define void @MAIN__() #0 !llfort.type_idx !2 {
alloca_0:
  %"test_$A" = alloca %complex_64bit, align 8, !llfort.type_idx !3
  %func_result = call i32 @for_set_fpe_(ptr @0), !llfort.type_idx !4
  %func_result2 = call i32 @for_set_reentrancy(ptr @1), !llfort.type_idx !4
  %0 = getelementptr %complex_64bit, ptr %"test_$A", i64 0, i32 0, !llfort.type_idx !5
  %1 = getelementptr %complex_64bit, ptr %"test_$A", i64 0, i32 1, !llfort.type_idx !6
  %fetch.1 = load float, ptr %0, align 1, !tbaa !7, !llfort.type_idx !5
  %fetch.2 = load float, ptr %1, align 1, !tbaa !12, !llfort.type_idx !6
  %insertval13 = insertvalue %complex_64bit zeroinitializer, float %fetch.1, 0
  %insertval14 = insertvalue %complex_64bit %insertval13, float %fetch.2, 1, !llfort.type_idx !3
  %insertval_comp_0 = extractvalue %complex_64bit %insertval14, 0
  %sub.1 = fsub reassoc ninf nsz arcp contract afn float %insertval_comp_0, 0x40091EB860000000
  %insertval_comp_1 = extractvalue %complex_64bit %insertval14, 1
  %sub.2 = fsub reassoc ninf nsz arcp contract afn float %insertval_comp_1, 0x40091EB860000000
  %insertval15 = insertvalue %complex_64bit zeroinitializer, float %sub.1, 0
  %insertval16 = insertvalue %complex_64bit %insertval15, float %sub.2, 1
  %insertval_comp_017 = extractvalue %complex_64bit %insertval16, 0
  %insertval_comp_118 = extractvalue %complex_64bit %insertval16, 1
  %abs.3 = call reassoc ninf nsz arcp contract afn float @hypotf(float %insertval_comp_017, float %insertval_comp_118), !llfort.type_idx !14
  %2 = getelementptr %complex_64bit, ptr %"test_$A", i64 0, i32 0, !llfort.type_idx !15
  %3 = getelementptr %complex_64bit, ptr %"test_$A", i64 0, i32 1, !llfort.type_idx !16
  %fetch.4 = load float, ptr %2, align 1, !tbaa !7, !llfort.type_idx !15
  %fetch.5 = load float, ptr %3, align 1, !tbaa !12, !llfort.type_idx !16
  %insertval = insertvalue %complex_64bit zeroinitializer, float %fetch.4, 0
  %insertval6 = insertvalue %complex_64bit %insertval, float %fetch.5, 1, !llfort.type_idx !3
  %insertval6_comp_0 = extractvalue %complex_64bit %insertval6, 0
  %insertval6_comp_1 = extractvalue %complex_64bit %insertval6, 1
  %abs.6 = call reassoc ninf nsz arcp contract afn float @hypotf(float %insertval6_comp_0, float %insertval6_comp_1), !llfort.type_idx !14
  %abs.7 = call reassoc ninf nsz arcp contract afn float @hypotf(float 0x40091EB860000000, float 0x40091EB860000000), !llfort.type_idx !14
  %"test_$FAILED.addr" = alloca ptr, align 8
  %"test_$A.addr" = alloca ptr, align 8
  store ptr @"test_$FAILED", ptr %"test_$FAILED.addr", align 8
  store ptr %"test_$A", ptr %"test_$A.addr", align 8
  %end.dir.temp = alloca i1, align 1
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @"test_$FAILED", i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"test_$A", %complex_64bit zeroinitializer, i32 1),
    "QUAL.OMP.OPERAND.ADDR"(ptr @"test_$FAILED", ptr %"test_$FAILED.addr"),
    "QUAL.OMP.OPERAND.ADDR"(ptr %"test_$A", ptr %"test_$A.addr"),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  %cmp = icmp ne i1 %temp.load, false
  br i1 %cmp, label %DIR.OMP.END.PARALLEL.1.split, label %DIR.OMP.PARALLEL.4

bb_new21_then:                                    ; preds = %DIR.OMP.PARALLEL.4
  store i32 -1, ptr %"test_$FAILED", align 4, !tbaa !17
  br label %DIR.OMP.END.PARALLEL.1.split

DIR.OMP.END.PARALLEL.1.split:                     ; preds = %DIR.OMP.PARALLEL.4, %bb_new21_then, %alloca_0
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL"() ]

  ret void

DIR.OMP.PARALLEL.4:                               ; preds = %alloca_0
  %"test_$FAILED" = load volatile ptr, ptr %"test_$FAILED.addr", align 8
  %"test_$A44" = load volatile ptr, ptr %"test_$A.addr", align 8
  %5 = getelementptr %complex_64bit, ptr %"test_$A44", i64 0, i32 0, !llfort.type_idx !19
  %6 = getelementptr %complex_64bit, ptr %"test_$A44", i64 0, i32 1, !llfort.type_idx !20
  %fetch.8 = load float, ptr %5, align 1, !tbaa !7, !llfort.type_idx !19
  %fetch.9 = load float, ptr %6, align 1, !tbaa !12, !llfort.type_idx !20
  %insertval33 = insertvalue %complex_64bit zeroinitializer, float %fetch.8, 0
  %insertval34 = insertvalue %complex_64bit %insertval33, float %fetch.9, 1, !llfort.type_idx !3
  %insertval_comp_035 = extractvalue %complex_64bit %insertval34, 0
  %sub.3 = fsub reassoc ninf nsz arcp contract afn float %insertval_comp_035, 0x40091EB860000000
  %insertval_comp_136 = extractvalue %complex_64bit %insertval34, 1
  %sub.4 = fsub reassoc ninf nsz arcp contract afn float %insertval_comp_136, 0x40091EB860000000
  %insertval37 = insertvalue %complex_64bit zeroinitializer, float %sub.3, 0
  %insertval38 = insertvalue %complex_64bit %insertval37, float %sub.4, 1
  %insertval_comp_039 = extractvalue %complex_64bit %insertval38, 0
  %insertval_comp_140 = extractvalue %complex_64bit %insertval38, 1
  %abs.10 = call reassoc ninf nsz arcp contract afn float @hypotf(float %insertval_comp_039, float %insertval_comp_140), !llfort.type_idx !14
  %7 = getelementptr %complex_64bit, ptr %"test_$A44", i64 0, i32 0, !llfort.type_idx !21
  %8 = getelementptr %complex_64bit, ptr %"test_$A44", i64 0, i32 1, !llfort.type_idx !22
  %fetch.11 = load float, ptr %7, align 1, !tbaa !7, !llfort.type_idx !21
  %fetch.12 = load float, ptr %8, align 1, !tbaa !12, !llfort.type_idx !22
  %insertval22 = insertvalue %complex_64bit zeroinitializer, float %fetch.11, 0
  %insertval24 = insertvalue %complex_64bit %insertval22, float %fetch.12, 1, !llfort.type_idx !3
  %insertval24_comp_0 = extractvalue %complex_64bit %insertval24, 0
  %insertval24_comp_1 = extractvalue %complex_64bit %insertval24, 1
  %abs.13 = call reassoc ninf nsz arcp contract afn float @hypotf(float %insertval24_comp_0, float %insertval24_comp_1), !llfort.type_idx !14
  %abs.14 = call reassoc ninf nsz arcp contract afn float @hypotf(float 0x40091EB860000000, float 0x40091EB860000000), !llfort.type_idx !14
  %add.2 = fadd reassoc ninf nsz arcp contract afn float %abs.13, %abs.14
  %rel.2 = fcmp reassoc ninf nsz arcp contract afn ogt float %abs.10, %add.2
  %int_zext30 = zext i1 %rel.2 to i32, !llfort.type_idx !4
  %int_zext32 = trunc i32 %int_zext30 to i1, !llfort.type_idx !23
  br i1 %int_zext32, label %bb_new21_then, label %DIR.OMP.END.PARALLEL.1.split
}

declare !llfort.intrin_id !24 !llfort.type_idx !25 i32 @for_set_fpe_(ptr nocapture readonly)

; Function Attrs: nofree
declare !llfort.intrin_id !26 !llfort.type_idx !27 i32 @for_set_reentrancy(ptr nocapture readonly) #1

; Function Attrs: nofree nosync nounwind memory(none)
declare !llfort.intrin_id !28 !llfort.type_idx !29 float @hypotf(float, float) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree nosync nounwind memory(none) }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!1}

!0 = !{i64 26}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i64 25}
!3 = !{i64 27}
!4 = !{i64 2}
!5 = !{i64 32}
!6 = !{i64 33}
!7 = !{!8, !8, i64 0}
!8 = !{!"ifx$unique_sym$1", !9, i64 0}
!9 = !{!"Fortran Data Symbol", !10, i64 0}
!10 = !{!"Generic Fortran Symbol", !11, i64 0}
!11 = !{!"ifx$root$1$MAIN__"}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$2", !9, i64 0}
!14 = !{i64 5}
!15 = !{i64 47}
!16 = !{i64 48}
!17 = !{!18, !18, i64 0}
!18 = !{!"ifx$unique_sym$3", !9, i64 0}
!19 = !{i64 60}
!20 = !{i64 61}
!21 = !{i64 75}
!22 = !{i64 76}
!23 = !{i64 59}
!24 = !{i32 97}
!25 = !{i64 28}
!26 = !{i32 98}
!27 = !{i64 30}
!28 = !{i32 623}
!29 = !{i64 86}
; end INTEL_CUSTOMIZATION
