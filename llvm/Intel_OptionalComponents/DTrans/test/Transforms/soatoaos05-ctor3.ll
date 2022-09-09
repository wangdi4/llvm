; UNSUPPORTED: enable-opaque-pointers

; CMPLRLLVM-9285 & CMPLRLLVM-9343 (Klocworks issue): SOAToAOS shouldn't
; crash when it doesn't find use of stored pointer as Call instruction
; (constructor). Commenting out "call void @_ZN3ArrIPiEC2Ei()" call in
; "_ZN1FC2Ev" to reproduce the crash. The nullptr access was reported by
; klocworks.

; RUN: opt < %s -whole-program-assume -disable-output                                                                   \
; RUN:          -passes='require<dtransanalysis>,require<soatoaos-approx>,require<soatoaos-struct-methods>'             \
; RUN:          -dtrans-soatoaos-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaos-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaos-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaos-method-call-site-comparison=ctor                                                       \
; RUN:          -dtrans-soatoaos-array-ctor=_ZN3ArrIPiEC2Ei                                                             \
; RUN:          -dtrans-soatoaos-array-ctor=_ZN3ArrIPfEC2Ei                                                             \
; RUN:          2>/dev/null

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { %struct.Arr*, %struct.Arr.0* }
%struct.Arr = type <{ i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i32, [4 x i8], float**, i32, [4 x i8] }>

declare i32 @__gxx_personality_v0(...)

declare noalias nonnull i8* @_Znwm(i64)

define hidden void @_ZN1FC2Ev(%class.F* nocapture %arg) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
bb:
  %tmp = tail call i8* @_Znwm(i64 24)
  %tmp1 = bitcast i8* %tmp to %struct.Arr*
  %tmp2 = getelementptr inbounds %class.F, %class.F* %arg, i64 0, i32 0
  store %struct.Arr* %tmp1, %struct.Arr** %tmp2, align 8
;
;  Commenting out the below call to reproduce Segmentation fault.
;  tail call void @_ZN3ArrIPiEC2Ei(%struct.Arr* %tmp1, i32 1)
;
  %tmp3 = tail call i8* @_Znwm(i64 24)
  %tmp4 = bitcast i8* %tmp3 to %struct.Arr.0*
  tail call void @_ZN3ArrIPfEC2Ei(%struct.Arr.0* %tmp4, i32 1)
  %tmp5 = getelementptr inbounds %class.F, %class.F* %arg, i64 0, i32 1
  store %struct.Arr.0* %tmp4, %struct.Arr.0** %tmp5, align 8
  ret void
}

declare hidden void @_ZN3ArrIPfEC2Ei(%struct.Arr.0*, i32)
declare hidden void @_ZN3ArrIPiEC2Ei(%struct.Arr*, i32)
