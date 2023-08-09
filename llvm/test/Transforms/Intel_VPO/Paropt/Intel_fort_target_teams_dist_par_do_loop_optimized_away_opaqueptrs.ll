; INTEL_CUSTOMIZATION

; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S -pass-remarks-missed=openmp -switch-to-offload %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S -pass-remarks-missed=openmp -switch-to-offload %s 2>&1 | FileCheck %s

; Check for remarks about the loop construct being optimized away and ignored.
; CHECK: remark:{{.*}}distribute parallel loop construct's associated loop was optimized away.

; ! Reproducer source
; contains
;   SUBROUTINE a
;     integer :: b, c 
;     !$OMP TARGET TEAMS DISTRIBUTE PARALLEL DO COLLAPSE(2)
;     DO iN_X = 1, b
;        DO iN_E = 1, c
;              CALL d 
;        END DO
;     END DO
;   END  
;   SUBROUTINE d
;      e = f ()
;   END  
;   FUNCTION f 
;      !$OMP DECLARE TARGET
;      END  
; END 
; ModuleID = 'reproducer.f90'
source_filename = "reproducer.f90"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
alloca_0:
  ret void
}

; Function Attrs: noinline nounwind uwtable
define void @"_unnamed_main$$_IP_a_"() local_unnamed_addr #0 {
DIR.OMP.TARGET.3:
  %"a$IN_E$_1" = alloca i32, align 8, !llfort.type_idx !3
  %"a$IN_X$_1" = alloca i32, align 8, !llfort.type_idx !4
  %"a$C$_1" = alloca i32, align 8, !llfort.type_idx !5
  %"a$B$_1" = alloca i32, align 8, !llfort.type_idx !6
  %"ascast$a$IN_X$_1" = addrspacecast ptr %"a$IN_X$_1" to ptr addrspace(4)
  %"ascast$a$IN_E$_1" = addrspacecast ptr %"a$IN_E$_1" to ptr addrspace(4)
  %"ascast$a$B$_1" = addrspacecast ptr %"a$B$_1" to ptr addrspace(4)
  %"ascast$a$C$_1" = addrspacecast ptr %"a$C$_1" to ptr addrspace(4)
  %temp = alloca i32, align 4, !llfort.type_idx !7
  %do.start = addrspacecast ptr %temp to ptr addrspace(4)
  store i32 1, ptr addrspace(4) %do.start, align 4, !tbaa !8
  %"ascast$a$C$_1_fetch.12" = load i32, ptr addrspace(4) %"ascast$a$C$_1", align 8, !tbaa !11
  %temp2 = alloca i32, align 4, !llfort.type_idx !7
  %do.step = addrspacecast ptr %temp2 to ptr addrspace(4)
  store i32 1, ptr addrspace(4) %do.step, align 4, !tbaa !8
  %temp3 = alloca i32, align 4, !llfort.type_idx !7
  %do.norm.lb = addrspacecast ptr %temp3 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %do.norm.lb, align 4, !tbaa !8
  %temp4 = alloca i32, align 4, !llfort.type_idx !7
  %do.norm.ub = addrspacecast ptr %temp4 to ptr addrspace(4)
  %do.start_fetch.14 = load i32, ptr addrspace(4) %do.start, align 4, !tbaa !8
  %sub.3 = sub nsw i32 %"ascast$a$C$_1_fetch.12", %do.start_fetch.14
  %do.step_fetch.15 = load i32, ptr addrspace(4) %do.step, align 4, !tbaa !8
  %add.3 = add nsw i32 %sub.3, %do.step_fetch.15
  %div.2 = sdiv i32 %add.3, %do.step_fetch.15
  %sub.4 = add nsw i32 %div.2, -1
  store volatile i32 %sub.4, ptr addrspace(4) %do.norm.ub, align 4, !tbaa !8
  %temp5 = alloca i32, align 4, !llfort.type_idx !7
  %do.norm.iv = addrspacecast ptr %temp5 to ptr addrspace(4)
  %temp6 = alloca i32, align 4, !llfort.type_idx !7
  %omp.pdo.start = addrspacecast ptr %temp6 to ptr addrspace(4)
  store i32 1, ptr addrspace(4) %omp.pdo.start, align 4, !tbaa !8
  %"ascast$a$B$_1_fetch.1" = load i32, ptr addrspace(4) %"ascast$a$B$_1", align 8, !tbaa !13
  %temp8 = alloca i32, align 4, !llfort.type_idx !7
  %omp.pdo.step = addrspacecast ptr %temp8 to ptr addrspace(4)
  store i32 1, ptr addrspace(4) %omp.pdo.step, align 4, !tbaa !8
  %temp9 = alloca i32, align 4, !llfort.type_idx !7
  %omp.pdo.norm.iv = addrspacecast ptr %temp9 to ptr addrspace(4)
  %temp10 = alloca i32, align 4, !llfort.type_idx !7
  %omp.pdo.norm.lb = addrspacecast ptr %temp10 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %omp.pdo.norm.lb, align 4, !tbaa !8
  %temp11 = alloca i32, align 4, !llfort.type_idx !7
  %omp.pdo.norm.ub = addrspacecast ptr %temp11 to ptr addrspace(4)
  %omp.pdo.start_fetch.3 = load i32, ptr addrspace(4) %omp.pdo.start, align 4, !tbaa !8
  %sub.1 = sub nsw i32 %"ascast$a$B$_1_fetch.1", %omp.pdo.start_fetch.3
  %omp.pdo.step_fetch.4 = load i32, ptr addrspace(4) %omp.pdo.step, align 4, !tbaa !8
  %add.1 = add nsw i32 %sub.1, %omp.pdo.step_fetch.4
  %div.1 = sdiv i32 %add.1, %omp.pdo.step_fetch.4
  %sub.2 = add nsw i32 %div.1, -1
  store volatile i32 %sub.2, ptr addrspace(4) %omp.pdo.norm.ub, align 4, !tbaa !8
  %end.dir.temp64 = alloca i1, align 1
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
 "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %"ascast$a$C$_1", i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %"ascast$a$B$_1", i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %"ascast$a$IN_E$_1", i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %"ascast$a$IN_X$_1", i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %do.norm.iv, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.pdo.norm.iv, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %do.norm.ub, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %do.norm.lb, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %do.step, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %do.start, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.pdo.norm.lb, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.pdo.step, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.pdo.start, i32 0, i32 1),
 "QUAL.OMP.OFFLOAD.NDRANGE"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, ptr addrspace(4) %do.norm.ub, i32 0),
 "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp64) ]

  br label %DIR.OMP.TARGET.367

DIR.OMP.TARGET.367:                               ; preds = %DIR.OMP.TARGET.2
  %temp.load65 = load volatile i1, ptr %end.dir.temp64, align 1
  br i1 %temp.load65, label %DIR.OMP.END.TARGET.7, label %DIR.OMP.TEAMS.10

omp.pdo.cond6:                                    ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.5, %omp.pdo.body7
  %omp.pdo.norm.iv_fetch.7 = load volatile i32, ptr addrspace(4) %omp.pdo.norm.iv, align 4, !tbaa !8
  %omp.pdo.norm.ub_fetch.8 = load volatile i32, ptr addrspace(4) %omp.pdo.norm.ub, align 4, !tbaa !8
  %rel.1.not = icmp sgt i32 %omp.pdo.norm.iv_fetch.7, %omp.pdo.norm.ub_fetch.8
  br i1 %rel.1.not, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4.loopexit, label %omp.pdo.body7

omp.pdo.body7:                                    ; preds = %omp.pdo.cond6
  %omp.pdo.norm.iv_fetch.9 = load volatile i32, ptr addrspace(4) %omp.pdo.norm.iv, align 4, !tbaa !8
  %omp.pdo.step_fetch.10 = load i32, ptr addrspace(4) %omp.pdo.step, align 4, !tbaa !8
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.9, %omp.pdo.step_fetch.10
  %omp.pdo.start_fetch.11 = load i32, ptr addrspace(4) %omp.pdo.start, align 4, !tbaa !8
  %add.2 = add nsw i32 %mul.1, %omp.pdo.start_fetch.11
  store i32 %add.2, ptr addrspace(4) %"ascast$a$IN_X$_1", align 8, !tbaa !15
  %do.norm.lb_fetch.17 = load i32, ptr addrspace(4) %do.norm.lb, align 4, !tbaa !8
  store volatile i32 %do.norm.lb_fetch.17, ptr addrspace(4) %do.norm.iv, align 4, !tbaa !8
  %do.norm.iv_fetch.18 = load volatile i32, ptr addrspace(4) %do.norm.iv, align 4, !tbaa !8
  %do.norm.ub_fetch.19 = load volatile i32, ptr addrspace(4) %do.norm.ub, align 4, !tbaa !8
  %rel.2.not = icmp sgt i32 %do.norm.iv_fetch.18, %do.norm.ub_fetch.19
  call void @llvm.assume(i1 %rel.2.not)
  %omp.pdo.norm.iv_fetch.24 = load volatile i32, ptr addrspace(4) %omp.pdo.norm.iv, align 4, !tbaa !8
  %add.6 = add nsw i32 %omp.pdo.norm.iv_fetch.24, 1
  store volatile i32 %add.6, ptr addrspace(4) %omp.pdo.norm.iv, align 4, !tbaa !8
  br label %omp.pdo.cond6

DIR.OMP.END.DISTRIBUTE.PARLOOP.4.loopexit:        ; preds = %omp.pdo.cond6
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4

DIR.OMP.END.DISTRIBUTE.PARLOOP.4:                 ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4.loopexit, %DIR.OMP.DISTRIBUTE.PARLOOP.1372
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.468

DIR.OMP.END.DISTRIBUTE.PARLOOP.468:               ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %DIR.OMP.END.TEAMS.5

DIR.OMP.END.TEAMS.5:                              ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.468, %DIR.OMP.TEAMS.1071
  br label %DIR.OMP.END.TEAMS.569

DIR.OMP.END.TEAMS.569:                            ; preds = %DIR.OMP.END.TEAMS.5
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TARGET.7

DIR.OMP.END.TARGET.7:                             ; preds = %DIR.OMP.TARGET.367, %DIR.OMP.END.TEAMS.569
  br label %DIR.OMP.END.TARGET.6

DIR.OMP.END.TARGET.6:                             ; preds = %DIR.OMP.END.TARGET.7
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.770

DIR.OMP.END.TARGET.770:                           ; preds = %DIR.OMP.END.TARGET.6
  ret void

DIR.OMP.DISTRIBUTE.PARLOOP.5:                     ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.1372
  %omp.pdo.norm.lb_fetch.6 = load i32, ptr addrspace(4) %omp.pdo.norm.lb, align 4, !tbaa !8
  store volatile i32 %omp.pdo.norm.lb_fetch.6, ptr addrspace(4) %omp.pdo.norm.iv, align 4, !tbaa !8
  br label %omp.pdo.cond6

DIR.OMP.TEAMS.10:                                 ; preds = %DIR.OMP.TARGET.367
  %end.dir.temp61 = alloca i1, align 1
  br label %DIR.OMP.TEAMS.8

DIR.OMP.TEAMS.8:                                  ; preds = %DIR.OMP.TEAMS.10
  br label %DIR.OMP.TEAMS.9

DIR.OMP.TEAMS.9:                                  ; preds = %DIR.OMP.TEAMS.8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %do.norm.ub, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %do.norm.lb, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) null, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) null, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.step, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) null, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) null, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %do.norm.iv, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.pdo.norm.iv, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %"ascast$a$IN_E$_1", i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %"ascast$a$IN_X$_1", i32 0, i32 1),
 "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp61),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %do.step, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %do.start, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %"ascast$a$C$_1", i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %"ascast$a$B$_1", i32 0, i32 1) ]

  br label %DIR.OMP.TEAMS.1071

DIR.OMP.TEAMS.1071:                               ; preds = %DIR.OMP.TEAMS.9
  %temp.load62 = load volatile i1, ptr %end.dir.temp61, align 1
  br i1 %temp.load62, label %DIR.OMP.END.TEAMS.5, label %DIR.OMP.DISTRIBUTE.PARLOOP.13

DIR.OMP.DISTRIBUTE.PARLOOP.13:                    ; preds = %DIR.OMP.TEAMS.1071
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.11

DIR.OMP.DISTRIBUTE.PARLOOP.11:                    ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.13
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.12

DIR.OMP.DISTRIBUTE.PARLOOP.12:                    ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.11
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
 "QUAL.OMP.COLLAPSE"(i32 2),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) null, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) null, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$a$IN_E$_1", i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$a$IN_X$_1", i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) null, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) null, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.step, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.pdo.start, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %do.norm.lb, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.pdo.norm.lb, i32 0, i32 1),
 "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %omp.pdo.norm.iv, i32 0, ptr addrspace(4) %do.norm.iv, i32 0),
 "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %omp.pdo.norm.ub, i32 0, ptr addrspace(4) %do.norm.ub, i32 0),
 "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"(i1 true),
 "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp),
 "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$a$C$_1", i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %"ascast$a$B$_1", i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %do.step, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %do.start, i32 0, i32 1) ]

  br label %DIR.OMP.DISTRIBUTE.PARLOOP.1372

DIR.OMP.DISTRIBUTE.PARLOOP.1372:                  ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.12
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.4, label %DIR.OMP.DISTRIBUTE.PARLOOP.5
}

; Function Attrs: nounwind uwtable
define spir_func void @"_unnamed_main$$_IP_d_"() local_unnamed_addr #1 {
alloca_2:
  unreachable
}

; Function Attrs: nounwind uwtable
define spir_func float @"_unnamed_main$$_IP_f_"() local_unnamed_addr #1 {
alloca_3:
  ret float undef
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #3

attributes #0 = { noinline nounwind uwtable "contains-openmp-target"="true" "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "openmp-target-declare"="true" }
attributes #2 = { nounwind }
attributes #3 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 77, i32 -683072239, !"_unnamed_main$$_IP_a_", i32 4, i32 0, i32 0}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"openmp-device", i32 50}
!3 = !{i64 27}
!4 = !{i64 28}
!5 = !{i64 29}
!6 = !{i64 30}
!7 = !{i64 2}
!8 = !{!9, !9, i64 0}
!9 = !{!"Generic Fortran Symbol", !10, i64 0}
!10 = !{!"ifx$root$1$_unnamed_main$$_IP_a_"}
!11 = !{!12, !12, i64 0}
!12 = !{!"ifx$unique_sym$3", !9, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"ifx$unique_sym$1", !9, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"ifx$unique_sym$2", !9, i64 0}
