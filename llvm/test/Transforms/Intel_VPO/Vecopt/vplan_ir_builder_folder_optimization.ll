;; This test makes sure that in case of constant, compile-time evaluatable operands for
;; 'fcmp' instruction, we do not try set any IR-flags on the return-value of
;; Builder.CreateFCmp(), which would be a llvm::Value and not a llvm::Instruction.
;; We just check that the compiler does not crash trying to set the flag and the
;; constant computed is propagated to its use.


; TODO: enable cfg merge after private support enabled
; RUN: opt -S -VPlanDriver -vplan-force-vf=2 -vplan-enable-all-liveouts -vplan-enable-cfg-merge=0 %s | FileCheck %s
; CHECK:      vector.body:
; CHECK:        [[VEC_PHI:%.*]] = phi <2 x i64> [ <i64 0, i64 1>, %vector.ph ], [ [[VEC_PHI_NEXT:%.*]], %vector.body ]
; CHECK-NEXT:   [[ADD1:%.*]] = add <2 x i64> %vec.phi, <i64 1, i64 1>
; CHECK-NEXT:   [[OP1:%.*]] = shl nuw <2 x i64> [[ADD1]], <i64 32, i64 32>
; CHECK-NEXT:   [[OP2:%.*]] = or <2 x i64> [[OP1]], <i64 9187343239835811840, i64 9187343239835811840>
; CHECK-NEXT:   [[SEL1:%.*]] = select i1 true, <2 x i64> [[OP1]], <2 x i64> [[OP2]]
; CHECK-NEXT:   [[SEL2:%.*]] = select i1 true, <2 x i64> [[OP2]], <2 x i64> [[OP1]]
; CHECK-NEXT:   [[SEL3:%.*]] = select i1 true, <2 x i64> [[OP1]], <2 x i64> [[OP2]]
; CHECK-NEXT:   [[VEC_PHI_NEXT]] = add nuw <2 x i64> [[VEC_PHI]], <i64 2, i64 2>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define i32 @test_fcmp_instruction_folding(i1 %flag, float %fval) {
  br label %simd.begin.region
simd.begin.region:                                ; preds = %0
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %simd.loop
simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i64 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %add = add i64 %index, 1
  %op1 = shl nuw i64 %add, 32
  %op2 = or i64 %op1, 9187343239835811840
  ;; Constant-folder within IRBuilder returns <2 x i1> <true, true>, a llvm::Value
  ;; and not a llvm::Instruction. We should not try to set IR-flags on llvm::Value. The
  ;; use of this instruction should directly be replaced by 'true'
  %fcmp1 = fcmp une float 2.000000e+00, 3.000000e+00
  %fneg = fneg float 2.000000e+00
  %fadd = fadd float 2.000000e+00, 9.000000e+00
  %fcmp2 = fcmp une float 2.000000e+00, %fneg
  %fcmp3 = fcmp une float %fadd, %fneg
  %res1 = select i1 %fcmp1, i64 %op1, i64 %op2
  %res2 = select i1 %fcmp2, i64 %op2, i64 %op1
  %res3 = select i1 %fcmp3, i64 %op1, i64 %op2
  br label %simd.loop.exit
simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i64 %index, 1
  %vl.cond = icmp ult i64 %indvar, 2
  br i1 %vl.cond, label %simd.loop, label %simd.end.region
simd.end.region:                                  ; preds = %simd.loop.exit
  %fcmp1.lcssa = phi i1 [ %fcmp1, %simd.loop.exit ]
  %fcmp2.lcssa = phi i1 [ %fcmp2, %simd.loop.exit ]
  %fcmp3.lcssa = phi i1 [ %fcmp3, %simd.loop.exit ]
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return
return:                                           ; preds = %simd.end.region
  %retval = select i1 %fcmp1.lcssa, i32 3, i32 1
  ret i32 %retval
}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
