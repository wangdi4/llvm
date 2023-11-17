; ModuleID = '2011-04-05-Bitcast.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @BitcastTest(ptr addrspace(1) %input, ptr addrspace(1) %output, i32 %buffer_size) nounwind {
entry:
  %0 = alloca ptr addrspace(1), align 4                                     ; <ptr addrspace(1)> [#uses=2]
  %1 = alloca ptr addrspace(1), align 4                                     ; <ptr addrspace(1)> [#uses=2]
  %2 = alloca ptr addrspace(1), align 4                                     ; <ptr addrspace(1)> [#uses=2]
  %3 = alloca i32, align 4                                                  ; <i32*> [#uses=1]
  %tid = alloca i32, align 4                                                ; <i32*> [#uses=3]
  %tmp = alloca <4 x float>, align 16                                       ; <<4 x float>*> [#uses=2]
  %a3 = alloca <16 x i16>, align 32                                         ; <<16 x i16>*> [#uses=1]
  %a4 = alloca <8 x i32>, align 32                                          ; <<8 x i32>*> [#uses=1]
  %a5 = alloca <8 x i32>, align 32                                          ; <<8 x i32>*> [#uses=1]
  %a6 = alloca <16 x i16>, align 32                                         ; <<16 x i16>*> [#uses=1]
  %a1 = alloca <4 x float>, align 16                                        ; <<4 x float>*> [#uses=1]
  %pf1 = alloca ptr, align 4                                                ; <ptr> [#uses=1]
  %a2 = alloca <4 x i32>, align 16                                          ; <<4 x i32>*> [#uses=1]
  %pf2 = alloca ptr, align 4                                                ; <ptr> [#uses=1]
  %pf3 = alloca ptr, align 4                                                ; <ptr> [#uses=0]
  %pf4 = alloca ptr, align 4                                                ; <ptr> [#uses=0]
  store ptr addrspace(1) %input, ptr %1
  store ptr addrspace(1) %output, ptr %2
  store i32 %buffer_size, ptr %3
  %4 = call i32 @_Z13get_global_idj(i32 0)                                  ; <i32> [#uses=1]
  store i32 %4, ptr %tid
  %5 = load i32, ptr %tid                                                   ; <i32> [#uses=1]
  %6 = load ptr addrspace(1), ptr %1                                        ; <ptr addrspace(1)> [#uses=1]
  %7 = getelementptr inbounds <4 x i32>, ptr addrspace(1) %6, i32 %5        ; <ptr addrspace(1)> [#uses=1]
  %8 = load <4 x i32>, ptr addrspace(1) %7                                  ; <<4 x i32>> [#uses=1]
  %9 = bitcast <4 x i32> %8 to <4 x float>                                  ; <<4 x float>> [#uses=1]
  store <4 x float> %9, ptr %tmp
  %10 = load <4 x float>, ptr %tmp                                          ; <<4 x float>> [#uses=1]
  %11 = load i32, ptr %tid                                                  ; <i32> [#uses=1]
  %12 = load ptr addrspace(1), ptr %2                                       ; <ptr addrspace(1)> [#uses=1]
  %13 = getelementptr inbounds <4 x float>, ptr addrspace(1) %12, i32 %11   ; <ptr addrspace(1)> [#uses=1]
  store <4 x float> %10, ptr addrspace(1) %13
  %14 = load <16 x i16>, ptr %a3                                            ; <<16 x i16>> [#uses=1]
  %15 = bitcast <16 x i16> %14 to <8 x i32>                                 ; <<8 x i32>> [#uses=1]
  store <8 x i32> %15, ptr %a4
  %16 = load <8 x i32>, ptr %a5                                             ; <<8 x i32>> [#uses=1]
  %17 = bitcast <8 x i32> %16 to <16 x i16>                                 ; <<16 x i16>> [#uses=1]
  store <16 x i16> %17, ptr %a6
  %18 = bitcast ptr %a1 to ptr                                              ; <ptr> [#uses=1]
  store ptr %18, ptr %pf1
  %19 = bitcast ptr %a2 to ptr                                              ; <ptr> [#uses=1]
  store ptr %19, ptr %pf2

  %aa1 = bitcast ptr %18 to ptr
  %aa2 = bitcast ptr %18 to ptr

  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @BitcastTest}
