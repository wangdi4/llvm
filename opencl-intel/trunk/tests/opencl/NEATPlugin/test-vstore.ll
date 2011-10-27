; RUN: llvm-as %s -o %t.bc
; RUN: lli -force-interpreter %t.bc > %t
; RUN: FileCheck %s <%t

@r_f_2 = internal constant [8 x i8] c" %f %f\0A\00"
@r_f_3 = internal constant [11 x i8] c" %f %f %f\0A\00"
@r_f_4 = internal constant [14 x i8] c" %f %f %f %f\0A\00"

declare i32 @printf(i8*, ...)

declare void @_Z7vstore2Dv2_fjPf(<2 x float>, i32, float*)
declare void @_Z7vstore3Dv3_fjPf(<3 x float>, i32, float*)
declare void @_Z7vstore4Dv4_fjPf(<4 x float>, i32, float*)
declare void @_Z7vstore8Dv8_fjPf(<8 x float>, i32, float*)
declare void @_Z8vstore16Dv16_fjPf(<16 x float>, i32, float*)

define i32 @vstore2()
{
    %ptr = getelementptr [8 x i8]* @r_f_2, i32 0, i32 0 ; to printf
       
    %p2_out = alloca <2 x float>, align 8           ; <<2 x float>*>

    %p_out = bitcast <2 x float>* %p2_out to float*

    call void @_Z7vstore2Dv2_fjPf(<2 x float> < float 10002.5, float -30000.5 >, i32 0, float* %p_out)

    %a2_out = load <2 x float>* %p2_out

    %r_0 = extractelement <2 x float> %a2_out, i32 0
    %r_1 = extractelement <2 x float> %a2_out, i32 1

    %rd_0 = fpext float %r_0 to double
    %rd_1 = fpext float %r_1 to double

    call i32 (i8*, ...)* @printf(i8* %ptr, double %rd_0, double %rd_1)
; CHECK:  10002.500000 -30000.500000

    ret i32 0
}

define i32 @vstore3()
{
    %ptr = getelementptr [11 x i8]* @r_f_3, i32 0, i32 0 ; to printf
       
    %p3_out = alloca <3 x float>, align 16           ; <<3 x float>*>

    %p_out = bitcast <3 x float>* %p3_out to float*

    call void @_Z7vstore3Dv3_fjPf(<3 x float> < float 3000.5, float -4000.5, float 5000.5 >, i32 0, float* %p_out)

    %a3_out = load <3 x float>* %p3_out

    %r_0 = extractelement <3 x float> %a3_out, i32 0
    %r_1 = extractelement <3 x float> %a3_out, i32 1
    %r_2 = extractelement <3 x float> %a3_out, i32 2

    %rd_0 = fpext float %r_0 to double
    %rd_1 = fpext float %r_1 to double
    %rd_2 = fpext float %r_2 to double

    call i32 (i8*, ...)* @printf(i8* %ptr, double %rd_0, double %rd_1, double %rd_2)
; CHECK:  3000.500000 -4000.500000 5000.500000
 
    ret i32 0
}


define i32 @vstore4()
{
    %ptr = getelementptr [14 x i8]* @r_f_4, i32 0, i32 0 ; to printf
       
    %p4_out = alloca <4 x float>, align 16           ; <<4 x float>*>

    %p_out = bitcast <4 x float>* %p4_out to float*

    call void @_Z7vstore4Dv4_fjPf(<4 x float> < float 300.5, float -400.5, float 500.5, float -600.5 >, i32 0, float* %p_out)

    %a4_out = load <4 x float>* %p4_out

    %r_0 = extractelement <4 x float> %a4_out, i32 0
    %r_1 = extractelement <4 x float> %a4_out, i32 1
    %r_2 = extractelement <4 x float> %a4_out, i32 2
    %r_3 = extractelement <4 x float> %a4_out, i32 3

    %rd_0 = fpext float %r_0 to double
    %rd_1 = fpext float %r_1 to double
    %rd_2 = fpext float %r_2 to double
    %rd_3 = fpext float %r_3 to double

    call i32 (i8*, ...)* @printf(i8* %ptr, double %rd_0, double %rd_1, double %rd_2, double %rd_3)
; CHECK: 300.500000 -400.500000 500.500000 -600.500000

    ret i32 0
}

define i32 @vstore8()
{
    %ptr = getelementptr [14 x i8]* @r_f_4, i32 0, i32 0 ; to printf
       
    %p8_out = alloca <8 x float>, align 32           ; <<8 x float>*>

    %p_out = bitcast <8 x float>* %p8_out to float*

    call void @_Z7vstore8Dv8_fjPf(<8 x float> < float 30.5, float -40.5, float 50.5, float -60.5, float 70.5, float -80.5, float 90.5, float -10.5 >, i32 0, float* %p_out)

    %a8_out = load <8 x float>* %p8_out

    %r_0 = extractelement <8 x float> %a8_out, i32 0
    %r_1 = extractelement <8 x float> %a8_out, i32 1
    %r_2 = extractelement <8 x float> %a8_out, i32 2
    %r_3 = extractelement <8 x float> %a8_out, i32 3
    %r_4 = extractelement <8 x float> %a8_out, i32 4
    %r_5 = extractelement <8 x float> %a8_out, i32 5
    %r_6 = extractelement <8 x float> %a8_out, i32 6
    %r_7 = extractelement <8 x float> %a8_out, i32 7    

    %rd_0 = fpext float %r_0 to double
    %rd_1 = fpext float %r_1 to double
    %rd_2 = fpext float %r_2 to double
    %rd_3 = fpext float %r_3 to double
    %rd_4 = fpext float %r_4 to double
    %rd_5 = fpext float %r_5 to double
    %rd_6 = fpext float %r_6 to double
    %rd_7 = fpext float %r_7 to double

    call i32 (i8*, ...)* @printf(i8* %ptr, double %rd_0, double %rd_1, double %rd_2, double %rd_3)
; CHECK: 30.500000 -40.500000 50.500000 -60.500000

    call i32 (i8*, ...)* @printf(i8* %ptr, double %rd_4, double %rd_5, double %rd_6, double %rd_7)
; CHECK: 70.500000 -80.500000 90.500000 -10.500000

    ret i32 0
}

define i32 @vstore16()
{
    %ptr = getelementptr [14 x i8]* @r_f_4, i32 0, i32 0 ; to printf
       
    %p16_out = alloca <16 x float>, align 64           ; <<16 x float>*>

    %p_out = bitcast <16 x float>* %p16_out to float*

    call void @_Z8vstore16Dv16_fjPf(<16 x float> < float 3.5, float -4.5, float 5.5, float -6.5, float 7.5, float -8.5, float 9.5, float -1.5, float 2.5, float -3.5, float 4.5, float -5.5, float 6.5, float -7.5, float 8.5, float -9.5 >, i32 0, float* %p_out)

    %a16_out = load <16 x float>* %p16_out

    %r_0 = extractelement <16 x float> %a16_out, i32 0
    %r_1 = extractelement <16 x float> %a16_out, i32 1
    %r_2 = extractelement <16 x float> %a16_out, i32 2
    %r_3 = extractelement <16 x float> %a16_out, i32 3
    %r_4 = extractelement <16 x float> %a16_out, i32 4
    %r_5 = extractelement <16 x float> %a16_out, i32 5
    %r_6 = extractelement <16 x float> %a16_out, i32 6
    %r_7 = extractelement <16 x float> %a16_out, i32 7    
    %r_8 = extractelement <16 x float> %a16_out, i32 8
    %r_9 = extractelement <16 x float> %a16_out, i32 9
    %r_10 = extractelement <16 x float> %a16_out, i32 10
    %r_11 = extractelement <16 x float> %a16_out, i32 11
    %r_12 = extractelement <16 x float> %a16_out, i32 12
    %r_13 = extractelement <16 x float> %a16_out, i32 13
    %r_14 = extractelement <16 x float> %a16_out, i32 14
    %r_15 = extractelement <16 x float> %a16_out, i32 15   

    %rd_0 = fpext float %r_0 to double
    %rd_1 = fpext float %r_1 to double
    %rd_2 = fpext float %r_2 to double
    %rd_3 = fpext float %r_3 to double
    %rd_4 = fpext float %r_4 to double
    %rd_5 = fpext float %r_5 to double
    %rd_6 = fpext float %r_6 to double
    %rd_7 = fpext float %r_7 to double
    %rd_8 = fpext float %r_8 to double
    %rd_9 = fpext float %r_9 to double
    %rd_10 = fpext float %r_10 to double
    %rd_11 = fpext float %r_11 to double
    %rd_12 = fpext float %r_12 to double
    %rd_13 = fpext float %r_13 to double
    %rd_14 = fpext float %r_14 to double
    %rd_15 = fpext float %r_15 to double    

    call i32 (i8*, ...)* @printf(i8* %ptr, double %rd_0, double %rd_1, double %rd_2, double %rd_3)
; CHECK: 3.500000 -4.500000 5.500000 -6.500000

    call i32 (i8*, ...)* @printf(i8* %ptr, double %rd_4, double %rd_5, double %rd_6, double %rd_7)
; CHECK: 7.500000 -8.500000 9.500000 -1.500000

    call i32 (i8*, ...)* @printf(i8* %ptr, double %rd_8, double %rd_9, double %rd_10, double %rd_11)
; CHECK: 2.500000 -3.500000 4.500000 -5.500000

    call i32 (i8*, ...)* @printf(i8* %ptr, double %rd_12, double %rd_13, double %rd_14, double %rd_15)
; CHECK: 6.500000 -7.500000 8.500000 -9.500000

    ret i32 0
}

define i32 @main()
{
       call i32 @vstore2()
       call i32 @vstore3()
       call i32 @vstore4()
       call i32 @vstore8()
       call i32 @vstore16()
       ret i32 0
}
