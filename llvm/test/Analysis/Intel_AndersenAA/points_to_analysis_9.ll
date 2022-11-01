; RUN: opt < %s -passes='require<anders-aa>'  -disable-output 2>/dev/null
; Test Andersens analysis that caused crash when constant of vectorType was
; used in instructions. These Vector type patterns appear when Andersens 
; analysis is enabled at LTO. 

%struct.s2 = type { %struct.s2*, %struct.s2*, i32 }
@x1 = common global %struct.s2 zeroinitializer, align 8
@x2 = common global %struct.s2 zeroinitializer, align 8

; Function Attrs: nounwind uwtable
define void @c3_5() {
  store <2 x %struct.s2*> <%struct.s2* @x1, %struct.s2* @x2>, <2 x %struct.s2*>* bitcast (%struct.s2* @x2 to <2 x %struct.s2*>*), align 8
  ret void
}
