; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-transform  -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-transform)' -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; Test src:
;
;         subroutine test()
;         integer :: i, j
;         !$omp tile sizes(7)
;         !$omp tile sizes(2)
;         !$omp tile sizes(4)
;         do i = 1, 100
;           call bar(i)
;         end do
;         end subroutine

; CHECK-DAG: FLOOR.PREHEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.HEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.LATCH{{[0-9]*}}: ; preds = %FLOOR.HEAD{{[0-9]*}}

; CHECK-DAG: FLOOR.PREHEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.HEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.LATCH{{[0-9]*}}: ; preds = %FLOOR.HEAD{{[0-9]*}}

; CHECK-DAG: FLOOR.PREHEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.HEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.LATCH{{[0-9]*}}

; CHECK-NOT:  call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),

; ModuleID = 'Intel_fort_tile1+tile1+tile1.ll'
source_filename = "tile1+tile1+tile1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @spam() #0 {
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
  br label %bb33

bb33:  ; preds = %bb
  %tmp34 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 7) ]
  br label %bb35

bb35:  ; preds = %bb33
  %tmp36 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 2) ]
  br label %bb30

bb30:  ; preds = %bb35
  %tmp31 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 4),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %tmp6, i32 0),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %tmp8, i32 0),
     "QUAL.OMP.LIVEIN"(ptr %tmp7) ]
  %tmp32 = load i32, ptr %tmp7, align 4, !tbaa !5
  store i32 %tmp32, ptr %tmp6, align 4, !tbaa !5
  br label %bb17

bb17:  ; preds = %bb21, %bb30
  %tmp18 = load i32, ptr %tmp6, align 4, !tbaa !5
  %tmp19 = load i32, ptr %tmp8, align 4, !tbaa !5
  %tmp20 = icmp sle i32 %tmp18, %tmp19
  br i1 %tmp20, label %bb21, label %bb29

bb21:  ; preds = %bb17
  %tmp22 = load i32, ptr %tmp6, align 4, !tbaa !5
  %tmp23 = load i32, ptr %tmp5, align 4, !tbaa !5
  %tmp24 = mul nsw i32 %tmp22, %tmp23
  %tmp25 = load i32, ptr %tmp3, align 4, !tbaa !5
  %tmp26 = add nsw i32 %tmp24, %tmp25
  store i32 %tmp26, ptr %tmp2, align 8, !tbaa !8
  call void @baz(ptr %tmp2), !llfort.type_idx !10
  %tmp27 = load i32, ptr %tmp6, align 4, !tbaa !5
  %tmp28 = add nsw i32 %tmp27, 1
  store i32 %tmp28, ptr %tmp6, align 4, !tbaa !5
  br label %bb17

bb29:  ; preds = %bb17
  call void @llvm.directive.region.exit(token %tmp31) [ "DIR.OMP.END.TILE"() ]
  call void @llvm.directive.region.exit(token %tmp36) [ "DIR.OMP.END.TILE"() ]
  call void @llvm.directive.region.exit(token %tmp34) [ "DIR.OMP.END.TILE"() ]
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind uwtable
define internal void @baz(ptr %arg) #2 {
bb:
  call void (...) @snork(ptr %arg)
  ret void

}

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare void @snork(...)

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
!10 = !{i64 27}
; end INTEL_CUSTOMIZATION
