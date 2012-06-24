; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN_XXX: NEATChecker -r %s -a %s.neat -t 0
; TODO: add NEATCHECKER instrumentation

; ModuleID = 'oclbuiltin.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @oclbuiltin(float addrspace(1)* %input, float addrspace(1)* %output, i32 addrspace(1)* %inputInt, i32 addrspace(1)* %outputInt, i32 %buffer_size) nounwind {
  %1 = alloca float addrspace(1)*, align 4
  %2 = alloca float addrspace(1)*, align 4
  %3 = alloca i32 addrspace(1)*, align 4
  %4 = alloca i32 addrspace(1)*, align 4
  %5 = alloca i32, align 4
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
  store float addrspace(1)* %input, float addrspace(1)** %1, align 4
  store float addrspace(1)* %output, float addrspace(1)** %2, align 4
  store i32 addrspace(1)* %inputInt, i32 addrspace(1)** %3, align 4
  store i32 addrspace(1)* %outputInt, i32 addrspace(1)** %4, align 4
  store i32 %buffer_size, i32* %5, align 4
  store i32 0, i32* %tid, align 4
  %6 = load i32* %tid, align 4
  %7 = load float addrspace(1)** %1, align 4
  %8 = getelementptr inbounds float addrspace(1)* %7, i32 %6
  %9 = load float addrspace(1)* %8
  store float %9, float* %a_in, align 4
  %10 = load i32* %tid, align 4
  %11 = load float addrspace(1)** %1, align 4
  %12 = getelementptr inbounds float addrspace(1)* %11, i32 %10
  %13 = load float addrspace(1)* %12
  %14 = insertelement <2 x float> undef, float %13, i32 0
  %15 = shufflevector <2 x float> %14, <2 x float> %14, <2 x i32> zeroinitializer
  store <2 x float> %15, <2 x float>* %a2_in, align 8
  %16 = load i32* %tid, align 4
  %17 = load float addrspace(1)** %1, align 4
  %18 = getelementptr inbounds float addrspace(1)* %17, i32 %16
  %19 = load float addrspace(1)* %18
  %20 = insertelement <3 x float> undef, float %19, i32 0
  %21 = shufflevector <3 x float> %20, <3 x float> %20, <3 x i32> zeroinitializer
  store <3 x float> %21, <3 x float>* %a3_in, align 16
  %22 = load i32* %tid, align 4
  %23 = load float addrspace(1)** %1, align 4
  %24 = getelementptr inbounds float addrspace(1)* %23, i32 %22
  %25 = load float addrspace(1)* %24
  %26 = insertelement <4 x float> undef, float %25, i32 0
  %27 = shufflevector <4 x float> %26, <4 x float> %26, <4 x i32> zeroinitializer
  store <4 x float> %27, <4 x float>* %a4_in, align 16
  %28 = load i32* %tid, align 4
  %29 = load float addrspace(1)** %1, align 4
  %30 = getelementptr inbounds float addrspace(1)* %29, i32 %28
  %31 = load float addrspace(1)* %30
  %32 = insertelement <8 x float> undef, float %31, i32 0
  %33 = shufflevector <8 x float> %32, <8 x float> %32, <8 x i32> zeroinitializer
  store <8 x float> %33, <8 x float>* %a8_in, align 32
  %34 = load i32* %tid, align 4
  %35 = load float addrspace(1)** %1, align 4
  %36 = getelementptr inbounds float addrspace(1)* %35, i32 %34
  %37 = load float addrspace(1)* %36
  %38 = insertelement <16 x float> undef, float %37, i32 0
  %39 = shufflevector <16 x float> %38, <16 x float> %38, <16 x i32> zeroinitializer
  store <16 x float> %39, <16 x float>* %a16_in, align 64
  %40 = load i32* %tid, align 4
  %41 = add i32 %40, 1
  %42 = load float addrspace(1)** %1, align 4
  %43 = getelementptr inbounds float addrspace(1)* %42, i32 %41
  %44 = load float addrspace(1)* %43
  store float %44, float* %b_in, align 4
  %45 = load i32* %tid, align 4
  %46 = load float addrspace(1)** %1, align 4
  %47 = getelementptr inbounds float addrspace(1)* %46, i32 %45
  %48 = load float addrspace(1)* %47
  %49 = insertelement <2 x float> undef, float %48, i32 0
  %50 = shufflevector <2 x float> %49, <2 x float> %49, <2 x i32> zeroinitializer
  store <2 x float> %50, <2 x float>* %b2_in, align 8
  %51 = load i32* %tid, align 4
  %52 = load float addrspace(1)** %1, align 4
  %53 = getelementptr inbounds float addrspace(1)* %52, i32 %51
  %54 = load float addrspace(1)* %53
  %55 = insertelement <3 x float> undef, float %54, i32 0
  %56 = shufflevector <3 x float> %55, <3 x float> %55, <3 x i32> zeroinitializer
  store <3 x float> %56, <3 x float>* %b3_in, align 16
  %57 = load i32* %tid, align 4
  %58 = load float addrspace(1)** %1, align 4
  %59 = getelementptr inbounds float addrspace(1)* %58, i32 %57
  %60 = load float addrspace(1)* %59
  %61 = insertelement <4 x float> undef, float %60, i32 0
  %62 = shufflevector <4 x float> %61, <4 x float> %61, <4 x i32> zeroinitializer
  store <4 x float> %62, <4 x float>* %b4_in, align 16
  %63 = load i32* %tid, align 4
  %64 = load float addrspace(1)** %1, align 4
  %65 = getelementptr inbounds float addrspace(1)* %64, i32 %63
  %66 = load float addrspace(1)* %65
  %67 = insertelement <8 x float> undef, float %66, i32 0
  %68 = shufflevector <8 x float> %67, <8 x float> %67, <8 x i32> zeroinitializer
  store <8 x float> %68, <8 x float>* %b8_in, align 32
  %69 = load i32* %tid, align 4
  %70 = load float addrspace(1)** %1, align 4
  %71 = getelementptr inbounds float addrspace(1)* %70, i32 %69
  %72 = load float addrspace(1)* %71
  %73 = insertelement <16 x float> undef, float %72, i32 0
  %74 = shufflevector <16 x float> %73, <16 x float> %73, <16 x i32> zeroinitializer
  store <16 x float> %74, <16 x float>* %b16_in, align 64
  %75 = load i32* %tid, align 4
  %76 = add i32 %75, 1
  %77 = load float addrspace(1)** %1, align 4
  %78 = getelementptr inbounds float addrspace(1)* %77, i32 %76
  %79 = load float addrspace(1)* %78
  store float %79, float* %c_in, align 4
  %80 = load i32* %tid, align 4
  %81 = load float addrspace(1)** %1, align 4
  %82 = getelementptr inbounds float addrspace(1)* %81, i32 %80
  %83 = load float addrspace(1)* %82
  %84 = insertelement <2 x float> undef, float %83, i32 0
  %85 = shufflevector <2 x float> %84, <2 x float> %84, <2 x i32> zeroinitializer
  store <2 x float> %85, <2 x float>* %c2_in, align 8
  %86 = load i32* %tid, align 4
  %87 = load float addrspace(1)** %1, align 4
  %88 = getelementptr inbounds float addrspace(1)* %87, i32 %86
  %89 = load float addrspace(1)* %88
  %90 = insertelement <3 x float> undef, float %89, i32 0
  %91 = shufflevector <3 x float> %90, <3 x float> %90, <3 x i32> zeroinitializer
  store <3 x float> %91, <3 x float>* %c3_in, align 16
  %92 = load i32* %tid, align 4
  %93 = load float addrspace(1)** %1, align 4
  %94 = getelementptr inbounds float addrspace(1)* %93, i32 %92
  %95 = load float addrspace(1)* %94
  %96 = insertelement <4 x float> undef, float %95, i32 0
  %97 = shufflevector <4 x float> %96, <4 x float> %96, <4 x i32> zeroinitializer
  store <4 x float> %97, <4 x float>* %c4_in, align 16
  %98 = load i32* %tid, align 4
  %99 = load float addrspace(1)** %1, align 4
  %100 = getelementptr inbounds float addrspace(1)* %99, i32 %98
  %101 = load float addrspace(1)* %100
  %102 = insertelement <8 x float> undef, float %101, i32 0
  %103 = shufflevector <8 x float> %102, <8 x float> %102, <8 x i32> zeroinitializer
  store <8 x float> %103, <8 x float>* %c8_in, align 32
  %104 = load i32* %tid, align 4
  %105 = load float addrspace(1)** %1, align 4
  %106 = getelementptr inbounds float addrspace(1)* %105, i32 %104
  %107 = load float addrspace(1)* %106
  %108 = insertelement <16 x float> undef, float %107, i32 0
  %109 = shufflevector <16 x float> %108, <16 x float> %108, <16 x i32> zeroinitializer
  store <16 x float> %109, <16 x float>* %c16_in, align 64
  %110 = load i32* %tid, align 4
  %111 = load float addrspace(1)** %2, align 4
  %112 = getelementptr inbounds float addrspace(1)* %111, i32 %110
  %113 = load float addrspace(1)* %112
  store float %113, float* %a_out, align 4
  %114 = load i32* %tid, align 4
  %115 = load float addrspace(1)** %2, align 4
  %116 = getelementptr inbounds float addrspace(1)* %115, i32 %114
  %117 = load float addrspace(1)* %116
  %118 = insertelement <2 x float> undef, float %117, i32 0
  %119 = shufflevector <2 x float> %118, <2 x float> %118, <2 x i32> zeroinitializer
  store <2 x float> %119, <2 x float>* %a2_out, align 8
  %120 = load i32* %tid, align 4
  %121 = load float addrspace(1)** %2, align 4
  %122 = getelementptr inbounds float addrspace(1)* %121, i32 %120
  %123 = load float addrspace(1)* %122
  %124 = insertelement <3 x float> undef, float %123, i32 0
  %125 = shufflevector <3 x float> %124, <3 x float> %124, <3 x i32> zeroinitializer
  store <3 x float> %125, <3 x float>* %a3_out, align 16
  %126 = load i32* %tid, align 4
  %127 = load float addrspace(1)** %2, align 4
  %128 = getelementptr inbounds float addrspace(1)* %127, i32 %126
  %129 = load float addrspace(1)* %128
  %130 = insertelement <4 x float> undef, float %129, i32 0
  %131 = shufflevector <4 x float> %130, <4 x float> %130, <4 x i32> zeroinitializer
  store <4 x float> %131, <4 x float>* %a4_out, align 16
  %132 = load i32* %tid, align 4
  %133 = load float addrspace(1)** %2, align 4
  %134 = getelementptr inbounds float addrspace(1)* %133, i32 %132
  %135 = load float addrspace(1)* %134
  %136 = insertelement <8 x float> undef, float %135, i32 0
  %137 = shufflevector <8 x float> %136, <8 x float> %136, <8 x i32> zeroinitializer
  store <8 x float> %137, <8 x float>* %a8_out, align 32
  %138 = load i32* %tid, align 4
  %139 = load float addrspace(1)** %2, align 4
  %140 = getelementptr inbounds float addrspace(1)* %139, i32 %138
  %141 = load float addrspace(1)* %140
  %142 = insertelement <16 x float> undef, float %141, i32 0
  %143 = shufflevector <16 x float> %142, <16 x float> %142, <16 x i32> zeroinitializer
  store <16 x float> %143, <16 x float>* %a16_out, align 64
  %144 = load i32* %tid, align 4
  %145 = load float addrspace(1)** %2, align 4
  %146 = getelementptr inbounds float addrspace(1)* %145, i32 %144
  %147 = load float addrspace(1)* %146
  store float %147, float* %b_out, align 4
  %148 = load i32* %tid, align 4
  %149 = load float addrspace(1)** %2, align 4
  %150 = getelementptr inbounds float addrspace(1)* %149, i32 %148
  %151 = load float addrspace(1)* %150
  %152 = insertelement <2 x float> undef, float %151, i32 0
  %153 = shufflevector <2 x float> %152, <2 x float> %152, <2 x i32> zeroinitializer
  store <2 x float> %153, <2 x float>* %b2_out, align 8
  %154 = load i32* %tid, align 4
  %155 = load float addrspace(1)** %2, align 4
  %156 = getelementptr inbounds float addrspace(1)* %155, i32 %154
  %157 = load float addrspace(1)* %156
  %158 = insertelement <3 x float> undef, float %157, i32 0
  %159 = shufflevector <3 x float> %158, <3 x float> %158, <3 x i32> zeroinitializer
  store <3 x float> %159, <3 x float>* %b3_out, align 16
  %160 = load i32* %tid, align 4
  %161 = load float addrspace(1)** %2, align 4
  %162 = getelementptr inbounds float addrspace(1)* %161, i32 %160
  %163 = load float addrspace(1)* %162
  %164 = insertelement <4 x float> undef, float %163, i32 0
  %165 = shufflevector <4 x float> %164, <4 x float> %164, <4 x i32> zeroinitializer
  store <4 x float> %165, <4 x float>* %b4_out, align 16
  %166 = load i32* %tid, align 4
  %167 = load float addrspace(1)** %2, align 4
  %168 = getelementptr inbounds float addrspace(1)* %167, i32 %166
  %169 = load float addrspace(1)* %168
  %170 = insertelement <8 x float> undef, float %169, i32 0
  %171 = shufflevector <8 x float> %170, <8 x float> %170, <8 x i32> zeroinitializer
  store <8 x float> %171, <8 x float>* %b8_out, align 32
  %172 = load i32* %tid, align 4
  %173 = load float addrspace(1)** %2, align 4
  %174 = getelementptr inbounds float addrspace(1)* %173, i32 %172
  %175 = load float addrspace(1)* %174
  %176 = insertelement <16 x float> undef, float %175, i32 0
  %177 = shufflevector <16 x float> %176, <16 x float> %176, <16 x i32> zeroinitializer
  store <16 x float> %177, <16 x float>* %b16_out, align 64
  %178 = load i32* %tid, align 4
  %179 = load float addrspace(1)** %2, align 4
  %180 = getelementptr inbounds float addrspace(1)* %179, i32 %178
  %181 = load float addrspace(1)* %180
  store float %181, float* %c_out, align 4
  %182 = load i32* %tid, align 4
  %183 = load float addrspace(1)** %2, align 4
  %184 = getelementptr inbounds float addrspace(1)* %183, i32 %182
  %185 = load float addrspace(1)* %184
  %186 = insertelement <2 x float> undef, float %185, i32 0
  %187 = shufflevector <2 x float> %186, <2 x float> %186, <2 x i32> zeroinitializer
  store <2 x float> %187, <2 x float>* %c2_out, align 8
  %188 = load i32* %tid, align 4
  %189 = load float addrspace(1)** %2, align 4
  %190 = getelementptr inbounds float addrspace(1)* %189, i32 %188
  %191 = load float addrspace(1)* %190
  %192 = insertelement <3 x float> undef, float %191, i32 0
  %193 = shufflevector <3 x float> %192, <3 x float> %192, <3 x i32> zeroinitializer
  store <3 x float> %193, <3 x float>* %c3_out, align 16
  %194 = load i32* %tid, align 4
  %195 = load float addrspace(1)** %2, align 4
  %196 = getelementptr inbounds float addrspace(1)* %195, i32 %194
  %197 = load float addrspace(1)* %196
  %198 = insertelement <4 x float> undef, float %197, i32 0
  %199 = shufflevector <4 x float> %198, <4 x float> %198, <4 x i32> zeroinitializer
  store <4 x float> %199, <4 x float>* %c4_out, align 16
  %200 = load i32* %tid, align 4
  %201 = load float addrspace(1)** %2, align 4
  %202 = getelementptr inbounds float addrspace(1)* %201, i32 %200
  %203 = load float addrspace(1)* %202
  %204 = insertelement <8 x float> undef, float %203, i32 0
  %205 = shufflevector <8 x float> %204, <8 x float> %204, <8 x i32> zeroinitializer
  store <8 x float> %205, <8 x float>* %c8_out, align 32
  %206 = load i32* %tid, align 4
  %207 = load float addrspace(1)** %2, align 4
  %208 = getelementptr inbounds float addrspace(1)* %207, i32 %206
  %209 = load float addrspace(1)* %208
  %210 = insertelement <16 x float> undef, float %209, i32 0
  %211 = shufflevector <16 x float> %210, <16 x float> %210, <16 x i32> zeroinitializer
  store <16 x float> %211, <16 x float>* %c16_out, align 64
  %212 = load i32* %tid, align 4
  %213 = load i32 addrspace(1)** %3, align 4
  %214 = getelementptr inbounds i32 addrspace(1)* %213, i32 %212
  %215 = load i32 addrspace(1)* %214
  store i32 %215, i32* %i_in, align 4
  %216 = load i32* %tid, align 4
  %217 = load i32 addrspace(1)** %3, align 4
  %218 = getelementptr inbounds i32 addrspace(1)* %217, i32 %216
  %219 = load i32 addrspace(1)* %218
  %220 = insertelement <2 x i32> undef, i32 %219, i32 0
  %221 = shufflevector <2 x i32> %220, <2 x i32> %220, <2 x i32> zeroinitializer
  store <2 x i32> %221, <2 x i32>* %i2_in, align 8
  %222 = load i32* %tid, align 4
  %223 = load i32 addrspace(1)** %3, align 4
  %224 = getelementptr inbounds i32 addrspace(1)* %223, i32 %222
  %225 = load i32 addrspace(1)* %224
  %226 = insertelement <3 x i32> undef, i32 %225, i32 0
  %227 = shufflevector <3 x i32> %226, <3 x i32> %226, <3 x i32> zeroinitializer
  store <3 x i32> %227, <3 x i32>* %i3_in, align 16
  %228 = load i32* %tid, align 4
  %229 = load i32 addrspace(1)** %3, align 4
  %230 = getelementptr inbounds i32 addrspace(1)* %229, i32 %228
  %231 = load i32 addrspace(1)* %230
  %232 = insertelement <4 x i32> undef, i32 %231, i32 0
  %233 = shufflevector <4 x i32> %232, <4 x i32> %232, <4 x i32> zeroinitializer
  store <4 x i32> %233, <4 x i32>* %i4_in, align 16
  %234 = load i32* %tid, align 4
  %235 = load i32 addrspace(1)** %3, align 4
  %236 = getelementptr inbounds i32 addrspace(1)* %235, i32 %234
  %237 = load i32 addrspace(1)* %236
  %238 = insertelement <8 x i32> undef, i32 %237, i32 0
  %239 = shufflevector <8 x i32> %238, <8 x i32> %238, <8 x i32> zeroinitializer
  store <8 x i32> %239, <8 x i32>* %i8_in, align 32
  %240 = load i32* %tid, align 4
  %241 = load i32 addrspace(1)** %3, align 4
  %242 = getelementptr inbounds i32 addrspace(1)* %241, i32 %240
  %243 = load i32 addrspace(1)* %242
  %244 = insertelement <16 x i32> undef, i32 %243, i32 0
  %245 = shufflevector <16 x i32> %244, <16 x i32> %244, <16 x i32> zeroinitializer
  store <16 x i32> %245, <16 x i32>* %i16_in, align 64
  %246 = load i32* %tid, align 4
  %247 = load i32 addrspace(1)** %4, align 4
  %248 = getelementptr inbounds i32 addrspace(1)* %247, i32 %246
  %249 = load i32 addrspace(1)* %248
  store i32 %249, i32* %i_out, align 4
  %250 = load i32* %tid, align 4
  %251 = load i32 addrspace(1)** %4, align 4
  %252 = getelementptr inbounds i32 addrspace(1)* %251, i32 %250
  %253 = load i32 addrspace(1)* %252
  %254 = insertelement <2 x i32> undef, i32 %253, i32 0
  %255 = shufflevector <2 x i32> %254, <2 x i32> %254, <2 x i32> zeroinitializer
  store <2 x i32> %255, <2 x i32>* %i2_out, align 8
  %256 = load i32* %tid, align 4
  %257 = load i32 addrspace(1)** %4, align 4
  %258 = getelementptr inbounds i32 addrspace(1)* %257, i32 %256
  %259 = load i32 addrspace(1)* %258
  %260 = insertelement <3 x i32> undef, i32 %259, i32 0
  %261 = shufflevector <3 x i32> %260, <3 x i32> %260, <3 x i32> zeroinitializer
  store <3 x i32> %261, <3 x i32>* %i3_out, align 16
  %262 = load i32* %tid, align 4
  %263 = load i32 addrspace(1)** %4, align 4
  %264 = getelementptr inbounds i32 addrspace(1)* %263, i32 %262
  %265 = load i32 addrspace(1)* %264
  %266 = insertelement <4 x i32> undef, i32 %265, i32 0
  %267 = shufflevector <4 x i32> %266, <4 x i32> %266, <4 x i32> zeroinitializer
  store <4 x i32> %267, <4 x i32>* %i4_out, align 16
  %268 = load i32* %tid, align 4
  %269 = load i32 addrspace(1)** %4, align 4
  %270 = getelementptr inbounds i32 addrspace(1)* %269, i32 %268
  %271 = load i32 addrspace(1)* %270
  %272 = insertelement <8 x i32> undef, i32 %271, i32 0
  %273 = shufflevector <8 x i32> %272, <8 x i32> %272, <8 x i32> zeroinitializer
  store <8 x i32> %273, <8 x i32>* %i8_out, align 32
  %274 = load i32* %tid, align 4
  %275 = load i32 addrspace(1)** %4, align 4
  %276 = getelementptr inbounds i32 addrspace(1)* %275, i32 %274
  %277 = load i32 addrspace(1)* %276
  %278 = insertelement <16 x i32> undef, i32 %277, i32 0
  %279 = shufflevector <16 x i32> %278, <16 x i32> %278, <16 x i32> zeroinitializer
  store <16 x i32> %279, <16 x i32>* %i16_out, align 64
  %280 = load float* %a_in, align 4
  %281 = call float @_Z4acosf(float %280) readnone
  store float %281, float* %a_out, align 4
  %282 = load <4 x float>* %a4_in, align 16
  %283 = call <4 x float> @_Z4acosDv4_f(<4 x float> %282) readnone
  store <4 x float> %283, <4 x float>* %a4_out, align 16
  %284 = load <8 x float>* %a8_in, align 32
  %285 = call <8 x float> @_Z4acosDv8_f(<8 x float> %284) readnone
  store <8 x float> %285, <8 x float>* %a8_out, align 32
  %286 = load <16 x float>* %a16_in, align 64
  %287 = call <16 x float> @_Z4acosDv16_f(<16 x float> %286) readnone
  store <16 x float> %287, <16 x float>* %a16_out, align 64
  %288 = load float* %a_in, align 4
  %289 = call float @_Z6acospif(float %288) readnone
  store float %289, float* %a_out, align 4
  %290 = load <4 x float>* %a4_in, align 16
  %291 = call <4 x float> @_Z6acospiDv4_f(<4 x float> %290) readnone
  store <4 x float> %291, <4 x float>* %a4_out, align 16
  %292 = load <8 x float>* %a8_in, align 32
  %293 = call <8 x float> @_Z6acospiDv8_f(<8 x float> %292) readnone
  store <8 x float> %293, <8 x float>* %a8_out, align 32
  %294 = load <16 x float>* %a16_in, align 64
  %295 = call <16 x float> @_Z6acospiDv16_f(<16 x float> %294) readnone
  store <16 x float> %295, <16 x float>* %a16_out, align 64
  %296 = load float* %a_in, align 4
  %297 = call float @_Z4asinf(float %296) readnone
  store float %297, float* %a_out, align 4
  %298 = load <4 x float>* %a4_in, align 16
  %299 = call <4 x float> @_Z4asinDv4_f(<4 x float> %298) readnone
  store <4 x float> %299, <4 x float>* %a4_out, align 16
  %300 = load <8 x float>* %a8_in, align 32
  %301 = call <8 x float> @_Z4asinDv8_f(<8 x float> %300) readnone
  store <8 x float> %301, <8 x float>* %a8_out, align 32
  %302 = load <16 x float>* %a16_in, align 64
  %303 = call <16 x float> @_Z4asinDv16_f(<16 x float> %302) readnone
  store <16 x float> %303, <16 x float>* %a16_out, align 64
  %304 = load float* %a_in, align 4
  %305 = call float @_Z6asinpif(float %304) readnone
  store float %305, float* %a_out, align 4
  %306 = load <4 x float>* %a4_in, align 16
  %307 = call <4 x float> @_Z6asinpiDv4_f(<4 x float> %306) readnone
  store <4 x float> %307, <4 x float>* %a4_out, align 16
  %308 = load <8 x float>* %a8_in, align 32
  %309 = call <8 x float> @_Z6asinpiDv8_f(<8 x float> %308) readnone
  store <8 x float> %309, <8 x float>* %a8_out, align 32
  %310 = load <16 x float>* %a16_in, align 64
  %311 = call <16 x float> @_Z6asinpiDv16_f(<16 x float> %310) readnone
  store <16 x float> %311, <16 x float>* %a16_out, align 64
  %312 = load float* %a_in, align 4
  %313 = call float @_Z4atanf(float %312) readnone
  store float %313, float* %a_out, align 4
  %314 = load <4 x float>* %a4_in, align 16
  %315 = call <4 x float> @_Z4atanDv4_f(<4 x float> %314) readnone
  store <4 x float> %315, <4 x float>* %a4_out, align 16
  %316 = load <8 x float>* %a8_in, align 32
  %317 = call <8 x float> @_Z4atanDv8_f(<8 x float> %316) readnone
  store <8 x float> %317, <8 x float>* %a8_out, align 32
  %318 = load <16 x float>* %a16_in, align 64
  %319 = call <16 x float> @_Z4atanDv16_f(<16 x float> %318) readnone
  store <16 x float> %319, <16 x float>* %a16_out, align 64
  %320 = load float* %a_in, align 4
  %321 = load float* %b_in, align 4
  %322 = call float @_Z5atan2ff(float %320, float %321) readnone
  store float %322, float* %a_out, align 4
  %323 = load <4 x float>* %a4_in, align 16
  %324 = load <4 x float>* %b4_in, align 16
  %325 = call <4 x float> @_Z5atan2Dv4_fS_(<4 x float> %323, <4 x float> %324) readnone
  store <4 x float> %325, <4 x float>* %a4_out, align 16
  %326 = load <8 x float>* %a8_in, align 32
  %327 = load <8 x float>* %b8_in, align 32
  %328 = call <8 x float> @_Z5atan2Dv8_fS_(<8 x float> %326, <8 x float> %327) readnone
  store <8 x float> %328, <8 x float>* %a8_out, align 32
  %329 = load <16 x float>* %a16_in, align 64
  %330 = load <16 x float>* %b16_in, align 64
  %331 = call <16 x float> @_Z5atan2Dv16_fS_(<16 x float> %329, <16 x float> %330) readnone
  store <16 x float> %331, <16 x float>* %a16_out, align 64
  %332 = load float* %a_in, align 4
  %333 = load float* %b_in, align 4
  %334 = call float @_Z7atan2piff(float %332, float %333) readnone
  store float %334, float* %a_out, align 4
  %335 = load <4 x float>* %a4_in, align 16
  %336 = load <4 x float>* %b4_in, align 16
  %337 = call <4 x float> @_Z7atan2piDv4_fS_(<4 x float> %335, <4 x float> %336) readnone
  store <4 x float> %337, <4 x float>* %a4_out, align 16
  %338 = load <8 x float>* %a8_in, align 32
  %339 = load <8 x float>* %b8_in, align 32
  %340 = call <8 x float> @_Z7atan2piDv8_fS_(<8 x float> %338, <8 x float> %339) readnone
  store <8 x float> %340, <8 x float>* %a8_out, align 32
  %341 = load <16 x float>* %a16_in, align 64
  %342 = load <16 x float>* %b16_in, align 64
  %343 = call <16 x float> @_Z7atan2piDv16_fS_(<16 x float> %341, <16 x float> %342) readnone
  store <16 x float> %343, <16 x float>* %a16_out, align 64
  %344 = load float* %a_in, align 4
  %345 = call float @_Z6atanpif(float %344) readnone
  store float %345, float* %a_out, align 4
  %346 = load <4 x float>* %a4_in, align 16
  %347 = call <4 x float> @_Z6atanpiDv4_f(<4 x float> %346) readnone
  store <4 x float> %347, <4 x float>* %a4_out, align 16
  %348 = load <8 x float>* %a8_in, align 32
  %349 = call <8 x float> @_Z6atanpiDv8_f(<8 x float> %348) readnone
  store <8 x float> %349, <8 x float>* %a8_out, align 32
  %350 = load <16 x float>* %a16_in, align 64
  %351 = call <16 x float> @_Z6atanpiDv16_f(<16 x float> %350) readnone
  store <16 x float> %351, <16 x float>* %a16_out, align 64
  %352 = load float* %a_in, align 4
  %353 = call float @_Z3cosf(float %352) readnone
  store float %353, float* %a_out, align 4
  %354 = load <4 x float>* %a4_in, align 16
  %355 = call <4 x float> @_Z3cosDv4_f(<4 x float> %354) readnone
  store <4 x float> %355, <4 x float>* %a4_out, align 16
  %356 = load <8 x float>* %a8_in, align 32
  %357 = call <8 x float> @_Z3cosDv8_f(<8 x float> %356) readnone
  store <8 x float> %357, <8 x float>* %a8_out, align 32
  %358 = load <16 x float>* %a16_in, align 64
  %359 = call <16 x float> @_Z3cosDv16_f(<16 x float> %358) readnone
  store <16 x float> %359, <16 x float>* %a16_out, align 64
  %360 = load float* %a_in, align 4
  %361 = call float @_Z4coshf(float %360) readnone
  store float %361, float* %a_out, align 4
  %362 = load <4 x float>* %a4_in, align 16
  %363 = call <4 x float> @_Z4coshDv4_f(<4 x float> %362) readnone
  store <4 x float> %363, <4 x float>* %a4_out, align 16
  %364 = load <8 x float>* %a8_in, align 32
  %365 = call <8 x float> @_Z4coshDv8_f(<8 x float> %364) readnone
  store <8 x float> %365, <8 x float>* %a8_out, align 32
  %366 = load <16 x float>* %a16_in, align 64
  %367 = call <16 x float> @_Z4coshDv16_f(<16 x float> %366) readnone
  store <16 x float> %367, <16 x float>* %a16_out, align 64
  %368 = load float* %a_in, align 4
  %369 = call float @_Z5cospif(float %368) readnone
  store float %369, float* %a_out, align 4
  %370 = load <4 x float>* %a4_in, align 16
  %371 = call <4 x float> @_Z5cospiDv4_f(<4 x float> %370) readnone
  store <4 x float> %371, <4 x float>* %a4_out, align 16
  %372 = load <8 x float>* %a8_in, align 32
  %373 = call <8 x float> @_Z5cospiDv8_f(<8 x float> %372) readnone
  store <8 x float> %373, <8 x float>* %a8_out, align 32
  %374 = load <16 x float>* %a16_in, align 64
  %375 = call <16 x float> @_Z5cospiDv16_f(<16 x float> %374) readnone
  store <16 x float> %375, <16 x float>* %a16_out, align 64
  %376 = load float* %a_in, align 4
  %377 = call float @_Z3expf(float %376) readnone
  store float %377, float* %a_out, align 4
  %378 = load <4 x float>* %a4_in, align 16
  %379 = call <4 x float> @_Z3expDv4_f(<4 x float> %378) readnone
  store <4 x float> %379, <4 x float>* %a4_out, align 16
  %380 = load <8 x float>* %a8_in, align 32
  %381 = call <8 x float> @_Z3expDv8_f(<8 x float> %380) readnone
  store <8 x float> %381, <8 x float>* %a8_out, align 32
  %382 = load <16 x float>* %a16_in, align 64
  %383 = call <16 x float> @_Z3expDv16_f(<16 x float> %382) readnone
  store <16 x float> %383, <16 x float>* %a16_out, align 64
  %384 = load float* %a_in, align 4
  %385 = call float @_Z4exp2f(float %384) readnone
  store float %385, float* %a_out, align 4
  %386 = load <4 x float>* %a4_in, align 16
  %387 = call <4 x float> @_Z4exp2Dv4_f(<4 x float> %386) readnone
  store <4 x float> %387, <4 x float>* %a4_out, align 16
  %388 = load <8 x float>* %a8_in, align 32
  %389 = call <8 x float> @_Z4exp2Dv8_f(<8 x float> %388) readnone
  store <8 x float> %389, <8 x float>* %a8_out, align 32
  %390 = load <16 x float>* %a16_in, align 64
  %391 = call <16 x float> @_Z4exp2Dv16_f(<16 x float> %390) readnone
  store <16 x float> %391, <16 x float>* %a16_out, align 64
  %392 = load float* %a_in, align 4
  %393 = call float @_Z5exp10f(float %392) readnone
  store float %393, float* %a_out, align 4
  %394 = load <4 x float>* %a4_in, align 16
  %395 = call <4 x float> @_Z5exp10Dv4_f(<4 x float> %394) readnone
  store <4 x float> %395, <4 x float>* %a4_out, align 16
  %396 = load <8 x float>* %a8_in, align 32
  %397 = call <8 x float> @_Z5exp10Dv8_f(<8 x float> %396) readnone
  store <8 x float> %397, <8 x float>* %a8_out, align 32
  %398 = load <16 x float>* %a16_in, align 64
  %399 = call <16 x float> @_Z5exp10Dv16_f(<16 x float> %398) readnone
  store <16 x float> %399, <16 x float>* %a16_out, align 64
  %400 = load float* %a_in, align 4
  %401 = call float @_Z5expm1f(float %400) readnone
  store float %401, float* %a_out, align 4
  %402 = load <4 x float>* %a4_in, align 16
  %403 = call <4 x float> @_Z5expm1Dv4_f(<4 x float> %402) readnone
  store <4 x float> %403, <4 x float>* %a4_out, align 16
  %404 = load <8 x float>* %a8_in, align 32
  %405 = call <8 x float> @_Z5expm1Dv8_f(<8 x float> %404) readnone
  store <8 x float> %405, <8 x float>* %a8_out, align 32
  %406 = load <16 x float>* %a16_in, align 64
  %407 = call <16 x float> @_Z5expm1Dv16_f(<16 x float> %406) readnone
  store <16 x float> %407, <16 x float>* %a16_out, align 64
  %408 = load float* %a_in, align 4
  %409 = call float @_Z3logf(float %408) readnone
  store float %409, float* %a_out, align 4
  %410 = load <4 x float>* %a4_in, align 16
  %411 = call <4 x float> @_Z3logDv4_f(<4 x float> %410) readnone
  store <4 x float> %411, <4 x float>* %a4_out, align 16
  %412 = load <8 x float>* %a8_in, align 32
  %413 = call <8 x float> @_Z3logDv8_f(<8 x float> %412) readnone
  store <8 x float> %413, <8 x float>* %a8_out, align 32
  %414 = load <16 x float>* %a16_in, align 64
  %415 = call <16 x float> @_Z3logDv16_f(<16 x float> %414) readnone
  store <16 x float> %415, <16 x float>* %a16_out, align 64
  %416 = load float* %a_in, align 4
  %417 = call float @_Z4log2f(float %416) readnone
  store float %417, float* %a_out, align 4
  %418 = load <4 x float>* %a4_in, align 16
  %419 = call <4 x float> @_Z4log2Dv4_f(<4 x float> %418) readnone
  store <4 x float> %419, <4 x float>* %a4_out, align 16
  %420 = load <8 x float>* %a8_in, align 32
  %421 = call <8 x float> @_Z4log2Dv8_f(<8 x float> %420) readnone
  store <8 x float> %421, <8 x float>* %a8_out, align 32
  %422 = load <16 x float>* %a16_in, align 64
  %423 = call <16 x float> @_Z4log2Dv16_f(<16 x float> %422) readnone
  store <16 x float> %423, <16 x float>* %a16_out, align 64
  %424 = load float* %a_in, align 4
  %425 = call float @_Z5log10f(float %424) readnone
  store float %425, float* %a_out, align 4
  %426 = load <4 x float>* %a4_in, align 16
  %427 = call <4 x float> @_Z5log10Dv4_f(<4 x float> %426) readnone
  store <4 x float> %427, <4 x float>* %a4_out, align 16
  %428 = load <8 x float>* %a8_in, align 32
  %429 = call <8 x float> @_Z5log10Dv8_f(<8 x float> %428) readnone
  store <8 x float> %429, <8 x float>* %a8_out, align 32
  %430 = load <16 x float>* %a16_in, align 64
  %431 = call <16 x float> @_Z5log10Dv16_f(<16 x float> %430) readnone
  store <16 x float> %431, <16 x float>* %a16_out, align 64
  %432 = load float* %a_in, align 4
  %433 = call float @_Z5log1pf(float %432) readnone
  store float %433, float* %a_out, align 4
  %434 = load <4 x float>* %a4_in, align 16
  %435 = call <4 x float> @_Z5log1pDv4_f(<4 x float> %434) readnone
  store <4 x float> %435, <4 x float>* %a4_out, align 16
  %436 = load <8 x float>* %a8_in, align 32
  %437 = call <8 x float> @_Z5log1pDv8_f(<8 x float> %436) readnone
  store <8 x float> %437, <8 x float>* %a8_out, align 32
  %438 = load <16 x float>* %a16_in, align 64
  %439 = call <16 x float> @_Z5log1pDv16_f(<16 x float> %438) readnone
  store <16 x float> %439, <16 x float>* %a16_out, align 64
  %440 = load float* %a_in, align 4
  %441 = call float @_Z4logbf(float %440) readnone
  store float %441, float* %a_out, align 4
  %442 = load <4 x float>* %a4_in, align 16
  %443 = call <4 x float> @_Z4logbDv4_f(<4 x float> %442) readnone
  store <4 x float> %443, <4 x float>* %a4_out, align 16
  %444 = load <8 x float>* %a8_in, align 32
  %445 = call <8 x float> @_Z4logbDv8_f(<8 x float> %444) readnone
  store <8 x float> %445, <8 x float>* %a8_out, align 32
  %446 = load <16 x float>* %a16_in, align 64
  %447 = call <16 x float> @_Z4logbDv16_f(<16 x float> %446) readnone
  store <16 x float> %447, <16 x float>* %a16_out, align 64
  %448 = load float* %a_in, align 4
  %449 = call float @_Z4ceilf(float %448) readnone
  store float %449, float* %a_out, align 4
  %450 = load <4 x float>* %a4_in, align 16
  %451 = call <4 x float> @_Z4ceilDv4_f(<4 x float> %450) readnone
  store <4 x float> %451, <4 x float>* %a4_out, align 16
  %452 = load <8 x float>* %a8_in, align 32
  %453 = call <8 x float> @_Z4ceilDv8_f(<8 x float> %452) readnone
  store <8 x float> %453, <8 x float>* %a8_out, align 32
  %454 = load <16 x float>* %a16_in, align 64
  %455 = call <16 x float> @_Z4ceilDv16_f(<16 x float> %454) readnone
  store <16 x float> %455, <16 x float>* %a16_out, align 64
  %456 = load float* %a_in, align 4
  %457 = load float* %b_in, align 4
  %458 = call float @_Z3powff(float %456, float %457) readnone
  store float %458, float* %a_out, align 4
  %459 = load <4 x float>* %a4_in, align 16
  %460 = load <4 x float>* %b4_in, align 16
  %461 = call <4 x float> @_Z3powDv4_fS_(<4 x float> %459, <4 x float> %460) readnone
  store <4 x float> %461, <4 x float>* %a4_out, align 16
  %462 = load <8 x float>* %a8_in, align 32
  %463 = load <8 x float>* %b8_in, align 32
  %464 = call <8 x float> @_Z3powDv8_fS_(<8 x float> %462, <8 x float> %463) readnone
  store <8 x float> %464, <8 x float>* %a8_out, align 32
  %465 = load <16 x float>* %a16_in, align 64
  %466 = load <16 x float>* %b16_in, align 64
  %467 = call <16 x float> @_Z3powDv16_fS_(<16 x float> %465, <16 x float> %466) readnone
  store <16 x float> %467, <16 x float>* %a16_out, align 64
  %468 = load float* %a_in, align 4
  %469 = load float* %b_in, align 4
  %470 = load float* %c_in, align 4
  %471 = call float @_Z5clampfff(float %468, float %469, float %470) readnone
  store float %471, float* %a_out, align 4
  %472 = load <4 x float>* %a4_in, align 16
  %473 = load <4 x float>* %b4_in, align 16
  %474 = load <4 x float>* %c4_in, align 16
  %475 = call <4 x float> @_Z5clampDv4_fS_S_(<4 x float> %472, <4 x float> %473, <4 x float> %474) readnone
  store <4 x float> %475, <4 x float>* %a4_out, align 16
  %476 = load <8 x float>* %a8_in, align 32
  %477 = load <8 x float>* %b8_in, align 32
  %478 = load <8 x float>* %c8_in, align 32
  %479 = call <8 x float> @_Z5clampDv8_fS_S_(<8 x float> %476, <8 x float> %477, <8 x float> %478) readnone
  store <8 x float> %479, <8 x float>* %a8_out, align 32
  %480 = load <16 x float>* %a16_in, align 64
  %481 = load <16 x float>* %b16_in, align 64
  %482 = load <16 x float>* %c16_in, align 64
  %483 = call <16 x float> @_Z5clampDv16_fS_S_(<16 x float> %480, <16 x float> %481, <16 x float> %482) readnone
  store <16 x float> %483, <16 x float>* %a16_out, align 64
  %484 = load float* %a_in, align 4
  %485 = load float* %b_in, align 4
  %486 = load float* %c_in, align 4
  %487 = call float @_Z5clampfff(float %484, float %485, float %486) readnone
  store float %487, float* %a_out, align 4
  %488 = load <4 x float>* %a4_in, align 16
  %489 = load float* %b_in, align 4
  %490 = load float* %c_in, align 4
  %491 = call <4 x float> @_Z5clampDv4_fff(<4 x float> %488, float %489, float %490) readnone
  store <4 x float> %491, <4 x float>* %a4_out, align 16
  %492 = load <8 x float>* %a8_in, align 32
  %493 = load float* %b_in, align 4
  %494 = load float* %c_in, align 4
  %495 = call <8 x float> @_Z5clampDv8_fff(<8 x float> %492, float %493, float %494) readnone
  store <8 x float> %495, <8 x float>* %a8_out, align 32
  %496 = load <16 x float>* %a16_in, align 64
  %497 = load float* %b_in, align 4
  %498 = load float* %c_in, align 4
  %499 = call <16 x float> @_Z5clampDv16_fff(<16 x float> %496, float %497, float %498) readnone
  store <16 x float> %499, <16 x float>* %a16_out, align 64
  %500 = load float* %a_in, align 4
  %501 = call float @_Z4sinhf(float %500) readnone
  store float %501, float* %a_out, align 4
  %502 = load <4 x float>* %a4_in, align 16
  %503 = call <4 x float> @_Z4sinhDv4_f(<4 x float> %502) readnone
  store <4 x float> %503, <4 x float>* %a4_out, align 16
  %504 = load <8 x float>* %a8_in, align 32
  %505 = call <8 x float> @_Z4sinhDv8_f(<8 x float> %504) readnone
  store <8 x float> %505, <8 x float>* %a8_out, align 32
  %506 = load <16 x float>* %a16_in, align 64
  %507 = call <16 x float> @_Z4sinhDv16_f(<16 x float> %506) readnone
  store <16 x float> %507, <16 x float>* %a16_out, align 64
  %508 = load float* %a_in, align 4
  %509 = call float @_Z3sinf(float %508) readnone
  store float %509, float* %a_out, align 4
  %510 = load <4 x float>* %a4_in, align 16
  %511 = call <4 x float> @_Z3sinDv4_f(<4 x float> %510) readnone
  store <4 x float> %511, <4 x float>* %a4_out, align 16
  %512 = load <8 x float>* %a8_in, align 32
  %513 = call <8 x float> @_Z3sinDv8_f(<8 x float> %512) readnone
  store <8 x float> %513, <8 x float>* %a8_out, align 32
  %514 = load <16 x float>* %a16_in, align 64
  %515 = call <16 x float> @_Z3sinDv16_f(<16 x float> %514) readnone
  store <16 x float> %515, <16 x float>* %a16_out, align 64
  %516 = load float* %a_in, align 4
  %517 = call float @_Z5sinpif(float %516) readnone
  store float %517, float* %a_out, align 4
  %518 = load <4 x float>* %a4_in, align 16
  %519 = call <4 x float> @_Z5sinpiDv4_f(<4 x float> %518) readnone
  store <4 x float> %519, <4 x float>* %a4_out, align 16
  %520 = load <8 x float>* %a8_in, align 32
  %521 = call <8 x float> @_Z5sinpiDv8_f(<8 x float> %520) readnone
  store <8 x float> %521, <8 x float>* %a8_out, align 32
  %522 = load <16 x float>* %a16_in, align 64
  %523 = call <16 x float> @_Z5sinpiDv16_f(<16 x float> %522) readnone
  store <16 x float> %523, <16 x float>* %a16_out, align 64
  %524 = load float* %a_in, align 4
  %525 = call float @_Z4sqrtf(float %524) readnone
  store float %525, float* %a_out, align 4
  %526 = load <4 x float>* %a4_in, align 16
  %527 = call <4 x float> @_Z4sqrtDv4_f(<4 x float> %526) readnone
  store <4 x float> %527, <4 x float>* %a4_out, align 16
  %528 = load <8 x float>* %a8_in, align 32
  %529 = call <8 x float> @_Z4sqrtDv8_f(<8 x float> %528) readnone
  store <8 x float> %529, <8 x float>* %a8_out, align 32
  %530 = load <16 x float>* %a16_in, align 64
  %531 = call <16 x float> @_Z4sqrtDv16_f(<16 x float> %530) readnone
  store <16 x float> %531, <16 x float>* %a16_out, align 64
  %532 = load float* %a_in, align 4
  %533 = call float @_Z5rsqrtf(float %532) readnone
  store float %533, float* %a_out, align 4
  %534 = load <4 x float>* %a4_in, align 16
  %535 = call <4 x float> @_Z5rsqrtDv4_f(<4 x float> %534) readnone
  store <4 x float> %535, <4 x float>* %a4_out, align 16
  %536 = load <8 x float>* %a8_in, align 32
  %537 = call <8 x float> @_Z5rsqrtDv8_f(<8 x float> %536) readnone
  store <8 x float> %537, <8 x float>* %a8_out, align 32
  %538 = load <16 x float>* %a16_in, align 64
  %539 = call <16 x float> @_Z5rsqrtDv16_f(<16 x float> %538) readnone
  store <16 x float> %539, <16 x float>* %a16_out, align 64
  %540 = load float* %a_in, align 4
  %541 = call float @_Z3tanf(float %540) readnone
  store float %541, float* %a_out, align 4
  %542 = load <4 x float>* %a4_in, align 16
  %543 = call <4 x float> @_Z3tanDv4_f(<4 x float> %542) readnone
  store <4 x float> %543, <4 x float>* %a4_out, align 16
  %544 = load <8 x float>* %a8_in, align 32
  %545 = call <8 x float> @_Z3tanDv8_f(<8 x float> %544) readnone
  store <8 x float> %545, <8 x float>* %a8_out, align 32
  %546 = load <16 x float>* %a16_in, align 64
  %547 = call <16 x float> @_Z3tanDv16_f(<16 x float> %546) readnone
  store <16 x float> %547, <16 x float>* %a16_out, align 64
  %548 = load float* %a_in, align 4
  %549 = call float @_Z4tanhf(float %548) readnone
  store float %549, float* %a_out, align 4
  %550 = load <4 x float>* %a4_in, align 16
  %551 = call <4 x float> @_Z4tanhDv4_f(<4 x float> %550) readnone
  store <4 x float> %551, <4 x float>* %a4_out, align 16
  %552 = load <8 x float>* %a8_in, align 32
  %553 = call <8 x float> @_Z4tanhDv8_f(<8 x float> %552) readnone
  store <8 x float> %553, <8 x float>* %a8_out, align 32
  %554 = load <16 x float>* %a16_in, align 64
  %555 = call <16 x float> @_Z4tanhDv16_f(<16 x float> %554) readnone
  store <16 x float> %555, <16 x float>* %a16_out, align 64
  %556 = load float* %a_in, align 4
  %557 = call float @_Z5tanpif(float %556) readnone
  store float %557, float* %a_out, align 4
  %558 = load <4 x float>* %a4_in, align 16
  %559 = call <4 x float> @_Z5tanpiDv4_f(<4 x float> %558) readnone
  store <4 x float> %559, <4 x float>* %a4_out, align 16
  %560 = load <8 x float>* %a8_in, align 32
  %561 = call <8 x float> @_Z5tanpiDv8_f(<8 x float> %560) readnone
  store <8 x float> %561, <8 x float>* %a8_out, align 32
  %562 = load <16 x float>* %a16_in, align 64
  %563 = call <16 x float> @_Z5tanpiDv16_f(<16 x float> %562) readnone
  store <16 x float> %563, <16 x float>* %a16_out, align 64
  %564 = load float* %a_in, align 4
  %565 = call float @_Z4fabsf(float %564) readnone
  store float %565, float* %a_out, align 4
  %566 = load <4 x float>* %a4_in, align 16
  %567 = call <4 x float> @_Z4fabsDv4_f(<4 x float> %566) readnone
  store <4 x float> %567, <4 x float>* %a4_out, align 16
  %568 = load <8 x float>* %a8_in, align 32
  %569 = call <8 x float> @_Z4fabsDv8_f(<8 x float> %568) readnone
  store <8 x float> %569, <8 x float>* %a8_out, align 32
  %570 = load <16 x float>* %a16_in, align 64
  %571 = call <16 x float> @_Z4fabsDv16_f(<16 x float> %570) readnone
  store <16 x float> %571, <16 x float>* %a16_out, align 64
  %572 = load float* %a_in, align 4
  %573 = call float @_Z10native_sinf(float %572) readnone
  store float %573, float* %a_out, align 4
  %574 = load <4 x float>* %a4_in, align 16
  %575 = call <4 x float> @_Z10native_sinDv4_f(<4 x float> %574) readnone
  store <4 x float> %575, <4 x float>* %a4_out, align 16
  %576 = load <8 x float>* %a8_in, align 32
  %577 = call <8 x float> @_Z10native_sinDv8_f(<8 x float> %576) readnone
  store <8 x float> %577, <8 x float>* %a8_out, align 32
  %578 = load <16 x float>* %a16_in, align 64
  %579 = call <16 x float> @_Z10native_sinDv16_f(<16 x float> %578) readnone
  store <16 x float> %579, <16 x float>* %a16_out, align 64
  %580 = load float* %a_in, align 4
  %581 = call float @_Z10native_cosf(float %580) readnone
  store float %581, float* %a_out, align 4
  %582 = load <4 x float>* %a4_in, align 16
  %583 = call <4 x float> @_Z10native_cosDv4_f(<4 x float> %582) readnone
  store <4 x float> %583, <4 x float>* %a4_out, align 16
  %584 = load <8 x float>* %a8_in, align 32
  %585 = call <8 x float> @_Z10native_cosDv8_f(<8 x float> %584) readnone
  store <8 x float> %585, <8 x float>* %a8_out, align 32
  %586 = load <16 x float>* %a16_in, align 64
  %587 = call <16 x float> @_Z10native_cosDv16_f(<16 x float> %586) readnone
  store <16 x float> %587, <16 x float>* %a16_out, align 64
  %588 = load float* %a_in, align 4
  %589 = call float @_Z12native_rsqrtf(float %588) readnone
  store float %589, float* %a_out, align 4
  %590 = load <4 x float>* %a4_in, align 16
  %591 = call <4 x float> @_Z12native_rsqrtDv4_f(<4 x float> %590) readnone
  store <4 x float> %591, <4 x float>* %a4_out, align 16
  %592 = load <8 x float>* %a8_in, align 32
  %593 = call <8 x float> @_Z12native_rsqrtDv8_f(<8 x float> %592) readnone
  store <8 x float> %593, <8 x float>* %a8_out, align 32
  %594 = load <16 x float>* %a16_in, align 64
  %595 = call <16 x float> @_Z12native_rsqrtDv16_f(<16 x float> %594) readnone
  store <16 x float> %595, <16 x float>* %a16_out, align 64
  %596 = load float* %a_in, align 4
  %597 = call float @_Z10native_logf(float %596) readnone
  store float %597, float* %a_out, align 4
  %598 = load <4 x float>* %a4_in, align 16
  %599 = call <4 x float> @_Z10native_logDv4_f(<4 x float> %598) readnone
  store <4 x float> %599, <4 x float>* %a4_out, align 16
  %600 = load <8 x float>* %a8_in, align 32
  %601 = call <8 x float> @_Z10native_logDv8_f(<8 x float> %600) readnone
  store <8 x float> %601, <8 x float>* %a8_out, align 32
  %602 = load <16 x float>* %a16_in, align 64
  %603 = call <16 x float> @_Z10native_logDv16_f(<16 x float> %602) readnone
  store <16 x float> %603, <16 x float>* %a16_out, align 64
  %604 = load float* %a_in, align 4
  %605 = call float @_Z11native_log2f(float %604) readnone
  store float %605, float* %a_out, align 4
  %606 = load <4 x float>* %a4_in, align 16
  %607 = call <4 x float> @_Z11native_log2Dv4_f(<4 x float> %606) readnone
  store <4 x float> %607, <4 x float>* %a4_out, align 16
  %608 = load <8 x float>* %a8_in, align 32
  %609 = call <8 x float> @_Z11native_log2Dv8_f(<8 x float> %608) readnone
  store <8 x float> %609, <8 x float>* %a8_out, align 32
  %610 = load <16 x float>* %a16_in, align 64
  %611 = call <16 x float> @_Z11native_log2Dv16_f(<16 x float> %610) readnone
  store <16 x float> %611, <16 x float>* %a16_out, align 64
  %612 = load float* %a_in, align 4
  %613 = call float @_Z12native_log10f(float %612) readnone
  store float %613, float* %a_out, align 4
  %614 = load <4 x float>* %a4_in, align 16
  %615 = call <4 x float> @_Z12native_log10Dv4_f(<4 x float> %614) readnone
  store <4 x float> %615, <4 x float>* %a4_out, align 16
  %616 = load <8 x float>* %a8_in, align 32
  %617 = call <8 x float> @_Z12native_log10Dv8_f(<8 x float> %616) readnone
  store <8 x float> %617, <8 x float>* %a8_out, align 32
  %618 = load <16 x float>* %a16_in, align 64
  %619 = call <16 x float> @_Z12native_log10Dv16_f(<16 x float> %618) readnone
  store <16 x float> %619, <16 x float>* %a16_out, align 64
  %620 = load float* %a_in, align 4
  %621 = call float @_Z10native_expf(float %620) readnone
  store float %621, float* %a_out, align 4
  %622 = load <4 x float>* %a4_in, align 16
  %623 = call <4 x float> @_Z10native_expDv4_f(<4 x float> %622) readnone
  store <4 x float> %623, <4 x float>* %a4_out, align 16
  %624 = load <8 x float>* %a8_in, align 32
  %625 = call <8 x float> @_Z10native_expDv8_f(<8 x float> %624) readnone
  store <8 x float> %625, <8 x float>* %a8_out, align 32
  %626 = load <16 x float>* %a16_in, align 64
  %627 = call <16 x float> @_Z10native_expDv16_f(<16 x float> %626) readnone
  store <16 x float> %627, <16 x float>* %a16_out, align 64
  %628 = load float* %a_in, align 4
  %629 = call float @_Z11native_exp2f(float %628) readnone
  store float %629, float* %a_out, align 4
  %630 = load <4 x float>* %a4_in, align 16
  %631 = call <4 x float> @_Z11native_exp2Dv4_f(<4 x float> %630) readnone
  store <4 x float> %631, <4 x float>* %a4_out, align 16
  %632 = load <8 x float>* %a8_in, align 32
  %633 = call <8 x float> @_Z11native_exp2Dv8_f(<8 x float> %632) readnone
  store <8 x float> %633, <8 x float>* %a8_out, align 32
  %634 = load <16 x float>* %a16_in, align 64
  %635 = call <16 x float> @_Z11native_exp2Dv16_f(<16 x float> %634) readnone
  store <16 x float> %635, <16 x float>* %a16_out, align 64
  %636 = load float* %a_in, align 4
  %637 = call float @_Z12native_exp10f(float %636) readnone
  store float %637, float* %a_out, align 4
  %638 = load <4 x float>* %a4_in, align 16
  %639 = call <4 x float> @_Z12native_exp10Dv4_f(<4 x float> %638) readnone
  store <4 x float> %639, <4 x float>* %a4_out, align 16
  %640 = load <8 x float>* %a8_in, align 32
  %641 = call <8 x float> @_Z12native_exp10Dv8_f(<8 x float> %640) readnone
  store <8 x float> %641, <8 x float>* %a8_out, align 32
  %642 = load <16 x float>* %a16_in, align 64
  %643 = call <16 x float> @_Z12native_exp10Dv16_f(<16 x float> %642) readnone
  store <16 x float> %643, <16 x float>* %a16_out, align 64
  %644 = load float* %a_in, align 4
  %645 = load float* %b_in, align 4
  %646 = call float @_Z13native_divideff(float %644, float %645) readnone
  store float %646, float* %a_out, align 4
  %647 = load <4 x float>* %a4_in, align 16
  %648 = load <4 x float>* %b4_in, align 16
  %649 = call <4 x float> @_Z13native_divideDv4_fS_(<4 x float> %647, <4 x float> %648) readnone
  store <4 x float> %649, <4 x float>* %a4_out, align 16
  %650 = load <8 x float>* %a8_in, align 32
  %651 = load <8 x float>* %b8_in, align 32
  %652 = call <8 x float> @_Z13native_divideDv8_fS_(<8 x float> %650, <8 x float> %651) readnone
  store <8 x float> %652, <8 x float>* %a8_out, align 32
  %653 = load <16 x float>* %a16_in, align 64
  %654 = load <16 x float>* %b16_in, align 64
  %655 = call <16 x float> @_Z13native_divideDv16_fS_(<16 x float> %653, <16 x float> %654) readnone
  store <16 x float> %655, <16 x float>* %a16_out, align 64
  %656 = load float* %a_in, align 4
  %657 = load float* %b_in, align 4
  %658 = call float @_Z11native_powrff(float %656, float %657) readnone
  store float %658, float* %a_out, align 4
  %659 = load <4 x float>* %a4_in, align 16
  %660 = load <4 x float>* %b4_in, align 16
  %661 = call <4 x float> @_Z11native_powrDv4_fS_(<4 x float> %659, <4 x float> %660) readnone
  store <4 x float> %661, <4 x float>* %a4_out, align 16
  %662 = load <8 x float>* %a8_in, align 32
  %663 = load <8 x float>* %b8_in, align 32
  %664 = call <8 x float> @_Z11native_powrDv8_fS_(<8 x float> %662, <8 x float> %663) readnone
  store <8 x float> %664, <8 x float>* %a8_out, align 32
  %665 = load <16 x float>* %a16_in, align 64
  %666 = load <16 x float>* %b16_in, align 64
  %667 = call <16 x float> @_Z11native_powrDv16_fS_(<16 x float> %665, <16 x float> %666) readnone
  store <16 x float> %667, <16 x float>* %a16_out, align 64
  %668 = load float* %a_in, align 4
  %669 = call float @_Z12native_recipf(float %668) readnone
  store float %669, float* %a_out, align 4
  %670 = load <4 x float>* %a4_in, align 16
  %671 = call <4 x float> @_Z12native_recipDv4_f(<4 x float> %670) readnone
  store <4 x float> %671, <4 x float>* %a4_out, align 16
  %672 = load <8 x float>* %a8_in, align 32
  %673 = call <8 x float> @_Z12native_recipDv8_f(<8 x float> %672) readnone
  store <8 x float> %673, <8 x float>* %a8_out, align 32
  %674 = load <16 x float>* %a16_in, align 64
  %675 = call <16 x float> @_Z12native_recipDv16_f(<16 x float> %674) readnone
  store <16 x float> %675, <16 x float>* %a16_out, align 64
  %676 = load float* %a_in, align 4
  %677 = call float @_Z11native_sqrtf(float %676) readnone
  store float %677, float* %a_out, align 4
  %678 = load <4 x float>* %a4_in, align 16
  %679 = call <4 x float> @_Z11native_sqrtDv4_f(<4 x float> %678) readnone
  store <4 x float> %679, <4 x float>* %a4_out, align 16
  %680 = load <8 x float>* %a8_in, align 32
  %681 = call <8 x float> @_Z11native_sqrtDv8_f(<8 x float> %680) readnone
  store <8 x float> %681, <8 x float>* %a8_out, align 32
  %682 = load <16 x float>* %a16_in, align 64
  %683 = call <16 x float> @_Z11native_sqrtDv16_f(<16 x float> %682) readnone
  store <16 x float> %683, <16 x float>* %a16_out, align 64
  %684 = load float* %a_in, align 4
  %685 = call float @_Z10native_tanf(float %684) readnone
  store float %685, float* %a_out, align 4
  %686 = load <4 x float>* %a4_in, align 16
  %687 = call <4 x float> @_Z10native_tanDv4_f(<4 x float> %686) readnone
  store <4 x float> %687, <4 x float>* %a4_out, align 16
  %688 = load <8 x float>* %a8_in, align 32
  %689 = call <8 x float> @_Z10native_tanDv8_f(<8 x float> %688) readnone
  store <8 x float> %689, <8 x float>* %a8_out, align 32
  %690 = load <16 x float>* %a16_in, align 64
  %691 = call <16 x float> @_Z10native_tanDv16_f(<16 x float> %690) readnone
  store <16 x float> %691, <16 x float>* %a16_out, align 64
  %692 = load float* %a_in, align 4
  %693 = call float @_Z8half_logf(float %692) readnone
  store float %693, float* %a_out, align 4
  %694 = load <4 x float>* %a4_in, align 16
  %695 = call <4 x float> @_Z8half_logDv4_f(<4 x float> %694) readnone
  store <4 x float> %695, <4 x float>* %a4_out, align 16
  %696 = load <8 x float>* %a8_in, align 32
  %697 = call <8 x float> @_Z8half_logDv8_f(<8 x float> %696) readnone
  store <8 x float> %697, <8 x float>* %a8_out, align 32
  %698 = load <16 x float>* %a16_in, align 64
  %699 = call <16 x float> @_Z8half_logDv16_f(<16 x float> %698) readnone
  store <16 x float> %699, <16 x float>* %a16_out, align 64
  %700 = load float* %a_in, align 4
  %701 = call float @_Z9half_log2f(float %700) readnone
  store float %701, float* %a_out, align 4
  %702 = load <4 x float>* %a4_in, align 16
  %703 = call <4 x float> @_Z9half_log2Dv4_f(<4 x float> %702) readnone
  store <4 x float> %703, <4 x float>* %a4_out, align 16
  %704 = load <8 x float>* %a8_in, align 32
  %705 = call <8 x float> @_Z9half_log2Dv8_f(<8 x float> %704) readnone
  store <8 x float> %705, <8 x float>* %a8_out, align 32
  %706 = load <16 x float>* %a16_in, align 64
  %707 = call <16 x float> @_Z9half_log2Dv16_f(<16 x float> %706) readnone
  store <16 x float> %707, <16 x float>* %a16_out, align 64
  %708 = load float* %a_in, align 4
  %709 = call float @_Z10half_log10f(float %708) readnone
  store float %709, float* %a_out, align 4
  %710 = load <4 x float>* %a4_in, align 16
  %711 = call <4 x float> @_Z10half_log10Dv4_f(<4 x float> %710) readnone
  store <4 x float> %711, <4 x float>* %a4_out, align 16
  %712 = load <8 x float>* %a8_in, align 32
  %713 = call <8 x float> @_Z10half_log10Dv8_f(<8 x float> %712) readnone
  store <8 x float> %713, <8 x float>* %a8_out, align 32
  %714 = load <16 x float>* %a16_in, align 64
  %715 = call <16 x float> @_Z10half_log10Dv16_f(<16 x float> %714) readnone
  store <16 x float> %715, <16 x float>* %a16_out, align 64
  %716 = load float* %a_in, align 4
  %717 = call float @_Z8half_expf(float %716) readnone
  store float %717, float* %a_out, align 4
  %718 = load <4 x float>* %a4_in, align 16
  %719 = call <4 x float> @_Z8half_expDv4_f(<4 x float> %718) readnone
  store <4 x float> %719, <4 x float>* %a4_out, align 16
  %720 = load <8 x float>* %a8_in, align 32
  %721 = call <8 x float> @_Z8half_expDv8_f(<8 x float> %720) readnone
  store <8 x float> %721, <8 x float>* %a8_out, align 32
  %722 = load <16 x float>* %a16_in, align 64
  %723 = call <16 x float> @_Z8half_expDv16_f(<16 x float> %722) readnone
  store <16 x float> %723, <16 x float>* %a16_out, align 64
  %724 = load float* %a_in, align 4
  %725 = call float @_Z9half_exp2f(float %724) readnone
  store float %725, float* %a_out, align 4
  %726 = load <4 x float>* %a4_in, align 16
  %727 = call <4 x float> @_Z9half_exp2Dv4_f(<4 x float> %726) readnone
  store <4 x float> %727, <4 x float>* %a4_out, align 16
  %728 = load <8 x float>* %a8_in, align 32
  %729 = call <8 x float> @_Z9half_exp2Dv8_f(<8 x float> %728) readnone
  store <8 x float> %729, <8 x float>* %a8_out, align 32
  %730 = load <16 x float>* %a16_in, align 64
  %731 = call <16 x float> @_Z9half_exp2Dv16_f(<16 x float> %730) readnone
  store <16 x float> %731, <16 x float>* %a16_out, align 64
  %732 = load float* %a_in, align 4
  %733 = call float @_Z10half_exp10f(float %732) readnone
  store float %733, float* %a_out, align 4
  %734 = load <4 x float>* %a4_in, align 16
  %735 = call <4 x float> @_Z10half_exp10Dv4_f(<4 x float> %734) readnone
  store <4 x float> %735, <4 x float>* %a4_out, align 16
  %736 = load <8 x float>* %a8_in, align 32
  %737 = call <8 x float> @_Z10half_exp10Dv8_f(<8 x float> %736) readnone
  store <8 x float> %737, <8 x float>* %a8_out, align 32
  %738 = load <16 x float>* %a16_in, align 64
  %739 = call <16 x float> @_Z10half_exp10Dv16_f(<16 x float> %738) readnone
  store <16 x float> %739, <16 x float>* %a16_out, align 64
  %740 = load float* %a_in, align 4
  %741 = call float @_Z8half_cosf(float %740) readnone
  store float %741, float* %a_out, align 4
  %742 = load <4 x float>* %a4_in, align 16
  %743 = call <4 x float> @_Z8half_cosDv4_f(<4 x float> %742) readnone
  store <4 x float> %743, <4 x float>* %a4_out, align 16
  %744 = load <8 x float>* %a8_in, align 32
  %745 = call <8 x float> @_Z8half_cosDv8_f(<8 x float> %744) readnone
  store <8 x float> %745, <8 x float>* %a8_out, align 32
  %746 = load <16 x float>* %a16_in, align 64
  %747 = call <16 x float> @_Z8half_cosDv16_f(<16 x float> %746) readnone
  store <16 x float> %747, <16 x float>* %a16_out, align 64
  %748 = load float* %a_in, align 4
  %749 = load float* %b_in, align 4
  %750 = call float @_Z11half_divideff(float %748, float %749) readnone
  store float %750, float* %a_out, align 4
  %751 = load <4 x float>* %a4_in, align 16
  %752 = load <4 x float>* %b4_in, align 16
  %753 = call <4 x float> @_Z11half_divideDv4_fS_(<4 x float> %751, <4 x float> %752) readnone
  store <4 x float> %753, <4 x float>* %a4_out, align 16
  %754 = load <8 x float>* %a8_in, align 32
  %755 = load <8 x float>* %b8_in, align 32
  %756 = call <8 x float> @_Z11half_divideDv8_fS_(<8 x float> %754, <8 x float> %755) readnone
  store <8 x float> %756, <8 x float>* %a8_out, align 32
  %757 = load <16 x float>* %a16_in, align 64
  %758 = load <16 x float>* %b16_in, align 64
  %759 = call <16 x float> @_Z11half_divideDv16_fS_(<16 x float> %757, <16 x float> %758) readnone
  store <16 x float> %759, <16 x float>* %a16_out, align 64
  %760 = load float* %a_in, align 4
  %761 = load float* %b_in, align 4
  %762 = call float @_Z9half_powrff(float %760, float %761) readnone
  store float %762, float* %a_out, align 4
  %763 = load <4 x float>* %a4_in, align 16
  %764 = load <4 x float>* %b4_in, align 16
  %765 = call <4 x float> @_Z9half_powrDv4_fS_(<4 x float> %763, <4 x float> %764) readnone
  store <4 x float> %765, <4 x float>* %a4_out, align 16
  %766 = load <8 x float>* %a8_in, align 32
  %767 = load <8 x float>* %b8_in, align 32
  %768 = call <8 x float> @_Z9half_powrDv8_fS_(<8 x float> %766, <8 x float> %767) readnone
  store <8 x float> %768, <8 x float>* %a8_out, align 32
  %769 = load <16 x float>* %a16_in, align 64
  %770 = load <16 x float>* %b16_in, align 64
  %771 = call <16 x float> @_Z9half_powrDv16_fS_(<16 x float> %769, <16 x float> %770) readnone
  store <16 x float> %771, <16 x float>* %a16_out, align 64
  %772 = load float* %a_in, align 4
  %773 = call float @_Z10half_recipf(float %772) readnone
  store float %773, float* %a_out, align 4
  %774 = load <4 x float>* %a4_in, align 16
  %775 = call <4 x float> @_Z10half_recipDv4_f(<4 x float> %774) readnone
  store <4 x float> %775, <4 x float>* %a4_out, align 16
  %776 = load <8 x float>* %a8_in, align 32
  %777 = call <8 x float> @_Z10half_recipDv8_f(<8 x float> %776) readnone
  store <8 x float> %777, <8 x float>* %a8_out, align 32
  %778 = load <16 x float>* %a16_in, align 64
  %779 = call <16 x float> @_Z10half_recipDv16_f(<16 x float> %778) readnone
  store <16 x float> %779, <16 x float>* %a16_out, align 64
  %780 = load float* %a_in, align 4
  %781 = call float @_Z10half_rsqrtf(float %780) readnone
  store float %781, float* %a_out, align 4
  %782 = load <4 x float>* %a4_in, align 16
  %783 = call <4 x float> @_Z10half_rsqrtDv4_f(<4 x float> %782) readnone
  store <4 x float> %783, <4 x float>* %a4_out, align 16
  %784 = load <8 x float>* %a8_in, align 32
  %785 = call <8 x float> @_Z10half_rsqrtDv8_f(<8 x float> %784) readnone
  store <8 x float> %785, <8 x float>* %a8_out, align 32
  %786 = load <16 x float>* %a16_in, align 64
  %787 = call <16 x float> @_Z10half_rsqrtDv16_f(<16 x float> %786) readnone
  store <16 x float> %787, <16 x float>* %a16_out, align 64
  %788 = load float* %a_in, align 4
  %789 = call float @_Z8half_sinf(float %788) readnone
  store float %789, float* %a_out, align 4
  %790 = load <4 x float>* %a4_in, align 16
  %791 = call <4 x float> @_Z8half_sinDv4_f(<4 x float> %790) readnone
  store <4 x float> %791, <4 x float>* %a4_out, align 16
  %792 = load <8 x float>* %a8_in, align 32
  %793 = call <8 x float> @_Z8half_sinDv8_f(<8 x float> %792) readnone
  store <8 x float> %793, <8 x float>* %a8_out, align 32
  %794 = load <16 x float>* %a16_in, align 64
  %795 = call <16 x float> @_Z8half_sinDv16_f(<16 x float> %794) readnone
  store <16 x float> %795, <16 x float>* %a16_out, align 64
  %796 = load float* %a_in, align 4
  %797 = call float @_Z9half_sqrtf(float %796) readnone
  store float %797, float* %a_out, align 4
  %798 = load <4 x float>* %a4_in, align 16
  %799 = call <4 x float> @_Z9half_sqrtDv4_f(<4 x float> %798) readnone
  store <4 x float> %799, <4 x float>* %a4_out, align 16
  %800 = load <8 x float>* %a8_in, align 32
  %801 = call <8 x float> @_Z9half_sqrtDv8_f(<8 x float> %800) readnone
  store <8 x float> %801, <8 x float>* %a8_out, align 32
  %802 = load <16 x float>* %a16_in, align 64
  %803 = call <16 x float> @_Z9half_sqrtDv16_f(<16 x float> %802) readnone
  store <16 x float> %803, <16 x float>* %a16_out, align 64
  %804 = load float* %a_in, align 4
  %805 = call float @_Z8half_tanf(float %804) readnone
  store float %805, float* %a_out, align 4
  %806 = load <4 x float>* %a4_in, align 16
  %807 = call <4 x float> @_Z8half_tanDv4_f(<4 x float> %806) readnone
  store <4 x float> %807, <4 x float>* %a4_out, align 16
  %808 = load <8 x float>* %a8_in, align 32
  %809 = call <8 x float> @_Z8half_tanDv8_f(<8 x float> %808) readnone
  store <8 x float> %809, <8 x float>* %a8_out, align 32
  %810 = load <16 x float>* %a16_in, align 64
  %811 = call <16 x float> @_Z8half_tanDv16_f(<16 x float> %810) readnone
  store <16 x float> %811, <16 x float>* %a16_out, align 64
  %812 = load float* %a_in, align 4
  %813 = call float @_Z5asinhf(float %812) readnone
  store float %813, float* %a_out, align 4
  %814 = load <4 x float>* %a4_in, align 16
  %815 = call <4 x float> @_Z5asinhDv4_f(<4 x float> %814) readnone
  store <4 x float> %815, <4 x float>* %a4_out, align 16
  %816 = load <8 x float>* %a8_in, align 32
  %817 = call <8 x float> @_Z5asinhDv8_f(<8 x float> %816) readnone
  store <8 x float> %817, <8 x float>* %a8_out, align 32
  %818 = load <16 x float>* %a16_in, align 64
  %819 = call <16 x float> @_Z5asinhDv16_f(<16 x float> %818) readnone
  store <16 x float> %819, <16 x float>* %a16_out, align 64
  %820 = load float* %a_in, align 4
  %821 = call float @_Z5acoshf(float %820) readnone
  store float %821, float* %a_out, align 4
  %822 = load <4 x float>* %a4_in, align 16
  %823 = call <4 x float> @_Z5acoshDv4_f(<4 x float> %822) readnone
  store <4 x float> %823, <4 x float>* %a4_out, align 16
  %824 = load <8 x float>* %a8_in, align 32
  %825 = call <8 x float> @_Z5acoshDv8_f(<8 x float> %824) readnone
  store <8 x float> %825, <8 x float>* %a8_out, align 32
  %826 = load <16 x float>* %a16_in, align 64
  %827 = call <16 x float> @_Z5acoshDv16_f(<16 x float> %826) readnone
  store <16 x float> %827, <16 x float>* %a16_out, align 64
  %828 = load float* %a_in, align 4
  %829 = call float @_Z5atanhf(float %828) readnone
  store float %829, float* %a_out, align 4
  %830 = load <4 x float>* %a4_in, align 16
  %831 = call <4 x float> @_Z5atanhDv4_f(<4 x float> %830) readnone
  store <4 x float> %831, <4 x float>* %a4_out, align 16
  %832 = load <8 x float>* %a8_in, align 32
  %833 = call <8 x float> @_Z5atanhDv8_f(<8 x float> %832) readnone
  store <8 x float> %833, <8 x float>* %a8_out, align 32
  %834 = load <16 x float>* %a16_in, align 64
  %835 = call <16 x float> @_Z5atanhDv16_f(<16 x float> %834) readnone
  store <16 x float> %835, <16 x float>* %a16_out, align 64
  %836 = load float* %a_in, align 4
  %837 = load float* %b_in, align 4
  %838 = call float @_Z3minff(float %836, float %837) readnone
  store float %838, float* %a_out, align 4
  %839 = load <4 x float>* %a4_in, align 16
  %840 = load <4 x float>* %b4_in, align 16
  %841 = call <4 x float> @_Z3minDv4_fS_(<4 x float> %839, <4 x float> %840) readnone
  store <4 x float> %841, <4 x float>* %a4_out, align 16
  %842 = load <8 x float>* %a8_in, align 32
  %843 = load <8 x float>* %b8_in, align 32
  %844 = call <8 x float> @_Z3minDv8_fS_(<8 x float> %842, <8 x float> %843) readnone
  store <8 x float> %844, <8 x float>* %a8_out, align 32
  %845 = load <16 x float>* %a16_in, align 64
  %846 = load <16 x float>* %b16_in, align 64
  %847 = call <16 x float> @_Z3minDv16_fS_(<16 x float> %845, <16 x float> %846) readnone
  store <16 x float> %847, <16 x float>* %a16_out, align 64
  %848 = load <4 x float>* %a4_in, align 16
  %849 = load float* %b_in, align 4
  %850 = call <4 x float> @_Z3minDv4_ff(<4 x float> %848, float %849) readnone
  store <4 x float> %850, <4 x float>* %a4_out, align 16
  %851 = load <8 x float>* %a8_in, align 32
  %852 = load float* %b_in, align 4
  %853 = call <8 x float> @_Z3minDv8_ff(<8 x float> %851, float %852) readnone
  store <8 x float> %853, <8 x float>* %a8_out, align 32
  %854 = load <16 x float>* %a16_in, align 64
  %855 = load float* %b_in, align 4
  %856 = call <16 x float> @_Z3minDv16_ff(<16 x float> %854, float %855) readnone
  store <16 x float> %856, <16 x float>* %a16_out, align 64
  %857 = load float* %a_in, align 4
  %858 = load float* %b_in, align 4
  %859 = call float @_Z3maxff(float %857, float %858) readnone
  store float %859, float* %a_out, align 4
  %860 = load <4 x float>* %a4_in, align 16
  %861 = load <4 x float>* %b4_in, align 16
  %862 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %860, <4 x float> %861) readnone
  store <4 x float> %862, <4 x float>* %a4_out, align 16
  %863 = load <8 x float>* %a8_in, align 32
  %864 = load <8 x float>* %b8_in, align 32
  %865 = call <8 x float> @_Z3maxDv8_fS_(<8 x float> %863, <8 x float> %864) readnone
  store <8 x float> %865, <8 x float>* %a8_out, align 32
  %866 = load <16 x float>* %a16_in, align 64
  %867 = load <16 x float>* %b16_in, align 64
  %868 = call <16 x float> @_Z3maxDv16_fS_(<16 x float> %866, <16 x float> %867) readnone
  store <16 x float> %868, <16 x float>* %a16_out, align 64
  %869 = load <4 x float>* %a4_in, align 16
  %870 = load float* %b_in, align 4
  %871 = call <4 x float> @_Z3maxDv4_ff(<4 x float> %869, float %870) readnone
  store <4 x float> %871, <4 x float>* %a4_out, align 16
  %872 = load <8 x float>* %a8_in, align 32
  %873 = load float* %b_in, align 4
  %874 = call <8 x float> @_Z3maxDv8_ff(<8 x float> %872, float %873) readnone
  store <8 x float> %874, <8 x float>* %a8_out, align 32
  %875 = load <16 x float>* %a16_in, align 64
  %876 = load float* %b_in, align 4
  %877 = call <16 x float> @_Z3maxDv16_ff(<16 x float> %875, float %876) readnone
  store <16 x float> %877, <16 x float>* %a16_out, align 64
  %878 = load float* %a_in, align 4
  %879 = load float* %b_in, align 4
  %880 = call float @_Z5hypotff(float %878, float %879) readnone
  store float %880, float* %a_out, align 4
  %881 = load <4 x float>* %a4_in, align 16
  %882 = load <4 x float>* %b4_in, align 16
  %883 = call <4 x float> @_Z5hypotDv4_fS_(<4 x float> %881, <4 x float> %882) readnone
  store <4 x float> %883, <4 x float>* %a4_out, align 16
  %884 = load <8 x float>* %a8_in, align 32
  %885 = load <8 x float>* %b8_in, align 32
  %886 = call <8 x float> @_Z5hypotDv8_fS_(<8 x float> %884, <8 x float> %885) readnone
  store <8 x float> %886, <8 x float>* %a8_out, align 32
  %887 = load <16 x float>* %a16_in, align 64
  %888 = load <16 x float>* %b16_in, align 64
  %889 = call <16 x float> @_Z5hypotDv16_fS_(<16 x float> %887, <16 x float> %888) readnone
  store <16 x float> %889, <16 x float>* %a16_out, align 64
  %890 = load float* %a_in, align 4
  %891 = load float* %b_in, align 4
  %892 = call float @_Z4stepff(float %890, float %891) readnone
  store float %892, float* %a_out, align 4
  %893 = load <4 x float>* %a4_in, align 16
  %894 = load <4 x float>* %b4_in, align 16
  %895 = call <4 x float> @_Z4stepDv4_fS_(<4 x float> %893, <4 x float> %894) readnone
  store <4 x float> %895, <4 x float>* %a4_out, align 16
  %896 = load <8 x float>* %a8_in, align 32
  %897 = load <8 x float>* %b8_in, align 32
  %898 = call <8 x float> @_Z4stepDv8_fS_(<8 x float> %896, <8 x float> %897) readnone
  store <8 x float> %898, <8 x float>* %a8_out, align 32
  %899 = load <16 x float>* %a16_in, align 64
  %900 = load <16 x float>* %b16_in, align 64
  %901 = call <16 x float> @_Z4stepDv16_fS_(<16 x float> %899, <16 x float> %900) readnone
  store <16 x float> %901, <16 x float>* %a16_out, align 64
  %902 = load float* %a_in, align 4
  %903 = load float* %b_in, align 4
  %904 = call float @_Z4stepff(float %902, float %903) readnone
  store float %904, float* %a_out, align 4
  %905 = load float* %a_in, align 4
  %906 = load <4 x float>* %b4_in, align 16
  %907 = call <4 x float> @_Z4stepfDv4_f(float %905, <4 x float> %906) readnone
  store <4 x float> %907, <4 x float>* %a4_out, align 16
  %908 = load float* %a_in, align 4
  %909 = load <8 x float>* %b8_in, align 32
  %910 = call <8 x float> @_Z4stepfDv8_f(float %908, <8 x float> %909) readnone
  store <8 x float> %910, <8 x float>* %a8_out, align 32
  %911 = load float* %a_in, align 4
  %912 = load <16 x float>* %b16_in, align 64
  %913 = call <16 x float> @_Z4stepfDv16_f(float %911, <16 x float> %912) readnone
  store <16 x float> %913, <16 x float>* %a16_out, align 64
  %914 = load float* %a_in, align 4
  %915 = load float* %b_in, align 4
  %916 = load float* %c_in, align 4
  %917 = call float @_Z10smoothstepfff(float %914, float %915, float %916) readnone
  store float %917, float* %a_out, align 4
  %918 = load <4 x float>* %a4_in, align 16
  %919 = load <4 x float>* %b4_in, align 16
  %920 = load <4 x float>* %c4_in, align 16
  %921 = call <4 x float> @_Z10smoothstepDv4_fS_S_(<4 x float> %918, <4 x float> %919, <4 x float> %920) readnone
  store <4 x float> %921, <4 x float>* %a4_out, align 16
  %922 = load <8 x float>* %a8_in, align 32
  %923 = load <8 x float>* %b8_in, align 32
  %924 = load <8 x float>* %c8_in, align 32
  %925 = call <8 x float> @_Z10smoothstepDv8_fS_S_(<8 x float> %922, <8 x float> %923, <8 x float> %924) readnone
  store <8 x float> %925, <8 x float>* %a8_out, align 32
  %926 = load <16 x float>* %a16_in, align 64
  %927 = load <16 x float>* %b16_in, align 64
  %928 = load <16 x float>* %c16_in, align 64
  %929 = call <16 x float> @_Z10smoothstepDv16_fS_S_(<16 x float> %926, <16 x float> %927, <16 x float> %928) readnone
  store <16 x float> %929, <16 x float>* %a16_out, align 64
  %930 = load float* %a_in, align 4
  %931 = load float* %b_in, align 4
  %932 = load float* %c_in, align 4
  %933 = call float @_Z10smoothstepfff(float %930, float %931, float %932) readnone
  store float %933, float* %a_out, align 4
  %934 = load float* %a_in, align 4
  %935 = load float* %b_in, align 4
  %936 = load <4 x float>* %c4_in, align 16
  %937 = call <4 x float> @_Z10smoothstepffDv4_f(float %934, float %935, <4 x float> %936) readnone
  store <4 x float> %937, <4 x float>* %a4_out, align 16
  %938 = load float* %a_in, align 4
  %939 = load float* %b_in, align 4
  %940 = load <8 x float>* %c8_in, align 32
  %941 = call <8 x float> @_Z10smoothstepffDv8_f(float %938, float %939, <8 x float> %940) readnone
  store <8 x float> %941, <8 x float>* %a8_out, align 32
  %942 = load float* %a_in, align 4
  %943 = load float* %b_in, align 4
  %944 = load <16 x float>* %c16_in, align 64
  %945 = call <16 x float> @_Z10smoothstepffDv16_f(float %942, float %943, <16 x float> %944) readnone
  store <16 x float> %945, <16 x float>* %a16_out, align 64
  %946 = load float* %a_in, align 4
  %947 = call float @_Z7radiansf(float %946) readnone
  store float %947, float* %a_out, align 4
  %948 = load <4 x float>* %a4_in, align 16
  %949 = call <4 x float> @_Z7radiansDv4_f(<4 x float> %948) readnone
  store <4 x float> %949, <4 x float>* %a4_out, align 16
  %950 = load <8 x float>* %a8_in, align 32
  %951 = call <8 x float> @_Z7radiansDv8_f(<8 x float> %950) readnone
  store <8 x float> %951, <8 x float>* %a8_out, align 32
  %952 = load <16 x float>* %a16_in, align 64
  %953 = call <16 x float> @_Z7radiansDv16_f(<16 x float> %952) readnone
  store <16 x float> %953, <16 x float>* %a16_out, align 64
  %954 = load float* %a_in, align 4
  %955 = call float @_Z7degreesf(float %954) readnone
  store float %955, float* %a_out, align 4
  %956 = load <4 x float>* %a4_in, align 16
  %957 = call <4 x float> @_Z7degreesDv4_f(<4 x float> %956) readnone
  store <4 x float> %957, <4 x float>* %a4_out, align 16
  %958 = load <8 x float>* %a8_in, align 32
  %959 = call <8 x float> @_Z7degreesDv8_f(<8 x float> %958) readnone
  store <8 x float> %959, <8 x float>* %a8_out, align 32
  %960 = load <16 x float>* %a16_in, align 64
  %961 = call <16 x float> @_Z7degreesDv16_f(<16 x float> %960) readnone
  store <16 x float> %961, <16 x float>* %a16_out, align 64
  %962 = load float* %a_in, align 4
  %963 = call float @_Z4signf(float %962) readnone
  store float %963, float* %a_out, align 4
  %964 = load <4 x float>* %a4_in, align 16
  %965 = call <4 x float> @_Z4signDv4_f(<4 x float> %964) readnone
  store <4 x float> %965, <4 x float>* %a4_out, align 16
  %966 = load <8 x float>* %a8_in, align 32
  %967 = call <8 x float> @_Z4signDv8_f(<8 x float> %966) readnone
  store <8 x float> %967, <8 x float>* %a8_out, align 32
  %968 = load <16 x float>* %a16_in, align 64
  %969 = call <16 x float> @_Z4signDv16_f(<16 x float> %968) readnone
  store <16 x float> %969, <16 x float>* %a16_out, align 64
  %970 = load float* %a_in, align 4
  %971 = call float @_Z5floorf(float %970) readnone
  store float %971, float* %a_out, align 4
  %972 = load <4 x float>* %a4_in, align 16
  %973 = call <4 x float> @_Z5floorDv4_f(<4 x float> %972) readnone
  store <4 x float> %973, <4 x float>* %a4_out, align 16
  %974 = load <8 x float>* %a8_in, align 32
  %975 = call <8 x float> @_Z5floorDv8_f(<8 x float> %974) readnone
  store <8 x float> %975, <8 x float>* %a8_out, align 32
  %976 = load <16 x float>* %a16_in, align 64
  %977 = call <16 x float> @_Z5floorDv16_f(<16 x float> %976) readnone
  store <16 x float> %977, <16 x float>* %a16_out, align 64
  %978 = load float* %a_in, align 4
  %979 = load float* %b_in, align 4
  %980 = call float @_Z3dotff(float %978, float %979) readnone
  store float %980, float* %a_out, align 4
  %981 = load <4 x float>* %a4_in, align 16
  %982 = load <4 x float>* %b4_in, align 16
  %983 = call float @_Z3dotDv4_fS_(<4 x float> %981, <4 x float> %982) readnone
  store float %983, float* %a_out, align 4
  %984 = load float* %a_in, align 4
  %985 = load float* %b_in, align 4
  %986 = load float* %c_in, align 4
  %987 = call float @_Z3mixfff(float %984, float %985, float %986) readnone
  store float %987, float* %a_out, align 4
  %988 = load <4 x float>* %a4_in, align 16
  %989 = load <4 x float>* %b4_in, align 16
  %990 = load <4 x float>* %c4_in, align 16
  %991 = call <4 x float> @_Z3mixDv4_fS_S_(<4 x float> %988, <4 x float> %989, <4 x float> %990) readnone
  store <4 x float> %991, <4 x float>* %a4_out, align 16
  %992 = load <4 x float>* %a4_in, align 16
  %993 = load <4 x float>* %b4_in, align 16
  %994 = load float* %c_in, align 4
  %995 = call <4 x float> @_Z3mixDv4_fS_f(<4 x float> %992, <4 x float> %993, float %994) readnone
  store <4 x float> %995, <4 x float>* %a4_out, align 16
  %996 = load <8 x float>* %a8_in, align 32
  %997 = load <8 x float>* %b8_in, align 32
  %998 = load float* %c_in, align 4
  %999 = call <8 x float> @_Z3mixDv8_fS_f(<8 x float> %996, <8 x float> %997, float %998) readnone
  store <8 x float> %999, <8 x float>* %a8_out, align 32
  %1000 = load <16 x float>* %a16_in, align 64
  %1001 = load <16 x float>* %b16_in, align 64
  %1002 = load float* %c_in, align 4
  %1003 = call <16 x float> @_Z3mixDv16_fS_f(<16 x float> %1000, <16 x float> %1001, float %1002) readnone
  store <16 x float> %1003, <16 x float>* %a16_out, align 64
  %1004 = load <8 x float>* %a8_in, align 32
  %1005 = load <8 x float>* %b8_in, align 32
  %1006 = load <8 x float>* %c8_in, align 32
  %1007 = call <8 x float> @_Z3mixDv8_fS_S_(<8 x float> %1004, <8 x float> %1005, <8 x float> %1006) readnone
  store <8 x float> %1007, <8 x float>* %a8_out, align 32
  %1008 = load <16 x float>* %a16_in, align 64
  %1009 = load <16 x float>* %b16_in, align 64
  %1010 = load <16 x float>* %c16_in, align 64
  %1011 = call <16 x float> @_Z3mixDv16_fS_S_(<16 x float> %1008, <16 x float> %1009, <16 x float> %1010) readnone
  store <16 x float> %1011, <16 x float>* %a16_out, align 64
  %1012 = load float* %a_in, align 4
  %1013 = call float @_Z9normalizef(float %1012) readnone
  store float %1013, float* %a_out, align 4
  %1014 = load <4 x float>* %a4_in, align 16
  %1015 = call <4 x float> @_Z9normalizeDv4_f(<4 x float> %1014) readnone
  store <4 x float> %1015, <4 x float>* %a4_out, align 16
  %1016 = load float* %a_in, align 4
  %1017 = call float @_Z14fast_normalizef(float %1016) readnone
  store float %1017, float* %a_out, align 4
  %1018 = load <4 x float>* %a4_in, align 16
  %1019 = call <4 x float> @_Z14fast_normalizeDv4_f(<4 x float> %1018) readnone
  store <4 x float> %1019, <4 x float>* %a4_out, align 16
  %1020 = load <4 x float>* %a4_in, align 16
  %1021 = load <4 x float>* %b4_in, align 16
  %1022 = call <4 x float> @_Z5crossDv4_fS_(<4 x float> %1020, <4 x float> %1021) readnone
  store <4 x float> %1022, <4 x float>* %a4_out, align 16
  %1023 = load float* %a_in, align 4
  %1024 = call float @_Z6lengthf(float %1023) readnone
  store float %1024, float* %a_out, align 4
  %1025 = load <2 x float>* %a2_in, align 8
  %1026 = call float @_Z6lengthDv2_f(<2 x float> %1025) readnone
  store float %1026, float* %a_out, align 4
  %1027 = load <4 x float>* %a4_in, align 16
  %1028 = call float @_Z6lengthDv4_f(<4 x float> %1027) readnone
  store float %1028, float* %a_out, align 4
  %1029 = load float* %a_in, align 4
  %1030 = call float @_Z11fast_lengthf(float %1029) readnone
  store float %1030, float* %a_out, align 4
  %1031 = load <2 x float>* %a2_in, align 8
  %1032 = call float @_Z11fast_lengthDv2_f(<2 x float> %1031) readnone
  store float %1032, float* %a_out, align 4
  %1033 = load <4 x float>* %a4_in, align 16
  %1034 = call float @_Z11fast_lengthDv4_f(<4 x float> %1033) readnone
  store float %1034, float* %a_out, align 4
  %1035 = load float* %a_in, align 4
  %1036 = load float* %b_in, align 4
  %1037 = call float @_Z8distanceff(float %1035, float %1036) readnone
  store float %1037, float* %a_out, align 4
  %1038 = load <2 x float>* %a2_in, align 8
  %1039 = load <2 x float>* %b2_in, align 8
  %1040 = call float @_Z8distanceDv2_fS_(<2 x float> %1038, <2 x float> %1039) readnone
  store float %1040, float* %a_out, align 4
  %1041 = load <4 x float>* %a4_in, align 16
  %1042 = load <4 x float>* %b4_in, align 16
  %1043 = call float @_Z8distanceDv4_fS_(<4 x float> %1041, <4 x float> %1042) readnone
  store float %1043, float* %a_out, align 4
  %1044 = load float* %a_in, align 4
  %1045 = load float* %b_in, align 4
  %1046 = call float @_Z13fast_distanceff(float %1044, float %1045) readnone
  store float %1046, float* %a_out, align 4
  %1047 = load <2 x float>* %a2_in, align 8
  %1048 = load <2 x float>* %b2_in, align 8
  %1049 = call float @_Z13fast_distanceDv2_fS_(<2 x float> %1047, <2 x float> %1048) readnone
  store float %1049, float* %a_out, align 4
  %1050 = load <4 x float>* %a4_in, align 16
  %1051 = load <4 x float>* %b4_in, align 16
  %1052 = call float @_Z13fast_distanceDv4_fS_(<4 x float> %1050, <4 x float> %1051) readnone
  store float %1052, float* %a_out, align 4
  %1053 = load i32* %tid, align 4
  %1054 = call float @_Z13convert_floati(i32 %1053) readnone
  store float %1054, float* %a_out, align 4
  %1055 = load i32* %tid, align 4
  %1056 = insertelement <4 x i32> undef, i32 %1055, i32 0
  %1057 = shufflevector <4 x i32> %1056, <4 x i32> %1056, <4 x i32> zeroinitializer
  %1058 = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %1057) readnone
  store <4 x float> %1058, <4 x float>* %a4_out, align 16
  %1059 = load i32* %tid, align 4
  %1060 = insertelement <8 x i32> undef, i32 %1059, i32 0
  %1061 = shufflevector <8 x i32> %1060, <8 x i32> %1060, <8 x i32> zeroinitializer
  %1062 = call <8 x float> @_Z14convert_float8Dv8_i(<8 x i32> %1061) readnone
  store <8 x float> %1062, <8 x float>* %a8_out, align 32
  %1063 = load i32* %tid, align 4
  %1064 = insertelement <16 x i32> undef, i32 %1063, i32 0
  %1065 = shufflevector <16 x i32> %1064, <16 x i32> %1064, <16 x i32> zeroinitializer
  %1066 = call <16 x float> @_Z15convert_float16Dv16_i(<16 x i32> %1065) readnone
  store <16 x float> %1066, <16 x float>* %a16_out, align 64
  %1067 = load i32* %tid, align 4
  %1068 = call float @_Z13convert_floatj(i32 %1067) readnone
  store float %1068, float* %a_out, align 4
  %1069 = load i32* %tid, align 4
  %1070 = insertelement <4 x i32> undef, i32 %1069, i32 0
  %1071 = shufflevector <4 x i32> %1070, <4 x i32> %1070, <4 x i32> zeroinitializer
  %1072 = call <4 x float> @_Z14convert_float4Dv4_j(<4 x i32> %1071) readnone
  store <4 x float> %1072, <4 x float>* %a4_out, align 16
  %1073 = load i32* %tid, align 4
  %1074 = insertelement <8 x i32> undef, i32 %1073, i32 0
  %1075 = shufflevector <8 x i32> %1074, <8 x i32> %1074, <8 x i32> zeroinitializer
  %1076 = call <8 x float> @_Z14convert_float8Dv8_j(<8 x i32> %1075) readnone
  store <8 x float> %1076, <8 x float>* %a8_out, align 32
  %1077 = load i32* %tid, align 4
  %1078 = insertelement <16 x i32> undef, i32 %1077, i32 0
  %1079 = shufflevector <16 x i32> %1078, <16 x i32> %1078, <16 x i32> zeroinitializer
  %1080 = call <16 x float> @_Z15convert_float16Dv16_j(<16 x i32> %1079) readnone
  store <16 x float> %1080, <16 x float>* %a16_out, align 64
  %1081 = load float* %a_in, align 4
  %1082 = load i32* %i_in, align 4
  %1083 = call float @_Z5rootnfi(float %1081, i32 %1082) readnone
  store float %1083, float* %a_out, align 4
  %1084 = load <4 x float>* %a4_in, align 16
  %1085 = load <4 x i32>* %i4_in, align 16
  %1086 = call <4 x float> @_Z5rootnDv4_fDv4_i(<4 x float> %1084, <4 x i32> %1085) readnone
  store <4 x float> %1086, <4 x float>* %a4_out, align 16
  %1087 = load <8 x float>* %a8_in, align 32
  %1088 = load <8 x i32>* %i8_in, align 32
  %1089 = call <8 x float> @_Z5rootnDv8_fDv8_i(<8 x float> %1087, <8 x i32> %1088) readnone
  store <8 x float> %1089, <8 x float>* %a8_out, align 32
  %1090 = load <16 x float>* %a16_in, align 64
  %1091 = load <16 x i32>* %i16_in, align 64
  %1092 = call <16 x float> @_Z5rootnDv16_fDv16_i(<16 x float> %1090, <16 x i32> %1091) readnone
  store <16 x float> %1092, <16 x float>* %a16_out, align 64
  %1093 = load float* %a_in, align 4
  %1094 = load i32* %i_in, align 4
  %1095 = call float @_Z5ldexpfi(float %1093, i32 %1094) readnone
  store float %1095, float* %a_out, align 4
  %1096 = load <4 x float>* %a4_in, align 16
  %1097 = load <4 x i32>* %i4_in, align 16
  %1098 = call <4 x float> @_Z5ldexpDv4_fDv4_i(<4 x float> %1096, <4 x i32> %1097) readnone
  store <4 x float> %1098, <4 x float>* %a4_out, align 16
  %1099 = load <8 x float>* %a8_in, align 32
  %1100 = load <8 x i32>* %i8_in, align 32
  %1101 = call <8 x float> @_Z5ldexpDv8_fDv8_i(<8 x float> %1099, <8 x i32> %1100) readnone
  store <8 x float> %1101, <8 x float>* %a8_out, align 32
  %1102 = load <16 x float>* %a16_in, align 64
  %1103 = load <16 x i32>* %i16_in, align 64
  %1104 = call <16 x float> @_Z5ldexpDv16_fDv16_i(<16 x float> %1102, <16 x i32> %1103) readnone
  store <16 x float> %1104, <16 x float>* %a16_out, align 64
  %1105 = load <4 x float>* %a4_in, align 16
  %1106 = load i32* %i_in, align 4
  %1107 = call <4 x float> @_Z5ldexpDv4_fi(<4 x float> %1105, i32 %1106) readnone
  store <4 x float> %1107, <4 x float>* %a4_out, align 16
  %1108 = load <8 x float>* %a8_in, align 32
  %1109 = load i32* %i_in, align 4
  %1110 = call <8 x float> @_Z5ldexpDv8_fi(<8 x float> %1108, i32 %1109) readnone
  store <8 x float> %1110, <8 x float>* %a8_out, align 32
  %1111 = load <16 x float>* %a16_in, align 64
  %1112 = load i32* %i_in, align 4
  %1113 = call <16 x float> @_Z5ldexpDv16_fi(<16 x float> %1111, i32 %1112) readnone
  store <16 x float> %1113, <16 x float>* %a16_out, align 64
  %1114 = load float* %a_in, align 4
  %1115 = call float @_Z4modffPf(float %1114, float* %b_out)
  store float %1115, float* %a_out, align 4
  %1116 = load <4 x float>* %a4_in, align 16
  %1117 = call <4 x float> @_Z4modfDv4_fPS_(<4 x float> %1116, <4 x float>* %b4_out)
  store <4 x float> %1117, <4 x float>* %a4_out, align 16
  %1118 = load <8 x float>* %a8_in, align 32
  %1119 = call <8 x float> @_Z4modfDv8_fPS_(<8 x float> %1118, <8 x float>* %b8_out)
  store <8 x float> %1119, <8 x float>* %a8_out, align 32
  %1120 = load <16 x float>* %a16_in, align 64
  %1121 = call <16 x float> @_Z4modfDv16_fPS_(<16 x float> %1120, <16 x float>* %b16_out)
  store <16 x float> %1121, <16 x float>* %a16_out, align 64
  %1122 = load float* %a_in, align 4
  %1123 = call float @_Z5frexpfPi(float %1122, i32* %i_out)
  store float %1123, float* %a_out, align 4
  %1124 = load <4 x float>* %a4_in, align 16
  %1125 = call <4 x float> @_Z5frexpDv4_fPDv4_i(<4 x float> %1124, <4 x i32>* %i4_out)
  store <4 x float> %1125, <4 x float>* %a4_out, align 16
  %1126 = load <8 x float>* %a8_in, align 32
  %1127 = call <8 x float> @_Z5frexpDv8_fPDv8_i(<8 x float> %1126, <8 x i32>* %i8_out)
  store <8 x float> %1127, <8 x float>* %a8_out, align 32
  %1128 = load <16 x float>* %a16_in, align 64
  %1129 = call <16 x float> @_Z5frexpDv16_fPDv16_i(<16 x float> %1128, <16 x i32>* %i16_out)
  store <16 x float> %1129, <16 x float>* %a16_out, align 64
  %1130 = load float* %a_in, align 4
  %1131 = load float* %b_in, align 4
  %1132 = call float @_Z6maxmagff(float %1130, float %1131) readnone
  store float %1132, float* %a_out, align 4
  %1133 = load <4 x float>* %a4_in, align 16
  %1134 = load <4 x float>* %b4_in, align 16
  %1135 = call <4 x float> @_Z6maxmagDv4_fS_(<4 x float> %1133, <4 x float> %1134) readnone
  store <4 x float> %1135, <4 x float>* %a4_out, align 16
  %1136 = load <8 x float>* %a8_in, align 32
  %1137 = load <8 x float>* %b8_in, align 32
  %1138 = call <8 x float> @_Z6maxmagDv8_fS_(<8 x float> %1136, <8 x float> %1137) readnone
  store <8 x float> %1138, <8 x float>* %a8_out, align 32
  %1139 = load <16 x float>* %a16_in, align 64
  %1140 = load <16 x float>* %b16_in, align 64
  %1141 = call <16 x float> @_Z6maxmagDv16_fS_(<16 x float> %1139, <16 x float> %1140) readnone
  store <16 x float> %1141, <16 x float>* %a16_out, align 64
  %1142 = load float* %a_in, align 4
  %1143 = load float* %b_in, align 4
  %1144 = call float @_Z6minmagff(float %1142, float %1143) readnone
  store float %1144, float* %a_out, align 4
  %1145 = load <4 x float>* %a4_in, align 16
  %1146 = load <4 x float>* %b4_in, align 16
  %1147 = call <4 x float> @_Z6minmagDv4_fS_(<4 x float> %1145, <4 x float> %1146) readnone
  store <4 x float> %1147, <4 x float>* %a4_out, align 16
  %1148 = load <8 x float>* %a8_in, align 32
  %1149 = load <8 x float>* %b8_in, align 32
  %1150 = call <8 x float> @_Z6minmagDv8_fS_(<8 x float> %1148, <8 x float> %1149) readnone
  store <8 x float> %1150, <8 x float>* %a8_out, align 32
  %1151 = load <16 x float>* %a16_in, align 64
  %1152 = load <16 x float>* %b16_in, align 64
  %1153 = call <16 x float> @_Z6minmagDv16_fS_(<16 x float> %1151, <16 x float> %1152) readnone
  store <16 x float> %1153, <16 x float>* %a16_out, align 64
  %1154 = load float* %a_in, align 4
  %1155 = load float* %b_in, align 4
  %1156 = call float @_Z8copysignff(float %1154, float %1155) readnone
  store float %1156, float* %a_out, align 4
  %1157 = load <4 x float>* %a4_in, align 16
  %1158 = load <4 x float>* %b4_in, align 16
  %1159 = call <4 x float> @_Z8copysignDv4_fS_(<4 x float> %1157, <4 x float> %1158) readnone
  store <4 x float> %1159, <4 x float>* %a4_out, align 16
  %1160 = load <8 x float>* %a8_in, align 32
  %1161 = load <8 x float>* %b8_in, align 32
  %1162 = call <8 x float> @_Z8copysignDv8_fS_(<8 x float> %1160, <8 x float> %1161) readnone
  store <8 x float> %1162, <8 x float>* %a8_out, align 32
  %1163 = load <16 x float>* %a16_in, align 64
  %1164 = load <16 x float>* %b16_in, align 64
  %1165 = call <16 x float> @_Z8copysignDv16_fS_(<16 x float> %1163, <16 x float> %1164) readnone
  store <16 x float> %1165, <16 x float>* %a16_out, align 64
  %1166 = load float* %a_in, align 4
  %1167 = load float* %b_in, align 4
  %1168 = call float @_Z9nextafterff(float %1166, float %1167) readnone
  store float %1168, float* %a_out, align 4
  %1169 = load <4 x float>* %a4_in, align 16
  %1170 = load <4 x float>* %b4_in, align 16
  %1171 = call <4 x float> @_Z9nextafterDv4_fS_(<4 x float> %1169, <4 x float> %1170) readnone
  store <4 x float> %1171, <4 x float>* %a4_out, align 16
  %1172 = load <8 x float>* %a8_in, align 32
  %1173 = load <8 x float>* %b8_in, align 32
  %1174 = call <8 x float> @_Z9nextafterDv8_fS_(<8 x float> %1172, <8 x float> %1173) readnone
  store <8 x float> %1174, <8 x float>* %a8_out, align 32
  %1175 = load <16 x float>* %a16_in, align 64
  %1176 = load <16 x float>* %b16_in, align 64
  %1177 = call <16 x float> @_Z9nextafterDv16_fS_(<16 x float> %1175, <16 x float> %1176) readnone
  store <16 x float> %1177, <16 x float>* %a16_out, align 64
  %1178 = load float* %a_in, align 4
  %1179 = load float* %b_in, align 4
  %1180 = call float @_Z4fdimff(float %1178, float %1179) readnone
  store float %1180, float* %a_out, align 4
  %1181 = load <4 x float>* %a4_in, align 16
  %1182 = load <4 x float>* %b4_in, align 16
  %1183 = call <4 x float> @_Z4fdimDv4_fS_(<4 x float> %1181, <4 x float> %1182) readnone
  store <4 x float> %1183, <4 x float>* %a4_out, align 16
  %1184 = load <8 x float>* %a8_in, align 32
  %1185 = load <8 x float>* %b8_in, align 32
  %1186 = call <8 x float> @_Z4fdimDv8_fS_(<8 x float> %1184, <8 x float> %1185) readnone
  store <8 x float> %1186, <8 x float>* %a8_out, align 32
  %1187 = load <16 x float>* %a16_in, align 64
  %1188 = load <16 x float>* %b16_in, align 64
  %1189 = call <16 x float> @_Z4fdimDv16_fS_(<16 x float> %1187, <16 x float> %1188) readnone
  store <16 x float> %1189, <16 x float>* %a16_out, align 64
  %1190 = load float* %a_in, align 4
  %1191 = load float* %b_in, align 4
  %1192 = load float* %c_in, align 4
  %1193 = call float @_Z3fmafff(float %1190, float %1191, float %1192) readnone
  store float %1193, float* %a_out, align 4
  %1194 = load <4 x float>* %a4_in, align 16
  %1195 = load <4 x float>* %b4_in, align 16
  %1196 = load <4 x float>* %c4_in, align 16
  %1197 = call <4 x float> @_Z3fmaDv4_fS_S_(<4 x float> %1194, <4 x float> %1195, <4 x float> %1196) readnone
  store <4 x float> %1197, <4 x float>* %a4_out, align 16
  %1198 = load <8 x float>* %a8_in, align 32
  %1199 = load <8 x float>* %b8_in, align 32
  %1200 = load <8 x float>* %c8_in, align 32
  %1201 = call <8 x float> @_Z3fmaDv8_fS_S_(<8 x float> %1198, <8 x float> %1199, <8 x float> %1200) readnone
  store <8 x float> %1201, <8 x float>* %a8_out, align 32
  %1202 = load <16 x float>* %a16_in, align 64
  %1203 = load <16 x float>* %b16_in, align 64
  %1204 = load <16 x float>* %c16_in, align 64
  %1205 = call <16 x float> @_Z3fmaDv16_fS_S_(<16 x float> %1202, <16 x float> %1203, <16 x float> %1204) readnone
  store <16 x float> %1205, <16 x float>* %a16_out, align 64
  %1206 = load float* %a_in, align 4
  %1207 = load float* %b_in, align 4
  %1208 = load float* %c_in, align 4
  %1209 = call float @_Z3madfff(float %1206, float %1207, float %1208) readnone
  store float %1209, float* %a_out, align 4
  %1210 = load <4 x float>* %a4_in, align 16
  %1211 = load <4 x float>* %b4_in, align 16
  %1212 = load <4 x float>* %c4_in, align 16
  %1213 = call <4 x float> @_Z3madDv4_fS_S_(<4 x float> %1210, <4 x float> %1211, <4 x float> %1212) readnone
  store <4 x float> %1213, <4 x float>* %a4_out, align 16
  %1214 = load <8 x float>* %a8_in, align 32
  %1215 = load <8 x float>* %b8_in, align 32
  %1216 = load <8 x float>* %c8_in, align 32
  %1217 = call <8 x float> @_Z3madDv8_fS_S_(<8 x float> %1214, <8 x float> %1215, <8 x float> %1216) readnone
  store <8 x float> %1217, <8 x float>* %a8_out, align 32
  %1218 = load <16 x float>* %a16_in, align 64
  %1219 = load <16 x float>* %b16_in, align 64
  %1220 = load <16 x float>* %c16_in, align 64
  %1221 = call <16 x float> @_Z3madDv16_fS_S_(<16 x float> %1218, <16 x float> %1219, <16 x float> %1220) readnone
  store <16 x float> %1221, <16 x float>* %a16_out, align 64
  %1222 = load float* %a_in, align 4
  %1223 = call float @_Z4rintf(float %1222) readnone
  store float %1223, float* %a_out, align 4
  %1224 = load <4 x float>* %a4_in, align 16
  %1225 = call <4 x float> @_Z4rintDv4_f(<4 x float> %1224) readnone
  store <4 x float> %1225, <4 x float>* %a4_out, align 16
  %1226 = load <8 x float>* %a8_in, align 32
  %1227 = call <8 x float> @_Z4rintDv8_f(<8 x float> %1226) readnone
  store <8 x float> %1227, <8 x float>* %a8_out, align 32
  %1228 = load <16 x float>* %a16_in, align 64
  %1229 = call <16 x float> @_Z4rintDv16_f(<16 x float> %1228) readnone
  store <16 x float> %1229, <16 x float>* %a16_out, align 64
  %1230 = load float* %a_in, align 4
  %1231 = call float @_Z5roundf(float %1230) readnone
  store float %1231, float* %a_out, align 4
  %1232 = load <4 x float>* %a4_in, align 16
  %1233 = call <4 x float> @_Z5roundDv4_f(<4 x float> %1232) readnone
  store <4 x float> %1233, <4 x float>* %a4_out, align 16
  %1234 = load <8 x float>* %a8_in, align 32
  %1235 = call <8 x float> @_Z5roundDv8_f(<8 x float> %1234) readnone
  store <8 x float> %1235, <8 x float>* %a8_out, align 32
  %1236 = load <16 x float>* %a16_in, align 64
  %1237 = call <16 x float> @_Z5roundDv16_f(<16 x float> %1236) readnone
  store <16 x float> %1237, <16 x float>* %a16_out, align 64
  %1238 = load float* %a_in, align 4
  %1239 = call float @_Z5truncf(float %1238) readnone
  store float %1239, float* %a_out, align 4
  %1240 = load <4 x float>* %a4_in, align 16
  %1241 = call <4 x float> @_Z5truncDv4_f(<4 x float> %1240) readnone
  store <4 x float> %1241, <4 x float>* %a4_out, align 16
  %1242 = load <8 x float>* %a8_in, align 32
  %1243 = call <8 x float> @_Z5truncDv8_f(<8 x float> %1242) readnone
  store <8 x float> %1243, <8 x float>* %a8_out, align 32
  %1244 = load <16 x float>* %a16_in, align 64
  %1245 = call <16 x float> @_Z5truncDv16_f(<16 x float> %1244) readnone
  store <16 x float> %1245, <16 x float>* %a16_out, align 64
  %1246 = load float* %a_in, align 4
  %1247 = call float @_Z4cbrtf(float %1246) readnone
  store float %1247, float* %a_out, align 4
  %1248 = load <4 x float>* %a4_in, align 16
  %1249 = call <4 x float> @_Z4cbrtDv4_f(<4 x float> %1248) readnone
  store <4 x float> %1249, <4 x float>* %a4_out, align 16
  %1250 = load <8 x float>* %a8_in, align 32
  %1251 = call <8 x float> @_Z4cbrtDv8_f(<8 x float> %1250) readnone
  store <8 x float> %1251, <8 x float>* %a8_out, align 32
  %1252 = load <16 x float>* %a16_in, align 64
  %1253 = call <16 x float> @_Z4cbrtDv16_f(<16 x float> %1252) readnone
  store <16 x float> %1253, <16 x float>* %a16_out, align 64
  %1254 = load float* %a_in, align 4
  %1255 = load float* %b_in, align 4
  %1256 = call float @_Z4powrff(float %1254, float %1255) readnone
  store float %1256, float* %a_out, align 4
  %1257 = load <4 x float>* %a4_in, align 16
  %1258 = load <4 x float>* %b4_in, align 16
  %1259 = call <4 x float> @_Z4powrDv4_fS_(<4 x float> %1257, <4 x float> %1258) readnone
  store <4 x float> %1259, <4 x float>* %a4_out, align 16
  %1260 = load <8 x float>* %a8_in, align 32
  %1261 = load <8 x float>* %b8_in, align 32
  %1262 = call <8 x float> @_Z4powrDv8_fS_(<8 x float> %1260, <8 x float> %1261) readnone
  store <8 x float> %1262, <8 x float>* %a8_out, align 32
  %1263 = load <16 x float>* %a16_in, align 64
  %1264 = load <16 x float>* %b16_in, align 64
  %1265 = call <16 x float> @_Z4powrDv16_fS_(<16 x float> %1263, <16 x float> %1264) readnone
  store <16 x float> %1265, <16 x float>* %a16_out, align 64
  %1266 = load float* %a_in, align 4
  %1267 = load float* %b_in, align 4
  %1268 = call float @_Z4fmodff(float %1266, float %1267) readnone
  store float %1268, float* %a_out, align 4
  %1269 = load <4 x float>* %a4_in, align 16
  %1270 = load <4 x float>* %b4_in, align 16
  %1271 = call <4 x float> @_Z4fmodDv4_fS_(<4 x float> %1269, <4 x float> %1270) readnone
  store <4 x float> %1271, <4 x float>* %a4_out, align 16
  %1272 = load <8 x float>* %a8_in, align 32
  %1273 = load <8 x float>* %b8_in, align 32
  %1274 = call <8 x float> @_Z4fmodDv8_fS_(<8 x float> %1272, <8 x float> %1273) readnone
  store <8 x float> %1274, <8 x float>* %a8_out, align 32
  %1275 = load <16 x float>* %a16_in, align 64
  %1276 = load <16 x float>* %b16_in, align 64
  %1277 = call <16 x float> @_Z4fmodDv16_fS_(<16 x float> %1275, <16 x float> %1276) readnone
  store <16 x float> %1277, <16 x float>* %a16_out, align 64
  %1278 = load float* %a_in, align 4
  %1279 = load float* %b_in, align 4
  %1280 = call float @_Z4fminff(float %1278, float %1279) readnone
  store float %1280, float* %a_out, align 4
  %1281 = load <4 x float>* %a4_in, align 16
  %1282 = load <4 x float>* %b4_in, align 16
  %1283 = call <4 x float> @_Z4fminDv4_fS_(<4 x float> %1281, <4 x float> %1282) readnone
  store <4 x float> %1283, <4 x float>* %a4_out, align 16
  %1284 = load <8 x float>* %a8_in, align 32
  %1285 = load <8 x float>* %b8_in, align 32
  %1286 = call <8 x float> @_Z4fminDv8_fS_(<8 x float> %1284, <8 x float> %1285) readnone
  store <8 x float> %1286, <8 x float>* %a8_out, align 32
  %1287 = load <16 x float>* %a16_in, align 64
  %1288 = load <16 x float>* %b16_in, align 64
  %1289 = call <16 x float> @_Z4fminDv16_fS_(<16 x float> %1287, <16 x float> %1288) readnone
  store <16 x float> %1289, <16 x float>* %a16_out, align 64
  %1290 = load float* %a_in, align 4
  %1291 = load float* %b_in, align 4
  %1292 = call float @_Z4fmaxff(float %1290, float %1291) readnone
  store float %1292, float* %a_out, align 4
  %1293 = load <4 x float>* %a4_in, align 16
  %1294 = load <4 x float>* %b4_in, align 16
  %1295 = call <4 x float> @_Z4fmaxDv4_fS_(<4 x float> %1293, <4 x float> %1294) readnone
  store <4 x float> %1295, <4 x float>* %a4_out, align 16
  %1296 = load <8 x float>* %a8_in, align 32
  %1297 = load <8 x float>* %b8_in, align 32
  %1298 = call <8 x float> @_Z4fmaxDv8_fS_(<8 x float> %1296, <8 x float> %1297) readnone
  store <8 x float> %1298, <8 x float>* %a8_out, align 32
  %1299 = load <16 x float>* %a16_in, align 64
  %1300 = load <16 x float>* %b16_in, align 64
  %1301 = call <16 x float> @_Z4fmaxDv16_fS_(<16 x float> %1299, <16 x float> %1300) readnone
  store <16 x float> %1301, <16 x float>* %a16_out, align 64
  %1302 = load <4 x float>* %a4_in, align 16
  %1303 = load float* %b_in, align 4
  %1304 = call <4 x float> @_Z4fminDv4_ff(<4 x float> %1302, float %1303) readnone
  store <4 x float> %1304, <4 x float>* %a4_out, align 16
  %1305 = load <8 x float>* %a8_in, align 32
  %1306 = load float* %b_in, align 4
  %1307 = call <8 x float> @_Z4fminDv8_ff(<8 x float> %1305, float %1306) readnone
  store <8 x float> %1307, <8 x float>* %a8_out, align 32
  %1308 = load <16 x float>* %a16_in, align 64
  %1309 = load float* %b_in, align 4
  %1310 = call <16 x float> @_Z4fminDv16_ff(<16 x float> %1308, float %1309) readnone
  store <16 x float> %1310, <16 x float>* %a16_out, align 64
  %1311 = load <4 x float>* %a4_in, align 16
  %1312 = load float* %b_in, align 4
  %1313 = call <4 x float> @_Z4fmaxDv4_ff(<4 x float> %1311, float %1312) readnone
  store <4 x float> %1313, <4 x float>* %a4_out, align 16
  %1314 = load <8 x float>* %a8_in, align 32
  %1315 = load float* %b_in, align 4
  %1316 = call <8 x float> @_Z4fmaxDv8_ff(<8 x float> %1314, float %1315) readnone
  store <8 x float> %1316, <8 x float>* %a8_out, align 32
  %1317 = load <16 x float>* %a16_in, align 64
  %1318 = load float* %b_in, align 4
  %1319 = call <16 x float> @_Z4fmaxDv16_ff(<16 x float> %1317, float %1318) readnone
  store <16 x float> %1319, <16 x float>* %a16_out, align 64
  %1320 = load <4 x float>* %a4_in, align 16
  %1321 = load i32* %i_in, align 4
  %1322 = insertelement <4 x i32> undef, i32 %1321, i32 0
  %1323 = shufflevector <4 x i32> %1322, <4 x i32> %1322, <4 x i32> zeroinitializer
  %1324 = call <4 x float> @_Z4pownDv4_fDv4_i(<4 x float> %1320, <4 x i32> %1323) readnone
  store <4 x float> %1324, <4 x float>* %a4_out, align 16
  %1325 = load <8 x float>* %a8_in, align 32
  %1326 = load i32* %i_in, align 4
  %1327 = insertelement <8 x i32> undef, i32 %1326, i32 0
  %1328 = shufflevector <8 x i32> %1327, <8 x i32> %1327, <8 x i32> zeroinitializer
  %1329 = call <8 x float> @_Z4pownDv8_fDv8_i(<8 x float> %1325, <8 x i32> %1328) readnone
  store <8 x float> %1329, <8 x float>* %a8_out, align 32
  %1330 = load <16 x float>* %a16_in, align 64
  %1331 = load i32* %i_in, align 4
  %1332 = insertelement <16 x i32> undef, i32 %1331, i32 0
  %1333 = shufflevector <16 x i32> %1332, <16 x i32> %1332, <16 x i32> zeroinitializer
  %1334 = call <16 x float> @_Z4pownDv16_fDv16_i(<16 x float> %1330, <16 x i32> %1333) readnone
  store <16 x float> %1334, <16 x float>* %a16_out, align 64
  %1335 = load float* %a_in, align 4
  %1336 = call i32 @_Z5ilogbf(float %1335) readnone
  store i32 %1336, i32* %i_out, align 4
  %1337 = load <4 x float>* %a4_in, align 16
  %1338 = call <4 x i32> @_Z5ilogbDv4_f(<4 x float> %1337) readnone
  store <4 x i32> %1338, <4 x i32>* %i4_out, align 16
  %1339 = load <8 x float>* %a8_in, align 32
  %1340 = call <8 x i32> @_Z5ilogbDv8_f(<8 x float> %1339) readnone
  store <8 x i32> %1340, <8 x i32>* %i8_out, align 32
  %1341 = load <16 x float>* %a16_in, align 64
  %1342 = call <16 x i32> @_Z5ilogbDv16_f(<16 x float> %1341) readnone
  store <16 x i32> %1342, <16 x i32>* %i16_out, align 64
  %1343 = load i32* %ui_in, align 4
  %1344 = call float @_Z3nanj(i32 %1343) readnone
  store float %1344, float* %a_out, align 4
  %1345 = load <4 x i32>* %ui4_in, align 16
  %1346 = call <4 x float> @_Z3nanDv4_j(<4 x i32> %1345) readnone
  store <4 x float> %1346, <4 x float>* %a4_out, align 16
  %1347 = load <8 x i32>* %ui8_in, align 32
  %1348 = call <8 x float> @_Z3nanDv8_j(<8 x i32> %1347) readnone
  store <8 x float> %1348, <8 x float>* %a8_out, align 32
  %1349 = load <16 x i32>* %ui16_in, align 64
  %1350 = call <16 x float> @_Z3nanDv16_j(<16 x i32> %1349) readnone
  store <16 x float> %1350, <16 x float>* %a16_out, align 64
  %1351 = load float* %a_in, align 4
  %1352 = call float @_Z5fractfPf(float %1351, float* %b_out)
  store float %1352, float* %a_out, align 4
  %1353 = load <4 x float>* %a4_in, align 16
  %1354 = call <4 x float> @_Z5fractDv4_fPS_(<4 x float> %1353, <4 x float>* %b4_out)
  store <4 x float> %1354, <4 x float>* %a4_out, align 16
  %1355 = load <8 x float>* %a8_in, align 32
  %1356 = call <8 x float> @_Z5fractDv8_fPS_(<8 x float> %1355, <8 x float>* %b8_out)
  store <8 x float> %1356, <8 x float>* %a8_out, align 32
  %1357 = load <16 x float>* %a16_in, align 64
  %1358 = call <16 x float> @_Z5fractDv16_fPS_(<16 x float> %1357, <16 x float>* %b16_out)
  store <16 x float> %1358, <16 x float>* %a16_out, align 64
  %1359 = load float* %a_in, align 4
  %1360 = call float @_Z6lgammaf(float %1359) readnone
  store float %1360, float* %a_out, align 4
  %1361 = load <4 x float>* %a4_in, align 16
  %1362 = call <4 x float> @_Z6lgammaDv4_f(<4 x float> %1361) readnone
  store <4 x float> %1362, <4 x float>* %a4_out, align 16
  %1363 = load <8 x float>* %a8_in, align 32
  %1364 = call <8 x float> @_Z6lgammaDv8_f(<8 x float> %1363) readnone
  store <8 x float> %1364, <8 x float>* %a8_out, align 32
  %1365 = load <16 x float>* %a16_in, align 64
  %1366 = call <16 x float> @_Z6lgammaDv16_f(<16 x float> %1365) readnone
  store <16 x float> %1366, <16 x float>* %a16_out, align 64
  %1367 = load float* %a_in, align 4
  %1368 = call float @_Z8lgamma_rfPi(float %1367, i32* %i_out)
  store float %1368, float* %a_out, align 4
  %1369 = load <4 x float>* %a4_in, align 16
  %1370 = call <4 x float> @_Z8lgamma_rDv4_fPDv4_i(<4 x float> %1369, <4 x i32>* %i4_out)
  store <4 x float> %1370, <4 x float>* %a4_out, align 16
  %1371 = load <8 x float>* %a8_in, align 32
  %1372 = call <8 x float> @_Z8lgamma_rDv8_fPDv8_i(<8 x float> %1371, <8 x i32>* %i8_out)
  store <8 x float> %1372, <8 x float>* %a8_out, align 32
  %1373 = load <16 x float>* %a16_in, align 64
  %1374 = call <16 x float> @_Z8lgamma_rDv16_fPDv16_i(<16 x float> %1373, <16 x i32>* %i16_out)
  store <16 x float> %1374, <16 x float>* %a16_out, align 64
  %1375 = load float* %a_in, align 4
  %1376 = load float* %b_in, align 4
  %1377 = load float* %c_in, align 4
  %1378 = call float @_Z9bitselectfff(float %1375, float %1376, float %1377) readnone
  store float %1378, float* %a_out, align 4
  %1379 = load <4 x float>* %a4_in, align 16
  %1380 = load <4 x float>* %b4_in, align 16
  %1381 = load <4 x float>* %c4_in, align 16
  %1382 = call <4 x float> @_Z9bitselectDv4_fS_S_(<4 x float> %1379, <4 x float> %1380, <4 x float> %1381) readnone
  store <4 x float> %1382, <4 x float>* %a4_out, align 16
  %1383 = load <8 x float>* %a8_in, align 32
  %1384 = load <8 x float>* %b8_in, align 32
  %1385 = load <8 x float>* %c8_in, align 32
  %1386 = call <8 x float> @_Z9bitselectDv8_fS_S_(<8 x float> %1383, <8 x float> %1384, <8 x float> %1385) readnone
  store <8 x float> %1386, <8 x float>* %a8_out, align 32
  %1387 = load <16 x float>* %a16_in, align 64
  %1388 = load <16 x float>* %b16_in, align 64
  %1389 = load <16 x float>* %c16_in, align 64
  %1390 = call <16 x float> @_Z9bitselectDv16_fS_S_(<16 x float> %1387, <16 x float> %1388, <16 x float> %1389) readnone
  store <16 x float> %1390, <16 x float>* %a16_out, align 64
  %1391 = load float* %a_in, align 4
  %1392 = load float* %b_in, align 4
  %1393 = load i8* %ch_in, align 1
  %1394 = call float @_Z6selectffc(float %1391, float %1392, i8 signext %1393) readnone
  store float %1394, float* %a_out, align 4
  %1395 = load <4 x float>* %a4_in, align 16
  %1396 = load <4 x float>* %b4_in, align 16
  %1397 = load <4 x i8>* %ch4_in, align 4
  %1398 = call <4 x float> @_Z6selectDv4_fS_Dv4_c(<4 x float> %1395, <4 x float> %1396, <4 x i8> %1397) readnone
  store <4 x float> %1398, <4 x float>* %a4_out, align 16
  %1399 = load <8 x float>* %a8_in, align 32
  %1400 = load <8 x float>* %b8_in, align 32
  %1401 = load <8 x i8>* %ch8_in, align 8
  %1402 = call <8 x float> @_Z6selectDv8_fS_Dv8_c(<8 x float> %1399, <8 x float> %1400, <8 x i8> %1401) readnone
  store <8 x float> %1402, <8 x float>* %a8_out, align 32
  %1403 = load <16 x float>* %a16_in, align 64
  %1404 = load <16 x float>* %b16_in, align 64
  %1405 = load <16 x i8>* %ch16_in, align 16
  %1406 = call <16 x float> @_Z6selectDv16_fS_Dv16_c(<16 x float> %1403, <16 x float> %1404, <16 x i8> %1405) readnone
  store <16 x float> %1406, <16 x float>* %a16_out, align 64
  %1407 = load float* %a_in, align 4
  %1408 = load float* %b_in, align 4
  %1409 = load i8* %uch_in, align 1
  %1410 = call float @_Z6selectffh(float %1407, float %1408, i8 zeroext %1409) readnone
  store float %1410, float* %a_out, align 4
  %1411 = load <4 x float>* %a4_in, align 16
  %1412 = load <4 x float>* %b4_in, align 16
  %1413 = load <4 x i8>* %uch4_in, align 4
  %1414 = call <4 x float> @_Z6selectDv4_fS_Dv4_h(<4 x float> %1411, <4 x float> %1412, <4 x i8> %1413) readnone
  store <4 x float> %1414, <4 x float>* %a4_out, align 16
  %1415 = load <8 x float>* %a8_in, align 32
  %1416 = load <8 x float>* %b8_in, align 32
  %1417 = load <8 x i8>* %uch8_in, align 8
  %1418 = call <8 x float> @_Z6selectDv8_fS_Dv8_h(<8 x float> %1415, <8 x float> %1416, <8 x i8> %1417) readnone
  store <8 x float> %1418, <8 x float>* %a8_out, align 32
  %1419 = load <16 x float>* %a16_in, align 64
  %1420 = load <16 x float>* %b16_in, align 64
  %1421 = load <16 x i8>* %uch16_in, align 16
  %1422 = call <16 x float> @_Z6selectDv16_fS_Dv16_h(<16 x float> %1419, <16 x float> %1420, <16 x i8> %1421) readnone
  store <16 x float> %1422, <16 x float>* %a16_out, align 64
  %1423 = load float* %a_in, align 4
  %1424 = load float* %b_in, align 4
  %1425 = load i16* %s_in, align 2
  %1426 = call float @_Z6selectffs(float %1423, float %1424, i16 signext %1425) readnone
  store float %1426, float* %a_out, align 4
  %1427 = load <4 x float>* %a4_in, align 16
  %1428 = load <4 x float>* %b4_in, align 16
  %1429 = load <4 x i16>* %s4_in, align 8
  %1430 = call <4 x float> @_Z6selectDv4_fS_Dv4_s(<4 x float> %1427, <4 x float> %1428, <4 x i16> %1429) readnone
  store <4 x float> %1430, <4 x float>* %a4_out, align 16
  %1431 = load <8 x float>* %a8_in, align 32
  %1432 = load <8 x float>* %b8_in, align 32
  %1433 = load <8 x i16>* %s8_in, align 16
  %1434 = call <8 x float> @_Z6selectDv8_fS_Dv8_s(<8 x float> %1431, <8 x float> %1432, <8 x i16> %1433) readnone
  store <8 x float> %1434, <8 x float>* %a8_out, align 32
  %1435 = load <16 x float>* %a16_in, align 64
  %1436 = load <16 x float>* %b16_in, align 64
  %1437 = load <16 x i16>* %s16_in, align 32
  %1438 = call <16 x float> @_Z6selectDv16_fS_Dv16_s(<16 x float> %1435, <16 x float> %1436, <16 x i16> %1437) readnone
  store <16 x float> %1438, <16 x float>* %a16_out, align 64
  %1439 = load float* %a_in, align 4
  %1440 = load float* %b_in, align 4
  %1441 = load i16* %us_in, align 2
  %1442 = call float @_Z6selectfft(float %1439, float %1440, i16 zeroext %1441) readnone
  store float %1442, float* %a_out, align 4
  %1443 = load <4 x float>* %a4_in, align 16
  %1444 = load <4 x float>* %b4_in, align 16
  %1445 = load <4 x i16>* %us4_in, align 8
  %1446 = call <4 x float> @_Z6selectDv4_fS_Dv4_t(<4 x float> %1443, <4 x float> %1444, <4 x i16> %1445) readnone
  store <4 x float> %1446, <4 x float>* %a4_out, align 16
  %1447 = load <8 x float>* %a8_in, align 32
  %1448 = load <8 x float>* %b8_in, align 32
  %1449 = load <8 x i16>* %us8_in, align 16
  %1450 = call <8 x float> @_Z6selectDv8_fS_Dv8_t(<8 x float> %1447, <8 x float> %1448, <8 x i16> %1449) readnone
  store <8 x float> %1450, <8 x float>* %a8_out, align 32
  %1451 = load <16 x float>* %a16_in, align 64
  %1452 = load <16 x float>* %b16_in, align 64
  %1453 = load <16 x i16>* %us16_in, align 32
  %1454 = call <16 x float> @_Z6selectDv16_fS_Dv16_t(<16 x float> %1451, <16 x float> %1452, <16 x i16> %1453) readnone
  store <16 x float> %1454, <16 x float>* %a16_out, align 64
  %1455 = load float* %a_in, align 4
  %1456 = load float* %b_in, align 4
  %1457 = load i32* %i_in, align 4
  %1458 = call float @_Z6selectffi(float %1455, float %1456, i32 %1457) readnone
  store float %1458, float* %a_out, align 4
  %1459 = load <4 x float>* %a4_in, align 16
  %1460 = load <4 x float>* %b4_in, align 16
  %1461 = load <4 x i32>* %i4_in, align 16
  %1462 = call <4 x float> @_Z6selectDv4_fS_Dv4_i(<4 x float> %1459, <4 x float> %1460, <4 x i32> %1461) readnone
  store <4 x float> %1462, <4 x float>* %a4_out, align 16
  %1463 = load <8 x float>* %a8_in, align 32
  %1464 = load <8 x float>* %b8_in, align 32
  %1465 = load <8 x i32>* %i8_in, align 32
  %1466 = call <8 x float> @_Z6selectDv8_fS_Dv8_i(<8 x float> %1463, <8 x float> %1464, <8 x i32> %1465) readnone
  store <8 x float> %1466, <8 x float>* %a8_out, align 32
  %1467 = load <16 x float>* %a16_in, align 64
  %1468 = load <16 x float>* %b16_in, align 64
  %1469 = load <16 x i32>* %i16_in, align 64
  %1470 = call <16 x float> @_Z6selectDv16_fS_Dv16_i(<16 x float> %1467, <16 x float> %1468, <16 x i32> %1469) readnone
  store <16 x float> %1470, <16 x float>* %a16_out, align 64
  %1471 = load float* %a_in, align 4
  %1472 = load float* %b_in, align 4
  %1473 = load i32* %ui_in, align 4
  %1474 = call float @_Z6selectffj(float %1471, float %1472, i32 %1473) readnone
  store float %1474, float* %a_out, align 4
  %1475 = load <4 x float>* %a4_in, align 16
  %1476 = load <4 x float>* %b4_in, align 16
  %1477 = load <4 x i32>* %ui4_in, align 16
  %1478 = call <4 x float> @_Z6selectDv4_fS_Dv4_j(<4 x float> %1475, <4 x float> %1476, <4 x i32> %1477) readnone
  store <4 x float> %1478, <4 x float>* %a4_out, align 16
  %1479 = load <8 x float>* %a8_in, align 32
  %1480 = load <8 x float>* %b8_in, align 32
  %1481 = load <8 x i32>* %ui8_in, align 32
  %1482 = call <8 x float> @_Z6selectDv8_fS_Dv8_j(<8 x float> %1479, <8 x float> %1480, <8 x i32> %1481) readnone
  store <8 x float> %1482, <8 x float>* %a8_out, align 32
  %1483 = load <16 x float>* %a16_in, align 64
  %1484 = load <16 x float>* %b16_in, align 64
  %1485 = load <16 x i32>* %ui16_in, align 64
  %1486 = call <16 x float> @_Z6selectDv16_fS_Dv16_j(<16 x float> %1483, <16 x float> %1484, <16 x i32> %1485) readnone
  store <16 x float> %1486, <16 x float>* %a16_out, align 64
  %1487 = load float* %a_in, align 4
  %1488 = load float* %b_in, align 4
  %1489 = load i64* %l_in, align 8
  %1490 = call float @_Z6selectffl(float %1487, float %1488, i64 %1489) readnone
  store float %1490, float* %a_out, align 4
  %1491 = load <4 x float>* %a4_in, align 16
  %1492 = load <4 x float>* %b4_in, align 16
  %1493 = load <4 x i64>* %l4_in, align 32
  %1494 = call <4 x float> @_Z6selectDv4_fS_Dv4_l(<4 x float> %1491, <4 x float> %1492, <4 x i64> %1493) readnone
  store <4 x float> %1494, <4 x float>* %a4_out, align 16
  %1495 = load <8 x float>* %a8_in, align 32
  %1496 = load <8 x float>* %b8_in, align 32
  %1497 = load <8 x i64>* %l8_in, align 64
  %1498 = call <8 x float> @_Z6selectDv8_fS_Dv8_l(<8 x float> %1495, <8 x float> %1496, <8 x i64> %1497) readnone
  store <8 x float> %1498, <8 x float>* %a8_out, align 32
  %1499 = load <16 x float>* %a16_in, align 64
  %1500 = load <16 x float>* %b16_in, align 64
  %1501 = load <16 x i64>* %l16_in, align 128
  %1502 = call <16 x float> @_Z6selectDv16_fS_Dv16_l(<16 x float> %1499, <16 x float> %1500, <16 x i64> %1501) readnone
  store <16 x float> %1502, <16 x float>* %a16_out, align 64
  %1503 = load float* %a_in, align 4
  %1504 = load float* %b_in, align 4
  %1505 = load i64* %ul_in, align 8
  %1506 = call float @_Z6selectffm(float %1503, float %1504, i64 %1505) readnone
  store float %1506, float* %a_out, align 4
  %1507 = load <4 x float>* %a4_in, align 16
  %1508 = load <4 x float>* %b4_in, align 16
  %1509 = load <4 x i64>* %ul4_in, align 32
  %1510 = call <4 x float> @_Z6selectDv4_fS_Dv4_m(<4 x float> %1507, <4 x float> %1508, <4 x i64> %1509) readnone
  store <4 x float> %1510, <4 x float>* %a4_out, align 16
  %1511 = load <8 x float>* %a8_in, align 32
  %1512 = load <8 x float>* %b8_in, align 32
  %1513 = load <8 x i64>* %ul8_in, align 64
  %1514 = call <8 x float> @_Z6selectDv8_fS_Dv8_m(<8 x float> %1511, <8 x float> %1512, <8 x i64> %1513) readnone
  store <8 x float> %1514, <8 x float>* %a8_out, align 32
  %1515 = load <16 x float>* %a16_in, align 64
  %1516 = load <16 x float>* %b16_in, align 64
  %1517 = load <16 x i64>* %ul16_in, align 128
  %1518 = call <16 x float> @_Z6selectDv16_fS_Dv16_m(<16 x float> %1515, <16 x float> %1516, <16 x i64> %1517) readnone
  store <16 x float> %1518, <16 x float>* %a16_out, align 64
  %1519 = load float* %a_in, align 4
  %1520 = load float* %b_in, align 4
  %1521 = call float @_Z9remainderff(float %1519, float %1520) readnone
  store float %1521, float* %a_out, align 4
  %1522 = load <4 x float>* %a4_in, align 16
  %1523 = load <4 x float>* %b4_in, align 16
  %1524 = call <4 x float> @_Z9remainderDv4_fS_(<4 x float> %1522, <4 x float> %1523) readnone
  store <4 x float> %1524, <4 x float>* %a4_out, align 16
  %1525 = load <8 x float>* %a8_in, align 32
  %1526 = load <8 x float>* %b8_in, align 32
  %1527 = call <8 x float> @_Z9remainderDv8_fS_(<8 x float> %1525, <8 x float> %1526) readnone
  store <8 x float> %1527, <8 x float>* %a8_out, align 32
  %1528 = load <16 x float>* %a16_in, align 64
  %1529 = load <16 x float>* %b16_in, align 64
  %1530 = call <16 x float> @_Z9remainderDv16_fS_(<16 x float> %1528, <16 x float> %1529) readnone
  store <16 x float> %1530, <16 x float>* %a16_out, align 64
  %1531 = load float* %a_in, align 4
  %1532 = load float* %b_in, align 4
  %1533 = call float @_Z6remquoffPi(float %1531, float %1532, i32* %i_out)
  store float %1533, float* %a_out, align 4
  %1534 = load <2 x float>* %a2_in, align 8
  %1535 = load <2 x float>* %b2_in, align 8
  %1536 = call <2 x float> @_Z6remquoDv2_fS_PDv2_i(<2 x float> %1534, <2 x float> %1535, <2 x i32>* %i2_out)
  store <2 x float> %1536, <2 x float>* %a2_out, align 8
  %1537 = load <3 x float>* %a3_in, align 16
  %1538 = load <3 x float>* %b3_in, align 16
  %1539 = call <3 x float> @_Z6remquoDv3_fS_PDv3_i(<3 x float> %1537, <3 x float> %1538, <3 x i32>* %i3_out)
  store <3 x float> %1539, <3 x float>* %a3_out, align 16
  %1540 = load <4 x float>* %a4_in, align 16
  %1541 = load <4 x float>* %b4_in, align 16
  %1542 = call <4 x float> @_Z6remquoDv4_fS_PDv4_i(<4 x float> %1540, <4 x float> %1541, <4 x i32>* %i4_out)
  store <4 x float> %1542, <4 x float>* %a4_out, align 16
  %1543 = load <8 x float>* %a8_in, align 32
  %1544 = load <8 x float>* %b8_in, align 32
  %1545 = call <8 x float> @_Z6remquoDv8_fS_PDv8_i(<8 x float> %1543, <8 x float> %1544, <8 x i32>* %i8_out)
  store <8 x float> %1545, <8 x float>* %a8_out, align 32
  %1546 = load <16 x float>* %a16_in, align 64
  %1547 = load <16 x float>* %b16_in, align 64
  %1548 = call <16 x float> @_Z6remquoDv16_fS_PDv16_i(<16 x float> %1546, <16 x float> %1547, <16 x i32>* %i16_out)
  store <16 x float> %1548, <16 x float>* %a16_out, align 64
  ret void
}

declare float @_Z4acosf(float) readnone

declare <4 x float> @_Z4acosDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4acosDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4acosDv16_f(<16 x float>) readnone

declare float @_Z6acospif(float) readnone

declare <4 x float> @_Z6acospiDv4_f(<4 x float>) readnone

declare <8 x float> @_Z6acospiDv8_f(<8 x float>) readnone

declare <16 x float> @_Z6acospiDv16_f(<16 x float>) readnone

declare float @_Z4asinf(float) readnone

declare <4 x float> @_Z4asinDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4asinDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4asinDv16_f(<16 x float>) readnone

declare float @_Z6asinpif(float) readnone

declare <4 x float> @_Z6asinpiDv4_f(<4 x float>) readnone

declare <8 x float> @_Z6asinpiDv8_f(<8 x float>) readnone

declare <16 x float> @_Z6asinpiDv16_f(<16 x float>) readnone

declare float @_Z4atanf(float) readnone

declare <4 x float> @_Z4atanDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4atanDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4atanDv16_f(<16 x float>) readnone

declare float @_Z5atan2ff(float, float) readnone

declare <4 x float> @_Z5atan2Dv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z5atan2Dv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z5atan2Dv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z7atan2piff(float, float) readnone

declare <4 x float> @_Z7atan2piDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z7atan2piDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z7atan2piDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z6atanpif(float) readnone

declare <4 x float> @_Z6atanpiDv4_f(<4 x float>) readnone

declare <8 x float> @_Z6atanpiDv8_f(<8 x float>) readnone

declare <16 x float> @_Z6atanpiDv16_f(<16 x float>) readnone

declare float @_Z3cosf(float) readnone

declare <4 x float> @_Z3cosDv4_f(<4 x float>) readnone

declare <8 x float> @_Z3cosDv8_f(<8 x float>) readnone

declare <16 x float> @_Z3cosDv16_f(<16 x float>) readnone

declare float @_Z4coshf(float) readnone

declare <4 x float> @_Z4coshDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4coshDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4coshDv16_f(<16 x float>) readnone

declare float @_Z5cospif(float) readnone

declare <4 x float> @_Z5cospiDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5cospiDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5cospiDv16_f(<16 x float>) readnone

declare float @_Z3expf(float) readnone

declare <4 x float> @_Z3expDv4_f(<4 x float>) readnone

declare <8 x float> @_Z3expDv8_f(<8 x float>) readnone

declare <16 x float> @_Z3expDv16_f(<16 x float>) readnone

declare float @_Z4exp2f(float) readnone

declare <4 x float> @_Z4exp2Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z4exp2Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z4exp2Dv16_f(<16 x float>) readnone

declare float @_Z5exp10f(float) readnone

declare <4 x float> @_Z5exp10Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z5exp10Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z5exp10Dv16_f(<16 x float>) readnone

declare float @_Z5expm1f(float) readnone

declare <4 x float> @_Z5expm1Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z5expm1Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z5expm1Dv16_f(<16 x float>) readnone

declare float @_Z3logf(float) readnone

declare <4 x float> @_Z3logDv4_f(<4 x float>) readnone

declare <8 x float> @_Z3logDv8_f(<8 x float>) readnone

declare <16 x float> @_Z3logDv16_f(<16 x float>) readnone

declare float @_Z4log2f(float) readnone

declare <4 x float> @_Z4log2Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z4log2Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z4log2Dv16_f(<16 x float>) readnone

declare float @_Z5log10f(float) readnone

declare <4 x float> @_Z5log10Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z5log10Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z5log10Dv16_f(<16 x float>) readnone

declare float @_Z5log1pf(float) readnone

declare <4 x float> @_Z5log1pDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5log1pDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5log1pDv16_f(<16 x float>) readnone

declare float @_Z4logbf(float) readnone

declare <4 x float> @_Z4logbDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4logbDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4logbDv16_f(<16 x float>) readnone

declare float @_Z4ceilf(float) readnone

declare <4 x float> @_Z4ceilDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4ceilDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4ceilDv16_f(<16 x float>) readnone

declare float @_Z3powff(float, float) readnone

declare <4 x float> @_Z3powDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z3powDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z3powDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z5clampfff(float, float, float) readnone

declare <4 x float> @_Z5clampDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) readnone

declare <8 x float> @_Z5clampDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) readnone

declare <16 x float> @_Z5clampDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) readnone

declare <4 x float> @_Z5clampDv4_fff(<4 x float>, float, float) readnone

declare <8 x float> @_Z5clampDv8_fff(<8 x float>, float, float) readnone

declare <16 x float> @_Z5clampDv16_fff(<16 x float>, float, float) readnone

declare float @_Z4sinhf(float) readnone

declare <4 x float> @_Z4sinhDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4sinhDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4sinhDv16_f(<16 x float>) readnone

declare float @_Z3sinf(float) readnone

declare <4 x float> @_Z3sinDv4_f(<4 x float>) readnone

declare <8 x float> @_Z3sinDv8_f(<8 x float>) readnone

declare <16 x float> @_Z3sinDv16_f(<16 x float>) readnone

declare float @_Z5sinpif(float) readnone

declare <4 x float> @_Z5sinpiDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5sinpiDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5sinpiDv16_f(<16 x float>) readnone

declare float @_Z4sqrtf(float) readnone

declare <4 x float> @_Z4sqrtDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4sqrtDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4sqrtDv16_f(<16 x float>) readnone

declare float @_Z5rsqrtf(float) readnone

declare <4 x float> @_Z5rsqrtDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5rsqrtDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5rsqrtDv16_f(<16 x float>) readnone

declare float @_Z3tanf(float) readnone

declare <4 x float> @_Z3tanDv4_f(<4 x float>) readnone

declare <8 x float> @_Z3tanDv8_f(<8 x float>) readnone

declare <16 x float> @_Z3tanDv16_f(<16 x float>) readnone

declare float @_Z4tanhf(float) readnone

declare <4 x float> @_Z4tanhDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4tanhDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4tanhDv16_f(<16 x float>) readnone

declare float @_Z5tanpif(float) readnone

declare <4 x float> @_Z5tanpiDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5tanpiDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5tanpiDv16_f(<16 x float>) readnone

declare float @_Z4fabsf(float) readnone

declare <4 x float> @_Z4fabsDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4fabsDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4fabsDv16_f(<16 x float>) readnone

declare float @_Z10native_sinf(float) readnone

declare <4 x float> @_Z10native_sinDv4_f(<4 x float>) readnone

declare <8 x float> @_Z10native_sinDv8_f(<8 x float>) readnone

declare <16 x float> @_Z10native_sinDv16_f(<16 x float>) readnone

declare float @_Z10native_cosf(float) readnone

declare <4 x float> @_Z10native_cosDv4_f(<4 x float>) readnone

declare <8 x float> @_Z10native_cosDv8_f(<8 x float>) readnone

declare <16 x float> @_Z10native_cosDv16_f(<16 x float>) readnone

declare float @_Z12native_rsqrtf(float) readnone

declare <4 x float> @_Z12native_rsqrtDv4_f(<4 x float>) readnone

declare <8 x float> @_Z12native_rsqrtDv8_f(<8 x float>) readnone

declare <16 x float> @_Z12native_rsqrtDv16_f(<16 x float>) readnone

declare float @_Z10native_logf(float) readnone

declare <4 x float> @_Z10native_logDv4_f(<4 x float>) readnone

declare <8 x float> @_Z10native_logDv8_f(<8 x float>) readnone

declare <16 x float> @_Z10native_logDv16_f(<16 x float>) readnone

declare float @_Z11native_log2f(float) readnone

declare <4 x float> @_Z11native_log2Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z11native_log2Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z11native_log2Dv16_f(<16 x float>) readnone

declare float @_Z12native_log10f(float) readnone

declare <4 x float> @_Z12native_log10Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z12native_log10Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z12native_log10Dv16_f(<16 x float>) readnone

declare float @_Z10native_expf(float) readnone

declare <4 x float> @_Z10native_expDv4_f(<4 x float>) readnone

declare <8 x float> @_Z10native_expDv8_f(<8 x float>) readnone

declare <16 x float> @_Z10native_expDv16_f(<16 x float>) readnone

declare float @_Z11native_exp2f(float) readnone

declare <4 x float> @_Z11native_exp2Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z11native_exp2Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z11native_exp2Dv16_f(<16 x float>) readnone

declare float @_Z12native_exp10f(float) readnone

declare <4 x float> @_Z12native_exp10Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z12native_exp10Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z12native_exp10Dv16_f(<16 x float>) readnone

declare float @_Z13native_divideff(float, float) readnone

declare <4 x float> @_Z13native_divideDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z13native_divideDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z13native_divideDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z11native_powrff(float, float) readnone

declare <4 x float> @_Z11native_powrDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z11native_powrDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z11native_powrDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z12native_recipf(float) readnone

declare <4 x float> @_Z12native_recipDv4_f(<4 x float>) readnone

declare <8 x float> @_Z12native_recipDv8_f(<8 x float>) readnone

declare <16 x float> @_Z12native_recipDv16_f(<16 x float>) readnone

declare float @_Z11native_sqrtf(float) readnone

declare <4 x float> @_Z11native_sqrtDv4_f(<4 x float>) readnone

declare <8 x float> @_Z11native_sqrtDv8_f(<8 x float>) readnone

declare <16 x float> @_Z11native_sqrtDv16_f(<16 x float>) readnone

declare float @_Z10native_tanf(float) readnone

declare <4 x float> @_Z10native_tanDv4_f(<4 x float>) readnone

declare <8 x float> @_Z10native_tanDv8_f(<8 x float>) readnone

declare <16 x float> @_Z10native_tanDv16_f(<16 x float>) readnone

declare float @_Z8half_logf(float) readnone

declare <4 x float> @_Z8half_logDv4_f(<4 x float>) readnone

declare <8 x float> @_Z8half_logDv8_f(<8 x float>) readnone

declare <16 x float> @_Z8half_logDv16_f(<16 x float>) readnone

declare float @_Z9half_log2f(float) readnone

declare <4 x float> @_Z9half_log2Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z9half_log2Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z9half_log2Dv16_f(<16 x float>) readnone

declare float @_Z10half_log10f(float) readnone

declare <4 x float> @_Z10half_log10Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z10half_log10Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z10half_log10Dv16_f(<16 x float>) readnone

declare float @_Z8half_expf(float) readnone

declare <4 x float> @_Z8half_expDv4_f(<4 x float>) readnone

declare <8 x float> @_Z8half_expDv8_f(<8 x float>) readnone

declare <16 x float> @_Z8half_expDv16_f(<16 x float>) readnone

declare float @_Z9half_exp2f(float) readnone

declare <4 x float> @_Z9half_exp2Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z9half_exp2Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z9half_exp2Dv16_f(<16 x float>) readnone

declare float @_Z10half_exp10f(float) readnone

declare <4 x float> @_Z10half_exp10Dv4_f(<4 x float>) readnone

declare <8 x float> @_Z10half_exp10Dv8_f(<8 x float>) readnone

declare <16 x float> @_Z10half_exp10Dv16_f(<16 x float>) readnone

declare float @_Z8half_cosf(float) readnone

declare <4 x float> @_Z8half_cosDv4_f(<4 x float>) readnone

declare <8 x float> @_Z8half_cosDv8_f(<8 x float>) readnone

declare <16 x float> @_Z8half_cosDv16_f(<16 x float>) readnone

declare float @_Z11half_divideff(float, float) readnone

declare <4 x float> @_Z11half_divideDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z11half_divideDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z11half_divideDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z9half_powrff(float, float) readnone

declare <4 x float> @_Z9half_powrDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z9half_powrDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z9half_powrDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z10half_recipf(float) readnone

declare <4 x float> @_Z10half_recipDv4_f(<4 x float>) readnone

declare <8 x float> @_Z10half_recipDv8_f(<8 x float>) readnone

declare <16 x float> @_Z10half_recipDv16_f(<16 x float>) readnone

declare float @_Z10half_rsqrtf(float) readnone

declare <4 x float> @_Z10half_rsqrtDv4_f(<4 x float>) readnone

declare <8 x float> @_Z10half_rsqrtDv8_f(<8 x float>) readnone

declare <16 x float> @_Z10half_rsqrtDv16_f(<16 x float>) readnone

declare float @_Z8half_sinf(float) readnone

declare <4 x float> @_Z8half_sinDv4_f(<4 x float>) readnone

declare <8 x float> @_Z8half_sinDv8_f(<8 x float>) readnone

declare <16 x float> @_Z8half_sinDv16_f(<16 x float>) readnone

declare float @_Z9half_sqrtf(float) readnone

declare <4 x float> @_Z9half_sqrtDv4_f(<4 x float>) readnone

declare <8 x float> @_Z9half_sqrtDv8_f(<8 x float>) readnone

declare <16 x float> @_Z9half_sqrtDv16_f(<16 x float>) readnone

declare float @_Z8half_tanf(float) readnone

declare <4 x float> @_Z8half_tanDv4_f(<4 x float>) readnone

declare <8 x float> @_Z8half_tanDv8_f(<8 x float>) readnone

declare <16 x float> @_Z8half_tanDv16_f(<16 x float>) readnone

declare float @_Z5asinhf(float) readnone

declare <4 x float> @_Z5asinhDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5asinhDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5asinhDv16_f(<16 x float>) readnone

declare float @_Z5acoshf(float) readnone

declare <4 x float> @_Z5acoshDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5acoshDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5acoshDv16_f(<16 x float>) readnone

declare float @_Z5atanhf(float) readnone

declare <4 x float> @_Z5atanhDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5atanhDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5atanhDv16_f(<16 x float>) readnone

declare float @_Z3minff(float, float) readnone

declare <4 x float> @_Z3minDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z3minDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z3minDv16_fS_(<16 x float>, <16 x float>) readnone

declare <4 x float> @_Z3minDv4_ff(<4 x float>, float) readnone

declare <8 x float> @_Z3minDv8_ff(<8 x float>, float) readnone

declare <16 x float> @_Z3minDv16_ff(<16 x float>, float) readnone

declare float @_Z3maxff(float, float) readnone

declare <4 x float> @_Z3maxDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z3maxDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z3maxDv16_fS_(<16 x float>, <16 x float>) readnone

declare <4 x float> @_Z3maxDv4_ff(<4 x float>, float) readnone

declare <8 x float> @_Z3maxDv8_ff(<8 x float>, float) readnone

declare <16 x float> @_Z3maxDv16_ff(<16 x float>, float) readnone

declare float @_Z5hypotff(float, float) readnone

declare <4 x float> @_Z5hypotDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z5hypotDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z5hypotDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z4stepff(float, float) readnone

declare <4 x float> @_Z4stepDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z4stepDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z4stepDv16_fS_(<16 x float>, <16 x float>) readnone

declare <4 x float> @_Z4stepfDv4_f(float, <4 x float>) readnone

declare <8 x float> @_Z4stepfDv8_f(float, <8 x float>) readnone

declare <16 x float> @_Z4stepfDv16_f(float, <16 x float>) readnone

declare float @_Z10smoothstepfff(float, float, float) readnone

declare <4 x float> @_Z10smoothstepDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) readnone

declare <8 x float> @_Z10smoothstepDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) readnone

declare <16 x float> @_Z10smoothstepDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) readnone

declare <4 x float> @_Z10smoothstepffDv4_f(float, float, <4 x float>) readnone

declare <8 x float> @_Z10smoothstepffDv8_f(float, float, <8 x float>) readnone

declare <16 x float> @_Z10smoothstepffDv16_f(float, float, <16 x float>) readnone

declare float @_Z7radiansf(float) readnone

declare <4 x float> @_Z7radiansDv4_f(<4 x float>) readnone

declare <8 x float> @_Z7radiansDv8_f(<8 x float>) readnone

declare <16 x float> @_Z7radiansDv16_f(<16 x float>) readnone

declare float @_Z7degreesf(float) readnone

declare <4 x float> @_Z7degreesDv4_f(<4 x float>) readnone

declare <8 x float> @_Z7degreesDv8_f(<8 x float>) readnone

declare <16 x float> @_Z7degreesDv16_f(<16 x float>) readnone

declare float @_Z4signf(float) readnone

declare <4 x float> @_Z4signDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4signDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4signDv16_f(<16 x float>) readnone

declare float @_Z5floorf(float) readnone

declare <4 x float> @_Z5floorDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5floorDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5floorDv16_f(<16 x float>) readnone

declare float @_Z3dotff(float, float) readnone

declare float @_Z3dotDv4_fS_(<4 x float>, <4 x float>) readnone

declare float @_Z3mixfff(float, float, float) readnone

declare <4 x float> @_Z3mixDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) readnone

declare <4 x float> @_Z3mixDv4_fS_f(<4 x float>, <4 x float>, float) readnone

declare <8 x float> @_Z3mixDv8_fS_f(<8 x float>, <8 x float>, float) readnone

declare <16 x float> @_Z3mixDv16_fS_f(<16 x float>, <16 x float>, float) readnone

declare <8 x float> @_Z3mixDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) readnone

declare <16 x float> @_Z3mixDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) readnone

declare float @_Z9normalizef(float) readnone

declare <4 x float> @_Z9normalizeDv4_f(<4 x float>) readnone

declare float @_Z14fast_normalizef(float) readnone

declare <4 x float> @_Z14fast_normalizeDv4_f(<4 x float>) readnone

declare <4 x float> @_Z5crossDv4_fS_(<4 x float>, <4 x float>) readnone

declare float @_Z6lengthf(float) readnone

declare float @_Z6lengthDv2_f(<2 x float>) readnone

declare float @_Z6lengthDv4_f(<4 x float>) readnone

declare float @_Z11fast_lengthf(float) readnone

declare float @_Z11fast_lengthDv2_f(<2 x float>) readnone

declare float @_Z11fast_lengthDv4_f(<4 x float>) readnone

declare float @_Z8distanceff(float, float) readnone

declare float @_Z8distanceDv2_fS_(<2 x float>, <2 x float>) readnone

declare float @_Z8distanceDv4_fS_(<4 x float>, <4 x float>) readnone

declare float @_Z13fast_distanceff(float, float) readnone

declare float @_Z13fast_distanceDv2_fS_(<2 x float>, <2 x float>) readnone

declare float @_Z13fast_distanceDv4_fS_(<4 x float>, <4 x float>) readnone

declare float @_Z13convert_floati(i32) readnone

declare <4 x float> @_Z14convert_float4Dv4_i(<4 x i32>) readnone

declare <8 x float> @_Z14convert_float8Dv8_i(<8 x i32>) readnone

declare <16 x float> @_Z15convert_float16Dv16_i(<16 x i32>) readnone

declare float @_Z13convert_floatj(i32) readnone

declare <4 x float> @_Z14convert_float4Dv4_j(<4 x i32>) readnone

declare <8 x float> @_Z14convert_float8Dv8_j(<8 x i32>) readnone

declare <16 x float> @_Z15convert_float16Dv16_j(<16 x i32>) readnone

declare float @_Z5rootnfi(float, i32) readnone

declare <4 x float> @_Z5rootnDv4_fDv4_i(<4 x float>, <4 x i32>) readnone

declare <8 x float> @_Z5rootnDv8_fDv8_i(<8 x float>, <8 x i32>) readnone

declare <16 x float> @_Z5rootnDv16_fDv16_i(<16 x float>, <16 x i32>) readnone

declare float @_Z5ldexpfi(float, i32) readnone

declare <4 x float> @_Z5ldexpDv4_fDv4_i(<4 x float>, <4 x i32>) readnone

declare <8 x float> @_Z5ldexpDv8_fDv8_i(<8 x float>, <8 x i32>) readnone

declare <16 x float> @_Z5ldexpDv16_fDv16_i(<16 x float>, <16 x i32>) readnone

declare <4 x float> @_Z5ldexpDv4_fi(<4 x float>, i32) readnone

declare <8 x float> @_Z5ldexpDv8_fi(<8 x float>, i32) readnone

declare <16 x float> @_Z5ldexpDv16_fi(<16 x float>, i32) readnone

declare float @_Z4modffPf(float, float*)

declare <4 x float> @_Z4modfDv4_fPS_(<4 x float>, <4 x float>*)

declare <8 x float> @_Z4modfDv8_fPS_(<8 x float>, <8 x float>*)

declare <16 x float> @_Z4modfDv16_fPS_(<16 x float>, <16 x float>*)

declare float @_Z5frexpfPi(float, i32*)

declare <4 x float> @_Z5frexpDv4_fPDv4_i(<4 x float>, <4 x i32>*)

declare <8 x float> @_Z5frexpDv8_fPDv8_i(<8 x float>, <8 x i32>*)

declare <16 x float> @_Z5frexpDv16_fPDv16_i(<16 x float>, <16 x i32>*)

declare float @_Z6maxmagff(float, float) readnone

declare <4 x float> @_Z6maxmagDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z6maxmagDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z6maxmagDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z6minmagff(float, float) readnone

declare <4 x float> @_Z6minmagDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z6minmagDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z6minmagDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z8copysignff(float, float) readnone

declare <4 x float> @_Z8copysignDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z8copysignDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z8copysignDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z9nextafterff(float, float) readnone

declare <4 x float> @_Z9nextafterDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z9nextafterDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z9nextafterDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z4fdimff(float, float) readnone

declare <4 x float> @_Z4fdimDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z4fdimDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z4fdimDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z3fmafff(float, float, float) readnone

declare <4 x float> @_Z3fmaDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) readnone

declare <8 x float> @_Z3fmaDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) readnone

declare <16 x float> @_Z3fmaDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) readnone

declare float @_Z3madfff(float, float, float) readnone

declare <4 x float> @_Z3madDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) readnone

declare <8 x float> @_Z3madDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) readnone

declare <16 x float> @_Z3madDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) readnone

declare float @_Z4rintf(float) readnone

declare <4 x float> @_Z4rintDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4rintDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4rintDv16_f(<16 x float>) readnone

declare float @_Z5roundf(float) readnone

declare <4 x float> @_Z5roundDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5roundDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5roundDv16_f(<16 x float>) readnone

declare float @_Z5truncf(float) readnone

declare <4 x float> @_Z5truncDv4_f(<4 x float>) readnone

declare <8 x float> @_Z5truncDv8_f(<8 x float>) readnone

declare <16 x float> @_Z5truncDv16_f(<16 x float>) readnone

declare float @_Z4cbrtf(float) readnone

declare <4 x float> @_Z4cbrtDv4_f(<4 x float>) readnone

declare <8 x float> @_Z4cbrtDv8_f(<8 x float>) readnone

declare <16 x float> @_Z4cbrtDv16_f(<16 x float>) readnone

declare float @_Z4powrff(float, float) readnone

declare <4 x float> @_Z4powrDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z4powrDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z4powrDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z4fmodff(float, float) readnone

declare <4 x float> @_Z4fmodDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z4fmodDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z4fmodDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z4fminff(float, float) readnone

declare <4 x float> @_Z4fminDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z4fminDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z4fminDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z4fmaxff(float, float) readnone

declare <4 x float> @_Z4fmaxDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z4fmaxDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z4fmaxDv16_fS_(<16 x float>, <16 x float>) readnone

declare <4 x float> @_Z4fminDv4_ff(<4 x float>, float) readnone

declare <8 x float> @_Z4fminDv8_ff(<8 x float>, float) readnone

declare <16 x float> @_Z4fminDv16_ff(<16 x float>, float) readnone

declare <4 x float> @_Z4fmaxDv4_ff(<4 x float>, float) readnone

declare <8 x float> @_Z4fmaxDv8_ff(<8 x float>, float) readnone

declare <16 x float> @_Z4fmaxDv16_ff(<16 x float>, float) readnone

declare <4 x float> @_Z4pownDv4_fDv4_i(<4 x float>, <4 x i32>) readnone

declare <8 x float> @_Z4pownDv8_fDv8_i(<8 x float>, <8 x i32>) readnone

declare <16 x float> @_Z4pownDv16_fDv16_i(<16 x float>, <16 x i32>) readnone

declare i32 @_Z5ilogbf(float) readnone

declare <4 x i32> @_Z5ilogbDv4_f(<4 x float>) readnone

declare <8 x i32> @_Z5ilogbDv8_f(<8 x float>) readnone

declare <16 x i32> @_Z5ilogbDv16_f(<16 x float>) readnone

declare float @_Z3nanj(i32) readnone

declare <4 x float> @_Z3nanDv4_j(<4 x i32>) readnone

declare <8 x float> @_Z3nanDv8_j(<8 x i32>) readnone

declare <16 x float> @_Z3nanDv16_j(<16 x i32>) readnone

declare float @_Z5fractfPf(float, float*)

declare <4 x float> @_Z5fractDv4_fPS_(<4 x float>, <4 x float>*)

declare <8 x float> @_Z5fractDv8_fPS_(<8 x float>, <8 x float>*)

declare <16 x float> @_Z5fractDv16_fPS_(<16 x float>, <16 x float>*)

declare float @_Z6lgammaf(float) readnone

declare <4 x float> @_Z6lgammaDv4_f(<4 x float>) readnone

declare <8 x float> @_Z6lgammaDv8_f(<8 x float>) readnone

declare <16 x float> @_Z6lgammaDv16_f(<16 x float>) readnone

declare float @_Z8lgamma_rfPi(float, i32*)

declare <4 x float> @_Z8lgamma_rDv4_fPDv4_i(<4 x float>, <4 x i32>*)

declare <8 x float> @_Z8lgamma_rDv8_fPDv8_i(<8 x float>, <8 x i32>*)

declare <16 x float> @_Z8lgamma_rDv16_fPDv16_i(<16 x float>, <16 x i32>*)

declare float @_Z9bitselectfff(float, float, float) readnone

declare <4 x float> @_Z9bitselectDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>) readnone

declare <8 x float> @_Z9bitselectDv8_fS_S_(<8 x float>, <8 x float>, <8 x float>) readnone

declare <16 x float> @_Z9bitselectDv16_fS_S_(<16 x float>, <16 x float>, <16 x float>) readnone

declare float @_Z6selectffc(float, float, i8 signext) readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_c(<4 x float>, <4 x float>, <4 x i8>) readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_c(<8 x float>, <8 x float>, <8 x i8>) readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_c(<16 x float>, <16 x float>, <16 x i8>) readnone

declare float @_Z6selectffh(float, float, i8 zeroext) readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_h(<4 x float>, <4 x float>, <4 x i8>) readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_h(<8 x float>, <8 x float>, <8 x i8>) readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_h(<16 x float>, <16 x float>, <16 x i8>) readnone

declare float @_Z6selectffs(float, float, i16 signext) readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_s(<4 x float>, <4 x float>, <4 x i16>) readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_s(<8 x float>, <8 x float>, <8 x i16>) readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_s(<16 x float>, <16 x float>, <16 x i16>) readnone

declare float @_Z6selectfft(float, float, i16 zeroext) readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_t(<4 x float>, <4 x float>, <4 x i16>) readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_t(<8 x float>, <8 x float>, <8 x i16>) readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_t(<16 x float>, <16 x float>, <16 x i16>) readnone

declare float @_Z6selectffi(float, float, i32) readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_i(<4 x float>, <4 x float>, <4 x i32>) readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_i(<8 x float>, <8 x float>, <8 x i32>) readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_i(<16 x float>, <16 x float>, <16 x i32>) readnone

declare float @_Z6selectffj(float, float, i32) readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_j(<4 x float>, <4 x float>, <4 x i32>) readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_j(<8 x float>, <8 x float>, <8 x i32>) readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_j(<16 x float>, <16 x float>, <16 x i32>) readnone

declare float @_Z6selectffl(float, float, i64) readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_l(<4 x float>, <4 x float>, <4 x i64>) readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_l(<8 x float>, <8 x float>, <8 x i64>) readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_l(<16 x float>, <16 x float>, <16 x i64>) readnone

declare float @_Z6selectffm(float, float, i64) readnone

declare <4 x float> @_Z6selectDv4_fS_Dv4_m(<4 x float>, <4 x float>, <4 x i64>) readnone

declare <8 x float> @_Z6selectDv8_fS_Dv8_m(<8 x float>, <8 x float>, <8 x i64>) readnone

declare <16 x float> @_Z6selectDv16_fS_Dv16_m(<16 x float>, <16 x float>, <16 x i64>) readnone

declare float @_Z9remainderff(float, float) readnone

declare <4 x float> @_Z9remainderDv4_fS_(<4 x float>, <4 x float>) readnone

declare <8 x float> @_Z9remainderDv8_fS_(<8 x float>, <8 x float>) readnone

declare <16 x float> @_Z9remainderDv16_fS_(<16 x float>, <16 x float>) readnone

declare float @_Z6remquoffPi(float, float, i32*)

declare <2 x float> @_Z6remquoDv2_fS_PDv2_i(<2 x float>, <2 x float>, <2 x i32>*)

declare <3 x float> @_Z6remquoDv3_fS_PDv3_i(<3 x float>, <3 x float>, <3 x i32>*)

declare <4 x float> @_Z6remquoDv4_fS_PDv4_i(<4 x float>, <4 x float>, <4 x i32>*)

declare <8 x float> @_Z6remquoDv8_fS_PDv8_i(<8 x float>, <8 x float>, <8 x i32>*)

declare <16 x float> @_Z6remquoDv16_fS_PDv16_i(<16 x float>, <16 x float>, <16 x i32>*)

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32)* @oclbuiltin, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, uint const", metadata !"opencl_oclbuiltin_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1, i32 1, i32 1, i32 1, i32 0}
!3 = metadata !{i32 3, i32 3, i32 3, i32 3, i32 3}
!4 = metadata !{metadata !"float*", metadata !"float*", metadata !"int*", metadata !"int*", metadata !"uint const"}
!5 = metadata !{metadata !"input", metadata !"output", metadata !"inputInt", metadata !"outputInt", metadata !"buffer_size"}
