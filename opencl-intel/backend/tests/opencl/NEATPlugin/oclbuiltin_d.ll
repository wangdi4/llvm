; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN_XXX: NEATChecker -r %s -a %s.neat -t 0
; TODO: add NEATCHECKER instrumentation

; ModuleID = 'oclbuiltin_d.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @oclbuiltin(double addrspace(1)* %input, double addrspace(1)* %output, i32 addrspace(1)* %inputInt, i32 addrspace(1)* %outputInt, i64 addrspace(1)* %inputUlong, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca double addrspace(1)*, align 4
  %output.addr = alloca double addrspace(1)*, align 4
  %inputInt.addr = alloca i32 addrspace(1)*, align 4
  %outputInt.addr = alloca i32 addrspace(1)*, align 4
  %inputUlong.addr = alloca i64 addrspace(1)*, align 4
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
  store i64 addrspace(1)* %inputUlong, i64 addrspace(1)** %inputUlong.addr, align 4
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
  %184 = load i32* %tid, align 4
  %185 = load i64 addrspace(1)** %inputUlong.addr, align 4
  %arrayidx88 = getelementptr inbounds i64 addrspace(1)* %185, i32 %184
  %186 = load i64 addrspace(1)* %arrayidx88
  store i64 %186, i64* %ul_in, align 8
  %187 = load i32* %tid, align 4
  %188 = load i64 addrspace(1)** %inputUlong.addr, align 4
  %arrayidx89 = getelementptr inbounds i64 addrspace(1)* %188, i32 %187
  %189 = load i64 addrspace(1)* %arrayidx89
  %190 = insertelement <2 x i64> undef, i64 %189, i32 0
  %splat90 = shufflevector <2 x i64> %190, <2 x i64> %190, <2 x i32> zeroinitializer
  store <2 x i64> %splat90, <2 x i64>* %ul2_in, align 16
  %191 = load i32* %tid, align 4
  %192 = load i64 addrspace(1)** %inputUlong.addr, align 4
  %arrayidx91 = getelementptr inbounds i64 addrspace(1)* %192, i32 %191
  %193 = load i64 addrspace(1)* %arrayidx91
  %194 = insertelement <3 x i64> undef, i64 %193, i32 0
  %splat92 = shufflevector <3 x i64> %194, <3 x i64> %194, <3 x i32> zeroinitializer
  store <3 x i64> %splat92, <3 x i64>* %ul3_in, align 32
  %195 = load i32* %tid, align 4
  %196 = load i64 addrspace(1)** %inputUlong.addr, align 4
  %arrayidx93 = getelementptr inbounds i64 addrspace(1)* %196, i32 %195
  %197 = load i64 addrspace(1)* %arrayidx93
  %198 = insertelement <4 x i64> undef, i64 %197, i32 0
  %splat94 = shufflevector <4 x i64> %198, <4 x i64> %198, <4 x i32> zeroinitializer
  store <4 x i64> %splat94, <4 x i64>* %ul4_in, align 32
  %199 = load i32* %tid, align 4
  %200 = load i64 addrspace(1)** %inputUlong.addr, align 4
  %arrayidx95 = getelementptr inbounds i64 addrspace(1)* %200, i32 %199
  %201 = load i64 addrspace(1)* %arrayidx95
  %202 = insertelement <8 x i64> undef, i64 %201, i32 0
  %splat96 = shufflevector <8 x i64> %202, <8 x i64> %202, <8 x i32> zeroinitializer
  store <8 x i64> %splat96, <8 x i64>* %ul8_in, align 64
  %203 = load i32* %tid, align 4
  %204 = load i64 addrspace(1)** %inputUlong.addr, align 4
  %arrayidx97 = getelementptr inbounds i64 addrspace(1)* %204, i32 %203
  %205 = load i64 addrspace(1)* %arrayidx97
  %206 = insertelement <16 x i64> undef, i64 %205, i32 0
  %splat98 = shufflevector <16 x i64> %206, <16 x i64> %206, <16 x i32> zeroinitializer
  store <16 x i64> %splat98, <16 x i64>* %ul16_in, align 128
  %207 = load double* %a_in, align 8
  %call = call double @_Z4acosd(double %207) nounwind readnone
  store double %call, double* %a_out, align 8
  %208 = load <4 x double>* %a4_in, align 32
  %call99 = call <4 x double> @_Z4acosDv4_d(<4 x double> %208) nounwind readnone
  store <4 x double> %call99, <4 x double>* %a4_out, align 32
  %209 = load <8 x double>* %a8_in, align 64
  %call100 = call <8 x double> @_Z4acosDv8_d(<8 x double> %209) nounwind readnone
  store <8 x double> %call100, <8 x double>* %a8_out, align 64
  %210 = load <16 x double>* %a16_in, align 128
  %call101 = call <16 x double> @_Z4acosDv16_d(<16 x double> %210) nounwind readnone
  store <16 x double> %call101, <16 x double>* %a16_out, align 128
  %211 = load double* %a_in, align 8
  %call102 = call double @_Z6acospid(double %211) nounwind readnone
  store double %call102, double* %a_out, align 8
  %212 = load <4 x double>* %a4_in, align 32
  %call103 = call <4 x double> @_Z6acospiDv4_d(<4 x double> %212) nounwind readnone
  store <4 x double> %call103, <4 x double>* %a4_out, align 32
  %213 = load <8 x double>* %a8_in, align 64
  %call104 = call <8 x double> @_Z6acospiDv8_d(<8 x double> %213) nounwind readnone
  store <8 x double> %call104, <8 x double>* %a8_out, align 64
  %214 = load <16 x double>* %a16_in, align 128
  %call105 = call <16 x double> @_Z6acospiDv16_d(<16 x double> %214) nounwind readnone
  store <16 x double> %call105, <16 x double>* %a16_out, align 128
  %215 = load double* %a_in, align 8
  %call106 = call double @_Z4asind(double %215) nounwind readnone
  store double %call106, double* %a_out, align 8
  %216 = load <4 x double>* %a4_in, align 32
  %call107 = call <4 x double> @_Z4asinDv4_d(<4 x double> %216) nounwind readnone
  store <4 x double> %call107, <4 x double>* %a4_out, align 32
  %217 = load <8 x double>* %a8_in, align 64
  %call108 = call <8 x double> @_Z4asinDv8_d(<8 x double> %217) nounwind readnone
  store <8 x double> %call108, <8 x double>* %a8_out, align 64
  %218 = load <16 x double>* %a16_in, align 128
  %call109 = call <16 x double> @_Z4asinDv16_d(<16 x double> %218) nounwind readnone
  store <16 x double> %call109, <16 x double>* %a16_out, align 128
  %219 = load double* %a_in, align 8
  %call110 = call double @_Z6asinpid(double %219) nounwind readnone
  store double %call110, double* %a_out, align 8
  %220 = load <4 x double>* %a4_in, align 32
  %call111 = call <4 x double> @_Z6asinpiDv4_d(<4 x double> %220) nounwind readnone
  store <4 x double> %call111, <4 x double>* %a4_out, align 32
  %221 = load <8 x double>* %a8_in, align 64
  %call112 = call <8 x double> @_Z6asinpiDv8_d(<8 x double> %221) nounwind readnone
  store <8 x double> %call112, <8 x double>* %a8_out, align 64
  %222 = load <16 x double>* %a16_in, align 128
  %call113 = call <16 x double> @_Z6asinpiDv16_d(<16 x double> %222) nounwind readnone
  store <16 x double> %call113, <16 x double>* %a16_out, align 128
  %223 = load double* %a_in, align 8
  %call114 = call double @_Z4atand(double %223) nounwind readnone
  store double %call114, double* %a_out, align 8
  %224 = load <4 x double>* %a4_in, align 32
  %call115 = call <4 x double> @_Z4atanDv4_d(<4 x double> %224) nounwind readnone
  store <4 x double> %call115, <4 x double>* %a4_out, align 32
  %225 = load <8 x double>* %a8_in, align 64
  %call116 = call <8 x double> @_Z4atanDv8_d(<8 x double> %225) nounwind readnone
  store <8 x double> %call116, <8 x double>* %a8_out, align 64
  %226 = load <16 x double>* %a16_in, align 128
  %call117 = call <16 x double> @_Z4atanDv16_d(<16 x double> %226) nounwind readnone
  store <16 x double> %call117, <16 x double>* %a16_out, align 128
  %227 = load double* %a_in, align 8
  %228 = load double* %b_in, align 8
  %call118 = call double @_Z5atan2dd(double %227, double %228) nounwind readnone
  store double %call118, double* %a_out, align 8
  %229 = load <4 x double>* %a4_in, align 32
  %230 = load <4 x double>* %b4_in, align 32
  %call119 = call <4 x double> @_Z5atan2Dv4_dS_(<4 x double> %229, <4 x double> %230) nounwind readnone
  store <4 x double> %call119, <4 x double>* %a4_out, align 32
  %231 = load <8 x double>* %a8_in, align 64
  %232 = load <8 x double>* %b8_in, align 64
  %call120 = call <8 x double> @_Z5atan2Dv8_dS_(<8 x double> %231, <8 x double> %232) nounwind readnone
  store <8 x double> %call120, <8 x double>* %a8_out, align 64
  %233 = load <16 x double>* %a16_in, align 128
  %234 = load <16 x double>* %b16_in, align 128
  %call121 = call <16 x double> @_Z5atan2Dv16_dS_(<16 x double> %233, <16 x double> %234) nounwind readnone
  store <16 x double> %call121, <16 x double>* %a16_out, align 128
  %235 = load double* %a_in, align 8
  %236 = load double* %b_in, align 8
  %call122 = call double @_Z7atan2pidd(double %235, double %236) nounwind readnone
  store double %call122, double* %a_out, align 8
  %237 = load <4 x double>* %a4_in, align 32
  %238 = load <4 x double>* %b4_in, align 32
  %call123 = call <4 x double> @_Z7atan2piDv4_dS_(<4 x double> %237, <4 x double> %238) nounwind readnone
  store <4 x double> %call123, <4 x double>* %a4_out, align 32
  %239 = load <8 x double>* %a8_in, align 64
  %240 = load <8 x double>* %b8_in, align 64
  %call124 = call <8 x double> @_Z7atan2piDv8_dS_(<8 x double> %239, <8 x double> %240) nounwind readnone
  store <8 x double> %call124, <8 x double>* %a8_out, align 64
  %241 = load <16 x double>* %a16_in, align 128
  %242 = load <16 x double>* %b16_in, align 128
  %call125 = call <16 x double> @_Z7atan2piDv16_dS_(<16 x double> %241, <16 x double> %242) nounwind readnone
  store <16 x double> %call125, <16 x double>* %a16_out, align 128
  %243 = load double* %a_in, align 8
  %call126 = call double @_Z6atanpid(double %243) nounwind readnone
  store double %call126, double* %a_out, align 8
  %244 = load <4 x double>* %a4_in, align 32
  %call127 = call <4 x double> @_Z6atanpiDv4_d(<4 x double> %244) nounwind readnone
  store <4 x double> %call127, <4 x double>* %a4_out, align 32
  %245 = load <8 x double>* %a8_in, align 64
  %call128 = call <8 x double> @_Z6atanpiDv8_d(<8 x double> %245) nounwind readnone
  store <8 x double> %call128, <8 x double>* %a8_out, align 64
  %246 = load <16 x double>* %a16_in, align 128
  %call129 = call <16 x double> @_Z6atanpiDv16_d(<16 x double> %246) nounwind readnone
  store <16 x double> %call129, <16 x double>* %a16_out, align 128
  %247 = load double* %a_in, align 8
  %call130 = call double @_Z3cosd(double %247) nounwind readnone
  store double %call130, double* %a_out, align 8
  %248 = load <4 x double>* %a4_in, align 32
  %call131 = call <4 x double> @_Z3cosDv4_d(<4 x double> %248) nounwind readnone
  store <4 x double> %call131, <4 x double>* %a4_out, align 32
  %249 = load <8 x double>* %a8_in, align 64
  %call132 = call <8 x double> @_Z3cosDv8_d(<8 x double> %249) nounwind readnone
  store <8 x double> %call132, <8 x double>* %a8_out, align 64
  %250 = load <16 x double>* %a16_in, align 128
  %call133 = call <16 x double> @_Z3cosDv16_d(<16 x double> %250) nounwind readnone
  store <16 x double> %call133, <16 x double>* %a16_out, align 128
  %251 = load double* %a_in, align 8
  %call134 = call double @_Z4coshd(double %251) nounwind readnone
  store double %call134, double* %a_out, align 8
  %252 = load <4 x double>* %a4_in, align 32
  %call135 = call <4 x double> @_Z4coshDv4_d(<4 x double> %252) nounwind readnone
  store <4 x double> %call135, <4 x double>* %a4_out, align 32
  %253 = load <8 x double>* %a8_in, align 64
  %call136 = call <8 x double> @_Z4coshDv8_d(<8 x double> %253) nounwind readnone
  store <8 x double> %call136, <8 x double>* %a8_out, align 64
  %254 = load <16 x double>* %a16_in, align 128
  %call137 = call <16 x double> @_Z4coshDv16_d(<16 x double> %254) nounwind readnone
  store <16 x double> %call137, <16 x double>* %a16_out, align 128
  %255 = load double* %a_in, align 8
  %call138 = call double @_Z5cospid(double %255) nounwind readnone
  store double %call138, double* %a_out, align 8
  %256 = load <4 x double>* %a4_in, align 32
  %call139 = call <4 x double> @_Z5cospiDv4_d(<4 x double> %256) nounwind readnone
  store <4 x double> %call139, <4 x double>* %a4_out, align 32
  %257 = load <8 x double>* %a8_in, align 64
  %call140 = call <8 x double> @_Z5cospiDv8_d(<8 x double> %257) nounwind readnone
  store <8 x double> %call140, <8 x double>* %a8_out, align 64
  %258 = load <16 x double>* %a16_in, align 128
  %call141 = call <16 x double> @_Z5cospiDv16_d(<16 x double> %258) nounwind readnone
  store <16 x double> %call141, <16 x double>* %a16_out, align 128
  %259 = load double* %a_in, align 8
  %call142 = call double @_Z3expd(double %259) nounwind readnone
  store double %call142, double* %a_out, align 8
  %260 = load <4 x double>* %a4_in, align 32
  %call143 = call <4 x double> @_Z3expDv4_d(<4 x double> %260) nounwind readnone
  store <4 x double> %call143, <4 x double>* %a4_out, align 32
  %261 = load <8 x double>* %a8_in, align 64
  %call144 = call <8 x double> @_Z3expDv8_d(<8 x double> %261) nounwind readnone
  store <8 x double> %call144, <8 x double>* %a8_out, align 64
  %262 = load <16 x double>* %a16_in, align 128
  %call145 = call <16 x double> @_Z3expDv16_d(<16 x double> %262) nounwind readnone
  store <16 x double> %call145, <16 x double>* %a16_out, align 128
  %263 = load double* %a_in, align 8
  %call146 = call double @_Z4exp2d(double %263) nounwind readnone
  store double %call146, double* %a_out, align 8
  %264 = load <4 x double>* %a4_in, align 32
  %call147 = call <4 x double> @_Z4exp2Dv4_d(<4 x double> %264) nounwind readnone
  store <4 x double> %call147, <4 x double>* %a4_out, align 32
  %265 = load <8 x double>* %a8_in, align 64
  %call148 = call <8 x double> @_Z4exp2Dv8_d(<8 x double> %265) nounwind readnone
  store <8 x double> %call148, <8 x double>* %a8_out, align 64
  %266 = load <16 x double>* %a16_in, align 128
  %call149 = call <16 x double> @_Z4exp2Dv16_d(<16 x double> %266) nounwind readnone
  store <16 x double> %call149, <16 x double>* %a16_out, align 128
  %267 = load double* %a_in, align 8
  %call150 = call double @_Z5exp10d(double %267) nounwind readnone
  store double %call150, double* %a_out, align 8
  %268 = load <4 x double>* %a4_in, align 32
  %call151 = call <4 x double> @_Z5exp10Dv4_d(<4 x double> %268) nounwind readnone
  store <4 x double> %call151, <4 x double>* %a4_out, align 32
  %269 = load <8 x double>* %a8_in, align 64
  %call152 = call <8 x double> @_Z5exp10Dv8_d(<8 x double> %269) nounwind readnone
  store <8 x double> %call152, <8 x double>* %a8_out, align 64
  %270 = load <16 x double>* %a16_in, align 128
  %call153 = call <16 x double> @_Z5exp10Dv16_d(<16 x double> %270) nounwind readnone
  store <16 x double> %call153, <16 x double>* %a16_out, align 128
  %271 = load double* %a_in, align 8
  %call154 = call double @_Z5expm1d(double %271) nounwind readnone
  store double %call154, double* %a_out, align 8
  %272 = load <4 x double>* %a4_in, align 32
  %call155 = call <4 x double> @_Z5expm1Dv4_d(<4 x double> %272) nounwind readnone
  store <4 x double> %call155, <4 x double>* %a4_out, align 32
  %273 = load <8 x double>* %a8_in, align 64
  %call156 = call <8 x double> @_Z5expm1Dv8_d(<8 x double> %273) nounwind readnone
  store <8 x double> %call156, <8 x double>* %a8_out, align 64
  %274 = load <16 x double>* %a16_in, align 128
  %call157 = call <16 x double> @_Z5expm1Dv16_d(<16 x double> %274) nounwind readnone
  store <16 x double> %call157, <16 x double>* %a16_out, align 128
  %275 = load double* %a_in, align 8
  %call158 = call double @_Z3logd(double %275) nounwind readnone
  store double %call158, double* %a_out, align 8
  %276 = load <4 x double>* %a4_in, align 32
  %call159 = call <4 x double> @_Z3logDv4_d(<4 x double> %276) nounwind readnone
  store <4 x double> %call159, <4 x double>* %a4_out, align 32
  %277 = load <8 x double>* %a8_in, align 64
  %call160 = call <8 x double> @_Z3logDv8_d(<8 x double> %277) nounwind readnone
  store <8 x double> %call160, <8 x double>* %a8_out, align 64
  %278 = load <16 x double>* %a16_in, align 128
  %call161 = call <16 x double> @_Z3logDv16_d(<16 x double> %278) nounwind readnone
  store <16 x double> %call161, <16 x double>* %a16_out, align 128
  %279 = load double* %a_in, align 8
  %call162 = call double @_Z4log2d(double %279) nounwind readnone
  store double %call162, double* %a_out, align 8
  %280 = load <4 x double>* %a4_in, align 32
  %call163 = call <4 x double> @_Z4log2Dv4_d(<4 x double> %280) nounwind readnone
  store <4 x double> %call163, <4 x double>* %a4_out, align 32
  %281 = load <8 x double>* %a8_in, align 64
  %call164 = call <8 x double> @_Z4log2Dv8_d(<8 x double> %281) nounwind readnone
  store <8 x double> %call164, <8 x double>* %a8_out, align 64
  %282 = load <16 x double>* %a16_in, align 128
  %call165 = call <16 x double> @_Z4log2Dv16_d(<16 x double> %282) nounwind readnone
  store <16 x double> %call165, <16 x double>* %a16_out, align 128
  %283 = load double* %a_in, align 8
  %call166 = call double @_Z5log10d(double %283) nounwind readnone
  store double %call166, double* %a_out, align 8
  %284 = load <4 x double>* %a4_in, align 32
  %call167 = call <4 x double> @_Z5log10Dv4_d(<4 x double> %284) nounwind readnone
  store <4 x double> %call167, <4 x double>* %a4_out, align 32
  %285 = load <8 x double>* %a8_in, align 64
  %call168 = call <8 x double> @_Z5log10Dv8_d(<8 x double> %285) nounwind readnone
  store <8 x double> %call168, <8 x double>* %a8_out, align 64
  %286 = load <16 x double>* %a16_in, align 128
  %call169 = call <16 x double> @_Z5log10Dv16_d(<16 x double> %286) nounwind readnone
  store <16 x double> %call169, <16 x double>* %a16_out, align 128
  %287 = load double* %a_in, align 8
  %call170 = call double @_Z5log1pd(double %287) nounwind readnone
  store double %call170, double* %a_out, align 8
  %288 = load <4 x double>* %a4_in, align 32
  %call171 = call <4 x double> @_Z5log1pDv4_d(<4 x double> %288) nounwind readnone
  store <4 x double> %call171, <4 x double>* %a4_out, align 32
  %289 = load <8 x double>* %a8_in, align 64
  %call172 = call <8 x double> @_Z5log1pDv8_d(<8 x double> %289) nounwind readnone
  store <8 x double> %call172, <8 x double>* %a8_out, align 64
  %290 = load <16 x double>* %a16_in, align 128
  %call173 = call <16 x double> @_Z5log1pDv16_d(<16 x double> %290) nounwind readnone
  store <16 x double> %call173, <16 x double>* %a16_out, align 128
  %291 = load double* %a_in, align 8
  %call174 = call double @_Z4logbd(double %291) nounwind readnone
  store double %call174, double* %a_out, align 8
  %292 = load <4 x double>* %a4_in, align 32
  %call175 = call <4 x double> @_Z4logbDv4_d(<4 x double> %292) nounwind readnone
  store <4 x double> %call175, <4 x double>* %a4_out, align 32
  %293 = load <8 x double>* %a8_in, align 64
  %call176 = call <8 x double> @_Z4logbDv8_d(<8 x double> %293) nounwind readnone
  store <8 x double> %call176, <8 x double>* %a8_out, align 64
  %294 = load <16 x double>* %a16_in, align 128
  %call177 = call <16 x double> @_Z4logbDv16_d(<16 x double> %294) nounwind readnone
  store <16 x double> %call177, <16 x double>* %a16_out, align 128
  %295 = load double* %a_in, align 8
  %call178 = call double @_Z4ceild(double %295) nounwind readnone
  store double %call178, double* %a_out, align 8
  %296 = load <4 x double>* %a4_in, align 32
  %call179 = call <4 x double> @_Z4ceilDv4_d(<4 x double> %296) nounwind readnone
  store <4 x double> %call179, <4 x double>* %a4_out, align 32
  %297 = load <8 x double>* %a8_in, align 64
  %call180 = call <8 x double> @_Z4ceilDv8_d(<8 x double> %297) nounwind readnone
  store <8 x double> %call180, <8 x double>* %a8_out, align 64
  %298 = load <16 x double>* %a16_in, align 128
  %call181 = call <16 x double> @_Z4ceilDv16_d(<16 x double> %298) nounwind readnone
  store <16 x double> %call181, <16 x double>* %a16_out, align 128
  %299 = load double* %a_in, align 8
  %300 = load double* %b_in, align 8
  %call182 = call double @_Z3powdd(double %299, double %300) nounwind readnone
  store double %call182, double* %a_out, align 8
  %301 = load <4 x double>* %a4_in, align 32
  %302 = load <4 x double>* %b4_in, align 32
  %call183 = call <4 x double> @_Z3powDv4_dS_(<4 x double> %301, <4 x double> %302) nounwind readnone
  store <4 x double> %call183, <4 x double>* %a4_out, align 32
  %303 = load <8 x double>* %a8_in, align 64
  %304 = load <8 x double>* %b8_in, align 64
  %call184 = call <8 x double> @_Z3powDv8_dS_(<8 x double> %303, <8 x double> %304) nounwind readnone
  store <8 x double> %call184, <8 x double>* %a8_out, align 64
  %305 = load <16 x double>* %a16_in, align 128
  %306 = load <16 x double>* %b16_in, align 128
  %call185 = call <16 x double> @_Z3powDv16_dS_(<16 x double> %305, <16 x double> %306) nounwind readnone
  store <16 x double> %call185, <16 x double>* %a16_out, align 128
  %307 = load double* %a_in, align 8
  %308 = load double* %b_in, align 8
  %309 = load double* %c_in, align 8
  %call186 = call double @_Z5clampddd(double %307, double %308, double %309) nounwind readnone
  store double %call186, double* %a_out, align 8
  %310 = load <4 x double>* %a4_in, align 32
  %311 = load <4 x double>* %b4_in, align 32
  %312 = load <4 x double>* %c4_in, align 32
  %call187 = call <4 x double> @_Z5clampDv4_dS_S_(<4 x double> %310, <4 x double> %311, <4 x double> %312) nounwind readnone
  store <4 x double> %call187, <4 x double>* %a4_out, align 32
  %313 = load <8 x double>* %a8_in, align 64
  %314 = load <8 x double>* %b8_in, align 64
  %315 = load <8 x double>* %c8_in, align 64
  %call188 = call <8 x double> @_Z5clampDv8_dS_S_(<8 x double> %313, <8 x double> %314, <8 x double> %315) nounwind readnone
  store <8 x double> %call188, <8 x double>* %a8_out, align 64
  %316 = load <16 x double>* %a16_in, align 128
  %317 = load <16 x double>* %b16_in, align 128
  %318 = load <16 x double>* %c16_in, align 128
  %call189 = call <16 x double> @_Z5clampDv16_dS_S_(<16 x double> %316, <16 x double> %317, <16 x double> %318) nounwind readnone
  store <16 x double> %call189, <16 x double>* %a16_out, align 128
  %319 = load double* %a_in, align 8
  %320 = load double* %b_in, align 8
  %321 = load double* %c_in, align 8
  %call190 = call double @_Z5clampddd(double %319, double %320, double %321) nounwind readnone
  store double %call190, double* %a_out, align 8
  %322 = load <4 x double>* %a4_in, align 32
  %323 = load double* %b_in, align 8
  %324 = load double* %c_in, align 8
  %call191 = call <4 x double> @_Z5clampDv4_ddd(<4 x double> %322, double %323, double %324) nounwind readnone
  store <4 x double> %call191, <4 x double>* %a4_out, align 32
  %325 = load <8 x double>* %a8_in, align 64
  %326 = load double* %b_in, align 8
  %327 = load double* %c_in, align 8
  %call192 = call <8 x double> @_Z5clampDv8_ddd(<8 x double> %325, double %326, double %327) nounwind readnone
  store <8 x double> %call192, <8 x double>* %a8_out, align 64
  %328 = load <16 x double>* %a16_in, align 128
  %329 = load double* %b_in, align 8
  %330 = load double* %c_in, align 8
  %call193 = call <16 x double> @_Z5clampDv16_ddd(<16 x double> %328, double %329, double %330) nounwind readnone
  store <16 x double> %call193, <16 x double>* %a16_out, align 128
  %331 = load double* %a_in, align 8
  %call194 = call double @_Z4sinhd(double %331) nounwind readnone
  store double %call194, double* %a_out, align 8
  %332 = load <4 x double>* %a4_in, align 32
  %call195 = call <4 x double> @_Z4sinhDv4_d(<4 x double> %332) nounwind readnone
  store <4 x double> %call195, <4 x double>* %a4_out, align 32
  %333 = load <8 x double>* %a8_in, align 64
  %call196 = call <8 x double> @_Z4sinhDv8_d(<8 x double> %333) nounwind readnone
  store <8 x double> %call196, <8 x double>* %a8_out, align 64
  %334 = load <16 x double>* %a16_in, align 128
  %call197 = call <16 x double> @_Z4sinhDv16_d(<16 x double> %334) nounwind readnone
  store <16 x double> %call197, <16 x double>* %a16_out, align 128
  %335 = load double* %a_in, align 8
  %call198 = call double @_Z3sind(double %335) nounwind readnone
  store double %call198, double* %a_out, align 8
  %336 = load <4 x double>* %a4_in, align 32
  %call199 = call <4 x double> @_Z3sinDv4_d(<4 x double> %336) nounwind readnone
  store <4 x double> %call199, <4 x double>* %a4_out, align 32
  %337 = load <8 x double>* %a8_in, align 64
  %call200 = call <8 x double> @_Z3sinDv8_d(<8 x double> %337) nounwind readnone
  store <8 x double> %call200, <8 x double>* %a8_out, align 64
  %338 = load <16 x double>* %a16_in, align 128
  %call201 = call <16 x double> @_Z3sinDv16_d(<16 x double> %338) nounwind readnone
  store <16 x double> %call201, <16 x double>* %a16_out, align 128
  %339 = load double* %a_in, align 8
  %call202 = call double @_Z5sinpid(double %339) nounwind readnone
  store double %call202, double* %a_out, align 8
  %340 = load <4 x double>* %a4_in, align 32
  %call203 = call <4 x double> @_Z5sinpiDv4_d(<4 x double> %340) nounwind readnone
  store <4 x double> %call203, <4 x double>* %a4_out, align 32
  %341 = load <8 x double>* %a8_in, align 64
  %call204 = call <8 x double> @_Z5sinpiDv8_d(<8 x double> %341) nounwind readnone
  store <8 x double> %call204, <8 x double>* %a8_out, align 64
  %342 = load <16 x double>* %a16_in, align 128
  %call205 = call <16 x double> @_Z5sinpiDv16_d(<16 x double> %342) nounwind readnone
  store <16 x double> %call205, <16 x double>* %a16_out, align 128
  %343 = load double* %a_in, align 8
  %call206 = call double @_Z4sqrtd(double %343) nounwind readnone
  store double %call206, double* %a_out, align 8
  %344 = load <4 x double>* %a4_in, align 32
  %call207 = call <4 x double> @_Z4sqrtDv4_d(<4 x double> %344) nounwind readnone
  store <4 x double> %call207, <4 x double>* %a4_out, align 32
  %345 = load <8 x double>* %a8_in, align 64
  %call208 = call <8 x double> @_Z4sqrtDv8_d(<8 x double> %345) nounwind readnone
  store <8 x double> %call208, <8 x double>* %a8_out, align 64
  %346 = load <16 x double>* %a16_in, align 128
  %call209 = call <16 x double> @_Z4sqrtDv16_d(<16 x double> %346) nounwind readnone
  store <16 x double> %call209, <16 x double>* %a16_out, align 128
  %347 = load double* %a_in, align 8
  %call210 = call double @_Z5rsqrtd(double %347) nounwind readnone
  store double %call210, double* %a_out, align 8
  %348 = load <4 x double>* %a4_in, align 32
  %call211 = call <4 x double> @_Z5rsqrtDv4_d(<4 x double> %348) nounwind readnone
  store <4 x double> %call211, <4 x double>* %a4_out, align 32
  %349 = load <8 x double>* %a8_in, align 64
  %call212 = call <8 x double> @_Z5rsqrtDv8_d(<8 x double> %349) nounwind readnone
  store <8 x double> %call212, <8 x double>* %a8_out, align 64
  %350 = load <16 x double>* %a16_in, align 128
  %call213 = call <16 x double> @_Z5rsqrtDv16_d(<16 x double> %350) nounwind readnone
  store <16 x double> %call213, <16 x double>* %a16_out, align 128
  %351 = load double* %a_in, align 8
  %call214 = call double @_Z3tand(double %351) nounwind readnone
  store double %call214, double* %a_out, align 8
  %352 = load <4 x double>* %a4_in, align 32
  %call215 = call <4 x double> @_Z3tanDv4_d(<4 x double> %352) nounwind readnone
  store <4 x double> %call215, <4 x double>* %a4_out, align 32
  %353 = load <8 x double>* %a8_in, align 64
  %call216 = call <8 x double> @_Z3tanDv8_d(<8 x double> %353) nounwind readnone
  store <8 x double> %call216, <8 x double>* %a8_out, align 64
  %354 = load <16 x double>* %a16_in, align 128
  %call217 = call <16 x double> @_Z3tanDv16_d(<16 x double> %354) nounwind readnone
  store <16 x double> %call217, <16 x double>* %a16_out, align 128
  %355 = load double* %a_in, align 8
  %call218 = call double @_Z4tanhd(double %355) nounwind readnone
  store double %call218, double* %a_out, align 8
  %356 = load <4 x double>* %a4_in, align 32
  %call219 = call <4 x double> @_Z4tanhDv4_d(<4 x double> %356) nounwind readnone
  store <4 x double> %call219, <4 x double>* %a4_out, align 32
  %357 = load <8 x double>* %a8_in, align 64
  %call220 = call <8 x double> @_Z4tanhDv8_d(<8 x double> %357) nounwind readnone
  store <8 x double> %call220, <8 x double>* %a8_out, align 64
  %358 = load <16 x double>* %a16_in, align 128
  %call221 = call <16 x double> @_Z4tanhDv16_d(<16 x double> %358) nounwind readnone
  store <16 x double> %call221, <16 x double>* %a16_out, align 128
  %359 = load double* %a_in, align 8
  %call222 = call double @_Z5tanpid(double %359) nounwind readnone
  store double %call222, double* %a_out, align 8
  %360 = load <4 x double>* %a4_in, align 32
  %call223 = call <4 x double> @_Z5tanpiDv4_d(<4 x double> %360) nounwind readnone
  store <4 x double> %call223, <4 x double>* %a4_out, align 32
  %361 = load <8 x double>* %a8_in, align 64
  %call224 = call <8 x double> @_Z5tanpiDv8_d(<8 x double> %361) nounwind readnone
  store <8 x double> %call224, <8 x double>* %a8_out, align 64
  %362 = load <16 x double>* %a16_in, align 128
  %call225 = call <16 x double> @_Z5tanpiDv16_d(<16 x double> %362) nounwind readnone
  store <16 x double> %call225, <16 x double>* %a16_out, align 128
  %363 = load double* %a_in, align 8
  %call226 = call double @_Z4fabsd(double %363) nounwind readnone
  store double %call226, double* %a_out, align 8
  %364 = load <4 x double>* %a4_in, align 32
  %call227 = call <4 x double> @_Z4fabsDv4_d(<4 x double> %364) nounwind readnone
  store <4 x double> %call227, <4 x double>* %a4_out, align 32
  %365 = load <8 x double>* %a8_in, align 64
  %call228 = call <8 x double> @_Z4fabsDv8_d(<8 x double> %365) nounwind readnone
  store <8 x double> %call228, <8 x double>* %a8_out, align 64
  %366 = load <16 x double>* %a16_in, align 128
  %call229 = call <16 x double> @_Z4fabsDv16_d(<16 x double> %366) nounwind readnone
  store <16 x double> %call229, <16 x double>* %a16_out, align 128
  %367 = load double* %a_in, align 8
  %call230 = call double @_Z5asinhd(double %367) nounwind readnone
  store double %call230, double* %a_out, align 8
  %368 = load <4 x double>* %a4_in, align 32
  %call231 = call <4 x double> @_Z5asinhDv4_d(<4 x double> %368) nounwind readnone
  store <4 x double> %call231, <4 x double>* %a4_out, align 32
  %369 = load <8 x double>* %a8_in, align 64
  %call232 = call <8 x double> @_Z5asinhDv8_d(<8 x double> %369) nounwind readnone
  store <8 x double> %call232, <8 x double>* %a8_out, align 64
  %370 = load <16 x double>* %a16_in, align 128
  %call233 = call <16 x double> @_Z5asinhDv16_d(<16 x double> %370) nounwind readnone
  store <16 x double> %call233, <16 x double>* %a16_out, align 128
  %371 = load double* %a_in, align 8
  %call234 = call double @_Z5acoshd(double %371) nounwind readnone
  store double %call234, double* %a_out, align 8
  %372 = load <4 x double>* %a4_in, align 32
  %call235 = call <4 x double> @_Z5acoshDv4_d(<4 x double> %372) nounwind readnone
  store <4 x double> %call235, <4 x double>* %a4_out, align 32
  %373 = load <8 x double>* %a8_in, align 64
  %call236 = call <8 x double> @_Z5acoshDv8_d(<8 x double> %373) nounwind readnone
  store <8 x double> %call236, <8 x double>* %a8_out, align 64
  %374 = load <16 x double>* %a16_in, align 128
  %call237 = call <16 x double> @_Z5acoshDv16_d(<16 x double> %374) nounwind readnone
  store <16 x double> %call237, <16 x double>* %a16_out, align 128
  %375 = load double* %a_in, align 8
  %call238 = call double @_Z5atanhd(double %375) nounwind readnone
  store double %call238, double* %a_out, align 8
  %376 = load <4 x double>* %a4_in, align 32
  %call239 = call <4 x double> @_Z5atanhDv4_d(<4 x double> %376) nounwind readnone
  store <4 x double> %call239, <4 x double>* %a4_out, align 32
  %377 = load <8 x double>* %a8_in, align 64
  %call240 = call <8 x double> @_Z5atanhDv8_d(<8 x double> %377) nounwind readnone
  store <8 x double> %call240, <8 x double>* %a8_out, align 64
  %378 = load <16 x double>* %a16_in, align 128
  %call241 = call <16 x double> @_Z5atanhDv16_d(<16 x double> %378) nounwind readnone
  store <16 x double> %call241, <16 x double>* %a16_out, align 128
  %call242 = call <4 x double> @_Z6vload4jPKd(i32 0, double* %b_in)
  store <4 x double> %call242, <4 x double>* %a4_out, align 32
  %call243 = call <8 x double> @_Z6vload8jPKd(i32 0, double* %b_in)
  store <8 x double> %call243, <8 x double>* %a8_out, align 64
  %call244 = call <16 x double> @_Z7vload16jPKd(i32 0, double* %b_in)
  store <16 x double> %call244, <16 x double>* %a16_out, align 128
  %379 = load <4 x double>* %a4_in, align 32
  %380 = bitcast <4 x double>* %a4_out to double*
  call void @_Z7vstore4Dv4_djPd(<4 x double> %379, i32 0, double* %380)
  %381 = load <8 x double>* %a8_in, align 64
  %382 = bitcast <8 x double>* %a8_out to double*
  call void @_Z7vstore8Dv8_djPd(<8 x double> %381, i32 0, double* %382)
  %383 = load <16 x double>* %a16_in, align 128
  %384 = bitcast <16 x double>* %a16_out to double*
  call void @_Z8vstore16Dv16_djPd(<16 x double> %383, i32 0, double* %384)
  %385 = load double* %a_in, align 8
  %386 = load double* %b_in, align 8
  %call245 = call double @_Z3mindd(double %385, double %386) nounwind readnone
  store double %call245, double* %a_out, align 8
  %387 = load <4 x double>* %a4_in, align 32
  %388 = load <4 x double>* %b4_in, align 32
  %call246 = call <4 x double> @_Z3minDv4_dS_(<4 x double> %387, <4 x double> %388) nounwind readnone
  store <4 x double> %call246, <4 x double>* %a4_out, align 32
  %389 = load <8 x double>* %a8_in, align 64
  %390 = load <8 x double>* %b8_in, align 64
  %call247 = call <8 x double> @_Z3minDv8_dS_(<8 x double> %389, <8 x double> %390) nounwind readnone
  store <8 x double> %call247, <8 x double>* %a8_out, align 64
  %391 = load <16 x double>* %a16_in, align 128
  %392 = load <16 x double>* %b16_in, align 128
  %call248 = call <16 x double> @_Z3minDv16_dS_(<16 x double> %391, <16 x double> %392) nounwind readnone
  store <16 x double> %call248, <16 x double>* %a16_out, align 128
  %393 = load <4 x double>* %a4_in, align 32
  %394 = load double* %b_in, align 8
  %call249 = call <4 x double> @_Z3minDv4_dd(<4 x double> %393, double %394) nounwind readnone
  store <4 x double> %call249, <4 x double>* %a4_out, align 32
  %395 = load <8 x double>* %a8_in, align 64
  %396 = load double* %b_in, align 8
  %call250 = call <8 x double> @_Z3minDv8_dd(<8 x double> %395, double %396) nounwind readnone
  store <8 x double> %call250, <8 x double>* %a8_out, align 64
  %397 = load <16 x double>* %a16_in, align 128
  %398 = load double* %b_in, align 8
  %call251 = call <16 x double> @_Z3minDv16_dd(<16 x double> %397, double %398) nounwind readnone
  store <16 x double> %call251, <16 x double>* %a16_out, align 128
  %399 = load double* %a_in, align 8
  %400 = load double* %b_in, align 8
  %call252 = call double @_Z3maxdd(double %399, double %400) nounwind readnone
  store double %call252, double* %a_out, align 8
  %401 = load <4 x double>* %a4_in, align 32
  %402 = load <4 x double>* %b4_in, align 32
  %call253 = call <4 x double> @_Z3maxDv4_dS_(<4 x double> %401, <4 x double> %402) nounwind readnone
  store <4 x double> %call253, <4 x double>* %a4_out, align 32
  %403 = load <8 x double>* %a8_in, align 64
  %404 = load <8 x double>* %b8_in, align 64
  %call254 = call <8 x double> @_Z3maxDv8_dS_(<8 x double> %403, <8 x double> %404) nounwind readnone
  store <8 x double> %call254, <8 x double>* %a8_out, align 64
  %405 = load <16 x double>* %a16_in, align 128
  %406 = load <16 x double>* %b16_in, align 128
  %call255 = call <16 x double> @_Z3maxDv16_dS_(<16 x double> %405, <16 x double> %406) nounwind readnone
  store <16 x double> %call255, <16 x double>* %a16_out, align 128
  %407 = load <4 x double>* %a4_in, align 32
  %408 = load double* %b_in, align 8
  %call256 = call <4 x double> @_Z3maxDv4_dd(<4 x double> %407, double %408) nounwind readnone
  store <4 x double> %call256, <4 x double>* %a4_out, align 32
  %409 = load <8 x double>* %a8_in, align 64
  %410 = load double* %b_in, align 8
  %call257 = call <8 x double> @_Z3maxDv8_dd(<8 x double> %409, double %410) nounwind readnone
  store <8 x double> %call257, <8 x double>* %a8_out, align 64
  %411 = load <16 x double>* %a16_in, align 128
  %412 = load double* %b_in, align 8
  %call258 = call <16 x double> @_Z3maxDv16_dd(<16 x double> %411, double %412) nounwind readnone
  store <16 x double> %call258, <16 x double>* %a16_out, align 128
  %413 = load double* %a_in, align 8
  %414 = load double* %b_in, align 8
  %call259 = call double @_Z5hypotdd(double %413, double %414) nounwind readnone
  store double %call259, double* %a_out, align 8
  %415 = load <4 x double>* %a4_in, align 32
  %416 = load <4 x double>* %b4_in, align 32
  %call260 = call <4 x double> @_Z5hypotDv4_dS_(<4 x double> %415, <4 x double> %416) nounwind readnone
  store <4 x double> %call260, <4 x double>* %a4_out, align 32
  %417 = load <8 x double>* %a8_in, align 64
  %418 = load <8 x double>* %b8_in, align 64
  %call261 = call <8 x double> @_Z5hypotDv8_dS_(<8 x double> %417, <8 x double> %418) nounwind readnone
  store <8 x double> %call261, <8 x double>* %a8_out, align 64
  %419 = load <16 x double>* %a16_in, align 128
  %420 = load <16 x double>* %b16_in, align 128
  %call262 = call <16 x double> @_Z5hypotDv16_dS_(<16 x double> %419, <16 x double> %420) nounwind readnone
  store <16 x double> %call262, <16 x double>* %a16_out, align 128
  %421 = load double* %a_in, align 8
  %422 = load double* %b_in, align 8
  %call263 = call double @_Z4stepdd(double %421, double %422) nounwind readnone
  store double %call263, double* %a_out, align 8
  %423 = load <4 x double>* %a4_in, align 32
  %424 = load <4 x double>* %b4_in, align 32
  %call264 = call <4 x double> @_Z4stepDv4_dS_(<4 x double> %423, <4 x double> %424) nounwind readnone
  store <4 x double> %call264, <4 x double>* %a4_out, align 32
  %425 = load <8 x double>* %a8_in, align 64
  %426 = load <8 x double>* %b8_in, align 64
  %call265 = call <8 x double> @_Z4stepDv8_dS_(<8 x double> %425, <8 x double> %426) nounwind readnone
  store <8 x double> %call265, <8 x double>* %a8_out, align 64
  %427 = load <16 x double>* %a16_in, align 128
  %428 = load <16 x double>* %b16_in, align 128
  %call266 = call <16 x double> @_Z4stepDv16_dS_(<16 x double> %427, <16 x double> %428) nounwind readnone
  store <16 x double> %call266, <16 x double>* %a16_out, align 128
  %429 = load double* %a_in, align 8
  %430 = load double* %b_in, align 8
  %call267 = call double @_Z4stepdd(double %429, double %430) nounwind readnone
  store double %call267, double* %a_out, align 8
  %431 = load double* %a_in, align 8
  %432 = load <4 x double>* %b4_in, align 32
  %call268 = call <4 x double> @_Z4stepdDv4_d(double %431, <4 x double> %432) nounwind readnone
  store <4 x double> %call268, <4 x double>* %a4_out, align 32
  %433 = load double* %a_in, align 8
  %434 = load <8 x double>* %b8_in, align 64
  %call269 = call <8 x double> @_Z4stepdDv8_d(double %433, <8 x double> %434) nounwind readnone
  store <8 x double> %call269, <8 x double>* %a8_out, align 64
  %435 = load double* %a_in, align 8
  %436 = load <16 x double>* %b16_in, align 128
  %call270 = call <16 x double> @_Z4stepdDv16_d(double %435, <16 x double> %436) nounwind readnone
  store <16 x double> %call270, <16 x double>* %a16_out, align 128
  %437 = load double* %a_in, align 8
  %438 = load double* %b_in, align 8
  %439 = load double* %c_in, align 8
  %call271 = call double @_Z10smoothstepddd(double %437, double %438, double %439) nounwind readnone
  store double %call271, double* %a_out, align 8
  %440 = load <4 x double>* %a4_in, align 32
  %441 = load <4 x double>* %b4_in, align 32
  %442 = load <4 x double>* %c4_in, align 32
  %call272 = call <4 x double> @_Z10smoothstepDv4_dS_S_(<4 x double> %440, <4 x double> %441, <4 x double> %442) nounwind readnone
  store <4 x double> %call272, <4 x double>* %a4_out, align 32
  %443 = load <8 x double>* %a8_in, align 64
  %444 = load <8 x double>* %b8_in, align 64
  %445 = load <8 x double>* %c8_in, align 64
  %call273 = call <8 x double> @_Z10smoothstepDv8_dS_S_(<8 x double> %443, <8 x double> %444, <8 x double> %445) nounwind readnone
  store <8 x double> %call273, <8 x double>* %a8_out, align 64
  %446 = load <16 x double>* %a16_in, align 128
  %447 = load <16 x double>* %b16_in, align 128
  %448 = load <16 x double>* %c16_in, align 128
  %call274 = call <16 x double> @_Z10smoothstepDv16_dS_S_(<16 x double> %446, <16 x double> %447, <16 x double> %448) nounwind readnone
  store <16 x double> %call274, <16 x double>* %a16_out, align 128
  %449 = load double* %a_in, align 8
  %450 = load double* %b_in, align 8
  %451 = load double* %c_in, align 8
  %call275 = call double @_Z10smoothstepddd(double %449, double %450, double %451) nounwind readnone
  store double %call275, double* %a_out, align 8
  %452 = load double* %a_in, align 8
  %453 = load double* %b_in, align 8
  %454 = load <4 x double>* %c4_in, align 32
  %call276 = call <4 x double> @_Z10smoothstepddDv4_d(double %452, double %453, <4 x double> %454) nounwind readnone
  store <4 x double> %call276, <4 x double>* %a4_out, align 32
  %455 = load double* %a_in, align 8
  %456 = load double* %b_in, align 8
  %457 = load <8 x double>* %c8_in, align 64
  %call277 = call <8 x double> @_Z10smoothstepddDv8_d(double %455, double %456, <8 x double> %457) nounwind readnone
  store <8 x double> %call277, <8 x double>* %a8_out, align 64
  %458 = load double* %a_in, align 8
  %459 = load double* %b_in, align 8
  %460 = load <16 x double>* %c16_in, align 128
  %call278 = call <16 x double> @_Z10smoothstepddDv16_d(double %458, double %459, <16 x double> %460) nounwind readnone
  store <16 x double> %call278, <16 x double>* %a16_out, align 128
  %461 = load double* %a_in, align 8
  %call279 = call double @_Z7radiansd(double %461) nounwind readnone
  store double %call279, double* %a_out, align 8
  %462 = load <4 x double>* %a4_in, align 32
  %call280 = call <4 x double> @_Z7radiansDv4_d(<4 x double> %462) nounwind readnone
  store <4 x double> %call280, <4 x double>* %a4_out, align 32
  %463 = load <8 x double>* %a8_in, align 64
  %call281 = call <8 x double> @_Z7radiansDv8_d(<8 x double> %463) nounwind readnone
  store <8 x double> %call281, <8 x double>* %a8_out, align 64
  %464 = load <16 x double>* %a16_in, align 128
  %call282 = call <16 x double> @_Z7radiansDv16_d(<16 x double> %464) nounwind readnone
  store <16 x double> %call282, <16 x double>* %a16_out, align 128
  %465 = load double* %a_in, align 8
  %call283 = call double @_Z7degreesd(double %465) nounwind readnone
  store double %call283, double* %a_out, align 8
  %466 = load <4 x double>* %a4_in, align 32
  %call284 = call <4 x double> @_Z7degreesDv4_d(<4 x double> %466) nounwind readnone
  store <4 x double> %call284, <4 x double>* %a4_out, align 32
  %467 = load <8 x double>* %a8_in, align 64
  %call285 = call <8 x double> @_Z7degreesDv8_d(<8 x double> %467) nounwind readnone
  store <8 x double> %call285, <8 x double>* %a8_out, align 64
  %468 = load <16 x double>* %a16_in, align 128
  %call286 = call <16 x double> @_Z7degreesDv16_d(<16 x double> %468) nounwind readnone
  store <16 x double> %call286, <16 x double>* %a16_out, align 128
  %469 = load double* %a_in, align 8
  %call287 = call double @_Z4signd(double %469) nounwind readnone
  store double %call287, double* %a_out, align 8
  %470 = load <4 x double>* %a4_in, align 32
  %call288 = call <4 x double> @_Z4signDv4_d(<4 x double> %470) nounwind readnone
  store <4 x double> %call288, <4 x double>* %a4_out, align 32
  %471 = load <8 x double>* %a8_in, align 64
  %call289 = call <8 x double> @_Z4signDv8_d(<8 x double> %471) nounwind readnone
  store <8 x double> %call289, <8 x double>* %a8_out, align 64
  %472 = load <16 x double>* %a16_in, align 128
  %call290 = call <16 x double> @_Z4signDv16_d(<16 x double> %472) nounwind readnone
  store <16 x double> %call290, <16 x double>* %a16_out, align 128
  %473 = load double* %a_in, align 8
  %call291 = call double @_Z5floord(double %473) nounwind readnone
  store double %call291, double* %a_out, align 8
  %474 = load <4 x double>* %a4_in, align 32
  %call292 = call <4 x double> @_Z5floorDv4_d(<4 x double> %474) nounwind readnone
  store <4 x double> %call292, <4 x double>* %a4_out, align 32
  %475 = load <8 x double>* %a8_in, align 64
  %call293 = call <8 x double> @_Z5floorDv8_d(<8 x double> %475) nounwind readnone
  store <8 x double> %call293, <8 x double>* %a8_out, align 64
  %476 = load <16 x double>* %a16_in, align 128
  %call294 = call <16 x double> @_Z5floorDv16_d(<16 x double> %476) nounwind readnone
  store <16 x double> %call294, <16 x double>* %a16_out, align 128
  %477 = load double* %a_in, align 8
  %478 = load double* %b_in, align 8
  %call295 = call double @_Z3dotdd(double %477, double %478) nounwind readnone
  store double %call295, double* %a_out, align 8
  %479 = load <4 x double>* %a4_in, align 32
  %480 = load <4 x double>* %b4_in, align 32
  %call296 = call double @_Z3dotDv4_dS_(<4 x double> %479, <4 x double> %480) nounwind readnone
  store double %call296, double* %a_out, align 8
  %481 = load double* %a_in, align 8
  %482 = load double* %b_in, align 8
  %483 = load double* %c_in, align 8
  %call297 = call double @_Z3mixddd(double %481, double %482, double %483) nounwind readnone
  store double %call297, double* %a_out, align 8
  %484 = load <4 x double>* %a4_in, align 32
  %485 = load <4 x double>* %b4_in, align 32
  %486 = load <4 x double>* %c4_in, align 32
  %call298 = call <4 x double> @_Z3mixDv4_dS_S_(<4 x double> %484, <4 x double> %485, <4 x double> %486) nounwind readnone
  store <4 x double> %call298, <4 x double>* %a4_out, align 32
  %487 = load <4 x double>* %a4_in, align 32
  %488 = load <4 x double>* %b4_in, align 32
  %489 = load double* %c_in, align 8
  %call299 = call <4 x double> @_Z3mixDv4_dS_d(<4 x double> %487, <4 x double> %488, double %489) nounwind readnone
  store <4 x double> %call299, <4 x double>* %a4_out, align 32
  %490 = load <8 x double>* %a8_in, align 64
  %491 = load <8 x double>* %b8_in, align 64
  %492 = load double* %c_in, align 8
  %call300 = call <8 x double> @_Z3mixDv8_dS_d(<8 x double> %490, <8 x double> %491, double %492) nounwind readnone
  store <8 x double> %call300, <8 x double>* %a8_out, align 64
  %493 = load <16 x double>* %a16_in, align 128
  %494 = load <16 x double>* %b16_in, align 128
  %495 = load double* %c_in, align 8
  %call301 = call <16 x double> @_Z3mixDv16_dS_d(<16 x double> %493, <16 x double> %494, double %495) nounwind readnone
  store <16 x double> %call301, <16 x double>* %a16_out, align 128
  %496 = load <8 x double>* %a8_in, align 64
  %497 = load <8 x double>* %b8_in, align 64
  %498 = load <8 x double>* %c8_in, align 64
  %call302 = call <8 x double> @_Z3mixDv8_dS_S_(<8 x double> %496, <8 x double> %497, <8 x double> %498) nounwind readnone
  store <8 x double> %call302, <8 x double>* %a8_out, align 64
  %499 = load <16 x double>* %a16_in, align 128
  %500 = load <16 x double>* %b16_in, align 128
  %501 = load <16 x double>* %c16_in, align 128
  %call303 = call <16 x double> @_Z3mixDv16_dS_S_(<16 x double> %499, <16 x double> %500, <16 x double> %501) nounwind readnone
  store <16 x double> %call303, <16 x double>* %a16_out, align 128
  %502 = load double* %a_in, align 8
  %call304 = call double @_Z9normalized(double %502) nounwind readnone
  store double %call304, double* %a_out, align 8
  %503 = load <4 x double>* %a4_in, align 32
  %call305 = call <4 x double> @_Z9normalizeDv4_d(<4 x double> %503) nounwind readnone
  store <4 x double> %call305, <4 x double>* %a4_out, align 32
  %504 = load <4 x double>* %a4_in, align 32
  %505 = load <4 x double>* %b4_in, align 32
  %call306 = call <4 x double> @_Z5crossDv4_dS_(<4 x double> %504, <4 x double> %505) nounwind readnone
  store <4 x double> %call306, <4 x double>* %a4_out, align 32
  %506 = load double* %a_in, align 8
  %call307 = call double @_Z6lengthd(double %506) nounwind readnone
  store double %call307, double* %a_out, align 8
  %507 = load <2 x double>* %a2_in, align 16
  %call308 = call double @_Z6lengthDv2_d(<2 x double> %507) nounwind readnone
  store double %call308, double* %a_out, align 8
  %508 = load <4 x double>* %a4_in, align 32
  %call309 = call double @_Z6lengthDv4_d(<4 x double> %508) nounwind readnone
  store double %call309, double* %a_out, align 8
  %509 = load double* %a_in, align 8
  %510 = load double* %b_in, align 8
  %call310 = call double @_Z8distancedd(double %509, double %510) nounwind readnone
  store double %call310, double* %a_out, align 8
  %511 = load <2 x double>* %a2_in, align 16
  %512 = load <2 x double>* %b2_in, align 16
  %call311 = call double @_Z8distanceDv2_dS_(<2 x double> %511, <2 x double> %512) nounwind readnone
  store double %call311, double* %a_out, align 8
  %513 = load <4 x double>* %a4_in, align 32
  %514 = load <4 x double>* %b4_in, align 32
  %call312 = call double @_Z8distanceDv4_dS_(<4 x double> %513, <4 x double> %514) nounwind readnone
  store double %call312, double* %a_out, align 8
  %515 = load double* %a_in, align 8
  %516 = load i32* %i_in, align 4
  %call313 = call double @_Z5rootndi(double %515, i32 %516) nounwind readnone
  store double %call313, double* %a_out, align 8
  %517 = load <4 x double>* %a4_in, align 32
  %518 = load <4 x i32>* %i4_in, align 16
  %call314 = call <4 x double> @_Z5rootnDv4_dDv4_i(<4 x double> %517, <4 x i32> %518) nounwind readnone
  store <4 x double> %call314, <4 x double>* %a4_out, align 32
  %519 = load <8 x double>* %a8_in, align 64
  %520 = load <8 x i32>* %i8_in, align 32
  %call315 = call <8 x double> @_Z5rootnDv8_dDv8_i(<8 x double> %519, <8 x i32> %520) nounwind readnone
  store <8 x double> %call315, <8 x double>* %a8_out, align 64
  %521 = load <16 x double>* %a16_in, align 128
  %522 = load <16 x i32>* %i16_in, align 64
  %call316 = call <16 x double> @_Z5rootnDv16_dDv16_i(<16 x double> %521, <16 x i32> %522) nounwind readnone
  store <16 x double> %call316, <16 x double>* %a16_out, align 128
  %523 = load double* %a_in, align 8
  %524 = load i32* %i_in, align 4
  %call317 = call double @_Z5ldexpdi(double %523, i32 %524) nounwind readnone
  store double %call317, double* %a_out, align 8
  %525 = load <4 x double>* %a4_in, align 32
  %526 = load <4 x i32>* %i4_in, align 16
  %call318 = call <4 x double> @_Z5ldexpDv4_dDv4_i(<4 x double> %525, <4 x i32> %526) nounwind readnone
  store <4 x double> %call318, <4 x double>* %a4_out, align 32
  %527 = load <8 x double>* %a8_in, align 64
  %528 = load <8 x i32>* %i8_in, align 32
  %call319 = call <8 x double> @_Z5ldexpDv8_dDv8_i(<8 x double> %527, <8 x i32> %528) nounwind readnone
  store <8 x double> %call319, <8 x double>* %a8_out, align 64
  %529 = load <16 x double>* %a16_in, align 128
  %530 = load <16 x i32>* %i16_in, align 64
  %call320 = call <16 x double> @_Z5ldexpDv16_dDv16_i(<16 x double> %529, <16 x i32> %530) nounwind readnone
  store <16 x double> %call320, <16 x double>* %a16_out, align 128
  %531 = load <4 x double>* %a4_in, align 32
  %532 = load i32* %i_in, align 4
  %call321 = call <4 x double> @_Z5ldexpDv4_di(<4 x double> %531, i32 %532) nounwind readnone
  store <4 x double> %call321, <4 x double>* %a4_out, align 32
  %533 = load <8 x double>* %a8_in, align 64
  %534 = load i32* %i_in, align 4
  %call322 = call <8 x double> @_Z5ldexpDv8_di(<8 x double> %533, i32 %534) nounwind readnone
  store <8 x double> %call322, <8 x double>* %a8_out, align 64
  %535 = load <16 x double>* %a16_in, align 128
  %536 = load i32* %i_in, align 4
  %call323 = call <16 x double> @_Z5ldexpDv16_di(<16 x double> %535, i32 %536) nounwind readnone
  store <16 x double> %call323, <16 x double>* %a16_out, align 128
  %537 = load double* %a_in, align 8
  %call324 = call double @_Z4modfdPd(double %537, double* %b_out)
  store double %call324, double* %a_out, align 8
  %538 = load <4 x double>* %a4_in, align 32
  %call325 = call <4 x double> @_Z4modfDv4_dPS_(<4 x double> %538, <4 x double>* %b4_out)
  store <4 x double> %call325, <4 x double>* %a4_out, align 32
  %539 = load <8 x double>* %a8_in, align 64
  %call326 = call <8 x double> @_Z4modfDv8_dPS_(<8 x double> %539, <8 x double>* %b8_out)
  store <8 x double> %call326, <8 x double>* %a8_out, align 64
  %540 = load <16 x double>* %a16_in, align 128
  %call327 = call <16 x double> @_Z4modfDv16_dPS_(<16 x double> %540, <16 x double>* %b16_out)
  store <16 x double> %call327, <16 x double>* %a16_out, align 128
  %541 = load double* %a_in, align 8
  %call328 = call double @_Z5frexpdPi(double %541, i32* %i_out)
  store double %call328, double* %a_out, align 8
  %542 = load <4 x double>* %a4_in, align 32
  %call329 = call <4 x double> @_Z5frexpDv4_dPDv4_i(<4 x double> %542, <4 x i32>* %i4_out)
  store <4 x double> %call329, <4 x double>* %a4_out, align 32
  %543 = load <8 x double>* %a8_in, align 64
  %call330 = call <8 x double> @_Z5frexpDv8_dPDv8_i(<8 x double> %543, <8 x i32>* %i8_out)
  store <8 x double> %call330, <8 x double>* %a8_out, align 64
  %544 = load <16 x double>* %a16_in, align 128
  %call331 = call <16 x double> @_Z5frexpDv16_dPDv16_i(<16 x double> %544, <16 x i32>* %i16_out)
  store <16 x double> %call331, <16 x double>* %a16_out, align 128
  %545 = load double* %a_in, align 8
  %546 = load double* %b_in, align 8
  %call332 = call double @_Z6maxmagdd(double %545, double %546) nounwind readnone
  store double %call332, double* %a_out, align 8
  %547 = load <4 x double>* %a4_in, align 32
  %548 = load <4 x double>* %b4_in, align 32
  %call333 = call <4 x double> @_Z6maxmagDv4_dS_(<4 x double> %547, <4 x double> %548) nounwind readnone
  store <4 x double> %call333, <4 x double>* %a4_out, align 32
  %549 = load <8 x double>* %a8_in, align 64
  %550 = load <8 x double>* %b8_in, align 64
  %call334 = call <8 x double> @_Z6maxmagDv8_dS_(<8 x double> %549, <8 x double> %550) nounwind readnone
  store <8 x double> %call334, <8 x double>* %a8_out, align 64
  %551 = load <16 x double>* %a16_in, align 128
  %552 = load <16 x double>* %b16_in, align 128
  %call335 = call <16 x double> @_Z6maxmagDv16_dS_(<16 x double> %551, <16 x double> %552) nounwind readnone
  store <16 x double> %call335, <16 x double>* %a16_out, align 128
  %553 = load double* %a_in, align 8
  %554 = load double* %b_in, align 8
  %call336 = call double @_Z6minmagdd(double %553, double %554) nounwind readnone
  store double %call336, double* %a_out, align 8
  %555 = load <4 x double>* %a4_in, align 32
  %556 = load <4 x double>* %b4_in, align 32
  %call337 = call <4 x double> @_Z6minmagDv4_dS_(<4 x double> %555, <4 x double> %556) nounwind readnone
  store <4 x double> %call337, <4 x double>* %a4_out, align 32
  %557 = load <8 x double>* %a8_in, align 64
  %558 = load <8 x double>* %b8_in, align 64
  %call338 = call <8 x double> @_Z6minmagDv8_dS_(<8 x double> %557, <8 x double> %558) nounwind readnone
  store <8 x double> %call338, <8 x double>* %a8_out, align 64
  %559 = load <16 x double>* %a16_in, align 128
  %560 = load <16 x double>* %b16_in, align 128
  %call339 = call <16 x double> @_Z6minmagDv16_dS_(<16 x double> %559, <16 x double> %560) nounwind readnone
  store <16 x double> %call339, <16 x double>* %a16_out, align 128
  %561 = load double* %a_in, align 8
  %562 = load double* %b_in, align 8
  %call340 = call double @_Z8copysigndd(double %561, double %562) nounwind readnone
  store double %call340, double* %a_out, align 8
  %563 = load <4 x double>* %a4_in, align 32
  %564 = load <4 x double>* %b4_in, align 32
  %call341 = call <4 x double> @_Z8copysignDv4_dS_(<4 x double> %563, <4 x double> %564) nounwind readnone
  store <4 x double> %call341, <4 x double>* %a4_out, align 32
  %565 = load <8 x double>* %a8_in, align 64
  %566 = load <8 x double>* %b8_in, align 64
  %call342 = call <8 x double> @_Z8copysignDv8_dS_(<8 x double> %565, <8 x double> %566) nounwind readnone
  store <8 x double> %call342, <8 x double>* %a8_out, align 64
  %567 = load <16 x double>* %a16_in, align 128
  %568 = load <16 x double>* %b16_in, align 128
  %call343 = call <16 x double> @_Z8copysignDv16_dS_(<16 x double> %567, <16 x double> %568) nounwind readnone
  store <16 x double> %call343, <16 x double>* %a16_out, align 128
  %569 = load double* %a_in, align 8
  %570 = load double* %b_in, align 8
  %call344 = call double @_Z9nextafterdd(double %569, double %570) nounwind readnone
  store double %call344, double* %a_out, align 8
  %571 = load <4 x double>* %a4_in, align 32
  %572 = load <4 x double>* %b4_in, align 32
  %call345 = call <4 x double> @_Z9nextafterDv4_dS_(<4 x double> %571, <4 x double> %572) nounwind readnone
  store <4 x double> %call345, <4 x double>* %a4_out, align 32
  %573 = load <8 x double>* %a8_in, align 64
  %574 = load <8 x double>* %b8_in, align 64
  %call346 = call <8 x double> @_Z9nextafterDv8_dS_(<8 x double> %573, <8 x double> %574) nounwind readnone
  store <8 x double> %call346, <8 x double>* %a8_out, align 64
  %575 = load <16 x double>* %a16_in, align 128
  %576 = load <16 x double>* %b16_in, align 128
  %call347 = call <16 x double> @_Z9nextafterDv16_dS_(<16 x double> %575, <16 x double> %576) nounwind readnone
  store <16 x double> %call347, <16 x double>* %a16_out, align 128
  %577 = load double* %a_in, align 8
  %578 = load double* %b_in, align 8
  %call348 = call double @_Z4fdimdd(double %577, double %578) nounwind readnone
  store double %call348, double* %a_out, align 8
  %579 = load <4 x double>* %a4_in, align 32
  %580 = load <4 x double>* %b4_in, align 32
  %call349 = call <4 x double> @_Z4fdimDv4_dS_(<4 x double> %579, <4 x double> %580) nounwind readnone
  store <4 x double> %call349, <4 x double>* %a4_out, align 32
  %581 = load <8 x double>* %a8_in, align 64
  %582 = load <8 x double>* %b8_in, align 64
  %call350 = call <8 x double> @_Z4fdimDv8_dS_(<8 x double> %581, <8 x double> %582) nounwind readnone
  store <8 x double> %call350, <8 x double>* %a8_out, align 64
  %583 = load <16 x double>* %a16_in, align 128
  %584 = load <16 x double>* %b16_in, align 128
  %call351 = call <16 x double> @_Z4fdimDv16_dS_(<16 x double> %583, <16 x double> %584) nounwind readnone
  store <16 x double> %call351, <16 x double>* %a16_out, align 128
  %585 = load double* %a_in, align 8
  %586 = load double* %b_in, align 8
  %587 = load double* %c_in, align 8
  %call352 = call double @_Z3fmaddd(double %585, double %586, double %587) nounwind readnone
  store double %call352, double* %a_out, align 8
  %588 = load <4 x double>* %a4_in, align 32
  %589 = load <4 x double>* %b4_in, align 32
  %590 = load <4 x double>* %c4_in, align 32
  %call353 = call <4 x double> @_Z3fmaDv4_dS_S_(<4 x double> %588, <4 x double> %589, <4 x double> %590) nounwind readnone
  store <4 x double> %call353, <4 x double>* %a4_out, align 32
  %591 = load <8 x double>* %a8_in, align 64
  %592 = load <8 x double>* %b8_in, align 64
  %593 = load <8 x double>* %c8_in, align 64
  %call354 = call <8 x double> @_Z3fmaDv8_dS_S_(<8 x double> %591, <8 x double> %592, <8 x double> %593) nounwind readnone
  store <8 x double> %call354, <8 x double>* %a8_out, align 64
  %594 = load <16 x double>* %a16_in, align 128
  %595 = load <16 x double>* %b16_in, align 128
  %596 = load <16 x double>* %c16_in, align 128
  %call355 = call <16 x double> @_Z3fmaDv16_dS_S_(<16 x double> %594, <16 x double> %595, <16 x double> %596) nounwind readnone
  store <16 x double> %call355, <16 x double>* %a16_out, align 128
  %597 = load double* %a_in, align 8
  %598 = load double* %b_in, align 8
  %599 = load double* %c_in, align 8
  %call356 = call double @_Z3madddd(double %597, double %598, double %599) nounwind readnone
  store double %call356, double* %a_out, align 8
  %600 = load <4 x double>* %a4_in, align 32
  %601 = load <4 x double>* %b4_in, align 32
  %602 = load <4 x double>* %c4_in, align 32
  %call357 = call <4 x double> @_Z3madDv4_dS_S_(<4 x double> %600, <4 x double> %601, <4 x double> %602) nounwind readnone
  store <4 x double> %call357, <4 x double>* %a4_out, align 32
  %603 = load <8 x double>* %a8_in, align 64
  %604 = load <8 x double>* %b8_in, align 64
  %605 = load <8 x double>* %c8_in, align 64
  %call358 = call <8 x double> @_Z3madDv8_dS_S_(<8 x double> %603, <8 x double> %604, <8 x double> %605) nounwind readnone
  store <8 x double> %call358, <8 x double>* %a8_out, align 64
  %606 = load <16 x double>* %a16_in, align 128
  %607 = load <16 x double>* %b16_in, align 128
  %608 = load <16 x double>* %c16_in, align 128
  %call359 = call <16 x double> @_Z3madDv16_dS_S_(<16 x double> %606, <16 x double> %607, <16 x double> %608) nounwind readnone
  store <16 x double> %call359, <16 x double>* %a16_out, align 128
  %609 = load double* %a_in, align 8
  %call360 = call double @_Z4rintd(double %609) nounwind readnone
  store double %call360, double* %a_out, align 8
  %610 = load <4 x double>* %a4_in, align 32
  %call361 = call <4 x double> @_Z4rintDv4_d(<4 x double> %610) nounwind readnone
  store <4 x double> %call361, <4 x double>* %a4_out, align 32
  %611 = load <8 x double>* %a8_in, align 64
  %call362 = call <8 x double> @_Z4rintDv8_d(<8 x double> %611) nounwind readnone
  store <8 x double> %call362, <8 x double>* %a8_out, align 64
  %612 = load <16 x double>* %a16_in, align 128
  %call363 = call <16 x double> @_Z4rintDv16_d(<16 x double> %612) nounwind readnone
  store <16 x double> %call363, <16 x double>* %a16_out, align 128
  %613 = load double* %a_in, align 8
  %call364 = call double @_Z5roundd(double %613) nounwind readnone
  store double %call364, double* %a_out, align 8
  %614 = load <4 x double>* %a4_in, align 32
  %call365 = call <4 x double> @_Z5roundDv4_d(<4 x double> %614) nounwind readnone
  store <4 x double> %call365, <4 x double>* %a4_out, align 32
  %615 = load <8 x double>* %a8_in, align 64
  %call366 = call <8 x double> @_Z5roundDv8_d(<8 x double> %615) nounwind readnone
  store <8 x double> %call366, <8 x double>* %a8_out, align 64
  %616 = load <16 x double>* %a16_in, align 128
  %call367 = call <16 x double> @_Z5roundDv16_d(<16 x double> %616) nounwind readnone
  store <16 x double> %call367, <16 x double>* %a16_out, align 128
  %617 = load double* %a_in, align 8
  %call368 = call double @_Z5truncd(double %617) nounwind readnone
  store double %call368, double* %a_out, align 8
  %618 = load <4 x double>* %a4_in, align 32
  %call369 = call <4 x double> @_Z5truncDv4_d(<4 x double> %618) nounwind readnone
  store <4 x double> %call369, <4 x double>* %a4_out, align 32
  %619 = load <8 x double>* %a8_in, align 64
  %call370 = call <8 x double> @_Z5truncDv8_d(<8 x double> %619) nounwind readnone
  store <8 x double> %call370, <8 x double>* %a8_out, align 64
  %620 = load <16 x double>* %a16_in, align 128
  %call371 = call <16 x double> @_Z5truncDv16_d(<16 x double> %620) nounwind readnone
  store <16 x double> %call371, <16 x double>* %a16_out, align 128
  %621 = load double* %a_in, align 8
  %call372 = call double @_Z4cbrtd(double %621) nounwind readnone
  store double %call372, double* %a_out, align 8
  %622 = load <4 x double>* %a4_in, align 32
  %call373 = call <4 x double> @_Z4cbrtDv4_d(<4 x double> %622) nounwind readnone
  store <4 x double> %call373, <4 x double>* %a4_out, align 32
  %623 = load <8 x double>* %a8_in, align 64
  %call374 = call <8 x double> @_Z4cbrtDv8_d(<8 x double> %623) nounwind readnone
  store <8 x double> %call374, <8 x double>* %a8_out, align 64
  %624 = load <16 x double>* %a16_in, align 128
  %call375 = call <16 x double> @_Z4cbrtDv16_d(<16 x double> %624) nounwind readnone
  store <16 x double> %call375, <16 x double>* %a16_out, align 128
  %625 = load double* %a_in, align 8
  %626 = load double* %b_in, align 8
  %call376 = call double @_Z4powrdd(double %625, double %626) nounwind readnone
  store double %call376, double* %a_out, align 8
  %627 = load <4 x double>* %a4_in, align 32
  %628 = load <4 x double>* %b4_in, align 32
  %call377 = call <4 x double> @_Z4powrDv4_dS_(<4 x double> %627, <4 x double> %628) nounwind readnone
  store <4 x double> %call377, <4 x double>* %a4_out, align 32
  %629 = load <8 x double>* %a8_in, align 64
  %630 = load <8 x double>* %b8_in, align 64
  %call378 = call <8 x double> @_Z4powrDv8_dS_(<8 x double> %629, <8 x double> %630) nounwind readnone
  store <8 x double> %call378, <8 x double>* %a8_out, align 64
  %631 = load <16 x double>* %a16_in, align 128
  %632 = load <16 x double>* %b16_in, align 128
  %call379 = call <16 x double> @_Z4powrDv16_dS_(<16 x double> %631, <16 x double> %632) nounwind readnone
  store <16 x double> %call379, <16 x double>* %a16_out, align 128
  %633 = load double* %a_in, align 8
  %634 = load double* %b_in, align 8
  %call380 = call double @_Z4fmoddd(double %633, double %634) nounwind readnone
  store double %call380, double* %a_out, align 8
  %635 = load <4 x double>* %a4_in, align 32
  %636 = load <4 x double>* %b4_in, align 32
  %call381 = call <4 x double> @_Z4fmodDv4_dS_(<4 x double> %635, <4 x double> %636) nounwind readnone
  store <4 x double> %call381, <4 x double>* %a4_out, align 32
  %637 = load <8 x double>* %a8_in, align 64
  %638 = load <8 x double>* %b8_in, align 64
  %call382 = call <8 x double> @_Z4fmodDv8_dS_(<8 x double> %637, <8 x double> %638) nounwind readnone
  store <8 x double> %call382, <8 x double>* %a8_out, align 64
  %639 = load <16 x double>* %a16_in, align 128
  %640 = load <16 x double>* %b16_in, align 128
  %call383 = call <16 x double> @_Z4fmodDv16_dS_(<16 x double> %639, <16 x double> %640) nounwind readnone
  store <16 x double> %call383, <16 x double>* %a16_out, align 128
  %641 = load double* %a_in, align 8
  %642 = load double* %b_in, align 8
  %call384 = call double @_Z4fmindd(double %641, double %642) nounwind readnone
  store double %call384, double* %a_out, align 8
  %643 = load <4 x double>* %a4_in, align 32
  %644 = load <4 x double>* %b4_in, align 32
  %call385 = call <4 x double> @_Z4fminDv4_dS_(<4 x double> %643, <4 x double> %644) nounwind readnone
  store <4 x double> %call385, <4 x double>* %a4_out, align 32
  %645 = load <8 x double>* %a8_in, align 64
  %646 = load <8 x double>* %b8_in, align 64
  %call386 = call <8 x double> @_Z4fminDv8_dS_(<8 x double> %645, <8 x double> %646) nounwind readnone
  store <8 x double> %call386, <8 x double>* %a8_out, align 64
  %647 = load <16 x double>* %a16_in, align 128
  %648 = load <16 x double>* %b16_in, align 128
  %call387 = call <16 x double> @_Z4fminDv16_dS_(<16 x double> %647, <16 x double> %648) nounwind readnone
  store <16 x double> %call387, <16 x double>* %a16_out, align 128
  %649 = load double* %a_in, align 8
  %650 = load double* %b_in, align 8
  %call388 = call double @_Z4fmaxdd(double %649, double %650) nounwind readnone
  store double %call388, double* %a_out, align 8
  %651 = load <4 x double>* %a4_in, align 32
  %652 = load <4 x double>* %b4_in, align 32
  %call389 = call <4 x double> @_Z4fmaxDv4_dS_(<4 x double> %651, <4 x double> %652) nounwind readnone
  store <4 x double> %call389, <4 x double>* %a4_out, align 32
  %653 = load <8 x double>* %a8_in, align 64
  %654 = load <8 x double>* %b8_in, align 64
  %call390 = call <8 x double> @_Z4fmaxDv8_dS_(<8 x double> %653, <8 x double> %654) nounwind readnone
  store <8 x double> %call390, <8 x double>* %a8_out, align 64
  %655 = load <16 x double>* %a16_in, align 128
  %656 = load <16 x double>* %b16_in, align 128
  %call391 = call <16 x double> @_Z4fmaxDv16_dS_(<16 x double> %655, <16 x double> %656) nounwind readnone
  store <16 x double> %call391, <16 x double>* %a16_out, align 128
  %657 = load <4 x double>* %a4_in, align 32
  %658 = load double* %b_in, align 8
  %call392 = call <4 x double> @_Z4fminDv4_dd(<4 x double> %657, double %658) nounwind readnone
  store <4 x double> %call392, <4 x double>* %a4_out, align 32
  %659 = load <8 x double>* %a8_in, align 64
  %660 = load double* %b_in, align 8
  %call393 = call <8 x double> @_Z4fminDv8_dd(<8 x double> %659, double %660) nounwind readnone
  store <8 x double> %call393, <8 x double>* %a8_out, align 64
  %661 = load <16 x double>* %a16_in, align 128
  %662 = load double* %b_in, align 8
  %call394 = call <16 x double> @_Z4fminDv16_dd(<16 x double> %661, double %662) nounwind readnone
  store <16 x double> %call394, <16 x double>* %a16_out, align 128
  %663 = load <4 x double>* %a4_in, align 32
  %664 = load double* %b_in, align 8
  %call395 = call <4 x double> @_Z4fmaxDv4_dd(<4 x double> %663, double %664) nounwind readnone
  store <4 x double> %call395, <4 x double>* %a4_out, align 32
  %665 = load <8 x double>* %a8_in, align 64
  %666 = load double* %b_in, align 8
  %call396 = call <8 x double> @_Z4fmaxDv8_dd(<8 x double> %665, double %666) nounwind readnone
  store <8 x double> %call396, <8 x double>* %a8_out, align 64
  %667 = load <16 x double>* %a16_in, align 128
  %668 = load double* %b_in, align 8
  %call397 = call <16 x double> @_Z4fmaxDv16_dd(<16 x double> %667, double %668) nounwind readnone
  store <16 x double> %call397, <16 x double>* %a16_out, align 128
  %669 = load <4 x double>* %a4_in, align 32
  %670 = load i32* %i_in, align 4
  %671 = insertelement <4 x i32> undef, i32 %670, i32 0
  %splat398 = shufflevector <4 x i32> %671, <4 x i32> %671, <4 x i32> zeroinitializer
  %call399 = call <4 x double> @_Z4pownDv4_dDv4_i(<4 x double> %669, <4 x i32> %splat398) nounwind readnone
  store <4 x double> %call399, <4 x double>* %a4_out, align 32
  %672 = load <8 x double>* %a8_in, align 64
  %673 = load i32* %i_in, align 4
  %674 = insertelement <8 x i32> undef, i32 %673, i32 0
  %splat400 = shufflevector <8 x i32> %674, <8 x i32> %674, <8 x i32> zeroinitializer
  %call401 = call <8 x double> @_Z4pownDv8_dDv8_i(<8 x double> %672, <8 x i32> %splat400) nounwind readnone
  store <8 x double> %call401, <8 x double>* %a8_out, align 64
  %675 = load <16 x double>* %a16_in, align 128
  %676 = load i32* %i_in, align 4
  %677 = insertelement <16 x i32> undef, i32 %676, i32 0
  %splat402 = shufflevector <16 x i32> %677, <16 x i32> %677, <16 x i32> zeroinitializer
  %call403 = call <16 x double> @_Z4pownDv16_dDv16_i(<16 x double> %675, <16 x i32> %splat402) nounwind readnone
  store <16 x double> %call403, <16 x double>* %a16_out, align 128
  %678 = load double* %a_in, align 8
  %call404 = call i32 @_Z5ilogbd(double %678) nounwind readnone
  store i32 %call404, i32* %i_out, align 4
  %679 = load <4 x double>* %a4_in, align 32
  %call405 = call <4 x i32> @_Z5ilogbDv4_d(<4 x double> %679) nounwind readnone
  store <4 x i32> %call405, <4 x i32>* %i4_out, align 16
  %680 = load <8 x double>* %a8_in, align 64
  %call406 = call <8 x i32> @_Z5ilogbDv8_d(<8 x double> %680) nounwind readnone
  store <8 x i32> %call406, <8 x i32>* %i8_out, align 32
  %681 = load <16 x double>* %a16_in, align 128
  %call407 = call <16 x i32> @_Z5ilogbDv16_d(<16 x double> %681) nounwind readnone
  store <16 x i32> %call407, <16 x i32>* %i16_out, align 64
  %682 = load i64* %ul_in, align 8
  %call408 = call double @_Z3nanm(i64 %682) nounwind readnone
  store double %call408, double* %a_out, align 8
  %683 = load <4 x i64>* %ul4_in, align 32
  %call409 = call <4 x double> @_Z3nanDv4_m(<4 x i64> %683) nounwind readnone
  store <4 x double> %call409, <4 x double>* %a4_out, align 32
  %684 = load <8 x i64>* %ul8_in, align 64
  %call410 = call <8 x double> @_Z3nanDv8_m(<8 x i64> %684) nounwind readnone
  store <8 x double> %call410, <8 x double>* %a8_out, align 64
  %685 = load <16 x i64>* %ul16_in, align 128
  %call411 = call <16 x double> @_Z3nanDv16_m(<16 x i64> %685) nounwind readnone
  store <16 x double> %call411, <16 x double>* %a16_out, align 128
  %686 = load double* %a_in, align 8
  %call412 = call double @_Z5fractdPd(double %686, double* %b_out)
  store double %call412, double* %a_out, align 8
  %687 = load <4 x double>* %a4_in, align 32
  %call413 = call <4 x double> @_Z5fractDv4_dPS_(<4 x double> %687, <4 x double>* %b4_out)
  store <4 x double> %call413, <4 x double>* %a4_out, align 32
  %688 = load <8 x double>* %a8_in, align 64
  %call414 = call <8 x double> @_Z5fractDv8_dPS_(<8 x double> %688, <8 x double>* %b8_out)
  store <8 x double> %call414, <8 x double>* %a8_out, align 64
  %689 = load <16 x double>* %a16_in, align 128
  %call415 = call <16 x double> @_Z5fractDv16_dPS_(<16 x double> %689, <16 x double>* %b16_out)
  store <16 x double> %call415, <16 x double>* %a16_out, align 128
  %690 = load double* %a_in, align 8
  %call416 = call double @_Z6lgammad(double %690) nounwind readnone
  store double %call416, double* %a_out, align 8
  %691 = load <4 x double>* %a4_in, align 32
  %call417 = call <4 x double> @_Z6lgammaDv4_d(<4 x double> %691) nounwind readnone
  store <4 x double> %call417, <4 x double>* %a4_out, align 32
  %692 = load <8 x double>* %a8_in, align 64
  %call418 = call <8 x double> @_Z6lgammaDv8_d(<8 x double> %692) nounwind readnone
  store <8 x double> %call418, <8 x double>* %a8_out, align 64
  %693 = load <16 x double>* %a16_in, align 128
  %call419 = call <16 x double> @_Z6lgammaDv16_d(<16 x double> %693) nounwind readnone
  store <16 x double> %call419, <16 x double>* %a16_out, align 128
  %694 = load double* %a_in, align 8
  %call420 = call double @_Z8lgamma_rdPi(double %694, i32* %i_out)
  store double %call420, double* %a_out, align 8
  %695 = load <4 x double>* %a4_in, align 32
  %call421 = call <4 x double> @_Z8lgamma_rDv4_dPDv4_i(<4 x double> %695, <4 x i32>* %i4_out)
  store <4 x double> %call421, <4 x double>* %a4_out, align 32
  %696 = load <8 x double>* %a8_in, align 64
  %call422 = call <8 x double> @_Z8lgamma_rDv8_dPDv8_i(<8 x double> %696, <8 x i32>* %i8_out)
  store <8 x double> %call422, <8 x double>* %a8_out, align 64
  %697 = load <16 x double>* %a16_in, align 128
  %call423 = call <16 x double> @_Z8lgamma_rDv16_dPDv16_i(<16 x double> %697, <16 x i32>* %i16_out)
  store <16 x double> %call423, <16 x double>* %a16_out, align 128
  %698 = load double* %a_in, align 8
  %699 = load double* %b_in, align 8
  %700 = load double* %c_in, align 8
  %call424 = call double @_Z9bitselectddd(double %698, double %699, double %700) nounwind readnone
  store double %call424, double* %a_out, align 8
  %701 = load <4 x double>* %a4_in, align 32
  %702 = load <4 x double>* %b4_in, align 32
  %703 = load <4 x double>* %c4_in, align 32
  %call425 = call <4 x double> @_Z9bitselectDv4_dS_S_(<4 x double> %701, <4 x double> %702, <4 x double> %703) nounwind readnone
  store <4 x double> %call425, <4 x double>* %a4_out, align 32
  %704 = load <8 x double>* %a8_in, align 64
  %705 = load <8 x double>* %b8_in, align 64
  %706 = load <8 x double>* %c8_in, align 64
  %call426 = call <8 x double> @_Z9bitselectDv8_dS_S_(<8 x double> %704, <8 x double> %705, <8 x double> %706) nounwind readnone
  store <8 x double> %call426, <8 x double>* %a8_out, align 64
  %707 = load <16 x double>* %a16_in, align 128
  %708 = load <16 x double>* %b16_in, align 128
  %709 = load <16 x double>* %c16_in, align 128
  %call427 = call <16 x double> @_Z9bitselectDv16_dS_S_(<16 x double> %707, <16 x double> %708, <16 x double> %709) nounwind readnone
  store <16 x double> %call427, <16 x double>* %a16_out, align 128
  %710 = load double* %a_in, align 8
  %711 = load double* %b_in, align 8
  %712 = load i64* %l_in, align 8
  %call428 = call double @_Z6selectddl(double %710, double %711, i64 %712) nounwind readnone
  store double %call428, double* %a_out, align 8
  %713 = load <4 x double>* %a4_in, align 32
  %714 = load <4 x double>* %b4_in, align 32
  %715 = load <4 x i64>* %l4_in, align 32
  %call429 = call <4 x double> @_Z6selectDv4_dS_Dv4_l(<4 x double> %713, <4 x double> %714, <4 x i64> %715) nounwind readnone
  store <4 x double> %call429, <4 x double>* %a4_out, align 32
  %716 = load <8 x double>* %a8_in, align 64
  %717 = load <8 x double>* %b8_in, align 64
  %718 = load <8 x i64>* %l8_in, align 64
  %call430 = call <8 x double> @_Z6selectDv8_dS_Dv8_l(<8 x double> %716, <8 x double> %717, <8 x i64> %718) nounwind readnone
  store <8 x double> %call430, <8 x double>* %a8_out, align 64
  %719 = load <16 x double>* %a16_in, align 128
  %720 = load <16 x double>* %b16_in, align 128
  %721 = load <16 x i64>* %l16_in, align 128
  %call431 = call <16 x double> @_Z6selectDv16_dS_Dv16_l(<16 x double> %719, <16 x double> %720, <16 x i64> %721) nounwind readnone
  store <16 x double> %call431, <16 x double>* %a16_out, align 128
  %722 = load double* %a_in, align 8
  %723 = load double* %b_in, align 8
  %724 = load i64* %ul_in, align 8
  %call432 = call double @_Z6selectddm(double %722, double %723, i64 %724) nounwind readnone
  store double %call432, double* %a_out, align 8
  %725 = load <4 x double>* %a4_in, align 32
  %726 = load <4 x double>* %b4_in, align 32
  %727 = load <4 x i64>* %ul4_in, align 32
  %call433 = call <4 x double> @_Z6selectDv4_dS_Dv4_m(<4 x double> %725, <4 x double> %726, <4 x i64> %727) nounwind readnone
  store <4 x double> %call433, <4 x double>* %a4_out, align 32
  %728 = load <8 x double>* %a8_in, align 64
  %729 = load <8 x double>* %b8_in, align 64
  %730 = load <8 x i64>* %ul8_in, align 64
  %call434 = call <8 x double> @_Z6selectDv8_dS_Dv8_m(<8 x double> %728, <8 x double> %729, <8 x i64> %730) nounwind readnone
  store <8 x double> %call434, <8 x double>* %a8_out, align 64
  %731 = load <16 x double>* %a16_in, align 128
  %732 = load <16 x double>* %b16_in, align 128
  %733 = load <16 x i64>* %ul16_in, align 128
  %call435 = call <16 x double> @_Z6selectDv16_dS_Dv16_m(<16 x double> %731, <16 x double> %732, <16 x i64> %733) nounwind readnone
  store <16 x double> %call435, <16 x double>* %a16_out, align 128
  %734 = load double* %a_in, align 8
  %735 = load double* %b_in, align 8
  %call436 = call double @_Z9remainderdd(double %734, double %735) nounwind readnone
  store double %call436, double* %a_out, align 8
  %736 = load <4 x double>* %a4_in, align 32
  %737 = load <4 x double>* %b4_in, align 32
  %call437 = call <4 x double> @_Z9remainderDv4_dS_(<4 x double> %736, <4 x double> %737) nounwind readnone
  store <4 x double> %call437, <4 x double>* %a4_out, align 32
  %738 = load <8 x double>* %a8_in, align 64
  %739 = load <8 x double>* %b8_in, align 64
  %call438 = call <8 x double> @_Z9remainderDv8_dS_(<8 x double> %738, <8 x double> %739) nounwind readnone
  store <8 x double> %call438, <8 x double>* %a8_out, align 64
  %740 = load <16 x double>* %a16_in, align 128
  %741 = load <16 x double>* %b16_in, align 128
  %call439 = call <16 x double> @_Z9remainderDv16_dS_(<16 x double> %740, <16 x double> %741) nounwind readnone
  store <16 x double> %call439, <16 x double>* %a16_out, align 128
  %742 = load double* %a_in, align 8
  %743 = load double* %b_in, align 8
  %call440 = call double @_Z6remquoddPi(double %742, double %743, i32* %i_out)
  store double %call440, double* %a_out, align 8
  %744 = load <2 x double>* %a2_in, align 16
  %745 = load <2 x double>* %b2_in, align 16
  %call441 = call <2 x double> @_Z6remquoDv2_dS_PDv2_i(<2 x double> %744, <2 x double> %745, <2 x i32>* %i2_out)
  store <2 x double> %call441, <2 x double>* %a2_out, align 16
  %746 = load <3 x double>* %a3_in, align 32
  %747 = load <3 x double>* %b3_in, align 32
  %call442 = call <3 x double> @_Z6remquoDv3_dS_PDv3_i(<3 x double> %746, <3 x double> %747, <3 x i32>* %i3_out)
  store <3 x double> %call442, <3 x double>* %a3_out, align 32
  %748 = load <4 x double>* %a4_in, align 32
  %749 = load <4 x double>* %b4_in, align 32
  %call443 = call <4 x double> @_Z6remquoDv4_dS_PDv4_i(<4 x double> %748, <4 x double> %749, <4 x i32>* %i4_out)
  store <4 x double> %call443, <4 x double>* %a4_out, align 32
  %750 = load <8 x double>* %a8_in, align 64
  %751 = load <8 x double>* %b8_in, align 64
  %call444 = call <8 x double> @_Z6remquoDv8_dS_PDv8_i(<8 x double> %750, <8 x double> %751, <8 x i32>* %i8_out)
  store <8 x double> %call444, <8 x double>* %a8_out, align 64
  %752 = load <16 x double>* %a16_in, align 128
  %753 = load <16 x double>* %b16_in, align 128
  %call445 = call <16 x double> @_Z6remquoDv16_dS_PDv16_i(<16 x double> %752, <16 x double> %753, <16 x i32>* %i16_out)
  store <16 x double> %call445, <16 x double>* %a16_out, align 128
  %754 = load <2 x double>* %a2_in, align 16
  %755 = load <2 x i64>* %ul2_in, align 16
  %call446 = call <2 x double> @_Z7shuffleDv2_dDv2_m(<2 x double> %754, <2 x i64> %755) nounwind readnone
  store <2 x double> %call446, <2 x double>* %a2_out, align 16
  %756 = load <4 x double>* %a4_in, align 32
  %757 = load <2 x i64>* %ul2_in, align 16
  %call447 = call <2 x double> @_Z7shuffleDv4_dDv2_m(<4 x double> %756, <2 x i64> %757) nounwind readnone
  store <2 x double> %call447, <2 x double>* %a2_out, align 16
  %758 = load <8 x double>* %a8_in, align 64
  %759 = load <2 x i64>* %ul2_in, align 16
  %call448 = call <2 x double> @_Z7shuffleDv8_dDv2_m(<8 x double> %758, <2 x i64> %759) nounwind readnone
  store <2 x double> %call448, <2 x double>* %a2_out, align 16
  %760 = load <16 x double>* %a16_in, align 128
  %761 = load <2 x i64>* %ul2_in, align 16
  %call449 = call <2 x double> @_Z7shuffleDv16_dDv2_m(<16 x double> %760, <2 x i64> %761) nounwind readnone
  store <2 x double> %call449, <2 x double>* %a2_out, align 16
  %762 = load <2 x double>* %a2_in, align 16
  %763 = load <4 x i64>* %ul4_in, align 32
  %call450 = call <4 x double> @_Z7shuffleDv2_dDv4_m(<2 x double> %762, <4 x i64> %763) nounwind readnone
  store <4 x double> %call450, <4 x double>* %a4_out, align 32
  %764 = load <4 x double>* %a4_in, align 32
  %765 = load <4 x i64>* %ul4_in, align 32
  %call451 = call <4 x double> @_Z7shuffleDv4_dDv4_m(<4 x double> %764, <4 x i64> %765) nounwind readnone
  store <4 x double> %call451, <4 x double>* %a4_out, align 32
  %766 = load <8 x double>* %a8_in, align 64
  %767 = load <4 x i64>* %ul4_in, align 32
  %call452 = call <4 x double> @_Z7shuffleDv8_dDv4_m(<8 x double> %766, <4 x i64> %767) nounwind readnone
  store <4 x double> %call452, <4 x double>* %a4_out, align 32
  %768 = load <16 x double>* %a16_in, align 128
  %769 = load <4 x i64>* %ul4_in, align 32
  %call453 = call <4 x double> @_Z7shuffleDv16_dDv4_m(<16 x double> %768, <4 x i64> %769) nounwind readnone
  store <4 x double> %call453, <4 x double>* %a4_out, align 32
  %770 = load <2 x double>* %a2_in, align 16
  %771 = load <8 x i64>* %ul8_in, align 64
  %call454 = call <8 x double> @_Z7shuffleDv2_dDv8_m(<2 x double> %770, <8 x i64> %771) nounwind readnone
  store <8 x double> %call454, <8 x double>* %a8_out, align 64
  %772 = load <4 x double>* %a4_in, align 32
  %773 = load <8 x i64>* %ul8_in, align 64
  %call455 = call <8 x double> @_Z7shuffleDv4_dDv8_m(<4 x double> %772, <8 x i64> %773) nounwind readnone
  store <8 x double> %call455, <8 x double>* %a8_out, align 64
  %774 = load <8 x double>* %a8_in, align 64
  %775 = load <8 x i64>* %ul8_in, align 64
  %call456 = call <8 x double> @_Z7shuffleDv8_dDv8_m(<8 x double> %774, <8 x i64> %775) nounwind readnone
  store <8 x double> %call456, <8 x double>* %a8_out, align 64
  %776 = load <16 x double>* %a16_in, align 128
  %777 = load <8 x i64>* %ul8_in, align 64
  %call457 = call <8 x double> @_Z7shuffleDv16_dDv8_m(<16 x double> %776, <8 x i64> %777) nounwind readnone
  store <8 x double> %call457, <8 x double>* %a8_out, align 64
  %778 = load <2 x double>* %a2_in, align 16
  %779 = load <16 x i64>* %ul16_in, align 128
  %call458 = call <16 x double> @_Z7shuffleDv2_dDv16_m(<2 x double> %778, <16 x i64> %779) nounwind readnone
  store <16 x double> %call458, <16 x double>* %a16_out, align 128
  %780 = load <4 x double>* %a4_in, align 32
  %781 = load <16 x i64>* %ul16_in, align 128
  %call459 = call <16 x double> @_Z7shuffleDv4_dDv16_m(<4 x double> %780, <16 x i64> %781) nounwind readnone
  store <16 x double> %call459, <16 x double>* %a16_out, align 128
  %782 = load <8 x double>* %a8_in, align 64
  %783 = load <16 x i64>* %ul16_in, align 128
  %call460 = call <16 x double> @_Z7shuffleDv8_dDv16_m(<8 x double> %782, <16 x i64> %783) nounwind readnone
  store <16 x double> %call460, <16 x double>* %a16_out, align 128
  %784 = load <16 x double>* %a16_in, align 128
  %785 = load <16 x i64>* %ul16_in, align 128
  %call461 = call <16 x double> @_Z7shuffleDv16_dDv16_m(<16 x double> %784, <16 x i64> %785) nounwind readnone
  store <16 x double> %call461, <16 x double>* %a16_out, align 128
  %786 = load <2 x double>* %a2_in, align 16
  %787 = load <2 x double>* %b2_in, align 16
  %788 = load <2 x i64>* %ul2_in, align 16
  %call462 = call <2 x double> @_Z8shuffle2Dv2_dS_Dv2_m(<2 x double> %786, <2 x double> %787, <2 x i64> %788) nounwind readnone
  store <2 x double> %call462, <2 x double>* %a2_out, align 16
  %789 = load <4 x double>* %a4_in, align 32
  %790 = load <4 x double>* %b4_in, align 32
  %791 = load <2 x i64>* %ul2_in, align 16
  %call463 = call <2 x double> @_Z8shuffle2Dv4_dS_Dv2_m(<4 x double> %789, <4 x double> %790, <2 x i64> %791) nounwind readnone
  store <2 x double> %call463, <2 x double>* %a2_out, align 16
  %792 = load <8 x double>* %a8_in, align 64
  %793 = load <8 x double>* %b8_in, align 64
  %794 = load <2 x i64>* %ul2_in, align 16
  %call464 = call <2 x double> @_Z8shuffle2Dv8_dS_Dv2_m(<8 x double> %792, <8 x double> %793, <2 x i64> %794) nounwind readnone
  store <2 x double> %call464, <2 x double>* %a2_out, align 16
  %795 = load <16 x double>* %a16_in, align 128
  %796 = load <16 x double>* %b16_in, align 128
  %797 = load <2 x i64>* %ul2_in, align 16
  %call465 = call <2 x double> @_Z8shuffle2Dv16_dS_Dv2_m(<16 x double> %795, <16 x double> %796, <2 x i64> %797) nounwind readnone
  store <2 x double> %call465, <2 x double>* %a2_out, align 16
  %798 = load <2 x double>* %a2_in, align 16
  %799 = load <2 x double>* %b2_in, align 16
  %800 = load <4 x i64>* %ul4_in, align 32
  %call466 = call <4 x double> @_Z8shuffle2Dv2_dS_Dv4_m(<2 x double> %798, <2 x double> %799, <4 x i64> %800) nounwind readnone
  store <4 x double> %call466, <4 x double>* %a4_out, align 32
  %801 = load <4 x double>* %a4_in, align 32
  %802 = load <4 x double>* %b4_in, align 32
  %803 = load <4 x i64>* %ul4_in, align 32
  %call467 = call <4 x double> @_Z8shuffle2Dv4_dS_Dv4_m(<4 x double> %801, <4 x double> %802, <4 x i64> %803) nounwind readnone
  store <4 x double> %call467, <4 x double>* %a4_out, align 32
  %804 = load <8 x double>* %a8_in, align 64
  %805 = load <8 x double>* %b8_in, align 64
  %806 = load <4 x i64>* %ul4_in, align 32
  %call468 = call <4 x double> @_Z8shuffle2Dv8_dS_Dv4_m(<8 x double> %804, <8 x double> %805, <4 x i64> %806) nounwind readnone
  store <4 x double> %call468, <4 x double>* %a4_out, align 32
  %807 = load <16 x double>* %a16_in, align 128
  %808 = load <16 x double>* %b16_in, align 128
  %809 = load <4 x i64>* %ul4_in, align 32
  %call469 = call <4 x double> @_Z8shuffle2Dv16_dS_Dv4_m(<16 x double> %807, <16 x double> %808, <4 x i64> %809) nounwind readnone
  store <4 x double> %call469, <4 x double>* %a4_out, align 32
  %810 = load <2 x double>* %a2_in, align 16
  %811 = load <2 x double>* %b2_in, align 16
  %812 = load <8 x i64>* %ul8_in, align 64
  %call470 = call <8 x double> @_Z8shuffle2Dv2_dS_Dv8_m(<2 x double> %810, <2 x double> %811, <8 x i64> %812) nounwind readnone
  store <8 x double> %call470, <8 x double>* %a8_out, align 64
  %813 = load <4 x double>* %a4_in, align 32
  %814 = load <4 x double>* %b4_in, align 32
  %815 = load <8 x i64>* %ul8_in, align 64
  %call471 = call <8 x double> @_Z8shuffle2Dv4_dS_Dv8_m(<4 x double> %813, <4 x double> %814, <8 x i64> %815) nounwind readnone
  store <8 x double> %call471, <8 x double>* %a8_out, align 64
  %816 = load <8 x double>* %a8_in, align 64
  %817 = load <8 x double>* %b8_in, align 64
  %818 = load <8 x i64>* %ul8_in, align 64
  %call472 = call <8 x double> @_Z8shuffle2Dv8_dS_Dv8_m(<8 x double> %816, <8 x double> %817, <8 x i64> %818) nounwind readnone
  store <8 x double> %call472, <8 x double>* %a8_out, align 64
  %819 = load <16 x double>* %a16_in, align 128
  %820 = load <16 x double>* %b16_in, align 128
  %821 = load <8 x i64>* %ul8_in, align 64
  %call473 = call <8 x double> @_Z8shuffle2Dv16_dS_Dv8_m(<16 x double> %819, <16 x double> %820, <8 x i64> %821) nounwind readnone
  store <8 x double> %call473, <8 x double>* %a8_out, align 64
  %822 = load <2 x double>* %a2_in, align 16
  %823 = load <2 x double>* %b2_in, align 16
  %824 = load <16 x i64>* %ul16_in, align 128
  %call474 = call <16 x double> @_Z8shuffle2Dv2_dS_Dv16_m(<2 x double> %822, <2 x double> %823, <16 x i64> %824) nounwind readnone
  store <16 x double> %call474, <16 x double>* %a16_out, align 128
  %825 = load <4 x double>* %a4_in, align 32
  %826 = load <4 x double>* %b4_in, align 32
  %827 = load <16 x i64>* %ul16_in, align 128
  %call475 = call <16 x double> @_Z8shuffle2Dv4_dS_Dv16_m(<4 x double> %825, <4 x double> %826, <16 x i64> %827) nounwind readnone
  store <16 x double> %call475, <16 x double>* %a16_out, align 128
  %828 = load <8 x double>* %a8_in, align 64
  %829 = load <8 x double>* %b8_in, align 64
  %830 = load <16 x i64>* %ul16_in, align 128
  %call476 = call <16 x double> @_Z8shuffle2Dv8_dS_Dv16_m(<8 x double> %828, <8 x double> %829, <16 x i64> %830) nounwind readnone
  store <16 x double> %call476, <16 x double>* %a16_out, align 128
  %831 = load <16 x double>* %a16_in, align 128
  %832 = load <16 x double>* %b16_in, align 128
  %833 = load <16 x i64>* %ul16_in, align 128
  %call477 = call <16 x double> @_Z8shuffle2Dv16_dS_Dv16_m(<16 x double> %831, <16 x double> %832, <16 x i64> %833) nounwind readnone
  store <16 x double> %call477, <16 x double>* %a16_out, align 128
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

declare <2 x double> @_Z7shuffleDv2_dDv2_m(<2 x double>, <2 x i64>) nounwind readnone

declare <2 x double> @_Z7shuffleDv4_dDv2_m(<4 x double>, <2 x i64>) nounwind readnone

declare <2 x double> @_Z7shuffleDv8_dDv2_m(<8 x double>, <2 x i64>) nounwind readnone

declare <2 x double> @_Z7shuffleDv16_dDv2_m(<16 x double>, <2 x i64>) nounwind readnone

declare <4 x double> @_Z7shuffleDv2_dDv4_m(<2 x double>, <4 x i64>) nounwind readnone

declare <4 x double> @_Z7shuffleDv4_dDv4_m(<4 x double>, <4 x i64>) nounwind readnone

declare <4 x double> @_Z7shuffleDv8_dDv4_m(<8 x double>, <4 x i64>) nounwind readnone

declare <4 x double> @_Z7shuffleDv16_dDv4_m(<16 x double>, <4 x i64>) nounwind readnone

declare <8 x double> @_Z7shuffleDv2_dDv8_m(<2 x double>, <8 x i64>) nounwind readnone

declare <8 x double> @_Z7shuffleDv4_dDv8_m(<4 x double>, <8 x i64>) nounwind readnone

declare <8 x double> @_Z7shuffleDv8_dDv8_m(<8 x double>, <8 x i64>) nounwind readnone

declare <8 x double> @_Z7shuffleDv16_dDv8_m(<16 x double>, <8 x i64>) nounwind readnone

declare <16 x double> @_Z7shuffleDv2_dDv16_m(<2 x double>, <16 x i64>) nounwind readnone

declare <16 x double> @_Z7shuffleDv4_dDv16_m(<4 x double>, <16 x i64>) nounwind readnone

declare <16 x double> @_Z7shuffleDv8_dDv16_m(<8 x double>, <16 x i64>) nounwind readnone

declare <16 x double> @_Z7shuffleDv16_dDv16_m(<16 x double>, <16 x i64>) nounwind readnone

declare <2 x double> @_Z8shuffle2Dv2_dS_Dv2_m(<2 x double>, <2 x double>, <2 x i64>) nounwind readnone

declare <2 x double> @_Z8shuffle2Dv4_dS_Dv2_m(<4 x double>, <4 x double>, <2 x i64>) nounwind readnone

declare <2 x double> @_Z8shuffle2Dv8_dS_Dv2_m(<8 x double>, <8 x double>, <2 x i64>) nounwind readnone

declare <2 x double> @_Z8shuffle2Dv16_dS_Dv2_m(<16 x double>, <16 x double>, <2 x i64>) nounwind readnone

declare <4 x double> @_Z8shuffle2Dv2_dS_Dv4_m(<2 x double>, <2 x double>, <4 x i64>) nounwind readnone

declare <4 x double> @_Z8shuffle2Dv4_dS_Dv4_m(<4 x double>, <4 x double>, <4 x i64>) nounwind readnone

declare <4 x double> @_Z8shuffle2Dv8_dS_Dv4_m(<8 x double>, <8 x double>, <4 x i64>) nounwind readnone

declare <4 x double> @_Z8shuffle2Dv16_dS_Dv4_m(<16 x double>, <16 x double>, <4 x i64>) nounwind readnone

declare <8 x double> @_Z8shuffle2Dv2_dS_Dv8_m(<2 x double>, <2 x double>, <8 x i64>) nounwind readnone

declare <8 x double> @_Z8shuffle2Dv4_dS_Dv8_m(<4 x double>, <4 x double>, <8 x i64>) nounwind readnone

declare <8 x double> @_Z8shuffle2Dv8_dS_Dv8_m(<8 x double>, <8 x double>, <8 x i64>) nounwind readnone

declare <8 x double> @_Z8shuffle2Dv16_dS_Dv8_m(<16 x double>, <16 x double>, <8 x i64>) nounwind readnone

declare <16 x double> @_Z8shuffle2Dv2_dS_Dv16_m(<2 x double>, <2 x double>, <16 x i64>) nounwind readnone

declare <16 x double> @_Z8shuffle2Dv4_dS_Dv16_m(<4 x double>, <4 x double>, <16 x i64>) nounwind readnone

declare <16 x double> @_Z8shuffle2Dv8_dS_Dv16_m(<8 x double>, <8 x double>, <16 x i64>) nounwind readnone

declare <16 x double> @_Z8shuffle2Dv16_dS_Dv16_m(<16 x double>, <16 x double>, <16 x i64>) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i64 addrspace(1)*, i32)* @oclbuiltin, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
