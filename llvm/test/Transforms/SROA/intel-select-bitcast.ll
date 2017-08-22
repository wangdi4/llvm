; Test to verify SROA can handle a case where a memory load is from an
; address chosen by a select statement, when there is an intervening
; bitcast between the select and load instruction. In this case,
; SROA should be able to eliminate the alloca statement, and just
; rely on registers.

; RUN: opt < %s -sroa -S | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-n8:16:32:64"

@gvalue = internal global float 1234.0

declare float @random_u32()

define void @test1() {
; CHECK-LABEL: @test1(
entry:
    %norma = alloca float, align 4
; CHECK-NOT: alloca

    %call1 = call float @random_u32()
    store float %call1, float* %norma, align 4

    %v1 = load float, float* %norma, align 4
    %v_gvalue = load float, float* @gvalue, align 4

    %cmp10.i19 = fcmp ogt float %v1, %v_gvalue

    ; Select the address that will be loaded from to feed the 'store'
    %addr1 = select i1 %cmp10.i19, float* @gvalue, float* %norma
    
    ; Load value from selected address, but first bitcast the type
    ; to i32 to see that SROA can handle this case.
    %addr2 = bitcast float* %addr1 to i32*
    %v2 = load i32, i32* %addr2
    
    ; store new value to gvalue
    %addr3 = bitcast float* @gvalue to i32*
    store i32 %v2, i32* %addr3

    ret void
}

