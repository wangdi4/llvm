; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-transform  -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-transform)' -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; Test src:
;         subroutine test()
;         integer :: i, j
;         !$omp do
;         do i = 1, 100
;           !$omp tile sizes(8)
;           do j = 1, 48
;             call bar(i,j)
;           end do
;         end do
;         end subroutine

; CHECK-DAG:  @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %tmp7, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %tmp6, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %tmp8, i32 0), "QUAL.OMP.LIVEIN"(ptr %tmp1) ]
; CHECK-DAG:  FLOOR.PREHEAD
; CHECK-DAG:  FLOOR.HEAD
; CHECK-DAG:  FLOOR.LATCH

; ModuleID = 'Intel_fort_do1__tile1.ll'
source_filename = "do1__tile1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @quux() #0 {
bb:
  %tmp = alloca [8 x i64], align 16, !llfort.type_idx !1
  %tmp1 = alloca i32, align 8, !llfort.type_idx !2
  %tmp2 = alloca i32, align 8, !llfort.type_idx !3
  %tmp3 = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %tmp3, align 4, !tbaa !5
  %tmp4 = alloca i32, align 4, !llfort.type_idx !4
  store i32 100, ptr %tmp4, align 4, !tbaa !5
  %tmp5 = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %tmp5, align 4, !tbaa !5
  %tmp6 = alloca i32, align 4, !llfort.type_idx !4
  %tmp7 = alloca i32, align 4, !llfort.type_idx !4
  store i32 0, ptr %tmp7, align 4, !tbaa !5
  %tmp8 = alloca i32, align 4, !llfort.type_idx !4
  %tmp9 = load i32, ptr %tmp4, align 4, !tbaa !5
  %tmp10 = load i32, ptr %tmp3, align 4, !tbaa !5
  %tmp11 = sub nsw i32 %tmp9, %tmp10
  %tmp12 = load i32, ptr %tmp5, align 4, !tbaa !5
  %tmp13 = add nsw i32 %tmp11, %tmp12
  %tmp14 = load i32, ptr %tmp5, align 4, !tbaa !5
  %tmp15 = sdiv i32 %tmp13, %tmp14
  %tmp16 = sub nsw i32 %tmp15, 1
  store i32 %tmp16, ptr %tmp8, align 4, !tbaa !5
  br label %bb59

bb59:  ; preds = %bb
  %tmp60 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1),
     "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %tmp7, i32 0, i32 1),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %tmp6, i32 0),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %tmp8, i32 0),
     "QUAL.OMP.LIVEIN"(ptr %tmp1) ]
  %tmp61 = load i32, ptr %tmp7, align 4, !tbaa !5
  store i32 %tmp61, ptr %tmp6, align 4, !tbaa !5
  br label %bb17

bb17:  ; preds = %bb56, %bb59
  %tmp18 = load i32, ptr %tmp6, align 4, !tbaa !5
  %tmp19 = load i32, ptr %tmp8, align 4, !tbaa !5
  %tmp20 = icmp sle i32 %tmp18, %tmp19
  br i1 %tmp20, label %bb21, label %bb62

bb21:  ; preds = %bb17
  %tmp22 = load i32, ptr %tmp6, align 4, !tbaa !5
  %tmp23 = load i32, ptr %tmp5, align 4, !tbaa !5
  %tmp24 = mul nsw i32 %tmp22, %tmp23
  %tmp25 = load i32, ptr %tmp3, align 4, !tbaa !5
  %tmp26 = add nsw i32 %tmp24, %tmp25
  store i32 %tmp26, ptr %tmp2, align 8, !tbaa !8
  %tmp27 = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %tmp27, align 4, !tbaa !5
  %tmp28 = alloca i32, align 4, !llfort.type_idx !4
  store i32 48, ptr %tmp28, align 4, !tbaa !5
  %tmp29 = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %tmp29, align 4, !tbaa !5
  %tmp30 = alloca i32, align 4, !llfort.type_idx !4
  %tmp31 = alloca i32, align 4, !llfort.type_idx !4
  store i32 0, ptr %tmp31, align 4, !tbaa !5
  %tmp32 = alloca i32, align 4, !llfort.type_idx !4
  %tmp33 = load i32, ptr %tmp28, align 4, !tbaa !5
  %tmp34 = load i32, ptr %tmp27, align 4, !tbaa !5
  %tmp35 = sub nsw i32 %tmp33, %tmp34
  %tmp36 = load i32, ptr %tmp29, align 4, !tbaa !5
  %tmp37 = add nsw i32 %tmp35, %tmp36
  %tmp38 = load i32, ptr %tmp29, align 4, !tbaa !5
  %tmp39 = sdiv i32 %tmp37, %tmp38
  %tmp40 = sub nsw i32 %tmp39, 1
  store i32 %tmp40, ptr %tmp32, align 4, !tbaa !5
  br label %bb53

bb53:  ; preds = %bb21
  %tmp54 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 8),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %tmp30, i32 0),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %tmp32, i32 0),
     "QUAL.OMP.LIVEIN"(ptr %tmp1),
     "QUAL.OMP.LIVEIN"(ptr %tmp2),
     "QUAL.OMP.LIVEIN"(ptr %tmp31) ]
  %tmp55 = load i32, ptr %tmp31, align 4, !tbaa !5
  store i32 %tmp55, ptr %tmp30, align 4, !tbaa !5
  br label %bb41

bb41:  ; preds = %bb45, %bb53
  %tmp42 = load i32, ptr %tmp30, align 4, !tbaa !5
  %tmp43 = load i32, ptr %tmp32, align 4, !tbaa !5
  %tmp44 = icmp sle i32 %tmp42, %tmp43
  br i1 %tmp44, label %bb45, label %bb56

bb45:  ; preds = %bb41
  %tmp46 = load i32, ptr %tmp30, align 4, !tbaa !5
  %tmp47 = load i32, ptr %tmp29, align 4, !tbaa !5
  %tmp48 = mul nsw i32 %tmp46, %tmp47
  %tmp49 = load i32, ptr %tmp27, align 4, !tbaa !5
  %tmp50 = add nsw i32 %tmp48, %tmp49
  store i32 %tmp50, ptr %tmp1, align 8, !tbaa !10
  call void @bar(ptr %tmp2, ptr %tmp1), !llfort.type_idx !12
  %tmp51 = load i32, ptr %tmp30, align 4, !tbaa !5
  %tmp52 = add nsw i32 %tmp51, 1
  store i32 %tmp52, ptr %tmp30, align 4, !tbaa !5
  br label %bb41

bb56:  ; preds = %bb41
  call void @llvm.directive.region.exit(token %tmp54) [ "DIR.OMP.END.TILE"() ]
  %tmp57 = load i32, ptr %tmp6, align 4, !tbaa !5
  %tmp58 = add nsw i32 %tmp57, 1
  store i32 %tmp58, ptr %tmp6, align 4, !tbaa !5
  br label %bb17

bb62:  ; preds = %bb17
  call void @llvm.directive.region.exit(token %tmp60) [ "DIR.OMP.END.LOOP"() ]
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind uwtable
define internal void @bar(ptr %arg, ptr %arg1) #2 {
bb:
  call void (...) @spam(ptr %arg, ptr %arg1)
  ret void

}

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare void @spam(...)

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 19}
!2 = !{i64 24}
!3 = !{i64 25}
!4 = !{i64 2}
!5 = !{!6, !6, i64 0}
!6 = !{!"Generic Fortran Symbol", !7, i64 0}
!7 = !{!"ifx$root$1$test_"}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$1", !6, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$2", !6, i64 0}
!12 = !{i64 27}
; end INTEL_CUSTOMIZATION
