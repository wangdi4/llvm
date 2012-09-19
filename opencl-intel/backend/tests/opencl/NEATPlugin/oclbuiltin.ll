; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN_XXX: NEATChecker -r %s -a %s.neat -t 0
; TODO: add NEATCHECKER instrumentation

; ModuleID = 'oclbuiltin.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @oclbuiltin(float addrspace(1)* %input, float addrspace(1)* %output, i32 addrspace(1)* %inputInt, i32 addrspace(1)* %outputInt, i32 addrspace(1)* %inputUint, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca float addrspace(1)*, align 4
  %output.addr = alloca float addrspace(1)*, align 4
  %inputInt.addr = alloca i32 addrspace(1)*, align 4
  %outputInt.addr = alloca i32 addrspace(1)*, align 4
  %inputUint.addr = alloca i32 addrspace(1)*, align 4
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
  store i32 addrspace(1)* %inputUint, i32 addrspace(1)** %inputUint.addr, align 4
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
  %184 = load i32* %tid, align 4
  %185 = load i32 addrspace(1)** %inputUint.addr, align 4
  %arrayidx88 = getelementptr inbounds i32 addrspace(1)* %185, i32 %184
  %186 = load i32 addrspace(1)* %arrayidx88
  store i32 %186, i32* %ui_in, align 4
  %187 = load i32* %tid, align 4
  %188 = load i32 addrspace(1)** %inputUint.addr, align 4
  %arrayidx89 = getelementptr inbounds i32 addrspace(1)* %188, i32 %187
  %189 = load i32 addrspace(1)* %arrayidx89
  %190 = insertelement <2 x i32> undef, i32 %189, i32 0
  %splat90 = shufflevector <2 x i32> %190, <2 x i32> %190, <2 x i32> zeroinitializer
  store <2 x i32> %splat90, <2 x i32>* %ui2_in, align 8
  %191 = load i32* %tid, align 4
  %192 = load i32 addrspace(1)** %inputUint.addr, align 4
  %arrayidx91 = getelementptr inbounds i32 addrspace(1)* %192, i32 %191
  %193 = load i32 addrspace(1)* %arrayidx91
  %194 = insertelement <3 x i32> undef, i32 %193, i32 0
  %splat92 = shufflevector <3 x i32> %194, <3 x i32> %194, <3 x i32> zeroinitializer
  store <3 x i32> %splat92, <3 x i32>* %ui3_in, align 16
  %195 = load i32* %tid, align 4
  %196 = load i32 addrspace(1)** %inputUint.addr, align 4
  %arrayidx93 = getelementptr inbounds i32 addrspace(1)* %196, i32 %195
  %197 = load i32 addrspace(1)* %arrayidx93
  %198 = insertelement <4 x i32> undef, i32 %197, i32 0
  %splat94 = shufflevector <4 x i32> %198, <4 x i32> %198, <4 x i32> zeroinitializer
  store <4 x i32> %splat94, <4 x i32>* %ui4_in, align 16
  %199 = load i32* %tid, align 4
  %200 = load i32 addrspace(1)** %inputUint.addr, align 4
  %arrayidx95 = getelementptr inbounds i32 addrspace(1)* %200, i32 %199
  %201 = load i32 addrspace(1)* %arrayidx95
  %202 = insertelement <8 x i32> undef, i32 %201, i32 0
  %splat96 = shufflevector <8 x i32> %202, <8 x i32> %202, <8 x i32> zeroinitializer
  store <8 x i32> %splat96, <8 x i32>* %ui8_in, align 32
  %203 = load i32* %tid, align 4
  %204 = load i32 addrspace(1)** %inputUint.addr, align 4
  %arrayidx97 = getelementptr inbounds i32 addrspace(1)* %204, i32 %203
  %205 = load i32 addrspace(1)* %arrayidx97
  %206 = insertelement <16 x i32> undef, i32 %205, i32 0
  %splat98 = shufflevector <16 x i32> %206, <16 x i32> %206, <16 x i32> zeroinitializer
  store <16 x i32> %splat98, <16 x i32>* %ui16_in, align 64
  %207 = load float* %a_in, align 4
  %call = call float @_Z4acosf(float %207) nounwind readnone
  store float %call, float* %a_out, align 4
  %208 = load <4 x float>* %a4_in, align 16
  %call99 = call <4 x float> @_Z4acosDv4_f(<4 x float> %208) nounwind readnone
  store <4 x float> %call99, <4 x float>* %a4_out, align 16
  %209 = load <8 x float>* %a8_in, align 32
  %call100 = call <8 x float> @_Z4acosDv8_f(<8 x float> %209) nounwind readnone
  store <8 x float> %call100, <8 x float>* %a8_out, align 32
  %210 = load <16 x float>* %a16_in, align 64
  %call101 = call <16 x float> @_Z4acosDv16_f(<16 x float> %210) nounwind readnone
  store <16 x float> %call101, <16 x float>* %a16_out, align 64
  %211 = load float* %a_in, align 4
  %call102 = call float @_Z6acospif(float %211) nounwind readnone
  store float %call102, float* %a_out, align 4
  %212 = load <4 x float>* %a4_in, align 16
  %call103 = call <4 x float> @_Z6acospiDv4_f(<4 x float> %212) nounwind readnone
  store <4 x float> %call103, <4 x float>* %a4_out, align 16
  %213 = load <8 x float>* %a8_in, align 32
  %call104 = call <8 x float> @_Z6acospiDv8_f(<8 x float> %213) nounwind readnone
  store <8 x float> %call104, <8 x float>* %a8_out, align 32
  %214 = load <16 x float>* %a16_in, align 64
  %call105 = call <16 x float> @_Z6acospiDv16_f(<16 x float> %214) nounwind readnone
  store <16 x float> %call105, <16 x float>* %a16_out, align 64
  %215 = load float* %a_in, align 4
  %call106 = call float @_Z4asinf(float %215) nounwind readnone
  store float %call106, float* %a_out, align 4
  %216 = load <4 x float>* %a4_in, align 16
  %call107 = call <4 x float> @_Z4asinDv4_f(<4 x float> %216) nounwind readnone
  store <4 x float> %call107, <4 x float>* %a4_out, align 16
  %217 = load <8 x float>* %a8_in, align 32
  %call108 = call <8 x float> @_Z4asinDv8_f(<8 x float> %217) nounwind readnone
  store <8 x float> %call108, <8 x float>* %a8_out, align 32
  %218 = load <16 x float>* %a16_in, align 64
  %call109 = call <16 x float> @_Z4asinDv16_f(<16 x float> %218) nounwind readnone
  store <16 x float> %call109, <16 x float>* %a16_out, align 64
  %219 = load float* %a_in, align 4
  %call110 = call float @_Z6asinpif(float %219) nounwind readnone
  store float %call110, float* %a_out, align 4
  %220 = load <4 x float>* %a4_in, align 16
  %call111 = call <4 x float> @_Z6asinpiDv4_f(<4 x float> %220) nounwind readnone
  store <4 x float> %call111, <4 x float>* %a4_out, align 16
  %221 = load <8 x float>* %a8_in, align 32
  %call112 = call <8 x float> @_Z6asinpiDv8_f(<8 x float> %221) nounwind readnone
  store <8 x float> %call112, <8 x float>* %a8_out, align 32
  %222 = load <16 x float>* %a16_in, align 64
  %call113 = call <16 x float> @_Z6asinpiDv16_f(<16 x float> %222) nounwind readnone
  store <16 x float> %call113, <16 x float>* %a16_out, align 64
  %223 = load float* %a_in, align 4
  %call114 = call float @_Z4atanf(float %223) nounwind readnone
  store float %call114, float* %a_out, align 4
  %224 = load <4 x float>* %a4_in, align 16
  %call115 = call <4 x float> @_Z4atanDv4_f(<4 x float> %224) nounwind readnone
  store <4 x float> %call115, <4 x float>* %a4_out, align 16
  %225 = load <8 x float>* %a8_in, align 32
  %call116 = call <8 x float> @_Z4atanDv8_f(<8 x float> %225) nounwind readnone
  store <8 x float> %call116, <8 x float>* %a8_out, align 32
  %226 = load <16 x float>* %a16_in, align 64
  %call117 = call <16 x float> @_Z4atanDv16_f(<16 x float> %226) nounwind readnone
  store <16 x float> %call117, <16 x float>* %a16_out, align 64
  %227 = load float* %a_in, align 4
  %228 = load float* %b_in, align 4
  %call118 = call float @_Z5atan2ff(float %227, float %228) nounwind readnone
  store float %call118, float* %a_out, align 4
  %229 = load <4 x float>* %a4_in, align 16
  %230 = load <4 x float>* %b4_in, align 16
  %call119 = call <4 x float> @_Z5atan2Dv4_fS_(<4 x float> %229, <4 x float> %230) nounwind readnone
  store <4 x float> %call119, <4 x float>* %a4_out, align 16
  %231 = load <8 x float>* %a8_in, align 32
  %232 = load <8 x float>* %b8_in, align 32
  %call120 = call <8 x float> @_Z5atan2Dv8_fS_(<8 x float> %231, <8 x float> %232) nounwind readnone
  store <8 x float> %call120, <8 x float>* %a8_out, align 32
  %233 = load <16 x float>* %a16_in, align 64
  %234 = load <16 x float>* %b16_in, align 64
  %call121 = call <16 x float> @_Z5atan2Dv16_fS_(<16 x float> %233, <16 x float> %234) nounwind readnone
  store <16 x float> %call121, <16 x float>* %a16_out, align 64
  %235 = load float* %a_in, align 4
  %236 = load float* %b_in, align 4
  %call122 = call float @_Z7atan2piff(float %235, float %236) nounwind readnone
  store float %call122, float* %a_out, align 4
  %237 = load <4 x float>* %a4_in, align 16
  %238 = load <4 x float>* %b4_in, align 16
  %call123 = call <4 x float> @_Z7atan2piDv4_fS_(<4 x float> %237, <4 x float> %238) nounwind readnone
  store <4 x float> %call123, <4 x float>* %a4_out, align 16
  %239 = load <8 x float>* %a8_in, align 32
  %240 = load <8 x float>* %b8_in, align 32
  %call124 = call <8 x float> @_Z7atan2piDv8_fS_(<8 x float> %239, <8 x float> %240) nounwind readnone
  store <8 x float> %call124, <8 x float>* %a8_out, align 32
  %241 = load <16 x float>* %a16_in, align 64
  %242 = load <16 x float>* %b16_in, align 64
  %call125 = call <16 x float> @_Z7atan2piDv16_fS_(<16 x float> %241, <16 x float> %242) nounwind readnone
  store <16 x float> %call125, <16 x float>* %a16_out, align 64
  %243 = load float* %a_in, align 4
  %call126 = call float @_Z6atanpif(float %243) nounwind readnone
  store float %call126, float* %a_out, align 4
  %244 = load <4 x float>* %a4_in, align 16
  %call127 = call <4 x float> @_Z6atanpiDv4_f(<4 x float> %244) nounwind readnone
  store <4 x float> %call127, <4 x float>* %a4_out, align 16
  %245 = load <8 x float>* %a8_in, align 32
  %call128 = call <8 x float> @_Z6atanpiDv8_f(<8 x float> %245) nounwind readnone
  store <8 x float> %call128, <8 x float>* %a8_out, align 32
  %246 = load <16 x float>* %a16_in, align 64
  %call129 = call <16 x float> @_Z6atanpiDv16_f(<16 x float> %246) nounwind readnone
  store <16 x float> %call129, <16 x float>* %a16_out, align 64
  %247 = load float* %a_in, align 4
  %call130 = call float @_Z3cosf(float %247) nounwind readnone
  store float %call130, float* %a_out, align 4
  %248 = load <4 x float>* %a4_in, align 16
  %call131 = call <4 x float> @_Z3cosDv4_f(<4 x float> %248) nounwind readnone
  store <4 x float> %call131, <4 x float>* %a4_out, align 16
  %249 = load <8 x float>* %a8_in, align 32
  %call132 = call <8 x float> @_Z3cosDv8_f(<8 x float> %249) nounwind readnone
  store <8 x float> %call132, <8 x float>* %a8_out, align 32
  %250 = load <16 x float>* %a16_in, align 64
  %call133 = call <16 x float> @_Z3cosDv16_f(<16 x float> %250) nounwind readnone
  store <16 x float> %call133, <16 x float>* %a16_out, align 64
  %251 = load float* %a_in, align 4
  %call134 = call float @_Z4coshf(float %251) nounwind readnone
  store float %call134, float* %a_out, align 4
  %252 = load <4 x float>* %a4_in, align 16
  %call135 = call <4 x float> @_Z4coshDv4_f(<4 x float> %252) nounwind readnone
  store <4 x float> %call135, <4 x float>* %a4_out, align 16
  %253 = load <8 x float>* %a8_in, align 32
  %call136 = call <8 x float> @_Z4coshDv8_f(<8 x float> %253) nounwind readnone
  store <8 x float> %call136, <8 x float>* %a8_out, align 32
  %254 = load <16 x float>* %a16_in, align 64
  %call137 = call <16 x float> @_Z4coshDv16_f(<16 x float> %254) nounwind readnone
  store <16 x float> %call137, <16 x float>* %a16_out, align 64
  %255 = load float* %a_in, align 4
  %call138 = call float @_Z5cospif(float %255) nounwind readnone
  store float %call138, float* %a_out, align 4
  %256 = load <4 x float>* %a4_in, align 16
  %call139 = call <4 x float> @_Z5cospiDv4_f(<4 x float> %256) nounwind readnone
  store <4 x float> %call139, <4 x float>* %a4_out, align 16
  %257 = load <8 x float>* %a8_in, align 32
  %call140 = call <8 x float> @_Z5cospiDv8_f(<8 x float> %257) nounwind readnone
  store <8 x float> %call140, <8 x float>* %a8_out, align 32
  %258 = load <16 x float>* %a16_in, align 64
  %call141 = call <16 x float> @_Z5cospiDv16_f(<16 x float> %258) nounwind readnone
  store <16 x float> %call141, <16 x float>* %a16_out, align 64
  %259 = load float* %a_in, align 4
  %call142 = call float @_Z3expf(float %259) nounwind readnone
  store float %call142, float* %a_out, align 4
  %260 = load <4 x float>* %a4_in, align 16
  %call143 = call <4 x float> @_Z3expDv4_f(<4 x float> %260) nounwind readnone
  store <4 x float> %call143, <4 x float>* %a4_out, align 16
  %261 = load <8 x float>* %a8_in, align 32
  %call144 = call <8 x float> @_Z3expDv8_f(<8 x float> %261) nounwind readnone
  store <8 x float> %call144, <8 x float>* %a8_out, align 32
  %262 = load <16 x float>* %a16_in, align 64
  %call145 = call <16 x float> @_Z3expDv16_f(<16 x float> %262) nounwind readnone
  store <16 x float> %call145, <16 x float>* %a16_out, align 64
  %263 = load float* %a_in, align 4
  %call146 = call float @_Z4exp2f(float %263) nounwind readnone
  store float %call146, float* %a_out, align 4
  %264 = load <4 x float>* %a4_in, align 16
  %call147 = call <4 x float> @_Z4exp2Dv4_f(<4 x float> %264) nounwind readnone
  store <4 x float> %call147, <4 x float>* %a4_out, align 16
  %265 = load <8 x float>* %a8_in, align 32
  %call148 = call <8 x float> @_Z4exp2Dv8_f(<8 x float> %265) nounwind readnone
  store <8 x float> %call148, <8 x float>* %a8_out, align 32
  %266 = load <16 x float>* %a16_in, align 64
  %call149 = call <16 x float> @_Z4exp2Dv16_f(<16 x float> %266) nounwind readnone
  store <16 x float> %call149, <16 x float>* %a16_out, align 64
  %267 = load float* %a_in, align 4
  %call150 = call float @_Z5exp10f(float %267) nounwind readnone
  store float %call150, float* %a_out, align 4
  %268 = load <4 x float>* %a4_in, align 16
  %call151 = call <4 x float> @_Z5exp10Dv4_f(<4 x float> %268) nounwind readnone
  store <4 x float> %call151, <4 x float>* %a4_out, align 16
  %269 = load <8 x float>* %a8_in, align 32
  %call152 = call <8 x float> @_Z5exp10Dv8_f(<8 x float> %269) nounwind readnone
  store <8 x float> %call152, <8 x float>* %a8_out, align 32
  %270 = load <16 x float>* %a16_in, align 64
  %call153 = call <16 x float> @_Z5exp10Dv16_f(<16 x float> %270) nounwind readnone
  store <16 x float> %call153, <16 x float>* %a16_out, align 64
  %271 = load float* %a_in, align 4
  %call154 = call float @_Z5expm1f(float %271) nounwind readnone
  store float %call154, float* %a_out, align 4
  %272 = load <4 x float>* %a4_in, align 16
  %call155 = call <4 x float> @_Z5expm1Dv4_f(<4 x float> %272) nounwind readnone
  store <4 x float> %call155, <4 x float>* %a4_out, align 16
  %273 = load <8 x float>* %a8_in, align 32
  %call156 = call <8 x float> @_Z5expm1Dv8_f(<8 x float> %273) nounwind readnone
  store <8 x float> %call156, <8 x float>* %a8_out, align 32
  %274 = load <16 x float>* %a16_in, align 64
  %call157 = call <16 x float> @_Z5expm1Dv16_f(<16 x float> %274) nounwind readnone
  store <16 x float> %call157, <16 x float>* %a16_out, align 64
  %275 = load float* %a_in, align 4
  %call158 = call float @_Z3logf(float %275) nounwind readnone
  store float %call158, float* %a_out, align 4
  %276 = load <4 x float>* %a4_in, align 16
  %call159 = call <4 x float> @_Z3logDv4_f(<4 x float> %276) nounwind readnone
  store <4 x float> %call159, <4 x float>* %a4_out, align 16
  %277 = load <8 x float>* %a8_in, align 32
  %call160 = call <8 x float> @_Z3logDv8_f(<8 x float> %277) nounwind readnone
  store <8 x float> %call160, <8 x float>* %a8_out, align 32
  %278 = load <16 x float>* %a16_in, align 64
  %call161 = call <16 x float> @_Z3logDv16_f(<16 x float> %278) nounwind readnone
  store <16 x float> %call161, <16 x float>* %a16_out, align 64
  %279 = load float* %a_in, align 4
  %call162 = call float @_Z4log2f(float %279) nounwind readnone
  store float %call162, float* %a_out, align 4
  %280 = load <4 x float>* %a4_in, align 16
  %call163 = call <4 x float> @_Z4log2Dv4_f(<4 x float> %280) nounwind readnone
  store <4 x float> %call163, <4 x float>* %a4_out, align 16
  %281 = load <8 x float>* %a8_in, align 32
  %call164 = call <8 x float> @_Z4log2Dv8_f(<8 x float> %281) nounwind readnone
  store <8 x float> %call164, <8 x float>* %a8_out, align 32
  %282 = load <16 x float>* %a16_in, align 64
  %call165 = call <16 x float> @_Z4log2Dv16_f(<16 x float> %282) nounwind readnone
  store <16 x float> %call165, <16 x float>* %a16_out, align 64
  %283 = load float* %a_in, align 4
  %call166 = call float @_Z5log10f(float %283) nounwind readnone
  store float %call166, float* %a_out, align 4
  %284 = load <4 x float>* %a4_in, align 16
  %call167 = call <4 x float> @_Z5log10Dv4_f(<4 x float> %284) nounwind readnone
  store <4 x float> %call167, <4 x float>* %a4_out, align 16
  %285 = load <8 x float>* %a8_in, align 32
  %call168 = call <8 x float> @_Z5log10Dv8_f(<8 x float> %285) nounwind readnone
  store <8 x float> %call168, <8 x float>* %a8_out, align 32
  %286 = load <16 x float>* %a16_in, align 64
  %call169 = call <16 x float> @_Z5log10Dv16_f(<16 x float> %286) nounwind readnone
  store <16 x float> %call169, <16 x float>* %a16_out, align 64
  %287 = load float* %a_in, align 4
  %call170 = call float @_Z5log1pf(float %287) nounwind readnone
  store float %call170, float* %a_out, align 4
  %288 = load <4 x float>* %a4_in, align 16
  %call171 = call <4 x float> @_Z5log1pDv4_f(<4 x float> %288) nounwind readnone
  store <4 x float> %call171, <4 x float>* %a4_out, align 16
  %289 = load <8 x float>* %a8_in, align 32
  %call172 = call <8 x float> @_Z5log1pDv8_f(<8 x float> %289) nounwind readnone
  store <8 x float> %call172, <8 x float>* %a8_out, align 32
  %290 = load <16 x float>* %a16_in, align 64
  %call173 = call <16 x float> @_Z5log1pDv16_f(<16 x float> %290) nounwind readnone
  store <16 x float> %call173, <16 x float>* %a16_out, align 64
  %291 = load float* %a_in, align 4
  %call174 = call float @_Z4logbf(float %291) nounwind readnone
  store float %call174, float* %a_out, align 4
  %292 = load <4 x float>* %a4_in, align 16
  %call175 = call <4 x float> @_Z4logbDv4_f(<4 x float> %292) nounwind readnone
  store <4 x float> %call175, <4 x float>* %a4_out, align 16
  %293 = load <8 x float>* %a8_in, align 32
  %call176 = call <8 x float> @_Z4logbDv8_f(<8 x float> %293) nounwind readnone
  store <8 x float> %call176, <8 x float>* %a8_out, align 32
  %294 = load <16 x float>* %a16_in, align 64
  %call177 = call <16 x float> @_Z4logbDv16_f(<16 x float> %294) nounwind readnone
  store <16 x float> %call177, <16 x float>* %a16_out, align 64
  %295 = load float* %a_in, align 4
  %call178 = call float @_Z4ceilf(float %295) nounwind readnone
  store float %call178, float* %a_out, align 4
  %296 = load <4 x float>* %a4_in, align 16
  %call179 = call <4 x float> @_Z4ceilDv4_f(<4 x float> %296) nounwind readnone
  store <4 x float> %call179, <4 x float>* %a4_out, align 16
  %297 = load <8 x float>* %a8_in, align 32
  %call180 = call <8 x float> @_Z4ceilDv8_f(<8 x float> %297) nounwind readnone
  store <8 x float> %call180, <8 x float>* %a8_out, align 32
  %298 = load <16 x float>* %a16_in, align 64
  %call181 = call <16 x float> @_Z4ceilDv16_f(<16 x float> %298) nounwind readnone
  store <16 x float> %call181, <16 x float>* %a16_out, align 64
  %299 = load float* %a_in, align 4
  %300 = load float* %b_in, align 4
  %call182 = call float @_Z3powff(float %299, float %300) nounwind readnone
  store float %call182, float* %a_out, align 4
  %301 = load <4 x float>* %a4_in, align 16
  %302 = load <4 x float>* %b4_in, align 16
  %call183 = call <4 x float> @_Z3powDv4_fS_(<4 x float> %301, <4 x float> %302) nounwind readnone
  store <4 x float> %call183, <4 x float>* %a4_out, align 16
  %303 = load <8 x float>* %a8_in, align 32
  %304 = load <8 x float>* %b8_in, align 32
  %call184 = call <8 x float> @_Z3powDv8_fS_(<8 x float> %303, <8 x float> %304) nounwind readnone
  store <8 x float> %call184, <8 x float>* %a8_out, align 32
  %305 = load <16 x float>* %a16_in, align 64
  %306 = load <16 x float>* %b16_in, align 64
  %call185 = call <16 x float> @_Z3powDv16_fS_(<16 x float> %305, <16 x float> %306) nounwind readnone
  store <16 x float> %call185, <16 x float>* %a16_out, align 64
  %307 = load float* %a_in, align 4
  %308 = load float* %b_in, align 4
  %309 = load float* %c_in, align 4
  %call186 = call float @_Z5clampfff(float %307, float %308, float %309) nounwind readnone
  store float %call186, float* %a_out, align 4
  %310 = load <4 x float>* %a4_in, align 16
  %311 = load <4 x float>* %b4_in, align 16
  %312 = load <4 x float>* %c4_in, align 16
  %call187 = call <4 x float> @_Z5clampDv4_fS_S_(<4 x float> %310, <4 x float> %311, <4 x float> %312) nounwind readnone
  store <4 x float> %call187, <4 x float>* %a4_out, align 16
  %313 = load <8 x float>* %a8_in, align 32
  %314 = load <8 x float>* %b8_in, align 32
  %315 = load <8 x float>* %c8_in, align 32
  %call188 = call <8 x float> @_Z5clampDv8_fS_S_(<8 x float> %313, <8 x float> %314, <8 x float> %315) nounwind readnone
  store <8 x float> %call188, <8 x float>* %a8_out, align 32
  %316 = load <16 x float>* %a16_in, align 64
  %317 = load <16 x float>* %b16_in, align 64
  %318 = load <16 x float>* %c16_in, align 64
  %call189 = call <16 x float> @_Z5clampDv16_fS_S_(<16 x float> %316, <16 x float> %317, <16 x float> %318) nounwind readnone
  store <16 x float> %call189, <16 x float>* %a16_out, align 64
  %319 = load float* %a_in, align 4
  %320 = load float* %b_in, align 4
  %321 = load float* %c_in, align 4
  %call190 = call float @_Z5clampfff(float %319, float %320, float %321) nounwind readnone
  store float %call190, float* %a_out, align 4
  %322 = load <4 x float>* %a4_in, align 16
  %323 = load float* %b_in, align 4
  %324 = load float* %c_in, align 4
  %call191 = call <4 x float> @_Z5clampDv4_fff(<4 x float> %322, float %323, float %324) nounwind readnone
  store <4 x float> %call191, <4 x float>* %a4_out, align 16
  %325 = load <8 x float>* %a8_in, align 32
  %326 = load float* %b_in, align 4
  %327 = load float* %c_in, align 4
  %call192 = call <8 x float> @_Z5clampDv8_fff(<8 x float> %325, float %326, float %327) nounwind readnone
  store <8 x float> %call192, <8 x float>* %a8_out, align 32
  %328 = load <16 x float>* %a16_in, align 64
  %329 = load float* %b_in, align 4
  %330 = load float* %c_in, align 4
  %call193 = call <16 x float> @_Z5clampDv16_fff(<16 x float> %328, float %329, float %330) nounwind readnone
  store <16 x float> %call193, <16 x float>* %a16_out, align 64
  %331 = load float* %a_in, align 4
  %call194 = call float @_Z4sinhf(float %331) nounwind readnone
  store float %call194, float* %a_out, align 4
  %332 = load <4 x float>* %a4_in, align 16
  %call195 = call <4 x float> @_Z4sinhDv4_f(<4 x float> %332) nounwind readnone
  store <4 x float> %call195, <4 x float>* %a4_out, align 16
  %333 = load <8 x float>* %a8_in, align 32
  %call196 = call <8 x float> @_Z4sinhDv8_f(<8 x float> %333) nounwind readnone
  store <8 x float> %call196, <8 x float>* %a8_out, align 32
  %334 = load <16 x float>* %a16_in, align 64
  %call197 = call <16 x float> @_Z4sinhDv16_f(<16 x float> %334) nounwind readnone
  store <16 x float> %call197, <16 x float>* %a16_out, align 64
  %335 = load float* %a_in, align 4
  %call198 = call float @_Z3sinf(float %335) nounwind readnone
  store float %call198, float* %a_out, align 4
  %336 = load <4 x float>* %a4_in, align 16
  %call199 = call <4 x float> @_Z3sinDv4_f(<4 x float> %336) nounwind readnone
  store <4 x float> %call199, <4 x float>* %a4_out, align 16
  %337 = load <8 x float>* %a8_in, align 32
  %call200 = call <8 x float> @_Z3sinDv8_f(<8 x float> %337) nounwind readnone
  store <8 x float> %call200, <8 x float>* %a8_out, align 32
  %338 = load <16 x float>* %a16_in, align 64
  %call201 = call <16 x float> @_Z3sinDv16_f(<16 x float> %338) nounwind readnone
  store <16 x float> %call201, <16 x float>* %a16_out, align 64
  %339 = load float* %a_in, align 4
  %call202 = call float @_Z5sinpif(float %339) nounwind readnone
  store float %call202, float* %a_out, align 4
  %340 = load <4 x float>* %a4_in, align 16
  %call203 = call <4 x float> @_Z5sinpiDv4_f(<4 x float> %340) nounwind readnone
  store <4 x float> %call203, <4 x float>* %a4_out, align 16
  %341 = load <8 x float>* %a8_in, align 32
  %call204 = call <8 x float> @_Z5sinpiDv8_f(<8 x float> %341) nounwind readnone
  store <8 x float> %call204, <8 x float>* %a8_out, align 32
  %342 = load <16 x float>* %a16_in, align 64
  %call205 = call <16 x float> @_Z5sinpiDv16_f(<16 x float> %342) nounwind readnone
  store <16 x float> %call205, <16 x float>* %a16_out, align 64
  %343 = load float* %a_in, align 4
  %call206 = call float @_Z4sqrtf(float %343) nounwind readnone
  store float %call206, float* %a_out, align 4
  %344 = load <4 x float>* %a4_in, align 16
  %call207 = call <4 x float> @_Z4sqrtDv4_f(<4 x float> %344) nounwind readnone
  store <4 x float> %call207, <4 x float>* %a4_out, align 16
  %345 = load <8 x float>* %a8_in, align 32
  %call208 = call <8 x float> @_Z4sqrtDv8_f(<8 x float> %345) nounwind readnone
  store <8 x float> %call208, <8 x float>* %a8_out, align 32
  %346 = load <16 x float>* %a16_in, align 64
  %call209 = call <16 x float> @_Z4sqrtDv16_f(<16 x float> %346) nounwind readnone
  store <16 x float> %call209, <16 x float>* %a16_out, align 64
  %347 = load float* %a_in, align 4
  %call210 = call float @_Z5rsqrtf(float %347) nounwind readnone
  store float %call210, float* %a_out, align 4
  %348 = load <4 x float>* %a4_in, align 16
  %call211 = call <4 x float> @_Z5rsqrtDv4_f(<4 x float> %348) nounwind readnone
  store <4 x float> %call211, <4 x float>* %a4_out, align 16
  %349 = load <8 x float>* %a8_in, align 32
  %call212 = call <8 x float> @_Z5rsqrtDv8_f(<8 x float> %349) nounwind readnone
  store <8 x float> %call212, <8 x float>* %a8_out, align 32
  %350 = load <16 x float>* %a16_in, align 64
  %call213 = call <16 x float> @_Z5rsqrtDv16_f(<16 x float> %350) nounwind readnone
  store <16 x float> %call213, <16 x float>* %a16_out, align 64
  %351 = load float* %a_in, align 4
  %call214 = call float @_Z3tanf(float %351) nounwind readnone
  store float %call214, float* %a_out, align 4
  %352 = load <4 x float>* %a4_in, align 16
  %call215 = call <4 x float> @_Z3tanDv4_f(<4 x float> %352) nounwind readnone
  store <4 x float> %call215, <4 x float>* %a4_out, align 16
  %353 = load <8 x float>* %a8_in, align 32
  %call216 = call <8 x float> @_Z3tanDv8_f(<8 x float> %353) nounwind readnone
  store <8 x float> %call216, <8 x float>* %a8_out, align 32
  %354 = load <16 x float>* %a16_in, align 64
  %call217 = call <16 x float> @_Z3tanDv16_f(<16 x float> %354) nounwind readnone
  store <16 x float> %call217, <16 x float>* %a16_out, align 64
  %355 = load float* %a_in, align 4
  %call218 = call float @_Z4tanhf(float %355) nounwind readnone
  store float %call218, float* %a_out, align 4
  %356 = load <4 x float>* %a4_in, align 16
  %call219 = call <4 x float> @_Z4tanhDv4_f(<4 x float> %356) nounwind readnone
  store <4 x float> %call219, <4 x float>* %a4_out, align 16
  %357 = load <8 x float>* %a8_in, align 32
  %call220 = call <8 x float> @_Z4tanhDv8_f(<8 x float> %357) nounwind readnone
  store <8 x float> %call220, <8 x float>* %a8_out, align 32
  %358 = load <16 x float>* %a16_in, align 64
  %call221 = call <16 x float> @_Z4tanhDv16_f(<16 x float> %358) nounwind readnone
  store <16 x float> %call221, <16 x float>* %a16_out, align 64
  %359 = load float* %a_in, align 4
  %call222 = call float @_Z5tanpif(float %359) nounwind readnone
  store float %call222, float* %a_out, align 4
  %360 = load <4 x float>* %a4_in, align 16
  %call223 = call <4 x float> @_Z5tanpiDv4_f(<4 x float> %360) nounwind readnone
  store <4 x float> %call223, <4 x float>* %a4_out, align 16
  %361 = load <8 x float>* %a8_in, align 32
  %call224 = call <8 x float> @_Z5tanpiDv8_f(<8 x float> %361) nounwind readnone
  store <8 x float> %call224, <8 x float>* %a8_out, align 32
  %362 = load <16 x float>* %a16_in, align 64
  %call225 = call <16 x float> @_Z5tanpiDv16_f(<16 x float> %362) nounwind readnone
  store <16 x float> %call225, <16 x float>* %a16_out, align 64
  %363 = load float* %a_in, align 4
  %call226 = call float @_Z4fabsf(float %363) nounwind readnone
  store float %call226, float* %a_out, align 4
  %364 = load <4 x float>* %a4_in, align 16
  %call227 = call <4 x float> @_Z4fabsDv4_f(<4 x float> %364) nounwind readnone
  store <4 x float> %call227, <4 x float>* %a4_out, align 16
  %365 = load <8 x float>* %a8_in, align 32
  %call228 = call <8 x float> @_Z4fabsDv8_f(<8 x float> %365) nounwind readnone
  store <8 x float> %call228, <8 x float>* %a8_out, align 32
  %366 = load <16 x float>* %a16_in, align 64
  %call229 = call <16 x float> @_Z4fabsDv16_f(<16 x float> %366) nounwind readnone
  store <16 x float> %call229, <16 x float>* %a16_out, align 64
  %367 = load float* %a_in, align 4
  %call230 = call float @_Z10native_sinf(float %367) nounwind readnone
  store float %call230, float* %a_out, align 4
  %368 = load <4 x float>* %a4_in, align 16
  %call231 = call <4 x float> @_Z10native_sinDv4_f(<4 x float> %368) nounwind readnone
  store <4 x float> %call231, <4 x float>* %a4_out, align 16
  %369 = load <8 x float>* %a8_in, align 32
  %call232 = call <8 x float> @_Z10native_sinDv8_f(<8 x float> %369) nounwind readnone
  store <8 x float> %call232, <8 x float>* %a8_out, align 32
  %370 = load <16 x float>* %a16_in, align 64
  %call233 = call <16 x float> @_Z10native_sinDv16_f(<16 x float> %370) nounwind readnone
  store <16 x float> %call233, <16 x float>* %a16_out, align 64
  %371 = load float* %a_in, align 4
  %call234 = call float @_Z10native_cosf(float %371) nounwind readnone
  store float %call234, float* %a_out, align 4
  %372 = load <4 x float>* %a4_in, align 16
  %call235 = call <4 x float> @_Z10native_cosDv4_f(<4 x float> %372) nounwind readnone
  store <4 x float> %call235, <4 x float>* %a4_out, align 16
  %373 = load <8 x float>* %a8_in, align 32
  %call236 = call <8 x float> @_Z10native_cosDv8_f(<8 x float> %373) nounwind readnone
  store <8 x float> %call236, <8 x float>* %a8_out, align 32
  %374 = load <16 x float>* %a16_in, align 64
  %call237 = call <16 x float> @_Z10native_cosDv16_f(<16 x float> %374) nounwind readnone
  store <16 x float> %call237, <16 x float>* %a16_out, align 64
  %375 = load float* %a_in, align 4
  %call238 = call float @_Z12native_rsqrtf(float %375) nounwind readnone
  store float %call238, float* %a_out, align 4
  %376 = load <4 x float>* %a4_in, align 16
  %call239 = call <4 x float> @_Z12native_rsqrtDv4_f(<4 x float> %376) nounwind readnone
  store <4 x float> %call239, <4 x float>* %a4_out, align 16
  %377 = load <8 x float>* %a8_in, align 32
  %call240 = call <8 x float> @_Z12native_rsqrtDv8_f(<8 x float> %377) nounwind readnone
  store <8 x float> %call240, <8 x float>* %a8_out, align 32
  %378 = load <16 x float>* %a16_in, align 64
  %call241 = call <16 x float> @_Z12native_rsqrtDv16_f(<16 x float> %378) nounwind readnone
  store <16 x float> %call241, <16 x float>* %a16_out, align 64
  %379 = load float* %a_in, align 4
  %call242 = call float @_Z10native_logf(float %379) nounwind readnone
  store float %call242, float* %a_out, align 4
  %380 = load <4 x float>* %a4_in, align 16
  %call243 = call <4 x float> @_Z10native_logDv4_f(<4 x float> %380) nounwind readnone
  store <4 x float> %call243, <4 x float>* %a4_out, align 16
  %381 = load <8 x float>* %a8_in, align 32
  %call244 = call <8 x float> @_Z10native_logDv8_f(<8 x float> %381) nounwind readnone
  store <8 x float> %call244, <8 x float>* %a8_out, align 32
  %382 = load <16 x float>* %a16_in, align 64
  %call245 = call <16 x float> @_Z10native_logDv16_f(<16 x float> %382) nounwind readnone
  store <16 x float> %call245, <16 x float>* %a16_out, align 64
  %383 = load float* %a_in, align 4
  %call246 = call float @_Z11native_log2f(float %383) nounwind readnone
  store float %call246, float* %a_out, align 4
  %384 = load <4 x float>* %a4_in, align 16
  %call247 = call <4 x float> @_Z11native_log2Dv4_f(<4 x float> %384) nounwind readnone
  store <4 x float> %call247, <4 x float>* %a4_out, align 16
  %385 = load <8 x float>* %a8_in, align 32
  %call248 = call <8 x float> @_Z11native_log2Dv8_f(<8 x float> %385) nounwind readnone
  store <8 x float> %call248, <8 x float>* %a8_out, align 32
  %386 = load <16 x float>* %a16_in, align 64
  %call249 = call <16 x float> @_Z11native_log2Dv16_f(<16 x float> %386) nounwind readnone
  store <16 x float> %call249, <16 x float>* %a16_out, align 64
  %387 = load float* %a_in, align 4
  %call250 = call float @_Z12native_log10f(float %387) nounwind readnone
  store float %call250, float* %a_out, align 4
  %388 = load <4 x float>* %a4_in, align 16
  %call251 = call <4 x float> @_Z12native_log10Dv4_f(<4 x float> %388) nounwind readnone
  store <4 x float> %call251, <4 x float>* %a4_out, align 16
  %389 = load <8 x float>* %a8_in, align 32
  %call252 = call <8 x float> @_Z12native_log10Dv8_f(<8 x float> %389) nounwind readnone
  store <8 x float> %call252, <8 x float>* %a8_out, align 32
  %390 = load <16 x float>* %a16_in, align 64
  %call253 = call <16 x float> @_Z12native_log10Dv16_f(<16 x float> %390) nounwind readnone
  store <16 x float> %call253, <16 x float>* %a16_out, align 64
  %391 = load float* %a_in, align 4
  %call254 = call float @_Z10native_expf(float %391) nounwind readnone
  store float %call254, float* %a_out, align 4
  %392 = load <4 x float>* %a4_in, align 16
  %call255 = call <4 x float> @_Z10native_expDv4_f(<4 x float> %392) nounwind readnone
  store <4 x float> %call255, <4 x float>* %a4_out, align 16
  %393 = load <8 x float>* %a8_in, align 32
  %call256 = call <8 x float> @_Z10native_expDv8_f(<8 x float> %393) nounwind readnone
  store <8 x float> %call256, <8 x float>* %a8_out, align 32
  %394 = load <16 x float>* %a16_in, align 64
  %call257 = call <16 x float> @_Z10native_expDv16_f(<16 x float> %394) nounwind readnone
  store <16 x float> %call257, <16 x float>* %a16_out, align 64
  %395 = load float* %a_in, align 4
  %call258 = call float @_Z11native_exp2f(float %395) nounwind readnone
  store float %call258, float* %a_out, align 4
  %396 = load <4 x float>* %a4_in, align 16
  %call259 = call <4 x float> @_Z11native_exp2Dv4_f(<4 x float> %396) nounwind readnone
  store <4 x float> %call259, <4 x float>* %a4_out, align 16
  %397 = load <8 x float>* %a8_in, align 32
  %call260 = call <8 x float> @_Z11native_exp2Dv8_f(<8 x float> %397) nounwind readnone
  store <8 x float> %call260, <8 x float>* %a8_out, align 32
  %398 = load <16 x float>* %a16_in, align 64
  %call261 = call <16 x float> @_Z11native_exp2Dv16_f(<16 x float> %398) nounwind readnone
  store <16 x float> %call261, <16 x float>* %a16_out, align 64
  %399 = load float* %a_in, align 4
  %call262 = call float @_Z12native_exp10f(float %399) nounwind readnone
  store float %call262, float* %a_out, align 4
  %400 = load <4 x float>* %a4_in, align 16
  %call263 = call <4 x float> @_Z12native_exp10Dv4_f(<4 x float> %400) nounwind readnone
  store <4 x float> %call263, <4 x float>* %a4_out, align 16
  %401 = load <8 x float>* %a8_in, align 32
  %call264 = call <8 x float> @_Z12native_exp10Dv8_f(<8 x float> %401) nounwind readnone
  store <8 x float> %call264, <8 x float>* %a8_out, align 32
  %402 = load <16 x float>* %a16_in, align 64
  %call265 = call <16 x float> @_Z12native_exp10Dv16_f(<16 x float> %402) nounwind readnone
  store <16 x float> %call265, <16 x float>* %a16_out, align 64
  %403 = load float* %a_in, align 4
  %404 = load float* %b_in, align 4
  %call266 = call float @_Z13native_divideff(float %403, float %404) nounwind readnone
  store float %call266, float* %a_out, align 4
  %405 = load <4 x float>* %a4_in, align 16
  %406 = load <4 x float>* %b4_in, align 16
  %call267 = call <4 x float> @_Z13native_divideDv4_fS_(<4 x float> %405, <4 x float> %406) nounwind readnone
  store <4 x float> %call267, <4 x float>* %a4_out, align 16
  %407 = load <8 x float>* %a8_in, align 32
  %408 = load <8 x float>* %b8_in, align 32
  %call268 = call <8 x float> @_Z13native_divideDv8_fS_(<8 x float> %407, <8 x float> %408) nounwind readnone
  store <8 x float> %call268, <8 x float>* %a8_out, align 32
  %409 = load <16 x float>* %a16_in, align 64
  %410 = load <16 x float>* %b16_in, align 64
  %call269 = call <16 x float> @_Z13native_divideDv16_fS_(<16 x float> %409, <16 x float> %410) nounwind readnone
  store <16 x float> %call269, <16 x float>* %a16_out, align 64
  %411 = load float* %a_in, align 4
  %412 = load float* %b_in, align 4
  %call270 = call float @_Z11native_powrff(float %411, float %412) nounwind readnone
  store float %call270, float* %a_out, align 4
  %413 = load <4 x float>* %a4_in, align 16
  %414 = load <4 x float>* %b4_in, align 16
  %call271 = call <4 x float> @_Z11native_powrDv4_fS_(<4 x float> %413, <4 x float> %414) nounwind readnone
  store <4 x float> %call271, <4 x float>* %a4_out, align 16
  %415 = load <8 x float>* %a8_in, align 32
  %416 = load <8 x float>* %b8_in, align 32
  %call272 = call <8 x float> @_Z11native_powrDv8_fS_(<8 x float> %415, <8 x float> %416) nounwind readnone
  store <8 x float> %call272, <8 x float>* %a8_out, align 32
  %417 = load <16 x float>* %a16_in, align 64
  %418 = load <16 x float>* %b16_in, align 64
  %call273 = call <16 x float> @_Z11native_powrDv16_fS_(<16 x float> %417, <16 x float> %418) nounwind readnone
  store <16 x float> %call273, <16 x float>* %a16_out, align 64
  %419 = load float* %a_in, align 4
  %call274 = call float @_Z12native_recipf(float %419) nounwind readnone
  store float %call274, float* %a_out, align 4
  %420 = load <4 x float>* %a4_in, align 16
  %call275 = call <4 x float> @_Z12native_recipDv4_f(<4 x float> %420) nounwind readnone
  store <4 x float> %call275, <4 x float>* %a4_out, align 16
  %421 = load <8 x float>* %a8_in, align 32
  %call276 = call <8 x float> @_Z12native_recipDv8_f(<8 x float> %421) nounwind readnone
  store <8 x float> %call276, <8 x float>* %a8_out, align 32
  %422 = load <16 x float>* %a16_in, align 64
  %call277 = call <16 x float> @_Z12native_recipDv16_f(<16 x float> %422) nounwind readnone
  store <16 x float> %call277, <16 x float>* %a16_out, align 64
  %423 = load float* %a_in, align 4
  %call278 = call float @_Z11native_sqrtf(float %423) nounwind readnone
  store float %call278, float* %a_out, align 4
  %424 = load <4 x float>* %a4_in, align 16
  %call279 = call <4 x float> @_Z11native_sqrtDv4_f(<4 x float> %424) nounwind readnone
  store <4 x float> %call279, <4 x float>* %a4_out, align 16
  %425 = load <8 x float>* %a8_in, align 32
  %call280 = call <8 x float> @_Z11native_sqrtDv8_f(<8 x float> %425) nounwind readnone
  store <8 x float> %call280, <8 x float>* %a8_out, align 32
  %426 = load <16 x float>* %a16_in, align 64
  %call281 = call <16 x float> @_Z11native_sqrtDv16_f(<16 x float> %426) nounwind readnone
  store <16 x float> %call281, <16 x float>* %a16_out, align 64
  %427 = load float* %a_in, align 4
  %call282 = call float @_Z10native_tanf(float %427) nounwind readnone
  store float %call282, float* %a_out, align 4
  %428 = load <4 x float>* %a4_in, align 16
  %call283 = call <4 x float> @_Z10native_tanDv4_f(<4 x float> %428) nounwind readnone
  store <4 x float> %call283, <4 x float>* %a4_out, align 16
  %429 = load <8 x float>* %a8_in, align 32
  %call284 = call <8 x float> @_Z10native_tanDv8_f(<8 x float> %429) nounwind readnone
  store <8 x float> %call284, <8 x float>* %a8_out, align 32
  %430 = load <16 x float>* %a16_in, align 64
  %call285 = call <16 x float> @_Z10native_tanDv16_f(<16 x float> %430) nounwind readnone
  store <16 x float> %call285, <16 x float>* %a16_out, align 64
  %431 = load float* %a_in, align 4
  %call286 = call float @_Z8half_logf(float %431) nounwind readnone
  store float %call286, float* %a_out, align 4
  %432 = load <4 x float>* %a4_in, align 16
  %call287 = call <4 x float> @_Z8half_logDv4_f(<4 x float> %432) nounwind readnone
  store <4 x float> %call287, <4 x float>* %a4_out, align 16
  %433 = load <8 x float>* %a8_in, align 32
  %call288 = call <8 x float> @_Z8half_logDv8_f(<8 x float> %433) nounwind readnone
  store <8 x float> %call288, <8 x float>* %a8_out, align 32
  %434 = load <16 x float>* %a16_in, align 64
  %call289 = call <16 x float> @_Z8half_logDv16_f(<16 x float> %434) nounwind readnone
  store <16 x float> %call289, <16 x float>* %a16_out, align 64
  %435 = load float* %a_in, align 4
  %call290 = call float @_Z9half_log2f(float %435) nounwind readnone
  store float %call290, float* %a_out, align 4
  %436 = load <4 x float>* %a4_in, align 16
  %call291 = call <4 x float> @_Z9half_log2Dv4_f(<4 x float> %436) nounwind readnone
  store <4 x float> %call291, <4 x float>* %a4_out, align 16
  %437 = load <8 x float>* %a8_in, align 32
  %call292 = call <8 x float> @_Z9half_log2Dv8_f(<8 x float> %437) nounwind readnone
  store <8 x float> %call292, <8 x float>* %a8_out, align 32
  %438 = load <16 x float>* %a16_in, align 64
  %call293 = call <16 x float> @_Z9half_log2Dv16_f(<16 x float> %438) nounwind readnone
  store <16 x float> %call293, <16 x float>* %a16_out, align 64
  %439 = load float* %a_in, align 4
  %call294 = call float @_Z10half_log10f(float %439) nounwind readnone
  store float %call294, float* %a_out, align 4
  %440 = load <4 x float>* %a4_in, align 16
  %call295 = call <4 x float> @_Z10half_log10Dv4_f(<4 x float> %440) nounwind readnone
  store <4 x float> %call295, <4 x float>* %a4_out, align 16
  %441 = load <8 x float>* %a8_in, align 32
  %call296 = call <8 x float> @_Z10half_log10Dv8_f(<8 x float> %441) nounwind readnone
  store <8 x float> %call296, <8 x float>* %a8_out, align 32
  %442 = load <16 x float>* %a16_in, align 64
  %call297 = call <16 x float> @_Z10half_log10Dv16_f(<16 x float> %442) nounwind readnone
  store <16 x float> %call297, <16 x float>* %a16_out, align 64
  %443 = load float* %a_in, align 4
  %call298 = call float @_Z8half_expf(float %443) nounwind readnone
  store float %call298, float* %a_out, align 4
  %444 = load <4 x float>* %a4_in, align 16
  %call299 = call <4 x float> @_Z8half_expDv4_f(<4 x float> %444) nounwind readnone
  store <4 x float> %call299, <4 x float>* %a4_out, align 16
  %445 = load <8 x float>* %a8_in, align 32
  %call300 = call <8 x float> @_Z8half_expDv8_f(<8 x float> %445) nounwind readnone
  store <8 x float> %call300, <8 x float>* %a8_out, align 32
  %446 = load <16 x float>* %a16_in, align 64
  %call301 = call <16 x float> @_Z8half_expDv16_f(<16 x float> %446) nounwind readnone
  store <16 x float> %call301, <16 x float>* %a16_out, align 64
  %447 = load float* %a_in, align 4
  %call302 = call float @_Z9half_exp2f(float %447) nounwind readnone
  store float %call302, float* %a_out, align 4
  %448 = load <4 x float>* %a4_in, align 16
  %call303 = call <4 x float> @_Z9half_exp2Dv4_f(<4 x float> %448) nounwind readnone
  store <4 x float> %call303, <4 x float>* %a4_out, align 16
  %449 = load <8 x float>* %a8_in, align 32
  %call304 = call <8 x float> @_Z9half_exp2Dv8_f(<8 x float> %449) nounwind readnone
  store <8 x float> %call304, <8 x float>* %a8_out, align 32
  %450 = load <16 x float>* %a16_in, align 64
  %call305 = call <16 x float> @_Z9half_exp2Dv16_f(<16 x float> %450) nounwind readnone
  store <16 x float> %call305, <16 x float>* %a16_out, align 64
  %451 = load float* %a_in, align 4
  %call306 = call float @_Z10half_exp10f(float %451) nounwind readnone
  store float %call306, float* %a_out, align 4
  %452 = load <4 x float>* %a4_in, align 16
  %call307 = call <4 x float> @_Z10half_exp10Dv4_f(<4 x float> %452) nounwind readnone
  store <4 x float> %call307, <4 x float>* %a4_out, align 16
  %453 = load <8 x float>* %a8_in, align 32
  %call308 = call <8 x float> @_Z10half_exp10Dv8_f(<8 x float> %453) nounwind readnone
  store <8 x float> %call308, <8 x float>* %a8_out, align 32
  %454 = load <16 x float>* %a16_in, align 64
  %call309 = call <16 x float> @_Z10half_exp10Dv16_f(<16 x float> %454) nounwind readnone
  store <16 x float> %call309, <16 x float>* %a16_out, align 64
  %455 = load float* %a_in, align 4
  %call310 = call float @_Z8half_cosf(float %455) nounwind readnone
  store float %call310, float* %a_out, align 4
  %456 = load <4 x float>* %a4_in, align 16
  %call311 = call <4 x float> @_Z8half_cosDv4_f(<4 x float> %456) nounwind readnone
  store <4 x float> %call311, <4 x float>* %a4_out, align 16
  %457 = load <8 x float>* %a8_in, align 32
  %call312 = call <8 x float> @_Z8half_cosDv8_f(<8 x float> %457) nounwind readnone
  store <8 x float> %call312, <8 x float>* %a8_out, align 32
  %458 = load <16 x float>* %a16_in, align 64
  %call313 = call <16 x float> @_Z8half_cosDv16_f(<16 x float> %458) nounwind readnone
  store <16 x float> %call313, <16 x float>* %a16_out, align 64
  %459 = load float* %a_in, align 4
  %460 = load float* %b_in, align 4
  %call314 = call float @_Z11half_divideff(float %459, float %460) nounwind readnone
  store float %call314, float* %a_out, align 4
  %461 = load <4 x float>* %a4_in, align 16
  %462 = load <4 x float>* %b4_in, align 16
  %call315 = call <4 x float> @_Z11half_divideDv4_fS_(<4 x float> %461, <4 x float> %462) nounwind readnone
  store <4 x float> %call315, <4 x float>* %a4_out, align 16
  %463 = load <8 x float>* %a8_in, align 32
  %464 = load <8 x float>* %b8_in, align 32
  %call316 = call <8 x float> @_Z11half_divideDv8_fS_(<8 x float> %463, <8 x float> %464) nounwind readnone
  store <8 x float> %call316, <8 x float>* %a8_out, align 32
  %465 = load <16 x float>* %a16_in, align 64
  %466 = load <16 x float>* %b16_in, align 64
  %call317 = call <16 x float> @_Z11half_divideDv16_fS_(<16 x float> %465, <16 x float> %466) nounwind readnone
  store <16 x float> %call317, <16 x float>* %a16_out, align 64
  %467 = load float* %a_in, align 4
  %468 = load float* %b_in, align 4
  %call318 = call float @_Z9half_powrff(float %467, float %468) nounwind readnone
  store float %call318, float* %a_out, align 4
  %469 = load <4 x float>* %a4_in, align 16
  %470 = load <4 x float>* %b4_in, align 16
  %call319 = call <4 x float> @_Z9half_powrDv4_fS_(<4 x float> %469, <4 x float> %470) nounwind readnone
  store <4 x float> %call319, <4 x float>* %a4_out, align 16
  %471 = load <8 x float>* %a8_in, align 32
  %472 = load <8 x float>* %b8_in, align 32
  %call320 = call <8 x float> @_Z9half_powrDv8_fS_(<8 x float> %471, <8 x float> %472) nounwind readnone
  store <8 x float> %call320, <8 x float>* %a8_out, align 32
  %473 = load <16 x float>* %a16_in, align 64
  %474 = load <16 x float>* %b16_in, align 64
  %call321 = call <16 x float> @_Z9half_powrDv16_fS_(<16 x float> %473, <16 x float> %474) nounwind readnone
  store <16 x float> %call321, <16 x float>* %a16_out, align 64
  %475 = load float* %a_in, align 4
  %call322 = call float @_Z10half_recipf(float %475) nounwind readnone
  store float %call322, float* %a_out, align 4
  %476 = load <4 x float>* %a4_in, align 16
  %call323 = call <4 x float> @_Z10half_recipDv4_f(<4 x float> %476) nounwind readnone
  store <4 x float> %call323, <4 x float>* %a4_out, align 16
  %477 = load <8 x float>* %a8_in, align 32
  %call324 = call <8 x float> @_Z10half_recipDv8_f(<8 x float> %477) nounwind readnone
  store <8 x float> %call324, <8 x float>* %a8_out, align 32
  %478 = load <16 x float>* %a16_in, align 64
  %call325 = call <16 x float> @_Z10half_recipDv16_f(<16 x float> %478) nounwind readnone
  store <16 x float> %call325, <16 x float>* %a16_out, align 64
  %479 = load float* %a_in, align 4
  %call326 = call float @_Z10half_rsqrtf(float %479) nounwind readnone
  store float %call326, float* %a_out, align 4
  %480 = load <4 x float>* %a4_in, align 16
  %call327 = call <4 x float> @_Z10half_rsqrtDv4_f(<4 x float> %480) nounwind readnone
  store <4 x float> %call327, <4 x float>* %a4_out, align 16
  %481 = load <8 x float>* %a8_in, align 32
  %call328 = call <8 x float> @_Z10half_rsqrtDv8_f(<8 x float> %481) nounwind readnone
  store <8 x float> %call328, <8 x float>* %a8_out, align 32
  %482 = load <16 x float>* %a16_in, align 64
  %call329 = call <16 x float> @_Z10half_rsqrtDv16_f(<16 x float> %482) nounwind readnone
  store <16 x float> %call329, <16 x float>* %a16_out, align 64
  %483 = load float* %a_in, align 4
  %call330 = call float @_Z8half_sinf(float %483) nounwind readnone
  store float %call330, float* %a_out, align 4
  %484 = load <4 x float>* %a4_in, align 16
  %call331 = call <4 x float> @_Z8half_sinDv4_f(<4 x float> %484) nounwind readnone
  store <4 x float> %call331, <4 x float>* %a4_out, align 16
  %485 = load <8 x float>* %a8_in, align 32
  %call332 = call <8 x float> @_Z8half_sinDv8_f(<8 x float> %485) nounwind readnone
  store <8 x float> %call332, <8 x float>* %a8_out, align 32
  %486 = load <16 x float>* %a16_in, align 64
  %call333 = call <16 x float> @_Z8half_sinDv16_f(<16 x float> %486) nounwind readnone
  store <16 x float> %call333, <16 x float>* %a16_out, align 64
  %487 = load float* %a_in, align 4
  %call334 = call float @_Z9half_sqrtf(float %487) nounwind readnone
  store float %call334, float* %a_out, align 4
  %488 = load <4 x float>* %a4_in, align 16
  %call335 = call <4 x float> @_Z9half_sqrtDv4_f(<4 x float> %488) nounwind readnone
  store <4 x float> %call335, <4 x float>* %a4_out, align 16
  %489 = load <8 x float>* %a8_in, align 32
  %call336 = call <8 x float> @_Z9half_sqrtDv8_f(<8 x float> %489) nounwind readnone
  store <8 x float> %call336, <8 x float>* %a8_out, align 32
  %490 = load <16 x float>* %a16_in, align 64
  %call337 = call <16 x float> @_Z9half_sqrtDv16_f(<16 x float> %490) nounwind readnone
  store <16 x float> %call337, <16 x float>* %a16_out, align 64
  %491 = load float* %a_in, align 4
  %call338 = call float @_Z8half_tanf(float %491) nounwind readnone
  store float %call338, float* %a_out, align 4
  %492 = load <4 x float>* %a4_in, align 16
  %call339 = call <4 x float> @_Z8half_tanDv4_f(<4 x float> %492) nounwind readnone
  store <4 x float> %call339, <4 x float>* %a4_out, align 16
  %493 = load <8 x float>* %a8_in, align 32
  %call340 = call <8 x float> @_Z8half_tanDv8_f(<8 x float> %493) nounwind readnone
  store <8 x float> %call340, <8 x float>* %a8_out, align 32
  %494 = load <16 x float>* %a16_in, align 64
  %call341 = call <16 x float> @_Z8half_tanDv16_f(<16 x float> %494) nounwind readnone
  store <16 x float> %call341, <16 x float>* %a16_out, align 64
  %495 = load float* %a_in, align 4
  %call342 = call float @_Z5asinhf(float %495) nounwind readnone
  store float %call342, float* %a_out, align 4
  %496 = load <4 x float>* %a4_in, align 16
  %call343 = call <4 x float> @_Z5asinhDv4_f(<4 x float> %496) nounwind readnone
  store <4 x float> %call343, <4 x float>* %a4_out, align 16
  %497 = load <8 x float>* %a8_in, align 32
  %call344 = call <8 x float> @_Z5asinhDv8_f(<8 x float> %497) nounwind readnone
  store <8 x float> %call344, <8 x float>* %a8_out, align 32
  %498 = load <16 x float>* %a16_in, align 64
  %call345 = call <16 x float> @_Z5asinhDv16_f(<16 x float> %498) nounwind readnone
  store <16 x float> %call345, <16 x float>* %a16_out, align 64
  %499 = load float* %a_in, align 4
  %call346 = call float @_Z5acoshf(float %499) nounwind readnone
  store float %call346, float* %a_out, align 4
  %500 = load <4 x float>* %a4_in, align 16
  %call347 = call <4 x float> @_Z5acoshDv4_f(<4 x float> %500) nounwind readnone
  store <4 x float> %call347, <4 x float>* %a4_out, align 16
  %501 = load <8 x float>* %a8_in, align 32
  %call348 = call <8 x float> @_Z5acoshDv8_f(<8 x float> %501) nounwind readnone
  store <8 x float> %call348, <8 x float>* %a8_out, align 32
  %502 = load <16 x float>* %a16_in, align 64
  %call349 = call <16 x float> @_Z5acoshDv16_f(<16 x float> %502) nounwind readnone
  store <16 x float> %call349, <16 x float>* %a16_out, align 64
  %503 = load float* %a_in, align 4
  %call350 = call float @_Z5atanhf(float %503) nounwind readnone
  store float %call350, float* %a_out, align 4
  %504 = load <4 x float>* %a4_in, align 16
  %call351 = call <4 x float> @_Z5atanhDv4_f(<4 x float> %504) nounwind readnone
  store <4 x float> %call351, <4 x float>* %a4_out, align 16
  %505 = load <8 x float>* %a8_in, align 32
  %call352 = call <8 x float> @_Z5atanhDv8_f(<8 x float> %505) nounwind readnone
  store <8 x float> %call352, <8 x float>* %a8_out, align 32
  %506 = load <16 x float>* %a16_in, align 64
  %call353 = call <16 x float> @_Z5atanhDv16_f(<16 x float> %506) nounwind readnone
  store <16 x float> %call353, <16 x float>* %a16_out, align 64
  %call354 = call <4 x float> @_Z6vload4jPKf(i32 0, float* %b_in)
  store <4 x float> %call354, <4 x float>* %a4_out, align 16
  %call355 = call <8 x float> @_Z6vload8jPKf(i32 0, float* %b_in)
  store <8 x float> %call355, <8 x float>* %a8_out, align 32
  %call356 = call <16 x float> @_Z7vload16jPKf(i32 0, float* %b_in)
  store <16 x float> %call356, <16 x float>* %a16_out, align 64
  %507 = load <4 x float>* %a4_in, align 16
  %508 = bitcast <4 x float>* %a4_out to float*
  call void @_Z7vstore4Dv4_fjPf(<4 x float> %507, i32 0, float* %508)
  %509 = load <8 x float>* %a8_in, align 32
  %510 = bitcast <8 x float>* %a8_out to float*
  call void @_Z7vstore8Dv8_fjPf(<8 x float> %509, i32 0, float* %510)
  %511 = load <16 x float>* %a16_in, align 64
  %512 = bitcast <16 x float>* %a16_out to float*
  call void @_Z8vstore16Dv16_fjPf(<16 x float> %511, i32 0, float* %512)
  %513 = load float* %a_in, align 4
  %514 = load float* %b_in, align 4
  %call357 = call float @_Z3minff(float %513, float %514) nounwind readnone
  store float %call357, float* %a_out, align 4
  %515 = load <4 x float>* %a4_in, align 16
  %516 = load <4 x float>* %b4_in, align 16
  %call358 = call <4 x float> @_Z3minDv4_fS_(<4 x float> %515, <4 x float> %516) nounwind readnone
  store <4 x float> %call358, <4 x float>* %a4_out, align 16
  %517 = load <8 x float>* %a8_in, align 32
  %518 = load <8 x float>* %b8_in, align 32
  %call359 = call <8 x float> @_Z3minDv8_fS_(<8 x float> %517, <8 x float> %518) nounwind readnone
  store <8 x float> %call359, <8 x float>* %a8_out, align 32
  %519 = load <16 x float>* %a16_in, align 64
  %520 = load <16 x float>* %b16_in, align 64
  %call360 = call <16 x float> @_Z3minDv16_fS_(<16 x float> %519, <16 x float> %520) nounwind readnone
  store <16 x float> %call360, <16 x float>* %a16_out, align 64
  %521 = load <4 x float>* %a4_in, align 16
  %522 = load float* %b_in, align 4
  %call361 = call <4 x float> @_Z3minDv4_ff(<4 x float> %521, float %522) nounwind readnone
  store <4 x float> %call361, <4 x float>* %a4_out, align 16
  %523 = load <8 x float>* %a8_in, align 32
  %524 = load float* %b_in, align 4
  %call362 = call <8 x float> @_Z3minDv8_ff(<8 x float> %523, float %524) nounwind readnone
  store <8 x float> %call362, <8 x float>* %a8_out, align 32
  %525 = load <16 x float>* %a16_in, align 64
  %526 = load float* %b_in, align 4
  %call363 = call <16 x float> @_Z3minDv16_ff(<16 x float> %525, float %526) nounwind readnone
  store <16 x float> %call363, <16 x float>* %a16_out, align 64
  %527 = load float* %a_in, align 4
  %528 = load float* %b_in, align 4
  %call364 = call float @_Z3maxff(float %527, float %528) nounwind readnone
  store float %call364, float* %a_out, align 4
  %529 = load <4 x float>* %a4_in, align 16
  %530 = load <4 x float>* %b4_in, align 16
  %call365 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %529, <4 x float> %530) nounwind readnone
  store <4 x float> %call365, <4 x float>* %a4_out, align 16
  %531 = load <8 x float>* %a8_in, align 32
  %532 = load <8 x float>* %b8_in, align 32
  %call366 = call <8 x float> @_Z3maxDv8_fS_(<8 x float> %531, <8 x float> %532) nounwind readnone
  store <8 x float> %call366, <8 x float>* %a8_out, align 32
  %533 = load <16 x float>* %a16_in, align 64
  %534 = load <16 x float>* %b16_in, align 64
  %call367 = call <16 x float> @_Z3maxDv16_fS_(<16 x float> %533, <16 x float> %534) nounwind readnone
  store <16 x float> %call367, <16 x float>* %a16_out, align 64
  %535 = load <4 x float>* %a4_in, align 16
  %536 = load float* %b_in, align 4
  %call368 = call <4 x float> @_Z3maxDv4_ff(<4 x float> %535, float %536) nounwind readnone
  store <4 x float> %call368, <4 x float>* %a4_out, align 16
  %537 = load <8 x float>* %a8_in, align 32
  %538 = load float* %b_in, align 4
  %call369 = call <8 x float> @_Z3maxDv8_ff(<8 x float> %537, float %538) nounwind readnone
  store <8 x float> %call369, <8 x float>* %a8_out, align 32
  %539 = load <16 x float>* %a16_in, align 64
  %540 = load float* %b_in, align 4
  %call370 = call <16 x float> @_Z3maxDv16_ff(<16 x float> %539, float %540) nounwind readnone
  store <16 x float> %call370, <16 x float>* %a16_out, align 64
  %541 = load float* %a_in, align 4
  %542 = load float* %b_in, align 4
  %call371 = call float @_Z5hypotff(float %541, float %542) nounwind readnone
  store float %call371, float* %a_out, align 4
  %543 = load <4 x float>* %a4_in, align 16
  %544 = load <4 x float>* %b4_in, align 16
  %call372 = call <4 x float> @_Z5hypotDv4_fS_(<4 x float> %543, <4 x float> %544) nounwind readnone
  store <4 x float> %call372, <4 x float>* %a4_out, align 16
  %545 = load <8 x float>* %a8_in, align 32
  %546 = load <8 x float>* %b8_in, align 32
  %call373 = call <8 x float> @_Z5hypotDv8_fS_(<8 x float> %545, <8 x float> %546) nounwind readnone
  store <8 x float> %call373, <8 x float>* %a8_out, align 32
  %547 = load <16 x float>* %a16_in, align 64
  %548 = load <16 x float>* %b16_in, align 64
  %call374 = call <16 x float> @_Z5hypotDv16_fS_(<16 x float> %547, <16 x float> %548) nounwind readnone
  store <16 x float> %call374, <16 x float>* %a16_out, align 64
  %549 = load float* %a_in, align 4
  %550 = load float* %b_in, align 4
  %call375 = call float @_Z4stepff(float %549, float %550) nounwind readnone
  store float %call375, float* %a_out, align 4
  %551 = load <4 x float>* %a4_in, align 16
  %552 = load <4 x float>* %b4_in, align 16
  %call376 = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %551, <4 x float> %552) nounwind readnone
  store <4 x float> %call376, <4 x float>* %a4_out, align 16
  %553 = load <8 x float>* %a8_in, align 32
  %554 = load <8 x float>* %b8_in, align 32
  %call377 = call <8 x float> @_Z4stepDv8_fS_(<8 x float> %553, <8 x float> %554) nounwind readnone
  store <8 x float> %call377, <8 x float>* %a8_out, align 32
  %555 = load <16 x float>* %a16_in, align 64
  %556 = load <16 x float>* %b16_in, align 64
  %call378 = call <16 x float> @_Z4stepDv16_fS_(<16 x float> %555, <16 x float> %556) nounwind readnone
  store <16 x float> %call378, <16 x float>* %a16_out, align 64
  %557 = load float* %a_in, align 4
  %558 = load float* %b_in, align 4
  %call379 = call float @_Z4stepff(float %557, float %558) nounwind readnone
  store float %call379, float* %a_out, align 4
  %559 = load float* %a_in, align 4
  %560 = load <4 x float>* %b4_in, align 16
  %call380 = call <4 x float> @_Z4stepfDv4_f(float %559, <4 x float> %560) nounwind readnone
  store <4 x float> %call380, <4 x float>* %a4_out, align 16
  %561 = load float* %a_in, align 4
  %562 = load <8 x float>* %b8_in, align 32
  %call381 = call <8 x float> @_Z4stepfDv8_f(float %561, <8 x float> %562) nounwind readnone
  store <8 x float> %call381, <8 x float>* %a8_out, align 32
  %563 = load float* %a_in, align 4
  %564 = load <16 x float>* %b16_in, align 64
  %call382 = call <16 x float> @_Z4stepfDv16_f(float %563, <16 x float> %564) nounwind readnone
  store <16 x float> %call382, <16 x float>* %a16_out, align 64
  %565 = load float* %a_in, align 4
  %566 = load float* %b_in, align 4
  %567 = load float* %c_in, align 4
  %call383 = call float @_Z10smoothstepfff(float %565, float %566, float %567) nounwind readnone
  store float %call383, float* %a_out, align 4
  %568 = load <4 x float>* %a4_in, align 16
  %569 = load <4 x float>* %b4_in, align 16
  %570 = load <4 x float>* %c4_in, align 16
  %call384 = call <4 x float> @_Z10smoothstepDv4_fS_S_(<4 x float> %568, <4 x float> %569, <4 x float> %570) nounwind readnone
  store <4 x float> %call384, <4 x float>* %a4_out, align 16
  %571 = load <8 x float>* %a8_in, align 32
  %572 = load <8 x float>* %b8_in, align 32
  %573 = load <8 x float>* %c8_in, align 32
  %call385 = call <8 x float> @_Z10smoothstepDv8_fS_S_(<8 x float> %571, <8 x float> %572, <8 x float> %573) nounwind readnone
  store <8 x float> %call385, <8 x float>* %a8_out, align 32
  %574 = load <16 x float>* %a16_in, align 64
  %575 = load <16 x float>* %b16_in, align 64
  %576 = load <16 x float>* %c16_in, align 64
  %call386 = call <16 x float> @_Z10smoothstepDv16_fS_S_(<16 x float> %574, <16 x float> %575, <16 x float> %576) nounwind readnone
  store <16 x float> %call386, <16 x float>* %a16_out, align 64
  %577 = load float* %a_in, align 4
  %578 = load float* %b_in, align 4
  %579 = load float* %c_in, align 4
  %call387 = call float @_Z10smoothstepfff(float %577, float %578, float %579) nounwind readnone
  store float %call387, float* %a_out, align 4
  %580 = load float* %a_in, align 4
  %581 = load float* %b_in, align 4
  %582 = load <4 x float>* %c4_in, align 16
  %call388 = call <4 x float> @_Z10smoothstepffDv4_f(float %580, float %581, <4 x float> %582) nounwind readnone
  store <4 x float> %call388, <4 x float>* %a4_out, align 16
  %583 = load float* %a_in, align 4
  %584 = load float* %b_in, align 4
  %585 = load <8 x float>* %c8_in, align 32
  %call389 = call <8 x float> @_Z10smoothstepffDv8_f(float %583, float %584, <8 x float> %585) nounwind readnone
  store <8 x float> %call389, <8 x float>* %a8_out, align 32
  %586 = load float* %a_in, align 4
  %587 = load float* %b_in, align 4
  %588 = load <16 x float>* %c16_in, align 64
  %call390 = call <16 x float> @_Z10smoothstepffDv16_f(float %586, float %587, <16 x float> %588) nounwind readnone
  store <16 x float> %call390, <16 x float>* %a16_out, align 64
  %589 = load float* %a_in, align 4
  %call391 = call float @_Z7radiansf(float %589) nounwind readnone
  store float %call391, float* %a_out, align 4
  %590 = load <4 x float>* %a4_in, align 16
  %call392 = call <4 x float> @_Z7radiansDv4_f(<4 x float> %590) nounwind readnone
  store <4 x float> %call392, <4 x float>* %a4_out, align 16
  %591 = load <8 x float>* %a8_in, align 32
  %call393 = call <8 x float> @_Z7radiansDv8_f(<8 x float> %591) nounwind readnone
  store <8 x float> %call393, <8 x float>* %a8_out, align 32
  %592 = load <16 x float>* %a16_in, align 64
  %call394 = call <16 x float> @_Z7radiansDv16_f(<16 x float> %592) nounwind readnone
  store <16 x float> %call394, <16 x float>* %a16_out, align 64
  %593 = load float* %a_in, align 4
  %call395 = call float @_Z7degreesf(float %593) nounwind readnone
  store float %call395, float* %a_out, align 4
  %594 = load <4 x float>* %a4_in, align 16
  %call396 = call <4 x float> @_Z7degreesDv4_f(<4 x float> %594) nounwind readnone
  store <4 x float> %call396, <4 x float>* %a4_out, align 16
  %595 = load <8 x float>* %a8_in, align 32
  %call397 = call <8 x float> @_Z7degreesDv8_f(<8 x float> %595) nounwind readnone
  store <8 x float> %call397, <8 x float>* %a8_out, align 32
  %596 = load <16 x float>* %a16_in, align 64
  %call398 = call <16 x float> @_Z7degreesDv16_f(<16 x float> %596) nounwind readnone
  store <16 x float> %call398, <16 x float>* %a16_out, align 64
  %597 = load float* %a_in, align 4
  %call399 = call float @_Z4signf(float %597) nounwind readnone
  store float %call399, float* %a_out, align 4
  %598 = load <4 x float>* %a4_in, align 16
  %call400 = call <4 x float> @_Z4signDv4_f(<4 x float> %598) nounwind readnone
  store <4 x float> %call400, <4 x float>* %a4_out, align 16
  %599 = load <8 x float>* %a8_in, align 32
  %call401 = call <8 x float> @_Z4signDv8_f(<8 x float> %599) nounwind readnone
  store <8 x float> %call401, <8 x float>* %a8_out, align 32
  %600 = load <16 x float>* %a16_in, align 64
  %call402 = call <16 x float> @_Z4signDv16_f(<16 x float> %600) nounwind readnone
  store <16 x float> %call402, <16 x float>* %a16_out, align 64
  %601 = load float* %a_in, align 4
  %call403 = call float @_Z5floorf(float %601) nounwind readnone
  store float %call403, float* %a_out, align 4
  %602 = load <4 x float>* %a4_in, align 16
  %call404 = call <4 x float> @_Z5floorDv4_f(<4 x float> %602) nounwind readnone
  store <4 x float> %call404, <4 x float>* %a4_out, align 16
  %603 = load <8 x float>* %a8_in, align 32
  %call405 = call <8 x float> @_Z5floorDv8_f(<8 x float> %603) nounwind readnone
  store <8 x float> %call405, <8 x float>* %a8_out, align 32
  %604 = load <16 x float>* %a16_in, align 64
  %call406 = call <16 x float> @_Z5floorDv16_f(<16 x float> %604) nounwind readnone
  store <16 x float> %call406, <16 x float>* %a16_out, align 64
  %605 = load float* %a_in, align 4
  %606 = load float* %b_in, align 4
  %call407 = call float @_Z3dotff(float %605, float %606) nounwind readnone
  store float %call407, float* %a_out, align 4
  %607 = load <4 x float>* %a4_in, align 16
  %608 = load <4 x float>* %b4_in, align 16
  %call408 = call float @_Z3dotDv4_fS_(<4 x float> %607, <4 x float> %608) nounwind readnone
  store float %call408, float* %a_out, align 4
  %609 = load float* %a_in, align 4
  %610 = load float* %b_in, align 4
  %611 = load float* %c_in, align 4
  %call409 = call float @_Z3mixfff(float %609, float %610, float %611) nounwind readnone
  store float %call409, float* %a_out, align 4
  %612 = load <4 x float>* %a4_in, align 16
  %613 = load <4 x float>* %b4_in, align 16
  %614 = load <4 x float>* %c4_in, align 16
  %call410 = call <4 x float> @_Z3mixDv4_fS_S_(<4 x float> %612, <4 x float> %613, <4 x float> %614) nounwind readnone
  store <4 x float> %call410, <4 x float>* %a4_out, align 16
  %615 = load <4 x float>* %a4_in, align 16
  %616 = load <4 x float>* %b4_in, align 16
  %617 = load float* %c_in, align 4
  %call411 = call <4 x float> @_Z3mixDv4_fS_f(<4 x float> %615, <4 x float> %616, float %617) nounwind readnone
  store <4 x float> %call411, <4 x float>* %a4_out, align 16
  %618 = load <8 x float>* %a8_in, align 32
  %619 = load <8 x float>* %b8_in, align 32
  %620 = load float* %c_in, align 4
  %call412 = call <8 x float> @_Z3mixDv8_fS_f(<8 x float> %618, <8 x float> %619, float %620) nounwind readnone
  store <8 x float> %call412, <8 x float>* %a8_out, align 32
  %621 = load <16 x float>* %a16_in, align 64
  %622 = load <16 x float>* %b16_in, align 64
  %623 = load float* %c_in, align 4
  %call413 = call <16 x float> @_Z3mixDv16_fS_f(<16 x float> %621, <16 x float> %622, float %623) nounwind readnone
  store <16 x float> %call413, <16 x float>* %a16_out, align 64
  %624 = load <8 x float>* %a8_in, align 32
  %625 = load <8 x float>* %b8_in, align 32
  %626 = load <8 x float>* %c8_in, align 32
  %call414 = call <8 x float> @_Z3mixDv8_fS_S_(<8 x float> %624, <8 x float> %625, <8 x float> %626) nounwind readnone
  store <8 x float> %call414, <8 x float>* %a8_out, align 32
  %627 = load <16 x float>* %a16_in, align 64
  %628 = load <16 x float>* %b16_in, align 64
  %629 = load <16 x float>* %c16_in, align 64
  %call415 = call <16 x float> @_Z3mixDv16_fS_S_(<16 x float> %627, <16 x float> %628, <16 x float> %629) nounwind readnone
  store <16 x float> %call415, <16 x float>* %a16_out, align 64
  %630 = load float* %a_in, align 4
  %call416 = call float @_Z9normalizef(float %630) nounwind readnone
  store float %call416, float* %a_out, align 4
  %631 = load <4 x float>* %a4_in, align 16
  %call417 = call <4 x float> @_Z9normalizeDv4_f(<4 x float> %631) nounwind readnone
  store <4 x float> %call417, <4 x float>* %a4_out, align 16
  %632 = load float* %a_in, align 4
  %call418 = call float @_Z14fast_normalizef(float %632) nounwind readnone
  store float %call418, float* %a_out, align 4
  %633 = load <4 x float>* %a4_in, align 16
  %call419 = call <4 x float> @_Z14fast_normalizeDv4_f(<4 x float> %633) nounwind readnone
  store <4 x float> %call419, <4 x float>* %a4_out, align 16
  %634 = load <4 x float>* %a4_in, align 16
  %635 = load <4 x float>* %b4_in, align 16
  %call420 = call <4 x float> @_Z5crossDv4_fS_(<4 x float> %634, <4 x float> %635) nounwind readnone
  store <4 x float> %call420, <4 x float>* %a4_out, align 16
  %636 = load float* %a_in, align 4
  %call421 = call float @_Z6lengthf(float %636) nounwind readnone
  store float %call421, float* %a_out, align 4
  %637 = load <2 x float>* %a2_in, align 8
  %call422 = call float @_Z6lengthDv2_f(<2 x float> %637) nounwind readnone
  store float %call422, float* %a_out, align 4
  %638 = load <4 x float>* %a4_in, align 16
  %call423 = call float @_Z6lengthDv4_f(<4 x float> %638) nounwind readnone
  store float %call423, float* %a_out, align 4
  %639 = load float* %a_in, align 4
  %call424 = call float @_Z11fast_lengthf(float %639) nounwind readnone
  store float %call424, float* %a_out, align 4
  %640 = load <2 x float>* %a2_in, align 8
  %call425 = call float @_Z11fast_lengthDv2_f(<2 x float> %640) nounwind readnone
  store float %call425, float* %a_out, align 4
  %641 = load <4 x float>* %a4_in, align 16
  %call426 = call float @_Z11fast_lengthDv4_f(<4 x float> %641) nounwind readnone
  store float %call426, float* %a_out, align 4
  %642 = load float* %a_in, align 4
  %643 = load float* %b_in, align 4
  %call427 = call float @_Z8distanceff(float %642, float %643) nounwind readnone
  store float %call427, float* %a_out, align 4
  %644 = load <2 x float>* %a2_in, align 8
  %645 = load <2 x float>* %b2_in, align 8
  %call428 = call float @_Z8distanceDv2_fS_(<2 x float> %644, <2 x float> %645) nounwind readnone
  store float %call428, float* %a_out, align 4
  %646 = load <4 x float>* %a4_in, align 16
  %647 = load <4 x float>* %b4_in, align 16
  %call429 = call float @_Z8distanceDv4_fS_(<4 x float> %646, <4 x float> %647) nounwind readnone
  store float %call429, float* %a_out, align 4
  %648 = load float* %a_in, align 4
  %649 = load float* %b_in, align 4
  %call430 = call float @_Z13fast_distanceff(float %648, float %649) nounwind readnone
  store float %call430, float* %a_out, align 4
  %650 = load <2 x float>* %a2_in, align 8
  %651 = load <2 x float>* %b2_in, align 8
  %call431 = call float @_Z13fast_distanceDv2_fS_(<2 x float> %650, <2 x float> %651) nounwind readnone
  store float %call431, float* %a_out, align 4
  %652 = load <4 x float>* %a4_in, align 16
  %653 = load <4 x float>* %b4_in, align 16
  %call432 = call float @_Z13fast_distanceDv4_fS_(<4 x float> %652, <4 x float> %653) nounwind readnone
  store float %call432, float* %a_out, align 4
  %654 = load i32* %tid, align 4
  %call433 = call float @_Z13convert_floati(i32 %654) nounwind readnone
  store float %call433, float* %a_out, align 4
  %655 = load i32* %tid, align 4
  %656 = insertelement <4 x i32> undef, i32 %655, i32 0
  %splat434 = shufflevector <4 x i32> %656, <4 x i32> %656, <4 x i32> zeroinitializer
  %call435 = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %splat434) nounwind readnone
  store <4 x float> %call435, <4 x float>* %a4_out, align 16
  %657 = load i32* %tid, align 4
  %658 = insertelement <8 x i32> undef, i32 %657, i32 0
  %splat436 = shufflevector <8 x i32> %658, <8 x i32> %658, <8 x i32> zeroinitializer
  %call437 = call <8 x float> @_Z14convert_float8Dv8_i(<8 x i32> %splat436) nounwind readnone
  store <8 x float> %call437, <8 x float>* %a8_out, align 32
  %659 = load i32* %tid, align 4
  %660 = insertelement <16 x i32> undef, i32 %659, i32 0
  %splat438 = shufflevector <16 x i32> %660, <16 x i32> %660, <16 x i32> zeroinitializer
  %call439 = call <16 x float> @_Z15convert_float16Dv16_i(<16 x i32> %splat438) nounwind readnone
  store <16 x float> %call439, <16 x float>* %a16_out, align 64
  %661 = load i32* %tid, align 4
  %call440 = call float @_Z13convert_floatj(i32 %661) nounwind readnone
  store float %call440, float* %a_out, align 4
  %662 = load i32* %tid, align 4
  %663 = insertelement <4 x i32> undef, i32 %662, i32 0
  %splat441 = shufflevector <4 x i32> %663, <4 x i32> %663, <4 x i32> zeroinitializer
  %call442 = call <4 x float> @_Z14convert_float4Dv4_j(<4 x i32> %splat441) nounwind readnone
  store <4 x float> %call442, <4 x float>* %a4_out, align 16
  %664 = load i32* %tid, align 4
  %665 = insertelement <8 x i32> undef, i32 %664, i32 0
  %splat443 = shufflevector <8 x i32> %665, <8 x i32> %665, <8 x i32> zeroinitializer
  %call444 = call <8 x float> @_Z14convert_float8Dv8_j(<8 x i32> %splat443) nounwind readnone
  store <8 x float> %call444, <8 x float>* %a8_out, align 32
  %666 = load i32* %tid, align 4
  %667 = insertelement <16 x i32> undef, i32 %666, i32 0
  %splat445 = shufflevector <16 x i32> %667, <16 x i32> %667, <16 x i32> zeroinitializer
  %call446 = call <16 x float> @_Z15convert_float16Dv16_j(<16 x i32> %splat445) nounwind readnone
  store <16 x float> %call446, <16 x float>* %a16_out, align 64
  %668 = load float* %a_in, align 4
  %669 = load i32* %i_in, align 4
  %call447 = call float @_Z5rootnfi(float %668, i32 %669) nounwind readnone
  store float %call447, float* %a_out, align 4
  %670 = load <4 x float>* %a4_in, align 16
  %671 = load <4 x i32>* %i4_in, align 16
  %call448 = call <4 x float> @_Z5rootnDv4_fDv4_i(<4 x float> %670, <4 x i32> %671) nounwind readnone
  store <4 x float> %call448, <4 x float>* %a4_out, align 16
  %672 = load <8 x float>* %a8_in, align 32
  %673 = load <8 x i32>* %i8_in, align 32
  %call449 = call <8 x float> @_Z5rootnDv8_fDv8_i(<8 x float> %672, <8 x i32> %673) nounwind readnone
  store <8 x float> %call449, <8 x float>* %a8_out, align 32
  %674 = load <16 x float>* %a16_in, align 64
  %675 = load <16 x i32>* %i16_in, align 64
  %call450 = call <16 x float> @_Z5rootnDv16_fDv16_i(<16 x float> %674, <16 x i32> %675) nounwind readnone
  store <16 x float> %call450, <16 x float>* %a16_out, align 64
  %676 = load float* %a_in, align 4
  %677 = load i32* %i_in, align 4
  %call451 = call float @_Z5ldexpfi(float %676, i32 %677) nounwind readnone
  store float %call451, float* %a_out, align 4
  %678 = load <4 x float>* %a4_in, align 16
  %679 = load <4 x i32>* %i4_in, align 16
  %call452 = call <4 x float> @_Z5ldexpDv4_fDv4_i(<4 x float> %678, <4 x i32> %679) nounwind readnone
  store <4 x float> %call452, <4 x float>* %a4_out, align 16
  %680 = load <8 x float>* %a8_in, align 32
  %681 = load <8 x i32>* %i8_in, align 32
  %call453 = call <8 x float> @_Z5ldexpDv8_fDv8_i(<8 x float> %680, <8 x i32> %681) nounwind readnone
  store <8 x float> %call453, <8 x float>* %a8_out, align 32
  %682 = load <16 x float>* %a16_in, align 64
  %683 = load <16 x i32>* %i16_in, align 64
  %call454 = call <16 x float> @_Z5ldexpDv16_fDv16_i(<16 x float> %682, <16 x i32> %683) nounwind readnone
  store <16 x float> %call454, <16 x float>* %a16_out, align 64
  %684 = load <4 x float>* %a4_in, align 16
  %685 = load i32* %i_in, align 4
  %call455 = call <4 x float> @_Z5ldexpDv4_fi(<4 x float> %684, i32 %685) nounwind readnone
  store <4 x float> %call455, <4 x float>* %a4_out, align 16
  %686 = load <8 x float>* %a8_in, align 32
  %687 = load i32* %i_in, align 4
  %call456 = call <8 x float> @_Z5ldexpDv8_fi(<8 x float> %686, i32 %687) nounwind readnone
  store <8 x float> %call456, <8 x float>* %a8_out, align 32
  %688 = load <16 x float>* %a16_in, align 64
  %689 = load i32* %i_in, align 4
  %call457 = call <16 x float> @_Z5ldexpDv16_fi(<16 x float> %688, i32 %689) nounwind readnone
  store <16 x float> %call457, <16 x float>* %a16_out, align 64
  %690 = load float* %a_in, align 4
  %call458 = call float @_Z4modffPf(float %690, float* %b_out)
  store float %call458, float* %a_out, align 4
  %691 = load <4 x float>* %a4_in, align 16
  %call459 = call <4 x float> @_Z4modfDv4_fPS_(<4 x float> %691, <4 x float>* %b4_out)
  store <4 x float> %call459, <4 x float>* %a4_out, align 16
  %692 = load <8 x float>* %a8_in, align 32
  %call460 = call <8 x float> @_Z4modfDv8_fPS_(<8 x float> %692, <8 x float>* %b8_out)
  store <8 x float> %call460, <8 x float>* %a8_out, align 32
  %693 = load <16 x float>* %a16_in, align 64
  %call461 = call <16 x float> @_Z4modfDv16_fPS_(<16 x float> %693, <16 x float>* %b16_out)
  store <16 x float> %call461, <16 x float>* %a16_out, align 64
  %694 = load float* %a_in, align 4
  %call462 = call float @_Z5frexpfPi(float %694, i32* %i_out)
  store float %call462, float* %a_out, align 4
  %695 = load <4 x float>* %a4_in, align 16
  %call463 = call <4 x float> @_Z5frexpDv4_fPDv4_i(<4 x float> %695, <4 x i32>* %i4_out)
  store <4 x float> %call463, <4 x float>* %a4_out, align 16
  %696 = load <8 x float>* %a8_in, align 32
  %call464 = call <8 x float> @_Z5frexpDv8_fPDv8_i(<8 x float> %696, <8 x i32>* %i8_out)
  store <8 x float> %call464, <8 x float>* %a8_out, align 32
  %697 = load <16 x float>* %a16_in, align 64
  %call465 = call <16 x float> @_Z5frexpDv16_fPDv16_i(<16 x float> %697, <16 x i32>* %i16_out)
  store <16 x float> %call465, <16 x float>* %a16_out, align 64
  %698 = load float* %a_in, align 4
  %699 = load float* %b_in, align 4
  %call466 = call float @_Z6maxmagff(float %698, float %699) nounwind readnone
  store float %call466, float* %a_out, align 4
  %700 = load <4 x float>* %a4_in, align 16
  %701 = load <4 x float>* %b4_in, align 16
  %call467 = call <4 x float> @_Z6maxmagDv4_fS_(<4 x float> %700, <4 x float> %701) nounwind readnone
  store <4 x float> %call467, <4 x float>* %a4_out, align 16
  %702 = load <8 x float>* %a8_in, align 32
  %703 = load <8 x float>* %b8_in, align 32
  %call468 = call <8 x float> @_Z6maxmagDv8_fS_(<8 x float> %702, <8 x float> %703) nounwind readnone
  store <8 x float> %call468, <8 x float>* %a8_out, align 32
  %704 = load <16 x float>* %a16_in, align 64
  %705 = load <16 x float>* %b16_in, align 64
  %call469 = call <16 x float> @_Z6maxmagDv16_fS_(<16 x float> %704, <16 x float> %705) nounwind readnone
  store <16 x float> %call469, <16 x float>* %a16_out, align 64
  %706 = load float* %a_in, align 4
  %707 = load float* %b_in, align 4
  %call470 = call float @_Z6minmagff(float %706, float %707) nounwind readnone
  store float %call470, float* %a_out, align 4
  %708 = load <4 x float>* %a4_in, align 16
  %709 = load <4 x float>* %b4_in, align 16
  %call471 = call <4 x float> @_Z6minmagDv4_fS_(<4 x float> %708, <4 x float> %709) nounwind readnone
  store <4 x float> %call471, <4 x float>* %a4_out, align 16
  %710 = load <8 x float>* %a8_in, align 32
  %711 = load <8 x float>* %b8_in, align 32
  %call472 = call <8 x float> @_Z6minmagDv8_fS_(<8 x float> %710, <8 x float> %711) nounwind readnone
  store <8 x float> %call472, <8 x float>* %a8_out, align 32
  %712 = load <16 x float>* %a16_in, align 64
  %713 = load <16 x float>* %b16_in, align 64
  %call473 = call <16 x float> @_Z6minmagDv16_fS_(<16 x float> %712, <16 x float> %713) nounwind readnone
  store <16 x float> %call473, <16 x float>* %a16_out, align 64
  %714 = load float* %a_in, align 4
  %715 = load float* %b_in, align 4
  %call474 = call float @_Z8copysignff(float %714, float %715) nounwind readnone
  store float %call474, float* %a_out, align 4
  %716 = load <4 x float>* %a4_in, align 16
  %717 = load <4 x float>* %b4_in, align 16
  %call475 = call <4 x float> @_Z8copysignDv4_fS_(<4 x float> %716, <4 x float> %717) nounwind readnone
  store <4 x float> %call475, <4 x float>* %a4_out, align 16
  %718 = load <8 x float>* %a8_in, align 32
  %719 = load <8 x float>* %b8_in, align 32
  %call476 = call <8 x float> @_Z8copysignDv8_fS_(<8 x float> %718, <8 x float> %719) nounwind readnone
  store <8 x float> %call476, <8 x float>* %a8_out, align 32
  %720 = load <16 x float>* %a16_in, align 64
  %721 = load <16 x float>* %b16_in, align 64
  %call477 = call <16 x float> @_Z8copysignDv16_fS_(<16 x float> %720, <16 x float> %721) nounwind readnone
  store <16 x float> %call477, <16 x float>* %a16_out, align 64
  %722 = load float* %a_in, align 4
  %723 = load float* %b_in, align 4
  %call478 = call float @_Z9nextafterff(float %722, float %723) nounwind readnone
  store float %call478, float* %a_out, align 4
  %724 = load <4 x float>* %a4_in, align 16
  %725 = load <4 x float>* %b4_in, align 16
  %call479 = call <4 x float> @_Z9nextafterDv4_fS_(<4 x float> %724, <4 x float> %725) nounwind readnone
  store <4 x float> %call479, <4 x float>* %a4_out, align 16
  %726 = load <8 x float>* %a8_in, align 32
  %727 = load <8 x float>* %b8_in, align 32
  %call480 = call <8 x float> @_Z9nextafterDv8_fS_(<8 x float> %726, <8 x float> %727) nounwind readnone
  store <8 x float> %call480, <8 x float>* %a8_out, align 32
  %728 = load <16 x float>* %a16_in, align 64
  %729 = load <16 x float>* %b16_in, align 64
  %call481 = call <16 x float> @_Z9nextafterDv16_fS_(<16 x float> %728, <16 x float> %729) nounwind readnone
  store <16 x float> %call481, <16 x float>* %a16_out, align 64
  %730 = load float* %a_in, align 4
  %731 = load float* %b_in, align 4
  %call482 = call float @_Z4fdimff(float %730, float %731) nounwind readnone
  store float %call482, float* %a_out, align 4
  %732 = load <4 x float>* %a4_in, align 16
  %733 = load <4 x float>* %b4_in, align 16
  %call483 = call <4 x float> @_Z4fdimDv4_fS_(<4 x float> %732, <4 x float> %733) nounwind readnone
  store <4 x float> %call483, <4 x float>* %a4_out, align 16
  %734 = load <8 x float>* %a8_in, align 32
  %735 = load <8 x float>* %b8_in, align 32
  %call484 = call <8 x float> @_Z4fdimDv8_fS_(<8 x float> %734, <8 x float> %735) nounwind readnone
  store <8 x float> %call484, <8 x float>* %a8_out, align 32
  %736 = load <16 x float>* %a16_in, align 64
  %737 = load <16 x float>* %b16_in, align 64
  %call485 = call <16 x float> @_Z4fdimDv16_fS_(<16 x float> %736, <16 x float> %737) nounwind readnone
  store <16 x float> %call485, <16 x float>* %a16_out, align 64
  %738 = load float* %a_in, align 4
  %739 = load float* %b_in, align 4
  %740 = load float* %c_in, align 4
  %call486 = call float @_Z3fmafff(float %738, float %739, float %740) nounwind readnone
  store float %call486, float* %a_out, align 4
  %741 = load <4 x float>* %a4_in, align 16
  %742 = load <4 x float>* %b4_in, align 16
  %743 = load <4 x float>* %c4_in, align 16
  %call487 = call <4 x float> @_Z3fmaDv4_fS_S_(<4 x float> %741, <4 x float> %742, <4 x float> %743) nounwind readnone
  store <4 x float> %call487, <4 x float>* %a4_out, align 16
  %744 = load <8 x float>* %a8_in, align 32
  %745 = load <8 x float>* %b8_in, align 32
  %746 = load <8 x float>* %c8_in, align 32
  %call488 = call <8 x float> @_Z3fmaDv8_fS_S_(<8 x float> %744, <8 x float> %745, <8 x float> %746) nounwind readnone
  store <8 x float> %call488, <8 x float>* %a8_out, align 32
  %747 = load <16 x float>* %a16_in, align 64
  %748 = load <16 x float>* %b16_in, align 64
  %749 = load <16 x float>* %c16_in, align 64
  %call489 = call <16 x float> @_Z3fmaDv16_fS_S_(<16 x float> %747, <16 x float> %748, <16 x float> %749) nounwind readnone
  store <16 x float> %call489, <16 x float>* %a16_out, align 64
  %750 = load float* %a_in, align 4
  %751 = load float* %b_in, align 4
  %752 = load float* %c_in, align 4
  %call490 = call float @_Z3madfff(float %750, float %751, float %752) nounwind readnone
  store float %call490, float* %a_out, align 4
  %753 = load <4 x float>* %a4_in, align 16
  %754 = load <4 x float>* %b4_in, align 16
  %755 = load <4 x float>* %c4_in, align 16
  %call491 = call <4 x float> @_Z3madDv4_fS_S_(<4 x float> %753, <4 x float> %754, <4 x float> %755) nounwind readnone
  store <4 x float> %call491, <4 x float>* %a4_out, align 16
  %756 = load <8 x float>* %a8_in, align 32
  %757 = load <8 x float>* %b8_in, align 32
  %758 = load <8 x float>* %c8_in, align 32
  %call492 = call <8 x float> @_Z3madDv8_fS_S_(<8 x float> %756, <8 x float> %757, <8 x float> %758) nounwind readnone
  store <8 x float> %call492, <8 x float>* %a8_out, align 32
  %759 = load <16 x float>* %a16_in, align 64
  %760 = load <16 x float>* %b16_in, align 64
  %761 = load <16 x float>* %c16_in, align 64
  %call493 = call <16 x float> @_Z3madDv16_fS_S_(<16 x float> %759, <16 x float> %760, <16 x float> %761) nounwind readnone
  store <16 x float> %call493, <16 x float>* %a16_out, align 64
  %762 = load float* %a_in, align 4
  %call494 = call float @_Z4rintf(float %762) nounwind readnone
  store float %call494, float* %a_out, align 4
  %763 = load <4 x float>* %a4_in, align 16
  %call495 = call <4 x float> @_Z4rintDv4_f(<4 x float> %763) nounwind readnone
  store <4 x float> %call495, <4 x float>* %a4_out, align 16
  %764 = load <8 x float>* %a8_in, align 32
  %call496 = call <8 x float> @_Z4rintDv8_f(<8 x float> %764) nounwind readnone
  store <8 x float> %call496, <8 x float>* %a8_out, align 32
  %765 = load <16 x float>* %a16_in, align 64
  %call497 = call <16 x float> @_Z4rintDv16_f(<16 x float> %765) nounwind readnone
  store <16 x float> %call497, <16 x float>* %a16_out, align 64
  %766 = load float* %a_in, align 4
  %call498 = call float @_Z5roundf(float %766) nounwind readnone
  store float %call498, float* %a_out, align 4
  %767 = load <4 x float>* %a4_in, align 16
  %call499 = call <4 x float> @_Z5roundDv4_f(<4 x float> %767) nounwind readnone
  store <4 x float> %call499, <4 x float>* %a4_out, align 16
  %768 = load <8 x float>* %a8_in, align 32
  %call500 = call <8 x float> @_Z5roundDv8_f(<8 x float> %768) nounwind readnone
  store <8 x float> %call500, <8 x float>* %a8_out, align 32
  %769 = load <16 x float>* %a16_in, align 64
  %call501 = call <16 x float> @_Z5roundDv16_f(<16 x float> %769) nounwind readnone
  store <16 x float> %call501, <16 x float>* %a16_out, align 64
  %770 = load float* %a_in, align 4
  %call502 = call float @_Z5truncf(float %770) nounwind readnone
  store float %call502, float* %a_out, align 4
  %771 = load <4 x float>* %a4_in, align 16
  %call503 = call <4 x float> @_Z5truncDv4_f(<4 x float> %771) nounwind readnone
  store <4 x float> %call503, <4 x float>* %a4_out, align 16
  %772 = load <8 x float>* %a8_in, align 32
  %call504 = call <8 x float> @_Z5truncDv8_f(<8 x float> %772) nounwind readnone
  store <8 x float> %call504, <8 x float>* %a8_out, align 32
  %773 = load <16 x float>* %a16_in, align 64
  %call505 = call <16 x float> @_Z5truncDv16_f(<16 x float> %773) nounwind readnone
  store <16 x float> %call505, <16 x float>* %a16_out, align 64
  %774 = load float* %a_in, align 4
  %call506 = call float @_Z4cbrtf(float %774) nounwind readnone
  store float %call506, float* %a_out, align 4
  %775 = load <4 x float>* %a4_in, align 16
  %call507 = call <4 x float> @_Z4cbrtDv4_f(<4 x float> %775) nounwind readnone
  store <4 x float> %call507, <4 x float>* %a4_out, align 16
  %776 = load <8 x float>* %a8_in, align 32
  %call508 = call <8 x float> @_Z4cbrtDv8_f(<8 x float> %776) nounwind readnone
  store <8 x float> %call508, <8 x float>* %a8_out, align 32
  %777 = load <16 x float>* %a16_in, align 64
  %call509 = call <16 x float> @_Z4cbrtDv16_f(<16 x float> %777) nounwind readnone
  store <16 x float> %call509, <16 x float>* %a16_out, align 64
  %778 = load float* %a_in, align 4
  %779 = load float* %b_in, align 4
  %call510 = call float @_Z4powrff(float %778, float %779) nounwind readnone
  store float %call510, float* %a_out, align 4
  %780 = load <4 x float>* %a4_in, align 16
  %781 = load <4 x float>* %b4_in, align 16
  %call511 = call <4 x float> @_Z4powrDv4_fS_(<4 x float> %780, <4 x float> %781) nounwind readnone
  store <4 x float> %call511, <4 x float>* %a4_out, align 16
  %782 = load <8 x float>* %a8_in, align 32
  %783 = load <8 x float>* %b8_in, align 32
  %call512 = call <8 x float> @_Z4powrDv8_fS_(<8 x float> %782, <8 x float> %783) nounwind readnone
  store <8 x float> %call512, <8 x float>* %a8_out, align 32
  %784 = load <16 x float>* %a16_in, align 64
  %785 = load <16 x float>* %b16_in, align 64
  %call513 = call <16 x float> @_Z4powrDv16_fS_(<16 x float> %784, <16 x float> %785) nounwind readnone
  store <16 x float> %call513, <16 x float>* %a16_out, align 64
  %786 = load float* %a_in, align 4
  %787 = load float* %b_in, align 4
  %call514 = call float @_Z4fmodff(float %786, float %787) nounwind readnone
  store float %call514, float* %a_out, align 4
  %788 = load <4 x float>* %a4_in, align 16
  %789 = load <4 x float>* %b4_in, align 16
  %call515 = call <4 x float> @_Z4fmodDv4_fS_(<4 x float> %788, <4 x float> %789) nounwind readnone
  store <4 x float> %call515, <4 x float>* %a4_out, align 16
  %790 = load <8 x float>* %a8_in, align 32
  %791 = load <8 x float>* %b8_in, align 32
  %call516 = call <8 x float> @_Z4fmodDv8_fS_(<8 x float> %790, <8 x float> %791) nounwind readnone
  store <8 x float> %call516, <8 x float>* %a8_out, align 32
  %792 = load <16 x float>* %a16_in, align 64
  %793 = load <16 x float>* %b16_in, align 64
  %call517 = call <16 x float> @_Z4fmodDv16_fS_(<16 x float> %792, <16 x float> %793) nounwind readnone
  store <16 x float> %call517, <16 x float>* %a16_out, align 64
  %794 = load float* %a_in, align 4
  %795 = load float* %b_in, align 4
  %call518 = call float @_Z4fminff(float %794, float %795) nounwind readnone
  store float %call518, float* %a_out, align 4
  %796 = load <4 x float>* %a4_in, align 16
  %797 = load <4 x float>* %b4_in, align 16
  %call519 = call <4 x float> @_Z4fminDv4_fS_(<4 x float> %796, <4 x float> %797) nounwind readnone
  store <4 x float> %call519, <4 x float>* %a4_out, align 16
  %798 = load <8 x float>* %a8_in, align 32
  %799 = load <8 x float>* %b8_in, align 32
  %call520 = call <8 x float> @_Z4fminDv8_fS_(<8 x float> %798, <8 x float> %799) nounwind readnone
  store <8 x float> %call520, <8 x float>* %a8_out, align 32
  %800 = load <16 x float>* %a16_in, align 64
  %801 = load <16 x float>* %b16_in, align 64
  %call521 = call <16 x float> @_Z4fminDv16_fS_(<16 x float> %800, <16 x float> %801) nounwind readnone
  store <16 x float> %call521, <16 x float>* %a16_out, align 64
  %802 = load float* %a_in, align 4
  %803 = load float* %b_in, align 4
  %call522 = call float @_Z4fmaxff(float %802, float %803) nounwind readnone
  store float %call522, float* %a_out, align 4
  %804 = load <4 x float>* %a4_in, align 16
  %805 = load <4 x float>* %b4_in, align 16
  %call523 = call <4 x float> @_Z4fmaxDv4_fS_(<4 x float> %804, <4 x float> %805) nounwind readnone
  store <4 x float> %call523, <4 x float>* %a4_out, align 16
  %806 = load <8 x float>* %a8_in, align 32
  %807 = load <8 x float>* %b8_in, align 32
  %call524 = call <8 x float> @_Z4fmaxDv8_fS_(<8 x float> %806, <8 x float> %807) nounwind readnone
  store <8 x float> %call524, <8 x float>* %a8_out, align 32
  %808 = load <16 x float>* %a16_in, align 64
  %809 = load <16 x float>* %b16_in, align 64
  %call525 = call <16 x float> @_Z4fmaxDv16_fS_(<16 x float> %808, <16 x float> %809) nounwind readnone
  store <16 x float> %call525, <16 x float>* %a16_out, align 64
  %810 = load <4 x float>* %a4_in, align 16
  %811 = load float* %b_in, align 4
  %call526 = call <4 x float> @_Z4fminDv4_ff(<4 x float> %810, float %811) nounwind readnone
  store <4 x float> %call526, <4 x float>* %a4_out, align 16
  %812 = load <8 x float>* %a8_in, align 32
  %813 = load float* %b_in, align 4
  %call527 = call <8 x float> @_Z4fminDv8_ff(<8 x float> %812, float %813) nounwind readnone
  store <8 x float> %call527, <8 x float>* %a8_out, align 32
  %814 = load <16 x float>* %a16_in, align 64
  %815 = load float* %b_in, align 4
  %call528 = call <16 x float> @_Z4fminDv16_ff(<16 x float> %814, float %815) nounwind readnone
  store <16 x float> %call528, <16 x float>* %a16_out, align 64
  %816 = load <4 x float>* %a4_in, align 16
  %817 = load float* %b_in, align 4
  %call529 = call <4 x float> @_Z4fmaxDv4_ff(<4 x float> %816, float %817) nounwind readnone
  store <4 x float> %call529, <4 x float>* %a4_out, align 16
  %818 = load <8 x float>* %a8_in, align 32
  %819 = load float* %b_in, align 4
  %call530 = call <8 x float> @_Z4fmaxDv8_ff(<8 x float> %818, float %819) nounwind readnone
  store <8 x float> %call530, <8 x float>* %a8_out, align 32
  %820 = load <16 x float>* %a16_in, align 64
  %821 = load float* %b_in, align 4
  %call531 = call <16 x float> @_Z4fmaxDv16_ff(<16 x float> %820, float %821) nounwind readnone
  store <16 x float> %call531, <16 x float>* %a16_out, align 64
  %822 = load <4 x float>* %a4_in, align 16
  %823 = load i32* %i_in, align 4
  %824 = insertelement <4 x i32> undef, i32 %823, i32 0
  %splat532 = shufflevector <4 x i32> %824, <4 x i32> %824, <4 x i32> zeroinitializer
  %call533 = call <4 x float> @_Z4pownDv4_fDv4_i(<4 x float> %822, <4 x i32> %splat532) nounwind readnone
  store <4 x float> %call533, <4 x float>* %a4_out, align 16
  %825 = load <8 x float>* %a8_in, align 32
  %826 = load i32* %i_in, align 4
  %827 = insertelement <8 x i32> undef, i32 %826, i32 0
  %splat534 = shufflevector <8 x i32> %827, <8 x i32> %827, <8 x i32> zeroinitializer
  %call535 = call <8 x float> @_Z4pownDv8_fDv8_i(<8 x float> %825, <8 x i32> %splat534) nounwind readnone
  store <8 x float> %call535, <8 x float>* %a8_out, align 32
  %828 = load <16 x float>* %a16_in, align 64
  %829 = load i32* %i_in, align 4
  %830 = insertelement <16 x i32> undef, i32 %829, i32 0
  %splat536 = shufflevector <16 x i32> %830, <16 x i32> %830, <16 x i32> zeroinitializer
  %call537 = call <16 x float> @_Z4pownDv16_fDv16_i(<16 x float> %828, <16 x i32> %splat536) nounwind readnone
  store <16 x float> %call537, <16 x float>* %a16_out, align 64
  %831 = load float* %a_in, align 4
  %call538 = call i32 @_Z5ilogbf(float %831) nounwind readnone
  store i32 %call538, i32* %i_out, align 4
  %832 = load <4 x float>* %a4_in, align 16
  %call539 = call <4 x i32> @_Z5ilogbDv4_f(<4 x float> %832) nounwind readnone
  store <4 x i32> %call539, <4 x i32>* %i4_out, align 16
  %833 = load <8 x float>* %a8_in, align 32
  %call540 = call <8 x i32> @_Z5ilogbDv8_f(<8 x float> %833) nounwind readnone
  store <8 x i32> %call540, <8 x i32>* %i8_out, align 32
  %834 = load <16 x float>* %a16_in, align 64
  %call541 = call <16 x i32> @_Z5ilogbDv16_f(<16 x float> %834) nounwind readnone
  store <16 x i32> %call541, <16 x i32>* %i16_out, align 64
  %835 = load i32* %ui_in, align 4
  %call542 = call float @_Z3nanj(i32 %835) nounwind readnone
  store float %call542, float* %a_out, align 4
  %836 = load <4 x i32>* %ui4_in, align 16
  %call543 = call <4 x float> @_Z3nanDv4_j(<4 x i32> %836) nounwind readnone
  store <4 x float> %call543, <4 x float>* %a4_out, align 16
  %837 = load <8 x i32>* %ui8_in, align 32
  %call544 = call <8 x float> @_Z3nanDv8_j(<8 x i32> %837) nounwind readnone
  store <8 x float> %call544, <8 x float>* %a8_out, align 32
  %838 = load <16 x i32>* %ui16_in, align 64
  %call545 = call <16 x float> @_Z3nanDv16_j(<16 x i32> %838) nounwind readnone
  store <16 x float> %call545, <16 x float>* %a16_out, align 64
  %839 = load float* %a_in, align 4
  %call546 = call float @_Z5fractfPf(float %839, float* %b_out)
  store float %call546, float* %a_out, align 4
  %840 = load <4 x float>* %a4_in, align 16
  %call547 = call <4 x float> @_Z5fractDv4_fPS_(<4 x float> %840, <4 x float>* %b4_out)
  store <4 x float> %call547, <4 x float>* %a4_out, align 16
  %841 = load <8 x float>* %a8_in, align 32
  %call548 = call <8 x float> @_Z5fractDv8_fPS_(<8 x float> %841, <8 x float>* %b8_out)
  store <8 x float> %call548, <8 x float>* %a8_out, align 32
  %842 = load <16 x float>* %a16_in, align 64
  %call549 = call <16 x float> @_Z5fractDv16_fPS_(<16 x float> %842, <16 x float>* %b16_out)
  store <16 x float> %call549, <16 x float>* %a16_out, align 64
  %843 = load float* %a_in, align 4
  %call550 = call float @_Z6lgammaf(float %843) nounwind readnone
  store float %call550, float* %a_out, align 4
  %844 = load <4 x float>* %a4_in, align 16
  %call551 = call <4 x float> @_Z6lgammaDv4_f(<4 x float> %844) nounwind readnone
  store <4 x float> %call551, <4 x float>* %a4_out, align 16
  %845 = load <8 x float>* %a8_in, align 32
  %call552 = call <8 x float> @_Z6lgammaDv8_f(<8 x float> %845) nounwind readnone
  store <8 x float> %call552, <8 x float>* %a8_out, align 32
  %846 = load <16 x float>* %a16_in, align 64
  %call553 = call <16 x float> @_Z6lgammaDv16_f(<16 x float> %846) nounwind readnone
  store <16 x float> %call553, <16 x float>* %a16_out, align 64
  %847 = load float* %a_in, align 4
  %call554 = call float @_Z8lgamma_rfPi(float %847, i32* %i_out)
  store float %call554, float* %a_out, align 4
  %848 = load <4 x float>* %a4_in, align 16
  %call555 = call <4 x float> @_Z8lgamma_rDv4_fPDv4_i(<4 x float> %848, <4 x i32>* %i4_out)
  store <4 x float> %call555, <4 x float>* %a4_out, align 16
  %849 = load <8 x float>* %a8_in, align 32
  %call556 = call <8 x float> @_Z8lgamma_rDv8_fPDv8_i(<8 x float> %849, <8 x i32>* %i8_out)
  store <8 x float> %call556, <8 x float>* %a8_out, align 32
  %850 = load <16 x float>* %a16_in, align 64
  %call557 = call <16 x float> @_Z8lgamma_rDv16_fPDv16_i(<16 x float> %850, <16 x i32>* %i16_out)
  store <16 x float> %call557, <16 x float>* %a16_out, align 64
  %851 = load float* %a_in, align 4
  %852 = load float* %b_in, align 4
  %853 = load float* %c_in, align 4
  %call558 = call float @_Z9bitselectfff(float %851, float %852, float %853) nounwind readnone
  store float %call558, float* %a_out, align 4
  %854 = load <4 x float>* %a4_in, align 16
  %855 = load <4 x float>* %b4_in, align 16
  %856 = load <4 x float>* %c4_in, align 16
  %call559 = call <4 x float> @_Z9bitselectDv4_fS_S_(<4 x float> %854, <4 x float> %855, <4 x float> %856) nounwind readnone
  store <4 x float> %call559, <4 x float>* %a4_out, align 16
  %857 = load <8 x float>* %a8_in, align 32
  %858 = load <8 x float>* %b8_in, align 32
  %859 = load <8 x float>* %c8_in, align 32
  %call560 = call <8 x float> @_Z9bitselectDv8_fS_S_(<8 x float> %857, <8 x float> %858, <8 x float> %859) nounwind readnone
  store <8 x float> %call560, <8 x float>* %a8_out, align 32
  %860 = load <16 x float>* %a16_in, align 64
  %861 = load <16 x float>* %b16_in, align 64
  %862 = load <16 x float>* %c16_in, align 64
  %call561 = call <16 x float> @_Z9bitselectDv16_fS_S_(<16 x float> %860, <16 x float> %861, <16 x float> %862) nounwind readnone
  store <16 x float> %call561, <16 x float>* %a16_out, align 64
  %863 = load float* %a_in, align 4
  %864 = load float* %b_in, align 4
  %865 = load i8* %ch_in, align 1
  %call562 = call float @_Z6selectffc(float %863, float %864, i8 signext %865) nounwind readnone
  store float %call562, float* %a_out, align 4
  %866 = load <4 x float>* %a4_in, align 16
  %867 = load <4 x float>* %b4_in, align 16
  %868 = load <4 x i8>* %ch4_in, align 4
  %call563 = call <4 x float> @_Z6selectDv4_fS_Dv4_c(<4 x float> %866, <4 x float> %867, <4 x i8> %868) nounwind readnone
  store <4 x float> %call563, <4 x float>* %a4_out, align 16
  %869 = load <8 x float>* %a8_in, align 32
  %870 = load <8 x float>* %b8_in, align 32
  %871 = load <8 x i8>* %ch8_in, align 8
  %call564 = call <8 x float> @_Z6selectDv8_fS_Dv8_c(<8 x float> %869, <8 x float> %870, <8 x i8> %871) nounwind readnone
  store <8 x float> %call564, <8 x float>* %a8_out, align 32
  %872 = load <16 x float>* %a16_in, align 64
  %873 = load <16 x float>* %b16_in, align 64
  %874 = load <16 x i8>* %ch16_in, align 16
  %call565 = call <16 x float> @_Z6selectDv16_fS_Dv16_c(<16 x float> %872, <16 x float> %873, <16 x i8> %874) nounwind readnone
  store <16 x float> %call565, <16 x float>* %a16_out, align 64
  %875 = load float* %a_in, align 4
  %876 = load float* %b_in, align 4
  %877 = load i8* %uch_in, align 1
  %call566 = call float @_Z6selectffh(float %875, float %876, i8 zeroext %877) nounwind readnone
  store float %call566, float* %a_out, align 4
  %878 = load <4 x float>* %a4_in, align 16
  %879 = load <4 x float>* %b4_in, align 16
  %880 = load <4 x i8>* %uch4_in, align 4
  %call567 = call <4 x float> @_Z6selectDv4_fS_Dv4_h(<4 x float> %878, <4 x float> %879, <4 x i8> %880) nounwind readnone
  store <4 x float> %call567, <4 x float>* %a4_out, align 16
  %881 = load <8 x float>* %a8_in, align 32
  %882 = load <8 x float>* %b8_in, align 32
  %883 = load <8 x i8>* %uch8_in, align 8
  %call568 = call <8 x float> @_Z6selectDv8_fS_Dv8_h(<8 x float> %881, <8 x float> %882, <8 x i8> %883) nounwind readnone
  store <8 x float> %call568, <8 x float>* %a8_out, align 32
  %884 = load <16 x float>* %a16_in, align 64
  %885 = load <16 x float>* %b16_in, align 64
  %886 = load <16 x i8>* %uch16_in, align 16
  %call569 = call <16 x float> @_Z6selectDv16_fS_Dv16_h(<16 x float> %884, <16 x float> %885, <16 x i8> %886) nounwind readnone
  store <16 x float> %call569, <16 x float>* %a16_out, align 64
  %887 = load float* %a_in, align 4
  %888 = load float* %b_in, align 4
  %889 = load i16* %s_in, align 2
  %call570 = call float @_Z6selectffs(float %887, float %888, i16 signext %889) nounwind readnone
  store float %call570, float* %a_out, align 4
  %890 = load <4 x float>* %a4_in, align 16
  %891 = load <4 x float>* %b4_in, align 16
  %892 = load <4 x i16>* %s4_in, align 8
  %call571 = call <4 x float> @_Z6selectDv4_fS_Dv4_s(<4 x float> %890, <4 x float> %891, <4 x i16> %892) nounwind readnone
  store <4 x float> %call571, <4 x float>* %a4_out, align 16
  %893 = load <8 x float>* %a8_in, align 32
  %894 = load <8 x float>* %b8_in, align 32
  %895 = load <8 x i16>* %s8_in, align 16
  %call572 = call <8 x float> @_Z6selectDv8_fS_Dv8_s(<8 x float> %893, <8 x float> %894, <8 x i16> %895) nounwind readnone
  store <8 x float> %call572, <8 x float>* %a8_out, align 32
  %896 = load <16 x float>* %a16_in, align 64
  %897 = load <16 x float>* %b16_in, align 64
  %898 = load <16 x i16>* %s16_in, align 32
  %call573 = call <16 x float> @_Z6selectDv16_fS_Dv16_s(<16 x float> %896, <16 x float> %897, <16 x i16> %898) nounwind readnone
  store <16 x float> %call573, <16 x float>* %a16_out, align 64
  %899 = load float* %a_in, align 4
  %900 = load float* %b_in, align 4
  %901 = load i16* %us_in, align 2
  %call574 = call float @_Z6selectfft(float %899, float %900, i16 zeroext %901) nounwind readnone
  store float %call574, float* %a_out, align 4
  %902 = load <4 x float>* %a4_in, align 16
  %903 = load <4 x float>* %b4_in, align 16
  %904 = load <4 x i16>* %us4_in, align 8
  %call575 = call <4 x float> @_Z6selectDv4_fS_Dv4_t(<4 x float> %902, <4 x float> %903, <4 x i16> %904) nounwind readnone
  store <4 x float> %call575, <4 x float>* %a4_out, align 16
  %905 = load <8 x float>* %a8_in, align 32
  %906 = load <8 x float>* %b8_in, align 32
  %907 = load <8 x i16>* %us8_in, align 16
  %call576 = call <8 x float> @_Z6selectDv8_fS_Dv8_t(<8 x float> %905, <8 x float> %906, <8 x i16> %907) nounwind readnone
  store <8 x float> %call576, <8 x float>* %a8_out, align 32
  %908 = load <16 x float>* %a16_in, align 64
  %909 = load <16 x float>* %b16_in, align 64
  %910 = load <16 x i16>* %us16_in, align 32
  %call577 = call <16 x float> @_Z6selectDv16_fS_Dv16_t(<16 x float> %908, <16 x float> %909, <16 x i16> %910) nounwind readnone
  store <16 x float> %call577, <16 x float>* %a16_out, align 64
  %911 = load float* %a_in, align 4
  %912 = load float* %b_in, align 4
  %913 = load i32* %i_in, align 4
  %call578 = call float @_Z6selectffi(float %911, float %912, i32 %913) nounwind readnone
  store float %call578, float* %a_out, align 4
  %914 = load <4 x float>* %a4_in, align 16
  %915 = load <4 x float>* %b4_in, align 16
  %916 = load <4 x i32>* %i4_in, align 16
  %call579 = call <4 x float> @_Z6selectDv4_fS_Dv4_i(<4 x float> %914, <4 x float> %915, <4 x i32> %916) nounwind readnone
  store <4 x float> %call579, <4 x float>* %a4_out, align 16
  %917 = load <8 x float>* %a8_in, align 32
  %918 = load <8 x float>* %b8_in, align 32
  %919 = load <8 x i32>* %i8_in, align 32
  %call580 = call <8 x float> @_Z6selectDv8_fS_Dv8_i(<8 x float> %917, <8 x float> %918, <8 x i32> %919) nounwind readnone
  store <8 x float> %call580, <8 x float>* %a8_out, align 32
  %920 = load <16 x float>* %a16_in, align 64
  %921 = load <16 x float>* %b16_in, align 64
  %922 = load <16 x i32>* %i16_in, align 64
  %call581 = call <16 x float> @_Z6selectDv16_fS_Dv16_i(<16 x float> %920, <16 x float> %921, <16 x i32> %922) nounwind readnone
  store <16 x float> %call581, <16 x float>* %a16_out, align 64
  %923 = load float* %a_in, align 4
  %924 = load float* %b_in, align 4
  %925 = load i32* %ui_in, align 4
  %call582 = call float @_Z6selectffj(float %923, float %924, i32 %925) nounwind readnone
  store float %call582, float* %a_out, align 4
  %926 = load <4 x float>* %a4_in, align 16
  %927 = load <4 x float>* %b4_in, align 16
  %928 = load <4 x i32>* %ui4_in, align 16
  %call583 = call <4 x float> @_Z6selectDv4_fS_Dv4_j(<4 x float> %926, <4 x float> %927, <4 x i32> %928) nounwind readnone
  store <4 x float> %call583, <4 x float>* %a4_out, align 16
  %929 = load <8 x float>* %a8_in, align 32
  %930 = load <8 x float>* %b8_in, align 32
  %931 = load <8 x i32>* %ui8_in, align 32
  %call584 = call <8 x float> @_Z6selectDv8_fS_Dv8_j(<8 x float> %929, <8 x float> %930, <8 x i32> %931) nounwind readnone
  store <8 x float> %call584, <8 x float>* %a8_out, align 32
  %932 = load <16 x float>* %a16_in, align 64
  %933 = load <16 x float>* %b16_in, align 64
  %934 = load <16 x i32>* %ui16_in, align 64
  %call585 = call <16 x float> @_Z6selectDv16_fS_Dv16_j(<16 x float> %932, <16 x float> %933, <16 x i32> %934) nounwind readnone
  store <16 x float> %call585, <16 x float>* %a16_out, align 64
  %935 = load float* %a_in, align 4
  %936 = load float* %b_in, align 4
  %937 = load i64* %l_in, align 8
  %call586 = call float @_Z6selectffl(float %935, float %936, i64 %937) nounwind readnone
  store float %call586, float* %a_out, align 4
  %938 = load <4 x float>* %a4_in, align 16
  %939 = load <4 x float>* %b4_in, align 16
  %940 = load <4 x i64>* %l4_in, align 32
  %call587 = call <4 x float> @_Z6selectDv4_fS_Dv4_l(<4 x float> %938, <4 x float> %939, <4 x i64> %940) nounwind readnone
  store <4 x float> %call587, <4 x float>* %a4_out, align 16
  %941 = load <8 x float>* %a8_in, align 32
  %942 = load <8 x float>* %b8_in, align 32
  %943 = load <8 x i64>* %l8_in, align 64
  %call588 = call <8 x float> @_Z6selectDv8_fS_Dv8_l(<8 x float> %941, <8 x float> %942, <8 x i64> %943) nounwind readnone
  store <8 x float> %call588, <8 x float>* %a8_out, align 32
  %944 = load <16 x float>* %a16_in, align 64
  %945 = load <16 x float>* %b16_in, align 64
  %946 = load <16 x i64>* %l16_in, align 128
  %call589 = call <16 x float> @_Z6selectDv16_fS_Dv16_l(<16 x float> %944, <16 x float> %945, <16 x i64> %946) nounwind readnone
  store <16 x float> %call589, <16 x float>* %a16_out, align 64
  %947 = load float* %a_in, align 4
  %948 = load float* %b_in, align 4
  %949 = load i64* %ul_in, align 8
  %call590 = call float @_Z6selectffm(float %947, float %948, i64 %949) nounwind readnone
  store float %call590, float* %a_out, align 4
  %950 = load <4 x float>* %a4_in, align 16
  %951 = load <4 x float>* %b4_in, align 16
  %952 = load <4 x i64>* %ul4_in, align 32
  %call591 = call <4 x float> @_Z6selectDv4_fS_Dv4_m(<4 x float> %950, <4 x float> %951, <4 x i64> %952) nounwind readnone
  store <4 x float> %call591, <4 x float>* %a4_out, align 16
  %953 = load <8 x float>* %a8_in, align 32
  %954 = load <8 x float>* %b8_in, align 32
  %955 = load <8 x i64>* %ul8_in, align 64
  %call592 = call <8 x float> @_Z6selectDv8_fS_Dv8_m(<8 x float> %953, <8 x float> %954, <8 x i64> %955) nounwind readnone
  store <8 x float> %call592, <8 x float>* %a8_out, align 32
  %956 = load <16 x float>* %a16_in, align 64
  %957 = load <16 x float>* %b16_in, align 64
  %958 = load <16 x i64>* %ul16_in, align 128
  %call593 = call <16 x float> @_Z6selectDv16_fS_Dv16_m(<16 x float> %956, <16 x float> %957, <16 x i64> %958) nounwind readnone
  store <16 x float> %call593, <16 x float>* %a16_out, align 64
  %959 = load float* %a_in, align 4
  %960 = load float* %b_in, align 4
  %call594 = call float @_Z9remainderff(float %959, float %960) nounwind readnone
  store float %call594, float* %a_out, align 4
  %961 = load <4 x float>* %a4_in, align 16
  %962 = load <4 x float>* %b4_in, align 16
  %call595 = call <4 x float> @_Z9remainderDv4_fS_(<4 x float> %961, <4 x float> %962) nounwind readnone
  store <4 x float> %call595, <4 x float>* %a4_out, align 16
  %963 = load <8 x float>* %a8_in, align 32
  %964 = load <8 x float>* %b8_in, align 32
  %call596 = call <8 x float> @_Z9remainderDv8_fS_(<8 x float> %963, <8 x float> %964) nounwind readnone
  store <8 x float> %call596, <8 x float>* %a8_out, align 32
  %965 = load <16 x float>* %a16_in, align 64
  %966 = load <16 x float>* %b16_in, align 64
  %call597 = call <16 x float> @_Z9remainderDv16_fS_(<16 x float> %965, <16 x float> %966) nounwind readnone
  store <16 x float> %call597, <16 x float>* %a16_out, align 64
  %967 = load float* %a_in, align 4
  %968 = load float* %b_in, align 4
  %call598 = call float @_Z6remquoffPi(float %967, float %968, i32* %i_out)
  store float %call598, float* %a_out, align 4
  %969 = load <2 x float>* %a2_in, align 8
  %970 = load <2 x float>* %b2_in, align 8
  %call599 = call <2 x float> @_Z6remquoDv2_fS_PDv2_i(<2 x float> %969, <2 x float> %970, <2 x i32>* %i2_out)
  store <2 x float> %call599, <2 x float>* %a2_out, align 8
  %971 = load <3 x float>* %a3_in, align 16
  %972 = load <3 x float>* %b3_in, align 16
  %call600 = call <3 x float> @_Z6remquoDv3_fS_PDv3_i(<3 x float> %971, <3 x float> %972, <3 x i32>* %i3_out)
  store <3 x float> %call600, <3 x float>* %a3_out, align 16
  %973 = load <4 x float>* %a4_in, align 16
  %974 = load <4 x float>* %b4_in, align 16
  %call601 = call <4 x float> @_Z6remquoDv4_fS_PDv4_i(<4 x float> %973, <4 x float> %974, <4 x i32>* %i4_out)
  store <4 x float> %call601, <4 x float>* %a4_out, align 16
  %975 = load <8 x float>* %a8_in, align 32
  %976 = load <8 x float>* %b8_in, align 32
  %call602 = call <8 x float> @_Z6remquoDv8_fS_PDv8_i(<8 x float> %975, <8 x float> %976, <8 x i32>* %i8_out)
  store <8 x float> %call602, <8 x float>* %a8_out, align 32
  %977 = load <16 x float>* %a16_in, align 64
  %978 = load <16 x float>* %b16_in, align 64
  %call603 = call <16 x float> @_Z6remquoDv16_fS_PDv16_i(<16 x float> %977, <16 x float> %978, <16 x i32>* %i16_out)
  store <16 x float> %call603, <16 x float>* %a16_out, align 64
  %979 = load <2 x float>* %a2_in, align 8
  %980 = load <2 x i32>* %ui2_in, align 8
  %call604 = call <2 x float> @_Z7shuffleDv2_fDv2_j(<2 x float> %979, <2 x i32> %980) nounwind readnone
  store <2 x float> %call604, <2 x float>* %a2_out, align 8
  %981 = load <4 x float>* %a4_in, align 16
  %982 = load <2 x i32>* %ui2_in, align 8
  %call605 = call <2 x float> @_Z7shuffleDv4_fDv2_j(<4 x float> %981, <2 x i32> %982) nounwind readnone
  store <2 x float> %call605, <2 x float>* %a2_out, align 8
  %983 = load <8 x float>* %a8_in, align 32
  %984 = load <2 x i32>* %ui2_in, align 8
  %call606 = call <2 x float> @_Z7shuffleDv8_fDv2_j(<8 x float> %983, <2 x i32> %984) nounwind readnone
  store <2 x float> %call606, <2 x float>* %a2_out, align 8
  %985 = load <16 x float>* %a16_in, align 64
  %986 = load <2 x i32>* %ui2_in, align 8
  %call607 = call <2 x float> @_Z7shuffleDv16_fDv2_j(<16 x float> %985, <2 x i32> %986) nounwind readnone
  store <2 x float> %call607, <2 x float>* %a2_out, align 8
  %987 = load <2 x float>* %a2_in, align 8
  %988 = load <4 x i32>* %ui4_in, align 16
  %call608 = call <4 x float> @_Z7shuffleDv2_fDv4_j(<2 x float> %987, <4 x i32> %988) nounwind readnone
  store <4 x float> %call608, <4 x float>* %a4_out, align 16
  %989 = load <4 x float>* %a4_in, align 16
  %990 = load <4 x i32>* %ui4_in, align 16
  %call609 = call <4 x float> @_Z7shuffleDv4_fDv4_j(<4 x float> %989, <4 x i32> %990) nounwind readnone
  store <4 x float> %call609, <4 x float>* %a4_out, align 16
  %991 = load <8 x float>* %a8_in, align 32
  %992 = load <4 x i32>* %ui4_in, align 16
  %call610 = call <4 x float> @_Z7shuffleDv8_fDv4_j(<8 x float> %991, <4 x i32> %992) nounwind readnone
  store <4 x float> %call610, <4 x float>* %a4_out, align 16
  %993 = load <16 x float>* %a16_in, align 64
  %994 = load <4 x i32>* %ui4_in, align 16
  %call611 = call <4 x float> @_Z7shuffleDv16_fDv4_j(<16 x float> %993, <4 x i32> %994) nounwind readnone
  store <4 x float> %call611, <4 x float>* %a4_out, align 16
  %995 = load <2 x float>* %a2_in, align 8
  %996 = load <8 x i32>* %ui8_in, align 32
  %call612 = call <8 x float> @_Z7shuffleDv2_fDv8_j(<2 x float> %995, <8 x i32> %996) nounwind readnone
  store <8 x float> %call612, <8 x float>* %a8_out, align 32
  %997 = load <4 x float>* %a4_in, align 16
  %998 = load <8 x i32>* %ui8_in, align 32
  %call613 = call <8 x float> @_Z7shuffleDv4_fDv8_j(<4 x float> %997, <8 x i32> %998) nounwind readnone
  store <8 x float> %call613, <8 x float>* %a8_out, align 32
  %999 = load <8 x float>* %a8_in, align 32
  %1000 = load <8 x i32>* %ui8_in, align 32
  %call614 = call <8 x float> @_Z7shuffleDv8_fDv8_j(<8 x float> %999, <8 x i32> %1000) nounwind readnone
  store <8 x float> %call614, <8 x float>* %a8_out, align 32
  %1001 = load <16 x float>* %a16_in, align 64
  %1002 = load <8 x i32>* %ui8_in, align 32
  %call615 = call <8 x float> @_Z7shuffleDv16_fDv8_j(<16 x float> %1001, <8 x i32> %1002) nounwind readnone
  store <8 x float> %call615, <8 x float>* %a8_out, align 32
  %1003 = load <2 x float>* %a2_in, align 8
  %1004 = load <16 x i32>* %ui16_in, align 64
  %call616 = call <16 x float> @_Z7shuffleDv2_fDv16_j(<2 x float> %1003, <16 x i32> %1004) nounwind readnone
  store <16 x float> %call616, <16 x float>* %a16_out, align 64
  %1005 = load <4 x float>* %a4_in, align 16
  %1006 = load <16 x i32>* %ui16_in, align 64
  %call617 = call <16 x float> @_Z7shuffleDv4_fDv16_j(<4 x float> %1005, <16 x i32> %1006) nounwind readnone
  store <16 x float> %call617, <16 x float>* %a16_out, align 64
  %1007 = load <8 x float>* %a8_in, align 32
  %1008 = load <16 x i32>* %ui16_in, align 64
  %call618 = call <16 x float> @_Z7shuffleDv8_fDv16_j(<8 x float> %1007, <16 x i32> %1008) nounwind readnone
  store <16 x float> %call618, <16 x float>* %a16_out, align 64
  %1009 = load <16 x float>* %a16_in, align 64
  %1010 = load <16 x i32>* %ui16_in, align 64
  %call619 = call <16 x float> @_Z7shuffleDv16_fDv16_j(<16 x float> %1009, <16 x i32> %1010) nounwind readnone
  store <16 x float> %call619, <16 x float>* %a16_out, align 64
  %1011 = load <2 x float>* %a2_in, align 8
  %1012 = load <2 x float>* %b2_in, align 8
  %1013 = load <2 x i32>* %ui2_in, align 8
  %call620 = call <2 x float> @_Z8shuffle2Dv2_fS_Dv2_j(<2 x float> %1011, <2 x float> %1012, <2 x i32> %1013) nounwind readnone
  store <2 x float> %call620, <2 x float>* %a2_out, align 8
  %1014 = load <4 x float>* %a4_in, align 16
  %1015 = load <4 x float>* %b4_in, align 16
  %1016 = load <2 x i32>* %ui2_in, align 8
  %call621 = call <2 x float> @_Z8shuffle2Dv4_fS_Dv2_j(<4 x float> %1014, <4 x float> %1015, <2 x i32> %1016) nounwind readnone
  store <2 x float> %call621, <2 x float>* %a2_out, align 8
  %1017 = load <8 x float>* %a8_in, align 32
  %1018 = load <8 x float>* %b8_in, align 32
  %1019 = load <2 x i32>* %ui2_in, align 8
  %call622 = call <2 x float> @_Z8shuffle2Dv8_fS_Dv2_j(<8 x float> %1017, <8 x float> %1018, <2 x i32> %1019) nounwind readnone
  store <2 x float> %call622, <2 x float>* %a2_out, align 8
  %1020 = load <16 x float>* %a16_in, align 64
  %1021 = load <16 x float>* %b16_in, align 64
  %1022 = load <2 x i32>* %ui2_in, align 8
  %call623 = call <2 x float> @_Z8shuffle2Dv16_fS_Dv2_j(<16 x float> %1020, <16 x float> %1021, <2 x i32> %1022) nounwind readnone
  store <2 x float> %call623, <2 x float>* %a2_out, align 8
  %1023 = load <2 x float>* %a2_in, align 8
  %1024 = load <2 x float>* %b2_in, align 8
  %1025 = load <4 x i32>* %ui4_in, align 16
  %call624 = call <4 x float> @_Z8shuffle2Dv2_fS_Dv4_j(<2 x float> %1023, <2 x float> %1024, <4 x i32> %1025) nounwind readnone
  store <4 x float> %call624, <4 x float>* %a4_out, align 16
  %1026 = load <4 x float>* %a4_in, align 16
  %1027 = load <4 x float>* %b4_in, align 16
  %1028 = load <4 x i32>* %ui4_in, align 16
  %call625 = call <4 x float> @_Z8shuffle2Dv4_fS_Dv4_j(<4 x float> %1026, <4 x float> %1027, <4 x i32> %1028) nounwind readnone
  store <4 x float> %call625, <4 x float>* %a4_out, align 16
  %1029 = load <8 x float>* %a8_in, align 32
  %1030 = load <8 x float>* %b8_in, align 32
  %1031 = load <4 x i32>* %ui4_in, align 16
  %call626 = call <4 x float> @_Z8shuffle2Dv8_fS_Dv4_j(<8 x float> %1029, <8 x float> %1030, <4 x i32> %1031) nounwind readnone
  store <4 x float> %call626, <4 x float>* %a4_out, align 16
  %1032 = load <16 x float>* %a16_in, align 64
  %1033 = load <16 x float>* %b16_in, align 64
  %1034 = load <4 x i32>* %ui4_in, align 16
  %call627 = call <4 x float> @_Z8shuffle2Dv16_fS_Dv4_j(<16 x float> %1032, <16 x float> %1033, <4 x i32> %1034) nounwind readnone
  store <4 x float> %call627, <4 x float>* %a4_out, align 16
  %1035 = load <2 x float>* %a2_in, align 8
  %1036 = load <2 x float>* %b2_in, align 8
  %1037 = load <8 x i32>* %ui8_in, align 32
  %call628 = call <8 x float> @_Z8shuffle2Dv2_fS_Dv8_j(<2 x float> %1035, <2 x float> %1036, <8 x i32> %1037) nounwind readnone
  store <8 x float> %call628, <8 x float>* %a8_out, align 32
  %1038 = load <4 x float>* %a4_in, align 16
  %1039 = load <4 x float>* %b4_in, align 16
  %1040 = load <8 x i32>* %ui8_in, align 32
  %call629 = call <8 x float> @_Z8shuffle2Dv4_fS_Dv8_j(<4 x float> %1038, <4 x float> %1039, <8 x i32> %1040) nounwind readnone
  store <8 x float> %call629, <8 x float>* %a8_out, align 32
  %1041 = load <8 x float>* %a8_in, align 32
  %1042 = load <8 x float>* %b8_in, align 32
  %1043 = load <8 x i32>* %ui8_in, align 32
  %call630 = call <8 x float> @_Z8shuffle2Dv8_fS_Dv8_j(<8 x float> %1041, <8 x float> %1042, <8 x i32> %1043) nounwind readnone
  store <8 x float> %call630, <8 x float>* %a8_out, align 32
  %1044 = load <16 x float>* %a16_in, align 64
  %1045 = load <16 x float>* %b16_in, align 64
  %1046 = load <8 x i32>* %ui8_in, align 32
  %call631 = call <8 x float> @_Z8shuffle2Dv16_fS_Dv8_j(<16 x float> %1044, <16 x float> %1045, <8 x i32> %1046) nounwind readnone
  store <8 x float> %call631, <8 x float>* %a8_out, align 32
  %1047 = load <2 x float>* %a2_in, align 8
  %1048 = load <2 x float>* %b2_in, align 8
  %1049 = load <16 x i32>* %ui16_in, align 64
  %call632 = call <16 x float> @_Z8shuffle2Dv2_fS_Dv16_j(<2 x float> %1047, <2 x float> %1048, <16 x i32> %1049) nounwind readnone
  store <16 x float> %call632, <16 x float>* %a16_out, align 64
  %1050 = load <4 x float>* %a4_in, align 16
  %1051 = load <4 x float>* %b4_in, align 16
  %1052 = load <16 x i32>* %ui16_in, align 64
  %call633 = call <16 x float> @_Z8shuffle2Dv4_fS_Dv16_j(<4 x float> %1050, <4 x float> %1051, <16 x i32> %1052) nounwind readnone
  store <16 x float> %call633, <16 x float>* %a16_out, align 64
  %1053 = load <8 x float>* %a8_in, align 32
  %1054 = load <8 x float>* %b8_in, align 32
  %1055 = load <16 x i32>* %ui16_in, align 64
  %call634 = call <16 x float> @_Z8shuffle2Dv8_fS_Dv16_j(<8 x float> %1053, <8 x float> %1054, <16 x i32> %1055) nounwind readnone
  store <16 x float> %call634, <16 x float>* %a16_out, align 64
  %1056 = load <16 x float>* %a16_in, align 64
  %1057 = load <16 x float>* %b16_in, align 64
  %1058 = load <16 x i32>* %ui16_in, align 64
  %call635 = call <16 x float> @_Z8shuffle2Dv16_fS_Dv16_j(<16 x float> %1056, <16 x float> %1057, <16 x i32> %1058) nounwind readnone
  store <16 x float> %call635, <16 x float>* %a16_out, align 64
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

declare <2 x float> @_Z7shuffleDv2_fDv2_j(<2 x float>, <2 x i32>) nounwind readnone

declare <2 x float> @_Z7shuffleDv4_fDv2_j(<4 x float>, <2 x i32>) nounwind readnone

declare <2 x float> @_Z7shuffleDv8_fDv2_j(<8 x float>, <2 x i32>) nounwind readnone

declare <2 x float> @_Z7shuffleDv16_fDv2_j(<16 x float>, <2 x i32>) nounwind readnone

declare <4 x float> @_Z7shuffleDv2_fDv4_j(<2 x float>, <4 x i32>) nounwind readnone

declare <4 x float> @_Z7shuffleDv4_fDv4_j(<4 x float>, <4 x i32>) nounwind readnone

declare <4 x float> @_Z7shuffleDv8_fDv4_j(<8 x float>, <4 x i32>) nounwind readnone

declare <4 x float> @_Z7shuffleDv16_fDv4_j(<16 x float>, <4 x i32>) nounwind readnone

declare <8 x float> @_Z7shuffleDv2_fDv8_j(<2 x float>, <8 x i32>) nounwind readnone

declare <8 x float> @_Z7shuffleDv4_fDv8_j(<4 x float>, <8 x i32>) nounwind readnone

declare <8 x float> @_Z7shuffleDv8_fDv8_j(<8 x float>, <8 x i32>) nounwind readnone

declare <8 x float> @_Z7shuffleDv16_fDv8_j(<16 x float>, <8 x i32>) nounwind readnone

declare <16 x float> @_Z7shuffleDv2_fDv16_j(<2 x float>, <16 x i32>) nounwind readnone

declare <16 x float> @_Z7shuffleDv4_fDv16_j(<4 x float>, <16 x i32>) nounwind readnone

declare <16 x float> @_Z7shuffleDv8_fDv16_j(<8 x float>, <16 x i32>) nounwind readnone

declare <16 x float> @_Z7shuffleDv16_fDv16_j(<16 x float>, <16 x i32>) nounwind readnone

declare <2 x float> @_Z8shuffle2Dv2_fS_Dv2_j(<2 x float>, <2 x float>, <2 x i32>) nounwind readnone

declare <2 x float> @_Z8shuffle2Dv4_fS_Dv2_j(<4 x float>, <4 x float>, <2 x i32>) nounwind readnone

declare <2 x float> @_Z8shuffle2Dv8_fS_Dv2_j(<8 x float>, <8 x float>, <2 x i32>) nounwind readnone

declare <2 x float> @_Z8shuffle2Dv16_fS_Dv2_j(<16 x float>, <16 x float>, <2 x i32>) nounwind readnone

declare <4 x float> @_Z8shuffle2Dv2_fS_Dv4_j(<2 x float>, <2 x float>, <4 x i32>) nounwind readnone

declare <4 x float> @_Z8shuffle2Dv4_fS_Dv4_j(<4 x float>, <4 x float>, <4 x i32>) nounwind readnone

declare <4 x float> @_Z8shuffle2Dv8_fS_Dv4_j(<8 x float>, <8 x float>, <4 x i32>) nounwind readnone

declare <4 x float> @_Z8shuffle2Dv16_fS_Dv4_j(<16 x float>, <16 x float>, <4 x i32>) nounwind readnone

declare <8 x float> @_Z8shuffle2Dv2_fS_Dv8_j(<2 x float>, <2 x float>, <8 x i32>) nounwind readnone

declare <8 x float> @_Z8shuffle2Dv4_fS_Dv8_j(<4 x float>, <4 x float>, <8 x i32>) nounwind readnone

declare <8 x float> @_Z8shuffle2Dv8_fS_Dv8_j(<8 x float>, <8 x float>, <8 x i32>) nounwind readnone

declare <8 x float> @_Z8shuffle2Dv16_fS_Dv8_j(<16 x float>, <16 x float>, <8 x i32>) nounwind readnone

declare <16 x float> @_Z8shuffle2Dv2_fS_Dv16_j(<2 x float>, <2 x float>, <16 x i32>) nounwind readnone

declare <16 x float> @_Z8shuffle2Dv4_fS_Dv16_j(<4 x float>, <4 x float>, <16 x i32>) nounwind readnone

declare <16 x float> @_Z8shuffle2Dv8_fS_Dv16_j(<8 x float>, <8 x float>, <16 x i32>) nounwind readnone

declare <16 x float> @_Z8shuffle2Dv16_fS_Dv16_j(<16 x float>, <16 x float>, <16 x i32>) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32)* @oclbuiltin, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
