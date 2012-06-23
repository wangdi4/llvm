; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN_XXX: NEATChecker -r %s -a %s.neat -t 0
; TODO: add NEATCHECKER instrumentation

; ModuleID = 'oclbuiltin.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @oclbuiltin(float addrspace(1)* %input, float addrspace(1)* %output, i32 addrspace(1)* %inputInt, i32 addrspace(1)* %outputInt, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca float addrspace(1)*, align 4
  %output.addr = alloca float addrspace(1)*, align 4
  %inputInt.addr = alloca i32 addrspace(1)*, align 4
  %outputInt.addr = alloca i32 addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %a_in = alloca float, align 4
  %a2_in = alloca <2 x float>, align 8
  %a3_in = alloca <3 x float>, align 16
  %a4_in = alloca <4 x float>, align 16
  %a8_in = alloca <8 x float>, align 32
  %a16_in = alloca <16 x float>, align 64
  %b_in = alloca float, align 4
  %b2_in = alloca <2 x float>, align 8
  %b3_in = alloca <3 x float>, align 16
  %b4_in = alloca <4 x float>, align 16
  %b8_in = alloca <8 x float>, align 32
  %b16_in = alloca <16 x float>, align 64
  %c_in = alloca float, align 4
  %c2_in = alloca <2 x float>, align 8
  %c3_in = alloca <3 x float>, align 16
  %c4_in = alloca <4 x float>, align 16
  %c8_in = alloca <8 x float>, align 32
  %c16_in = alloca <16 x float>, align 64
  %a_out = alloca float, align 4
  %a2_out = alloca <2 x float>, align 8
  %a3_out = alloca <3 x float>, align 16
  %a4_out = alloca <4 x float>, align 16
  %a8_out = alloca <8 x float>, align 32
  %a16_out = alloca <16 x float>, align 64
  %b_out = alloca float, align 4
  %b2_out = alloca <2 x float>, align 8
  %b3_out = alloca <3 x float>, align 16
  %b4_out = alloca <4 x float>, align 16
  %b8_out = alloca <8 x float>, align 32
  %b16_out = alloca <16 x float>, align 64
  %c_out = alloca float, align 4
  %c2_out = alloca <2 x float>, align 8
  %c3_out = alloca <3 x float>, align 16
  %c4_out = alloca <4 x float>, align 16
  %c8_out = alloca <8 x float>, align 32
  %c16_out = alloca <16 x float>, align 64
  %i_in = alloca i32, align 4
  %i2_in = alloca <2 x i32>, align 8
  %i3_in = alloca <3 x i32>, align 16
  %i4_in = alloca <4 x i32>, align 16
  %i8_in = alloca <8 x i32>, align 32
  %i16_in = alloca <16 x i32>, align 64
  %i_out = alloca i32, align 4
  %i2_out = alloca <2 x i32>, align 8
  %i3_out = alloca <3 x i32>, align 16
  %i4_out = alloca <4 x i32>, align 16
  %i8_out = alloca <8 x i32>, align 32
  %i16_out = alloca <16 x i32>, align 64
  %ui_in = alloca i32, align 4
  %ui2_in = alloca <2 x i32>, align 8
  %ui3_in = alloca <3 x i32>, align 16
  %ui4_in = alloca <4 x i32>, align 16
  %ui8_in = alloca <8 x i32>, align 32
  %ui16_in = alloca <16 x i32>, align 64
  %ch_in = alloca i8, align 1
  %ch2_in = alloca <2 x i8>, align 2
  %ch3_in = alloca <3 x i8>, align 4
  %ch4_in = alloca <4 x i8>, align 4
  %ch8_in = alloca <8 x i8>, align 8
  %ch16_in = alloca <16 x i8>, align 16
  %uch_in = alloca i8, align 1
  %uch2_in = alloca <2 x i8>, align 2
  %uch3_in = alloca <3 x i8>, align 4
  %uch4_in = alloca <4 x i8>, align 4
  %uch8_in = alloca <8 x i8>, align 8
  %uch16_in = alloca <16 x i8>, align 16
  %s_in = alloca i16, align 2
  %s2_in = alloca <2 x i16>, align 4
  %s3_in = alloca <3 x i16>, align 8
  %s4_in = alloca <4 x i16>, align 8
  %s8_in = alloca <8 x i16>, align 16
  %s16_in = alloca <16 x i16>, align 32
  %us_in = alloca i16, align 2
  %us2_in = alloca <2 x i16>, align 4
  %us3_in = alloca <3 x i16>, align 8
  %us4_in = alloca <4 x i16>, align 8
  %us8_in = alloca <8 x i16>, align 16
  %us16_in = alloca <16 x i16>, align 32
  %l_in = alloca i64, align 8
  %l2_in = alloca <2 x i64>, align 16
  %l3_in = alloca <3 x i64>, align 32
  %l4_in = alloca <4 x i64>, align 32
  %l8_in = alloca <8 x i64>, align 64
  %l16_in = alloca <16 x i64>, align 128
  %ul_in = alloca i64, align 8
  %ul2_in = alloca <2 x i64>, align 16
  %ul3_in = alloca <3 x i64>, align 32
  %ul4_in = alloca <4 x i64>, align 32
  %ul8_in = alloca <8 x i64>, align 64
  %ul16_in = alloca <16 x i64>, align 128
  store float addrspace(1)* %input, float addrspace(1)** %input.addr, align 4
  store float addrspace(1)* %output, float addrspace(1)** %output.addr, align 4
  store i32 addrspace(1)* %inputInt, i32 addrspace(1)** %inputInt.addr, align 4
  store i32 addrspace(1)* %outputInt, i32 addrspace(1)** %outputInt.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  store i32 0, i32* %tid, align 4
  %0 = load i32* %tid, align 4
  %1 = load float addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds float addrspace(1)* %1, i32 %0
  %2 = load float addrspace(1)* %arrayidx
  store float %2, float* %a_in, align 4
  %3 = load i32* %tid, align 4
  %4 = load float addrspace(1)** %input.addr, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %4, i32 %3
  %5 = load float addrspace(1)* %arrayidx1
  %6 = insertelement <2 x float> undef, float %5, i32 0
  %splat = shufflevector <2 x float> %6, <2 x float> %6, <2 x i32> zeroinitializer
  store <2 x float> %splat, <2 x float>* %a2_in, align 8
  %7 = load i32* %tid, align 4
  %8 = load float addrspace(1)** %input.addr, align 4
  %arrayidx2 = getelementptr inbounds float addrspace(1)* %8, i32 %7
  %9 = load float addrspace(1)* %arrayidx2
  %10 = insertelement <3 x float> undef, float %9, i32 0
  %splat3 = shufflevector <3 x float> %10, <3 x float> %10, <3 x i32> zeroinitializer
  store <3 x float> %splat3, <3 x float>* %a3_in, align 16
  %11 = load i32* %tid, align 4
  %12 = load float addrspace(1)** %input.addr, align 4
  %arrayidx4 = getelementptr inbounds float addrspace(1)* %12, i32 %11
  %13 = load float addrspace(1)* %arrayidx4
  %14 = insertelement <4 x float> undef, float %13, i32 0
  %splat5 = shufflevector <4 x float> %14, <4 x float> %14, <4 x i32> zeroinitializer
  store <4 x float> %splat5, <4 x float>* %a4_in, align 16
  %15 = load i32* %tid, align 4
  %16 = load float addrspace(1)** %input.addr, align 4
  %arrayidx6 = getelementptr inbounds float addrspace(1)* %16, i32 %15
  %17 = load float addrspace(1)* %arrayidx6
  %18 = insertelement <8 x float> undef, float %17, i32 0
  %splat7 = shufflevector <8 x float> %18, <8 x float> %18, <8 x i32> zeroinitializer
  store <8 x float> %splat7, <8 x float>* %a8_in, align 32
  %19 = load i32* %tid, align 4
  %20 = load float addrspace(1)** %input.addr, align 4
  %arrayidx8 = getelementptr inbounds float addrspace(1)* %20, i32 %19
  %21 = load float addrspace(1)* %arrayidx8
  %22 = insertelement <16 x float> undef, float %21, i32 0
  %splat9 = shufflevector <16 x float> %22, <16 x float> %22, <16 x i32> zeroinitializer
  store <16 x float> %splat9, <16 x float>* %a16_in, align 64
  %23 = load i32* %tid, align 4
  %add = add i32 %23, 1
  %24 = load float addrspace(1)** %input.addr, align 4
  %arrayidx10 = getelementptr inbounds float addrspace(1)* %24, i32 %add
  %25 = load float addrspace(1)* %arrayidx10
  store float %25, float* %b_in, align 4
  %26 = load i32* %tid, align 4
  %27 = load float addrspace(1)** %input.addr, align 4
  %arrayidx11 = getelementptr inbounds float addrspace(1)* %27, i32 %26
  %28 = load float addrspace(1)* %arrayidx11
  %29 = insertelement <2 x float> undef, float %28, i32 0
  %splat12 = shufflevector <2 x float> %29, <2 x float> %29, <2 x i32> zeroinitializer
  store <2 x float> %splat12, <2 x float>* %b2_in, align 8
  %30 = load i32* %tid, align 4
  %31 = load float addrspace(1)** %input.addr, align 4
  %arrayidx13 = getelementptr inbounds float addrspace(1)* %31, i32 %30
  %32 = load float addrspace(1)* %arrayidx13
  %33 = insertelement <3 x float> undef, float %32, i32 0
  %splat14 = shufflevector <3 x float> %33, <3 x float> %33, <3 x i32> zeroinitializer
  store <3 x float> %splat14, <3 x float>* %b3_in, align 16
  %34 = load i32* %tid, align 4
  %35 = load float addrspace(1)** %input.addr, align 4
  %arrayidx15 = getelementptr inbounds float addrspace(1)* %35, i32 %34
  %36 = load float addrspace(1)* %arrayidx15
  %37 = insertelement <4 x float> undef, float %36, i32 0
  %splat16 = shufflevector <4 x float> %37, <4 x float> %37, <4 x i32> zeroinitializer
  store <4 x float> %splat16, <4 x float>* %b4_in, align 16
  %38 = load i32* %tid, align 4
  %39 = load float addrspace(1)** %input.addr, align 4
  %arrayidx17 = getelementptr inbounds float addrspace(1)* %39, i32 %38
  %40 = load float addrspace(1)* %arrayidx17
  %41 = insertelement <8 x float> undef, float %40, i32 0
  %splat18 = shufflevector <8 x float> %41, <8 x float> %41, <8 x i32> zeroinitializer
  store <8 x float> %splat18, <8 x float>* %b8_in, align 32
  %42 = load i32* %tid, align 4
  %43 = load float addrspace(1)** %input.addr, align 4
  %arrayidx19 = getelementptr inbounds float addrspace(1)* %43, i32 %42
  %44 = load float addrspace(1)* %arrayidx19
  %45 = insertelement <16 x float> undef, float %44, i32 0
  %splat20 = shufflevector <16 x float> %45, <16 x float> %45, <16 x i32> zeroinitializer
  store <16 x float> %splat20, <16 x float>* %b16_in, align 64
  %46 = load i32* %tid, align 4
  %add21 = add i32 %46, 1
  %47 = load float addrspace(1)** %input.addr, align 4
  %arrayidx22 = getelementptr inbounds float addrspace(1)* %47, i32 %add21
  %48 = load float addrspace(1)* %arrayidx22
  store float %48, float* %c_in, align 4
  %49 = load i32* %tid, align 4
  %50 = load float addrspace(1)** %input.addr, align 4
  %arrayidx23 = getelementptr inbounds float addrspace(1)* %50, i32 %49
  %51 = load float addrspace(1)* %arrayidx23
  %52 = insertelement <2 x float> undef, float %51, i32 0
  %splat24 = shufflevector <2 x float> %52, <2 x float> %52, <2 x i32> zeroinitializer
  store <2 x float> %splat24, <2 x float>* %c2_in, align 8
  %53 = load i32* %tid, align 4
  %54 = load float addrspace(1)** %input.addr, align 4
  %arrayidx25 = getelementptr inbounds float addrspace(1)* %54, i32 %53
  %55 = load float addrspace(1)* %arrayidx25
  %56 = insertelement <3 x float> undef, float %55, i32 0
  %splat26 = shufflevector <3 x float> %56, <3 x float> %56, <3 x i32> zeroinitializer
  store <3 x float> %splat26, <3 x float>* %c3_in, align 16
  %57 = load i32* %tid, align 4
  %58 = load float addrspace(1)** %input.addr, align 4
  %arrayidx27 = getelementptr inbounds float addrspace(1)* %58, i32 %57
  %59 = load float addrspace(1)* %arrayidx27
  %60 = insertelement <4 x float> undef, float %59, i32 0
  %splat28 = shufflevector <4 x float> %60, <4 x float> %60, <4 x i32> zeroinitializer
  store <4 x float> %splat28, <4 x float>* %c4_in, align 16
  %61 = load i32* %tid, align 4
  %62 = load float addrspace(1)** %input.addr, align 4
  %arrayidx29 = getelementptr inbounds float addrspace(1)* %62, i32 %61
  %63 = load float addrspace(1)* %arrayidx29
  %64 = insertelement <8 x float> undef, float %63, i32 0
  %splat30 = shufflevector <8 x float> %64, <8 x float> %64, <8 x i32> zeroinitializer
  store <8 x float> %splat30, <8 x float>* %c8_in, align 32
  %65 = load i32* %tid, align 4
  %66 = load float addrspace(1)** %input.addr, align 4
  %arrayidx31 = getelementptr inbounds float addrspace(1)* %66, i32 %65
  %67 = load float addrspace(1)* %arrayidx31
  %68 = insertelement <16 x float> undef, float %67, i32 0
  %splat32 = shufflevector <16 x float> %68, <16 x float> %68, <16 x i32> zeroinitializer
  store <16 x float> %splat32, <16 x float>* %c16_in, align 64
  %69 = load i32* %tid, align 4
  %70 = load float addrspace(1)** %output.addr, align 4
  %arrayidx33 = getelementptr inbounds float addrspace(1)* %70, i32 %69
  %71 = load float addrspace(1)* %arrayidx33
  store float %71, float* %a_out, align 4
  %72 = load i32* %tid, align 4
  %73 = load float addrspace(1)** %output.addr, align 4
  %arrayidx34 = getelementptr inbounds float addrspace(1)* %73, i32 %72
  %74 = load float addrspace(1)* %arrayidx34
  %75 = insertelement <2 x float> undef, float %74, i32 0
  %splat35 = shufflevector <2 x float> %75, <2 x float> %75, <2 x i32> zeroinitializer
  store <2 x float> %splat35, <2 x float>* %a2_out, align 8
  %76 = load i32* %tid, align 4
  %77 = load float addrspace(1)** %output.addr, align 4
  %arrayidx36 = getelementptr inbounds float addrspace(1)* %77, i32 %76
  %78 = load float addrspace(1)* %arrayidx36
  %79 = insertelement <3 x float> undef, float %78, i32 0
  %splat37 = shufflevector <3 x float> %79, <3 x float> %79, <3 x i32> zeroinitializer
  store <3 x float> %splat37, <3 x float>* %a3_out, align 16
  %80 = load i32* %tid, align 4
  %81 = load float addrspace(1)** %output.addr, align 4
  %arrayidx38 = getelementptr inbounds float addrspace(1)* %81, i32 %80
  %82 = load float addrspace(1)* %arrayidx38
  %83 = insertelement <4 x float> undef, float %82, i32 0
  %splat39 = shufflevector <4 x float> %83, <4 x float> %83, <4 x i32> zeroinitializer
  store <4 x float> %splat39, <4 x float>* %a4_out, align 16
  %84 = load i32* %tid, align 4
  %85 = load float addrspace(1)** %output.addr, align 4
  %arrayidx40 = getelementptr inbounds float addrspace(1)* %85, i32 %84
  %86 = load float addrspace(1)* %arrayidx40
  %87 = insertelement <8 x float> undef, float %86, i32 0
  %splat41 = shufflevector <8 x float> %87, <8 x float> %87, <8 x i32> zeroinitializer
  store <8 x float> %splat41, <8 x float>* %a8_out, align 32
  %88 = load i32* %tid, align 4
  %89 = load float addrspace(1)** %output.addr, align 4
  %arrayidx42 = getelementptr inbounds float addrspace(1)* %89, i32 %88
  %90 = load float addrspace(1)* %arrayidx42
  %91 = insertelement <16 x float> undef, float %90, i32 0
  %splat43 = shufflevector <16 x float> %91, <16 x float> %91, <16 x i32> zeroinitializer
  store <16 x float> %splat43, <16 x float>* %a16_out, align 64
  %92 = load i32* %tid, align 4
  %93 = load float addrspace(1)** %output.addr, align 4
  %arrayidx44 = getelementptr inbounds float addrspace(1)* %93, i32 %92
  %94 = load float addrspace(1)* %arrayidx44
  store float %94, float* %b_out, align 4
  %95 = load i32* %tid, align 4
  %96 = load float addrspace(1)** %output.addr, align 4
  %arrayidx45 = getelementptr inbounds float addrspace(1)* %96, i32 %95
  %97 = load float addrspace(1)* %arrayidx45
  %98 = insertelement <2 x float> undef, float %97, i32 0
  %splat46 = shufflevector <2 x float> %98, <2 x float> %98, <2 x i32> zeroinitializer
  store <2 x float> %splat46, <2 x float>* %b2_out, align 8
  %99 = load i32* %tid, align 4
  %100 = load float addrspace(1)** %output.addr, align 4
  %arrayidx47 = getelementptr inbounds float addrspace(1)* %100, i32 %99
  %101 = load float addrspace(1)* %arrayidx47
  %102 = insertelement <3 x float> undef, float %101, i32 0
  %splat48 = shufflevector <3 x float> %102, <3 x float> %102, <3 x i32> zeroinitializer
  store <3 x float> %splat48, <3 x float>* %b3_out, align 16
  %103 = load i32* %tid, align 4
  %104 = load float addrspace(1)** %output.addr, align 4
  %arrayidx49 = getelementptr inbounds float addrspace(1)* %104, i32 %103
  %105 = load float addrspace(1)* %arrayidx49
  %106 = insertelement <4 x float> undef, float %105, i32 0
  %splat50 = shufflevector <4 x float> %106, <4 x float> %106, <4 x i32> zeroinitializer
  store <4 x float> %splat50, <4 x float>* %b4_out, align 16
  %107 = load i32* %tid, align 4
  %108 = load float addrspace(1)** %output.addr, align 4
  %arrayidx51 = getelementptr inbounds float addrspace(1)* %108, i32 %107
  %109 = load float addrspace(1)* %arrayidx51
  %110 = insertelement <8 x float> undef, float %109, i32 0
  %splat52 = shufflevector <8 x float> %110, <8 x float> %110, <8 x i32> zeroinitializer
  store <8 x float> %splat52, <8 x float>* %b8_out, align 32
  %111 = load i32* %tid, align 4
  %112 = load float addrspace(1)** %output.addr, align 4
  %arrayidx53 = getelementptr inbounds float addrspace(1)* %112, i32 %111
  %113 = load float addrspace(1)* %arrayidx53
  %114 = insertelement <16 x float> undef, float %113, i32 0
  %splat54 = shufflevector <16 x float> %114, <16 x float> %114, <16 x i32> zeroinitializer
  store <16 x float> %splat54, <16 x float>* %b16_out, align 64
  %115 = load i32* %tid, align 4
  %116 = load float addrspace(1)** %output.addr, align 4
  %arrayidx55 = getelementptr inbounds float addrspace(1)* %116, i32 %115
  %117 = load float addrspace(1)* %arrayidx55
  store float %117, float* %c_out, align 4
  %118 = load i32* %tid, align 4
  %119 = load float addrspace(1)** %output.addr, align 4
  %arrayidx56 = getelementptr inbounds float addrspace(1)* %119, i32 %118
  %120 = load float addrspace(1)* %arrayidx56
  %121 = insertelement <2 x float> undef, float %120, i32 0
  %splat57 = shufflevector <2 x float> %121, <2 x float> %121, <2 x i32> zeroinitializer
  store <2 x float> %splat57, <2 x float>* %c2_out, align 8
  %122 = load i32* %tid, align 4
  %123 = load float addrspace(1)** %output.addr, align 4
  %arrayidx58 = getelementptr inbounds float addrspace(1)* %123, i32 %122
  %124 = load float addrspace(1)* %arrayidx58
  %125 = insertelement <3 x float> undef, float %124, i32 0
  %splat59 = shufflevector <3 x float> %125, <3 x float> %125, <3 x i32> zeroinitializer
  store <3 x float> %splat59, <3 x float>* %c3_out, align 16
  %126 = load i32* %tid, align 4
  %127 = load float addrspace(1)** %output.addr, align 4
  %arrayidx60 = getelementptr inbounds float addrspace(1)* %127, i32 %126
  %128 = load float addrspace(1)* %arrayidx60
  %129 = insertelement <4 x float> undef, float %128, i32 0
  %splat61 = shufflevector <4 x float> %129, <4 x float> %129, <4 x i32> zeroinitializer
  store <4 x float> %splat61, <4 x float>* %c4_out, align 16
  %130 = load i32* %tid, align 4
  %131 = load float addrspace(1)** %output.addr, align 4
  %arrayidx62 = getelementptr inbounds float addrspace(1)* %131, i32 %130
  %132 = load float addrspace(1)* %arrayidx62
  %133 = insertelement <8 x float> undef, float %132, i32 0
  %splat63 = shufflevector <8 x float> %133, <8 x float> %133, <8 x i32> zeroinitializer
  store <8 x float> %splat63, <8 x float>* %c8_out, align 32
  %134 = load i32* %tid, align 4
  %135 = load float addrspace(1)** %output.addr, align 4
  %arrayidx64 = getelementptr inbounds float addrspace(1)* %135, i32 %134
  %136 = load float addrspace(1)* %arrayidx64
  %137 = insertelement <16 x float> undef, float %136, i32 0
  %splat65 = shufflevector <16 x float> %137, <16 x float> %137, <16 x i32> zeroinitializer
  store <16 x float> %splat65, <16 x float>* %c16_out, align 64
  %138 = load i32* %tid, align 4
  %139 = load i32 addrspace(1)** %inputInt.addr, align 4
  %arrayidx66 = getelementptr inbounds i32 addrspace(1)* %139, i32 %138
  %140 = load i32 addrspace(1)* %arrayidx66
  store i32 %140, i32* %i_in, align 4
  %141 = load i32* %tid, align 4
  %142 = load i32 addrspace(1)** %inputInt.addr, align 4
  %arrayidx67 = getelementptr inbounds i32 addrspace(1)* %142, i32 %141
  %143 = load i32 addrspace(1)* %arrayidx67
  %144 = insertelement <2 x i32> undef, i32 %143, i32 0
  %splat68 = shufflevector <2 x i32> %144, <2 x i32> %144, <2 x i32> zeroinitializer
  store <2 x i32> %splat68, <2 x i32>* %i2_in, align 8
  %145 = load i32* %tid, align 4
  %146 = load i32 addrspace(1)** %inputInt.addr, align 4
  %arrayidx69 = getelementptr inbounds i32 addrspace(1)* %146, i32 %145
  %147 = load i32 addrspace(1)* %arrayidx69
  %148 = insertelement <3 x i32> undef, i32 %147, i32 0
  %splat70 = shufflevector <3 x i32> %148, <3 x i32> %148, <3 x i32> zeroinitializer
  store <3 x i32> %splat70, <3 x i32>* %i3_in, align 16
  %149 = load i32* %tid, align 4
  %150 = load i32 addrspace(1)** %inputInt.addr, align 4
  %arrayidx71 = getelementptr inbounds i32 addrspace(1)* %150, i32 %149
  %151 = load i32 addrspace(1)* %arrayidx71
  %152 = insertelement <4 x i32> undef, i32 %151, i32 0
  %splat72 = shufflevector <4 x i32> %152, <4 x i32> %152, <4 x i32> zeroinitializer
  store <4 x i32> %splat72, <4 x i32>* %i4_in, align 16
  %153 = load i32* %tid, align 4
  %154 = load i32 addrspace(1)** %inputInt.addr, align 4
  %arrayidx73 = getelementptr inbounds i32 addrspace(1)* %154, i32 %153
  %155 = load i32 addrspace(1)* %arrayidx73
  %156 = insertelement <8 x i32> undef, i32 %155, i32 0
  %splat74 = shufflevector <8 x i32> %156, <8 x i32> %156, <8 x i32> zeroinitializer
  store <8 x i32> %splat74, <8 x i32>* %i8_in, align 32
  %157 = load i32* %tid, align 4
  %158 = load i32 addrspace(1)** %inputInt.addr, align 4
  %arrayidx75 = getelementptr inbounds i32 addrspace(1)* %158, i32 %157
  %159 = load i32 addrspace(1)* %arrayidx75
  %160 = insertelement <16 x i32> undef, i32 %159, i32 0
  %splat76 = shufflevector <16 x i32> %160, <16 x i32> %160, <16 x i32> zeroinitializer
  store <16 x i32> %splat76, <16 x i32>* %i16_in, align 64
  %161 = load i32* %tid, align 4
  %162 = load i32 addrspace(1)** %outputInt.addr, align 4
  %arrayidx77 = getelementptr inbounds i32 addrspace(1)* %162, i32 %161
  %163 = load i32 addrspace(1)* %arrayidx77
  store i32 %163, i32* %i_out, align 4
  %164 = load i32* %tid, align 4
  %165 = load i32 addrspace(1)** %outputInt.addr, align 4
  %arrayidx78 = getelementptr inbounds i32 addrspace(1)* %165, i32 %164
  %166 = load i32 addrspace(1)* %arrayidx78
  %167 = insertelement <2 x i32> undef, i32 %166, i32 0
  %splat79 = shufflevector <2 x i32> %167, <2 x i32> %167, <2 x i32> zeroinitializer
  store <2 x i32> %splat79, <2 x i32>* %i2_out, align 8
  %168 = load i32* %tid, align 4
  %169 = load i32 addrspace(1)** %outputInt.addr, align 4
  %arrayidx80 = getelementptr inbounds i32 addrspace(1)* %169, i32 %168
  %170 = load i32 addrspace(1)* %arrayidx80
  %171 = insertelement <3 x i32> undef, i32 %170, i32 0
  %splat81 = shufflevector <3 x i32> %171, <3 x i32> %171, <3 x i32> zeroinitializer
  store <3 x i32> %splat81, <3 x i32>* %i3_out, align 16
  %172 = load i32* %tid, align 4
  %173 = load i32 addrspace(1)** %outputInt.addr, align 4
  %arrayidx82 = getelementptr inbounds i32 addrspace(1)* %173, i32 %172
  %174 = load i32 addrspace(1)* %arrayidx82
  %175 = insertelement <4 x i32> undef, i32 %174, i32 0
  %splat83 = shufflevector <4 x i32> %175, <4 x i32> %175, <4 x i32> zeroinitializer
  store <4 x i32> %splat83, <4 x i32>* %i4_out, align 16
  %176 = load i32* %tid, align 4
  %177 = load i32 addrspace(1)** %outputInt.addr, align 4
  %arrayidx84 = getelementptr inbounds i32 addrspace(1)* %177, i32 %176
  %178 = load i32 addrspace(1)* %arrayidx84
  %179 = insertelement <8 x i32> undef, i32 %178, i32 0
  %splat85 = shufflevector <8 x i32> %179, <8 x i32> %179, <8 x i32> zeroinitializer
  store <8 x i32> %splat85, <8 x i32>* %i8_out, align 32
  %180 = load i32* %tid, align 4
  %181 = load i32 addrspace(1)** %outputInt.addr, align 4
  %arrayidx86 = getelementptr inbounds i32 addrspace(1)* %181, i32 %180
  %182 = load i32 addrspace(1)* %arrayidx86
  %183 = insertelement <16 x i32> undef, i32 %182, i32 0
  %splat87 = shufflevector <16 x i32> %183, <16 x i32> %183, <16 x i32> zeroinitializer
  store <16 x i32> %splat87, <16 x i32>* %i16_out, align 64
  %184 = load float* %a_in, align 4
  %call = call float @_Z4acosf(float %184) nounwind readnone
  store float %call, float* %a_out, align 4
  %185 = load <4 x float>* %a4_in, align 16
  %call88 = call <4 x float> @_Z4acosDv4_f(<4 x float> %185) nounwind readnone
  store <4 x float> %call88, <4 x float>* %a4_out, align 16
  %186 = load <8 x float>* %a8_in, align 32
  %call89 = call <8 x float> @_Z4acosDv8_f(<8 x float> %186) nounwind readnone
  store <8 x float> %call89, <8 x float>* %a8_out, align 32
  %187 = load <16 x float>* %a16_in, align 64
  %call90 = call <16 x float> @_Z4acosDv16_f(<16 x float> %187) nounwind readnone
  store <16 x float> %call90, <16 x float>* %a16_out, align 64
  %188 = load float* %a_in, align 4
  %call91 = call float @_Z6acospif(float %188) nounwind readnone
  store float %call91, float* %a_out, align 4
  %189 = load <4 x float>* %a4_in, align 16
  %call92 = call <4 x float> @_Z6acospiDv4_f(<4 x float> %189) nounwind readnone
  store <4 x float> %call92, <4 x float>* %a4_out, align 16
  %190 = load <8 x float>* %a8_in, align 32
  %call93 = call <8 x float> @_Z6acospiDv8_f(<8 x float> %190) nounwind readnone
  store <8 x float> %call93, <8 x float>* %a8_out, align 32
  %191 = load <16 x float>* %a16_in, align 64
  %call94 = call <16 x float> @_Z6acospiDv16_f(<16 x float> %191) nounwind readnone
  store <16 x float> %call94, <16 x float>* %a16_out, align 64
  %192 = load float* %a_in, align 4
  %call95 = call float @_Z4asinf(float %192) nounwind readnone
  store float %call95, float* %a_out, align 4
  %193 = load <4 x float>* %a4_in, align 16
  %call96 = call <4 x float> @_Z4asinDv4_f(<4 x float> %193) nounwind readnone
  store <4 x float> %call96, <4 x float>* %a4_out, align 16
  %194 = load <8 x float>* %a8_in, align 32
  %call97 = call <8 x float> @_Z4asinDv8_f(<8 x float> %194) nounwind readnone
  store <8 x float> %call97, <8 x float>* %a8_out, align 32
  %195 = load <16 x float>* %a16_in, align 64
  %call98 = call <16 x float> @_Z4asinDv16_f(<16 x float> %195) nounwind readnone
  store <16 x float> %call98, <16 x float>* %a16_out, align 64
  %196 = load float* %a_in, align 4
  %call99 = call float @_Z6asinpif(float %196) nounwind readnone
  store float %call99, float* %a_out, align 4
  %197 = load <4 x float>* %a4_in, align 16
  %call100 = call <4 x float> @_Z6asinpiDv4_f(<4 x float> %197) nounwind readnone
  store <4 x float> %call100, <4 x float>* %a4_out, align 16
  %198 = load <8 x float>* %a8_in, align 32
  %call101 = call <8 x float> @_Z6asinpiDv8_f(<8 x float> %198) nounwind readnone
  store <8 x float> %call101, <8 x float>* %a8_out, align 32
  %199 = load <16 x float>* %a16_in, align 64
  %call102 = call <16 x float> @_Z6asinpiDv16_f(<16 x float> %199) nounwind readnone
  store <16 x float> %call102, <16 x float>* %a16_out, align 64
  %200 = load float* %a_in, align 4
  %call103 = call float @_Z4atanf(float %200) nounwind readnone
  store float %call103, float* %a_out, align 4
  %201 = load <4 x float>* %a4_in, align 16
  %call104 = call <4 x float> @_Z4atanDv4_f(<4 x float> %201) nounwind readnone
  store <4 x float> %call104, <4 x float>* %a4_out, align 16
  %202 = load <8 x float>* %a8_in, align 32
  %call105 = call <8 x float> @_Z4atanDv8_f(<8 x float> %202) nounwind readnone
  store <8 x float> %call105, <8 x float>* %a8_out, align 32
  %203 = load <16 x float>* %a16_in, align 64
  %call106 = call <16 x float> @_Z4atanDv16_f(<16 x float> %203) nounwind readnone
  store <16 x float> %call106, <16 x float>* %a16_out, align 64
  %204 = load float* %a_in, align 4
  %205 = load float* %b_in, align 4
  %call107 = call float @_Z5atan2ff(float %204, float %205) nounwind readnone
  store float %call107, float* %a_out, align 4
  %206 = load <4 x float>* %a4_in, align 16
  %207 = load <4 x float>* %b4_in, align 16
  %call108 = call <4 x float> @_Z5atan2Dv4_fS_(<4 x float> %206, <4 x float> %207) nounwind readnone
  store <4 x float> %call108, <4 x float>* %a4_out, align 16
  %208 = load <8 x float>* %a8_in, align 32
  %209 = load <8 x float>* %b8_in, align 32
  %call109 = call <8 x float> @_Z5atan2Dv8_fS_(<8 x float> %208, <8 x float> %209) nounwind readnone
  store <8 x float> %call109, <8 x float>* %a8_out, align 32
  %210 = load <16 x float>* %a16_in, align 64
  %211 = load <16 x float>* %b16_in, align 64
  %call110 = call <16 x float> @_Z5atan2Dv16_fS_(<16 x float> %210, <16 x float> %211) nounwind readnone
  store <16 x float> %call110, <16 x float>* %a16_out, align 64
  %212 = load float* %a_in, align 4
  %213 = load float* %b_in, align 4
  %call111 = call float @_Z7atan2piff(float %212, float %213) nounwind readnone
  store float %call111, float* %a_out, align 4
  %214 = load <4 x float>* %a4_in, align 16
  %215 = load <4 x float>* %b4_in, align 16
  %call112 = call <4 x float> @_Z7atan2piDv4_fS_(<4 x float> %214, <4 x float> %215) nounwind readnone
  store <4 x float> %call112, <4 x float>* %a4_out, align 16
  %216 = load <8 x float>* %a8_in, align 32
  %217 = load <8 x float>* %b8_in, align 32
  %call113 = call <8 x float> @_Z7atan2piDv8_fS_(<8 x float> %216, <8 x float> %217) nounwind readnone
  store <8 x float> %call113, <8 x float>* %a8_out, align 32
  %218 = load <16 x float>* %a16_in, align 64
  %219 = load <16 x float>* %b16_in, align 64
  %call114 = call <16 x float> @_Z7atan2piDv16_fS_(<16 x float> %218, <16 x float> %219) nounwind readnone
  store <16 x float> %call114, <16 x float>* %a16_out, align 64
  %220 = load float* %a_in, align 4
  %call115 = call float @_Z6atanpif(float %220) nounwind readnone
  store float %call115, float* %a_out, align 4
  %221 = load <4 x float>* %a4_in, align 16
  %call116 = call <4 x float> @_Z6atanpiDv4_f(<4 x float> %221) nounwind readnone
  store <4 x float> %call116, <4 x float>* %a4_out, align 16
  %222 = load <8 x float>* %a8_in, align 32
  %call117 = call <8 x float> @_Z6atanpiDv8_f(<8 x float> %222) nounwind readnone
  store <8 x float> %call117, <8 x float>* %a8_out, align 32
  %223 = load <16 x float>* %a16_in, align 64
  %call118 = call <16 x float> @_Z6atanpiDv16_f(<16 x float> %223) nounwind readnone
  store <16 x float> %call118, <16 x float>* %a16_out, align 64
  %224 = load float* %a_in, align 4
  %call119 = call float @_Z3cosf(float %224) nounwind readnone
  store float %call119, float* %a_out, align 4
  %225 = load <4 x float>* %a4_in, align 16
  %call120 = call <4 x float> @_Z3cosDv4_f(<4 x float> %225) nounwind readnone
  store <4 x float> %call120, <4 x float>* %a4_out, align 16
  %226 = load <8 x float>* %a8_in, align 32
  %call121 = call <8 x float> @_Z3cosDv8_f(<8 x float> %226) nounwind readnone
  store <8 x float> %call121, <8 x float>* %a8_out, align 32
  %227 = load <16 x float>* %a16_in, align 64
  %call122 = call <16 x float> @_Z3cosDv16_f(<16 x float> %227) nounwind readnone
  store <16 x float> %call122, <16 x float>* %a16_out, align 64
  %228 = load float* %a_in, align 4
  %call123 = call float @_Z4coshf(float %228) nounwind readnone
  store float %call123, float* %a_out, align 4
  %229 = load <4 x float>* %a4_in, align 16
  %call124 = call <4 x float> @_Z4coshDv4_f(<4 x float> %229) nounwind readnone
  store <4 x float> %call124, <4 x float>* %a4_out, align 16
  %230 = load <8 x float>* %a8_in, align 32
  %call125 = call <8 x float> @_Z4coshDv8_f(<8 x float> %230) nounwind readnone
  store <8 x float> %call125, <8 x float>* %a8_out, align 32
  %231 = load <16 x float>* %a16_in, align 64
  %call126 = call <16 x float> @_Z4coshDv16_f(<16 x float> %231) nounwind readnone
  store <16 x float> %call126, <16 x float>* %a16_out, align 64
  %232 = load float* %a_in, align 4
  %call127 = call float @_Z5cospif(float %232) nounwind readnone
  store float %call127, float* %a_out, align 4
  %233 = load <4 x float>* %a4_in, align 16
  %call128 = call <4 x float> @_Z5cospiDv4_f(<4 x float> %233) nounwind readnone
  store <4 x float> %call128, <4 x float>* %a4_out, align 16
  %234 = load <8 x float>* %a8_in, align 32
  %call129 = call <8 x float> @_Z5cospiDv8_f(<8 x float> %234) nounwind readnone
  store <8 x float> %call129, <8 x float>* %a8_out, align 32
  %235 = load <16 x float>* %a16_in, align 64
  %call130 = call <16 x float> @_Z5cospiDv16_f(<16 x float> %235) nounwind readnone
  store <16 x float> %call130, <16 x float>* %a16_out, align 64
  %236 = load float* %a_in, align 4
  %call131 = call float @_Z3expf(float %236) nounwind readnone
  store float %call131, float* %a_out, align 4
  %237 = load <4 x float>* %a4_in, align 16
  %call132 = call <4 x float> @_Z3expDv4_f(<4 x float> %237) nounwind readnone
  store <4 x float> %call132, <4 x float>* %a4_out, align 16
  %238 = load <8 x float>* %a8_in, align 32
  %call133 = call <8 x float> @_Z3expDv8_f(<8 x float> %238) nounwind readnone
  store <8 x float> %call133, <8 x float>* %a8_out, align 32
  %239 = load <16 x float>* %a16_in, align 64
  %call134 = call <16 x float> @_Z3expDv16_f(<16 x float> %239) nounwind readnone
  store <16 x float> %call134, <16 x float>* %a16_out, align 64
  %240 = load float* %a_in, align 4
  %call135 = call float @_Z4exp2f(float %240) nounwind readnone
  store float %call135, float* %a_out, align 4
  %241 = load <4 x float>* %a4_in, align 16
  %call136 = call <4 x float> @_Z4exp2Dv4_f(<4 x float> %241) nounwind readnone
  store <4 x float> %call136, <4 x float>* %a4_out, align 16
  %242 = load <8 x float>* %a8_in, align 32
  %call137 = call <8 x float> @_Z4exp2Dv8_f(<8 x float> %242) nounwind readnone
  store <8 x float> %call137, <8 x float>* %a8_out, align 32
  %243 = load <16 x float>* %a16_in, align 64
  %call138 = call <16 x float> @_Z4exp2Dv16_f(<16 x float> %243) nounwind readnone
  store <16 x float> %call138, <16 x float>* %a16_out, align 64
  %244 = load float* %a_in, align 4
  %call139 = call float @_Z5exp10f(float %244) nounwind readnone
  store float %call139, float* %a_out, align 4
  %245 = load <4 x float>* %a4_in, align 16
  %call140 = call <4 x float> @_Z5exp10Dv4_f(<4 x float> %245) nounwind readnone
  store <4 x float> %call140, <4 x float>* %a4_out, align 16
  %246 = load <8 x float>* %a8_in, align 32
  %call141 = call <8 x float> @_Z5exp10Dv8_f(<8 x float> %246) nounwind readnone
  store <8 x float> %call141, <8 x float>* %a8_out, align 32
  %247 = load <16 x float>* %a16_in, align 64
  %call142 = call <16 x float> @_Z5exp10Dv16_f(<16 x float> %247) nounwind readnone
  store <16 x float> %call142, <16 x float>* %a16_out, align 64
  %248 = load float* %a_in, align 4
  %call143 = call float @_Z5expm1f(float %248) nounwind readnone
  store float %call143, float* %a_out, align 4
  %249 = load <4 x float>* %a4_in, align 16
  %call144 = call <4 x float> @_Z5expm1Dv4_f(<4 x float> %249) nounwind readnone
  store <4 x float> %call144, <4 x float>* %a4_out, align 16
  %250 = load <8 x float>* %a8_in, align 32
  %call145 = call <8 x float> @_Z5expm1Dv8_f(<8 x float> %250) nounwind readnone
  store <8 x float> %call145, <8 x float>* %a8_out, align 32
  %251 = load <16 x float>* %a16_in, align 64
  %call146 = call <16 x float> @_Z5expm1Dv16_f(<16 x float> %251) nounwind readnone
  store <16 x float> %call146, <16 x float>* %a16_out, align 64
  %252 = load float* %a_in, align 4
  %call147 = call float @_Z3logf(float %252) nounwind readnone
  store float %call147, float* %a_out, align 4
  %253 = load <4 x float>* %a4_in, align 16
  %call148 = call <4 x float> @_Z3logDv4_f(<4 x float> %253) nounwind readnone
  store <4 x float> %call148, <4 x float>* %a4_out, align 16
  %254 = load <8 x float>* %a8_in, align 32
  %call149 = call <8 x float> @_Z3logDv8_f(<8 x float> %254) nounwind readnone
  store <8 x float> %call149, <8 x float>* %a8_out, align 32
  %255 = load <16 x float>* %a16_in, align 64
  %call150 = call <16 x float> @_Z3logDv16_f(<16 x float> %255) nounwind readnone
  store <16 x float> %call150, <16 x float>* %a16_out, align 64
  %256 = load float* %a_in, align 4
  %call151 = call float @_Z4log2f(float %256) nounwind readnone
  store float %call151, float* %a_out, align 4
  %257 = load <4 x float>* %a4_in, align 16
  %call152 = call <4 x float> @_Z4log2Dv4_f(<4 x float> %257) nounwind readnone
  store <4 x float> %call152, <4 x float>* %a4_out, align 16
  %258 = load <8 x float>* %a8_in, align 32
  %call153 = call <8 x float> @_Z4log2Dv8_f(<8 x float> %258) nounwind readnone
  store <8 x float> %call153, <8 x float>* %a8_out, align 32
  %259 = load <16 x float>* %a16_in, align 64
  %call154 = call <16 x float> @_Z4log2Dv16_f(<16 x float> %259) nounwind readnone
  store <16 x float> %call154, <16 x float>* %a16_out, align 64
  %260 = load float* %a_in, align 4
  %call155 = call float @_Z5log10f(float %260) nounwind readnone
  store float %call155, float* %a_out, align 4
  %261 = load <4 x float>* %a4_in, align 16
  %call156 = call <4 x float> @_Z5log10Dv4_f(<4 x float> %261) nounwind readnone
  store <4 x float> %call156, <4 x float>* %a4_out, align 16
  %262 = load <8 x float>* %a8_in, align 32
  %call157 = call <8 x float> @_Z5log10Dv8_f(<8 x float> %262) nounwind readnone
  store <8 x float> %call157, <8 x float>* %a8_out, align 32
  %263 = load <16 x float>* %a16_in, align 64
  %call158 = call <16 x float> @_Z5log10Dv16_f(<16 x float> %263) nounwind readnone
  store <16 x float> %call158, <16 x float>* %a16_out, align 64
  %264 = load float* %a_in, align 4
  %call159 = call float @_Z5log1pf(float %264) nounwind readnone
  store float %call159, float* %a_out, align 4
  %265 = load <4 x float>* %a4_in, align 16
  %call160 = call <4 x float> @_Z5log1pDv4_f(<4 x float> %265) nounwind readnone
  store <4 x float> %call160, <4 x float>* %a4_out, align 16
  %266 = load <8 x float>* %a8_in, align 32
  %call161 = call <8 x float> @_Z5log1pDv8_f(<8 x float> %266) nounwind readnone
  store <8 x float> %call161, <8 x float>* %a8_out, align 32
  %267 = load <16 x float>* %a16_in, align 64
  %call162 = call <16 x float> @_Z5log1pDv16_f(<16 x float> %267) nounwind readnone
  store <16 x float> %call162, <16 x float>* %a16_out, align 64
  %268 = load float* %a_in, align 4
  %call163 = call float @_Z4logbf(float %268) nounwind readnone
  store float %call163, float* %a_out, align 4
  %269 = load <4 x float>* %a4_in, align 16
  %call164 = call <4 x float> @_Z4logbDv4_f(<4 x float> %269) nounwind readnone
  store <4 x float> %call164, <4 x float>* %a4_out, align 16
  %270 = load <8 x float>* %a8_in, align 32
  %call165 = call <8 x float> @_Z4logbDv8_f(<8 x float> %270) nounwind readnone
  store <8 x float> %call165, <8 x float>* %a8_out, align 32
  %271 = load <16 x float>* %a16_in, align 64
  %call166 = call <16 x float> @_Z4logbDv16_f(<16 x float> %271) nounwind readnone
  store <16 x float> %call166, <16 x float>* %a16_out, align 64
  %272 = load float* %a_in, align 4
  %call167 = call float @_Z4ceilf(float %272) nounwind readnone
  store float %call167, float* %a_out, align 4
  %273 = load <4 x float>* %a4_in, align 16
  %call168 = call <4 x float> @_Z4ceilDv4_f(<4 x float> %273) nounwind readnone
  store <4 x float> %call168, <4 x float>* %a4_out, align 16
  %274 = load <8 x float>* %a8_in, align 32
  %call169 = call <8 x float> @_Z4ceilDv8_f(<8 x float> %274) nounwind readnone
  store <8 x float> %call169, <8 x float>* %a8_out, align 32
  %275 = load <16 x float>* %a16_in, align 64
  %call170 = call <16 x float> @_Z4ceilDv16_f(<16 x float> %275) nounwind readnone
  store <16 x float> %call170, <16 x float>* %a16_out, align 64
  %276 = load float* %a_in, align 4
  %277 = load float* %b_in, align 4
  %call171 = call float @_Z3powff(float %276, float %277) nounwind readnone
  store float %call171, float* %a_out, align 4
  %278 = load <4 x float>* %a4_in, align 16
  %279 = load <4 x float>* %b4_in, align 16
  %call172 = call <4 x float> @_Z3powDv4_fS_(<4 x float> %278, <4 x float> %279) nounwind readnone
  store <4 x float> %call172, <4 x float>* %a4_out, align 16
  %280 = load <8 x float>* %a8_in, align 32
  %281 = load <8 x float>* %b8_in, align 32
  %call173 = call <8 x float> @_Z3powDv8_fS_(<8 x float> %280, <8 x float> %281) nounwind readnone
  store <8 x float> %call173, <8 x float>* %a8_out, align 32
  %282 = load <16 x float>* %a16_in, align 64
  %283 = load <16 x float>* %b16_in, align 64
  %call174 = call <16 x float> @_Z3powDv16_fS_(<16 x float> %282, <16 x float> %283) nounwind readnone
  store <16 x float> %call174, <16 x float>* %a16_out, align 64
  %284 = load float* %a_in, align 4
  %285 = load float* %b_in, align 4
  %286 = load float* %c_in, align 4
  %call175 = call float @_Z5clampfff(float %284, float %285, float %286) nounwind readnone
  store float %call175, float* %a_out, align 4
  %287 = load <4 x float>* %a4_in, align 16
  %288 = load <4 x float>* %b4_in, align 16
  %289 = load <4 x float>* %c4_in, align 16
  %call176 = call <4 x float> @_Z5clampDv4_fS_S_(<4 x float> %287, <4 x float> %288, <4 x float> %289) nounwind readnone
  store <4 x float> %call176, <4 x float>* %a4_out, align 16
  %290 = load <8 x float>* %a8_in, align 32
  %291 = load <8 x float>* %b8_in, align 32
  %292 = load <8 x float>* %c8_in, align 32
  %call177 = call <8 x float> @_Z5clampDv8_fS_S_(<8 x float> %290, <8 x float> %291, <8 x float> %292) nounwind readnone
  store <8 x float> %call177, <8 x float>* %a8_out, align 32
  %293 = load <16 x float>* %a16_in, align 64
  %294 = load <16 x float>* %b16_in, align 64
  %295 = load <16 x float>* %c16_in, align 64
  %call178 = call <16 x float> @_Z5clampDv16_fS_S_(<16 x float> %293, <16 x float> %294, <16 x float> %295) nounwind readnone
  store <16 x float> %call178, <16 x float>* %a16_out, align 64
  %296 = load float* %a_in, align 4
  %297 = load float* %b_in, align 4
  %298 = load float* %c_in, align 4
  %call179 = call float @_Z5clampfff(float %296, float %297, float %298) nounwind readnone
  store float %call179, float* %a_out, align 4
  %299 = load <4 x float>* %a4_in, align 16
  %300 = load float* %b_in, align 4
  %301 = load float* %c_in, align 4
  %call180 = call <4 x float> @_Z5clampDv4_fff(<4 x float> %299, float %300, float %301) nounwind readnone
  store <4 x float> %call180, <4 x float>* %a4_out, align 16
  %302 = load <8 x float>* %a8_in, align 32
  %303 = load float* %b_in, align 4
  %304 = load float* %c_in, align 4
  %call181 = call <8 x float> @_Z5clampDv8_fff(<8 x float> %302, float %303, float %304) nounwind readnone
  store <8 x float> %call181, <8 x float>* %a8_out, align 32
  %305 = load <16 x float>* %a16_in, align 64
  %306 = load float* %b_in, align 4
  %307 = load float* %c_in, align 4
  %call182 = call <16 x float> @_Z5clampDv16_fff(<16 x float> %305, float %306, float %307) nounwind readnone
  store <16 x float> %call182, <16 x float>* %a16_out, align 64
  %308 = load float* %a_in, align 4
  %call183 = call float @_Z4sinhf(float %308) nounwind readnone
  store float %call183, float* %a_out, align 4
  %309 = load <4 x float>* %a4_in, align 16
  %call184 = call <4 x float> @_Z4sinhDv4_f(<4 x float> %309) nounwind readnone
  store <4 x float> %call184, <4 x float>* %a4_out, align 16
  %310 = load <8 x float>* %a8_in, align 32
  %call185 = call <8 x float> @_Z4sinhDv8_f(<8 x float> %310) nounwind readnone
  store <8 x float> %call185, <8 x float>* %a8_out, align 32
  %311 = load <16 x float>* %a16_in, align 64
  %call186 = call <16 x float> @_Z4sinhDv16_f(<16 x float> %311) nounwind readnone
  store <16 x float> %call186, <16 x float>* %a16_out, align 64
  %312 = load float* %a_in, align 4
  %call187 = call float @_Z3sinf(float %312) nounwind readnone
  store float %call187, float* %a_out, align 4
  %313 = load <4 x float>* %a4_in, align 16
  %call188 = call <4 x float> @_Z3sinDv4_f(<4 x float> %313) nounwind readnone
  store <4 x float> %call188, <4 x float>* %a4_out, align 16
  %314 = load <8 x float>* %a8_in, align 32
  %call189 = call <8 x float> @_Z3sinDv8_f(<8 x float> %314) nounwind readnone
  store <8 x float> %call189, <8 x float>* %a8_out, align 32
  %315 = load <16 x float>* %a16_in, align 64
  %call190 = call <16 x float> @_Z3sinDv16_f(<16 x float> %315) nounwind readnone
  store <16 x float> %call190, <16 x float>* %a16_out, align 64
  %316 = load float* %a_in, align 4
  %call191 = call float @_Z5sinpif(float %316) nounwind readnone
  store float %call191, float* %a_out, align 4
  %317 = load <4 x float>* %a4_in, align 16
  %call192 = call <4 x float> @_Z5sinpiDv4_f(<4 x float> %317) nounwind readnone
  store <4 x float> %call192, <4 x float>* %a4_out, align 16
  %318 = load <8 x float>* %a8_in, align 32
  %call193 = call <8 x float> @_Z5sinpiDv8_f(<8 x float> %318) nounwind readnone
  store <8 x float> %call193, <8 x float>* %a8_out, align 32
  %319 = load <16 x float>* %a16_in, align 64
  %call194 = call <16 x float> @_Z5sinpiDv16_f(<16 x float> %319) nounwind readnone
  store <16 x float> %call194, <16 x float>* %a16_out, align 64
  %320 = load float* %a_in, align 4
  %call195 = call float @_Z4sqrtf(float %320) nounwind readnone
  store float %call195, float* %a_out, align 4
  %321 = load <4 x float>* %a4_in, align 16
  %call196 = call <4 x float> @_Z4sqrtDv4_f(<4 x float> %321) nounwind readnone
  store <4 x float> %call196, <4 x float>* %a4_out, align 16
  %322 = load <8 x float>* %a8_in, align 32
  %call197 = call <8 x float> @_Z4sqrtDv8_f(<8 x float> %322) nounwind readnone
  store <8 x float> %call197, <8 x float>* %a8_out, align 32
  %323 = load <16 x float>* %a16_in, align 64
  %call198 = call <16 x float> @_Z4sqrtDv16_f(<16 x float> %323) nounwind readnone
  store <16 x float> %call198, <16 x float>* %a16_out, align 64
  %324 = load float* %a_in, align 4
  %call199 = call float @_Z5rsqrtf(float %324) nounwind readnone
  store float %call199, float* %a_out, align 4
  %325 = load <4 x float>* %a4_in, align 16
  %call200 = call <4 x float> @_Z5rsqrtDv4_f(<4 x float> %325) nounwind readnone
  store <4 x float> %call200, <4 x float>* %a4_out, align 16
  %326 = load <8 x float>* %a8_in, align 32
  %call201 = call <8 x float> @_Z5rsqrtDv8_f(<8 x float> %326) nounwind readnone
  store <8 x float> %call201, <8 x float>* %a8_out, align 32
  %327 = load <16 x float>* %a16_in, align 64
  %call202 = call <16 x float> @_Z5rsqrtDv16_f(<16 x float> %327) nounwind readnone
  store <16 x float> %call202, <16 x float>* %a16_out, align 64
  %328 = load float* %a_in, align 4
  %call203 = call float @_Z3tanf(float %328) nounwind readnone
  store float %call203, float* %a_out, align 4
  %329 = load <4 x float>* %a4_in, align 16
  %call204 = call <4 x float> @_Z3tanDv4_f(<4 x float> %329) nounwind readnone
  store <4 x float> %call204, <4 x float>* %a4_out, align 16
  %330 = load <8 x float>* %a8_in, align 32
  %call205 = call <8 x float> @_Z3tanDv8_f(<8 x float> %330) nounwind readnone
  store <8 x float> %call205, <8 x float>* %a8_out, align 32
  %331 = load <16 x float>* %a16_in, align 64
  %call206 = call <16 x float> @_Z3tanDv16_f(<16 x float> %331) nounwind readnone
  store <16 x float> %call206, <16 x float>* %a16_out, align 64
  %332 = load float* %a_in, align 4
  %call207 = call float @_Z4tanhf(float %332) nounwind readnone
  store float %call207, float* %a_out, align 4
  %333 = load <4 x float>* %a4_in, align 16
  %call208 = call <4 x float> @_Z4tanhDv4_f(<4 x float> %333) nounwind readnone
  store <4 x float> %call208, <4 x float>* %a4_out, align 16
  %334 = load <8 x float>* %a8_in, align 32
  %call209 = call <8 x float> @_Z4tanhDv8_f(<8 x float> %334) nounwind readnone
  store <8 x float> %call209, <8 x float>* %a8_out, align 32
  %335 = load <16 x float>* %a16_in, align 64
  %call210 = call <16 x float> @_Z4tanhDv16_f(<16 x float> %335) nounwind readnone
  store <16 x float> %call210, <16 x float>* %a16_out, align 64
  %336 = load float* %a_in, align 4
  %call211 = call float @_Z5tanpif(float %336) nounwind readnone
  store float %call211, float* %a_out, align 4
  %337 = load <4 x float>* %a4_in, align 16
  %call212 = call <4 x float> @_Z5tanpiDv4_f(<4 x float> %337) nounwind readnone
  store <4 x float> %call212, <4 x float>* %a4_out, align 16
  %338 = load <8 x float>* %a8_in, align 32
  %call213 = call <8 x float> @_Z5tanpiDv8_f(<8 x float> %338) nounwind readnone
  store <8 x float> %call213, <8 x float>* %a8_out, align 32
  %339 = load <16 x float>* %a16_in, align 64
  %call214 = call <16 x float> @_Z5tanpiDv16_f(<16 x float> %339) nounwind readnone
  store <16 x float> %call214, <16 x float>* %a16_out, align 64
  %340 = load float* %a_in, align 4
  %call215 = call float @_Z4fabsf(float %340) nounwind readnone
  store float %call215, float* %a_out, align 4
  %341 = load <4 x float>* %a4_in, align 16
  %call216 = call <4 x float> @_Z4fabsDv4_f(<4 x float> %341) nounwind readnone
  store <4 x float> %call216, <4 x float>* %a4_out, align 16
  %342 = load <8 x float>* %a8_in, align 32
  %call217 = call <8 x float> @_Z4fabsDv8_f(<8 x float> %342) nounwind readnone
  store <8 x float> %call217, <8 x float>* %a8_out, align 32
  %343 = load <16 x float>* %a16_in, align 64
  %call218 = call <16 x float> @_Z4fabsDv16_f(<16 x float> %343) nounwind readnone
  store <16 x float> %call218, <16 x float>* %a16_out, align 64
  %344 = load float* %a_in, align 4
  %call219 = call float @_Z10native_sinf(float %344) nounwind readnone
  store float %call219, float* %a_out, align 4
  %345 = load <4 x float>* %a4_in, align 16
  %call220 = call <4 x float> @_Z10native_sinDv4_f(<4 x float> %345) nounwind readnone
  store <4 x float> %call220, <4 x float>* %a4_out, align 16
  %346 = load <8 x float>* %a8_in, align 32
  %call221 = call <8 x float> @_Z10native_sinDv8_f(<8 x float> %346) nounwind readnone
  store <8 x float> %call221, <8 x float>* %a8_out, align 32
  %347 = load <16 x float>* %a16_in, align 64
  %call222 = call <16 x float> @_Z10native_sinDv16_f(<16 x float> %347) nounwind readnone
  store <16 x float> %call222, <16 x float>* %a16_out, align 64
  %348 = load float* %a_in, align 4
  %call223 = call float @_Z10native_cosf(float %348) nounwind readnone
  store float %call223, float* %a_out, align 4
  %349 = load <4 x float>* %a4_in, align 16
  %call224 = call <4 x float> @_Z10native_cosDv4_f(<4 x float> %349) nounwind readnone
  store <4 x float> %call224, <4 x float>* %a4_out, align 16
  %350 = load <8 x float>* %a8_in, align 32
  %call225 = call <8 x float> @_Z10native_cosDv8_f(<8 x float> %350) nounwind readnone
  store <8 x float> %call225, <8 x float>* %a8_out, align 32
  %351 = load <16 x float>* %a16_in, align 64
  %call226 = call <16 x float> @_Z10native_cosDv16_f(<16 x float> %351) nounwind readnone
  store <16 x float> %call226, <16 x float>* %a16_out, align 64
  %352 = load float* %a_in, align 4
  %call227 = call float @_Z12native_rsqrtf(float %352) nounwind readnone
  store float %call227, float* %a_out, align 4
  %353 = load <4 x float>* %a4_in, align 16
  %call228 = call <4 x float> @_Z12native_rsqrtDv4_f(<4 x float> %353) nounwind readnone
  store <4 x float> %call228, <4 x float>* %a4_out, align 16
  %354 = load <8 x float>* %a8_in, align 32
  %call229 = call <8 x float> @_Z12native_rsqrtDv8_f(<8 x float> %354) nounwind readnone
  store <8 x float> %call229, <8 x float>* %a8_out, align 32
  %355 = load <16 x float>* %a16_in, align 64
  %call230 = call <16 x float> @_Z12native_rsqrtDv16_f(<16 x float> %355) nounwind readnone
  store <16 x float> %call230, <16 x float>* %a16_out, align 64
  %356 = load float* %a_in, align 4
  %call231 = call float @_Z10native_logf(float %356) nounwind readnone
  store float %call231, float* %a_out, align 4
  %357 = load <4 x float>* %a4_in, align 16
  %call232 = call <4 x float> @_Z10native_logDv4_f(<4 x float> %357) nounwind readnone
  store <4 x float> %call232, <4 x float>* %a4_out, align 16
  %358 = load <8 x float>* %a8_in, align 32
  %call233 = call <8 x float> @_Z10native_logDv8_f(<8 x float> %358) nounwind readnone
  store <8 x float> %call233, <8 x float>* %a8_out, align 32
  %359 = load <16 x float>* %a16_in, align 64
  %call234 = call <16 x float> @_Z10native_logDv16_f(<16 x float> %359) nounwind readnone
  store <16 x float> %call234, <16 x float>* %a16_out, align 64
  %360 = load float* %a_in, align 4
  %call235 = call float @_Z11native_log2f(float %360) nounwind readnone
  store float %call235, float* %a_out, align 4
  %361 = load <4 x float>* %a4_in, align 16
  %call236 = call <4 x float> @_Z11native_log2Dv4_f(<4 x float> %361) nounwind readnone
  store <4 x float> %call236, <4 x float>* %a4_out, align 16
  %362 = load <8 x float>* %a8_in, align 32
  %call237 = call <8 x float> @_Z11native_log2Dv8_f(<8 x float> %362) nounwind readnone
  store <8 x float> %call237, <8 x float>* %a8_out, align 32
  %363 = load <16 x float>* %a16_in, align 64
  %call238 = call <16 x float> @_Z11native_log2Dv16_f(<16 x float> %363) nounwind readnone
  store <16 x float> %call238, <16 x float>* %a16_out, align 64
  %364 = load float* %a_in, align 4
  %call239 = call float @_Z12native_log10f(float %364) nounwind readnone
  store float %call239, float* %a_out, align 4
  %365 = load <4 x float>* %a4_in, align 16
  %call240 = call <4 x float> @_Z12native_log10Dv4_f(<4 x float> %365) nounwind readnone
  store <4 x float> %call240, <4 x float>* %a4_out, align 16
  %366 = load <8 x float>* %a8_in, align 32
  %call241 = call <8 x float> @_Z12native_log10Dv8_f(<8 x float> %366) nounwind readnone
  store <8 x float> %call241, <8 x float>* %a8_out, align 32
  %367 = load <16 x float>* %a16_in, align 64
  %call242 = call <16 x float> @_Z12native_log10Dv16_f(<16 x float> %367) nounwind readnone
  store <16 x float> %call242, <16 x float>* %a16_out, align 64
  %368 = load float* %a_in, align 4
  %call243 = call float @_Z10native_expf(float %368) nounwind readnone
  store float %call243, float* %a_out, align 4
  %369 = load <4 x float>* %a4_in, align 16
  %call244 = call <4 x float> @_Z10native_expDv4_f(<4 x float> %369) nounwind readnone
  store <4 x float> %call244, <4 x float>* %a4_out, align 16
  %370 = load <8 x float>* %a8_in, align 32
  %call245 = call <8 x float> @_Z10native_expDv8_f(<8 x float> %370) nounwind readnone
  store <8 x float> %call245, <8 x float>* %a8_out, align 32
  %371 = load <16 x float>* %a16_in, align 64
  %call246 = call <16 x float> @_Z10native_expDv16_f(<16 x float> %371) nounwind readnone
  store <16 x float> %call246, <16 x float>* %a16_out, align 64
  %372 = load float* %a_in, align 4
  %call247 = call float @_Z11native_exp2f(float %372) nounwind readnone
  store float %call247, float* %a_out, align 4
  %373 = load <4 x float>* %a4_in, align 16
  %call248 = call <4 x float> @_Z11native_exp2Dv4_f(<4 x float> %373) nounwind readnone
  store <4 x float> %call248, <4 x float>* %a4_out, align 16
  %374 = load <8 x float>* %a8_in, align 32
  %call249 = call <8 x float> @_Z11native_exp2Dv8_f(<8 x float> %374) nounwind readnone
  store <8 x float> %call249, <8 x float>* %a8_out, align 32
  %375 = load <16 x float>* %a16_in, align 64
  %call250 = call <16 x float> @_Z11native_exp2Dv16_f(<16 x float> %375) nounwind readnone
  store <16 x float> %call250, <16 x float>* %a16_out, align 64
  %376 = load float* %a_in, align 4
  %call251 = call float @_Z12native_exp10f(float %376) nounwind readnone
  store float %call251, float* %a_out, align 4
  %377 = load <4 x float>* %a4_in, align 16
  %call252 = call <4 x float> @_Z12native_exp10Dv4_f(<4 x float> %377) nounwind readnone
  store <4 x float> %call252, <4 x float>* %a4_out, align 16
  %378 = load <8 x float>* %a8_in, align 32
  %call253 = call <8 x float> @_Z12native_exp10Dv8_f(<8 x float> %378) nounwind readnone
  store <8 x float> %call253, <8 x float>* %a8_out, align 32
  %379 = load <16 x float>* %a16_in, align 64
  %call254 = call <16 x float> @_Z12native_exp10Dv16_f(<16 x float> %379) nounwind readnone
  store <16 x float> %call254, <16 x float>* %a16_out, align 64
  %380 = load float* %a_in, align 4
  %381 = load float* %b_in, align 4
  %call255 = call float @_Z13native_divideff(float %380, float %381) nounwind readnone
  store float %call255, float* %a_out, align 4
  %382 = load <4 x float>* %a4_in, align 16
  %383 = load <4 x float>* %b4_in, align 16
  %call256 = call <4 x float> @_Z13native_divideDv4_fS_(<4 x float> %382, <4 x float> %383) nounwind readnone
  store <4 x float> %call256, <4 x float>* %a4_out, align 16
  %384 = load <8 x float>* %a8_in, align 32
  %385 = load <8 x float>* %b8_in, align 32
  %call257 = call <8 x float> @_Z13native_divideDv8_fS_(<8 x float> %384, <8 x float> %385) nounwind readnone
  store <8 x float> %call257, <8 x float>* %a8_out, align 32
  %386 = load <16 x float>* %a16_in, align 64
  %387 = load <16 x float>* %b16_in, align 64
  %call258 = call <16 x float> @_Z13native_divideDv16_fS_(<16 x float> %386, <16 x float> %387) nounwind readnone
  store <16 x float> %call258, <16 x float>* %a16_out, align 64
  %388 = load float* %a_in, align 4
  %389 = load float* %b_in, align 4
  %call259 = call float @_Z11native_powrff(float %388, float %389) nounwind readnone
  store float %call259, float* %a_out, align 4
  %390 = load <4 x float>* %a4_in, align 16
  %391 = load <4 x float>* %b4_in, align 16
  %call260 = call <4 x float> @_Z11native_powrDv4_fS_(<4 x float> %390, <4 x float> %391) nounwind readnone
  store <4 x float> %call260, <4 x float>* %a4_out, align 16
  %392 = load <8 x float>* %a8_in, align 32
  %393 = load <8 x float>* %b8_in, align 32
  %call261 = call <8 x float> @_Z11native_powrDv8_fS_(<8 x float> %392, <8 x float> %393) nounwind readnone
  store <8 x float> %call261, <8 x float>* %a8_out, align 32
  %394 = load <16 x float>* %a16_in, align 64
  %395 = load <16 x float>* %b16_in, align 64
  %call262 = call <16 x float> @_Z11native_powrDv16_fS_(<16 x float> %394, <16 x float> %395) nounwind readnone
  store <16 x float> %call262, <16 x float>* %a16_out, align 64
  %396 = load float* %a_in, align 4
  %call263 = call float @_Z12native_recipf(float %396) nounwind readnone
  store float %call263, float* %a_out, align 4
  %397 = load <4 x float>* %a4_in, align 16
  %call264 = call <4 x float> @_Z12native_recipDv4_f(<4 x float> %397) nounwind readnone
  store <4 x float> %call264, <4 x float>* %a4_out, align 16
  %398 = load <8 x float>* %a8_in, align 32
  %call265 = call <8 x float> @_Z12native_recipDv8_f(<8 x float> %398) nounwind readnone
  store <8 x float> %call265, <8 x float>* %a8_out, align 32
  %399 = load <16 x float>* %a16_in, align 64
  %call266 = call <16 x float> @_Z12native_recipDv16_f(<16 x float> %399) nounwind readnone
  store <16 x float> %call266, <16 x float>* %a16_out, align 64
  %400 = load float* %a_in, align 4
  %call267 = call float @_Z11native_sqrtf(float %400) nounwind readnone
  store float %call267, float* %a_out, align 4
  %401 = load <4 x float>* %a4_in, align 16
  %call268 = call <4 x float> @_Z11native_sqrtDv4_f(<4 x float> %401) nounwind readnone
  store <4 x float> %call268, <4 x float>* %a4_out, align 16
  %402 = load <8 x float>* %a8_in, align 32
  %call269 = call <8 x float> @_Z11native_sqrtDv8_f(<8 x float> %402) nounwind readnone
  store <8 x float> %call269, <8 x float>* %a8_out, align 32
  %403 = load <16 x float>* %a16_in, align 64
  %call270 = call <16 x float> @_Z11native_sqrtDv16_f(<16 x float> %403) nounwind readnone
  store <16 x float> %call270, <16 x float>* %a16_out, align 64
  %404 = load float* %a_in, align 4
  %call271 = call float @_Z10native_tanf(float %404) nounwind readnone
  store float %call271, float* %a_out, align 4
  %405 = load <4 x float>* %a4_in, align 16
  %call272 = call <4 x float> @_Z10native_tanDv4_f(<4 x float> %405) nounwind readnone
  store <4 x float> %call272, <4 x float>* %a4_out, align 16
  %406 = load <8 x float>* %a8_in, align 32
  %call273 = call <8 x float> @_Z10native_tanDv8_f(<8 x float> %406) nounwind readnone
  store <8 x float> %call273, <8 x float>* %a8_out, align 32
  %407 = load <16 x float>* %a16_in, align 64
  %call274 = call <16 x float> @_Z10native_tanDv16_f(<16 x float> %407) nounwind readnone
  store <16 x float> %call274, <16 x float>* %a16_out, align 64
  %408 = load float* %a_in, align 4
  %call275 = call float @_Z8half_logf(float %408) nounwind readnone
  store float %call275, float* %a_out, align 4
  %409 = load <4 x float>* %a4_in, align 16
  %call276 = call <4 x float> @_Z8half_logDv4_f(<4 x float> %409) nounwind readnone
  store <4 x float> %call276, <4 x float>* %a4_out, align 16
  %410 = load <8 x float>* %a8_in, align 32
  %call277 = call <8 x float> @_Z8half_logDv8_f(<8 x float> %410) nounwind readnone
  store <8 x float> %call277, <8 x float>* %a8_out, align 32
  %411 = load <16 x float>* %a16_in, align 64
  %call278 = call <16 x float> @_Z8half_logDv16_f(<16 x float> %411) nounwind readnone
  store <16 x float> %call278, <16 x float>* %a16_out, align 64
  %412 = load float* %a_in, align 4
  %call279 = call float @_Z9half_log2f(float %412) nounwind readnone
  store float %call279, float* %a_out, align 4
  %413 = load <4 x float>* %a4_in, align 16
  %call280 = call <4 x float> @_Z9half_log2Dv4_f(<4 x float> %413) nounwind readnone
  store <4 x float> %call280, <4 x float>* %a4_out, align 16
  %414 = load <8 x float>* %a8_in, align 32
  %call281 = call <8 x float> @_Z9half_log2Dv8_f(<8 x float> %414) nounwind readnone
  store <8 x float> %call281, <8 x float>* %a8_out, align 32
  %415 = load <16 x float>* %a16_in, align 64
  %call282 = call <16 x float> @_Z9half_log2Dv16_f(<16 x float> %415) nounwind readnone
  store <16 x float> %call282, <16 x float>* %a16_out, align 64
  %416 = load float* %a_in, align 4
  %call283 = call float @_Z10half_log10f(float %416) nounwind readnone
  store float %call283, float* %a_out, align 4
  %417 = load <4 x float>* %a4_in, align 16
  %call284 = call <4 x float> @_Z10half_log10Dv4_f(<4 x float> %417) nounwind readnone
  store <4 x float> %call284, <4 x float>* %a4_out, align 16
  %418 = load <8 x float>* %a8_in, align 32
  %call285 = call <8 x float> @_Z10half_log10Dv8_f(<8 x float> %418) nounwind readnone
  store <8 x float> %call285, <8 x float>* %a8_out, align 32
  %419 = load <16 x float>* %a16_in, align 64
  %call286 = call <16 x float> @_Z10half_log10Dv16_f(<16 x float> %419) nounwind readnone
  store <16 x float> %call286, <16 x float>* %a16_out, align 64
  %420 = load float* %a_in, align 4
  %call287 = call float @_Z8half_expf(float %420) nounwind readnone
  store float %call287, float* %a_out, align 4
  %421 = load <4 x float>* %a4_in, align 16
  %call288 = call <4 x float> @_Z8half_expDv4_f(<4 x float> %421) nounwind readnone
  store <4 x float> %call288, <4 x float>* %a4_out, align 16
  %422 = load <8 x float>* %a8_in, align 32
  %call289 = call <8 x float> @_Z8half_expDv8_f(<8 x float> %422) nounwind readnone
  store <8 x float> %call289, <8 x float>* %a8_out, align 32
  %423 = load <16 x float>* %a16_in, align 64
  %call290 = call <16 x float> @_Z8half_expDv16_f(<16 x float> %423) nounwind readnone
  store <16 x float> %call290, <16 x float>* %a16_out, align 64
  %424 = load float* %a_in, align 4
  %call291 = call float @_Z9half_exp2f(float %424) nounwind readnone
  store float %call291, float* %a_out, align 4
  %425 = load <4 x float>* %a4_in, align 16
  %call292 = call <4 x float> @_Z9half_exp2Dv4_f(<4 x float> %425) nounwind readnone
  store <4 x float> %call292, <4 x float>* %a4_out, align 16
  %426 = load <8 x float>* %a8_in, align 32
  %call293 = call <8 x float> @_Z9half_exp2Dv8_f(<8 x float> %426) nounwind readnone
  store <8 x float> %call293, <8 x float>* %a8_out, align 32
  %427 = load <16 x float>* %a16_in, align 64
  %call294 = call <16 x float> @_Z9half_exp2Dv16_f(<16 x float> %427) nounwind readnone
  store <16 x float> %call294, <16 x float>* %a16_out, align 64
  %428 = load float* %a_in, align 4
  %call295 = call float @_Z10half_exp10f(float %428) nounwind readnone
  store float %call295, float* %a_out, align 4
  %429 = load <4 x float>* %a4_in, align 16
  %call296 = call <4 x float> @_Z10half_exp10Dv4_f(<4 x float> %429) nounwind readnone
  store <4 x float> %call296, <4 x float>* %a4_out, align 16
  %430 = load <8 x float>* %a8_in, align 32
  %call297 = call <8 x float> @_Z10half_exp10Dv8_f(<8 x float> %430) nounwind readnone
  store <8 x float> %call297, <8 x float>* %a8_out, align 32
  %431 = load <16 x float>* %a16_in, align 64
  %call298 = call <16 x float> @_Z10half_exp10Dv16_f(<16 x float> %431) nounwind readnone
  store <16 x float> %call298, <16 x float>* %a16_out, align 64
  %432 = load float* %a_in, align 4
  %call299 = call float @_Z8half_cosf(float %432) nounwind readnone
  store float %call299, float* %a_out, align 4
  %433 = load <4 x float>* %a4_in, align 16
  %call300 = call <4 x float> @_Z8half_cosDv4_f(<4 x float> %433) nounwind readnone
  store <4 x float> %call300, <4 x float>* %a4_out, align 16
  %434 = load <8 x float>* %a8_in, align 32
  %call301 = call <8 x float> @_Z8half_cosDv8_f(<8 x float> %434) nounwind readnone
  store <8 x float> %call301, <8 x float>* %a8_out, align 32
  %435 = load <16 x float>* %a16_in, align 64
  %call302 = call <16 x float> @_Z8half_cosDv16_f(<16 x float> %435) nounwind readnone
  store <16 x float> %call302, <16 x float>* %a16_out, align 64
  %436 = load float* %a_in, align 4
  %437 = load float* %b_in, align 4
  %call303 = call float @_Z11half_divideff(float %436, float %437) nounwind readnone
  store float %call303, float* %a_out, align 4
  %438 = load <4 x float>* %a4_in, align 16
  %439 = load <4 x float>* %b4_in, align 16
  %call304 = call <4 x float> @_Z11half_divideDv4_fS_(<4 x float> %438, <4 x float> %439) nounwind readnone
  store <4 x float> %call304, <4 x float>* %a4_out, align 16
  %440 = load <8 x float>* %a8_in, align 32
  %441 = load <8 x float>* %b8_in, align 32
  %call305 = call <8 x float> @_Z11half_divideDv8_fS_(<8 x float> %440, <8 x float> %441) nounwind readnone
  store <8 x float> %call305, <8 x float>* %a8_out, align 32
  %442 = load <16 x float>* %a16_in, align 64
  %443 = load <16 x float>* %b16_in, align 64
  %call306 = call <16 x float> @_Z11half_divideDv16_fS_(<16 x float> %442, <16 x float> %443) nounwind readnone
  store <16 x float> %call306, <16 x float>* %a16_out, align 64
  %444 = load float* %a_in, align 4
  %445 = load float* %b_in, align 4
  %call307 = call float @_Z9half_powrff(float %444, float %445) nounwind readnone
  store float %call307, float* %a_out, align 4
  %446 = load <4 x float>* %a4_in, align 16
  %447 = load <4 x float>* %b4_in, align 16
  %call308 = call <4 x float> @_Z9half_powrDv4_fS_(<4 x float> %446, <4 x float> %447) nounwind readnone
  store <4 x float> %call308, <4 x float>* %a4_out, align 16
  %448 = load <8 x float>* %a8_in, align 32
  %449 = load <8 x float>* %b8_in, align 32
  %call309 = call <8 x float> @_Z9half_powrDv8_fS_(<8 x float> %448, <8 x float> %449) nounwind readnone
  store <8 x float> %call309, <8 x float>* %a8_out, align 32
  %450 = load <16 x float>* %a16_in, align 64
  %451 = load <16 x float>* %b16_in, align 64
  %call310 = call <16 x float> @_Z9half_powrDv16_fS_(<16 x float> %450, <16 x float> %451) nounwind readnone
  store <16 x float> %call310, <16 x float>* %a16_out, align 64
  %452 = load float* %a_in, align 4
  %call311 = call float @_Z10half_recipf(float %452) nounwind readnone
  store float %call311, float* %a_out, align 4
  %453 = load <4 x float>* %a4_in, align 16
  %call312 = call <4 x float> @_Z10half_recipDv4_f(<4 x float> %453) nounwind readnone
  store <4 x float> %call312, <4 x float>* %a4_out, align 16
  %454 = load <8 x float>* %a8_in, align 32
  %call313 = call <8 x float> @_Z10half_recipDv8_f(<8 x float> %454) nounwind readnone
  store <8 x float> %call313, <8 x float>* %a8_out, align 32
  %455 = load <16 x float>* %a16_in, align 64
  %call314 = call <16 x float> @_Z10half_recipDv16_f(<16 x float> %455) nounwind readnone
  store <16 x float> %call314, <16 x float>* %a16_out, align 64
  %456 = load float* %a_in, align 4
  %call315 = call float @_Z10half_rsqrtf(float %456) nounwind readnone
  store float %call315, float* %a_out, align 4
  %457 = load <4 x float>* %a4_in, align 16
  %call316 = call <4 x float> @_Z10half_rsqrtDv4_f(<4 x float> %457) nounwind readnone
  store <4 x float> %call316, <4 x float>* %a4_out, align 16
  %458 = load <8 x float>* %a8_in, align 32
  %call317 = call <8 x float> @_Z10half_rsqrtDv8_f(<8 x float> %458) nounwind readnone
  store <8 x float> %call317, <8 x float>* %a8_out, align 32
  %459 = load <16 x float>* %a16_in, align 64
  %call318 = call <16 x float> @_Z10half_rsqrtDv16_f(<16 x float> %459) nounwind readnone
  store <16 x float> %call318, <16 x float>* %a16_out, align 64
  %460 = load float* %a_in, align 4
  %call319 = call float @_Z8half_sinf(float %460) nounwind readnone
  store float %call319, float* %a_out, align 4
  %461 = load <4 x float>* %a4_in, align 16
  %call320 = call <4 x float> @_Z8half_sinDv4_f(<4 x float> %461) nounwind readnone
  store <4 x float> %call320, <4 x float>* %a4_out, align 16
  %462 = load <8 x float>* %a8_in, align 32
  %call321 = call <8 x float> @_Z8half_sinDv8_f(<8 x float> %462) nounwind readnone
  store <8 x float> %call321, <8 x float>* %a8_out, align 32
  %463 = load <16 x float>* %a16_in, align 64
  %call322 = call <16 x float> @_Z8half_sinDv16_f(<16 x float> %463) nounwind readnone
  store <16 x float> %call322, <16 x float>* %a16_out, align 64
  %464 = load float* %a_in, align 4
  %call323 = call float @_Z9half_sqrtf(float %464) nounwind readnone
  store float %call323, float* %a_out, align 4
  %465 = load <4 x float>* %a4_in, align 16
  %call324 = call <4 x float> @_Z9half_sqrtDv4_f(<4 x float> %465) nounwind readnone
  store <4 x float> %call324, <4 x float>* %a4_out, align 16
  %466 = load <8 x float>* %a8_in, align 32
  %call325 = call <8 x float> @_Z9half_sqrtDv8_f(<8 x float> %466) nounwind readnone
  store <8 x float> %call325, <8 x float>* %a8_out, align 32
  %467 = load <16 x float>* %a16_in, align 64
  %call326 = call <16 x float> @_Z9half_sqrtDv16_f(<16 x float> %467) nounwind readnone
  store <16 x float> %call326, <16 x float>* %a16_out, align 64
  %468 = load float* %a_in, align 4
  %call327 = call float @_Z8half_tanf(float %468) nounwind readnone
  store float %call327, float* %a_out, align 4
  %469 = load <4 x float>* %a4_in, align 16
  %call328 = call <4 x float> @_Z8half_tanDv4_f(<4 x float> %469) nounwind readnone
  store <4 x float> %call328, <4 x float>* %a4_out, align 16
  %470 = load <8 x float>* %a8_in, align 32
  %call329 = call <8 x float> @_Z8half_tanDv8_f(<8 x float> %470) nounwind readnone
  store <8 x float> %call329, <8 x float>* %a8_out, align 32
  %471 = load <16 x float>* %a16_in, align 64
  %call330 = call <16 x float> @_Z8half_tanDv16_f(<16 x float> %471) nounwind readnone
  store <16 x float> %call330, <16 x float>* %a16_out, align 64
  %472 = load float* %a_in, align 4
  %call331 = call float @_Z5asinhf(float %472) nounwind readnone
  store float %call331, float* %a_out, align 4
  %473 = load <4 x float>* %a4_in, align 16
  %call332 = call <4 x float> @_Z5asinhDv4_f(<4 x float> %473) nounwind readnone
  store <4 x float> %call332, <4 x float>* %a4_out, align 16
  %474 = load <8 x float>* %a8_in, align 32
  %call333 = call <8 x float> @_Z5asinhDv8_f(<8 x float> %474) nounwind readnone
  store <8 x float> %call333, <8 x float>* %a8_out, align 32
  %475 = load <16 x float>* %a16_in, align 64
  %call334 = call <16 x float> @_Z5asinhDv16_f(<16 x float> %475) nounwind readnone
  store <16 x float> %call334, <16 x float>* %a16_out, align 64
  %476 = load float* %a_in, align 4
  %call335 = call float @_Z5acoshf(float %476) nounwind readnone
  store float %call335, float* %a_out, align 4
  %477 = load <4 x float>* %a4_in, align 16
  %call336 = call <4 x float> @_Z5acoshDv4_f(<4 x float> %477) nounwind readnone
  store <4 x float> %call336, <4 x float>* %a4_out, align 16
  %478 = load <8 x float>* %a8_in, align 32
  %call337 = call <8 x float> @_Z5acoshDv8_f(<8 x float> %478) nounwind readnone
  store <8 x float> %call337, <8 x float>* %a8_out, align 32
  %479 = load <16 x float>* %a16_in, align 64
  %call338 = call <16 x float> @_Z5acoshDv16_f(<16 x float> %479) nounwind readnone
  store <16 x float> %call338, <16 x float>* %a16_out, align 64
  %480 = load float* %a_in, align 4
  %call339 = call float @_Z5atanhf(float %480) nounwind readnone
  store float %call339, float* %a_out, align 4
  %481 = load <4 x float>* %a4_in, align 16
  %call340 = call <4 x float> @_Z5atanhDv4_f(<4 x float> %481) nounwind readnone
  store <4 x float> %call340, <4 x float>* %a4_out, align 16
  %482 = load <8 x float>* %a8_in, align 32
  %call341 = call <8 x float> @_Z5atanhDv8_f(<8 x float> %482) nounwind readnone
  store <8 x float> %call341, <8 x float>* %a8_out, align 32
  %483 = load <16 x float>* %a16_in, align 64
  %call342 = call <16 x float> @_Z5atanhDv16_f(<16 x float> %483) nounwind readnone
  store <16 x float> %call342, <16 x float>* %a16_out, align 64
  %call343 = call <4 x float> @_Z6vload4jPKf(i32 0, float* %b_in)
  store <4 x float> %call343, <4 x float>* %a4_out, align 16
  %call344 = call <8 x float> @_Z6vload8jPKf(i32 0, float* %b_in)
  store <8 x float> %call344, <8 x float>* %a8_out, align 32
  %call345 = call <16 x float> @_Z7vload16jPKf(i32 0, float* %b_in)
  store <16 x float> %call345, <16 x float>* %a16_out, align 64
  %484 = load <4 x float>* %a4_in, align 16
  %485 = bitcast <4 x float>* %a4_out to float*
  call void @_Z7vstore4Dv4_fjPf(<4 x float> %484, i32 0, float* %485)
  %486 = load <8 x float>* %a8_in, align 32
  %487 = bitcast <8 x float>* %a8_out to float*
  call void @_Z7vstore8Dv8_fjPf(<8 x float> %486, i32 0, float* %487)
  %488 = load <16 x float>* %a16_in, align 64
  %489 = bitcast <16 x float>* %a16_out to float*
  call void @_Z8vstore16Dv16_fjPf(<16 x float> %488, i32 0, float* %489)
  %490 = load float* %a_in, align 4
  %491 = load float* %b_in, align 4
  %call346 = call float @_Z3minff(float %490, float %491) nounwind readnone
  store float %call346, float* %a_out, align 4
  %492 = load <4 x float>* %a4_in, align 16
  %493 = load <4 x float>* %b4_in, align 16
  %call347 = call <4 x float> @_Z3minDv4_fS_(<4 x float> %492, <4 x float> %493) nounwind readnone
  store <4 x float> %call347, <4 x float>* %a4_out, align 16
  %494 = load <8 x float>* %a8_in, align 32
  %495 = load <8 x float>* %b8_in, align 32
  %call348 = call <8 x float> @_Z3minDv8_fS_(<8 x float> %494, <8 x float> %495) nounwind readnone
  store <8 x float> %call348, <8 x float>* %a8_out, align 32
  %496 = load <16 x float>* %a16_in, align 64
  %497 = load <16 x float>* %b16_in, align 64
  %call349 = call <16 x float> @_Z3minDv16_fS_(<16 x float> %496, <16 x float> %497) nounwind readnone
  store <16 x float> %call349, <16 x float>* %a16_out, align 64
  %498 = load <4 x float>* %a4_in, align 16
  %499 = load float* %b_in, align 4
  %call350 = call <4 x float> @_Z3minDv4_ff(<4 x float> %498, float %499) nounwind readnone
  store <4 x float> %call350, <4 x float>* %a4_out, align 16
  %500 = load <8 x float>* %a8_in, align 32
  %501 = load float* %b_in, align 4
  %call351 = call <8 x float> @_Z3minDv8_ff(<8 x float> %500, float %501) nounwind readnone
  store <8 x float> %call351, <8 x float>* %a8_out, align 32
  %502 = load <16 x float>* %a16_in, align 64
  %503 = load float* %b_in, align 4
  %call352 = call <16 x float> @_Z3minDv16_ff(<16 x float> %502, float %503) nounwind readnone
  store <16 x float> %call352, <16 x float>* %a16_out, align 64
  %504 = load float* %a_in, align 4
  %505 = load float* %b_in, align 4
  %call353 = call float @_Z3maxff(float %504, float %505) nounwind readnone
  store float %call353, float* %a_out, align 4
  %506 = load <4 x float>* %a4_in, align 16
  %507 = load <4 x float>* %b4_in, align 16
  %call354 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %506, <4 x float> %507) nounwind readnone
  store <4 x float> %call354, <4 x float>* %a4_out, align 16
  %508 = load <8 x float>* %a8_in, align 32
  %509 = load <8 x float>* %b8_in, align 32
  %call355 = call <8 x float> @_Z3maxDv8_fS_(<8 x float> %508, <8 x float> %509) nounwind readnone
  store <8 x float> %call355, <8 x float>* %a8_out, align 32
  %510 = load <16 x float>* %a16_in, align 64
  %511 = load <16 x float>* %b16_in, align 64
  %call356 = call <16 x float> @_Z3maxDv16_fS_(<16 x float> %510, <16 x float> %511) nounwind readnone
  store <16 x float> %call356, <16 x float>* %a16_out, align 64
  %512 = load <4 x float>* %a4_in, align 16
  %513 = load float* %b_in, align 4
  %call357 = call <4 x float> @_Z3maxDv4_ff(<4 x float> %512, float %513) nounwind readnone
  store <4 x float> %call357, <4 x float>* %a4_out, align 16
  %514 = load <8 x float>* %a8_in, align 32
  %515 = load float* %b_in, align 4
  %call358 = call <8 x float> @_Z3maxDv8_ff(<8 x float> %514, float %515) nounwind readnone
  store <8 x float> %call358, <8 x float>* %a8_out, align 32
  %516 = load <16 x float>* %a16_in, align 64
  %517 = load float* %b_in, align 4
  %call359 = call <16 x float> @_Z3maxDv16_ff(<16 x float> %516, float %517) nounwind readnone
  store <16 x float> %call359, <16 x float>* %a16_out, align 64
  %518 = load float* %a_in, align 4
  %519 = load float* %b_in, align 4
  %call360 = call float @_Z5hypotff(float %518, float %519) nounwind readnone
  store float %call360, float* %a_out, align 4
  %520 = load <4 x float>* %a4_in, align 16
  %521 = load <4 x float>* %b4_in, align 16
  %call361 = call <4 x float> @_Z5hypotDv4_fS_(<4 x float> %520, <4 x float> %521) nounwind readnone
  store <4 x float> %call361, <4 x float>* %a4_out, align 16
  %522 = load <8 x float>* %a8_in, align 32
  %523 = load <8 x float>* %b8_in, align 32
  %call362 = call <8 x float> @_Z5hypotDv8_fS_(<8 x float> %522, <8 x float> %523) nounwind readnone
  store <8 x float> %call362, <8 x float>* %a8_out, align 32
  %524 = load <16 x float>* %a16_in, align 64
  %525 = load <16 x float>* %b16_in, align 64
  %call363 = call <16 x float> @_Z5hypotDv16_fS_(<16 x float> %524, <16 x float> %525) nounwind readnone
  store <16 x float> %call363, <16 x float>* %a16_out, align 64
  %526 = load float* %a_in, align 4
  %527 = load float* %b_in, align 4
  %call364 = call float @_Z4stepff(float %526, float %527) nounwind readnone
  store float %call364, float* %a_out, align 4
  %528 = load <4 x float>* %a4_in, align 16
  %529 = load <4 x float>* %b4_in, align 16
  %call365 = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %528, <4 x float> %529) nounwind readnone
  store <4 x float> %call365, <4 x float>* %a4_out, align 16
  %530 = load <8 x float>* %a8_in, align 32
  %531 = load <8 x float>* %b8_in, align 32
  %call366 = call <8 x float> @_Z4stepDv8_fS_(<8 x float> %530, <8 x float> %531) nounwind readnone
  store <8 x float> %call366, <8 x float>* %a8_out, align 32
  %532 = load <16 x float>* %a16_in, align 64
  %533 = load <16 x float>* %b16_in, align 64
  %call367 = call <16 x float> @_Z4stepDv16_fS_(<16 x float> %532, <16 x float> %533) nounwind readnone
  store <16 x float> %call367, <16 x float>* %a16_out, align 64
  %534 = load float* %a_in, align 4
  %535 = load float* %b_in, align 4
  %call368 = call float @_Z4stepff(float %534, float %535) nounwind readnone
  store float %call368, float* %a_out, align 4
  %536 = load float* %a_in, align 4
  %537 = load <4 x float>* %b4_in, align 16
  %call369 = call <4 x float> @_Z4stepfDv4_f(float %536, <4 x float> %537) nounwind readnone
  store <4 x float> %call369, <4 x float>* %a4_out, align 16
  %538 = load float* %a_in, align 4
  %539 = load <8 x float>* %b8_in, align 32
  %call370 = call <8 x float> @_Z4stepfDv8_f(float %538, <8 x float> %539) nounwind readnone
  store <8 x float> %call370, <8 x float>* %a8_out, align 32
  %540 = load float* %a_in, align 4
  %541 = load <16 x float>* %b16_in, align 64
  %call371 = call <16 x float> @_Z4stepfDv16_f(float %540, <16 x float> %541) nounwind readnone
  store <16 x float> %call371, <16 x float>* %a16_out, align 64
  %542 = load float* %a_in, align 4
  %543 = load float* %b_in, align 4
  %544 = load float* %c_in, align 4
  %call372 = call float @_Z10smoothstepfff(float %542, float %543, float %544) nounwind readnone
  store float %call372, float* %a_out, align 4
  %545 = load <4 x float>* %a4_in, align 16
  %546 = load <4 x float>* %b4_in, align 16
  %547 = load <4 x float>* %c4_in, align 16
  %call373 = call <4 x float> @_Z10smoothstepDv4_fS_S_(<4 x float> %545, <4 x float> %546, <4 x float> %547) nounwind readnone
  store <4 x float> %call373, <4 x float>* %a4_out, align 16
  %548 = load <8 x float>* %a8_in, align 32
  %549 = load <8 x float>* %b8_in, align 32
  %550 = load <8 x float>* %c8_in, align 32
  %call374 = call <8 x float> @_Z10smoothstepDv8_fS_S_(<8 x float> %548, <8 x float> %549, <8 x float> %550) nounwind readnone
  store <8 x float> %call374, <8 x float>* %a8_out, align 32
  %551 = load <16 x float>* %a16_in, align 64
  %552 = load <16 x float>* %b16_in, align 64
  %553 = load <16 x float>* %c16_in, align 64
  %call375 = call <16 x float> @_Z10smoothstepDv16_fS_S_(<16 x float> %551, <16 x float> %552, <16 x float> %553) nounwind readnone
  store <16 x float> %call375, <16 x float>* %a16_out, align 64
  %554 = load float* %a_in, align 4
  %555 = load float* %b_in, align 4
  %556 = load float* %c_in, align 4
  %call376 = call float @_Z10smoothstepfff(float %554, float %555, float %556) nounwind readnone
  store float %call376, float* %a_out, align 4
  %557 = load float* %a_in, align 4
  %558 = load float* %b_in, align 4
  %559 = load <4 x float>* %c4_in, align 16
  %call377 = call <4 x float> @_Z10smoothstepffDv4_f(float %557, float %558, <4 x float> %559) nounwind readnone
  store <4 x float> %call377, <4 x float>* %a4_out, align 16
  %560 = load float* %a_in, align 4
  %561 = load float* %b_in, align 4
  %562 = load <8 x float>* %c8_in, align 32
  %call378 = call <8 x float> @_Z10smoothstepffDv8_f(float %560, float %561, <8 x float> %562) nounwind readnone
  store <8 x float> %call378, <8 x float>* %a8_out, align 32
  %563 = load float* %a_in, align 4
  %564 = load float* %b_in, align 4
  %565 = load <16 x float>* %c16_in, align 64
  %call379 = call <16 x float> @_Z10smoothstepffDv16_f(float %563, float %564, <16 x float> %565) nounwind readnone
  store <16 x float> %call379, <16 x float>* %a16_out, align 64
  %566 = load float* %a_in, align 4
  %call380 = call float @_Z7radiansf(float %566) nounwind readnone
  store float %call380, float* %a_out, align 4
  %567 = load <4 x float>* %a4_in, align 16
  %call381 = call <4 x float> @_Z7radiansDv4_f(<4 x float> %567) nounwind readnone
  store <4 x float> %call381, <4 x float>* %a4_out, align 16
  %568 = load <8 x float>* %a8_in, align 32
  %call382 = call <8 x float> @_Z7radiansDv8_f(<8 x float> %568) nounwind readnone
  store <8 x float> %call382, <8 x float>* %a8_out, align 32
  %569 = load <16 x float>* %a16_in, align 64
  %call383 = call <16 x float> @_Z7radiansDv16_f(<16 x float> %569) nounwind readnone
  store <16 x float> %call383, <16 x float>* %a16_out, align 64
  %570 = load float* %a_in, align 4
  %call384 = call float @_Z7degreesf(float %570) nounwind readnone
  store float %call384, float* %a_out, align 4
  %571 = load <4 x float>* %a4_in, align 16
  %call385 = call <4 x float> @_Z7degreesDv4_f(<4 x float> %571) nounwind readnone
  store <4 x float> %call385, <4 x float>* %a4_out, align 16
  %572 = load <8 x float>* %a8_in, align 32
  %call386 = call <8 x float> @_Z7degreesDv8_f(<8 x float> %572) nounwind readnone
  store <8 x float> %call386, <8 x float>* %a8_out, align 32
  %573 = load <16 x float>* %a16_in, align 64
  %call387 = call <16 x float> @_Z7degreesDv16_f(<16 x float> %573) nounwind readnone
  store <16 x float> %call387, <16 x float>* %a16_out, align 64
  %574 = load float* %a_in, align 4
  %call388 = call float @_Z4signf(float %574) nounwind readnone
  store float %call388, float* %a_out, align 4
  %575 = load <4 x float>* %a4_in, align 16
  %call389 = call <4 x float> @_Z4signDv4_f(<4 x float> %575) nounwind readnone
  store <4 x float> %call389, <4 x float>* %a4_out, align 16
  %576 = load <8 x float>* %a8_in, align 32
  %call390 = call <8 x float> @_Z4signDv8_f(<8 x float> %576) nounwind readnone
  store <8 x float> %call390, <8 x float>* %a8_out, align 32
  %577 = load <16 x float>* %a16_in, align 64
  %call391 = call <16 x float> @_Z4signDv16_f(<16 x float> %577) nounwind readnone
  store <16 x float> %call391, <16 x float>* %a16_out, align 64
  %578 = load float* %a_in, align 4
  %call392 = call float @_Z5floorf(float %578) nounwind readnone
  store float %call392, float* %a_out, align 4
  %579 = load <4 x float>* %a4_in, align 16
  %call393 = call <4 x float> @_Z5floorDv4_f(<4 x float> %579) nounwind readnone
  store <4 x float> %call393, <4 x float>* %a4_out, align 16
  %580 = load <8 x float>* %a8_in, align 32
  %call394 = call <8 x float> @_Z5floorDv8_f(<8 x float> %580) nounwind readnone
  store <8 x float> %call394, <8 x float>* %a8_out, align 32
  %581 = load <16 x float>* %a16_in, align 64
  %call395 = call <16 x float> @_Z5floorDv16_f(<16 x float> %581) nounwind readnone
  store <16 x float> %call395, <16 x float>* %a16_out, align 64
  %582 = load float* %a_in, align 4
  %583 = load float* %b_in, align 4
  %call396 = call float @_Z3dotff(float %582, float %583) nounwind readnone
  store float %call396, float* %a_out, align 4
  %584 = load <4 x float>* %a4_in, align 16
  %585 = load <4 x float>* %b4_in, align 16
  %call397 = call float @_Z3dotDv4_fS_(<4 x float> %584, <4 x float> %585) nounwind readnone
  store float %call397, float* %a_out, align 4
  %586 = load float* %a_in, align 4
  %587 = load float* %b_in, align 4
  %588 = load float* %c_in, align 4
  %call398 = call float @_Z3mixfff(float %586, float %587, float %588) nounwind readnone
  store float %call398, float* %a_out, align 4
  %589 = load <4 x float>* %a4_in, align 16
  %590 = load <4 x float>* %b4_in, align 16
  %591 = load <4 x float>* %c4_in, align 16
  %call399 = call <4 x float> @_Z3mixDv4_fS_S_(<4 x float> %589, <4 x float> %590, <4 x float> %591) nounwind readnone
  store <4 x float> %call399, <4 x float>* %a4_out, align 16
  %592 = load <4 x float>* %a4_in, align 16
  %593 = load <4 x float>* %b4_in, align 16
  %594 = load float* %c_in, align 4
  %call400 = call <4 x float> @_Z3mixDv4_fS_f(<4 x float> %592, <4 x float> %593, float %594) nounwind readnone
  store <4 x float> %call400, <4 x float>* %a4_out, align 16
  %595 = load <8 x float>* %a8_in, align 32
  %596 = load <8 x float>* %b8_in, align 32
  %597 = load float* %c_in, align 4
  %call401 = call <8 x float> @_Z3mixDv8_fS_f(<8 x float> %595, <8 x float> %596, float %597) nounwind readnone
  store <8 x float> %call401, <8 x float>* %a8_out, align 32
  %598 = load <16 x float>* %a16_in, align 64
  %599 = load <16 x float>* %b16_in, align 64
  %600 = load float* %c_in, align 4
  %call402 = call <16 x float> @_Z3mixDv16_fS_f(<16 x float> %598, <16 x float> %599, float %600) nounwind readnone
  store <16 x float> %call402, <16 x float>* %a16_out, align 64
  %601 = load <8 x float>* %a8_in, align 32
  %602 = load <8 x float>* %b8_in, align 32
  %603 = load <8 x float>* %c8_in, align 32
  %call403 = call <8 x float> @_Z3mixDv8_fS_S_(<8 x float> %601, <8 x float> %602, <8 x float> %603) nounwind readnone
  store <8 x float> %call403, <8 x float>* %a8_out, align 32
  %604 = load <16 x float>* %a16_in, align 64
  %605 = load <16 x float>* %b16_in, align 64
  %606 = load <16 x float>* %c16_in, align 64
  %call404 = call <16 x float> @_Z3mixDv16_fS_S_(<16 x float> %604, <16 x float> %605, <16 x float> %606) nounwind readnone
  store <16 x float> %call404, <16 x float>* %a16_out, align 64
  %607 = load float* %a_in, align 4
  %call405 = call float @_Z9normalizef(float %607) nounwind readnone
  store float %call405, float* %a_out, align 4
  %608 = load <4 x float>* %a4_in, align 16
  %call406 = call <4 x float> @_Z9normalizeDv4_f(<4 x float> %608) nounwind readnone
  store <4 x float> %call406, <4 x float>* %a4_out, align 16
  %609 = load float* %a_in, align 4
  %call407 = call float @_Z14fast_normalizef(float %609) nounwind readnone
  store float %call407, float* %a_out, align 4
  %610 = load <4 x float>* %a4_in, align 16
  %call408 = call <4 x float> @_Z14fast_normalizeDv4_f(<4 x float> %610) nounwind readnone
  store <4 x float> %call408, <4 x float>* %a4_out, align 16
  %611 = load <4 x float>* %a4_in, align 16
  %612 = load <4 x float>* %b4_in, align 16
  %call409 = call <4 x float> @_Z5crossDv4_fS_(<4 x float> %611, <4 x float> %612) nounwind readnone
  store <4 x float> %call409, <4 x float>* %a4_out, align 16
  %613 = load float* %a_in, align 4
  %call410 = call float @_Z6lengthf(float %613) nounwind readnone
  store float %call410, float* %a_out, align 4
  %614 = load <2 x float>* %a2_in, align 8
  %call411 = call float @_Z6lengthDv2_f(<2 x float> %614) nounwind readnone
  store float %call411, float* %a_out, align 4
  %615 = load <4 x float>* %a4_in, align 16
  %call412 = call float @_Z6lengthDv4_f(<4 x float> %615) nounwind readnone
  store float %call412, float* %a_out, align 4
  %616 = load float* %a_in, align 4
  %call413 = call float @_Z11fast_lengthf(float %616) nounwind readnone
  store float %call413, float* %a_out, align 4
  %617 = load <2 x float>* %a2_in, align 8
  %call414 = call float @_Z11fast_lengthDv2_f(<2 x float> %617) nounwind readnone
  store float %call414, float* %a_out, align 4
  %618 = load <4 x float>* %a4_in, align 16
  %call415 = call float @_Z11fast_lengthDv4_f(<4 x float> %618) nounwind readnone
  store float %call415, float* %a_out, align 4
  %619 = load float* %a_in, align 4
  %620 = load float* %b_in, align 4
  %call416 = call float @_Z8distanceff(float %619, float %620) nounwind readnone
  store float %call416, float* %a_out, align 4
  %621 = load <2 x float>* %a2_in, align 8
  %622 = load <2 x float>* %b2_in, align 8
  %call417 = call float @_Z8distanceDv2_fS_(<2 x float> %621, <2 x float> %622) nounwind readnone
  store float %call417, float* %a_out, align 4
  %623 = load <4 x float>* %a4_in, align 16
  %624 = load <4 x float>* %b4_in, align 16
  %call418 = call float @_Z8distanceDv4_fS_(<4 x float> %623, <4 x float> %624) nounwind readnone
  store float %call418, float* %a_out, align 4
  %625 = load float* %a_in, align 4
  %626 = load float* %b_in, align 4
  %call419 = call float @_Z13fast_distanceff(float %625, float %626) nounwind readnone
  store float %call419, float* %a_out, align 4
  %627 = load <2 x float>* %a2_in, align 8
  %628 = load <2 x float>* %b2_in, align 8
  %call420 = call float @_Z13fast_distanceDv2_fS_(<2 x float> %627, <2 x float> %628) nounwind readnone
  store float %call420, float* %a_out, align 4
  %629 = load <4 x float>* %a4_in, align 16
  %630 = load <4 x float>* %b4_in, align 16
  %call421 = call float @_Z13fast_distanceDv4_fS_(<4 x float> %629, <4 x float> %630) nounwind readnone
  store float %call421, float* %a_out, align 4
  %631 = load i32* %tid, align 4
  %call422 = call float @_Z13convert_floati(i32 %631) nounwind readnone
  store float %call422, float* %a_out, align 4
  %632 = load i32* %tid, align 4
  %633 = insertelement <4 x i32> undef, i32 %632, i32 0
  %splat423 = shufflevector <4 x i32> %633, <4 x i32> %633, <4 x i32> zeroinitializer
  %call424 = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %splat423) nounwind readnone
  store <4 x float> %call424, <4 x float>* %a4_out, align 16
  %634 = load i32* %tid, align 4
  %635 = insertelement <8 x i32> undef, i32 %634, i32 0
  %splat425 = shufflevector <8 x i32> %635, <8 x i32> %635, <8 x i32> zeroinitializer
  %call426 = call <8 x float> @_Z14convert_float8Dv8_i(<8 x i32> %splat425) nounwind readnone
  store <8 x float> %call426, <8 x float>* %a8_out, align 32
  %636 = load i32* %tid, align 4
  %637 = insertelement <16 x i32> undef, i32 %636, i32 0
  %splat427 = shufflevector <16 x i32> %637, <16 x i32> %637, <16 x i32> zeroinitializer
  %call428 = call <16 x float> @_Z15convert_float16Dv16_i(<16 x i32> %splat427) nounwind readnone
  store <16 x float> %call428, <16 x float>* %a16_out, align 64
  %638 = load i32* %tid, align 4
  %call429 = call float @_Z13convert_floatj(i32 %638) nounwind readnone
  store float %call429, float* %a_out, align 4
  %639 = load i32* %tid, align 4
  %640 = insertelement <4 x i32> undef, i32 %639, i32 0
  %splat430 = shufflevector <4 x i32> %640, <4 x i32> %640, <4 x i32> zeroinitializer
  %call431 = call <4 x float> @_Z14convert_float4Dv4_j(<4 x i32> %splat430) nounwind readnone
  store <4 x float> %call431, <4 x float>* %a4_out, align 16
  %641 = load i32* %tid, align 4
  %642 = insertelement <8 x i32> undef, i32 %641, i32 0
  %splat432 = shufflevector <8 x i32> %642, <8 x i32> %642, <8 x i32> zeroinitializer
  %call433 = call <8 x float> @_Z14convert_float8Dv8_j(<8 x i32> %splat432) nounwind readnone
  store <8 x float> %call433, <8 x float>* %a8_out, align 32
  %643 = load i32* %tid, align 4
  %644 = insertelement <16 x i32> undef, i32 %643, i32 0
  %splat434 = shufflevector <16 x i32> %644, <16 x i32> %644, <16 x i32> zeroinitializer
  %call435 = call <16 x float> @_Z15convert_float16Dv16_j(<16 x i32> %splat434) nounwind readnone
  store <16 x float> %call435, <16 x float>* %a16_out, align 64
  %645 = load float* %a_in, align 4
  %646 = load i32* %i_in, align 4
  %call436 = call float @_Z5rootnfi(float %645, i32 %646) nounwind readnone
  store float %call436, float* %a_out, align 4
  %647 = load <4 x float>* %a4_in, align 16
  %648 = load <4 x i32>* %i4_in, align 16
  %call437 = call <4 x float> @_Z5rootnDv4_fDv4_i(<4 x float> %647, <4 x i32> %648) nounwind readnone
  store <4 x float> %call437, <4 x float>* %a4_out, align 16
  %649 = load <8 x float>* %a8_in, align 32
  %650 = load <8 x i32>* %i8_in, align 32
  %call438 = call <8 x float> @_Z5rootnDv8_fDv8_i(<8 x float> %649, <8 x i32> %650) nounwind readnone
  store <8 x float> %call438, <8 x float>* %a8_out, align 32
  %651 = load <16 x float>* %a16_in, align 64
  %652 = load <16 x i32>* %i16_in, align 64
  %call439 = call <16 x float> @_Z5rootnDv16_fDv16_i(<16 x float> %651, <16 x i32> %652) nounwind readnone
  store <16 x float> %call439, <16 x float>* %a16_out, align 64
  %653 = load float* %a_in, align 4
  %654 = load i32* %i_in, align 4
  %call440 = call float @_Z5ldexpfi(float %653, i32 %654) nounwind readnone
  store float %call440, float* %a_out, align 4
  %655 = load <4 x float>* %a4_in, align 16
  %656 = load <4 x i32>* %i4_in, align 16
  %call441 = call <4 x float> @_Z5ldexpDv4_fDv4_i(<4 x float> %655, <4 x i32> %656) nounwind readnone
  store <4 x float> %call441, <4 x float>* %a4_out, align 16
  %657 = load <8 x float>* %a8_in, align 32
  %658 = load <8 x i32>* %i8_in, align 32
  %call442 = call <8 x float> @_Z5ldexpDv8_fDv8_i(<8 x float> %657, <8 x i32> %658) nounwind readnone
  store <8 x float> %call442, <8 x float>* %a8_out, align 32
  %659 = load <16 x float>* %a16_in, align 64
  %660 = load <16 x i32>* %i16_in, align 64
  %call443 = call <16 x float> @_Z5ldexpDv16_fDv16_i(<16 x float> %659, <16 x i32> %660) nounwind readnone
  store <16 x float> %call443, <16 x float>* %a16_out, align 64
  %661 = load <4 x float>* %a4_in, align 16
  %662 = load i32* %i_in, align 4
  %call444 = call <4 x float> @_Z5ldexpDv4_fi(<4 x float> %661, i32 %662) nounwind readnone
  store <4 x float> %call444, <4 x float>* %a4_out, align 16
  %663 = load <8 x float>* %a8_in, align 32
  %664 = load i32* %i_in, align 4
  %call445 = call <8 x float> @_Z5ldexpDv8_fi(<8 x float> %663, i32 %664) nounwind readnone
  store <8 x float> %call445, <8 x float>* %a8_out, align 32
  %665 = load <16 x float>* %a16_in, align 64
  %666 = load i32* %i_in, align 4
  %call446 = call <16 x float> @_Z5ldexpDv16_fi(<16 x float> %665, i32 %666) nounwind readnone
  store <16 x float> %call446, <16 x float>* %a16_out, align 64
  %667 = load float* %a_in, align 4
  %call447 = call float @_Z4modffPf(float %667, float* %b_out)
  store float %call447, float* %a_out, align 4
  %668 = load <4 x float>* %a4_in, align 16
  %call448 = call <4 x float> @_Z4modfDv4_fPS_(<4 x float> %668, <4 x float>* %b4_out)
  store <4 x float> %call448, <4 x float>* %a4_out, align 16
  %669 = load <8 x float>* %a8_in, align 32
  %call449 = call <8 x float> @_Z4modfDv8_fPS_(<8 x float> %669, <8 x float>* %b8_out)
  store <8 x float> %call449, <8 x float>* %a8_out, align 32
  %670 = load <16 x float>* %a16_in, align 64
  %call450 = call <16 x float> @_Z4modfDv16_fPS_(<16 x float> %670, <16 x float>* %b16_out)
  store <16 x float> %call450, <16 x float>* %a16_out, align 64
  %671 = load float* %a_in, align 4
  %call451 = call float @_Z5frexpfPi(float %671, i32* %i_out)
  store float %call451, float* %a_out, align 4
  %672 = load <4 x float>* %a4_in, align 16
  %call452 = call <4 x float> @_Z5frexpDv4_fPDv4_i(<4 x float> %672, <4 x i32>* %i4_out)
  store <4 x float> %call452, <4 x float>* %a4_out, align 16
  %673 = load <8 x float>* %a8_in, align 32
  %call453 = call <8 x float> @_Z5frexpDv8_fPDv8_i(<8 x float> %673, <8 x i32>* %i8_out)
  store <8 x float> %call453, <8 x float>* %a8_out, align 32
  %674 = load <16 x float>* %a16_in, align 64
  %call454 = call <16 x float> @_Z5frexpDv16_fPDv16_i(<16 x float> %674, <16 x i32>* %i16_out)
  store <16 x float> %call454, <16 x float>* %a16_out, align 64
  %675 = load float* %a_in, align 4
  %676 = load float* %b_in, align 4
  %call455 = call float @_Z6maxmagff(float %675, float %676) nounwind readnone
  store float %call455, float* %a_out, align 4
  %677 = load <4 x float>* %a4_in, align 16
  %678 = load <4 x float>* %b4_in, align 16
  %call456 = call <4 x float> @_Z6maxmagDv4_fS_(<4 x float> %677, <4 x float> %678) nounwind readnone
  store <4 x float> %call456, <4 x float>* %a4_out, align 16
  %679 = load <8 x float>* %a8_in, align 32
  %680 = load <8 x float>* %b8_in, align 32
  %call457 = call <8 x float> @_Z6maxmagDv8_fS_(<8 x float> %679, <8 x float> %680) nounwind readnone
  store <8 x float> %call457, <8 x float>* %a8_out, align 32
  %681 = load <16 x float>* %a16_in, align 64
  %682 = load <16 x float>* %b16_in, align 64
  %call458 = call <16 x float> @_Z6maxmagDv16_fS_(<16 x float> %681, <16 x float> %682) nounwind readnone
  store <16 x float> %call458, <16 x float>* %a16_out, align 64
  %683 = load float* %a_in, align 4
  %684 = load float* %b_in, align 4
  %call459 = call float @_Z6minmagff(float %683, float %684) nounwind readnone
  store float %call459, float* %a_out, align 4
  %685 = load <4 x float>* %a4_in, align 16
  %686 = load <4 x float>* %b4_in, align 16
  %call460 = call <4 x float> @_Z6minmagDv4_fS_(<4 x float> %685, <4 x float> %686) nounwind readnone
  store <4 x float> %call460, <4 x float>* %a4_out, align 16
  %687 = load <8 x float>* %a8_in, align 32
  %688 = load <8 x float>* %b8_in, align 32
  %call461 = call <8 x float> @_Z6minmagDv8_fS_(<8 x float> %687, <8 x float> %688) nounwind readnone
  store <8 x float> %call461, <8 x float>* %a8_out, align 32
  %689 = load <16 x float>* %a16_in, align 64
  %690 = load <16 x float>* %b16_in, align 64
  %call462 = call <16 x float> @_Z6minmagDv16_fS_(<16 x float> %689, <16 x float> %690) nounwind readnone
  store <16 x float> %call462, <16 x float>* %a16_out, align 64
  %691 = load float* %a_in, align 4
  %692 = load float* %b_in, align 4
  %call463 = call float @_Z8copysignff(float %691, float %692) nounwind readnone
  store float %call463, float* %a_out, align 4
  %693 = load <4 x float>* %a4_in, align 16
  %694 = load <4 x float>* %b4_in, align 16
  %call464 = call <4 x float> @_Z8copysignDv4_fS_(<4 x float> %693, <4 x float> %694) nounwind readnone
  store <4 x float> %call464, <4 x float>* %a4_out, align 16
  %695 = load <8 x float>* %a8_in, align 32
  %696 = load <8 x float>* %b8_in, align 32
  %call465 = call <8 x float> @_Z8copysignDv8_fS_(<8 x float> %695, <8 x float> %696) nounwind readnone
  store <8 x float> %call465, <8 x float>* %a8_out, align 32
  %697 = load <16 x float>* %a16_in, align 64
  %698 = load <16 x float>* %b16_in, align 64
  %call466 = call <16 x float> @_Z8copysignDv16_fS_(<16 x float> %697, <16 x float> %698) nounwind readnone
  store <16 x float> %call466, <16 x float>* %a16_out, align 64
  %699 = load float* %a_in, align 4
  %700 = load float* %b_in, align 4
  %call467 = call float @_Z9nextafterff(float %699, float %700) nounwind readnone
  store float %call467, float* %a_out, align 4
  %701 = load <4 x float>* %a4_in, align 16
  %702 = load <4 x float>* %b4_in, align 16
  %call468 = call <4 x float> @_Z9nextafterDv4_fS_(<4 x float> %701, <4 x float> %702) nounwind readnone
  store <4 x float> %call468, <4 x float>* %a4_out, align 16
  %703 = load <8 x float>* %a8_in, align 32
  %704 = load <8 x float>* %b8_in, align 32
  %call469 = call <8 x float> @_Z9nextafterDv8_fS_(<8 x float> %703, <8 x float> %704) nounwind readnone
  store <8 x float> %call469, <8 x float>* %a8_out, align 32
  %705 = load <16 x float>* %a16_in, align 64
  %706 = load <16 x float>* %b16_in, align 64
  %call470 = call <16 x float> @_Z9nextafterDv16_fS_(<16 x float> %705, <16 x float> %706) nounwind readnone
  store <16 x float> %call470, <16 x float>* %a16_out, align 64
  %707 = load float* %a_in, align 4
  %708 = load float* %b_in, align 4
  %call471 = call float @_Z4fdimff(float %707, float %708) nounwind readnone
  store float %call471, float* %a_out, align 4
  %709 = load <4 x float>* %a4_in, align 16
  %710 = load <4 x float>* %b4_in, align 16
  %call472 = call <4 x float> @_Z4fdimDv4_fS_(<4 x float> %709, <4 x float> %710) nounwind readnone
  store <4 x float> %call472, <4 x float>* %a4_out, align 16
  %711 = load <8 x float>* %a8_in, align 32
  %712 = load <8 x float>* %b8_in, align 32
  %call473 = call <8 x float> @_Z4fdimDv8_fS_(<8 x float> %711, <8 x float> %712) nounwind readnone
  store <8 x float> %call473, <8 x float>* %a8_out, align 32
  %713 = load <16 x float>* %a16_in, align 64
  %714 = load <16 x float>* %b16_in, align 64
  %call474 = call <16 x float> @_Z4fdimDv16_fS_(<16 x float> %713, <16 x float> %714) nounwind readnone
  store <16 x float> %call474, <16 x float>* %a16_out, align 64
  %715 = load float* %a_in, align 4
  %716 = load float* %b_in, align 4
  %717 = load float* %c_in, align 4
  %call475 = call float @_Z3fmafff(float %715, float %716, float %717) nounwind readnone
  store float %call475, float* %a_out, align 4
  %718 = load <4 x float>* %a4_in, align 16
  %719 = load <4 x float>* %b4_in, align 16
  %720 = load <4 x float>* %c4_in, align 16
  %call476 = call <4 x float> @_Z3fmaDv4_fS_S_(<4 x float> %718, <4 x float> %719, <4 x float> %720) nounwind readnone
  store <4 x float> %call476, <4 x float>* %a4_out, align 16
  %721 = load <8 x float>* %a8_in, align 32
  %722 = load <8 x float>* %b8_in, align 32
  %723 = load <8 x float>* %c8_in, align 32
  %call477 = call <8 x float> @_Z3fmaDv8_fS_S_(<8 x float> %721, <8 x float> %722, <8 x float> %723) nounwind readnone
  store <8 x float> %call477, <8 x float>* %a8_out, align 32
  %724 = load <16 x float>* %a16_in, align 64
  %725 = load <16 x float>* %b16_in, align 64
  %726 = load <16 x float>* %c16_in, align 64
  %call478 = call <16 x float> @_Z3fmaDv16_fS_S_(<16 x float> %724, <16 x float> %725, <16 x float> %726) nounwind readnone
  store <16 x float> %call478, <16 x float>* %a16_out, align 64
  %727 = load float* %a_in, align 4
  %728 = load float* %b_in, align 4
  %729 = load float* %c_in, align 4
  %call479 = call float @_Z3madfff(float %727, float %728, float %729) nounwind readnone
  store float %call479, float* %a_out, align 4
  %730 = load <4 x float>* %a4_in, align 16
  %731 = load <4 x float>* %b4_in, align 16
  %732 = load <4 x float>* %c4_in, align 16
  %call480 = call <4 x float> @_Z3madDv4_fS_S_(<4 x float> %730, <4 x float> %731, <4 x float> %732) nounwind readnone
  store <4 x float> %call480, <4 x float>* %a4_out, align 16
  %733 = load <8 x float>* %a8_in, align 32
  %734 = load <8 x float>* %b8_in, align 32
  %735 = load <8 x float>* %c8_in, align 32
  %call481 = call <8 x float> @_Z3madDv8_fS_S_(<8 x float> %733, <8 x float> %734, <8 x float> %735) nounwind readnone
  store <8 x float> %call481, <8 x float>* %a8_out, align 32
  %736 = load <16 x float>* %a16_in, align 64
  %737 = load <16 x float>* %b16_in, align 64
  %738 = load <16 x float>* %c16_in, align 64
  %call482 = call <16 x float> @_Z3madDv16_fS_S_(<16 x float> %736, <16 x float> %737, <16 x float> %738) nounwind readnone
  store <16 x float> %call482, <16 x float>* %a16_out, align 64
  %739 = load float* %a_in, align 4
  %call483 = call float @_Z4rintf(float %739) nounwind readnone
  store float %call483, float* %a_out, align 4
  %740 = load <4 x float>* %a4_in, align 16
  %call484 = call <4 x float> @_Z4rintDv4_f(<4 x float> %740) nounwind readnone
  store <4 x float> %call484, <4 x float>* %a4_out, align 16
  %741 = load <8 x float>* %a8_in, align 32
  %call485 = call <8 x float> @_Z4rintDv8_f(<8 x float> %741) nounwind readnone
  store <8 x float> %call485, <8 x float>* %a8_out, align 32
  %742 = load <16 x float>* %a16_in, align 64
  %call486 = call <16 x float> @_Z4rintDv16_f(<16 x float> %742) nounwind readnone
  store <16 x float> %call486, <16 x float>* %a16_out, align 64
  %743 = load float* %a_in, align 4
  %call487 = call float @_Z5roundf(float %743) nounwind readnone
  store float %call487, float* %a_out, align 4
  %744 = load <4 x float>* %a4_in, align 16
  %call488 = call <4 x float> @_Z5roundDv4_f(<4 x float> %744) nounwind readnone
  store <4 x float> %call488, <4 x float>* %a4_out, align 16
  %745 = load <8 x float>* %a8_in, align 32
  %call489 = call <8 x float> @_Z5roundDv8_f(<8 x float> %745) nounwind readnone
  store <8 x float> %call489, <8 x float>* %a8_out, align 32
  %746 = load <16 x float>* %a16_in, align 64
  %call490 = call <16 x float> @_Z5roundDv16_f(<16 x float> %746) nounwind readnone
  store <16 x float> %call490, <16 x float>* %a16_out, align 64
  %747 = load float* %a_in, align 4
  %call491 = call float @_Z5truncf(float %747) nounwind readnone
  store float %call491, float* %a_out, align 4
  %748 = load <4 x float>* %a4_in, align 16
  %call492 = call <4 x float> @_Z5truncDv4_f(<4 x float> %748) nounwind readnone
  store <4 x float> %call492, <4 x float>* %a4_out, align 16
  %749 = load <8 x float>* %a8_in, align 32
  %call493 = call <8 x float> @_Z5truncDv8_f(<8 x float> %749) nounwind readnone
  store <8 x float> %call493, <8 x float>* %a8_out, align 32
  %750 = load <16 x float>* %a16_in, align 64
  %call494 = call <16 x float> @_Z5truncDv16_f(<16 x float> %750) nounwind readnone
  store <16 x float> %call494, <16 x float>* %a16_out, align 64
  %751 = load float* %a_in, align 4
  %call495 = call float @_Z4cbrtf(float %751) nounwind readnone
  store float %call495, float* %a_out, align 4
  %752 = load <4 x float>* %a4_in, align 16
  %call496 = call <4 x float> @_Z4cbrtDv4_f(<4 x float> %752) nounwind readnone
  store <4 x float> %call496, <4 x float>* %a4_out, align 16
  %753 = load <8 x float>* %a8_in, align 32
  %call497 = call <8 x float> @_Z4cbrtDv8_f(<8 x float> %753) nounwind readnone
  store <8 x float> %call497, <8 x float>* %a8_out, align 32
  %754 = load <16 x float>* %a16_in, align 64
  %call498 = call <16 x float> @_Z4cbrtDv16_f(<16 x float> %754) nounwind readnone
  store <16 x float> %call498, <16 x float>* %a16_out, align 64
  %755 = load float* %a_in, align 4
  %756 = load float* %b_in, align 4
  %call499 = call float @_Z4powrff(float %755, float %756) nounwind readnone
  store float %call499, float* %a_out, align 4
  %757 = load <4 x float>* %a4_in, align 16
  %758 = load <4 x float>* %b4_in, align 16
  %call500 = call <4 x float> @_Z4powrDv4_fS_(<4 x float> %757, <4 x float> %758) nounwind readnone
  store <4 x float> %call500, <4 x float>* %a4_out, align 16
  %759 = load <8 x float>* %a8_in, align 32
  %760 = load <8 x float>* %b8_in, align 32
  %call501 = call <8 x float> @_Z4powrDv8_fS_(<8 x float> %759, <8 x float> %760) nounwind readnone
  store <8 x float> %call501, <8 x float>* %a8_out, align 32
  %761 = load <16 x float>* %a16_in, align 64
  %762 = load <16 x float>* %b16_in, align 64
  %call502 = call <16 x float> @_Z4powrDv16_fS_(<16 x float> %761, <16 x float> %762) nounwind readnone
  store <16 x float> %call502, <16 x float>* %a16_out, align 64
  %763 = load float* %a_in, align 4
  %764 = load float* %b_in, align 4
  %call503 = call float @_Z4fmodff(float %763, float %764) nounwind readnone
  store float %call503, float* %a_out, align 4
  %765 = load <4 x float>* %a4_in, align 16
  %766 = load <4 x float>* %b4_in, align 16
  %call504 = call <4 x float> @_Z4fmodDv4_fS_(<4 x float> %765, <4 x float> %766) nounwind readnone
  store <4 x float> %call504, <4 x float>* %a4_out, align 16
  %767 = load <8 x float>* %a8_in, align 32
  %768 = load <8 x float>* %b8_in, align 32
  %call505 = call <8 x float> @_Z4fmodDv8_fS_(<8 x float> %767, <8 x float> %768) nounwind readnone
  store <8 x float> %call505, <8 x float>* %a8_out, align 32
  %769 = load <16 x float>* %a16_in, align 64
  %770 = load <16 x float>* %b16_in, align 64
  %call506 = call <16 x float> @_Z4fmodDv16_fS_(<16 x float> %769, <16 x float> %770) nounwind readnone
  store <16 x float> %call506, <16 x float>* %a16_out, align 64
  %771 = load float* %a_in, align 4
  %772 = load float* %b_in, align 4
  %call507 = call float @_Z4fminff(float %771, float %772) nounwind readnone
  store float %call507, float* %a_out, align 4
  %773 = load <4 x float>* %a4_in, align 16
  %774 = load <4 x float>* %b4_in, align 16
  %call508 = call <4 x float> @_Z4fminDv4_fS_(<4 x float> %773, <4 x float> %774) nounwind readnone
  store <4 x float> %call508, <4 x float>* %a4_out, align 16
  %775 = load <8 x float>* %a8_in, align 32
  %776 = load <8 x float>* %b8_in, align 32
  %call509 = call <8 x float> @_Z4fminDv8_fS_(<8 x float> %775, <8 x float> %776) nounwind readnone
  store <8 x float> %call509, <8 x float>* %a8_out, align 32
  %777 = load <16 x float>* %a16_in, align 64
  %778 = load <16 x float>* %b16_in, align 64
  %call510 = call <16 x float> @_Z4fminDv16_fS_(<16 x float> %777, <16 x float> %778) nounwind readnone
  store <16 x float> %call510, <16 x float>* %a16_out, align 64
  %779 = load float* %a_in, align 4
  %780 = load float* %b_in, align 4
  %call511 = call float @_Z4fmaxff(float %779, float %780) nounwind readnone
  store float %call511, float* %a_out, align 4
  %781 = load <4 x float>* %a4_in, align 16
  %782 = load <4 x float>* %b4_in, align 16
  %call512 = call <4 x float> @_Z4fmaxDv4_fS_(<4 x float> %781, <4 x float> %782) nounwind readnone
  store <4 x float> %call512, <4 x float>* %a4_out, align 16
  %783 = load <8 x float>* %a8_in, align 32
  %784 = load <8 x float>* %b8_in, align 32
  %call513 = call <8 x float> @_Z4fmaxDv8_fS_(<8 x float> %783, <8 x float> %784) nounwind readnone
  store <8 x float> %call513, <8 x float>* %a8_out, align 32
  %785 = load <16 x float>* %a16_in, align 64
  %786 = load <16 x float>* %b16_in, align 64
  %call514 = call <16 x float> @_Z4fmaxDv16_fS_(<16 x float> %785, <16 x float> %786) nounwind readnone
  store <16 x float> %call514, <16 x float>* %a16_out, align 64
  %787 = load <4 x float>* %a4_in, align 16
  %788 = load float* %b_in, align 4
  %call515 = call <4 x float> @_Z4fminDv4_ff(<4 x float> %787, float %788) nounwind readnone
  store <4 x float> %call515, <4 x float>* %a4_out, align 16
  %789 = load <8 x float>* %a8_in, align 32
  %790 = load float* %b_in, align 4
  %call516 = call <8 x float> @_Z4fminDv8_ff(<8 x float> %789, float %790) nounwind readnone
  store <8 x float> %call516, <8 x float>* %a8_out, align 32
  %791 = load <16 x float>* %a16_in, align 64
  %792 = load float* %b_in, align 4
  %call517 = call <16 x float> @_Z4fminDv16_ff(<16 x float> %791, float %792) nounwind readnone
  store <16 x float> %call517, <16 x float>* %a16_out, align 64
  %793 = load <4 x float>* %a4_in, align 16
  %794 = load float* %b_in, align 4
  %call518 = call <4 x float> @_Z4fmaxDv4_ff(<4 x float> %793, float %794) nounwind readnone
  store <4 x float> %call518, <4 x float>* %a4_out, align 16
  %795 = load <8 x float>* %a8_in, align 32
  %796 = load float* %b_in, align 4
  %call519 = call <8 x float> @_Z4fmaxDv8_ff(<8 x float> %795, float %796) nounwind readnone
  store <8 x float> %call519, <8 x float>* %a8_out, align 32
  %797 = load <16 x float>* %a16_in, align 64
  %798 = load float* %b_in, align 4
  %call520 = call <16 x float> @_Z4fmaxDv16_ff(<16 x float> %797, float %798) nounwind readnone
  store <16 x float> %call520, <16 x float>* %a16_out, align 64
  %799 = load <4 x float>* %a4_in, align 16
  %800 = load i32* %i_in, align 4
  %801 = insertelement <4 x i32> undef, i32 %800, i32 0
  %splat521 = shufflevector <4 x i32> %801, <4 x i32> %801, <4 x i32> zeroinitializer
  %call522 = call <4 x float> @_Z4pownDv4_fDv4_i(<4 x float> %799, <4 x i32> %splat521) nounwind readnone
  store <4 x float> %call522, <4 x float>* %a4_out, align 16
  %802 = load <8 x float>* %a8_in, align 32
  %803 = load i32* %i_in, align 4
  %804 = insertelement <8 x i32> undef, i32 %803, i32 0
  %splat523 = shufflevector <8 x i32> %804, <8 x i32> %804, <8 x i32> zeroinitializer
  %call524 = call <8 x float> @_Z4pownDv8_fDv8_i(<8 x float> %802, <8 x i32> %splat523) nounwind readnone
  store <8 x float> %call524, <8 x float>* %a8_out, align 32
  %805 = load <16 x float>* %a16_in, align 64
  %806 = load i32* %i_in, align 4
  %807 = insertelement <16 x i32> undef, i32 %806, i32 0
  %splat525 = shufflevector <16 x i32> %807, <16 x i32> %807, <16 x i32> zeroinitializer
  %call526 = call <16 x float> @_Z4pownDv16_fDv16_i(<16 x float> %805, <16 x i32> %splat525) nounwind readnone
  store <16 x float> %call526, <16 x float>* %a16_out, align 64
  %808 = load float* %a_in, align 4
  %call527 = call i32 @_Z5ilogbf(float %808) nounwind readnone
  store i32 %call527, i32* %i_out, align 4
  %809 = load <4 x float>* %a4_in, align 16
  %call528 = call <4 x i32> @_Z5ilogbDv4_f(<4 x float> %809) nounwind readnone
  store <4 x i32> %call528, <4 x i32>* %i4_out, align 16
  %810 = load <8 x float>* %a8_in, align 32
  %call529 = call <8 x i32> @_Z5ilogbDv8_f(<8 x float> %810) nounwind readnone
  store <8 x i32> %call529, <8 x i32>* %i8_out, align 32
  %811 = load <16 x float>* %a16_in, align 64
  %call530 = call <16 x i32> @_Z5ilogbDv16_f(<16 x float> %811) nounwind readnone
  store <16 x i32> %call530, <16 x i32>* %i16_out, align 64
  %812 = load i32* %ui_in, align 4
  %call531 = call float @_Z3nanj(i32 %812) nounwind readnone
  store float %call531, float* %a_out, align 4
  %813 = load <4 x i32>* %ui4_in, align 16
  %call532 = call <4 x float> @_Z3nanDv4_j(<4 x i32> %813) nounwind readnone
  store <4 x float> %call532, <4 x float>* %a4_out, align 16
  %814 = load <8 x i32>* %ui8_in, align 32
  %call533 = call <8 x float> @_Z3nanDv8_j(<8 x i32> %814) nounwind readnone
  store <8 x float> %call533, <8 x float>* %a8_out, align 32
  %815 = load <16 x i32>* %ui16_in, align 64
  %call534 = call <16 x float> @_Z3nanDv16_j(<16 x i32> %815) nounwind readnone
  store <16 x float> %call534, <16 x float>* %a16_out, align 64
  %816 = load float* %a_in, align 4
  %call535 = call float @_Z5fractfPf(float %816, float* %b_out)
  store float %call535, float* %a_out, align 4
  %817 = load <4 x float>* %a4_in, align 16
  %call536 = call <4 x float> @_Z5fractDv4_fPS_(<4 x float> %817, <4 x float>* %b4_out)
  store <4 x float> %call536, <4 x float>* %a4_out, align 16
  %818 = load <8 x float>* %a8_in, align 32
  %call537 = call <8 x float> @_Z5fractDv8_fPS_(<8 x float> %818, <8 x float>* %b8_out)
  store <8 x float> %call537, <8 x float>* %a8_out, align 32
  %819 = load <16 x float>* %a16_in, align 64
  %call538 = call <16 x float> @_Z5fractDv16_fPS_(<16 x float> %819, <16 x float>* %b16_out)
  store <16 x float> %call538, <16 x float>* %a16_out, align 64
  %820 = load float* %a_in, align 4
  %call539 = call float @_Z6lgammaf(float %820) nounwind readnone
  store float %call539, float* %a_out, align 4
  %821 = load <4 x float>* %a4_in, align 16
  %call540 = call <4 x float> @_Z6lgammaDv4_f(<4 x float> %821) nounwind readnone
  store <4 x float> %call540, <4 x float>* %a4_out, align 16
  %822 = load <8 x float>* %a8_in, align 32
  %call541 = call <8 x float> @_Z6lgammaDv8_f(<8 x float> %822) nounwind readnone
  store <8 x float> %call541, <8 x float>* %a8_out, align 32
  %823 = load <16 x float>* %a16_in, align 64
  %call542 = call <16 x float> @_Z6lgammaDv16_f(<16 x float> %823) nounwind readnone
  store <16 x float> %call542, <16 x float>* %a16_out, align 64
  %824 = load float* %a_in, align 4
  %call543 = call float @_Z8lgamma_rfPi(float %824, i32* %i_out)
  store float %call543, float* %a_out, align 4
  %825 = load <4 x float>* %a4_in, align 16
  %call544 = call <4 x float> @_Z8lgamma_rDv4_fPDv4_i(<4 x float> %825, <4 x i32>* %i4_out)
  store <4 x float> %call544, <4 x float>* %a4_out, align 16
  %826 = load <8 x float>* %a8_in, align 32
  %call545 = call <8 x float> @_Z8lgamma_rDv8_fPDv8_i(<8 x float> %826, <8 x i32>* %i8_out)
  store <8 x float> %call545, <8 x float>* %a8_out, align 32
  %827 = load <16 x float>* %a16_in, align 64
  %call546 = call <16 x float> @_Z8lgamma_rDv16_fPDv16_i(<16 x float> %827, <16 x i32>* %i16_out)
  store <16 x float> %call546, <16 x float>* %a16_out, align 64
  %828 = load float* %a_in, align 4
  %829 = load float* %b_in, align 4
  %830 = load float* %c_in, align 4
  %call547 = call float @_Z9bitselectfff(float %828, float %829, float %830) nounwind readnone
  store float %call547, float* %a_out, align 4
  %831 = load <4 x float>* %a4_in, align 16
  %832 = load <4 x float>* %b4_in, align 16
  %833 = load <4 x float>* %c4_in, align 16
  %call548 = call <4 x float> @_Z9bitselectDv4_fS_S_(<4 x float> %831, <4 x float> %832, <4 x float> %833) nounwind readnone
  store <4 x float> %call548, <4 x float>* %a4_out, align 16
  %834 = load <8 x float>* %a8_in, align 32
  %835 = load <8 x float>* %b8_in, align 32
  %836 = load <8 x float>* %c8_in, align 32
  %call549 = call <8 x float> @_Z9bitselectDv8_fS_S_(<8 x float> %834, <8 x float> %835, <8 x float> %836) nounwind readnone
  store <8 x float> %call549, <8 x float>* %a8_out, align 32
  %837 = load <16 x float>* %a16_in, align 64
  %838 = load <16 x float>* %b16_in, align 64
  %839 = load <16 x float>* %c16_in, align 64
  %call550 = call <16 x float> @_Z9bitselectDv16_fS_S_(<16 x float> %837, <16 x float> %838, <16 x float> %839) nounwind readnone
  store <16 x float> %call550, <16 x float>* %a16_out, align 64
  %840 = load float* %a_in, align 4
  %841 = load float* %b_in, align 4
  %842 = load i8* %ch_in, align 1
  %call551 = call float @_Z6selectffc(float %840, float %841, i8 signext %842) nounwind readnone
  store float %call551, float* %a_out, align 4
  %843 = load <4 x float>* %a4_in, align 16
  %844 = load <4 x float>* %b4_in, align 16
  %845 = load <4 x i8>* %ch4_in, align 4
  %call552 = call <4 x float> @_Z6selectDv4_fS_Dv4_c(<4 x float> %843, <4 x float> %844, <4 x i8> %845) nounwind readnone
  store <4 x float> %call552, <4 x float>* %a4_out, align 16
  %846 = load <8 x float>* %a8_in, align 32
  %847 = load <8 x float>* %b8_in, align 32
  %848 = load <8 x i8>* %ch8_in, align 8
  %call553 = call <8 x float> @_Z6selectDv8_fS_Dv8_c(<8 x float> %846, <8 x float> %847, <8 x i8> %848) nounwind readnone
  store <8 x float> %call553, <8 x float>* %a8_out, align 32
  %849 = load <16 x float>* %a16_in, align 64
  %850 = load <16 x float>* %b16_in, align 64
  %851 = load <16 x i8>* %ch16_in, align 16
  %call554 = call <16 x float> @_Z6selectDv16_fS_Dv16_c(<16 x float> %849, <16 x float> %850, <16 x i8> %851) nounwind readnone
  store <16 x float> %call554, <16 x float>* %a16_out, align 64
  %852 = load float* %a_in, align 4
  %853 = load float* %b_in, align 4
  %854 = load i8* %uch_in, align 1
  %call555 = call float @_Z6selectffh(float %852, float %853, i8 zeroext %854) nounwind readnone
  store float %call555, float* %a_out, align 4
  %855 = load <4 x float>* %a4_in, align 16
  %856 = load <4 x float>* %b4_in, align 16
  %857 = load <4 x i8>* %uch4_in, align 4
  %call556 = call <4 x float> @_Z6selectDv4_fS_Dv4_h(<4 x float> %855, <4 x float> %856, <4 x i8> %857) nounwind readnone
  store <4 x float> %call556, <4 x float>* %a4_out, align 16
  %858 = load <8 x float>* %a8_in, align 32
  %859 = load <8 x float>* %b8_in, align 32
  %860 = load <8 x i8>* %uch8_in, align 8
  %call557 = call <8 x float> @_Z6selectDv8_fS_Dv8_h(<8 x float> %858, <8 x float> %859, <8 x i8> %860) nounwind readnone
  store <8 x float> %call557, <8 x float>* %a8_out, align 32
  %861 = load <16 x float>* %a16_in, align 64
  %862 = load <16 x float>* %b16_in, align 64
  %863 = load <16 x i8>* %uch16_in, align 16
  %call558 = call <16 x float> @_Z6selectDv16_fS_Dv16_h(<16 x float> %861, <16 x float> %862, <16 x i8> %863) nounwind readnone
  store <16 x float> %call558, <16 x float>* %a16_out, align 64
  %864 = load float* %a_in, align 4
  %865 = load float* %b_in, align 4
  %866 = load i16* %s_in, align 2
  %call559 = call float @_Z6selectffs(float %864, float %865, i16 signext %866) nounwind readnone
  store float %call559, float* %a_out, align 4
  %867 = load <4 x float>* %a4_in, align 16
  %868 = load <4 x float>* %b4_in, align 16
  %869 = load <4 x i16>* %s4_in, align 8
  %call560 = call <4 x float> @_Z6selectDv4_fS_Dv4_s(<4 x float> %867, <4 x float> %868, <4 x i16> %869) nounwind readnone
  store <4 x float> %call560, <4 x float>* %a4_out, align 16
  %870 = load <8 x float>* %a8_in, align 32
  %871 = load <8 x float>* %b8_in, align 32
  %872 = load <8 x i16>* %s8_in, align 16
  %call561 = call <8 x float> @_Z6selectDv8_fS_Dv8_s(<8 x float> %870, <8 x float> %871, <8 x i16> %872) nounwind readnone
  store <8 x float> %call561, <8 x float>* %a8_out, align 32
  %873 = load <16 x float>* %a16_in, align 64
  %874 = load <16 x float>* %b16_in, align 64
  %875 = load <16 x i16>* %s16_in, align 32
  %call562 = call <16 x float> @_Z6selectDv16_fS_Dv16_s(<16 x float> %873, <16 x float> %874, <16 x i16> %875) nounwind readnone
  store <16 x float> %call562, <16 x float>* %a16_out, align 64
  %876 = load float* %a_in, align 4
  %877 = load float* %b_in, align 4
  %878 = load i16* %us_in, align 2
  %call563 = call float @_Z6selectfft(float %876, float %877, i16 zeroext %878) nounwind readnone
  store float %call563, float* %a_out, align 4
  %879 = load <4 x float>* %a4_in, align 16
  %880 = load <4 x float>* %b4_in, align 16
  %881 = load <4 x i16>* %us4_in, align 8
  %call564 = call <4 x float> @_Z6selectDv4_fS_Dv4_t(<4 x float> %879, <4 x float> %880, <4 x i16> %881) nounwind readnone
  store <4 x float> %call564, <4 x float>* %a4_out, align 16
  %882 = load <8 x float>* %a8_in, align 32
  %883 = load <8 x float>* %b8_in, align 32
  %884 = load <8 x i16>* %us8_in, align 16
  %call565 = call <8 x float> @_Z6selectDv8_fS_Dv8_t(<8 x float> %882, <8 x float> %883, <8 x i16> %884) nounwind readnone
  store <8 x float> %call565, <8 x float>* %a8_out, align 32
  %885 = load <16 x float>* %a16_in, align 64
  %886 = load <16 x float>* %b16_in, align 64
  %887 = load <16 x i16>* %us16_in, align 32
  %call566 = call <16 x float> @_Z6selectDv16_fS_Dv16_t(<16 x float> %885, <16 x float> %886, <16 x i16> %887) nounwind readnone
  store <16 x float> %call566, <16 x float>* %a16_out, align 64
  %888 = load float* %a_in, align 4
  %889 = load float* %b_in, align 4
  %890 = load i32* %i_in, align 4
  %call567 = call float @_Z6selectffi(float %888, float %889, i32 %890) nounwind readnone
  store float %call567, float* %a_out, align 4
  %891 = load <4 x float>* %a4_in, align 16
  %892 = load <4 x float>* %b4_in, align 16
  %893 = load <4 x i32>* %i4_in, align 16
  %call568 = call <4 x float> @_Z6selectDv4_fS_Dv4_i(<4 x float> %891, <4 x float> %892, <4 x i32> %893) nounwind readnone
  store <4 x float> %call568, <4 x float>* %a4_out, align 16
  %894 = load <8 x float>* %a8_in, align 32
  %895 = load <8 x float>* %b8_in, align 32
  %896 = load <8 x i32>* %i8_in, align 32
  %call569 = call <8 x float> @_Z6selectDv8_fS_Dv8_i(<8 x float> %894, <8 x float> %895, <8 x i32> %896) nounwind readnone
  store <8 x float> %call569, <8 x float>* %a8_out, align 32
  %897 = load <16 x float>* %a16_in, align 64
  %898 = load <16 x float>* %b16_in, align 64
  %899 = load <16 x i32>* %i16_in, align 64
  %call570 = call <16 x float> @_Z6selectDv16_fS_Dv16_i(<16 x float> %897, <16 x float> %898, <16 x i32> %899) nounwind readnone
  store <16 x float> %call570, <16 x float>* %a16_out, align 64
  %900 = load float* %a_in, align 4
  %901 = load float* %b_in, align 4
  %902 = load i32* %ui_in, align 4
  %call571 = call float @_Z6selectffj(float %900, float %901, i32 %902) nounwind readnone
  store float %call571, float* %a_out, align 4
  %903 = load <4 x float>* %a4_in, align 16
  %904 = load <4 x float>* %b4_in, align 16
  %905 = load <4 x i32>* %ui4_in, align 16
  %call572 = call <4 x float> @_Z6selectDv4_fS_Dv4_j(<4 x float> %903, <4 x float> %904, <4 x i32> %905) nounwind readnone
  store <4 x float> %call572, <4 x float>* %a4_out, align 16
  %906 = load <8 x float>* %a8_in, align 32
  %907 = load <8 x float>* %b8_in, align 32
  %908 = load <8 x i32>* %ui8_in, align 32
  %call573 = call <8 x float> @_Z6selectDv8_fS_Dv8_j(<8 x float> %906, <8 x float> %907, <8 x i32> %908) nounwind readnone
  store <8 x float> %call573, <8 x float>* %a8_out, align 32
  %909 = load <16 x float>* %a16_in, align 64
  %910 = load <16 x float>* %b16_in, align 64
  %911 = load <16 x i32>* %ui16_in, align 64
  %call574 = call <16 x float> @_Z6selectDv16_fS_Dv16_j(<16 x float> %909, <16 x float> %910, <16 x i32> %911) nounwind readnone
  store <16 x float> %call574, <16 x float>* %a16_out, align 64
  %912 = load float* %a_in, align 4
  %913 = load float* %b_in, align 4
  %914 = load i64* %l_in, align 8
  %call575 = call float @_Z6selectffl(float %912, float %913, i64 %914) nounwind readnone
  store float %call575, float* %a_out, align 4
  %915 = load <4 x float>* %a4_in, align 16
  %916 = load <4 x float>* %b4_in, align 16
  %917 = load <4 x i64>* %l4_in, align 32
  %call576 = call <4 x float> @_Z6selectDv4_fS_Dv4_l(<4 x float> %915, <4 x float> %916, <4 x i64> %917) nounwind readnone
  store <4 x float> %call576, <4 x float>* %a4_out, align 16
  %918 = load <8 x float>* %a8_in, align 32
  %919 = load <8 x float>* %b8_in, align 32
  %920 = load <8 x i64>* %l8_in, align 64
  %call577 = call <8 x float> @_Z6selectDv8_fS_Dv8_l(<8 x float> %918, <8 x float> %919, <8 x i64> %920) nounwind readnone
  store <8 x float> %call577, <8 x float>* %a8_out, align 32
  %921 = load <16 x float>* %a16_in, align 64
  %922 = load <16 x float>* %b16_in, align 64
  %923 = load <16 x i64>* %l16_in, align 128
  %call578 = call <16 x float> @_Z6selectDv16_fS_Dv16_l(<16 x float> %921, <16 x float> %922, <16 x i64> %923) nounwind readnone
  store <16 x float> %call578, <16 x float>* %a16_out, align 64
  %924 = load float* %a_in, align 4
  %925 = load float* %b_in, align 4
  %926 = load i64* %ul_in, align 8
  %call579 = call float @_Z6selectffm(float %924, float %925, i64 %926) nounwind readnone
  store float %call579, float* %a_out, align 4
  %927 = load <4 x float>* %a4_in, align 16
  %928 = load <4 x float>* %b4_in, align 16
  %929 = load <4 x i64>* %ul4_in, align 32
  %call580 = call <4 x float> @_Z6selectDv4_fS_Dv4_m(<4 x float> %927, <4 x float> %928, <4 x i64> %929) nounwind readnone
  store <4 x float> %call580, <4 x float>* %a4_out, align 16
  %930 = load <8 x float>* %a8_in, align 32
  %931 = load <8 x float>* %b8_in, align 32
  %932 = load <8 x i64>* %ul8_in, align 64
  %call581 = call <8 x float> @_Z6selectDv8_fS_Dv8_m(<8 x float> %930, <8 x float> %931, <8 x i64> %932) nounwind readnone
  store <8 x float> %call581, <8 x float>* %a8_out, align 32
  %933 = load <16 x float>* %a16_in, align 64
  %934 = load <16 x float>* %b16_in, align 64
  %935 = load <16 x i64>* %ul16_in, align 128
  %call582 = call <16 x float> @_Z6selectDv16_fS_Dv16_m(<16 x float> %933, <16 x float> %934, <16 x i64> %935) nounwind readnone
  store <16 x float> %call582, <16 x float>* %a16_out, align 64
  %936 = load float* %a_in, align 4
  %937 = load float* %b_in, align 4
  %call583 = call float @_Z9remainderff(float %936, float %937) nounwind readnone
  store float %call583, float* %a_out, align 4
  %938 = load <4 x float>* %a4_in, align 16
  %939 = load <4 x float>* %b4_in, align 16
  %call584 = call <4 x float> @_Z9remainderDv4_fS_(<4 x float> %938, <4 x float> %939) nounwind readnone
  store <4 x float> %call584, <4 x float>* %a4_out, align 16
  %940 = load <8 x float>* %a8_in, align 32
  %941 = load <8 x float>* %b8_in, align 32
  %call585 = call <8 x float> @_Z9remainderDv8_fS_(<8 x float> %940, <8 x float> %941) nounwind readnone
  store <8 x float> %call585, <8 x float>* %a8_out, align 32
  %942 = load <16 x float>* %a16_in, align 64
  %943 = load <16 x float>* %b16_in, align 64
  %call586 = call <16 x float> @_Z9remainderDv16_fS_(<16 x float> %942, <16 x float> %943) nounwind readnone
  store <16 x float> %call586, <16 x float>* %a16_out, align 64
  %944 = load float* %a_in, align 4
  %945 = load float* %b_in, align 4
  %call587 = call float @_Z6remquoffPi(float %944, float %945, i32* %i_out)
  store float %call587, float* %a_out, align 4
  %946 = load <2 x float>* %a2_in, align 8
  %947 = load <2 x float>* %b2_in, align 8
  %call588 = call <2 x float> @_Z6remquoDv2_fS_PDv2_i(<2 x float> %946, <2 x float> %947, <2 x i32>* %i2_out)
  store <2 x float> %call588, <2 x float>* %a2_out, align 8
  %948 = load <3 x float>* %a3_in, align 16
  %949 = load <3 x float>* %b3_in, align 16
  %call589 = call <3 x float> @_Z6remquoDv3_fS_PDv3_i(<3 x float> %948, <3 x float> %949, <3 x i32>* %i3_out)
  store <3 x float> %call589, <3 x float>* %a3_out, align 16
  %950 = load <4 x float>* %a4_in, align 16
  %951 = load <4 x float>* %b4_in, align 16
  %call590 = call <4 x float> @_Z6remquoDv4_fS_PDv4_i(<4 x float> %950, <4 x float> %951, <4 x i32>* %i4_out)
  store <4 x float> %call590, <4 x float>* %a4_out, align 16
  %952 = load <8 x float>* %a8_in, align 32
  %953 = load <8 x float>* %b8_in, align 32
  %call591 = call <8 x float> @_Z6remquoDv8_fS_PDv8_i(<8 x float> %952, <8 x float> %953, <8 x i32>* %i8_out)
  store <8 x float> %call591, <8 x float>* %a8_out, align 32
  %954 = load <16 x float>* %a16_in, align 64
  %955 = load <16 x float>* %b16_in, align 64
  %call592 = call <16 x float> @_Z6remquoDv16_fS_PDv16_i(<16 x float> %954, <16 x float> %955, <16 x i32>* %i16_out)
  store <16 x float> %call592, <16 x float>* %a16_out, align 64
  ret void
}

declare float @_Z4acosf(float) nounwind readnone

declare <4 x float> @_Z4acosDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4acosDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4acosDv16_f(<16 x float>) nounwind readnone

declare float @_Z6acospif(float) nounwind readnone

declare <4 x float> @_Z6acospiDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z6acospiDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z6acospiDv16_f(<16 x float>) nounwind readnone

declare float @_Z4asinf(float) nounwind readnone

declare <4 x float> @_Z4asinDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4asinDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4asinDv16_f(<16 x float>) nounwind readnone

declare float @_Z6asinpif(float) nounwind readnone

declare <4 x float> @_Z6asinpiDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z6asinpiDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z6asinpiDv16_f(<16 x float>) nounwind readnone

declare float @_Z4atanf(float) nounwind readnone

declare <4 x float> @_Z4atanDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4atanDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4atanDv16_f(<16 x float>) nounwind readnone

declare float @_Z5atan2ff(float, float) nounwind readnone

declare <4 x float> @_Z5atan2Dv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z5atan2Dv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z5atan2Dv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z7atan2piff(float, float) nounwind readnone

declare <4 x float> @_Z7atan2piDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z7atan2piDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z7atan2piDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z6atanpif(float) nounwind readnone

declare <4 x float> @_Z6atanpiDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z6atanpiDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z6atanpiDv16_f(<16 x float>) nounwind readnone

declare float @_Z3cosf(float) nounwind readnone

declare <4 x float> @_Z3cosDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z3cosDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z3cosDv16_f(<16 x float>) nounwind readnone

declare float @_Z4coshf(float) nounwind readnone

declare <4 x float> @_Z4coshDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4coshDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4coshDv16_f(<16 x float>) nounwind readnone

declare float @_Z5cospif(float) nounwind readnone

declare <4 x float> @_Z5cospiDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5cospiDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5cospiDv16_f(<16 x float>) nounwind readnone

declare float @_Z3expf(float) nounwind readnone

declare <4 x float> @_Z3expDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z3expDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z3expDv16_f(<16 x float>) nounwind readnone

declare float @_Z4exp2f(float) nounwind readnone

declare <4 x float> @_Z4exp2Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4exp2Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4exp2Dv16_f(<16 x float>) nounwind readnone

declare float @_Z5exp10f(float) nounwind readnone

declare <4 x float> @_Z5exp10Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5exp10Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5exp10Dv16_f(<16 x float>) nounwind readnone

declare float @_Z5expm1f(float) nounwind readnone

declare <4 x float> @_Z5expm1Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5expm1Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5expm1Dv16_f(<16 x float>) nounwind readnone

declare float @_Z3logf(float) nounwind readnone

declare <4 x float> @_Z3logDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z3logDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z3logDv16_f(<16 x float>) nounwind readnone

declare float @_Z4log2f(float) nounwind readnone

declare <4 x float> @_Z4log2Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4log2Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4log2Dv16_f(<16 x float>) nounwind readnone

declare float @_Z5log10f(float) nounwind readnone

declare <4 x float> @_Z5log10Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5log10Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5log10Dv16_f(<16 x float>) nounwind readnone

declare float @_Z5log1pf(float) nounwind readnone

declare <4 x float> @_Z5log1pDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5log1pDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5log1pDv16_f(<16 x float>) nounwind readnone

declare float @_Z4logbf(float) nounwind readnone

declare <4 x float> @_Z4logbDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4logbDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4logbDv16_f(<16 x float>) nounwind readnone

declare float @_Z4ceilf(float) nounwind readnone

declare <4 x float> @_Z4ceilDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4ceilDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4ceilDv16_f(<16 x float>) nounwind readnone

declare float @_Z3powff(float, float) nounwind readnone

declare <4 x float> @_Z3powDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z3powDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z3powDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z5clampfff(float, float, float) nounwind readnone

declare <4 x float> @_Z5clampDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z5clampDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z5clampDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) nounwind readnone

declare <4 x float> @_Z5clampDv4_fff(<4 x float>, float, float) nounwind readnone

declare <8 x float> @_Z5clampDv8_fff(<8 x float>, float, float) nounwind readnone

declare <16 x float> @_Z5clampDv16_fff(<16 x float>, float, float) nounwind readnone

declare float @_Z4sinhf(float) nounwind readnone

declare <4 x float> @_Z4sinhDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4sinhDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4sinhDv16_f(<16 x float>) nounwind readnone

declare float @_Z3sinf(float) nounwind readnone

declare <4 x float> @_Z3sinDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z3sinDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z3sinDv16_f(<16 x float>) nounwind readnone

declare float @_Z5sinpif(float) nounwind readnone

declare <4 x float> @_Z5sinpiDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5sinpiDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5sinpiDv16_f(<16 x float>) nounwind readnone

declare float @_Z4sqrtf(float) nounwind readnone

declare <4 x float> @_Z4sqrtDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4sqrtDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4sqrtDv16_f(<16 x float>) nounwind readnone

declare float @_Z5rsqrtf(float) nounwind readnone

declare <4 x float> @_Z5rsqrtDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5rsqrtDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5rsqrtDv16_f(<16 x float>) nounwind readnone

declare float @_Z3tanf(float) nounwind readnone

declare <4 x float> @_Z3tanDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z3tanDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z3tanDv16_f(<16 x float>) nounwind readnone

declare float @_Z4tanhf(float) nounwind readnone

declare <4 x float> @_Z4tanhDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4tanhDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4tanhDv16_f(<16 x float>) nounwind readnone

declare float @_Z5tanpif(float) nounwind readnone

declare <4 x float> @_Z5tanpiDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5tanpiDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5tanpiDv16_f(<16 x float>) nounwind readnone

declare float @_Z4fabsf(float) nounwind readnone

declare <4 x float> @_Z4fabsDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4fabsDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4fabsDv16_f(<16 x float>) nounwind readnone

declare float @_Z10native_sinf(float) nounwind readnone

declare <4 x float> @_Z10native_sinDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z10native_sinDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z10native_sinDv16_f(<16 x float>) nounwind readnone

declare float @_Z10native_cosf(float) nounwind readnone

declare <4 x float> @_Z10native_cosDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z10native_cosDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z10native_cosDv16_f(<16 x float>) nounwind readnone

declare float @_Z12native_rsqrtf(float) nounwind readnone

declare <4 x float> @_Z12native_rsqrtDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z12native_rsqrtDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z12native_rsqrtDv16_f(<16 x float>) nounwind readnone

declare float @_Z10native_logf(float) nounwind readnone

declare <4 x float> @_Z10native_logDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z10native_logDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z10native_logDv16_f(<16 x float>) nounwind readnone

declare float @_Z11native_log2f(float) nounwind readnone

declare <4 x float> @_Z11native_log2Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z11native_log2Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z11native_log2Dv16_f(<16 x float>) nounwind readnone

declare float @_Z12native_log10f(float) nounwind readnone

declare <4 x float> @_Z12native_log10Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z12native_log10Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z12native_log10Dv16_f(<16 x float>) nounwind readnone

declare float @_Z10native_expf(float) nounwind readnone

declare <4 x float> @_Z10native_expDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z10native_expDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z10native_expDv16_f(<16 x float>) nounwind readnone

declare float @_Z11native_exp2f(float) nounwind readnone

declare <4 x float> @_Z11native_exp2Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z11native_exp2Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z11native_exp2Dv16_f(<16 x float>) nounwind readnone

declare float @_Z12native_exp10f(float) nounwind readnone

declare <4 x float> @_Z12native_exp10Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z12native_exp10Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z12native_exp10Dv16_f(<16 x float>) nounwind readnone

declare float @_Z13native_divideff(float, float) nounwind readnone

declare <4 x float> @_Z13native_divideDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z13native_divideDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z13native_divideDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z11native_powrff(float, float) nounwind readnone

declare <4 x float> @_Z11native_powrDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z11native_powrDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z11native_powrDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z12native_recipf(float) nounwind readnone

declare <4 x float> @_Z12native_recipDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z12native_recipDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z12native_recipDv16_f(<16 x float>) nounwind readnone

declare float @_Z11native_sqrtf(float) nounwind readnone

declare <4 x float> @_Z11native_sqrtDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z11native_sqrtDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z11native_sqrtDv16_f(<16 x float>) nounwind readnone

declare float @_Z10native_tanf(float) nounwind readnone

declare <4 x float> @_Z10native_tanDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z10native_tanDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z10native_tanDv16_f(<16 x float>) nounwind readnone

declare float @_Z8half_logf(float) nounwind readnone

declare <4 x float> @_Z8half_logDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z8half_logDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z8half_logDv16_f(<16 x float>) nounwind readnone

declare float @_Z9half_log2f(float) nounwind readnone

declare <4 x float> @_Z9half_log2Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z9half_log2Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z9half_log2Dv16_f(<16 x float>) nounwind readnone

declare float @_Z10half_log10f(float) nounwind readnone

declare <4 x float> @_Z10half_log10Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z10half_log10Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z10half_log10Dv16_f(<16 x float>) nounwind readnone

declare float @_Z8half_expf(float) nounwind readnone

declare <4 x float> @_Z8half_expDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z8half_expDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z8half_expDv16_f(<16 x float>) nounwind readnone

declare float @_Z9half_exp2f(float) nounwind readnone

declare <4 x float> @_Z9half_exp2Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z9half_exp2Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z9half_exp2Dv16_f(<16 x float>) nounwind readnone

declare float @_Z10half_exp10f(float) nounwind readnone

declare <4 x float> @_Z10half_exp10Dv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z10half_exp10Dv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z10half_exp10Dv16_f(<16 x float>) nounwind readnone

declare float @_Z8half_cosf(float) nounwind readnone

declare <4 x float> @_Z8half_cosDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z8half_cosDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z8half_cosDv16_f(<16 x float>) nounwind readnone

declare float @_Z11half_divideff(float, float) nounwind readnone

declare <4 x float> @_Z11half_divideDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z11half_divideDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z11half_divideDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z9half_powrff(float, float) nounwind readnone

declare <4 x float> @_Z9half_powrDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z9half_powrDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z9half_powrDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z10half_recipf(float) nounwind readnone

declare <4 x float> @_Z10half_recipDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z10half_recipDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z10half_recipDv16_f(<16 x float>) nounwind readnone

declare float @_Z10half_rsqrtf(float) nounwind readnone

declare <4 x float> @_Z10half_rsqrtDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z10half_rsqrtDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z10half_rsqrtDv16_f(<16 x float>) nounwind readnone

declare float @_Z8half_sinf(float) nounwind readnone

declare <4 x float> @_Z8half_sinDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z8half_sinDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z8half_sinDv16_f(<16 x float>) nounwind readnone

declare float @_Z9half_sqrtf(float) nounwind readnone

declare <4 x float> @_Z9half_sqrtDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z9half_sqrtDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z9half_sqrtDv16_f(<16 x float>) nounwind readnone

declare float @_Z8half_tanf(float) nounwind readnone

declare <4 x float> @_Z8half_tanDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z8half_tanDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z8half_tanDv16_f(<16 x float>) nounwind readnone

declare float @_Z5asinhf(float) nounwind readnone

declare <4 x float> @_Z5asinhDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5asinhDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5asinhDv16_f(<16 x float>) nounwind readnone

declare float @_Z5acoshf(float) nounwind readnone

declare <4 x float> @_Z5acoshDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5acoshDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5acoshDv16_f(<16 x float>) nounwind readnone

declare float @_Z5atanhf(float) nounwind readnone

declare <4 x float> @_Z5atanhDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5atanhDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5atanhDv16_f(<16 x float>) nounwind readnone

declare <4 x float> @_Z6vload4jPKf(i32, float*)

declare <8 x float> @_Z6vload8jPKf(i32, float*)

declare <16 x float> @_Z7vload16jPKf(i32, float*)

declare void @_Z7vstore4Dv4_fjPf(<4 x float>, i32, float*)

declare void @_Z7vstore8Dv8_fjPf(<8 x float>, i32, float*)

declare void @_Z8vstore16Dv16_fjPf(<16 x float>, i32, float*)

declare float @_Z3minff(float, float) nounwind readnone

declare <4 x float> @_Z3minDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z3minDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z3minDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare <4 x float> @_Z3minDv4_ff(<4 x float>, float) nounwind readnone

declare <8 x float> @_Z3minDv8_ff(<8 x float>, float) nounwind readnone

declare <16 x float> @_Z3minDv16_ff(<16 x float>, float) nounwind readnone

declare float @_Z3maxff(float, float) nounwind readnone

declare <4 x float> @_Z3maxDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z3maxDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z3maxDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare <4 x float> @_Z3maxDv4_ff(<4 x float>, float) nounwind readnone

declare <8 x float> @_Z3maxDv8_ff(<8 x float>, float) nounwind readnone

declare <16 x float> @_Z3maxDv16_ff(<16 x float>, float) nounwind readnone

declare float @_Z5hypotff(float, float) nounwind readnone

declare <4 x float> @_Z5hypotDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z5hypotDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z5hypotDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z4stepff(float, float) nounwind readnone

declare <4 x float> @_Z4stepDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z4stepDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z4stepDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare <4 x float> @_Z4stepfDv4_f(float, <4 x float>) nounwind readnone

declare <8 x float> @_Z4stepfDv8_f(float, <8 x float>) nounwind readnone

declare <16 x float> @_Z4stepfDv16_f(float, <16 x float>) nounwind readnone

declare float @_Z10smoothstepfff(float, float, float) nounwind readnone

declare <4 x float> @_Z10smoothstepDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z10smoothstepDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z10smoothstepDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) nounwind readnone

declare <4 x float> @_Z10smoothstepffDv4_f(float, float, <4 x float>) nounwind readnone

declare <8 x float> @_Z10smoothstepffDv8_f(float, float, <8 x float>) nounwind readnone

declare <16 x float> @_Z10smoothstepffDv16_f(float, float, <16 x float>) nounwind readnone

declare float @_Z7radiansf(float) nounwind readnone

declare <4 x float> @_Z7radiansDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z7radiansDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z7radiansDv16_f(<16 x float>) nounwind readnone

declare float @_Z7degreesf(float) nounwind readnone

declare <4 x float> @_Z7degreesDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z7degreesDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z7degreesDv16_f(<16 x float>) nounwind readnone

declare float @_Z4signf(float) nounwind readnone

declare <4 x float> @_Z4signDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4signDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4signDv16_f(<16 x float>) nounwind readnone

declare float @_Z5floorf(float) nounwind readnone

declare <4 x float> @_Z5floorDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5floorDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5floorDv16_f(<16 x float>) nounwind readnone

declare float @_Z3dotff(float, float) nounwind readnone

declare float @_Z3dotDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare float @_Z3mixfff(float, float, float) nounwind readnone

declare <4 x float> @_Z3mixDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) nounwind readnone

declare <4 x float> @_Z3mixDv4_fS_f(<4 x float>, <4 x float>, float) nounwind readnone

declare <8 x float> @_Z3mixDv8_fS_f(<8 x float>, <8 x float>, float) nounwind readnone

declare <16 x float> @_Z3mixDv16_fS_f(<16 x float>, <16 x float>, float) nounwind readnone

declare <8 x float> @_Z3mixDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z3mixDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) nounwind readnone

declare float @_Z9normalizef(float) nounwind readnone

declare <4 x float> @_Z9normalizeDv4_f(<4 x float>) nounwind readnone

declare float @_Z14fast_normalizef(float) nounwind readnone

declare <4 x float> @_Z14fast_normalizeDv4_f(<4 x float>) nounwind readnone

declare <4 x float> @_Z5crossDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare float @_Z6lengthf(float) nounwind readnone

declare float @_Z6lengthDv2_f(<2 x float>) nounwind readnone

declare float @_Z6lengthDv4_f(<4 x float>) nounwind readnone

declare float @_Z11fast_lengthf(float) nounwind readnone

declare float @_Z11fast_lengthDv2_f(<2 x float>) nounwind readnone

declare float @_Z11fast_lengthDv4_f(<4 x float>) nounwind readnone

declare float @_Z8distanceff(float, float) nounwind readnone

declare float @_Z8distanceDv2_fS_(<2 x float>, <2 x float>) nounwind readnone

declare float @_Z8distanceDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare float @_Z13fast_distanceff(float, float) nounwind readnone

declare float @_Z13fast_distanceDv2_fS_(<2 x float>, <2 x float>) nounwind readnone

declare float @_Z13fast_distanceDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare float @_Z13convert_floati(i32) nounwind readnone

declare <4 x float> @_Z14convert_float4Dv4_i(<4 x i32>) nounwind readnone

declare <8 x float> @_Z14convert_float8Dv8_i(<8 x i32>) nounwind readnone

declare <16 x float> @_Z15convert_float16Dv16_i(<16 x i32>) nounwind readnone

declare float @_Z13convert_floatj(i32) nounwind readnone

declare <4 x float> @_Z14convert_float4Dv4_j(<4 x i32>) nounwind readnone

declare <8 x float> @_Z14convert_float8Dv8_j(<8 x i32>) nounwind readnone

declare <16 x float> @_Z15convert_float16Dv16_j(<16 x i32>) nounwind readnone

declare float @_Z5rootnfi(float, i32) nounwind readnone

declare <4 x float> @_Z5rootnDv4_fDv4_i(<4 x float>, <4 x i32>) nounwind readnone

declare <8 x float> @_Z5rootnDv8_fDv8_i(<8 x float>, <8 x i32>) nounwind readnone

declare <16 x float> @_Z5rootnDv16_fDv16_i(<16 x float>, <16 x i32>) nounwind readnone

declare float @_Z5ldexpfi(float, i32) nounwind readnone

declare <4 x float> @_Z5ldexpDv4_fDv4_i(<4 x float>, <4 x i32>) nounwind readnone

declare <8 x float> @_Z5ldexpDv8_fDv8_i(<8 x float>, <8 x i32>) nounwind readnone

declare <16 x float> @_Z5ldexpDv16_fDv16_i(<16 x float>, <16 x i32>) nounwind readnone

declare <4 x float> @_Z5ldexpDv4_fi(<4 x float>, i32) nounwind readnone

declare <8 x float> @_Z5ldexpDv8_fi(<8 x float>, i32) nounwind readnone

declare <16 x float> @_Z5ldexpDv16_fi(<16 x float>, i32) nounwind readnone

declare float @_Z4modffPf(float, float*)

declare <4 x float> @_Z4modfDv4_fPS_(<4 x float>, <4 x float>*)

declare <8 x float> @_Z4modfDv8_fPS_(<8 x float>, <8 x float>*)

declare <16 x float> @_Z4modfDv16_fPS_(<16 x float>, <16 x float>*)

declare float @_Z5frexpfPi(float, i32*)

declare <4 x float> @_Z5frexpDv4_fPDv4_i(<4 x float>, <4 x i32>*)

declare <8 x float> @_Z5frexpDv8_fPDv8_i(<8 x float>, <8 x i32>*)

declare <16 x float> @_Z5frexpDv16_fPDv16_i(<16 x float>, <16 x i32>*)

declare float @_Z6maxmagff(float, float) nounwind readnone

declare <4 x float> @_Z6maxmagDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z6maxmagDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z6maxmagDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z6minmagff(float, float) nounwind readnone

declare <4 x float> @_Z6minmagDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z6minmagDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z6minmagDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z8copysignff(float, float) nounwind readnone

declare <4 x float> @_Z8copysignDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z8copysignDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z8copysignDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z9nextafterff(float, float) nounwind readnone

declare <4 x float> @_Z9nextafterDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z9nextafterDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z9nextafterDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z4fdimff(float, float) nounwind readnone

declare <4 x float> @_Z4fdimDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z4fdimDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z4fdimDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z3fmafff(float, float, float) nounwind readnone

declare <4 x float> @_Z3fmaDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z3fmaDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z3fmaDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) nounwind readnone

declare float @_Z3madfff(float, float, float) nounwind readnone

declare <4 x float> @_Z3madDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z3madDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z3madDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) nounwind readnone

declare float @_Z4rintf(float) nounwind readnone

declare <4 x float> @_Z4rintDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4rintDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4rintDv16_f(<16 x float>) nounwind readnone

declare float @_Z5roundf(float) nounwind readnone

declare <4 x float> @_Z5roundDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5roundDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5roundDv16_f(<16 x float>) nounwind readnone

declare float @_Z5truncf(float) nounwind readnone

declare <4 x float> @_Z5truncDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z5truncDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z5truncDv16_f(<16 x float>) nounwind readnone

declare float @_Z4cbrtf(float) nounwind readnone

declare <4 x float> @_Z4cbrtDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z4cbrtDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z4cbrtDv16_f(<16 x float>) nounwind readnone

declare float @_Z4powrff(float, float) nounwind readnone

declare <4 x float> @_Z4powrDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z4powrDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z4powrDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z4fmodff(float, float) nounwind readnone

declare <4 x float> @_Z4fmodDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z4fmodDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z4fmodDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z4fminff(float, float) nounwind readnone

declare <4 x float> @_Z4fminDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z4fminDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z4fminDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z4fmaxff(float, float) nounwind readnone

declare <4 x float> @_Z4fmaxDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z4fmaxDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z4fmaxDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare <4 x float> @_Z4fminDv4_ff(<4 x float>, float) nounwind readnone

declare <8 x float> @_Z4fminDv8_ff(<8 x float>, float) nounwind readnone

declare <16 x float> @_Z4fminDv16_ff(<16 x float>, float) nounwind readnone

declare <4 x float> @_Z4fmaxDv4_ff(<4 x float>, float) nounwind readnone

declare <8 x float> @_Z4fmaxDv8_ff(<8 x float>, float) nounwind readnone

declare <16 x float> @_Z4fmaxDv16_ff(<16 x float>, float) nounwind readnone

declare <4 x float> @_Z4pownDv4_fDv4_i(<4 x float>, <4 x i32>) nounwind readnone

declare <8 x float> @_Z4pownDv8_fDv8_i(<8 x float>, <8 x i32>) nounwind readnone

declare <16 x float> @_Z4pownDv16_fDv16_i(<16 x float>, <16 x i32>) nounwind readnone

declare i32 @_Z5ilogbf(float) nounwind readnone

declare <4 x i32> @_Z5ilogbDv4_f(<4 x float>) nounwind readnone

declare <8 x i32> @_Z5ilogbDv8_f(<8 x float>) nounwind readnone

declare <16 x i32> @_Z5ilogbDv16_f(<16 x float>) nounwind readnone

declare float @_Z3nanj(i32) nounwind readnone

declare <4 x float> @_Z3nanDv4_j(<4 x i32>) nounwind readnone

declare <8 x float> @_Z3nanDv8_j(<8 x i32>) nounwind readnone

declare <16 x float> @_Z3nanDv16_j(<16 x i32>) nounwind readnone

declare float @_Z5fractfPf(float, float*)

declare <4 x float> @_Z5fractDv4_fPS_(<4 x float>, <4 x float>*)

declare <8 x float> @_Z5fractDv8_fPS_(<8 x float>, <8 x float>*)

declare <16 x float> @_Z5fractDv16_fPS_(<16 x float>, <16 x float>*)

declare float @_Z6lgammaf(float) nounwind readnone

declare <4 x float> @_Z6lgammaDv4_f(<4 x float>) nounwind readnone

declare <8 x float> @_Z6lgammaDv8_f(<8 x float>) nounwind readnone

declare <16 x float> @_Z6lgammaDv16_f(<16 x float>) nounwind readnone

declare float @_Z8lgamma_rfPi(float, i32*)

declare <4 x float> @_Z8lgamma_rDv4_fPDv4_i(<4 x float>, <4 x i32>*)

declare <8 x float> @_Z8lgamma_rDv8_fPDv8_i(<8 x float>, <8 x i32>*)

declare <16 x float> @_Z8lgamma_rDv16_fPDv16_i(<16 x float>, <16 x i32>*)

declare float @_Z9bitselectfff(float, float, float) nounwind readnone

declare <4 x float> @_Z9bitselectDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z9bitselectDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z9bitselectDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) nounwind readnone

declare float @_Z6selectffc(float, float, i8 signext) nounwind readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_c(<4 x float>, <4 x float>, <4 x i8>) nounwind readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_c(<8 x float>, <8 x float>, <8 x i8>) nounwind readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_c(<16 x float>, <16 x float>, <16 x i8>) nounwind readnone

declare float @_Z6selectffh(float, float, i8 zeroext) nounwind readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_h(<4 x float>, <4 x float>, <4 x i8>) nounwind readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_h(<8 x float>, <8 x float>, <8 x i8>) nounwind readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_h(<16 x float>, <16 x float>, <16 x i8>) nounwind readnone

declare float @_Z6selectffs(float, float, i16 signext) nounwind readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_s(<4 x float>, <4 x float>, <4 x i16>) nounwind readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_s(<8 x float>, <8 x float>, <8 x i16>) nounwind readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_s(<16 x float>, <16 x float>, <16 x i16>) nounwind readnone

declare float @_Z6selectfft(float, float, i16 zeroext) nounwind readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_t(<4 x float>, <4 x float>, <4 x i16>) nounwind readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_t(<8 x float>, <8 x float>, <8 x i16>) nounwind readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_t(<16 x float>, <16 x float>, <16 x i16>) nounwind readnone

declare float @_Z6selectffi(float, float, i32) nounwind readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_i(<4 x float>, <4 x float>, <4 x i32>) nounwind readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_i(<8 x float>, <8 x float>, <8 x i32>) nounwind readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_i(<16 x float>, <16 x float>, <16 x i32>) nounwind readnone

declare float @_Z6selectffj(float, float, i32) nounwind readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_j(<4 x float>, <4 x float>, <4 x i32>) nounwind readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_j(<8 x float>, <8 x float>, <8 x i32>) nounwind readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_j(<16 x float>, <16 x float>, <16 x i32>) nounwind readnone

declare float @_Z6selectffl(float, float, i64) nounwind readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_l(<4 x float>, <4 x float>, <4 x i64>) nounwind readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_l(<8 x float>, <8 x float>, <8 x i64>) nounwind readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_l(<16 x float>, <16 x float>, <16 x i64>) nounwind readnone

declare float @_Z6selectffm(float, float, i64) nounwind readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_m(<4 x float>, <4 x float>, <4 x i64>) nounwind readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_m(<8 x float>, <8 x float>, <8 x i64>) nounwind readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_m(<16 x float>, <16 x float>, <16 x i64>) nounwind readnone

declare float @_Z9remainderff(float, float) nounwind readnone

declare <4 x float> @_Z9remainderDv4_fS_(<4 x float>, <4 x float>) nounwind readnone

declare <8 x float> @_Z9remainderDv8_fS_(<8 x float>, <8 x float>) nounwind readnone

declare <16 x float> @_Z9remainderDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare float @_Z6remquoffPi(float, float, i32*)

declare <2 x float> @_Z6remquoDv2_fS_PDv2_i(<2 x float>, <2 x float>, <2 x i32>*)

declare <3 x float> @_Z6remquoDv3_fS_PDv3_i(<3 x float>, <3 x float>, <3 x i32>*)

declare <4 x float> @_Z6remquoDv4_fS_PDv4_i(<4 x float>, <4 x float>, <4 x i32>*)

declare <8 x float> @_Z6remquoDv8_fS_PDv8_i(<8 x float>, <8 x float>, <8 x i32>*)

declare <16 x float> @_Z6remquoDv16_fS_PDv16_i(<16 x float>, <16 x float>, <16 x i32>*)

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32)* @oclbuiltin, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
