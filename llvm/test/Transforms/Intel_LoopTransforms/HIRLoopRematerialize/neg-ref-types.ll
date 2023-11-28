; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-rematerialize,print<hir>" -hir-loop-rematerialize-tc-lb=1 -aa-pipeline="basic-aa" -hir-allow-loop-materialization-regions=true -disable-output < %s 2>&1 | FileCheck %s

; Verify that loop rematerialization doesn't happen as the types of 3 refs,
; (%biasVal)[0],(%biasVal)[1],(%biasVal)[2] are not the same.
; If only base CE types are checked, they are all same as 'ptr'.
; Details:
; (float*)(LINEAR ptr %biasVal)[i64 0]
;    - SrcType  : %VecType = type { %Vector3Storage }
;    - Dest type: float
; (LINEAR ptr %biasVal)[i64 1]
;    - SrcType  : float
;    - Dest type: float


; CHECK: Function: test
;
; CHECK:       BEGIN REGION { }
; CHECK:             (%biasVal)[0].0.0.0.0 = 0.000000e+00;
; CHECK:             (%biasVal)[1] = 0.000000e+00;
; CHECK:             (%biasVal)[2] = 0.000000e+00;
; CHECK:       END REGION


%VecType = type { %Vector3Storage }
%Vector3Storage = type { %union }
%union = type { %struct }
%struct = type { float, float, float }
define void @test() {
entry:
  %biasVal = alloca %"VecType", align 4
  br label %init
init:
  store float 0.000000e+00, ptr %biasVal, align 4
  %offset_1 = getelementptr inbounds float, ptr %biasVal, i64 1
  store float 0.000000e+00, ptr %offset_1, align 4
  %offset_2 = getelementptr inbounds float, ptr %biasVal, i64 2
  store float 0.000000e+00, ptr %offset_2, align 4
  br label %end
end:
  ret void
}
