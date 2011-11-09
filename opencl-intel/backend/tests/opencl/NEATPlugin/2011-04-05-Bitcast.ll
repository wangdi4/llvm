; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1

; ModuleID = '2011-04-05-Bitcast.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @BitcastTest(<4 x i32> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %0 = alloca <4 x i32> addrspace(1)*, align 4    ; <<4 x i32> addrspace(1)**> [#uses=2]
  %1 = alloca <4 x i32> addrspace(1)*, align 4    ; <<4 x i32> addrspace(1)**> [#uses=2]
  %2 = alloca <4 x float> addrspace(1)*, align 4  ; <<4 x float> addrspace(1)**> [#uses=2]
  %3 = alloca i32, align 4                        ; <i32*> [#uses=1]
  %tid = alloca i32, align 4                      ; <i32*> [#uses=3]
  %tmp = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %a3 = alloca <16 x i16>, align 32               ; <<16 x i16>*> [#uses=1]
  %a4 = alloca <8 x i32>, align 32                ; <<8 x i32>*> [#uses=1]
  %a5 = alloca <8 x i32>, align 32                ; <<8 x i32>*> [#uses=1]
  %a6 = alloca <16 x i16>, align 32               ; <<16 x i16>*> [#uses=1]
  %a1 = alloca <4 x float>, align 16              ; <<4 x float>*> [#uses=1]
  %pf1 = alloca float*, align 4                   ; <float**> [#uses=1]
  %a2 = alloca <4 x i32>, align 16                ; <<4 x i32>*> [#uses=1]
  %pf2 = alloca i32*, align 4                     ; <i32**> [#uses=1]
  %pf3 = alloca i8*, align 4                      ; <i8**> [#uses=0]
  %pf4 = alloca i16*, align 4                     ; <i16**> [#uses=0]
  store <4 x i32> addrspace(1)* %input, <4 x i32> addrspace(1)** %1
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %2
  store i32 %buffer_size, i32* %3
  %4 = call i32 @get_global_id(i32 0)             ; <i32> [#uses=1]
  store i32 %4, i32* %tid
  %5 = load i32* %tid                             ; <i32> [#uses=1]
  %6 = load <4 x i32> addrspace(1)** %1           ; <<4 x i32> addrspace(1)*> [#uses=1]
  %7 = getelementptr inbounds <4 x i32> addrspace(1)* %6, i32 %5 ; <<4 x i32> addrspace(1)*> [#uses=1]
  %8 = load <4 x i32> addrspace(1)* %7            ; <<4 x i32>> [#uses=1]
  %9 = bitcast <4 x i32> %8 to <4 x float>        ; <<4 x float>> [#uses=1]
  store <4 x float> %9, <4 x float>* %tmp
  %10 = load <4 x float>* %tmp                    ; <<4 x float>> [#uses=1]
  %11 = load i32* %tid                            ; <i32> [#uses=1]
  %12 = load <4 x float> addrspace(1)** %2        ; <<4 x float> addrspace(1)*> [#uses=1]
  %13 = getelementptr inbounds <4 x float> addrspace(1)* %12, i32 %11 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %10, <4 x float> addrspace(1)* %13
  %14 = load <16 x i16>* %a3                      ; <<16 x i16>> [#uses=1]
  %15 = bitcast <16 x i16> %14 to <8 x i32>       ; <<8 x i32>> [#uses=1]
  store <8 x i32> %15, <8 x i32>* %a4
  %16 = load <8 x i32>* %a5                       ; <<8 x i32>> [#uses=1]
  %17 = bitcast <8 x i32> %16 to <16 x i16>       ; <<16 x i16>> [#uses=1]
  store <16 x i16> %17, <16 x i16>* %a6
  %18 = bitcast <4 x float>* %a1 to float*        ; <float*> [#uses=1]
  store float* %18, float** %pf1
  %19 = bitcast <4 x i32>* %a2 to i32*            ; <i32*> [#uses=1]
  store i32* %19, i32** %pf2

  %aa1 = bitcast float* %18 to <4 x float>*
  %aa2 = bitcast float* %18 to float*

  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x i32> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @BitcastTest, metadata !1, metadata !1, metadata !"", metadata !"<4 x i32> __attribute__((address_space(1))) *, <4 x float> __attribute__((address_space(1))) *, uint const", metadata !"opencl_bitcast_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
