; RUN: opt -S -passes=vplan-vec -vplan-force-vf=2 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s -check-prefix=HIR

; subroutine foo( a, b, n, cl )
;     implicit none
;
;     type i_ty
;         integer                 :: i_id
;         integer, allocatable    :: i_arr(:)
;     end type i_ty
;
;     type(i_ty), allocatable  :: cl
;
;     integer :: i, n, tmp
;     integer :: a(n), b(n)
;
;     !$omp simd lastprivate(cl)
;     do i = 1,n
;       cl%i_arr(i) = a(i) + b(i)
;       cl%i_id = a(i) + cl%i_id
;     end do
;
; end subroutine foo

; Incomming HIR
;
; <0>          BEGIN REGION { }
; <2>                %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR:IV(&((%"foo_$I.linear.iv")[0])1),  QUAL.OMP.LASTPRIVATE:F90_NONPOD(&((%"foo_$CL.lpriv")[0])&((@"%FOO$.btI_TY.omp.mold_ctor_deref")[0])&((@"%FOO$.btI_TY.omp.copy_assign_deref")[0])&((@"%FOO$.btI_TY.omp.dtor_deref")[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LIVEIN(&((%"foo_$B")[0])),  QUAL.OMP.LIVEIN(&((%"foo_$A")[0])),  QUAL.OMP.LIVEIN(&((%"foo_$N")[0])) ]
; <5>                %"foo_$CL_fetch.18" = (%"foo_$CL.lpriv")[0];
; <7>                %"foo_$CL_fetch.18.I_ARR$.addr_a0$_fetch.19" = (%"foo_$CL_fetch.18")[0].2.0;
; <10>               %"foo_$CL_fetch.18.I_ARR$.dim_info$.lower_bound$[]_fetch.20" = (%"foo_$CL_fetch.18")[0].2.6[0].2;
; <12>               %"foo_$CL_fetch.25.I_ID$.promoted" = (%"foo_$CL_fetch.18")[0].0;
; <15>               %add.429 = %"foo_$CL_fetch.25.I_ID$.promoted";
; <39>
; <39>               + DO i1 = 0, zext.i32.i64(%"foo_$N_fetch.3") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <simd> <ivdep>
; <20>               |   %"foo_$A[]_fetch.15" = (%"foo_$A")[i1];
; <22>               |   %"foo_$B[]_fetch.17" = (%"foo_$B")[i1];
; <25>               |   (%"foo_$CL_fetch.18.I_ARR$.addr_a0$_fetch.19")[i1 + 1] = %"foo_$A[]_fetch.15" + %"foo_$B[]_fetch.17";
; <26>               |   %add.429 = %add.429  +  %"foo_$A[]_fetch.15"; <Safe Reduction>
; <39>               + END LOOP
; <39>
; <33>               (%"foo_$I.linear.iv")[0] = %"foo_$N_fetch.3";
; <34>               (%"foo_$CL_fetch.18")[0].0 = %add.429;
; <37>               @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; <0>          END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%dyn_typ = type { ptr, ptr }
%"XDESC_1_0$ptr$" = type { %"QNCA_a0$ptr$rank1$", ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"QNCA_a0$ptr$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%dyn_typ_nopdt = type { ptr, ptr, ptr }
%"FOO$.btI_TY" = type <{ i32, [4 x i8], %"QNCA_a0$ptr$rank1$" }>
@"var$5" = internal global %"XDESC_1_0$ptr$" zeroinitializer

; Function Attrs: nounwind uwtable
define void @foo_(ptr noalias nocapture dereferenceable(4) %"foo_$A", ptr noalias nocapture dereferenceable(4) %"foo_$B", ptr noalias nocapture dereferenceable(4) %"foo_$N", ptr noalias dereferenceable(80) %"foo_$CL") local_unnamed_addr #0 {
; CHECK:         [[LPRIV_VEC:%.*]] = alloca <2 x ptr>, align 16
; CHECK-NEXT:    [[LPRIV_VEC_BASE_ADDR:%.*]] = getelementptr ptr, ptr [[LPRIV_VEC]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[LPRIV_VEC_BASE_ADDR_EXTRACT_1:%.*]] = extractelement <2 x ptr> [[LPRIV_VEC_BASE_ADDR]], i32 1
; CHECK-NEXT:    [[LPRIV_VEC_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x ptr> [[LPRIV_VEC_BASE_ADDR]], i32 0
; CHECK:  [[BB1:BB[0-9]+]]:
; CHECK:         call void @"%FOO$.btI_TY.omp.mold_ctor_deref"(ptr [[LPRIV_VEC_BASE_ADDR_EXTRACT_0]], ptr [[LPRIV:%.*]])
; CHECK-NEXT:    call void @"%FOO$.btI_TY.omp.mold_ctor_deref"(ptr [[LPRIV_VEC_BASE_ADDR_EXTRACT_1]], ptr [[LPRIV]])
; CHECK:  [[BB2:BB[0-9]+]]:
; CHECK:         call void @"%FOO$.btI_TY.omp.copy_assign_deref"(ptr [[LPRIV]], ptr [[LPRIV_VEC_BASE_ADDR_EXTRACT_1]])
; CHECK-NEXT:    call void @"%FOO$.btI_TY.omp.dtor_deref"(ptr [[LPRIV_VEC_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call void @"%FOO$.btI_TY.omp.dtor_deref"(ptr [[LPRIV_VEC_BASE_ADDR_EXTRACT_1]])
;
; HIR:        [[PRIV_MEM_BC0:%.*]] = &((ptr)([[PRIV_MEM0:%.*]])[0])
; HIR:        @"%FOO$.btI_TY.omp.mold_ctor_deref"(&((ptr)([[PRIV_MEM0]])[0]),  [[LPRIV:%.*]])
; HIR:        [[EXTRACT_1_0:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM_BC0]])[<i32 0, i32 1>]),  1
; HIR:        @"%FOO$.btI_TY.omp.mold_ctor_deref"([[EXTRACT_1_0]],  [[LPRIV]])
; HIR:        [[EXTRACT_1_120:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM_BC0]])[<i32 0, i32 1>]),  1
; HIR:        @"%FOO$.btI_TY.omp.copy_assign_deref"([[LPRIV]],  [[EXTRACT_1_120]])
; HIR:        @"%FOO$.btI_TY.omp.dtor_deref"(&((ptr)([[PRIV_MEM0]])[0]))
; HIR:        [[EXTRACT_1_130:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM_BC0]])[<i32 0, i32 1>]),  1
; HIR:        @"%FOO$.btI_TY.omp.dtor_deref"([[EXTRACT_1_130]])
;
DIR.OMP.SIMD.1:
  %"foo_$CL.lpriv" = alloca ptr, align 8
  call void @"%FOO$.btI_TY.omp.mold_ctor_deref"(ptr nonnull %"foo_$CL.lpriv", ptr nonnull %"foo_$CL") #5
  %"foo_$I.linear.iv" = alloca i32, align 8
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(48) getelementptr inbounds (%"XDESC_1_0$ptr$", ptr @"var$5", i64 0, i32 4), i8 0, i64 48, i1 false)
  %"foo_$N_fetch.3" = load i32, ptr %"foo_$N", align 1
  %rel.1.not22 = icmp slt i32 %"foo_$N_fetch.3", 1
  br i1 %rel.1.not22, label %DIR.OMP.END.SIMD.421, label %DIR.OMP.SIMD.132

omp.pdo.body5:                                    ; preds = %DIR.OMP.SIMD.134, %omp.pdo.body5
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.134 ], [ %indvars.iv.next, %omp.pdo.body5 ]
  %add.429 = phi i32 [ %"foo_$CL_fetch.25.I_ID$.promoted", %DIR.OMP.SIMD.134 ], [ %add.4, %omp.pdo.body5 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %"foo_$A[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"foo_$A", i64 %indvars.iv.next)
  %"foo_$A[]_fetch.15" = load i32, ptr %"foo_$A[]", align 1
  %"foo_$B[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"foo_$B", i64 %indvars.iv.next)
  %"foo_$B[]_fetch.17" = load i32, ptr %"foo_$B[]", align 1
  %add.3 = add nsw i32 %"foo_$B[]_fetch.17", %"foo_$A[]_fetch.15"
  %"foo_$CL_fetch.18.I_ARR$.addr_a0$_fetch.19[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$CL_fetch.18.I_ARR$.dim_info$.lower_bound$[]_fetch.20", i64 4, ptr elementtype(i32) %"foo_$CL_fetch.18.I_ARR$.addr_a0$_fetch.19", i64 %indvars.iv.next)
  store i32 %add.3, ptr %"foo_$CL_fetch.18.I_ARR$.addr_a0$_fetch.19[]", align 1
  %add.4 = add nsw i32 %add.429, %"foo_$A[]_fetch.15"
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.233, label %omp.pdo.body5

DIR.OMP.SIMD.132:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %"foo_$I.linear.iv", i32 0, i32 1, i32 1), "QUAL.OMP.LASTPRIVATE:F90_NONPOD.TYPED"(ptr %"foo_$CL.lpriv", ptr zeroinitializer, i32 1, ptr @"%FOO$.btI_TY.omp.mold_ctor_deref", ptr @"%FOO$.btI_TY.omp.copy_assign_deref", ptr @"%FOO$.btI_TY.omp.dtor_deref"), "QUAL.OMP.LIVEIN"(ptr %"foo_$B"), "QUAL.OMP.LIVEIN"(ptr %"foo_$A"), "QUAL.OMP.LIVEIN"(ptr %"foo_$N") ]
  br label %DIR.OMP.SIMD.134

DIR.OMP.SIMD.134:                                 ; preds = %DIR.OMP.SIMD.132
  %"foo_$CL_fetch.18" = load ptr, ptr %"foo_$CL.lpriv", align 8
  %"foo_$CL_fetch.18.I_ARR$.addr_a0$" = getelementptr inbounds %"FOO$.btI_TY", ptr %"foo_$CL_fetch.18", i64 0, i32 2, i32 0
  %"foo_$CL_fetch.18.I_ARR$.addr_a0$_fetch.19" = load ptr, ptr %"foo_$CL_fetch.18.I_ARR$.addr_a0$", align 1
  %"foo_$CL_fetch.18.I_ARR$.dim_info$.lower_bound$" = getelementptr inbounds %"FOO$.btI_TY", ptr %"foo_$CL_fetch.18", i64 0, i32 2, i32 6, i64 0, i32 2
  %"foo_$CL_fetch.18.I_ARR$.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$CL_fetch.18.I_ARR$.dim_info$.lower_bound$", i32 0)
  %"foo_$CL_fetch.18.I_ARR$.dim_info$.lower_bound$[]_fetch.20" = load i64, ptr %"foo_$CL_fetch.18.I_ARR$.dim_info$.lower_bound$[]", align 1
  %"foo_$CL_fetch.25.I_ID$.promoted" = load i32, ptr %"foo_$CL_fetch.18", align 1
  %wide.trip.count = zext i32 %"foo_$N_fetch.3" to i64
  br label %omp.pdo.body5

DIR.OMP.END.SIMD.233:                             ; preds = %omp.pdo.body5
  %add.4.lcssa = phi i32 [ %add.4, %omp.pdo.body5 ]
  store i32 %"foo_$N_fetch.3", ptr %"foo_$I.linear.iv", align 8
  store i32 %add.4.lcssa, ptr %"foo_$CL_fetch.18", align 1
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.233
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.pdo.cond

omp.pdo.cond: ; preds = %DIR.OMP.END.SIMD.3
  store ptr null, ptr %"foo_$CL", align 1
  br label %DIR.OMP.END.SIMD.421

DIR.OMP.END.SIMD.421:                             ; preds = %DIR.OMP.SIMD.1, %omp.pdo.cond
  ret void
}

; Function Attrs: nounwind uwtable
declare void @"%FOO$.btI_TY.omp.mold_ctor_deref"(ptr %dst, ptr %src)

; Function Attrs: nounwind uwtable
declare void @"%FOO$.btI_TY.omp.copy_assign_deref"(ptr %dst, ptr %src)

; Function Attrs: nounwind uwtable
declare void @"%FOO$.btI_TY.omp.dtor_deref"(ptr %old)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #4
