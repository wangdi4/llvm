; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 -disable-output | FileCheck %s
;
; Make sure reroll doesn't happen with refs with different types.
;
; In the following examples, the source types of the two lval refs are different.
; First one has implicit bit-cast from (%Vector3Storage *) to (float *)
;
; (float*)(%biasVal)[2 * i1]
;    Src type: %VecType = type { %Vector3Storage }, dest type: float
; (%biasVal)[2 * i1 + 1]
;    Src type: float, dest type: float
;
;
; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:              |   (float*)(%biasVal)[2 * i1] = 0.000000e+00;
; CHECK:              |   (%biasVal)[2 * i1 + 1] = 0.000000e+00;
; CHECK:              + END LOOP
; CHECK:        END REGION

%VecType = type { %Vector3Storage }
%Vector3Storage = type { %union }
%union = type { %struct }
%struct = type { float, float, float }
define void @test() {
entry:
  %biasVal = alloca %"VecType", align 4
  br label %init
init:
  %inv = phi i64 [0, %entry], [%inv.next, %init]
  %offset_0 = getelementptr inbounds %VecType, ptr %biasVal, i64 %inv
  store float 0.000000e+00, ptr %offset_0, align 4
  %second = or i64 %inv, 1
  %offset_1 = getelementptr inbounds float, ptr %biasVal, i64 %second
  store float 0.000000e+00, ptr %offset_1, align 4
  %inv.next = add nuw nsw i64 %inv, 2
  %cond = icmp slt i64 %inv.next, 10
  br i1 %cond, label %init, label %end
end:
  ret void
}
