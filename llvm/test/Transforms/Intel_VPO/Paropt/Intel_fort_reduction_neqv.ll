; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL

; This test is used to check NEQV reduction type.
;  PROGRAM OMP_TEST
;
;          IMPLICIT NONE
;
;          LOGICAL F
;          LOGICAL R
;
;          R = F(1)
;  C$OMP     PARALLEL REDUCTION( .NEQV. : R )
;            R = R .NEQV. F( 2 )
;  C$OMP     END PARALLEL
;        END
;
;        LOGICAL FUNCTION F( X )
;
;          IMPLICIT NONE
;
;          INTEGER X
;
;          F = MOD( X, 2 ) .EQ. 0
;        END

source_filename = "reduction_neqv.f"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = internal unnamed_addr constant i32 2
@1 = internal unnamed_addr constant i32 1
@2 = internal unnamed_addr constant i32 2

define void @MAIN__() #0 {
alloca:
  %"var$1" = alloca [8 x i64], align 16
  %"omp_test_$R" = alloca i32, align 8
  br label %bb2

bb2:                                              ; preds = %alloca
  br label %bb3

bb3:                                              ; preds = %bb2
  %func_result = call i32 @for_set_reentrancy(i32* @0)
  br label %bb4

bb7:                                              ; preds = %bb8
  br label %bb9

bb5:                                              ; preds = %bb4
  br label %bb8

bb8:                                              ; preds = %bb5
  br label %bb7

bb9:                                              ; preds = %bb7
  %func_result2 = call i32 @f_(i32* @1)
  br label %bb6

bb6:                                              ; preds = %bb9
  store i32 %func_result2, i32* %"omp_test_$R"
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.NEQV"(i32* %"omp_test_$R") ]
  br label %bb10

; CHECK:  store i32 0, i32* %"omp_test_$R.red"

bb4:                                              ; preds = %bb3
  br label %bb5

bb14:                                             ; preds = %bb15
  br label %bb16

bb11:                                             ; preds = %bb10
  br label %bb12

bb12:                                             ; preds = %bb11
  br label %bb15

bb15:                                             ; preds = %bb12
  br label %bb14

bb16:                                             ; preds = %bb14
  %func_result4 = call i32 @f_(i32* @2)
  br label %bb13

bb13:                                             ; preds = %bb16
  %xor = xor i32 %"omp_test_$R_fetch", %func_result4
  store i32 %xor, i32* %"omp_test_$R"
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %bb1

bb10:                                             ; preds = %bb6
  %"omp_test_$R_fetch" = load i32, i32* %"omp_test_$R"
  br label %bb11

; CRITICAL:  %{{[0-9]+}} = load i32, i32* %"omp_test_$R.red"
; FASTRED:  %{{[0-9]+}} = load i32, i32* %"omp_test_$R.fast_red"
; ALL-NEXT:  %{{[0-9]+}} = load i32, i32* %"omp_test_$R"
; ALL-NEXT:  %{{[0-9]+}} = xor i32 %{{[0-9]+}}, %{{[0-9]+}}
; ALL-NEXT:  store i32 %{{[0-9]+}}, i32* %"omp_test_$R"
; FASTRED-NOT: __kmpc_atomic

bb1:                                              ; preds = %bb13
  ret void
}

declare i32 @for_set_reentrancy(i32*)

define i32 @f_(i32* noalias %"f_$X") #0 {
alloca:
  %"var$2" = alloca [8 x i64], align 16
  %"f_$F" = alloca i32, align 8
  br label %bb18

bb18:                                             ; preds = %alloca
  br label %bb19

bb19:                                             ; preds = %bb18
  %"f_$X_fetch" = load i32, i32* %"f_$X"
  %mod = srem i32 %"f_$X_fetch", 2
  %rel = icmp eq i32 %mod, 0
  %int_zext = zext i1 %rel to i32
  %rel1 = icmp ne i32 %int_zext, 0
  %slct = select i1 %rel1, i32 -1, i32 0
  store i32 %slct, i32* %"f_$F"
  br label %bb17

bb17:                                             ; preds = %bb19
  %"f_$F_fetch" = load i32, i32* %"f_$F"
  ret i32 %"f_$F_fetch"
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!omp_offload.info = !{}

; end INTEL_CUSTOMIZATION
