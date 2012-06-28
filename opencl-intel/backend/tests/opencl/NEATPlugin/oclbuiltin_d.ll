; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN_XXX: NEATChecker -r %s -a %s.neat -t 0
; TODO: add NEATCHECKER instrumentation

; ModuleID = 'oclbuiltin_d.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @oclbuiltin(double addrspace(1)* %input, double addrspace(1)* %output, i32 addrspace(1)* %inputInt, i32 addrspace(1)* %outputInt, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca double addrspace(1)*, align 4
  %output.addr = alloca double addrspace(1)*, align 4
  %inputInt.addr = alloca i32 addrspace(1)*, align 4
  %outputInt.addr = alloca i32 addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %a_in = alloca double, align 8
  %a2_in = alloca <2 x double>, align 16
  %a3_in = alloca <3 x double>, align 32
  %a4_in = alloca <4 x double>, align 32
  %a8_in = alloca <8 x double>, align 64
  %a16_in = alloca <16 x double>, align 128
  %b_in = alloca double, align 8
  %b2_in = alloca <2 x double>, align 16
  %b3_in = alloca <3 x double>, align 32
  %b4_in = alloca <4 x double>, align 32
  %b8_in = alloca <8 x double>, align 64
  %b16_in = alloca <16 x double>, align 128
  %c_in = alloca double, align 8
  %c2_in = alloca <2 x double>, align 16
  %c3_in = alloca <3 x double>, align 32
  %c4_in = alloca <4 x double>, align 32
  %c8_in = alloca <8 x double>, align 64
  %c16_in = alloca <16 x double>, align 128
  %a_out = alloca double, align 8
  %a2_out = alloca <2 x double>, align 16
  %a3_out = alloca <3 x double>, align 32
  %a4_out = alloca <4 x double>, align 32
  %a8_out = alloca <8 x double>, align 64
  %a16_out = alloca <16 x double>, align 128
  %b_out = alloca double, align 8
  %b2_out = alloca <2 x double>, align 16
  %b3_out = alloca <3 x double>, align 32
  %b4_out = alloca <4 x double>, align 32
  %b8_out = alloca <8 x double>, align 64
  %b16_out = alloca <16 x double>, align 128
  %c_out = alloca double, align 8
  %c2_out = alloca <2 x double>, align 16
  %c3_out = alloca <3 x double>, align 32
  %c4_out = alloca <4 x double>, align 32
  %c8_out = alloca <8 x double>, align 64
  %c16_out = alloca <16 x double>, align 128
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
  store double addrspace(1)* %input, double addrspace(1)** %input.addr, align 4
  store double addrspace(1)* %output, double addrspace(1)** %output.addr, align 4
  store i32 addrspace(1)* %inputInt, i32 addrspace(1)** %inputInt.addr, align 4
  store i32 addrspace(1)* %outputInt, i32 addrspace(1)** %outputInt.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  store i32 0, i32* %tid, align 4
  %0 = load i32* %tid, align 4
  %1 = load double addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds double addrspace(1)* %1, i32 %0
  %2 = load double addrspace(1)* %arrayidx
  store double %2, double* %a_in, align 8
  %3 = load i32* %tid, align 4
  %4 = load double addrspace(1)** %input.addr, align 4
  %arrayidx1 = getelementptr inbounds double addrspace(1)* %4, i32 %3
  %5 = load double addrspace(1)* %arrayidx1
  %6 = insertelement <2 x double> undef, double %5, i32 0
  %splat = shufflevector <2 x double> %6, <2 x double> %6, <2 x i32> zeroinitializer
  store <2 x double> %splat, <2 x double>* %a2_in, align 16
  %7 = load i32* %tid, align 4
  %8 = load double addrspace(1)** %input.addr, align 4
  %arrayidx2 = getelementptr inbounds double addrspace(1)* %8, i32 %7
  %9 = load double addrspace(1)* %arrayidx2
  %10 = insertelement <3 x double> undef, double %9, i32 0
  %splat3 = shufflevector <3 x double> %10, <3 x double> %10, <3 x i32> zeroinitializer
  store <3 x double> %splat3, <3 x double>* %a3_in, align 32
  %11 = load i32* %tid, align 4
  %12 = load double addrspace(1)** %input.addr, align 4
  %arrayidx4 = getelementptr inbounds double addrspace(1)* %12, i32 %11
  %13 = load double addrspace(1)* %arrayidx4
  %14 = insertelement <4 x double> undef, double %13, i32 0
  %splat5 = shufflevector <4 x double> %14, <4 x double> %14, <4 x i32> zeroinitializer
  store <4 x double> %splat5, <4 x double>* %a4_in, align 32
  %15 = load i32* %tid, align 4
  %16 = load double addrspace(1)** %input.addr, align 4
  %arrayidx6 = getelementptr inbounds double addrspace(1)* %16, i32 %15
  %17 = load double addrspace(1)* %arrayidx6
  %18 = insertelement <8 x double> undef, double %17, i32 0
  %splat7 = shufflevector <8 x double> %18, <8 x double> %18, <8 x i32> zeroinitializer
  store <8 x double> %splat7, <8 x double>* %a8_in, align 64
  %19 = load i32* %tid, align 4
  %20 = load double addrspace(1)** %input.addr, align 4
  %arrayidx8 = getelementptr inbounds double addrspace(1)* %20, i32 %19
  %21 = load double addrspace(1)* %arrayidx8
  %22 = insertelement <16 x double> undef, double %21, i32 0
  %splat9 = shufflevector <16 x double> %22, <16 x double> %22, <16 x i32> zeroinitializer
  store <16 x double> %splat9, <16 x double>* %a16_in, align 128
  %23 = load i32* %tid, align 4
  %add = add i32 %23, 1
  %24 = load double addrspace(1)** %input.addr, align 4
  %arrayidx10 = getelementptr inbounds double addrspace(1)* %24, i32 %add
  %25 = load double addrspace(1)* %arrayidx10
  store double %25, double* %b_in, align 8
  %26 = load i32* %tid, align 4
  %27 = load double addrspace(1)** %input.addr, align 4
  %arrayidx11 = getelementptr inbounds double addrspace(1)* %27, i32 %26
  %28 = load double addrspace(1)* %arrayidx11
  %29 = insertelement <2 x double> undef, double %28, i32 0
  %splat12 = shufflevector <2 x double> %29, <2 x double> %29, <2 x i32> zeroinitializer
  store <2 x double> %splat12, <2 x double>* %b2_in, align 16
  %30 = load i32* %tid, align 4
  %31 = load double addrspace(1)** %input.addr, align 4
  %arrayidx13 = getelementptr inbounds double addrspace(1)* %31, i32 %30
  %32 = load double addrspace(1)* %arrayidx13
  %33 = insertelement <3 x double> undef, double %32, i32 0
  %splat14 = shufflevector <3 x double> %33, <3 x double> %33, <3 x i32> zeroinitializer
  store <3 x double> %splat14, <3 x double>* %b3_in, align 32
  %34 = load i32* %tid, align 4
  %35 = load double addrspace(1)** %input.addr, align 4
  %arrayidx15 = getelementptr inbounds double addrspace(1)* %35, i32 %34
  %36 = load double addrspace(1)* %arrayidx15
  %37 = insertelement <4 x double> undef, double %36, i32 0
  %splat16 = shufflevector <4 x double> %37, <4 x double> %37, <4 x i32> zeroinitializer
  store <4 x double> %splat16, <4 x double>* %b4_in, align 32
  %38 = load i32* %tid, align 4
  %39 = load double addrspace(1)** %input.addr, align 4
  %arrayidx17 = getelementptr inbounds double addrspace(1)* %39, i32 %38
  %40 = load double addrspace(1)* %arrayidx17
  %41 = insertelement <8 x double> undef, double %40, i32 0
  %splat18 = shufflevector <8 x double> %41, <8 x double> %41, <8 x i32> zeroinitializer
  store <8 x double> %splat18, <8 x double>* %b8_in, align 64
  %42 = load i32* %tid, align 4
  %43 = load double addrspace(1)** %input.addr, align 4
  %arrayidx19 = getelementptr inbounds double addrspace(1)* %43, i32 %42
  %44 = load double addrspace(1)* %arrayidx19
  %45 = insertelement <16 x double> undef, double %44, i32 0
  %splat20 = shufflevector <16 x double> %45, <16 x double> %45, <16 x i32> zeroinitializer
  store <16 x double> %splat20, <16 x double>* %b16_in, align 128
  %46 = load i32* %tid, align 4
  %add21 = add i32 %46, 1
  %47 = load double addrspace(1)** %input.addr, align 4
  %arrayidx22 = getelementptr inbounds double addrspace(1)* %47, i32 %add21
  %48 = load double addrspace(1)* %arrayidx22
  store double %48, double* %c_in, align 8
  %49 = load i32* %tid, align 4
  %50 = load double addrspace(1)** %input.addr, align 4
  %arrayidx23 = getelementptr inbounds double addrspace(1)* %50, i32 %49
  %51 = load double addrspace(1)* %arrayidx23
  %52 = insertelement <2 x double> undef, double %51, i32 0
  %splat24 = shufflevector <2 x double> %52, <2 x double> %52, <2 x i32> zeroinitializer
  store <2 x double> %splat24, <2 x double>* %c2_in, align 16
  %53 = load i32* %tid, align 4
  %54 = load double addrspace(1)** %input.addr, align 4
  %arrayidx25 = getelementptr inbounds double addrspace(1)* %54, i32 %53
  %55 = load double addrspace(1)* %arrayidx25
  %56 = insertelement <3 x double> undef, double %55, i32 0
  %splat26 = shufflevector <3 x double> %56, <3 x double> %56, <3 x i32> zeroinitializer
  store <3 x double> %splat26, <3 x double>* %c3_in, align 32
  %57 = load i32* %tid, align 4
  %58 = load double addrspace(1)** %input.addr, align 4
  %arrayidx27 = getelementptr inbounds double addrspace(1)* %58, i32 %57
  %59 = load double addrspace(1)* %arrayidx27
  %60 = insertelement <4 x double> undef, double %59, i32 0
  %splat28 = shufflevector <4 x double> %60, <4 x double> %60, <4 x i32> zeroinitializer
  store <4 x double> %splat28, <4 x double>* %c4_in, align 32
  %61 = load i32* %tid, align 4
  %62 = load double addrspace(1)** %input.addr, align 4
  %arrayidx29 = getelementptr inbounds double addrspace(1)* %62, i32 %61
  %63 = load double addrspace(1)* %arrayidx29
  %64 = insertelement <8 x double> undef, double %63, i32 0
  %splat30 = shufflevector <8 x double> %64, <8 x double> %64, <8 x i32> zeroinitializer
  store <8 x double> %splat30, <8 x double>* %c8_in, align 64
  %65 = load i32* %tid, align 4
  %66 = load double addrspace(1)** %input.addr, align 4
  %arrayidx31 = getelementptr inbounds double addrspace(1)* %66, i32 %65
  %67 = load double addrspace(1)* %arrayidx31
  %68 = insertelement <16 x double> undef, double %67, i32 0
  %splat32 = shufflevector <16 x double> %68, <16 x double> %68, <16 x i32> zeroinitializer
  store <16 x double> %splat32, <16 x double>* %c16_in, align 128
  %69 = load i32* %tid, align 4
  %70 = load double addrspace(1)** %output.addr, align 4
  %arrayidx33 = getelementptr inbounds double addrspace(1)* %70, i32 %69
  %71 = load double addrspace(1)* %arrayidx33
  store double %71, double* %a_out, align 8
  %72 = load i32* %tid, align 4
  %73 = load double addrspace(1)** %output.addr, align 4
  %arrayidx34 = getelementptr inbounds double addrspace(1)* %73, i32 %72
  %74 = load double addrspace(1)* %arrayidx34
  %75 = insertelement <2 x double> undef, double %74, i32 0
  %splat35 = shufflevector <2 x double> %75, <2 x double> %75, <2 x i32> zeroinitializer
  store <2 x double> %splat35, <2 x double>* %a2_out, align 16
  %76 = load i32* %tid, align 4
  %77 = load double addrspace(1)** %output.addr, align 4
  %arrayidx36 = getelementptr inbounds double addrspace(1)* %77, i32 %76
  %78 = load double addrspace(1)* %arrayidx36
  %79 = insertelement <3 x double> undef, double %78, i32 0
  %splat37 = shufflevector <3 x double> %79, <3 x double> %79, <3 x i32> zeroinitializer
  store <3 x double> %splat37, <3 x double>* %a3_out, align 32
  %80 = load i32* %tid, align 4
  %81 = load double addrspace(1)** %output.addr, align 4
  %arrayidx38 = getelementptr inbounds double addrspace(1)* %81, i32 %80
  %82 = load double addrspace(1)* %arrayidx38
  %83 = insertelement <4 x double> undef, double %82, i32 0
  %splat39 = shufflevector <4 x double> %83, <4 x double> %83, <4 x i32> zeroinitializer
  store <4 x double> %splat39, <4 x double>* %a4_out, align 32
  %84 = load i32* %tid, align 4
  %85 = load double addrspace(1)** %output.addr, align 4
  %arrayidx40 = getelementptr inbounds double addrspace(1)* %85, i32 %84
  %86 = load double addrspace(1)* %arrayidx40
  %87 = insertelement <8 x double> undef, double %86, i32 0
  %splat41 = shufflevector <8 x double> %87, <8 x double> %87, <8 x i32> zeroinitializer
  store <8 x double> %splat41, <8 x double>* %a8_out, align 64
  %88 = load i32* %tid, align 4
  %89 = load double addrspace(1)** %output.addr, align 4
  %arrayidx42 = getelementptr inbounds double addrspace(1)* %89, i32 %88
  %90 = load double addrspace(1)* %arrayidx42
  %91 = insertelement <16 x double> undef, double %90, i32 0
  %splat43 = shufflevector <16 x double> %91, <16 x double> %91, <16 x i32> zeroinitializer
  store <16 x double> %splat43, <16 x double>* %a16_out, align 128
  %92 = load i32* %tid, align 4
  %93 = load double addrspace(1)** %output.addr, align 4
  %arrayidx44 = getelementptr inbounds double addrspace(1)* %93, i32 %92
  %94 = load double addrspace(1)* %arrayidx44
  store double %94, double* %b_out, align 8
  %95 = load i32* %tid, align 4
  %96 = load double addrspace(1)** %output.addr, align 4
  %arrayidx45 = getelementptr inbounds double addrspace(1)* %96, i32 %95
  %97 = load double addrspace(1)* %arrayidx45
  %98 = insertelement <2 x double> undef, double %97, i32 0
  %splat46 = shufflevector <2 x double> %98, <2 x double> %98, <2 x i32> zeroinitializer
  store <2 x double> %splat46, <2 x double>* %b2_out, align 16
  %99 = load i32* %tid, align 4
  %100 = load double addrspace(1)** %output.addr, align 4
  %arrayidx47 = getelementptr inbounds double addrspace(1)* %100, i32 %99
  %101 = load double addrspace(1)* %arrayidx47
  %102 = insertelement <3 x double> undef, double %101, i32 0
  %splat48 = shufflevector <3 x double> %102, <3 x double> %102, <3 x i32> zeroinitializer
  store <3 x double> %splat48, <3 x double>* %b3_out, align 32
  %103 = load i32* %tid, align 4
  %104 = load double addrspace(1)** %output.addr, align 4
  %arrayidx49 = getelementptr inbounds double addrspace(1)* %104, i32 %103
  %105 = load double addrspace(1)* %arrayidx49
  %106 = insertelement <4 x double> undef, double %105, i32 0
  %splat50 = shufflevector <4 x double> %106, <4 x double> %106, <4 x i32> zeroinitializer
  store <4 x double> %splat50, <4 x double>* %b4_out, align 32
  %107 = load i32* %tid, align 4
  %108 = load double addrspace(1)** %output.addr, align 4
  %arrayidx51 = getelementptr inbounds double addrspace(1)* %108, i32 %107
  %109 = load double addrspace(1)* %arrayidx51
  %110 = insertelement <8 x double> undef, double %109, i32 0
  %splat52 = shufflevector <8 x double> %110, <8 x double> %110, <8 x i32> zeroinitializer
  store <8 x double> %splat52, <8 x double>* %b8_out, align 64
  %111 = load i32* %tid, align 4
  %112 = load double addrspace(1)** %output.addr, align 4
  %arrayidx53 = getelementptr inbounds double addrspace(1)* %112, i32 %111
  %113 = load double addrspace(1)* %arrayidx53
  %114 = insertelement <16 x double> undef, double %113, i32 0
  %splat54 = shufflevector <16 x double> %114, <16 x double> %114, <16 x i32> zeroinitializer
  store <16 x double> %splat54, <16 x double>* %b16_out, align 128
  %115 = load i32* %tid, align 4
  %116 = load double addrspace(1)** %output.addr, align 4
  %arrayidx55 = getelementptr inbounds double addrspace(1)* %116, i32 %115
  %117 = load double addrspace(1)* %arrayidx55
  store double %117, double* %c_out, align 8
  %118 = load i32* %tid, align 4
  %119 = load double addrspace(1)** %output.addr, align 4
  %arrayidx56 = getelementptr inbounds double addrspace(1)* %119, i32 %118
  %120 = load double addrspace(1)* %arrayidx56
  %121 = insertelement <2 x double> undef, double %120, i32 0
  %splat57 = shufflevector <2 x double> %121, <2 x double> %121, <2 x i32> zeroinitializer
  store <2 x double> %splat57, <2 x double>* %c2_out, align 16
  %122 = load i32* %tid, align 4
  %123 = load double addrspace(1)** %output.addr, align 4
  %arrayidx58 = getelementptr inbounds double addrspace(1)* %123, i32 %122
  %124 = load double addrspace(1)* %arrayidx58
  %125 = insertelement <3 x double> undef, double %124, i32 0
  %splat59 = shufflevector <3 x double> %125, <3 x double> %125, <3 x i32> zeroinitializer
  store <3 x double> %splat59, <3 x double>* %c3_out, align 32
  %126 = load i32* %tid, align 4
  %127 = load double addrspace(1)** %output.addr, align 4
  %arrayidx60 = getelementptr inbounds double addrspace(1)* %127, i32 %126
  %128 = load double addrspace(1)* %arrayidx60
  %129 = insertelement <4 x double> undef, double %128, i32 0
  %splat61 = shufflevector <4 x double> %129, <4 x double> %129, <4 x i32> zeroinitializer
  store <4 x double> %splat61, <4 x double>* %c4_out, align 32
  %130 = load i32* %tid, align 4
  %131 = load double addrspace(1)** %output.addr, align 4
  %arrayidx62 = getelementptr inbounds double addrspace(1)* %131, i32 %130
  %132 = load double addrspace(1)* %arrayidx62
  %133 = insertelement <8 x double> undef, double %132, i32 0
  %splat63 = shufflevector <8 x double> %133, <8 x double> %133, <8 x i32> zeroinitializer
  store <8 x double> %splat63, <8 x double>* %c8_out, align 64
  %134 = load i32* %tid, align 4
  %135 = load double addrspace(1)** %output.addr, align 4
  %arrayidx64 = getelementptr inbounds double addrspace(1)* %135, i32 %134
  %136 = load double addrspace(1)* %arrayidx64
  %137 = insertelement <16 x double> undef, double %136, i32 0
  %splat65 = shufflevector <16 x double> %137, <16 x double> %137, <16 x i32> zeroinitializer
  store <16 x double> %splat65, <16 x double>* %c16_out, align 128
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
  %184 = load double* %a_in, align 8
  %call = call double @_Z4acosd(double %184) nounwind readnone
  store double %call, double* %a_out, align 8
  %185 = load <4 x double>* %a4_in, align 32
  %call88 = call <4 x double> @_Z4acosDv4_d(<4 x double> %185) nounwind readnone
  store <4 x double> %call88, <4 x double>* %a4_out, align 32
  %186 = load <8 x double>* %a8_in, align 64
  %call89 = call <8 x double> @_Z4acosDv8_d(<8 x double> %186) nounwind readnone
  store <8 x double> %call89, <8 x double>* %a8_out, align 64
  %187 = load <16 x double>* %a16_in, align 128
  %call90 = call <16 x double> @_Z4acosDv16_d(<16 x double> %187) nounwind readnone
  store <16 x double> %call90, <16 x double>* %a16_out, align 128
  %188 = load double* %a_in, align 8
  %call91 = call double @_Z6acospid(double %188) nounwind readnone
  store double %call91, double* %a_out, align 8
  %189 = load <4 x double>* %a4_in, align 32
  %call92 = call <4 x double> @_Z6acospiDv4_d(<4 x double> %189) nounwind readnone
  store <4 x double> %call92, <4 x double>* %a4_out, align 32
  %190 = load <8 x double>* %a8_in, align 64
  %call93 = call <8 x double> @_Z6acospiDv8_d(<8 x double> %190) nounwind readnone
  store <8 x double> %call93, <8 x double>* %a8_out, align 64
  %191 = load <16 x double>* %a16_in, align 128
  %call94 = call <16 x double> @_Z6acospiDv16_d(<16 x double> %191) nounwind readnone
  store <16 x double> %call94, <16 x double>* %a16_out, align 128
  %192 = load double* %a_in, align 8
  %call95 = call double @_Z4asind(double %192) nounwind readnone
  store double %call95, double* %a_out, align 8
  %193 = load <4 x double>* %a4_in, align 32
  %call96 = call <4 x double> @_Z4asinDv4_d(<4 x double> %193) nounwind readnone
  store <4 x double> %call96, <4 x double>* %a4_out, align 32
  %194 = load <8 x double>* %a8_in, align 64
  %call97 = call <8 x double> @_Z4asinDv8_d(<8 x double> %194) nounwind readnone
  store <8 x double> %call97, <8 x double>* %a8_out, align 64
  %195 = load <16 x double>* %a16_in, align 128
  %call98 = call <16 x double> @_Z4asinDv16_d(<16 x double> %195) nounwind readnone
  store <16 x double> %call98, <16 x double>* %a16_out, align 128
  %196 = load double* %a_in, align 8
  %call99 = call double @_Z6asinpid(double %196) nounwind readnone
  store double %call99, double* %a_out, align 8
  %197 = load <4 x double>* %a4_in, align 32
  %call100 = call <4 x double> @_Z6asinpiDv4_d(<4 x double> %197) nounwind readnone
  store <4 x double> %call100, <4 x double>* %a4_out, align 32
  %198 = load <8 x double>* %a8_in, align 64
  %call101 = call <8 x double> @_Z6asinpiDv8_d(<8 x double> %198) nounwind readnone
  store <8 x double> %call101, <8 x double>* %a8_out, align 64
  %199 = load <16 x double>* %a16_in, align 128
  %call102 = call <16 x double> @_Z6asinpiDv16_d(<16 x double> %199) nounwind readnone
  store <16 x double> %call102, <16 x double>* %a16_out, align 128
  %200 = load double* %a_in, align 8
  %call103 = call double @_Z4atand(double %200) nounwind readnone
  store double %call103, double* %a_out, align 8
  %201 = load <4 x double>* %a4_in, align 32
  %call104 = call <4 x double> @_Z4atanDv4_d(<4 x double> %201) nounwind readnone
  store <4 x double> %call104, <4 x double>* %a4_out, align 32
  %202 = load <8 x double>* %a8_in, align 64
  %call105 = call <8 x double> @_Z4atanDv8_d(<8 x double> %202) nounwind readnone
  store <8 x double> %call105, <8 x double>* %a8_out, align 64
  %203 = load <16 x double>* %a16_in, align 128
  %call106 = call <16 x double> @_Z4atanDv16_d(<16 x double> %203) nounwind readnone
  store <16 x double> %call106, <16 x double>* %a16_out, align 128
  %204 = load double* %a_in, align 8
  %205 = load double* %b_in, align 8
  %call107 = call double @_Z5atan2dd(double %204, double %205) nounwind readnone
  store double %call107, double* %a_out, align 8
  %206 = load <4 x double>* %a4_in, align 32
  %207 = load <4 x double>* %b4_in, align 32
  %call108 = call <4 x double> @_Z5atan2Dv4_dS_(<4 x double> %206, <4 x double> %207) nounwind readnone
  store <4 x double> %call108, <4 x double>* %a4_out, align 32
  %208 = load <8 x double>* %a8_in, align 64
  %209 = load <8 x double>* %b8_in, align 64
  %call109 = call <8 x double> @_Z5atan2Dv8_dS_(<8 x double> %208, <8 x double> %209) nounwind readnone
  store <8 x double> %call109, <8 x double>* %a8_out, align 64
  %210 = load <16 x double>* %a16_in, align 128
  %211 = load <16 x double>* %b16_in, align 128
  %call110 = call <16 x double> @_Z5atan2Dv16_dS_(<16 x double> %210, <16 x double> %211) nounwind readnone
  store <16 x double> %call110, <16 x double>* %a16_out, align 128
  %212 = load double* %a_in, align 8
  %213 = load double* %b_in, align 8
  %call111 = call double @_Z7atan2pidd(double %212, double %213) nounwind readnone
  store double %call111, double* %a_out, align 8
  %214 = load <4 x double>* %a4_in, align 32
  %215 = load <4 x double>* %b4_in, align 32
  %call112 = call <4 x double> @_Z7atan2piDv4_dS_(<4 x double> %214, <4 x double> %215) nounwind readnone
  store <4 x double> %call112, <4 x double>* %a4_out, align 32
  %216 = load <8 x double>* %a8_in, align 64
  %217 = load <8 x double>* %b8_in, align 64
  %call113 = call <8 x double> @_Z7atan2piDv8_dS_(<8 x double> %216, <8 x double> %217) nounwind readnone
  store <8 x double> %call113, <8 x double>* %a8_out, align 64
  %218 = load <16 x double>* %a16_in, align 128
  %219 = load <16 x double>* %b16_in, align 128
  %call114 = call <16 x double> @_Z7atan2piDv16_dS_(<16 x double> %218, <16 x double> %219) nounwind readnone
  store <16 x double> %call114, <16 x double>* %a16_out, align 128
  %220 = load double* %a_in, align 8
  %call115 = call double @_Z6atanpid(double %220) nounwind readnone
  store double %call115, double* %a_out, align 8
  %221 = load <4 x double>* %a4_in, align 32
  %call116 = call <4 x double> @_Z6atanpiDv4_d(<4 x double> %221) nounwind readnone
  store <4 x double> %call116, <4 x double>* %a4_out, align 32
  %222 = load <8 x double>* %a8_in, align 64
  %call117 = call <8 x double> @_Z6atanpiDv8_d(<8 x double> %222) nounwind readnone
  store <8 x double> %call117, <8 x double>* %a8_out, align 64
  %223 = load <16 x double>* %a16_in, align 128
  %call118 = call <16 x double> @_Z6atanpiDv16_d(<16 x double> %223) nounwind readnone
  store <16 x double> %call118, <16 x double>* %a16_out, align 128
  %224 = load double* %a_in, align 8
  %call119 = call double @_Z3cosd(double %224) nounwind readnone
  store double %call119, double* %a_out, align 8
  %225 = load <4 x double>* %a4_in, align 32
  %call120 = call <4 x double> @_Z3cosDv4_d(<4 x double> %225) nounwind readnone
  store <4 x double> %call120, <4 x double>* %a4_out, align 32
  %226 = load <8 x double>* %a8_in, align 64
  %call121 = call <8 x double> @_Z3cosDv8_d(<8 x double> %226) nounwind readnone
  store <8 x double> %call121, <8 x double>* %a8_out, align 64
  %227 = load <16 x double>* %a16_in, align 128
  %call122 = call <16 x double> @_Z3cosDv16_d(<16 x double> %227) nounwind readnone
  store <16 x double> %call122, <16 x double>* %a16_out, align 128
  %228 = load double* %a_in, align 8
  %call123 = call double @_Z4coshd(double %228) nounwind readnone
  store double %call123, double* %a_out, align 8
  %229 = load <4 x double>* %a4_in, align 32
  %call124 = call <4 x double> @_Z4coshDv4_d(<4 x double> %229) nounwind readnone
  store <4 x double> %call124, <4 x double>* %a4_out, align 32
  %230 = load <8 x double>* %a8_in, align 64
  %call125 = call <8 x double> @_Z4coshDv8_d(<8 x double> %230) nounwind readnone
  store <8 x double> %call125, <8 x double>* %a8_out, align 64
  %231 = load <16 x double>* %a16_in, align 128
  %call126 = call <16 x double> @_Z4coshDv16_d(<16 x double> %231) nounwind readnone
  store <16 x double> %call126, <16 x double>* %a16_out, align 128
  %232 = load double* %a_in, align 8
  %call127 = call double @_Z5cospid(double %232) nounwind readnone
  store double %call127, double* %a_out, align 8
  %233 = load <4 x double>* %a4_in, align 32
  %call128 = call <4 x double> @_Z5cospiDv4_d(<4 x double> %233) nounwind readnone
  store <4 x double> %call128, <4 x double>* %a4_out, align 32
  %234 = load <8 x double>* %a8_in, align 64
  %call129 = call <8 x double> @_Z5cospiDv8_d(<8 x double> %234) nounwind readnone
  store <8 x double> %call129, <8 x double>* %a8_out, align 64
  %235 = load <16 x double>* %a16_in, align 128
  %call130 = call <16 x double> @_Z5cospiDv16_d(<16 x double> %235) nounwind readnone
  store <16 x double> %call130, <16 x double>* %a16_out, align 128
  %236 = load double* %a_in, align 8
  %call131 = call double @_Z3expd(double %236) nounwind readnone
  store double %call131, double* %a_out, align 8
  %237 = load <4 x double>* %a4_in, align 32
  %call132 = call <4 x double> @_Z3expDv4_d(<4 x double> %237) nounwind readnone
  store <4 x double> %call132, <4 x double>* %a4_out, align 32
  %238 = load <8 x double>* %a8_in, align 64
  %call133 = call <8 x double> @_Z3expDv8_d(<8 x double> %238) nounwind readnone
  store <8 x double> %call133, <8 x double>* %a8_out, align 64
  %239 = load <16 x double>* %a16_in, align 128
  %call134 = call <16 x double> @_Z3expDv16_d(<16 x double> %239) nounwind readnone
  store <16 x double> %call134, <16 x double>* %a16_out, align 128
  %240 = load double* %a_in, align 8
  %call135 = call double @_Z4exp2d(double %240) nounwind readnone
  store double %call135, double* %a_out, align 8
  %241 = load <4 x double>* %a4_in, align 32
  %call136 = call <4 x double> @_Z4exp2Dv4_d(<4 x double> %241) nounwind readnone
  store <4 x double> %call136, <4 x double>* %a4_out, align 32
  %242 = load <8 x double>* %a8_in, align 64
  %call137 = call <8 x double> @_Z4exp2Dv8_d(<8 x double> %242) nounwind readnone
  store <8 x double> %call137, <8 x double>* %a8_out, align 64
  %243 = load <16 x double>* %a16_in, align 128
  %call138 = call <16 x double> @_Z4exp2Dv16_d(<16 x double> %243) nounwind readnone
  store <16 x double> %call138, <16 x double>* %a16_out, align 128
  %244 = load double* %a_in, align 8
  %call139 = call double @_Z5exp10d(double %244) nounwind readnone
  store double %call139, double* %a_out, align 8
  %245 = load <4 x double>* %a4_in, align 32
  %call140 = call <4 x double> @_Z5exp10Dv4_d(<4 x double> %245) nounwind readnone
  store <4 x double> %call140, <4 x double>* %a4_out, align 32
  %246 = load <8 x double>* %a8_in, align 64
  %call141 = call <8 x double> @_Z5exp10Dv8_d(<8 x double> %246) nounwind readnone
  store <8 x double> %call141, <8 x double>* %a8_out, align 64
  %247 = load <16 x double>* %a16_in, align 128
  %call142 = call <16 x double> @_Z5exp10Dv16_d(<16 x double> %247) nounwind readnone
  store <16 x double> %call142, <16 x double>* %a16_out, align 128
  %248 = load double* %a_in, align 8
  %call143 = call double @_Z5expm1d(double %248) nounwind readnone
  store double %call143, double* %a_out, align 8
  %249 = load <4 x double>* %a4_in, align 32
  %call144 = call <4 x double> @_Z5expm1Dv4_d(<4 x double> %249) nounwind readnone
  store <4 x double> %call144, <4 x double>* %a4_out, align 32
  %250 = load <8 x double>* %a8_in, align 64
  %call145 = call <8 x double> @_Z5expm1Dv8_d(<8 x double> %250) nounwind readnone
  store <8 x double> %call145, <8 x double>* %a8_out, align 64
  %251 = load <16 x double>* %a16_in, align 128
  %call146 = call <16 x double> @_Z5expm1Dv16_d(<16 x double> %251) nounwind readnone
  store <16 x double> %call146, <16 x double>* %a16_out, align 128
  %252 = load double* %a_in, align 8
  %call147 = call double @_Z3logd(double %252) nounwind readnone
  store double %call147, double* %a_out, align 8
  %253 = load <4 x double>* %a4_in, align 32
  %call148 = call <4 x double> @_Z3logDv4_d(<4 x double> %253) nounwind readnone
  store <4 x double> %call148, <4 x double>* %a4_out, align 32
  %254 = load <8 x double>* %a8_in, align 64
  %call149 = call <8 x double> @_Z3logDv8_d(<8 x double> %254) nounwind readnone
  store <8 x double> %call149, <8 x double>* %a8_out, align 64
  %255 = load <16 x double>* %a16_in, align 128
  %call150 = call <16 x double> @_Z3logDv16_d(<16 x double> %255) nounwind readnone
  store <16 x double> %call150, <16 x double>* %a16_out, align 128
  %256 = load double* %a_in, align 8
  %call151 = call double @_Z4log2d(double %256) nounwind readnone
  store double %call151, double* %a_out, align 8
  %257 = load <4 x double>* %a4_in, align 32
  %call152 = call <4 x double> @_Z4log2Dv4_d(<4 x double> %257) nounwind readnone
  store <4 x double> %call152, <4 x double>* %a4_out, align 32
  %258 = load <8 x double>* %a8_in, align 64
  %call153 = call <8 x double> @_Z4log2Dv8_d(<8 x double> %258) nounwind readnone
  store <8 x double> %call153, <8 x double>* %a8_out, align 64
  %259 = load <16 x double>* %a16_in, align 128
  %call154 = call <16 x double> @_Z4log2Dv16_d(<16 x double> %259) nounwind readnone
  store <16 x double> %call154, <16 x double>* %a16_out, align 128
  %260 = load double* %a_in, align 8
  %call155 = call double @_Z5log10d(double %260) nounwind readnone
  store double %call155, double* %a_out, align 8
  %261 = load <4 x double>* %a4_in, align 32
  %call156 = call <4 x double> @_Z5log10Dv4_d(<4 x double> %261) nounwind readnone
  store <4 x double> %call156, <4 x double>* %a4_out, align 32
  %262 = load <8 x double>* %a8_in, align 64
  %call157 = call <8 x double> @_Z5log10Dv8_d(<8 x double> %262) nounwind readnone
  store <8 x double> %call157, <8 x double>* %a8_out, align 64
  %263 = load <16 x double>* %a16_in, align 128
  %call158 = call <16 x double> @_Z5log10Dv16_d(<16 x double> %263) nounwind readnone
  store <16 x double> %call158, <16 x double>* %a16_out, align 128
  %264 = load double* %a_in, align 8
  %call159 = call double @_Z5log1pd(double %264) nounwind readnone
  store double %call159, double* %a_out, align 8
  %265 = load <4 x double>* %a4_in, align 32
  %call160 = call <4 x double> @_Z5log1pDv4_d(<4 x double> %265) nounwind readnone
  store <4 x double> %call160, <4 x double>* %a4_out, align 32
  %266 = load <8 x double>* %a8_in, align 64
  %call161 = call <8 x double> @_Z5log1pDv8_d(<8 x double> %266) nounwind readnone
  store <8 x double> %call161, <8 x double>* %a8_out, align 64
  %267 = load <16 x double>* %a16_in, align 128
  %call162 = call <16 x double> @_Z5log1pDv16_d(<16 x double> %267) nounwind readnone
  store <16 x double> %call162, <16 x double>* %a16_out, align 128
  %268 = load double* %a_in, align 8
  %call163 = call double @_Z4logbd(double %268) nounwind readnone
  store double %call163, double* %a_out, align 8
  %269 = load <4 x double>* %a4_in, align 32
  %call164 = call <4 x double> @_Z4logbDv4_d(<4 x double> %269) nounwind readnone
  store <4 x double> %call164, <4 x double>* %a4_out, align 32
  %270 = load <8 x double>* %a8_in, align 64
  %call165 = call <8 x double> @_Z4logbDv8_d(<8 x double> %270) nounwind readnone
  store <8 x double> %call165, <8 x double>* %a8_out, align 64
  %271 = load <16 x double>* %a16_in, align 128
  %call166 = call <16 x double> @_Z4logbDv16_d(<16 x double> %271) nounwind readnone
  store <16 x double> %call166, <16 x double>* %a16_out, align 128
  %272 = load double* %a_in, align 8
  %call167 = call double @_Z4ceild(double %272) nounwind readnone
  store double %call167, double* %a_out, align 8
  %273 = load <4 x double>* %a4_in, align 32
  %call168 = call <4 x double> @_Z4ceilDv4_d(<4 x double> %273) nounwind readnone
  store <4 x double> %call168, <4 x double>* %a4_out, align 32
  %274 = load <8 x double>* %a8_in, align 64
  %call169 = call <8 x double> @_Z4ceilDv8_d(<8 x double> %274) nounwind readnone
  store <8 x double> %call169, <8 x double>* %a8_out, align 64
  %275 = load <16 x double>* %a16_in, align 128
  %call170 = call <16 x double> @_Z4ceilDv16_d(<16 x double> %275) nounwind readnone
  store <16 x double> %call170, <16 x double>* %a16_out, align 128
  %276 = load double* %a_in, align 8
  %277 = load double* %b_in, align 8
  %call171 = call double @_Z3powdd(double %276, double %277) nounwind readnone
  store double %call171, double* %a_out, align 8
  %278 = load <4 x double>* %a4_in, align 32
  %279 = load <4 x double>* %b4_in, align 32
  %call172 = call <4 x double> @_Z3powDv4_dS_(<4 x double> %278, <4 x double> %279) nounwind readnone
  store <4 x double> %call172, <4 x double>* %a4_out, align 32
  %280 = load <8 x double>* %a8_in, align 64
  %281 = load <8 x double>* %b8_in, align 64
  %call173 = call <8 x double> @_Z3powDv8_dS_(<8 x double> %280, <8 x double> %281) nounwind readnone
  store <8 x double> %call173, <8 x double>* %a8_out, align 64
  %282 = load <16 x double>* %a16_in, align 128
  %283 = load <16 x double>* %b16_in, align 128
  %call174 = call <16 x double> @_Z3powDv16_dS_(<16 x double> %282, <16 x double> %283) nounwind readnone
  store <16 x double> %call174, <16 x double>* %a16_out, align 128
  %284 = load double* %a_in, align 8
  %285 = load double* %b_in, align 8
  %286 = load double* %c_in, align 8
  %call175 = call double @_Z5clampddd(double %284, double %285, double %286) nounwind readnone
  store double %call175, double* %a_out, align 8
  %287 = load <4 x double>* %a4_in, align 32
  %288 = load <4 x double>* %b4_in, align 32
  %289 = load <4 x double>* %c4_in, align 32
  %call176 = call <4 x double> @_Z5clampDv4_dS_S_(<4 x double> %287, <4 x double> %288, <4 x double> %289) nounwind readnone
  store <4 x double> %call176, <4 x double>* %a4_out, align 32
  %290 = load <8 x double>* %a8_in, align 64
  %291 = load <8 x double>* %b8_in, align 64
  %292 = load <8 x double>* %c8_in, align 64
  %call177 = call <8 x double> @_Z5clampDv8_dS_S_(<8 x double> %290, <8 x double> %291, <8 x double> %292) nounwind readnone
  store <8 x double> %call177, <8 x double>* %a8_out, align 64
  %293 = load <16 x double>* %a16_in, align 128
  %294 = load <16 x double>* %b16_in, align 128
  %295 = load <16 x double>* %c16_in, align 128
  %call178 = call <16 x double> @_Z5clampDv16_dS_S_(<16 x double> %293, <16 x double> %294, <16 x double> %295) nounwind readnone
  store <16 x double> %call178, <16 x double>* %a16_out, align 128
  %296 = load double* %a_in, align 8
  %297 = load double* %b_in, align 8
  %298 = load double* %c_in, align 8
  %call179 = call double @_Z5clampddd(double %296, double %297, double %298) nounwind readnone
  store double %call179, double* %a_out, align 8
  %299 = load <4 x double>* %a4_in, align 32
  %300 = load double* %b_in, align 8
  %301 = load double* %c_in, align 8
  %call180 = call <4 x double> @_Z5clampDv4_ddd(<4 x double> %299, double %300, double %301) nounwind readnone
  store <4 x double> %call180, <4 x double>* %a4_out, align 32
  %302 = load <8 x double>* %a8_in, align 64
  %303 = load double* %b_in, align 8
  %304 = load double* %c_in, align 8
  %call181 = call <8 x double> @_Z5clampDv8_ddd(<8 x double> %302, double %303, double %304) nounwind readnone
  store <8 x double> %call181, <8 x double>* %a8_out, align 64
  %305 = load <16 x double>* %a16_in, align 128
  %306 = load double* %b_in, align 8
  %307 = load double* %c_in, align 8
  %call182 = call <16 x double> @_Z5clampDv16_ddd(<16 x double> %305, double %306, double %307) nounwind readnone
  store <16 x double> %call182, <16 x double>* %a16_out, align 128
  %308 = load double* %a_in, align 8
  %call183 = call double @_Z4sinhd(double %308) nounwind readnone
  store double %call183, double* %a_out, align 8
  %309 = load <4 x double>* %a4_in, align 32
  %call184 = call <4 x double> @_Z4sinhDv4_d(<4 x double> %309) nounwind readnone
  store <4 x double> %call184, <4 x double>* %a4_out, align 32
  %310 = load <8 x double>* %a8_in, align 64
  %call185 = call <8 x double> @_Z4sinhDv8_d(<8 x double> %310) nounwind readnone
  store <8 x double> %call185, <8 x double>* %a8_out, align 64
  %311 = load <16 x double>* %a16_in, align 128
  %call186 = call <16 x double> @_Z4sinhDv16_d(<16 x double> %311) nounwind readnone
  store <16 x double> %call186, <16 x double>* %a16_out, align 128
  %312 = load double* %a_in, align 8
  %call187 = call double @_Z3sind(double %312) nounwind readnone
  store double %call187, double* %a_out, align 8
  %313 = load <4 x double>* %a4_in, align 32
  %call188 = call <4 x double> @_Z3sinDv4_d(<4 x double> %313) nounwind readnone
  store <4 x double> %call188, <4 x double>* %a4_out, align 32
  %314 = load <8 x double>* %a8_in, align 64
  %call189 = call <8 x double> @_Z3sinDv8_d(<8 x double> %314) nounwind readnone
  store <8 x double> %call189, <8 x double>* %a8_out, align 64
  %315 = load <16 x double>* %a16_in, align 128
  %call190 = call <16 x double> @_Z3sinDv16_d(<16 x double> %315) nounwind readnone
  store <16 x double> %call190, <16 x double>* %a16_out, align 128
  %316 = load double* %a_in, align 8
  %call191 = call double @_Z5sinpid(double %316) nounwind readnone
  store double %call191, double* %a_out, align 8
  %317 = load <4 x double>* %a4_in, align 32
  %call192 = call <4 x double> @_Z5sinpiDv4_d(<4 x double> %317) nounwind readnone
  store <4 x double> %call192, <4 x double>* %a4_out, align 32
  %318 = load <8 x double>* %a8_in, align 64
  %call193 = call <8 x double> @_Z5sinpiDv8_d(<8 x double> %318) nounwind readnone
  store <8 x double> %call193, <8 x double>* %a8_out, align 64
  %319 = load <16 x double>* %a16_in, align 128
  %call194 = call <16 x double> @_Z5sinpiDv16_d(<16 x double> %319) nounwind readnone
  store <16 x double> %call194, <16 x double>* %a16_out, align 128
  %320 = load double* %a_in, align 8
  %call195 = call double @_Z4sqrtd(double %320) nounwind readnone
  store double %call195, double* %a_out, align 8
  %321 = load <4 x double>* %a4_in, align 32
  %call196 = call <4 x double> @_Z4sqrtDv4_d(<4 x double> %321) nounwind readnone
  store <4 x double> %call196, <4 x double>* %a4_out, align 32
  %322 = load <8 x double>* %a8_in, align 64
  %call197 = call <8 x double> @_Z4sqrtDv8_d(<8 x double> %322) nounwind readnone
  store <8 x double> %call197, <8 x double>* %a8_out, align 64
  %323 = load <16 x double>* %a16_in, align 128
  %call198 = call <16 x double> @_Z4sqrtDv16_d(<16 x double> %323) nounwind readnone
  store <16 x double> %call198, <16 x double>* %a16_out, align 128
  %324 = load double* %a_in, align 8
  %call199 = call double @_Z5rsqrtd(double %324) nounwind readnone
  store double %call199, double* %a_out, align 8
  %325 = load <4 x double>* %a4_in, align 32
  %call200 = call <4 x double> @_Z5rsqrtDv4_d(<4 x double> %325) nounwind readnone
  store <4 x double> %call200, <4 x double>* %a4_out, align 32
  %326 = load <8 x double>* %a8_in, align 64
  %call201 = call <8 x double> @_Z5rsqrtDv8_d(<8 x double> %326) nounwind readnone
  store <8 x double> %call201, <8 x double>* %a8_out, align 64
  %327 = load <16 x double>* %a16_in, align 128
  %call202 = call <16 x double> @_Z5rsqrtDv16_d(<16 x double> %327) nounwind readnone
  store <16 x double> %call202, <16 x double>* %a16_out, align 128
  %328 = load double* %a_in, align 8
  %call203 = call double @_Z3tand(double %328) nounwind readnone
  store double %call203, double* %a_out, align 8
  %329 = load <4 x double>* %a4_in, align 32
  %call204 = call <4 x double> @_Z3tanDv4_d(<4 x double> %329) nounwind readnone
  store <4 x double> %call204, <4 x double>* %a4_out, align 32
  %330 = load <8 x double>* %a8_in, align 64
  %call205 = call <8 x double> @_Z3tanDv8_d(<8 x double> %330) nounwind readnone
  store <8 x double> %call205, <8 x double>* %a8_out, align 64
  %331 = load <16 x double>* %a16_in, align 128
  %call206 = call <16 x double> @_Z3tanDv16_d(<16 x double> %331) nounwind readnone
  store <16 x double> %call206, <16 x double>* %a16_out, align 128
  %332 = load double* %a_in, align 8
  %call207 = call double @_Z4tanhd(double %332) nounwind readnone
  store double %call207, double* %a_out, align 8
  %333 = load <4 x double>* %a4_in, align 32
  %call208 = call <4 x double> @_Z4tanhDv4_d(<4 x double> %333) nounwind readnone
  store <4 x double> %call208, <4 x double>* %a4_out, align 32
  %334 = load <8 x double>* %a8_in, align 64
  %call209 = call <8 x double> @_Z4tanhDv8_d(<8 x double> %334) nounwind readnone
  store <8 x double> %call209, <8 x double>* %a8_out, align 64
  %335 = load <16 x double>* %a16_in, align 128
  %call210 = call <16 x double> @_Z4tanhDv16_d(<16 x double> %335) nounwind readnone
  store <16 x double> %call210, <16 x double>* %a16_out, align 128
  %336 = load double* %a_in, align 8
  %call211 = call double @_Z5tanpid(double %336) nounwind readnone
  store double %call211, double* %a_out, align 8
  %337 = load <4 x double>* %a4_in, align 32
  %call212 = call <4 x double> @_Z5tanpiDv4_d(<4 x double> %337) nounwind readnone
  store <4 x double> %call212, <4 x double>* %a4_out, align 32
  %338 = load <8 x double>* %a8_in, align 64
  %call213 = call <8 x double> @_Z5tanpiDv8_d(<8 x double> %338) nounwind readnone
  store <8 x double> %call213, <8 x double>* %a8_out, align 64
  %339 = load <16 x double>* %a16_in, align 128
  %call214 = call <16 x double> @_Z5tanpiDv16_d(<16 x double> %339) nounwind readnone
  store <16 x double> %call214, <16 x double>* %a16_out, align 128
  %340 = load double* %a_in, align 8
  %call215 = call double @_Z4fabsd(double %340) nounwind readnone
  store double %call215, double* %a_out, align 8
  %341 = load <4 x double>* %a4_in, align 32
  %call216 = call <4 x double> @_Z4fabsDv4_d(<4 x double> %341) nounwind readnone
  store <4 x double> %call216, <4 x double>* %a4_out, align 32
  %342 = load <8 x double>* %a8_in, align 64
  %call217 = call <8 x double> @_Z4fabsDv8_d(<8 x double> %342) nounwind readnone
  store <8 x double> %call217, <8 x double>* %a8_out, align 64
  %343 = load <16 x double>* %a16_in, align 128
  %call218 = call <16 x double> @_Z4fabsDv16_d(<16 x double> %343) nounwind readnone
  store <16 x double> %call218, <16 x double>* %a16_out, align 128
  %344 = load double* %a_in, align 8
  %call219 = call double @_Z5asinhd(double %344) nounwind readnone
  store double %call219, double* %a_out, align 8
  %345 = load <4 x double>* %a4_in, align 32
  %call220 = call <4 x double> @_Z5asinhDv4_d(<4 x double> %345) nounwind readnone
  store <4 x double> %call220, <4 x double>* %a4_out, align 32
  %346 = load <8 x double>* %a8_in, align 64
  %call221 = call <8 x double> @_Z5asinhDv8_d(<8 x double> %346) nounwind readnone
  store <8 x double> %call221, <8 x double>* %a8_out, align 64
  %347 = load <16 x double>* %a16_in, align 128
  %call222 = call <16 x double> @_Z5asinhDv16_d(<16 x double> %347) nounwind readnone
  store <16 x double> %call222, <16 x double>* %a16_out, align 128
  %348 = load double* %a_in, align 8
  %call223 = call double @_Z5acoshd(double %348) nounwind readnone
  store double %call223, double* %a_out, align 8
  %349 = load <4 x double>* %a4_in, align 32
  %call224 = call <4 x double> @_Z5acoshDv4_d(<4 x double> %349) nounwind readnone
  store <4 x double> %call224, <4 x double>* %a4_out, align 32
  %350 = load <8 x double>* %a8_in, align 64
  %call225 = call <8 x double> @_Z5acoshDv8_d(<8 x double> %350) nounwind readnone
  store <8 x double> %call225, <8 x double>* %a8_out, align 64
  %351 = load <16 x double>* %a16_in, align 128
  %call226 = call <16 x double> @_Z5acoshDv16_d(<16 x double> %351) nounwind readnone
  store <16 x double> %call226, <16 x double>* %a16_out, align 128
  %352 = load double* %a_in, align 8
  %call227 = call double @_Z5atanhd(double %352) nounwind readnone
  store double %call227, double* %a_out, align 8
  %353 = load <4 x double>* %a4_in, align 32
  %call228 = call <4 x double> @_Z5atanhDv4_d(<4 x double> %353) nounwind readnone
  store <4 x double> %call228, <4 x double>* %a4_out, align 32
  %354 = load <8 x double>* %a8_in, align 64
  %call229 = call <8 x double> @_Z5atanhDv8_d(<8 x double> %354) nounwind readnone
  store <8 x double> %call229, <8 x double>* %a8_out, align 64
  %355 = load <16 x double>* %a16_in, align 128
  %call230 = call <16 x double> @_Z5atanhDv16_d(<16 x double> %355) nounwind readnone
  store <16 x double> %call230, <16 x double>* %a16_out, align 128
  %call231 = call <4 x double> @_Z6vload4jPKd(i32 0, double* %b_in)
  store <4 x double> %call231, <4 x double>* %a4_out, align 32
  %call232 = call <8 x double> @_Z6vload8jPKd(i32 0, double* %b_in)
  store <8 x double> %call232, <8 x double>* %a8_out, align 64
  %call233 = call <16 x double> @_Z7vload16jPKd(i32 0, double* %b_in)
  store <16 x double> %call233, <16 x double>* %a16_out, align 128
  %356 = load <4 x double>* %a4_in, align 32
  %357 = bitcast <4 x double>* %a4_out to double*
  call void @_Z7vstore4Dv4_djPd(<4 x double> %356, i32 0, double* %357)
  %358 = load <8 x double>* %a8_in, align 64
  %359 = bitcast <8 x double>* %a8_out to double*
  call void @_Z7vstore8Dv8_djPd(<8 x double> %358, i32 0, double* %359)
  %360 = load <16 x double>* %a16_in, align 128
  %361 = bitcast <16 x double>* %a16_out to double*
  call void @_Z8vstore16Dv16_djPd(<16 x double> %360, i32 0, double* %361)
  %362 = load double* %a_in, align 8
  %363 = load double* %b_in, align 8
  %call234 = call double @_Z3mindd(double %362, double %363) nounwind readnone
  store double %call234, double* %a_out, align 8
  %364 = load <4 x double>* %a4_in, align 32
  %365 = load <4 x double>* %b4_in, align 32
  %call235 = call <4 x double> @_Z3minDv4_dS_(<4 x double> %364, <4 x double> %365) nounwind readnone
  store <4 x double> %call235, <4 x double>* %a4_out, align 32
  %366 = load <8 x double>* %a8_in, align 64
  %367 = load <8 x double>* %b8_in, align 64
  %call236 = call <8 x double> @_Z3minDv8_dS_(<8 x double> %366, <8 x double> %367) nounwind readnone
  store <8 x double> %call236, <8 x double>* %a8_out, align 64
  %368 = load <16 x double>* %a16_in, align 128
  %369 = load <16 x double>* %b16_in, align 128
  %call237 = call <16 x double> @_Z3minDv16_dS_(<16 x double> %368, <16 x double> %369) nounwind readnone
  store <16 x double> %call237, <16 x double>* %a16_out, align 128
  %370 = load <4 x double>* %a4_in, align 32
  %371 = load double* %b_in, align 8
  %call238 = call <4 x double> @_Z3minDv4_dd(<4 x double> %370, double %371) nounwind readnone
  store <4 x double> %call238, <4 x double>* %a4_out, align 32
  %372 = load <8 x double>* %a8_in, align 64
  %373 = load double* %b_in, align 8
  %call239 = call <8 x double> @_Z3minDv8_dd(<8 x double> %372, double %373) nounwind readnone
  store <8 x double> %call239, <8 x double>* %a8_out, align 64
  %374 = load <16 x double>* %a16_in, align 128
  %375 = load double* %b_in, align 8
  %call240 = call <16 x double> @_Z3minDv16_dd(<16 x double> %374, double %375) nounwind readnone
  store <16 x double> %call240, <16 x double>* %a16_out, align 128
  %376 = load double* %a_in, align 8
  %377 = load double* %b_in, align 8
  %call241 = call double @_Z3maxdd(double %376, double %377) nounwind readnone
  store double %call241, double* %a_out, align 8
  %378 = load <4 x double>* %a4_in, align 32
  %379 = load <4 x double>* %b4_in, align 32
  %call242 = call <4 x double> @_Z3maxDv4_dS_(<4 x double> %378, <4 x double> %379) nounwind readnone
  store <4 x double> %call242, <4 x double>* %a4_out, align 32
  %380 = load <8 x double>* %a8_in, align 64
  %381 = load <8 x double>* %b8_in, align 64
  %call243 = call <8 x double> @_Z3maxDv8_dS_(<8 x double> %380, <8 x double> %381) nounwind readnone
  store <8 x double> %call243, <8 x double>* %a8_out, align 64
  %382 = load <16 x double>* %a16_in, align 128
  %383 = load <16 x double>* %b16_in, align 128
  %call244 = call <16 x double> @_Z3maxDv16_dS_(<16 x double> %382, <16 x double> %383) nounwind readnone
  store <16 x double> %call244, <16 x double>* %a16_out, align 128
  %384 = load <4 x double>* %a4_in, align 32
  %385 = load double* %b_in, align 8
  %call245 = call <4 x double> @_Z3maxDv4_dd(<4 x double> %384, double %385) nounwind readnone
  store <4 x double> %call245, <4 x double>* %a4_out, align 32
  %386 = load <8 x double>* %a8_in, align 64
  %387 = load double* %b_in, align 8
  %call246 = call <8 x double> @_Z3maxDv8_dd(<8 x double> %386, double %387) nounwind readnone
  store <8 x double> %call246, <8 x double>* %a8_out, align 64
  %388 = load <16 x double>* %a16_in, align 128
  %389 = load double* %b_in, align 8
  %call247 = call <16 x double> @_Z3maxDv16_dd(<16 x double> %388, double %389) nounwind readnone
  store <16 x double> %call247, <16 x double>* %a16_out, align 128
  %390 = load double* %a_in, align 8
  %391 = load double* %b_in, align 8
  %call248 = call double @_Z5hypotdd(double %390, double %391) nounwind readnone
  store double %call248, double* %a_out, align 8
  %392 = load <4 x double>* %a4_in, align 32
  %393 = load <4 x double>* %b4_in, align 32
  %call249 = call <4 x double> @_Z5hypotDv4_dS_(<4 x double> %392, <4 x double> %393) nounwind readnone
  store <4 x double> %call249, <4 x double>* %a4_out, align 32
  %394 = load <8 x double>* %a8_in, align 64
  %395 = load <8 x double>* %b8_in, align 64
  %call250 = call <8 x double> @_Z5hypotDv8_dS_(<8 x double> %394, <8 x double> %395) nounwind readnone
  store <8 x double> %call250, <8 x double>* %a8_out, align 64
  %396 = load <16 x double>* %a16_in, align 128
  %397 = load <16 x double>* %b16_in, align 128
  %call251 = call <16 x double> @_Z5hypotDv16_dS_(<16 x double> %396, <16 x double> %397) nounwind readnone
  store <16 x double> %call251, <16 x double>* %a16_out, align 128
  %398 = load double* %a_in, align 8
  %399 = load double* %b_in, align 8
  %call252 = call double @_Z4stepdd(double %398, double %399) nounwind readnone
  store double %call252, double* %a_out, align 8
  %400 = load <4 x double>* %a4_in, align 32
  %401 = load <4 x double>* %b4_in, align 32
  %call253 = call <4 x double> @_Z4stepDv4_dS_(<4 x double> %400, <4 x double> %401) nounwind readnone
  store <4 x double> %call253, <4 x double>* %a4_out, align 32
  %402 = load <8 x double>* %a8_in, align 64
  %403 = load <8 x double>* %b8_in, align 64
  %call254 = call <8 x double> @_Z4stepDv8_dS_(<8 x double> %402, <8 x double> %403) nounwind readnone
  store <8 x double> %call254, <8 x double>* %a8_out, align 64
  %404 = load <16 x double>* %a16_in, align 128
  %405 = load <16 x double>* %b16_in, align 128
  %call255 = call <16 x double> @_Z4stepDv16_dS_(<16 x double> %404, <16 x double> %405) nounwind readnone
  store <16 x double> %call255, <16 x double>* %a16_out, align 128
  %406 = load double* %a_in, align 8
  %407 = load double* %b_in, align 8
  %call256 = call double @_Z4stepdd(double %406, double %407) nounwind readnone
  store double %call256, double* %a_out, align 8
  %408 = load double* %a_in, align 8
  %409 = load <4 x double>* %b4_in, align 32
  %call257 = call <4 x double> @_Z4stepdDv4_d(double %408, <4 x double> %409) nounwind readnone
  store <4 x double> %call257, <4 x double>* %a4_out, align 32
  %410 = load double* %a_in, align 8
  %411 = load <8 x double>* %b8_in, align 64
  %call258 = call <8 x double> @_Z4stepdDv8_d(double %410, <8 x double> %411) nounwind readnone
  store <8 x double> %call258, <8 x double>* %a8_out, align 64
  %412 = load double* %a_in, align 8
  %413 = load <16 x double>* %b16_in, align 128
  %call259 = call <16 x double> @_Z4stepdDv16_d(double %412, <16 x double> %413) nounwind readnone
  store <16 x double> %call259, <16 x double>* %a16_out, align 128
  %414 = load double* %a_in, align 8
  %415 = load double* %b_in, align 8
  %416 = load double* %c_in, align 8
  %call260 = call double @_Z10smoothstepddd(double %414, double %415, double %416) nounwind readnone
  store double %call260, double* %a_out, align 8
  %417 = load <4 x double>* %a4_in, align 32
  %418 = load <4 x double>* %b4_in, align 32
  %419 = load <4 x double>* %c4_in, align 32
  %call261 = call <4 x double> @_Z10smoothstepDv4_dS_S_(<4 x double> %417, <4 x double> %418, <4 x double> %419) nounwind readnone
  store <4 x double> %call261, <4 x double>* %a4_out, align 32
  %420 = load <8 x double>* %a8_in, align 64
  %421 = load <8 x double>* %b8_in, align 64
  %422 = load <8 x double>* %c8_in, align 64
  %call262 = call <8 x double> @_Z10smoothstepDv8_dS_S_(<8 x double> %420, <8 x double> %421, <8 x double> %422) nounwind readnone
  store <8 x double> %call262, <8 x double>* %a8_out, align 64
  %423 = load <16 x double>* %a16_in, align 128
  %424 = load <16 x double>* %b16_in, align 128
  %425 = load <16 x double>* %c16_in, align 128
  %call263 = call <16 x double> @_Z10smoothstepDv16_dS_S_(<16 x double> %423, <16 x double> %424, <16 x double> %425) nounwind readnone
  store <16 x double> %call263, <16 x double>* %a16_out, align 128
  %426 = load double* %a_in, align 8
  %427 = load double* %b_in, align 8
  %428 = load double* %c_in, align 8
  %call264 = call double @_Z10smoothstepddd(double %426, double %427, double %428) nounwind readnone
  store double %call264, double* %a_out, align 8
  %429 = load double* %a_in, align 8
  %430 = load double* %b_in, align 8
  %431 = load <4 x double>* %c4_in, align 32
  %call265 = call <4 x double> @_Z10smoothstepddDv4_d(double %429, double %430, <4 x double> %431) nounwind readnone
  store <4 x double> %call265, <4 x double>* %a4_out, align 32
  %432 = load double* %a_in, align 8
  %433 = load double* %b_in, align 8
  %434 = load <8 x double>* %c8_in, align 64
  %call266 = call <8 x double> @_Z10smoothstepddDv8_d(double %432, double %433, <8 x double> %434) nounwind readnone
  store <8 x double> %call266, <8 x double>* %a8_out, align 64
  %435 = load double* %a_in, align 8
  %436 = load double* %b_in, align 8
  %437 = load <16 x double>* %c16_in, align 128
  %call267 = call <16 x double> @_Z10smoothstepddDv16_d(double %435, double %436, <16 x double> %437) nounwind readnone
  store <16 x double> %call267, <16 x double>* %a16_out, align 128
  %438 = load double* %a_in, align 8
  %call268 = call double @_Z7radiansd(double %438) nounwind readnone
  store double %call268, double* %a_out, align 8
  %439 = load <4 x double>* %a4_in, align 32
  %call269 = call <4 x double> @_Z7radiansDv4_d(<4 x double> %439) nounwind readnone
  store <4 x double> %call269, <4 x double>* %a4_out, align 32
  %440 = load <8 x double>* %a8_in, align 64
  %call270 = call <8 x double> @_Z7radiansDv8_d(<8 x double> %440) nounwind readnone
  store <8 x double> %call270, <8 x double>* %a8_out, align 64
  %441 = load <16 x double>* %a16_in, align 128
  %call271 = call <16 x double> @_Z7radiansDv16_d(<16 x double> %441) nounwind readnone
  store <16 x double> %call271, <16 x double>* %a16_out, align 128
  %442 = load double* %a_in, align 8
  %call272 = call double @_Z7degreesd(double %442) nounwind readnone
  store double %call272, double* %a_out, align 8
  %443 = load <4 x double>* %a4_in, align 32
  %call273 = call <4 x double> @_Z7degreesDv4_d(<4 x double> %443) nounwind readnone
  store <4 x double> %call273, <4 x double>* %a4_out, align 32
  %444 = load <8 x double>* %a8_in, align 64
  %call274 = call <8 x double> @_Z7degreesDv8_d(<8 x double> %444) nounwind readnone
  store <8 x double> %call274, <8 x double>* %a8_out, align 64
  %445 = load <16 x double>* %a16_in, align 128
  %call275 = call <16 x double> @_Z7degreesDv16_d(<16 x double> %445) nounwind readnone
  store <16 x double> %call275, <16 x double>* %a16_out, align 128
  %446 = load double* %a_in, align 8
  %call276 = call double @_Z4signd(double %446) nounwind readnone
  store double %call276, double* %a_out, align 8
  %447 = load <4 x double>* %a4_in, align 32
  %call277 = call <4 x double> @_Z4signDv4_d(<4 x double> %447) nounwind readnone
  store <4 x double> %call277, <4 x double>* %a4_out, align 32
  %448 = load <8 x double>* %a8_in, align 64
  %call278 = call <8 x double> @_Z4signDv8_d(<8 x double> %448) nounwind readnone
  store <8 x double> %call278, <8 x double>* %a8_out, align 64
  %449 = load <16 x double>* %a16_in, align 128
  %call279 = call <16 x double> @_Z4signDv16_d(<16 x double> %449) nounwind readnone
  store <16 x double> %call279, <16 x double>* %a16_out, align 128
  %450 = load double* %a_in, align 8
  %call280 = call double @_Z5floord(double %450) nounwind readnone
  store double %call280, double* %a_out, align 8
  %451 = load <4 x double>* %a4_in, align 32
  %call281 = call <4 x double> @_Z5floorDv4_d(<4 x double> %451) nounwind readnone
  store <4 x double> %call281, <4 x double>* %a4_out, align 32
  %452 = load <8 x double>* %a8_in, align 64
  %call282 = call <8 x double> @_Z5floorDv8_d(<8 x double> %452) nounwind readnone
  store <8 x double> %call282, <8 x double>* %a8_out, align 64
  %453 = load <16 x double>* %a16_in, align 128
  %call283 = call <16 x double> @_Z5floorDv16_d(<16 x double> %453) nounwind readnone
  store <16 x double> %call283, <16 x double>* %a16_out, align 128
  %454 = load double* %a_in, align 8
  %455 = load double* %b_in, align 8
  %call284 = call double @_Z3dotdd(double %454, double %455) nounwind readnone
  store double %call284, double* %a_out, align 8
  %456 = load <4 x double>* %a4_in, align 32
  %457 = load <4 x double>* %b4_in, align 32
  %call285 = call double @_Z3dotDv4_dS_(<4 x double> %456, <4 x double> %457) nounwind readnone
  store double %call285, double* %a_out, align 8
  %458 = load double* %a_in, align 8
  %459 = load double* %b_in, align 8
  %460 = load double* %c_in, align 8
  %call286 = call double @_Z3mixddd(double %458, double %459, double %460) nounwind readnone
  store double %call286, double* %a_out, align 8
  %461 = load <4 x double>* %a4_in, align 32
  %462 = load <4 x double>* %b4_in, align 32
  %463 = load <4 x double>* %c4_in, align 32
  %call287 = call <4 x double> @_Z3mixDv4_dS_S_(<4 x double> %461, <4 x double> %462, <4 x double> %463) nounwind readnone
  store <4 x double> %call287, <4 x double>* %a4_out, align 32
  %464 = load <4 x double>* %a4_in, align 32
  %465 = load <4 x double>* %b4_in, align 32
  %466 = load double* %c_in, align 8
  %call288 = call <4 x double> @_Z3mixDv4_dS_d(<4 x double> %464, <4 x double> %465, double %466) nounwind readnone
  store <4 x double> %call288, <4 x double>* %a4_out, align 32
  %467 = load <8 x double>* %a8_in, align 64
  %468 = load <8 x double>* %b8_in, align 64
  %469 = load double* %c_in, align 8
  %call289 = call <8 x double> @_Z3mixDv8_dS_d(<8 x double> %467, <8 x double> %468, double %469) nounwind readnone
  store <8 x double> %call289, <8 x double>* %a8_out, align 64
  %470 = load <16 x double>* %a16_in, align 128
  %471 = load <16 x double>* %b16_in, align 128
  %472 = load double* %c_in, align 8
  %call290 = call <16 x double> @_Z3mixDv16_dS_d(<16 x double> %470, <16 x double> %471, double %472) nounwind readnone
  store <16 x double> %call290, <16 x double>* %a16_out, align 128
  %473 = load <8 x double>* %a8_in, align 64
  %474 = load <8 x double>* %b8_in, align 64
  %475 = load <8 x double>* %c8_in, align 64
  %call291 = call <8 x double> @_Z3mixDv8_dS_S_(<8 x double> %473, <8 x double> %474, <8 x double> %475) nounwind readnone
  store <8 x double> %call291, <8 x double>* %a8_out, align 64
  %476 = load <16 x double>* %a16_in, align 128
  %477 = load <16 x double>* %b16_in, align 128
  %478 = load <16 x double>* %c16_in, align 128
  %call292 = call <16 x double> @_Z3mixDv16_dS_S_(<16 x double> %476, <16 x double> %477, <16 x double> %478) nounwind readnone
  store <16 x double> %call292, <16 x double>* %a16_out, align 128
  %479 = load double* %a_in, align 8
  %call293 = call double @_Z9normalized(double %479) nounwind readnone
  store double %call293, double* %a_out, align 8
  %480 = load <4 x double>* %a4_in, align 32
  %call294 = call <4 x double> @_Z9normalizeDv4_d(<4 x double> %480) nounwind readnone
  store <4 x double> %call294, <4 x double>* %a4_out, align 32
  %481 = load <4 x double>* %a4_in, align 32
  %482 = load <4 x double>* %b4_in, align 32
  %call295 = call <4 x double> @_Z5crossDv4_dS_(<4 x double> %481, <4 x double> %482) nounwind readnone
  store <4 x double> %call295, <4 x double>* %a4_out, align 32
  %483 = load double* %a_in, align 8
  %call296 = call double @_Z6lengthd(double %483) nounwind readnone
  store double %call296, double* %a_out, align 8
  %484 = load <2 x double>* %a2_in, align 16
  %call297 = call double @_Z6lengthDv2_d(<2 x double> %484) nounwind readnone
  store double %call297, double* %a_out, align 8
  %485 = load <4 x double>* %a4_in, align 32
  %call298 = call double @_Z6lengthDv4_d(<4 x double> %485) nounwind readnone
  store double %call298, double* %a_out, align 8
  %486 = load double* %a_in, align 8
  %487 = load double* %b_in, align 8
  %call299 = call double @_Z8distancedd(double %486, double %487) nounwind readnone
  store double %call299, double* %a_out, align 8
  %488 = load <2 x double>* %a2_in, align 16
  %489 = load <2 x double>* %b2_in, align 16
  %call300 = call double @_Z8distanceDv2_dS_(<2 x double> %488, <2 x double> %489) nounwind readnone
  store double %call300, double* %a_out, align 8
  %490 = load <4 x double>* %a4_in, align 32
  %491 = load <4 x double>* %b4_in, align 32
  %call301 = call double @_Z8distanceDv4_dS_(<4 x double> %490, <4 x double> %491) nounwind readnone
  store double %call301, double* %a_out, align 8
  %492 = load double* %a_in, align 8
  %493 = load i32* %i_in, align 4
  %call302 = call double @_Z5rootndi(double %492, i32 %493) nounwind readnone
  store double %call302, double* %a_out, align 8
  %494 = load <4 x double>* %a4_in, align 32
  %495 = load <4 x i32>* %i4_in, align 16
  %call303 = call <4 x double> @_Z5rootnDv4_dDv4_i(<4 x double> %494, <4 x i32> %495) nounwind readnone
  store <4 x double> %call303, <4 x double>* %a4_out, align 32
  %496 = load <8 x double>* %a8_in, align 64
  %497 = load <8 x i32>* %i8_in, align 32
  %call304 = call <8 x double> @_Z5rootnDv8_dDv8_i(<8 x double> %496, <8 x i32> %497) nounwind readnone
  store <8 x double> %call304, <8 x double>* %a8_out, align 64
  %498 = load <16 x double>* %a16_in, align 128
  %499 = load <16 x i32>* %i16_in, align 64
  %call305 = call <16 x double> @_Z5rootnDv16_dDv16_i(<16 x double> %498, <16 x i32> %499) nounwind readnone
  store <16 x double> %call305, <16 x double>* %a16_out, align 128
  %500 = load double* %a_in, align 8
  %501 = load i32* %i_in, align 4
  %call306 = call double @_Z5ldexpdi(double %500, i32 %501) nounwind readnone
  store double %call306, double* %a_out, align 8
  %502 = load <4 x double>* %a4_in, align 32
  %503 = load <4 x i32>* %i4_in, align 16
  %call307 = call <4 x double> @_Z5ldexpDv4_dDv4_i(<4 x double> %502, <4 x i32> %503) nounwind readnone
  store <4 x double> %call307, <4 x double>* %a4_out, align 32
  %504 = load <8 x double>* %a8_in, align 64
  %505 = load <8 x i32>* %i8_in, align 32
  %call308 = call <8 x double> @_Z5ldexpDv8_dDv8_i(<8 x double> %504, <8 x i32> %505) nounwind readnone
  store <8 x double> %call308, <8 x double>* %a8_out, align 64
  %506 = load <16 x double>* %a16_in, align 128
  %507 = load <16 x i32>* %i16_in, align 64
  %call309 = call <16 x double> @_Z5ldexpDv16_dDv16_i(<16 x double> %506, <16 x i32> %507) nounwind readnone
  store <16 x double> %call309, <16 x double>* %a16_out, align 128
  %508 = load <4 x double>* %a4_in, align 32
  %509 = load i32* %i_in, align 4
  %call310 = call <4 x double> @_Z5ldexpDv4_di(<4 x double> %508, i32 %509) nounwind readnone
  store <4 x double> %call310, <4 x double>* %a4_out, align 32
  %510 = load <8 x double>* %a8_in, align 64
  %511 = load i32* %i_in, align 4
  %call311 = call <8 x double> @_Z5ldexpDv8_di(<8 x double> %510, i32 %511) nounwind readnone
  store <8 x double> %call311, <8 x double>* %a8_out, align 64
  %512 = load <16 x double>* %a16_in, align 128
  %513 = load i32* %i_in, align 4
  %call312 = call <16 x double> @_Z5ldexpDv16_di(<16 x double> %512, i32 %513) nounwind readnone
  store <16 x double> %call312, <16 x double>* %a16_out, align 128
  %514 = load double* %a_in, align 8
  %call313 = call double @_Z4modfdPd(double %514, double* %b_out)
  store double %call313, double* %a_out, align 8
  %515 = load <4 x double>* %a4_in, align 32
  %call314 = call <4 x double> @_Z4modfDv4_dPS_(<4 x double> %515, <4 x double>* %b4_out)
  store <4 x double> %call314, <4 x double>* %a4_out, align 32
  %516 = load <8 x double>* %a8_in, align 64
  %call315 = call <8 x double> @_Z4modfDv8_dPS_(<8 x double> %516, <8 x double>* %b8_out)
  store <8 x double> %call315, <8 x double>* %a8_out, align 64
  %517 = load <16 x double>* %a16_in, align 128
  %call316 = call <16 x double> @_Z4modfDv16_dPS_(<16 x double> %517, <16 x double>* %b16_out)
  store <16 x double> %call316, <16 x double>* %a16_out, align 128
  %518 = load double* %a_in, align 8
  %call317 = call double @_Z5frexpdPi(double %518, i32* %i_out)
  store double %call317, double* %a_out, align 8
  %519 = load <4 x double>* %a4_in, align 32
  %call318 = call <4 x double> @_Z5frexpDv4_dPDv4_i(<4 x double> %519, <4 x i32>* %i4_out)
  store <4 x double> %call318, <4 x double>* %a4_out, align 32
  %520 = load <8 x double>* %a8_in, align 64
  %call319 = call <8 x double> @_Z5frexpDv8_dPDv8_i(<8 x double> %520, <8 x i32>* %i8_out)
  store <8 x double> %call319, <8 x double>* %a8_out, align 64
  %521 = load <16 x double>* %a16_in, align 128
  %call320 = call <16 x double> @_Z5frexpDv16_dPDv16_i(<16 x double> %521, <16 x i32>* %i16_out)
  store <16 x double> %call320, <16 x double>* %a16_out, align 128
  %522 = load double* %a_in, align 8
  %523 = load double* %b_in, align 8
  %call321 = call double @_Z6maxmagdd(double %522, double %523) nounwind readnone
  store double %call321, double* %a_out, align 8
  %524 = load <4 x double>* %a4_in, align 32
  %525 = load <4 x double>* %b4_in, align 32
  %call322 = call <4 x double> @_Z6maxmagDv4_dS_(<4 x double> %524, <4 x double> %525) nounwind readnone
  store <4 x double> %call322, <4 x double>* %a4_out, align 32
  %526 = load <8 x double>* %a8_in, align 64
  %527 = load <8 x double>* %b8_in, align 64
  %call323 = call <8 x double> @_Z6maxmagDv8_dS_(<8 x double> %526, <8 x double> %527) nounwind readnone
  store <8 x double> %call323, <8 x double>* %a8_out, align 64
  %528 = load <16 x double>* %a16_in, align 128
  %529 = load <16 x double>* %b16_in, align 128
  %call324 = call <16 x double> @_Z6maxmagDv16_dS_(<16 x double> %528, <16 x double> %529) nounwind readnone
  store <16 x double> %call324, <16 x double>* %a16_out, align 128
  %530 = load double* %a_in, align 8
  %531 = load double* %b_in, align 8
  %call325 = call double @_Z6minmagdd(double %530, double %531) nounwind readnone
  store double %call325, double* %a_out, align 8
  %532 = load <4 x double>* %a4_in, align 32
  %533 = load <4 x double>* %b4_in, align 32
  %call326 = call <4 x double> @_Z6minmagDv4_dS_(<4 x double> %532, <4 x double> %533) nounwind readnone
  store <4 x double> %call326, <4 x double>* %a4_out, align 32
  %534 = load <8 x double>* %a8_in, align 64
  %535 = load <8 x double>* %b8_in, align 64
  %call327 = call <8 x double> @_Z6minmagDv8_dS_(<8 x double> %534, <8 x double> %535) nounwind readnone
  store <8 x double> %call327, <8 x double>* %a8_out, align 64
  %536 = load <16 x double>* %a16_in, align 128
  %537 = load <16 x double>* %b16_in, align 128
  %call328 = call <16 x double> @_Z6minmagDv16_dS_(<16 x double> %536, <16 x double> %537) nounwind readnone
  store <16 x double> %call328, <16 x double>* %a16_out, align 128
  %538 = load double* %a_in, align 8
  %539 = load double* %b_in, align 8
  %call329 = call double @_Z8copysigndd(double %538, double %539) nounwind readnone
  store double %call329, double* %a_out, align 8
  %540 = load <4 x double>* %a4_in, align 32
  %541 = load <4 x double>* %b4_in, align 32
  %call330 = call <4 x double> @_Z8copysignDv4_dS_(<4 x double> %540, <4 x double> %541) nounwind readnone
  store <4 x double> %call330, <4 x double>* %a4_out, align 32
  %542 = load <8 x double>* %a8_in, align 64
  %543 = load <8 x double>* %b8_in, align 64
  %call331 = call <8 x double> @_Z8copysignDv8_dS_(<8 x double> %542, <8 x double> %543) nounwind readnone
  store <8 x double> %call331, <8 x double>* %a8_out, align 64
  %544 = load <16 x double>* %a16_in, align 128
  %545 = load <16 x double>* %b16_in, align 128
  %call332 = call <16 x double> @_Z8copysignDv16_dS_(<16 x double> %544, <16 x double> %545) nounwind readnone
  store <16 x double> %call332, <16 x double>* %a16_out, align 128
  %546 = load double* %a_in, align 8
  %547 = load double* %b_in, align 8
  %call333 = call double @_Z9nextafterdd(double %546, double %547) nounwind readnone
  store double %call333, double* %a_out, align 8
  %548 = load <4 x double>* %a4_in, align 32
  %549 = load <4 x double>* %b4_in, align 32
  %call334 = call <4 x double> @_Z9nextafterDv4_dS_(<4 x double> %548, <4 x double> %549) nounwind readnone
  store <4 x double> %call334, <4 x double>* %a4_out, align 32
  %550 = load <8 x double>* %a8_in, align 64
  %551 = load <8 x double>* %b8_in, align 64
  %call335 = call <8 x double> @_Z9nextafterDv8_dS_(<8 x double> %550, <8 x double> %551) nounwind readnone
  store <8 x double> %call335, <8 x double>* %a8_out, align 64
  %552 = load <16 x double>* %a16_in, align 128
  %553 = load <16 x double>* %b16_in, align 128
  %call336 = call <16 x double> @_Z9nextafterDv16_dS_(<16 x double> %552, <16 x double> %553) nounwind readnone
  store <16 x double> %call336, <16 x double>* %a16_out, align 128
  %554 = load double* %a_in, align 8
  %555 = load double* %b_in, align 8
  %call337 = call double @_Z4fdimdd(double %554, double %555) nounwind readnone
  store double %call337, double* %a_out, align 8
  %556 = load <4 x double>* %a4_in, align 32
  %557 = load <4 x double>* %b4_in, align 32
  %call338 = call <4 x double> @_Z4fdimDv4_dS_(<4 x double> %556, <4 x double> %557) nounwind readnone
  store <4 x double> %call338, <4 x double>* %a4_out, align 32
  %558 = load <8 x double>* %a8_in, align 64
  %559 = load <8 x double>* %b8_in, align 64
  %call339 = call <8 x double> @_Z4fdimDv8_dS_(<8 x double> %558, <8 x double> %559) nounwind readnone
  store <8 x double> %call339, <8 x double>* %a8_out, align 64
  %560 = load <16 x double>* %a16_in, align 128
  %561 = load <16 x double>* %b16_in, align 128
  %call340 = call <16 x double> @_Z4fdimDv16_dS_(<16 x double> %560, <16 x double> %561) nounwind readnone
  store <16 x double> %call340, <16 x double>* %a16_out, align 128
  %562 = load double* %a_in, align 8
  %563 = load double* %b_in, align 8
  %564 = load double* %c_in, align 8
  %call341 = call double @_Z3fmaddd(double %562, double %563, double %564) nounwind readnone
  store double %call341, double* %a_out, align 8
  %565 = load <4 x double>* %a4_in, align 32
  %566 = load <4 x double>* %b4_in, align 32
  %567 = load <4 x double>* %c4_in, align 32
  %call342 = call <4 x double> @_Z3fmaDv4_dS_S_(<4 x double> %565, <4 x double> %566, <4 x double> %567) nounwind readnone
  store <4 x double> %call342, <4 x double>* %a4_out, align 32
  %568 = load <8 x double>* %a8_in, align 64
  %569 = load <8 x double>* %b8_in, align 64
  %570 = load <8 x double>* %c8_in, align 64
  %call343 = call <8 x double> @_Z3fmaDv8_dS_S_(<8 x double> %568, <8 x double> %569, <8 x double> %570) nounwind readnone
  store <8 x double> %call343, <8 x double>* %a8_out, align 64
  %571 = load <16 x double>* %a16_in, align 128
  %572 = load <16 x double>* %b16_in, align 128
  %573 = load <16 x double>* %c16_in, align 128
  %call344 = call <16 x double> @_Z3fmaDv16_dS_S_(<16 x double> %571, <16 x double> %572, <16 x double> %573) nounwind readnone
  store <16 x double> %call344, <16 x double>* %a16_out, align 128
  %574 = load double* %a_in, align 8
  %575 = load double* %b_in, align 8
  %576 = load double* %c_in, align 8
  %call345 = call double @_Z3madddd(double %574, double %575, double %576) nounwind readnone
  store double %call345, double* %a_out, align 8
  %577 = load <4 x double>* %a4_in, align 32
  %578 = load <4 x double>* %b4_in, align 32
  %579 = load <4 x double>* %c4_in, align 32
  %call346 = call <4 x double> @_Z3madDv4_dS_S_(<4 x double> %577, <4 x double> %578, <4 x double> %579) nounwind readnone
  store <4 x double> %call346, <4 x double>* %a4_out, align 32
  %580 = load <8 x double>* %a8_in, align 64
  %581 = load <8 x double>* %b8_in, align 64
  %582 = load <8 x double>* %c8_in, align 64
  %call347 = call <8 x double> @_Z3madDv8_dS_S_(<8 x double> %580, <8 x double> %581, <8 x double> %582) nounwind readnone
  store <8 x double> %call347, <8 x double>* %a8_out, align 64
  %583 = load <16 x double>* %a16_in, align 128
  %584 = load <16 x double>* %b16_in, align 128
  %585 = load <16 x double>* %c16_in, align 128
  %call348 = call <16 x double> @_Z3madDv16_dS_S_(<16 x double> %583, <16 x double> %584, <16 x double> %585) nounwind readnone
  store <16 x double> %call348, <16 x double>* %a16_out, align 128
  %586 = load double* %a_in, align 8
  %call349 = call double @_Z4rintd(double %586) nounwind readnone
  store double %call349, double* %a_out, align 8
  %587 = load <4 x double>* %a4_in, align 32
  %call350 = call <4 x double> @_Z4rintDv4_d(<4 x double> %587) nounwind readnone
  store <4 x double> %call350, <4 x double>* %a4_out, align 32
  %588 = load <8 x double>* %a8_in, align 64
  %call351 = call <8 x double> @_Z4rintDv8_d(<8 x double> %588) nounwind readnone
  store <8 x double> %call351, <8 x double>* %a8_out, align 64
  %589 = load <16 x double>* %a16_in, align 128
  %call352 = call <16 x double> @_Z4rintDv16_d(<16 x double> %589) nounwind readnone
  store <16 x double> %call352, <16 x double>* %a16_out, align 128
  %590 = load double* %a_in, align 8
  %call353 = call double @_Z5roundd(double %590) nounwind readnone
  store double %call353, double* %a_out, align 8
  %591 = load <4 x double>* %a4_in, align 32
  %call354 = call <4 x double> @_Z5roundDv4_d(<4 x double> %591) nounwind readnone
  store <4 x double> %call354, <4 x double>* %a4_out, align 32
  %592 = load <8 x double>* %a8_in, align 64
  %call355 = call <8 x double> @_Z5roundDv8_d(<8 x double> %592) nounwind readnone
  store <8 x double> %call355, <8 x double>* %a8_out, align 64
  %593 = load <16 x double>* %a16_in, align 128
  %call356 = call <16 x double> @_Z5roundDv16_d(<16 x double> %593) nounwind readnone
  store <16 x double> %call356, <16 x double>* %a16_out, align 128
  %594 = load double* %a_in, align 8
  %call357 = call double @_Z5truncd(double %594) nounwind readnone
  store double %call357, double* %a_out, align 8
  %595 = load <4 x double>* %a4_in, align 32
  %call358 = call <4 x double> @_Z5truncDv4_d(<4 x double> %595) nounwind readnone
  store <4 x double> %call358, <4 x double>* %a4_out, align 32
  %596 = load <8 x double>* %a8_in, align 64
  %call359 = call <8 x double> @_Z5truncDv8_d(<8 x double> %596) nounwind readnone
  store <8 x double> %call359, <8 x double>* %a8_out, align 64
  %597 = load <16 x double>* %a16_in, align 128
  %call360 = call <16 x double> @_Z5truncDv16_d(<16 x double> %597) nounwind readnone
  store <16 x double> %call360, <16 x double>* %a16_out, align 128
  %598 = load double* %a_in, align 8
  %call361 = call double @_Z4cbrtd(double %598) nounwind readnone
  store double %call361, double* %a_out, align 8
  %599 = load <4 x double>* %a4_in, align 32
  %call362 = call <4 x double> @_Z4cbrtDv4_d(<4 x double> %599) nounwind readnone
  store <4 x double> %call362, <4 x double>* %a4_out, align 32
  %600 = load <8 x double>* %a8_in, align 64
  %call363 = call <8 x double> @_Z4cbrtDv8_d(<8 x double> %600) nounwind readnone
  store <8 x double> %call363, <8 x double>* %a8_out, align 64
  %601 = load <16 x double>* %a16_in, align 128
  %call364 = call <16 x double> @_Z4cbrtDv16_d(<16 x double> %601) nounwind readnone
  store <16 x double> %call364, <16 x double>* %a16_out, align 128
  %602 = load double* %a_in, align 8
  %603 = load double* %b_in, align 8
  %call365 = call double @_Z4powrdd(double %602, double %603) nounwind readnone
  store double %call365, double* %a_out, align 8
  %604 = load <4 x double>* %a4_in, align 32
  %605 = load <4 x double>* %b4_in, align 32
  %call366 = call <4 x double> @_Z4powrDv4_dS_(<4 x double> %604, <4 x double> %605) nounwind readnone
  store <4 x double> %call366, <4 x double>* %a4_out, align 32
  %606 = load <8 x double>* %a8_in, align 64
  %607 = load <8 x double>* %b8_in, align 64
  %call367 = call <8 x double> @_Z4powrDv8_dS_(<8 x double> %606, <8 x double> %607) nounwind readnone
  store <8 x double> %call367, <8 x double>* %a8_out, align 64
  %608 = load <16 x double>* %a16_in, align 128
  %609 = load <16 x double>* %b16_in, align 128
  %call368 = call <16 x double> @_Z4powrDv16_dS_(<16 x double> %608, <16 x double> %609) nounwind readnone
  store <16 x double> %call368, <16 x double>* %a16_out, align 128
  %610 = load double* %a_in, align 8
  %611 = load double* %b_in, align 8
  %call369 = call double @_Z4fmoddd(double %610, double %611) nounwind readnone
  store double %call369, double* %a_out, align 8
  %612 = load <4 x double>* %a4_in, align 32
  %613 = load <4 x double>* %b4_in, align 32
  %call370 = call <4 x double> @_Z4fmodDv4_dS_(<4 x double> %612, <4 x double> %613) nounwind readnone
  store <4 x double> %call370, <4 x double>* %a4_out, align 32
  %614 = load <8 x double>* %a8_in, align 64
  %615 = load <8 x double>* %b8_in, align 64
  %call371 = call <8 x double> @_Z4fmodDv8_dS_(<8 x double> %614, <8 x double> %615) nounwind readnone
  store <8 x double> %call371, <8 x double>* %a8_out, align 64
  %616 = load <16 x double>* %a16_in, align 128
  %617 = load <16 x double>* %b16_in, align 128
  %call372 = call <16 x double> @_Z4fmodDv16_dS_(<16 x double> %616, <16 x double> %617) nounwind readnone
  store <16 x double> %call372, <16 x double>* %a16_out, align 128
  %618 = load double* %a_in, align 8
  %619 = load double* %b_in, align 8
  %call373 = call double @_Z4fmindd(double %618, double %619) nounwind readnone
  store double %call373, double* %a_out, align 8
  %620 = load <4 x double>* %a4_in, align 32
  %621 = load <4 x double>* %b4_in, align 32
  %call374 = call <4 x double> @_Z4fminDv4_dS_(<4 x double> %620, <4 x double> %621) nounwind readnone
  store <4 x double> %call374, <4 x double>* %a4_out, align 32
  %622 = load <8 x double>* %a8_in, align 64
  %623 = load <8 x double>* %b8_in, align 64
  %call375 = call <8 x double> @_Z4fminDv8_dS_(<8 x double> %622, <8 x double> %623) nounwind readnone
  store <8 x double> %call375, <8 x double>* %a8_out, align 64
  %624 = load <16 x double>* %a16_in, align 128
  %625 = load <16 x double>* %b16_in, align 128
  %call376 = call <16 x double> @_Z4fminDv16_dS_(<16 x double> %624, <16 x double> %625) nounwind readnone
  store <16 x double> %call376, <16 x double>* %a16_out, align 128
  %626 = load double* %a_in, align 8
  %627 = load double* %b_in, align 8
  %call377 = call double @_Z4fmaxdd(double %626, double %627) nounwind readnone
  store double %call377, double* %a_out, align 8
  %628 = load <4 x double>* %a4_in, align 32
  %629 = load <4 x double>* %b4_in, align 32
  %call378 = call <4 x double> @_Z4fmaxDv4_dS_(<4 x double> %628, <4 x double> %629) nounwind readnone
  store <4 x double> %call378, <4 x double>* %a4_out, align 32
  %630 = load <8 x double>* %a8_in, align 64
  %631 = load <8 x double>* %b8_in, align 64
  %call379 = call <8 x double> @_Z4fmaxDv8_dS_(<8 x double> %630, <8 x double> %631) nounwind readnone
  store <8 x double> %call379, <8 x double>* %a8_out, align 64
  %632 = load <16 x double>* %a16_in, align 128
  %633 = load <16 x double>* %b16_in, align 128
  %call380 = call <16 x double> @_Z4fmaxDv16_dS_(<16 x double> %632, <16 x double> %633) nounwind readnone
  store <16 x double> %call380, <16 x double>* %a16_out, align 128
  %634 = load <4 x double>* %a4_in, align 32
  %635 = load double* %b_in, align 8
  %call381 = call <4 x double> @_Z4fminDv4_dd(<4 x double> %634, double %635) nounwind readnone
  store <4 x double> %call381, <4 x double>* %a4_out, align 32
  %636 = load <8 x double>* %a8_in, align 64
  %637 = load double* %b_in, align 8
  %call382 = call <8 x double> @_Z4fminDv8_dd(<8 x double> %636, double %637) nounwind readnone
  store <8 x double> %call382, <8 x double>* %a8_out, align 64
  %638 = load <16 x double>* %a16_in, align 128
  %639 = load double* %b_in, align 8
  %call383 = call <16 x double> @_Z4fminDv16_dd(<16 x double> %638, double %639) nounwind readnone
  store <16 x double> %call383, <16 x double>* %a16_out, align 128
  %640 = load <4 x double>* %a4_in, align 32
  %641 = load double* %b_in, align 8
  %call384 = call <4 x double> @_Z4fmaxDv4_dd(<4 x double> %640, double %641) nounwind readnone
  store <4 x double> %call384, <4 x double>* %a4_out, align 32
  %642 = load <8 x double>* %a8_in, align 64
  %643 = load double* %b_in, align 8
  %call385 = call <8 x double> @_Z4fmaxDv8_dd(<8 x double> %642, double %643) nounwind readnone
  store <8 x double> %call385, <8 x double>* %a8_out, align 64
  %644 = load <16 x double>* %a16_in, align 128
  %645 = load double* %b_in, align 8
  %call386 = call <16 x double> @_Z4fmaxDv16_dd(<16 x double> %644, double %645) nounwind readnone
  store <16 x double> %call386, <16 x double>* %a16_out, align 128
  %646 = load <4 x double>* %a4_in, align 32
  %647 = load i32* %i_in, align 4
  %648 = insertelement <4 x i32> undef, i32 %647, i32 0
  %splat387 = shufflevector <4 x i32> %648, <4 x i32> %648, <4 x i32> zeroinitializer
  %call388 = call <4 x double> @_Z4pownDv4_dDv4_i(<4 x double> %646, <4 x i32> %splat387) nounwind readnone
  store <4 x double> %call388, <4 x double>* %a4_out, align 32
  %649 = load <8 x double>* %a8_in, align 64
  %650 = load i32* %i_in, align 4
  %651 = insertelement <8 x i32> undef, i32 %650, i32 0
  %splat389 = shufflevector <8 x i32> %651, <8 x i32> %651, <8 x i32> zeroinitializer
  %call390 = call <8 x double> @_Z4pownDv8_dDv8_i(<8 x double> %649, <8 x i32> %splat389) nounwind readnone
  store <8 x double> %call390, <8 x double>* %a8_out, align 64
  %652 = load <16 x double>* %a16_in, align 128
  %653 = load i32* %i_in, align 4
  %654 = insertelement <16 x i32> undef, i32 %653, i32 0
  %splat391 = shufflevector <16 x i32> %654, <16 x i32> %654, <16 x i32> zeroinitializer
  %call392 = call <16 x double> @_Z4pownDv16_dDv16_i(<16 x double> %652, <16 x i32> %splat391) nounwind readnone
  store <16 x double> %call392, <16 x double>* %a16_out, align 128
  %655 = load double* %a_in, align 8
  %call393 = call i32 @_Z5ilogbd(double %655) nounwind readnone
  store i32 %call393, i32* %i_out, align 4
  %656 = load <4 x double>* %a4_in, align 32
  %call394 = call <4 x i32> @_Z5ilogbDv4_d(<4 x double> %656) nounwind readnone
  store <4 x i32> %call394, <4 x i32>* %i4_out, align 16
  %657 = load <8 x double>* %a8_in, align 64
  %call395 = call <8 x i32> @_Z5ilogbDv8_d(<8 x double> %657) nounwind readnone
  store <8 x i32> %call395, <8 x i32>* %i8_out, align 32
  %658 = load <16 x double>* %a16_in, align 128
  %call396 = call <16 x i32> @_Z5ilogbDv16_d(<16 x double> %658) nounwind readnone
  store <16 x i32> %call396, <16 x i32>* %i16_out, align 64
  %659 = load i64* %ul_in, align 8
  %call397 = call double @_Z3nanm(i64 %659) nounwind readnone
  store double %call397, double* %a_out, align 8
  %660 = load <4 x i64>* %ul4_in, align 32
  %call398 = call <4 x double> @_Z3nanDv4_m(<4 x i64> %660) nounwind readnone
  store <4 x double> %call398, <4 x double>* %a4_out, align 32
  %661 = load <8 x i64>* %ul8_in, align 64
  %call399 = call <8 x double> @_Z3nanDv8_m(<8 x i64> %661) nounwind readnone
  store <8 x double> %call399, <8 x double>* %a8_out, align 64
  %662 = load <16 x i64>* %ul16_in, align 128
  %call400 = call <16 x double> @_Z3nanDv16_m(<16 x i64> %662) nounwind readnone
  store <16 x double> %call400, <16 x double>* %a16_out, align 128
  %663 = load double* %a_in, align 8
  %call401 = call double @_Z5fractdPd(double %663, double* %b_out)
  store double %call401, double* %a_out, align 8
  %664 = load <4 x double>* %a4_in, align 32
  %call402 = call <4 x double> @_Z5fractDv4_dPS_(<4 x double> %664, <4 x double>* %b4_out)
  store <4 x double> %call402, <4 x double>* %a4_out, align 32
  %665 = load <8 x double>* %a8_in, align 64
  %call403 = call <8 x double> @_Z5fractDv8_dPS_(<8 x double> %665, <8 x double>* %b8_out)
  store <8 x double> %call403, <8 x double>* %a8_out, align 64
  %666 = load <16 x double>* %a16_in, align 128
  %call404 = call <16 x double> @_Z5fractDv16_dPS_(<16 x double> %666, <16 x double>* %b16_out)
  store <16 x double> %call404, <16 x double>* %a16_out, align 128
  %667 = load double* %a_in, align 8
  %call405 = call double @_Z6lgammad(double %667) nounwind readnone
  store double %call405, double* %a_out, align 8
  %668 = load <4 x double>* %a4_in, align 32
  %call406 = call <4 x double> @_Z6lgammaDv4_d(<4 x double> %668) nounwind readnone
  store <4 x double> %call406, <4 x double>* %a4_out, align 32
  %669 = load <8 x double>* %a8_in, align 64
  %call407 = call <8 x double> @_Z6lgammaDv8_d(<8 x double> %669) nounwind readnone
  store <8 x double> %call407, <8 x double>* %a8_out, align 64
  %670 = load <16 x double>* %a16_in, align 128
  %call408 = call <16 x double> @_Z6lgammaDv16_d(<16 x double> %670) nounwind readnone
  store <16 x double> %call408, <16 x double>* %a16_out, align 128
  %671 = load double* %a_in, align 8
  %call409 = call double @_Z8lgamma_rdPi(double %671, i32* %i_out)
  store double %call409, double* %a_out, align 8
  %672 = load <4 x double>* %a4_in, align 32
  %call410 = call <4 x double> @_Z8lgamma_rDv4_dPDv4_i(<4 x double> %672, <4 x i32>* %i4_out)
  store <4 x double> %call410, <4 x double>* %a4_out, align 32
  %673 = load <8 x double>* %a8_in, align 64
  %call411 = call <8 x double> @_Z8lgamma_rDv8_dPDv8_i(<8 x double> %673, <8 x i32>* %i8_out)
  store <8 x double> %call411, <8 x double>* %a8_out, align 64
  %674 = load <16 x double>* %a16_in, align 128
  %call412 = call <16 x double> @_Z8lgamma_rDv16_dPDv16_i(<16 x double> %674, <16 x i32>* %i16_out)
  store <16 x double> %call412, <16 x double>* %a16_out, align 128
  %675 = load double* %a_in, align 8
  %676 = load double* %b_in, align 8
  %677 = load double* %c_in, align 8
  %call413 = call double @_Z9bitselectddd(double %675, double %676, double %677) nounwind readnone
  store double %call413, double* %a_out, align 8
  %678 = load <4 x double>* %a4_in, align 32
  %679 = load <4 x double>* %b4_in, align 32
  %680 = load <4 x double>* %c4_in, align 32
  %call414 = call <4 x double> @_Z9bitselectDv4_dS_S_(<4 x double> %678, <4 x double> %679, <4 x double> %680) nounwind readnone
  store <4 x double> %call414, <4 x double>* %a4_out, align 32
  %681 = load <8 x double>* %a8_in, align 64
  %682 = load <8 x double>* %b8_in, align 64
  %683 = load <8 x double>* %c8_in, align 64
  %call415 = call <8 x double> @_Z9bitselectDv8_dS_S_(<8 x double> %681, <8 x double> %682, <8 x double> %683) nounwind readnone
  store <8 x double> %call415, <8 x double>* %a8_out, align 64
  %684 = load <16 x double>* %a16_in, align 128
  %685 = load <16 x double>* %b16_in, align 128
  %686 = load <16 x double>* %c16_in, align 128
  %call416 = call <16 x double> @_Z9bitselectDv16_dS_S_(<16 x double> %684, <16 x double> %685, <16 x double> %686) nounwind readnone
  store <16 x double> %call416, <16 x double>* %a16_out, align 128
  %687 = load double* %a_in, align 8
  %688 = load double* %b_in, align 8
  %689 = load i64* %l_in, align 8
  %call417 = call double @_Z6selectddl(double %687, double %688, i64 %689) nounwind readnone
  store double %call417, double* %a_out, align 8
  %690 = load <4 x double>* %a4_in, align 32
  %691 = load <4 x double>* %b4_in, align 32
  %692 = load <4 x i64>* %l4_in, align 32
  %call418 = call <4 x double> @_Z6selectDv4_dS_Dv4_l(<4 x double> %690, <4 x double> %691, <4 x i64> %692) nounwind readnone
  store <4 x double> %call418, <4 x double>* %a4_out, align 32
  %693 = load <8 x double>* %a8_in, align 64
  %694 = load <8 x double>* %b8_in, align 64
  %695 = load <8 x i64>* %l8_in, align 64
  %call419 = call <8 x double> @_Z6selectDv8_dS_Dv8_l(<8 x double> %693, <8 x double> %694, <8 x i64> %695) nounwind readnone
  store <8 x double> %call419, <8 x double>* %a8_out, align 64
  %696 = load <16 x double>* %a16_in, align 128
  %697 = load <16 x double>* %b16_in, align 128
  %698 = load <16 x i64>* %l16_in, align 128
  %call420 = call <16 x double> @_Z6selectDv16_dS_Dv16_l(<16 x double> %696, <16 x double> %697, <16 x i64> %698) nounwind readnone
  store <16 x double> %call420, <16 x double>* %a16_out, align 128
  %699 = load double* %a_in, align 8
  %700 = load double* %b_in, align 8
  %701 = load i64* %ul_in, align 8
  %call421 = call double @_Z6selectddm(double %699, double %700, i64 %701) nounwind readnone
  store double %call421, double* %a_out, align 8
  %702 = load <4 x double>* %a4_in, align 32
  %703 = load <4 x double>* %b4_in, align 32
  %704 = load <4 x i64>* %ul4_in, align 32
  %call422 = call <4 x double> @_Z6selectDv4_dS_Dv4_m(<4 x double> %702, <4 x double> %703, <4 x i64> %704) nounwind readnone
  store <4 x double> %call422, <4 x double>* %a4_out, align 32
  %705 = load <8 x double>* %a8_in, align 64
  %706 = load <8 x double>* %b8_in, align 64
  %707 = load <8 x i64>* %ul8_in, align 64
  %call423 = call <8 x double> @_Z6selectDv8_dS_Dv8_m(<8 x double> %705, <8 x double> %706, <8 x i64> %707) nounwind readnone
  store <8 x double> %call423, <8 x double>* %a8_out, align 64
  %708 = load <16 x double>* %a16_in, align 128
  %709 = load <16 x double>* %b16_in, align 128
  %710 = load <16 x i64>* %ul16_in, align 128
  %call424 = call <16 x double> @_Z6selectDv16_dS_Dv16_m(<16 x double> %708, <16 x double> %709, <16 x i64> %710) nounwind readnone
  store <16 x double> %call424, <16 x double>* %a16_out, align 128
  %711 = load double* %a_in, align 8
  %712 = load double* %b_in, align 8
  %call425 = call double @_Z9remainderdd(double %711, double %712) nounwind readnone
  store double %call425, double* %a_out, align 8
  %713 = load <4 x double>* %a4_in, align 32
  %714 = load <4 x double>* %b4_in, align 32
  %call426 = call <4 x double> @_Z9remainderDv4_dS_(<4 x double> %713, <4 x double> %714) nounwind readnone
  store <4 x double> %call426, <4 x double>* %a4_out, align 32
  %715 = load <8 x double>* %a8_in, align 64
  %716 = load <8 x double>* %b8_in, align 64
  %call427 = call <8 x double> @_Z9remainderDv8_dS_(<8 x double> %715, <8 x double> %716) nounwind readnone
  store <8 x double> %call427, <8 x double>* %a8_out, align 64
  %717 = load <16 x double>* %a16_in, align 128
  %718 = load <16 x double>* %b16_in, align 128
  %call428 = call <16 x double> @_Z9remainderDv16_dS_(<16 x double> %717, <16 x double> %718) nounwind readnone
  store <16 x double> %call428, <16 x double>* %a16_out, align 128
  %719 = load double* %a_in, align 8
  %720 = load double* %b_in, align 8
  %call429 = call double @_Z6remquoddPi(double %719, double %720, i32* %i_out)
  store double %call429, double* %a_out, align 8
  %721 = load <2 x double>* %a2_in, align 16
  %722 = load <2 x double>* %b2_in, align 16
  %call430 = call <2 x double> @_Z6remquoDv2_dS_PDv2_i(<2 x double> %721, <2 x double> %722, <2 x i32>* %i2_out)
  store <2 x double> %call430, <2 x double>* %a2_out, align 16
  %723 = load <3 x double>* %a3_in, align 32
  %724 = load <3 x double>* %b3_in, align 32
  %call431 = call <3 x double> @_Z6remquoDv3_dS_PDv3_i(<3 x double> %723, <3 x double> %724, <3 x i32>* %i3_out)
  store <3 x double> %call431, <3 x double>* %a3_out, align 32
  %725 = load <4 x double>* %a4_in, align 32
  %726 = load <4 x double>* %b4_in, align 32
  %call432 = call <4 x double> @_Z6remquoDv4_dS_PDv4_i(<4 x double> %725, <4 x double> %726, <4 x i32>* %i4_out)
  store <4 x double> %call432, <4 x double>* %a4_out, align 32
  %727 = load <8 x double>* %a8_in, align 64
  %728 = load <8 x double>* %b8_in, align 64
  %call433 = call <8 x double> @_Z6remquoDv8_dS_PDv8_i(<8 x double> %727, <8 x double> %728, <8 x i32>* %i8_out)
  store <8 x double> %call433, <8 x double>* %a8_out, align 64
  %729 = load <16 x double>* %a16_in, align 128
  %730 = load <16 x double>* %b16_in, align 128
  %call434 = call <16 x double> @_Z6remquoDv16_dS_PDv16_i(<16 x double> %729, <16 x double> %730, <16 x i32>* %i16_out)
  store <16 x double> %call434, <16 x double>* %a16_out, align 128
  ret void
}

declare double @_Z4acosd(double) nounwind readnone

declare <4 x double> @_Z4acosDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4acosDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4acosDv16_d(<16 x double>) nounwind readnone

declare double @_Z6acospid(double) nounwind readnone

declare <4 x double> @_Z6acospiDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z6acospiDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z6acospiDv16_d(<16 x double>) nounwind readnone

declare double @_Z4asind(double) nounwind readnone

declare <4 x double> @_Z4asinDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4asinDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4asinDv16_d(<16 x double>) nounwind readnone

declare double @_Z6asinpid(double) nounwind readnone

declare <4 x double> @_Z6asinpiDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z6asinpiDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z6asinpiDv16_d(<16 x double>) nounwind readnone

declare double @_Z4atand(double) nounwind readnone

declare <4 x double> @_Z4atanDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4atanDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4atanDv16_d(<16 x double>) nounwind readnone

declare double @_Z5atan2dd(double, double) nounwind readnone

declare <4 x double> @_Z5atan2Dv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z5atan2Dv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z5atan2Dv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z7atan2pidd(double, double) nounwind readnone

declare <4 x double> @_Z7atan2piDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z7atan2piDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z7atan2piDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z6atanpid(double) nounwind readnone

declare <4 x double> @_Z6atanpiDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z6atanpiDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z6atanpiDv16_d(<16 x double>) nounwind readnone

declare double @_Z3cosd(double) nounwind readnone

declare <4 x double> @_Z3cosDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z3cosDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z3cosDv16_d(<16 x double>) nounwind readnone

declare double @_Z4coshd(double) nounwind readnone

declare <4 x double> @_Z4coshDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4coshDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4coshDv16_d(<16 x double>) nounwind readnone

declare double @_Z5cospid(double) nounwind readnone

declare <4 x double> @_Z5cospiDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5cospiDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5cospiDv16_d(<16 x double>) nounwind readnone

declare double @_Z3expd(double) nounwind readnone

declare <4 x double> @_Z3expDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z3expDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z3expDv16_d(<16 x double>) nounwind readnone

declare double @_Z4exp2d(double) nounwind readnone

declare <4 x double> @_Z4exp2Dv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4exp2Dv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4exp2Dv16_d(<16 x double>) nounwind readnone

declare double @_Z5exp10d(double) nounwind readnone

declare <4 x double> @_Z5exp10Dv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5exp10Dv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5exp10Dv16_d(<16 x double>) nounwind readnone

declare double @_Z5expm1d(double) nounwind readnone

declare <4 x double> @_Z5expm1Dv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5expm1Dv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5expm1Dv16_d(<16 x double>) nounwind readnone

declare double @_Z3logd(double) nounwind readnone

declare <4 x double> @_Z3logDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z3logDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z3logDv16_d(<16 x double>) nounwind readnone

declare double @_Z4log2d(double) nounwind readnone

declare <4 x double> @_Z4log2Dv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4log2Dv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4log2Dv16_d(<16 x double>) nounwind readnone

declare double @_Z5log10d(double) nounwind readnone

declare <4 x double> @_Z5log10Dv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5log10Dv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5log10Dv16_d(<16 x double>) nounwind readnone

declare double @_Z5log1pd(double) nounwind readnone

declare <4 x double> @_Z5log1pDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5log1pDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5log1pDv16_d(<16 x double>) nounwind readnone

declare double @_Z4logbd(double) nounwind readnone

declare <4 x double> @_Z4logbDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4logbDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4logbDv16_d(<16 x double>) nounwind readnone

declare double @_Z4ceild(double) nounwind readnone

declare <4 x double> @_Z4ceilDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4ceilDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4ceilDv16_d(<16 x double>) nounwind readnone

declare double @_Z3powdd(double, double) nounwind readnone

declare <4 x double> @_Z3powDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z3powDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z3powDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z5clampddd(double, double, double) nounwind readnone

declare <4 x double> @_Z5clampDv4_dS_S_(<4 x double>, <4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z5clampDv8_dS_S_(<8 x double>, <8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z5clampDv16_dS_S_(<16 x double>, <16 x double>, <16 x double>) nounwind readnone

declare <4 x double> @_Z5clampDv4_ddd(<4 x double>, double, double) nounwind readnone

declare <8 x double> @_Z5clampDv8_ddd(<8 x double>, double, double) nounwind readnone

declare <16 x double> @_Z5clampDv16_ddd(<16 x double>, double, double) nounwind readnone

declare double @_Z4sinhd(double) nounwind readnone

declare <4 x double> @_Z4sinhDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4sinhDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4sinhDv16_d(<16 x double>) nounwind readnone

declare double @_Z3sind(double) nounwind readnone

declare <4 x double> @_Z3sinDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z3sinDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z3sinDv16_d(<16 x double>) nounwind readnone

declare double @_Z5sinpid(double) nounwind readnone

declare <4 x double> @_Z5sinpiDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5sinpiDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5sinpiDv16_d(<16 x double>) nounwind readnone

declare double @_Z4sqrtd(double) nounwind readnone

declare <4 x double> @_Z4sqrtDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4sqrtDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4sqrtDv16_d(<16 x double>) nounwind readnone

declare double @_Z5rsqrtd(double) nounwind readnone

declare <4 x double> @_Z5rsqrtDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5rsqrtDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5rsqrtDv16_d(<16 x double>) nounwind readnone

declare double @_Z3tand(double) nounwind readnone

declare <4 x double> @_Z3tanDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z3tanDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z3tanDv16_d(<16 x double>) nounwind readnone

declare double @_Z4tanhd(double) nounwind readnone

declare <4 x double> @_Z4tanhDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4tanhDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4tanhDv16_d(<16 x double>) nounwind readnone

declare double @_Z5tanpid(double) nounwind readnone

declare <4 x double> @_Z5tanpiDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5tanpiDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5tanpiDv16_d(<16 x double>) nounwind readnone

declare double @_Z4fabsd(double) nounwind readnone

declare <4 x double> @_Z4fabsDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4fabsDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4fabsDv16_d(<16 x double>) nounwind readnone

declare double @_Z5asinhd(double) nounwind readnone

declare <4 x double> @_Z5asinhDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5asinhDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5asinhDv16_d(<16 x double>) nounwind readnone

declare double @_Z5acoshd(double) nounwind readnone

declare <4 x double> @_Z5acoshDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5acoshDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5acoshDv16_d(<16 x double>) nounwind readnone

declare double @_Z5atanhd(double) nounwind readnone

declare <4 x double> @_Z5atanhDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5atanhDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5atanhDv16_d(<16 x double>) nounwind readnone

declare <4 x double> @_Z6vload4jPKd(i32, double*)

declare <8 x double> @_Z6vload8jPKd(i32, double*)

declare <16 x double> @_Z7vload16jPKd(i32, double*)

declare void @_Z7vstore4Dv4_djPd(<4 x double>, i32, double*)

declare void @_Z7vstore8Dv8_djPd(<8 x double>, i32, double*)

declare void @_Z8vstore16Dv16_djPd(<16 x double>, i32, double*)

declare double @_Z3mindd(double, double) nounwind readnone

declare <4 x double> @_Z3minDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z3minDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z3minDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare <4 x double> @_Z3minDv4_dd(<4 x double>, double) nounwind readnone

declare <8 x double> @_Z3minDv8_dd(<8 x double>, double) nounwind readnone

declare <16 x double> @_Z3minDv16_dd(<16 x double>, double) nounwind readnone

declare double @_Z3maxdd(double, double) nounwind readnone

declare <4 x double> @_Z3maxDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z3maxDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z3maxDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare <4 x double> @_Z3maxDv4_dd(<4 x double>, double) nounwind readnone

declare <8 x double> @_Z3maxDv8_dd(<8 x double>, double) nounwind readnone

declare <16 x double> @_Z3maxDv16_dd(<16 x double>, double) nounwind readnone

declare double @_Z5hypotdd(double, double) nounwind readnone

declare <4 x double> @_Z5hypotDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z5hypotDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z5hypotDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z4stepdd(double, double) nounwind readnone

declare <4 x double> @_Z4stepDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z4stepDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z4stepDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare <4 x double> @_Z4stepdDv4_d(double, <4 x double>) nounwind readnone

declare <8 x double> @_Z4stepdDv8_d(double, <8 x double>) nounwind readnone

declare <16 x double> @_Z4stepdDv16_d(double, <16 x double>) nounwind readnone

declare double @_Z10smoothstepddd(double, double, double) nounwind readnone

declare <4 x double> @_Z10smoothstepDv4_dS_S_(<4 x double>, <4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z10smoothstepDv8_dS_S_(<8 x double>, <8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z10smoothstepDv16_dS_S_(<16 x double>, <16 x double>, <16 x double>) nounwind readnone

declare <4 x double> @_Z10smoothstepddDv4_d(double, double, <4 x double>) nounwind readnone

declare <8 x double> @_Z10smoothstepddDv8_d(double, double, <8 x double>) nounwind readnone

declare <16 x double> @_Z10smoothstepddDv16_d(double, double, <16 x double>) nounwind readnone

declare double @_Z7radiansd(double) nounwind readnone

declare <4 x double> @_Z7radiansDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z7radiansDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z7radiansDv16_d(<16 x double>) nounwind readnone

declare double @_Z7degreesd(double) nounwind readnone

declare <4 x double> @_Z7degreesDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z7degreesDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z7degreesDv16_d(<16 x double>) nounwind readnone

declare double @_Z4signd(double) nounwind readnone

declare <4 x double> @_Z4signDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4signDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4signDv16_d(<16 x double>) nounwind readnone

declare double @_Z5floord(double) nounwind readnone

declare <4 x double> @_Z5floorDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5floorDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5floorDv16_d(<16 x double>) nounwind readnone

declare double @_Z3dotdd(double, double) nounwind readnone

declare double @_Z3dotDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare double @_Z3mixddd(double, double, double) nounwind readnone

declare <4 x double> @_Z3mixDv4_dS_S_(<4 x double>, <4 x double>, <4 x double>) nounwind readnone

declare <4 x double> @_Z3mixDv4_dS_d(<4 x double>, <4 x double>, double) nounwind readnone

declare <8 x double> @_Z3mixDv8_dS_d(<8 x double>, <8 x double>, double) nounwind readnone

declare <16 x double> @_Z3mixDv16_dS_d(<16 x double>, <16 x double>, double) nounwind readnone

declare <8 x double> @_Z3mixDv8_dS_S_(<8 x double>, <8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z3mixDv16_dS_S_(<16 x double>, <16 x double>, <16 x double>) nounwind readnone

declare double @_Z9normalized(double) nounwind readnone

declare <4 x double> @_Z9normalizeDv4_d(<4 x double>) nounwind readnone

declare <4 x double> @_Z5crossDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare double @_Z6lengthd(double) nounwind readnone

declare double @_Z6lengthDv2_d(<2 x double>) nounwind readnone

declare double @_Z6lengthDv4_d(<4 x double>) nounwind readnone

declare double @_Z8distancedd(double, double) nounwind readnone

declare double @_Z8distanceDv2_dS_(<2 x double>, <2 x double>) nounwind readnone

declare double @_Z8distanceDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare double @_Z5rootndi(double, i32) nounwind readnone

declare <4 x double> @_Z5rootnDv4_dDv4_i(<4 x double>, <4 x i32>) nounwind readnone

declare <8 x double> @_Z5rootnDv8_dDv8_i(<8 x double>, <8 x i32>) nounwind readnone

declare <16 x double> @_Z5rootnDv16_dDv16_i(<16 x double>, <16 x i32>) nounwind readnone

declare double @_Z5ldexpdi(double, i32) nounwind readnone

declare <4 x double> @_Z5ldexpDv4_dDv4_i(<4 x double>, <4 x i32>) nounwind readnone

declare <8 x double> @_Z5ldexpDv8_dDv8_i(<8 x double>, <8 x i32>) nounwind readnone

declare <16 x double> @_Z5ldexpDv16_dDv16_i(<16 x double>, <16 x i32>) nounwind readnone

declare <4 x double> @_Z5ldexpDv4_di(<4 x double>, i32) nounwind readnone

declare <8 x double> @_Z5ldexpDv8_di(<8 x double>, i32) nounwind readnone

declare <16 x double> @_Z5ldexpDv16_di(<16 x double>, i32) nounwind readnone

declare double @_Z4modfdPd(double, double*)

declare <4 x double> @_Z4modfDv4_dPS_(<4 x double>, <4 x double>*)

declare <8 x double> @_Z4modfDv8_dPS_(<8 x double>, <8 x double>*)

declare <16 x double> @_Z4modfDv16_dPS_(<16 x double>, <16 x double>*)

declare double @_Z5frexpdPi(double, i32*)

declare <4 x double> @_Z5frexpDv4_dPDv4_i(<4 x double>, <4 x i32>*)

declare <8 x double> @_Z5frexpDv8_dPDv8_i(<8 x double>, <8 x i32>*)

declare <16 x double> @_Z5frexpDv16_dPDv16_i(<16 x double>, <16 x i32>*)

declare double @_Z6maxmagdd(double, double) nounwind readnone

declare <4 x double> @_Z6maxmagDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z6maxmagDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z6maxmagDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z6minmagdd(double, double) nounwind readnone

declare <4 x double> @_Z6minmagDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z6minmagDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z6minmagDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z8copysigndd(double, double) nounwind readnone

declare <4 x double> @_Z8copysignDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z8copysignDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z8copysignDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z9nextafterdd(double, double) nounwind readnone

declare <4 x double> @_Z9nextafterDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z9nextafterDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z9nextafterDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z4fdimdd(double, double) nounwind readnone

declare <4 x double> @_Z4fdimDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z4fdimDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z4fdimDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z3fmaddd(double, double, double) nounwind readnone

declare <4 x double> @_Z3fmaDv4_dS_S_(<4 x double>, <4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z3fmaDv8_dS_S_(<8 x double>, <8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z3fmaDv16_dS_S_(<16 x double>, <16 x double>, <16 x double>) nounwind readnone

declare double @_Z3madddd(double, double, double) nounwind readnone

declare <4 x double> @_Z3madDv4_dS_S_(<4 x double>, <4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z3madDv8_dS_S_(<8 x double>, <8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z3madDv16_dS_S_(<16 x double>, <16 x double>, <16 x double>) nounwind readnone

declare double @_Z4rintd(double) nounwind readnone

declare <4 x double> @_Z4rintDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4rintDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4rintDv16_d(<16 x double>) nounwind readnone

declare double @_Z5roundd(double) nounwind readnone

declare <4 x double> @_Z5roundDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5roundDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5roundDv16_d(<16 x double>) nounwind readnone

declare double @_Z5truncd(double) nounwind readnone

declare <4 x double> @_Z5truncDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z5truncDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z5truncDv16_d(<16 x double>) nounwind readnone

declare double @_Z4cbrtd(double) nounwind readnone

declare <4 x double> @_Z4cbrtDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z4cbrtDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z4cbrtDv16_d(<16 x double>) nounwind readnone

declare double @_Z4powrdd(double, double) nounwind readnone

declare <4 x double> @_Z4powrDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z4powrDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z4powrDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z4fmoddd(double, double) nounwind readnone

declare <4 x double> @_Z4fmodDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z4fmodDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z4fmodDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z4fmindd(double, double) nounwind readnone

declare <4 x double> @_Z4fminDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z4fminDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z4fminDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z4fmaxdd(double, double) nounwind readnone

declare <4 x double> @_Z4fmaxDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z4fmaxDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z4fmaxDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare <4 x double> @_Z4fminDv4_dd(<4 x double>, double) nounwind readnone

declare <8 x double> @_Z4fminDv8_dd(<8 x double>, double) nounwind readnone

declare <16 x double> @_Z4fminDv16_dd(<16 x double>, double) nounwind readnone

declare <4 x double> @_Z4fmaxDv4_dd(<4 x double>, double) nounwind readnone

declare <8 x double> @_Z4fmaxDv8_dd(<8 x double>, double) nounwind readnone

declare <16 x double> @_Z4fmaxDv16_dd(<16 x double>, double) nounwind readnone

declare <4 x double> @_Z4pownDv4_dDv4_i(<4 x double>, <4 x i32>) nounwind readnone

declare <8 x double> @_Z4pownDv8_dDv8_i(<8 x double>, <8 x i32>) nounwind readnone

declare <16 x double> @_Z4pownDv16_dDv16_i(<16 x double>, <16 x i32>) nounwind readnone

declare i32 @_Z5ilogbd(double) nounwind readnone

declare <4 x i32> @_Z5ilogbDv4_d(<4 x double>) nounwind readnone

declare <8 x i32> @_Z5ilogbDv8_d(<8 x double>) nounwind readnone

declare <16 x i32> @_Z5ilogbDv16_d(<16 x double>) nounwind readnone

declare double @_Z3nanm(i64) nounwind readnone

declare <4 x double> @_Z3nanDv4_m(<4 x i64>) nounwind readnone

declare <8 x double> @_Z3nanDv8_m(<8 x i64>) nounwind readnone

declare <16 x double> @_Z3nanDv16_m(<16 x i64>) nounwind readnone

declare double @_Z5fractdPd(double, double*)

declare <4 x double> @_Z5fractDv4_dPS_(<4 x double>, <4 x double>*)

declare <8 x double> @_Z5fractDv8_dPS_(<8 x double>, <8 x double>*)

declare <16 x double> @_Z5fractDv16_dPS_(<16 x double>, <16 x double>*)

declare double @_Z6lgammad(double) nounwind readnone

declare <4 x double> @_Z6lgammaDv4_d(<4 x double>) nounwind readnone

declare <8 x double> @_Z6lgammaDv8_d(<8 x double>) nounwind readnone

declare <16 x double> @_Z6lgammaDv16_d(<16 x double>) nounwind readnone

declare double @_Z8lgamma_rdPi(double, i32*)

declare <4 x double> @_Z8lgamma_rDv4_dPDv4_i(<4 x double>, <4 x i32>*)

declare <8 x double> @_Z8lgamma_rDv8_dPDv8_i(<8 x double>, <8 x i32>*)

declare <16 x double> @_Z8lgamma_rDv16_dPDv16_i(<16 x double>, <16 x i32>*)

declare double @_Z9bitselectddd(double, double, double) nounwind readnone

declare <4 x double> @_Z9bitselectDv4_dS_S_(<4 x double>, <4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z9bitselectDv8_dS_S_(<8 x double>, <8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z9bitselectDv16_dS_S_(<16 x double>, <16 x double>, <16 x double>) nounwind readnone

declare double @_Z6selectddl(double, double, i64) nounwind readnone

declare <4 x double> @_Z6selectDv4_dS_Dv4_l(<4 x double>, <4 x double>, <4 x i64>) nounwind readnone

declare <8 x double> @_Z6selectDv8_dS_Dv8_l(<8 x double>, <8 x double>, <8 x i64>) nounwind readnone

declare <16 x double> @_Z6selectDv16_dS_Dv16_l(<16 x double>, <16 x double>, <16 x i64>) nounwind readnone

declare double @_Z6selectddm(double, double, i64) nounwind readnone

declare <4 x double> @_Z6selectDv4_dS_Dv4_m(<4 x double>, <4 x double>, <4 x i64>) nounwind readnone

declare <8 x double> @_Z6selectDv8_dS_Dv8_m(<8 x double>, <8 x double>, <8 x i64>) nounwind readnone

declare <16 x double> @_Z6selectDv16_dS_Dv16_m(<16 x double>, <16 x double>, <16 x i64>) nounwind readnone

declare double @_Z9remainderdd(double, double) nounwind readnone

declare <4 x double> @_Z9remainderDv4_dS_(<4 x double>, <4 x double>) nounwind readnone

declare <8 x double> @_Z9remainderDv8_dS_(<8 x double>, <8 x double>) nounwind readnone

declare <16 x double> @_Z9remainderDv16_dS_(<16 x double>, <16 x double>) nounwind readnone

declare double @_Z6remquoddPi(double, double, i32*)

declare <2 x double> @_Z6remquoDv2_dS_PDv2_i(<2 x double>, <2 x double>, <2 x i32>*)

declare <3 x double> @_Z6remquoDv3_dS_PDv3_i(<3 x double>, <3 x double>, <3 x i32>*)

declare <4 x double> @_Z6remquoDv4_dS_PDv4_i(<4 x double>, <4 x double>, <4 x i32>*)

declare <8 x double> @_Z6remquoDv8_dS_PDv8_i(<8 x double>, <8 x double>, <8 x i32>*)

declare <16 x double> @_Z6remquoDv16_dS_PDv16_i(<16 x double>, <16 x double>, <16 x i32>*)

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32)* @oclbuiltin, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
