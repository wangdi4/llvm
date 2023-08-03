; RUN: opt < %s -passes='require<anders-aa>'  -disable-output 2>/dev/null
; Test Andersens analysis that caused crash when constant of vectorType was
; used in instructions. These Vector type patterns appear when Andersens 
; analysis is enabled at LTO. 

%struct.s2 = type { ptr, ptr, i32 }

@x1 = common global %struct.s2 zeroinitializer, align 8
@x2 = common global %struct.s2 zeroinitializer, align 8

define void @c3_5() {
bb:
  store <2 x ptr> <ptr @x1, ptr @x2>, ptr @x2, align 8
  ret void
}
