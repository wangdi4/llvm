; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlNVMedian.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@ckMedian.fMedianEstimate = internal constant [3 x float] [float 1.280000e+002, float 1.280000e+002, float 1.280000e+002], align 4 ; <[3 x float]*> [#uses=1]
@ckMedian.fMaxBound = internal constant [3 x float] [float 2.550000e+002, float 2.550000e+002, float 2.550000e+002], align 4 ; <[3 x float]*> [#uses=1]
@opencl_ckMedian_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_ckMedian_parameters = appending global [152 x i8] c"uchar4 __attribute__((address_space(1))) *, unsigned int __attribute__((address_space(1))) *, uchar4 __attribute__((address_space(3))) *, int, int, int\00", section "llvm.metadata" ; <[152 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x i8> addrspace(1)*, i32 addrspace(1)*, <4 x i8> addrspace(3)*, i32, i32, i32)* @ckMedian to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_ckMedian_locals to i8*), i8* getelementptr inbounds ([152 x i8]* @opencl_ckMedian_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @ckMedian(<4 x i8> addrspace(1)* %uc4Source, i32 addrspace(1)* %uiDest, <4 x i8> addrspace(3)* %uc4LocalData, i32 %iLocalPixPitch, i32 %uiImageWidth, i32 %uiDevImageHeight) nounwind {
entry:
  %uc4Source.addr = alloca <4 x i8> addrspace(1)*, align 4 ; <<4 x i8> addrspace(1)**> [#uses=7]
  %uiDest.addr = alloca i32 addrspace(1)*, align 4 ; <i32 addrspace(1)**> [#uses=2]
  %uc4LocalData.addr = alloca <4 x i8> addrspace(3)*, align 4 ; <<4 x i8> addrspace(3)**> [#uses=40]
  %iLocalPixPitch.addr = alloca i32, align 4      ; <i32*> [#uses=10]
  %uiImageWidth.addr = alloca i32, align 4        ; <i32*> [#uses=6]
  %uiDevImageHeight.addr = alloca i32, align 4    ; <i32*> [#uses=8]
  %iImagePosX = alloca i32, align 4               ; <i32*> [#uses=5]
  %iDevYPrime = alloca i32, align 4               ; <i32*> [#uses=16]
  %iDevGMEMOffset = alloca i32, align 4           ; <i32*> [#uses=4]
  %iLocalPixOffset = alloca i32, align 4          ; <i32*> [#uses=59]
  %fMedianEstimate = alloca [3 x float], align 4  ; <[3 x float]*> [#uses=40]
  %fMinBound = alloca [3 x float], align 4        ; <[3 x float]*> [#uses=7]
  %fMaxBound = alloca [3 x float], align 4        ; <[3 x float]*> [#uses=7]
  %iSearch = alloca i32, align 4                  ; <i32*> [#uses=4]
  %uiHighCount = alloca [3 x i32], align 4        ; <[3 x i32]*> [#uses=31]
  %uiPackedPix = alloca i32, align 4              ; <i32*> [#uses=6]
  store <4 x i8> addrspace(1)* %uc4Source, <4 x i8> addrspace(1)** %uc4Source.addr
  store i32 addrspace(1)* %uiDest, i32 addrspace(1)** %uiDest.addr
  store <4 x i8> addrspace(3)* %uc4LocalData, <4 x i8> addrspace(3)** %uc4LocalData.addr
  store i32 %iLocalPixPitch, i32* %iLocalPixPitch.addr
  store i32 %uiImageWidth, i32* %uiImageWidth.addr
  store i32 %uiDevImageHeight, i32* %uiDevImageHeight.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %iImagePosX
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  %sub = sub i32 %call1, 1                        ; <i32> [#uses=1]
  store i32 %sub, i32* %iDevYPrime
  %tmp = load i32* %iDevYPrime                    ; <i32> [#uses=1]
  %call2 = call i32 @get_global_size(i32 0)       ; <i32> [#uses=1]
  %call3 = call i32 @_Z5mul24ii(i32 %tmp, i32 %call2) ; <i32> [#uses=1]
  %tmp4 = load i32* %iImagePosX                   ; <i32> [#uses=1]
  %add = add nsw i32 %call3, %tmp4                ; <i32> [#uses=1]
  store i32 %add, i32* %iDevGMEMOffset
  %call6 = call i32 @get_local_id(i32 1)          ; <i32> [#uses=1]
  %tmp7 = load i32* %iLocalPixPitch.addr          ; <i32> [#uses=1]
  %call8 = call i32 @_Z5mul24ii(i32 %call6, i32 %tmp7) ; <i32> [#uses=1]
  %call9 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  %add10 = add i32 %call8, %call9                 ; <i32> [#uses=1]
  %add11 = add i32 %add10, 1                      ; <i32> [#uses=1]
  store i32 %add11, i32* %iLocalPixOffset
  %tmp12 = load i32* %iDevYPrime                  ; <i32> [#uses=1]
  %cmp = icmp sge i32 %tmp12, 0                   ; <i1> [#uses=1]
  br i1 %cmp, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %entry
  %tmp13 = load i32* %iDevYPrime                  ; <i32> [#uses=1]
  %tmp14 = load i32* %uiDevImageHeight.addr       ; <i32> [#uses=1]
  %cmp15 = icmp slt i32 %tmp13, %tmp14            ; <i1> [#uses=1]
  br i1 %cmp15, label %land.lhs.true16, label %if.else

land.lhs.true16:                                  ; preds = %land.lhs.true
  %tmp17 = load i32* %iImagePosX                  ; <i32> [#uses=1]
  %tmp18 = load i32* %uiImageWidth.addr           ; <i32> [#uses=1]
  %cmp19 = icmp slt i32 %tmp17, %tmp18            ; <i1> [#uses=1]
  br i1 %cmp19, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true16
  %tmp20 = load i32* %iDevGMEMOffset              ; <i32> [#uses=1]
  %tmp21 = load <4 x i8> addrspace(1)** %uc4Source.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %tmp21, i32 %tmp20 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp22 = load <4 x i8> addrspace(1)* %arrayidx  ; <<4 x i8>> [#uses=1]
  %tmp23 = load i32* %iLocalPixOffset             ; <i32> [#uses=1]
  %tmp24 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx25 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp24, i32 %tmp23 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> %tmp22, <4 x i8> addrspace(3)* %arrayidx25
  br label %if.end

if.else:                                          ; preds = %land.lhs.true16, %land.lhs.true, %entry
  %tmp26 = load i32* %iLocalPixOffset             ; <i32> [#uses=1]
  %tmp27 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx28 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp27, i32 %tmp26 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx28
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %call29 = call i32 @get_local_id(i32 1)         ; <i32> [#uses=1]
  %cmp30 = icmp ult i32 %call29, 2                ; <i1> [#uses=1]
  br i1 %cmp30, label %if.then31, label %if.end63

if.then31:                                        ; preds = %if.end
  %call32 = call i32 @get_local_size(i32 1)       ; <i32> [#uses=1]
  %tmp33 = load i32* %iLocalPixPitch.addr         ; <i32> [#uses=1]
  %call34 = call i32 @_Z5mul24ii(i32 %call32, i32 %tmp33) ; <i32> [#uses=1]
  %tmp35 = load i32* %iLocalPixOffset             ; <i32> [#uses=1]
  %add36 = add nsw i32 %tmp35, %call34            ; <i32> [#uses=1]
  store i32 %add36, i32* %iLocalPixOffset
  %tmp37 = load i32* %iDevYPrime                  ; <i32> [#uses=1]
  %call38 = call i32 @get_local_size(i32 1)       ; <i32> [#uses=1]
  %add39 = add i32 %tmp37, %call38                ; <i32> [#uses=1]
  %tmp40 = load i32* %uiDevImageHeight.addr       ; <i32> [#uses=1]
  %cmp41 = icmp ult i32 %add39, %tmp40            ; <i1> [#uses=1]
  br i1 %cmp41, label %land.lhs.true42, label %if.else58

land.lhs.true42:                                  ; preds = %if.then31
  %tmp43 = load i32* %iImagePosX                  ; <i32> [#uses=1]
  %tmp44 = load i32* %uiImageWidth.addr           ; <i32> [#uses=1]
  %cmp45 = icmp slt i32 %tmp43, %tmp44            ; <i1> [#uses=1]
  br i1 %cmp45, label %if.then46, label %if.else58

if.then46:                                        ; preds = %land.lhs.true42
  %tmp47 = load i32* %iDevGMEMOffset              ; <i32> [#uses=1]
  %call48 = call i32 @get_local_size(i32 1)       ; <i32> [#uses=1]
  %call49 = call i32 @get_global_size(i32 0)      ; <i32> [#uses=1]
  %call50 = call i32 @_Z5mul24jj(i32 %call48, i32 %call49) ; <i32> [#uses=1]
  %add51 = add i32 %tmp47, %call50                ; <i32> [#uses=1]
  %tmp52 = load <4 x i8> addrspace(1)** %uc4Source.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx53 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp52, i32 %add51 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp54 = load <4 x i8> addrspace(1)* %arrayidx53 ; <<4 x i8>> [#uses=1]
  %tmp55 = load i32* %iLocalPixOffset             ; <i32> [#uses=1]
  %tmp56 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx57 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp56, i32 %tmp55 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> %tmp54, <4 x i8> addrspace(3)* %arrayidx57
  br label %if.end62

if.else58:                                        ; preds = %land.lhs.true42, %if.then31
  %tmp59 = load i32* %iLocalPixOffset             ; <i32> [#uses=1]
  %tmp60 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx61 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp60, i32 %tmp59 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx61
  br label %if.end62

if.end62:                                         ; preds = %if.else58, %if.then46
  br label %if.end63

if.end63:                                         ; preds = %if.end62, %if.end
  %call64 = call i32 @get_local_id(i32 0)         ; <i32> [#uses=1]
  %call65 = call i32 @get_local_size(i32 0)       ; <i32> [#uses=1]
  %sub66 = sub i32 %call65, 1                     ; <i32> [#uses=1]
  %cmp67 = icmp eq i32 %call64, %sub66            ; <i1> [#uses=1]
  br i1 %cmp67, label %if.then68, label %if.else140

if.then68:                                        ; preds = %if.end63
  %call69 = call i32 @get_local_id(i32 1)         ; <i32> [#uses=1]
  %tmp70 = load i32* %iLocalPixPitch.addr         ; <i32> [#uses=1]
  %call71 = call i32 @_Z5mul24ii(i32 %call69, i32 %tmp70) ; <i32> [#uses=1]
  store i32 %call71, i32* %iLocalPixOffset
  %tmp72 = load i32* %iDevYPrime                  ; <i32> [#uses=1]
  %cmp73 = icmp sge i32 %tmp72, 0                 ; <i1> [#uses=1]
  br i1 %cmp73, label %land.lhs.true74, label %if.else96

land.lhs.true74:                                  ; preds = %if.then68
  %tmp75 = load i32* %iDevYPrime                  ; <i32> [#uses=1]
  %tmp76 = load i32* %uiDevImageHeight.addr       ; <i32> [#uses=1]
  %cmp77 = icmp slt i32 %tmp75, %tmp76            ; <i1> [#uses=1]
  br i1 %cmp77, label %land.lhs.true78, label %if.else96

land.lhs.true78:                                  ; preds = %land.lhs.true74
  %call79 = call i32 @get_group_id(i32 0)         ; <i32> [#uses=1]
  %cmp80 = icmp ugt i32 %call79, 0                ; <i1> [#uses=1]
  br i1 %cmp80, label %if.then81, label %if.else96

if.then81:                                        ; preds = %land.lhs.true78
  %tmp82 = load i32* %iDevYPrime                  ; <i32> [#uses=1]
  %call83 = call i32 @get_global_size(i32 0)      ; <i32> [#uses=1]
  %call84 = call i32 @_Z5mul24ii(i32 %tmp82, i32 %call83) ; <i32> [#uses=1]
  %call85 = call i32 @get_group_id(i32 0)         ; <i32> [#uses=1]
  %call86 = call i32 @get_local_size(i32 0)       ; <i32> [#uses=1]
  %call87 = call i32 @_Z5mul24jj(i32 %call85, i32 %call86) ; <i32> [#uses=1]
  %add88 = add i32 %call84, %call87               ; <i32> [#uses=1]
  %sub89 = sub i32 %add88, 1                      ; <i32> [#uses=1]
  %tmp90 = load <4 x i8> addrspace(1)** %uc4Source.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx91 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp90, i32 %sub89 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp92 = load <4 x i8> addrspace(1)* %arrayidx91 ; <<4 x i8>> [#uses=1]
  %tmp93 = load i32* %iLocalPixOffset             ; <i32> [#uses=1]
  %tmp94 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx95 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp94, i32 %tmp93 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> %tmp92, <4 x i8> addrspace(3)* %arrayidx95
  br label %if.end100

if.else96:                                        ; preds = %land.lhs.true78, %land.lhs.true74, %if.then68
  %tmp97 = load i32* %iLocalPixOffset             ; <i32> [#uses=1]
  %tmp98 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx99 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp98, i32 %tmp97 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx99
  br label %if.end100

if.end100:                                        ; preds = %if.else96, %if.then81
  %call101 = call i32 @get_local_id(i32 1)        ; <i32> [#uses=1]
  %cmp102 = icmp ult i32 %call101, 2              ; <i1> [#uses=1]
  br i1 %cmp102, label %if.then103, label %if.end139

if.then103:                                       ; preds = %if.end100
  %call104 = call i32 @get_local_size(i32 1)      ; <i32> [#uses=1]
  %tmp105 = load i32* %iLocalPixPitch.addr        ; <i32> [#uses=1]
  %call106 = call i32 @_Z5mul24ii(i32 %call104, i32 %tmp105) ; <i32> [#uses=1]
  %tmp107 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %add108 = add nsw i32 %tmp107, %call106         ; <i32> [#uses=1]
  store i32 %add108, i32* %iLocalPixOffset
  %tmp109 = load i32* %iDevYPrime                 ; <i32> [#uses=1]
  %call110 = call i32 @get_local_size(i32 1)      ; <i32> [#uses=1]
  %add111 = add i32 %tmp109, %call110             ; <i32> [#uses=1]
  %tmp112 = load i32* %uiDevImageHeight.addr      ; <i32> [#uses=1]
  %cmp113 = icmp ult i32 %add111, %tmp112         ; <i1> [#uses=1]
  br i1 %cmp113, label %land.lhs.true114, label %if.else134

land.lhs.true114:                                 ; preds = %if.then103
  %call115 = call i32 @get_group_id(i32 0)        ; <i32> [#uses=1]
  %cmp116 = icmp ugt i32 %call115, 0              ; <i1> [#uses=1]
  br i1 %cmp116, label %if.then117, label %if.else134

if.then117:                                       ; preds = %land.lhs.true114
  %tmp118 = load i32* %iDevYPrime                 ; <i32> [#uses=1]
  %call119 = call i32 @get_local_size(i32 1)      ; <i32> [#uses=1]
  %add120 = add nsw i32 %tmp118, %call119         ; <i32> [#uses=1]
  %call121 = call i32 @get_global_size(i32 0)     ; <i32> [#uses=1]
  %call122 = call i32 @_Z5mul24ii(i32 %add120, i32 %call121) ; <i32> [#uses=1]
  %call123 = call i32 @get_group_id(i32 0)        ; <i32> [#uses=1]
  %call124 = call i32 @get_local_size(i32 0)      ; <i32> [#uses=1]
  %call125 = call i32 @_Z5mul24jj(i32 %call123, i32 %call124) ; <i32> [#uses=1]
  %add126 = add i32 %call122, %call125            ; <i32> [#uses=1]
  %sub127 = sub i32 %add126, 1                    ; <i32> [#uses=1]
  %tmp128 = load <4 x i8> addrspace(1)** %uc4Source.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx129 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp128, i32 %sub127 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp130 = load <4 x i8> addrspace(1)* %arrayidx129 ; <<4 x i8>> [#uses=1]
  %tmp131 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp132 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx133 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp132, i32 %tmp131 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> %tmp130, <4 x i8> addrspace(3)* %arrayidx133
  br label %if.end138

if.else134:                                       ; preds = %land.lhs.true114, %if.then103
  %tmp135 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp136 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx137 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp136, i32 %tmp135 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx137
  br label %if.end138

if.end138:                                        ; preds = %if.else134, %if.then117
  br label %if.end139

if.end139:                                        ; preds = %if.end138, %if.end100
  br label %if.end226

if.else140:                                       ; preds = %if.end63
  %call141 = call i32 @get_local_id(i32 0)        ; <i32> [#uses=1]
  %cmp142 = icmp eq i32 %call141, 0               ; <i1> [#uses=1]
  br i1 %cmp142, label %if.then143, label %if.end225

if.then143:                                       ; preds = %if.else140
  %call144 = call i32 @get_local_id(i32 1)        ; <i32> [#uses=1]
  %add145 = add nsw i32 %call144, 1               ; <i32> [#uses=1]
  %tmp146 = load i32* %iLocalPixPitch.addr        ; <i32> [#uses=1]
  %call147 = call i32 @_Z5mul24ii(i32 %add145, i32 %tmp146) ; <i32> [#uses=1]
  %sub148 = sub i32 %call147, 1                   ; <i32> [#uses=1]
  store i32 %sub148, i32* %iLocalPixOffset
  %tmp149 = load i32* %iDevYPrime                 ; <i32> [#uses=1]
  %cmp150 = icmp sge i32 %tmp149, 0               ; <i1> [#uses=1]
  br i1 %cmp150, label %land.lhs.true151, label %if.else177

land.lhs.true151:                                 ; preds = %if.then143
  %tmp152 = load i32* %iDevYPrime                 ; <i32> [#uses=1]
  %tmp153 = load i32* %uiDevImageHeight.addr      ; <i32> [#uses=1]
  %cmp154 = icmp slt i32 %tmp152, %tmp153         ; <i1> [#uses=1]
  br i1 %cmp154, label %land.lhs.true155, label %if.else177

land.lhs.true155:                                 ; preds = %land.lhs.true151
  %call156 = call i32 @get_group_id(i32 0)        ; <i32> [#uses=1]
  %add157 = add nsw i32 %call156, 1               ; <i32> [#uses=1]
  %call158 = call i32 @get_local_size(i32 0)      ; <i32> [#uses=1]
  %call159 = call i32 @_Z5mul24ii(i32 %add157, i32 %call158) ; <i32> [#uses=1]
  %tmp160 = load i32* %uiImageWidth.addr          ; <i32> [#uses=1]
  %cmp161 = icmp slt i32 %call159, %tmp160        ; <i1> [#uses=1]
  br i1 %cmp161, label %if.then162, label %if.else177

if.then162:                                       ; preds = %land.lhs.true155
  %tmp163 = load i32* %iDevYPrime                 ; <i32> [#uses=1]
  %call164 = call i32 @get_global_size(i32 0)     ; <i32> [#uses=1]
  %call165 = call i32 @_Z5mul24ii(i32 %tmp163, i32 %call164) ; <i32> [#uses=1]
  %call166 = call i32 @get_group_id(i32 0)        ; <i32> [#uses=1]
  %add167 = add i32 %call166, 1                   ; <i32> [#uses=1]
  %call168 = call i32 @get_local_size(i32 0)      ; <i32> [#uses=1]
  %call169 = call i32 @_Z5mul24jj(i32 %add167, i32 %call168) ; <i32> [#uses=1]
  %add170 = add i32 %call165, %call169            ; <i32> [#uses=1]
  %tmp171 = load <4 x i8> addrspace(1)** %uc4Source.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx172 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp171, i32 %add170 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp173 = load <4 x i8> addrspace(1)* %arrayidx172 ; <<4 x i8>> [#uses=1]
  %tmp174 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp175 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx176 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp175, i32 %tmp174 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> %tmp173, <4 x i8> addrspace(3)* %arrayidx176
  br label %if.end181

if.else177:                                       ; preds = %land.lhs.true155, %land.lhs.true151, %if.then143
  %tmp178 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp179 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx180 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp179, i32 %tmp178 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx180
  br label %if.end181

if.end181:                                        ; preds = %if.else177, %if.then162
  %call182 = call i32 @get_local_id(i32 1)        ; <i32> [#uses=1]
  %cmp183 = icmp ult i32 %call182, 2              ; <i1> [#uses=1]
  br i1 %cmp183, label %if.then184, label %if.end224

if.then184:                                       ; preds = %if.end181
  %call185 = call i32 @get_local_size(i32 1)      ; <i32> [#uses=1]
  %tmp186 = load i32* %iLocalPixPitch.addr        ; <i32> [#uses=1]
  %call187 = call i32 @_Z5mul24ii(i32 %call185, i32 %tmp186) ; <i32> [#uses=1]
  %tmp188 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %add189 = add nsw i32 %tmp188, %call187         ; <i32> [#uses=1]
  store i32 %add189, i32* %iLocalPixOffset
  %tmp190 = load i32* %iDevYPrime                 ; <i32> [#uses=1]
  %call191 = call i32 @get_local_size(i32 1)      ; <i32> [#uses=1]
  %add192 = add i32 %tmp190, %call191             ; <i32> [#uses=1]
  %tmp193 = load i32* %uiDevImageHeight.addr      ; <i32> [#uses=1]
  %cmp194 = icmp ult i32 %add192, %tmp193         ; <i1> [#uses=1]
  br i1 %cmp194, label %land.lhs.true195, label %if.else219

land.lhs.true195:                                 ; preds = %if.then184
  %call196 = call i32 @get_group_id(i32 0)        ; <i32> [#uses=1]
  %add197 = add i32 %call196, 1                   ; <i32> [#uses=1]
  %call198 = call i32 @get_local_size(i32 0)      ; <i32> [#uses=1]
  %call199 = call i32 @_Z5mul24jj(i32 %add197, i32 %call198) ; <i32> [#uses=1]
  %tmp200 = load i32* %uiImageWidth.addr          ; <i32> [#uses=1]
  %cmp201 = icmp ult i32 %call199, %tmp200        ; <i1> [#uses=1]
  br i1 %cmp201, label %if.then202, label %if.else219

if.then202:                                       ; preds = %land.lhs.true195
  %tmp203 = load i32* %iDevYPrime                 ; <i32> [#uses=1]
  %call204 = call i32 @get_local_size(i32 1)      ; <i32> [#uses=1]
  %add205 = add nsw i32 %tmp203, %call204         ; <i32> [#uses=1]
  %call206 = call i32 @get_global_size(i32 0)     ; <i32> [#uses=1]
  %call207 = call i32 @_Z5mul24ii(i32 %add205, i32 %call206) ; <i32> [#uses=1]
  %call208 = call i32 @get_group_id(i32 0)        ; <i32> [#uses=1]
  %add209 = add i32 %call208, 1                   ; <i32> [#uses=1]
  %call210 = call i32 @get_local_size(i32 0)      ; <i32> [#uses=1]
  %call211 = call i32 @_Z5mul24jj(i32 %add209, i32 %call210) ; <i32> [#uses=1]
  %add212 = add i32 %call207, %call211            ; <i32> [#uses=1]
  %tmp213 = load <4 x i8> addrspace(1)** %uc4Source.addr ; <<4 x i8> addrspace(1)*> [#uses=1]
  %arrayidx214 = getelementptr inbounds <4 x i8> addrspace(1)* %tmp213, i32 %add212 ; <<4 x i8> addrspace(1)*> [#uses=1]
  %tmp215 = load <4 x i8> addrspace(1)* %arrayidx214 ; <<4 x i8>> [#uses=1]
  %tmp216 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp217 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx218 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp217, i32 %tmp216 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> %tmp215, <4 x i8> addrspace(3)* %arrayidx218
  br label %if.end223

if.else219:                                       ; preds = %land.lhs.true195, %if.then184
  %tmp220 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp221 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx222 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp221, i32 %tmp220 ; <<4 x i8> addrspace(3)*> [#uses=1]
  store <4 x i8> zeroinitializer, <4 x i8> addrspace(3)* %arrayidx222
  br label %if.end223

if.end223:                                        ; preds = %if.else219, %if.then202
  br label %if.end224

if.end224:                                        ; preds = %if.end223, %if.end181
  br label %if.end225

if.end225:                                        ; preds = %if.end224, %if.else140
  br label %if.end226

if.end226:                                        ; preds = %if.end225, %if.end139
  call void @barrier(i32 1)
  %tmp228 = bitcast [3 x float]* %fMedianEstimate to i8* ; <i8*> [#uses=1]
  call void @llvm.memcpy.i32(i8* %tmp228, i8* bitcast ([3 x float]* @ckMedian.fMedianEstimate to i8*), i32 12, i32 4)
  %tmp230 = bitcast [3 x float]* %fMinBound to i8* ; <i8*> [#uses=1]
  call void @llvm.memset.i32(i8* %tmp230, i8 0, i32 12, i32 4)
  %tmp232 = bitcast [3 x float]* %fMaxBound to i8* ; <i8*> [#uses=1]
  call void @llvm.memcpy.i32(i8* %tmp232, i8* bitcast ([3 x float]* @ckMedian.fMaxBound to i8*), i32 12, i32 4)
  store i32 0, i32* %iSearch
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end226
  %tmp234 = load i32* %iSearch                    ; <i32> [#uses=1]
  %cmp235 = icmp slt i32 %tmp234, 8               ; <i1> [#uses=1]
  br i1 %cmp235, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp237 = bitcast [3 x i32]* %uiHighCount to i8* ; <i8*> [#uses=1]
  call void @llvm.memset.i32(i8* %tmp237, i8 0, i32 12, i32 4)
  %call238 = call i32 @get_local_id(i32 1)        ; <i32> [#uses=1]
  %tmp239 = load i32* %iLocalPixPitch.addr        ; <i32> [#uses=1]
  %call240 = call i32 @_Z5mul24ii(i32 %call238, i32 %tmp239) ; <i32> [#uses=1]
  %call241 = call i32 @get_local_id(i32 0)        ; <i32> [#uses=1]
  %add242 = add i32 %call240, %call241            ; <i32> [#uses=1]
  store i32 %add242, i32* %iLocalPixOffset
  %arraydecay = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx243 = getelementptr inbounds float* %arraydecay, i32 0 ; <float*> [#uses=1]
  %tmp244 = load float* %arrayidx243              ; <float> [#uses=1]
  %tmp245 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp246 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx247 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp246, i32 %tmp245 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp248 = load <4 x i8> addrspace(3)* %arrayidx247 ; <<4 x i8>> [#uses=1]
  %tmp249 = extractelement <4 x i8> %tmp248, i32 0 ; <i8> [#uses=1]
  %conv = zext i8 %tmp249 to i32                  ; <i32> [#uses=1]
  %conv250 = sitofp i32 %conv to float            ; <float> [#uses=1]
  %cmp251 = fcmp olt float %tmp244, %conv250      ; <i1> [#uses=1]
  %conv252 = zext i1 %cmp251 to i32               ; <i32> [#uses=1]
  %arraydecay253 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx254 = getelementptr inbounds i32* %arraydecay253, i32 0 ; <i32*> [#uses=2]
  %tmp255 = load i32* %arrayidx254                ; <i32> [#uses=1]
  %add256 = add i32 %tmp255, %conv252             ; <i32> [#uses=1]
  store i32 %add256, i32* %arrayidx254
  %arraydecay257 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx258 = getelementptr inbounds float* %arraydecay257, i32 1 ; <float*> [#uses=1]
  %tmp259 = load float* %arrayidx258              ; <float> [#uses=1]
  %tmp260 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp261 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx262 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp261, i32 %tmp260 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp263 = load <4 x i8> addrspace(3)* %arrayidx262 ; <<4 x i8>> [#uses=1]
  %tmp264 = extractelement <4 x i8> %tmp263, i32 1 ; <i8> [#uses=1]
  %conv265 = zext i8 %tmp264 to i32               ; <i32> [#uses=1]
  %conv266 = sitofp i32 %conv265 to float         ; <float> [#uses=1]
  %cmp267 = fcmp olt float %tmp259, %conv266      ; <i1> [#uses=1]
  %conv268 = zext i1 %cmp267 to i32               ; <i32> [#uses=1]
  %arraydecay269 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx270 = getelementptr inbounds i32* %arraydecay269, i32 1 ; <i32*> [#uses=2]
  %tmp271 = load i32* %arrayidx270                ; <i32> [#uses=1]
  %add272 = add i32 %tmp271, %conv268             ; <i32> [#uses=1]
  store i32 %add272, i32* %arrayidx270
  %arraydecay273 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx274 = getelementptr inbounds float* %arraydecay273, i32 2 ; <float*> [#uses=1]
  %tmp275 = load float* %arrayidx274              ; <float> [#uses=1]
  %tmp276 = load i32* %iLocalPixOffset            ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp276, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %iLocalPixOffset
  %tmp277 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx278 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp277, i32 %tmp276 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp279 = load <4 x i8> addrspace(3)* %arrayidx278 ; <<4 x i8>> [#uses=1]
  %tmp280 = extractelement <4 x i8> %tmp279, i32 2 ; <i8> [#uses=1]
  %conv281 = zext i8 %tmp280 to i32               ; <i32> [#uses=1]
  %conv282 = sitofp i32 %conv281 to float         ; <float> [#uses=1]
  %cmp283 = fcmp olt float %tmp275, %conv282      ; <i1> [#uses=1]
  %conv284 = zext i1 %cmp283 to i32               ; <i32> [#uses=1]
  %arraydecay285 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx286 = getelementptr inbounds i32* %arraydecay285, i32 2 ; <i32*> [#uses=2]
  %tmp287 = load i32* %arrayidx286                ; <i32> [#uses=1]
  %add288 = add i32 %tmp287, %conv284             ; <i32> [#uses=1]
  store i32 %add288, i32* %arrayidx286
  %arraydecay289 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx290 = getelementptr inbounds float* %arraydecay289, i32 0 ; <float*> [#uses=1]
  %tmp291 = load float* %arrayidx290              ; <float> [#uses=1]
  %tmp292 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp293 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx294 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp293, i32 %tmp292 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp295 = load <4 x i8> addrspace(3)* %arrayidx294 ; <<4 x i8>> [#uses=1]
  %tmp296 = extractelement <4 x i8> %tmp295, i32 0 ; <i8> [#uses=1]
  %conv297 = zext i8 %tmp296 to i32               ; <i32> [#uses=1]
  %conv298 = sitofp i32 %conv297 to float         ; <float> [#uses=1]
  %cmp299 = fcmp olt float %tmp291, %conv298      ; <i1> [#uses=1]
  %conv300 = zext i1 %cmp299 to i32               ; <i32> [#uses=1]
  %arraydecay301 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx302 = getelementptr inbounds i32* %arraydecay301, i32 0 ; <i32*> [#uses=2]
  %tmp303 = load i32* %arrayidx302                ; <i32> [#uses=1]
  %add304 = add i32 %tmp303, %conv300             ; <i32> [#uses=1]
  store i32 %add304, i32* %arrayidx302
  %arraydecay305 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx306 = getelementptr inbounds float* %arraydecay305, i32 1 ; <float*> [#uses=1]
  %tmp307 = load float* %arrayidx306              ; <float> [#uses=1]
  %tmp308 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp309 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx310 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp309, i32 %tmp308 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp311 = load <4 x i8> addrspace(3)* %arrayidx310 ; <<4 x i8>> [#uses=1]
  %tmp312 = extractelement <4 x i8> %tmp311, i32 1 ; <i8> [#uses=1]
  %conv313 = zext i8 %tmp312 to i32               ; <i32> [#uses=1]
  %conv314 = sitofp i32 %conv313 to float         ; <float> [#uses=1]
  %cmp315 = fcmp olt float %tmp307, %conv314      ; <i1> [#uses=1]
  %conv316 = zext i1 %cmp315 to i32               ; <i32> [#uses=1]
  %arraydecay317 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx318 = getelementptr inbounds i32* %arraydecay317, i32 1 ; <i32*> [#uses=2]
  %tmp319 = load i32* %arrayidx318                ; <i32> [#uses=1]
  %add320 = add i32 %tmp319, %conv316             ; <i32> [#uses=1]
  store i32 %add320, i32* %arrayidx318
  %arraydecay321 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx322 = getelementptr inbounds float* %arraydecay321, i32 2 ; <float*> [#uses=1]
  %tmp323 = load float* %arrayidx322              ; <float> [#uses=1]
  %tmp324 = load i32* %iLocalPixOffset            ; <i32> [#uses=2]
  %inc325 = add nsw i32 %tmp324, 1                ; <i32> [#uses=1]
  store i32 %inc325, i32* %iLocalPixOffset
  %tmp326 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx327 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp326, i32 %tmp324 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp328 = load <4 x i8> addrspace(3)* %arrayidx327 ; <<4 x i8>> [#uses=1]
  %tmp329 = extractelement <4 x i8> %tmp328, i32 2 ; <i8> [#uses=1]
  %conv330 = zext i8 %tmp329 to i32               ; <i32> [#uses=1]
  %conv331 = sitofp i32 %conv330 to float         ; <float> [#uses=1]
  %cmp332 = fcmp olt float %tmp323, %conv331      ; <i1> [#uses=1]
  %conv333 = zext i1 %cmp332 to i32               ; <i32> [#uses=1]
  %arraydecay334 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx335 = getelementptr inbounds i32* %arraydecay334, i32 2 ; <i32*> [#uses=2]
  %tmp336 = load i32* %arrayidx335                ; <i32> [#uses=1]
  %add337 = add i32 %tmp336, %conv333             ; <i32> [#uses=1]
  store i32 %add337, i32* %arrayidx335
  %arraydecay338 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx339 = getelementptr inbounds float* %arraydecay338, i32 0 ; <float*> [#uses=1]
  %tmp340 = load float* %arrayidx339              ; <float> [#uses=1]
  %tmp341 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp342 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx343 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp342, i32 %tmp341 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp344 = load <4 x i8> addrspace(3)* %arrayidx343 ; <<4 x i8>> [#uses=1]
  %tmp345 = extractelement <4 x i8> %tmp344, i32 0 ; <i8> [#uses=1]
  %conv346 = zext i8 %tmp345 to i32               ; <i32> [#uses=1]
  %conv347 = sitofp i32 %conv346 to float         ; <float> [#uses=1]
  %cmp348 = fcmp olt float %tmp340, %conv347      ; <i1> [#uses=1]
  %conv349 = zext i1 %cmp348 to i32               ; <i32> [#uses=1]
  %arraydecay350 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx351 = getelementptr inbounds i32* %arraydecay350, i32 0 ; <i32*> [#uses=2]
  %tmp352 = load i32* %arrayidx351                ; <i32> [#uses=1]
  %add353 = add i32 %tmp352, %conv349             ; <i32> [#uses=1]
  store i32 %add353, i32* %arrayidx351
  %arraydecay354 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx355 = getelementptr inbounds float* %arraydecay354, i32 1 ; <float*> [#uses=1]
  %tmp356 = load float* %arrayidx355              ; <float> [#uses=1]
  %tmp357 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp358 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx359 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp358, i32 %tmp357 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp360 = load <4 x i8> addrspace(3)* %arrayidx359 ; <<4 x i8>> [#uses=1]
  %tmp361 = extractelement <4 x i8> %tmp360, i32 1 ; <i8> [#uses=1]
  %conv362 = zext i8 %tmp361 to i32               ; <i32> [#uses=1]
  %conv363 = sitofp i32 %conv362 to float         ; <float> [#uses=1]
  %cmp364 = fcmp olt float %tmp356, %conv363      ; <i1> [#uses=1]
  %conv365 = zext i1 %cmp364 to i32               ; <i32> [#uses=1]
  %arraydecay366 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx367 = getelementptr inbounds i32* %arraydecay366, i32 1 ; <i32*> [#uses=2]
  %tmp368 = load i32* %arrayidx367                ; <i32> [#uses=1]
  %add369 = add i32 %tmp368, %conv365             ; <i32> [#uses=1]
  store i32 %add369, i32* %arrayidx367
  %arraydecay370 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx371 = getelementptr inbounds float* %arraydecay370, i32 2 ; <float*> [#uses=1]
  %tmp372 = load float* %arrayidx371              ; <float> [#uses=1]
  %tmp373 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp374 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx375 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp374, i32 %tmp373 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp376 = load <4 x i8> addrspace(3)* %arrayidx375 ; <<4 x i8>> [#uses=1]
  %tmp377 = extractelement <4 x i8> %tmp376, i32 2 ; <i8> [#uses=1]
  %conv378 = zext i8 %tmp377 to i32               ; <i32> [#uses=1]
  %conv379 = sitofp i32 %conv378 to float         ; <float> [#uses=1]
  %cmp380 = fcmp olt float %tmp372, %conv379      ; <i1> [#uses=1]
  %conv381 = zext i1 %cmp380 to i32               ; <i32> [#uses=1]
  %arraydecay382 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx383 = getelementptr inbounds i32* %arraydecay382, i32 2 ; <i32*> [#uses=2]
  %tmp384 = load i32* %arrayidx383                ; <i32> [#uses=1]
  %add385 = add i32 %tmp384, %conv381             ; <i32> [#uses=1]
  store i32 %add385, i32* %arrayidx383
  %tmp386 = load i32* %iLocalPixPitch.addr        ; <i32> [#uses=1]
  %sub387 = sub i32 %tmp386, 2                    ; <i32> [#uses=1]
  %tmp388 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %add389 = add nsw i32 %tmp388, %sub387          ; <i32> [#uses=1]
  store i32 %add389, i32* %iLocalPixOffset
  %arraydecay390 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx391 = getelementptr inbounds float* %arraydecay390, i32 0 ; <float*> [#uses=1]
  %tmp392 = load float* %arrayidx391              ; <float> [#uses=1]
  %tmp393 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp394 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx395 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp394, i32 %tmp393 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp396 = load <4 x i8> addrspace(3)* %arrayidx395 ; <<4 x i8>> [#uses=1]
  %tmp397 = extractelement <4 x i8> %tmp396, i32 0 ; <i8> [#uses=1]
  %conv398 = zext i8 %tmp397 to i32               ; <i32> [#uses=1]
  %conv399 = sitofp i32 %conv398 to float         ; <float> [#uses=1]
  %cmp400 = fcmp olt float %tmp392, %conv399      ; <i1> [#uses=1]
  %conv401 = zext i1 %cmp400 to i32               ; <i32> [#uses=1]
  %arraydecay402 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx403 = getelementptr inbounds i32* %arraydecay402, i32 0 ; <i32*> [#uses=2]
  %tmp404 = load i32* %arrayidx403                ; <i32> [#uses=1]
  %add405 = add i32 %tmp404, %conv401             ; <i32> [#uses=1]
  store i32 %add405, i32* %arrayidx403
  %arraydecay406 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx407 = getelementptr inbounds float* %arraydecay406, i32 1 ; <float*> [#uses=1]
  %tmp408 = load float* %arrayidx407              ; <float> [#uses=1]
  %tmp409 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp410 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx411 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp410, i32 %tmp409 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp412 = load <4 x i8> addrspace(3)* %arrayidx411 ; <<4 x i8>> [#uses=1]
  %tmp413 = extractelement <4 x i8> %tmp412, i32 1 ; <i8> [#uses=1]
  %conv414 = zext i8 %tmp413 to i32               ; <i32> [#uses=1]
  %conv415 = sitofp i32 %conv414 to float         ; <float> [#uses=1]
  %cmp416 = fcmp olt float %tmp408, %conv415      ; <i1> [#uses=1]
  %conv417 = zext i1 %cmp416 to i32               ; <i32> [#uses=1]
  %arraydecay418 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx419 = getelementptr inbounds i32* %arraydecay418, i32 1 ; <i32*> [#uses=2]
  %tmp420 = load i32* %arrayidx419                ; <i32> [#uses=1]
  %add421 = add i32 %tmp420, %conv417             ; <i32> [#uses=1]
  store i32 %add421, i32* %arrayidx419
  %arraydecay422 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx423 = getelementptr inbounds float* %arraydecay422, i32 2 ; <float*> [#uses=1]
  %tmp424 = load float* %arrayidx423              ; <float> [#uses=1]
  %tmp425 = load i32* %iLocalPixOffset            ; <i32> [#uses=2]
  %inc426 = add nsw i32 %tmp425, 1                ; <i32> [#uses=1]
  store i32 %inc426, i32* %iLocalPixOffset
  %tmp427 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx428 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp427, i32 %tmp425 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp429 = load <4 x i8> addrspace(3)* %arrayidx428 ; <<4 x i8>> [#uses=1]
  %tmp430 = extractelement <4 x i8> %tmp429, i32 2 ; <i8> [#uses=1]
  %conv431 = zext i8 %tmp430 to i32               ; <i32> [#uses=1]
  %conv432 = sitofp i32 %conv431 to float         ; <float> [#uses=1]
  %cmp433 = fcmp olt float %tmp424, %conv432      ; <i1> [#uses=1]
  %conv434 = zext i1 %cmp433 to i32               ; <i32> [#uses=1]
  %arraydecay435 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx436 = getelementptr inbounds i32* %arraydecay435, i32 2 ; <i32*> [#uses=2]
  %tmp437 = load i32* %arrayidx436                ; <i32> [#uses=1]
  %add438 = add i32 %tmp437, %conv434             ; <i32> [#uses=1]
  store i32 %add438, i32* %arrayidx436
  %arraydecay439 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx440 = getelementptr inbounds float* %arraydecay439, i32 0 ; <float*> [#uses=1]
  %tmp441 = load float* %arrayidx440              ; <float> [#uses=1]
  %tmp442 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp443 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx444 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp443, i32 %tmp442 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp445 = load <4 x i8> addrspace(3)* %arrayidx444 ; <<4 x i8>> [#uses=1]
  %tmp446 = extractelement <4 x i8> %tmp445, i32 0 ; <i8> [#uses=1]
  %conv447 = zext i8 %tmp446 to i32               ; <i32> [#uses=1]
  %conv448 = sitofp i32 %conv447 to float         ; <float> [#uses=1]
  %cmp449 = fcmp olt float %tmp441, %conv448      ; <i1> [#uses=1]
  %conv450 = zext i1 %cmp449 to i32               ; <i32> [#uses=1]
  %arraydecay451 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx452 = getelementptr inbounds i32* %arraydecay451, i32 0 ; <i32*> [#uses=2]
  %tmp453 = load i32* %arrayidx452                ; <i32> [#uses=1]
  %add454 = add i32 %tmp453, %conv450             ; <i32> [#uses=1]
  store i32 %add454, i32* %arrayidx452
  %arraydecay455 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx456 = getelementptr inbounds float* %arraydecay455, i32 1 ; <float*> [#uses=1]
  %tmp457 = load float* %arrayidx456              ; <float> [#uses=1]
  %tmp458 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp459 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx460 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp459, i32 %tmp458 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp461 = load <4 x i8> addrspace(3)* %arrayidx460 ; <<4 x i8>> [#uses=1]
  %tmp462 = extractelement <4 x i8> %tmp461, i32 1 ; <i8> [#uses=1]
  %conv463 = zext i8 %tmp462 to i32               ; <i32> [#uses=1]
  %conv464 = sitofp i32 %conv463 to float         ; <float> [#uses=1]
  %cmp465 = fcmp olt float %tmp457, %conv464      ; <i1> [#uses=1]
  %conv466 = zext i1 %cmp465 to i32               ; <i32> [#uses=1]
  %arraydecay467 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx468 = getelementptr inbounds i32* %arraydecay467, i32 1 ; <i32*> [#uses=2]
  %tmp469 = load i32* %arrayidx468                ; <i32> [#uses=1]
  %add470 = add i32 %tmp469, %conv466             ; <i32> [#uses=1]
  store i32 %add470, i32* %arrayidx468
  %arraydecay471 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx472 = getelementptr inbounds float* %arraydecay471, i32 2 ; <float*> [#uses=1]
  %tmp473 = load float* %arrayidx472              ; <float> [#uses=1]
  %tmp474 = load i32* %iLocalPixOffset            ; <i32> [#uses=2]
  %inc475 = add nsw i32 %tmp474, 1                ; <i32> [#uses=1]
  store i32 %inc475, i32* %iLocalPixOffset
  %tmp476 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx477 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp476, i32 %tmp474 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp478 = load <4 x i8> addrspace(3)* %arrayidx477 ; <<4 x i8>> [#uses=1]
  %tmp479 = extractelement <4 x i8> %tmp478, i32 2 ; <i8> [#uses=1]
  %conv480 = zext i8 %tmp479 to i32               ; <i32> [#uses=1]
  %conv481 = sitofp i32 %conv480 to float         ; <float> [#uses=1]
  %cmp482 = fcmp olt float %tmp473, %conv481      ; <i1> [#uses=1]
  %conv483 = zext i1 %cmp482 to i32               ; <i32> [#uses=1]
  %arraydecay484 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx485 = getelementptr inbounds i32* %arraydecay484, i32 2 ; <i32*> [#uses=2]
  %tmp486 = load i32* %arrayidx485                ; <i32> [#uses=1]
  %add487 = add i32 %tmp486, %conv483             ; <i32> [#uses=1]
  store i32 %add487, i32* %arrayidx485
  %arraydecay488 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx489 = getelementptr inbounds float* %arraydecay488, i32 0 ; <float*> [#uses=1]
  %tmp490 = load float* %arrayidx489              ; <float> [#uses=1]
  %tmp491 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp492 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx493 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp492, i32 %tmp491 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp494 = load <4 x i8> addrspace(3)* %arrayidx493 ; <<4 x i8>> [#uses=1]
  %tmp495 = extractelement <4 x i8> %tmp494, i32 0 ; <i8> [#uses=1]
  %conv496 = zext i8 %tmp495 to i32               ; <i32> [#uses=1]
  %conv497 = sitofp i32 %conv496 to float         ; <float> [#uses=1]
  %cmp498 = fcmp olt float %tmp490, %conv497      ; <i1> [#uses=1]
  %conv499 = zext i1 %cmp498 to i32               ; <i32> [#uses=1]
  %arraydecay500 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx501 = getelementptr inbounds i32* %arraydecay500, i32 0 ; <i32*> [#uses=2]
  %tmp502 = load i32* %arrayidx501                ; <i32> [#uses=1]
  %add503 = add i32 %tmp502, %conv499             ; <i32> [#uses=1]
  store i32 %add503, i32* %arrayidx501
  %arraydecay504 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx505 = getelementptr inbounds float* %arraydecay504, i32 1 ; <float*> [#uses=1]
  %tmp506 = load float* %arrayidx505              ; <float> [#uses=1]
  %tmp507 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp508 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx509 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp508, i32 %tmp507 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp510 = load <4 x i8> addrspace(3)* %arrayidx509 ; <<4 x i8>> [#uses=1]
  %tmp511 = extractelement <4 x i8> %tmp510, i32 1 ; <i8> [#uses=1]
  %conv512 = zext i8 %tmp511 to i32               ; <i32> [#uses=1]
  %conv513 = sitofp i32 %conv512 to float         ; <float> [#uses=1]
  %cmp514 = fcmp olt float %tmp506, %conv513      ; <i1> [#uses=1]
  %conv515 = zext i1 %cmp514 to i32               ; <i32> [#uses=1]
  %arraydecay516 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx517 = getelementptr inbounds i32* %arraydecay516, i32 1 ; <i32*> [#uses=2]
  %tmp518 = load i32* %arrayidx517                ; <i32> [#uses=1]
  %add519 = add i32 %tmp518, %conv515             ; <i32> [#uses=1]
  store i32 %add519, i32* %arrayidx517
  %arraydecay520 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx521 = getelementptr inbounds float* %arraydecay520, i32 2 ; <float*> [#uses=1]
  %tmp522 = load float* %arrayidx521              ; <float> [#uses=1]
  %tmp523 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp524 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx525 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp524, i32 %tmp523 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp526 = load <4 x i8> addrspace(3)* %arrayidx525 ; <<4 x i8>> [#uses=1]
  %tmp527 = extractelement <4 x i8> %tmp526, i32 2 ; <i8> [#uses=1]
  %conv528 = zext i8 %tmp527 to i32               ; <i32> [#uses=1]
  %conv529 = sitofp i32 %conv528 to float         ; <float> [#uses=1]
  %cmp530 = fcmp olt float %tmp522, %conv529      ; <i1> [#uses=1]
  %conv531 = zext i1 %cmp530 to i32               ; <i32> [#uses=1]
  %arraydecay532 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx533 = getelementptr inbounds i32* %arraydecay532, i32 2 ; <i32*> [#uses=2]
  %tmp534 = load i32* %arrayidx533                ; <i32> [#uses=1]
  %add535 = add i32 %tmp534, %conv531             ; <i32> [#uses=1]
  store i32 %add535, i32* %arrayidx533
  %tmp536 = load i32* %iLocalPixPitch.addr        ; <i32> [#uses=1]
  %sub537 = sub i32 %tmp536, 2                    ; <i32> [#uses=1]
  %tmp538 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %add539 = add nsw i32 %tmp538, %sub537          ; <i32> [#uses=1]
  store i32 %add539, i32* %iLocalPixOffset
  %arraydecay540 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx541 = getelementptr inbounds float* %arraydecay540, i32 0 ; <float*> [#uses=1]
  %tmp542 = load float* %arrayidx541              ; <float> [#uses=1]
  %tmp543 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp544 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx545 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp544, i32 %tmp543 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp546 = load <4 x i8> addrspace(3)* %arrayidx545 ; <<4 x i8>> [#uses=1]
  %tmp547 = extractelement <4 x i8> %tmp546, i32 0 ; <i8> [#uses=1]
  %conv548 = zext i8 %tmp547 to i32               ; <i32> [#uses=1]
  %conv549 = sitofp i32 %conv548 to float         ; <float> [#uses=1]
  %cmp550 = fcmp olt float %tmp542, %conv549      ; <i1> [#uses=1]
  %conv551 = zext i1 %cmp550 to i32               ; <i32> [#uses=1]
  %arraydecay552 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx553 = getelementptr inbounds i32* %arraydecay552, i32 0 ; <i32*> [#uses=2]
  %tmp554 = load i32* %arrayidx553                ; <i32> [#uses=1]
  %add555 = add i32 %tmp554, %conv551             ; <i32> [#uses=1]
  store i32 %add555, i32* %arrayidx553
  %arraydecay556 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx557 = getelementptr inbounds float* %arraydecay556, i32 1 ; <float*> [#uses=1]
  %tmp558 = load float* %arrayidx557              ; <float> [#uses=1]
  %tmp559 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp560 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx561 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp560, i32 %tmp559 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp562 = load <4 x i8> addrspace(3)* %arrayidx561 ; <<4 x i8>> [#uses=1]
  %tmp563 = extractelement <4 x i8> %tmp562, i32 1 ; <i8> [#uses=1]
  %conv564 = zext i8 %tmp563 to i32               ; <i32> [#uses=1]
  %conv565 = sitofp i32 %conv564 to float         ; <float> [#uses=1]
  %cmp566 = fcmp olt float %tmp558, %conv565      ; <i1> [#uses=1]
  %conv567 = zext i1 %cmp566 to i32               ; <i32> [#uses=1]
  %arraydecay568 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx569 = getelementptr inbounds i32* %arraydecay568, i32 1 ; <i32*> [#uses=2]
  %tmp570 = load i32* %arrayidx569                ; <i32> [#uses=1]
  %add571 = add i32 %tmp570, %conv567             ; <i32> [#uses=1]
  store i32 %add571, i32* %arrayidx569
  %arraydecay572 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx573 = getelementptr inbounds float* %arraydecay572, i32 2 ; <float*> [#uses=1]
  %tmp574 = load float* %arrayidx573              ; <float> [#uses=1]
  %tmp575 = load i32* %iLocalPixOffset            ; <i32> [#uses=2]
  %inc576 = add nsw i32 %tmp575, 1                ; <i32> [#uses=1]
  store i32 %inc576, i32* %iLocalPixOffset
  %tmp577 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx578 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp577, i32 %tmp575 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp579 = load <4 x i8> addrspace(3)* %arrayidx578 ; <<4 x i8>> [#uses=1]
  %tmp580 = extractelement <4 x i8> %tmp579, i32 2 ; <i8> [#uses=1]
  %conv581 = zext i8 %tmp580 to i32               ; <i32> [#uses=1]
  %conv582 = sitofp i32 %conv581 to float         ; <float> [#uses=1]
  %cmp583 = fcmp olt float %tmp574, %conv582      ; <i1> [#uses=1]
  %conv584 = zext i1 %cmp583 to i32               ; <i32> [#uses=1]
  %arraydecay585 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx586 = getelementptr inbounds i32* %arraydecay585, i32 2 ; <i32*> [#uses=2]
  %tmp587 = load i32* %arrayidx586                ; <i32> [#uses=1]
  %add588 = add i32 %tmp587, %conv584             ; <i32> [#uses=1]
  store i32 %add588, i32* %arrayidx586
  %arraydecay589 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx590 = getelementptr inbounds float* %arraydecay589, i32 0 ; <float*> [#uses=1]
  %tmp591 = load float* %arrayidx590              ; <float> [#uses=1]
  %tmp592 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp593 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx594 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp593, i32 %tmp592 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp595 = load <4 x i8> addrspace(3)* %arrayidx594 ; <<4 x i8>> [#uses=1]
  %tmp596 = extractelement <4 x i8> %tmp595, i32 0 ; <i8> [#uses=1]
  %conv597 = zext i8 %tmp596 to i32               ; <i32> [#uses=1]
  %conv598 = sitofp i32 %conv597 to float         ; <float> [#uses=1]
  %cmp599 = fcmp olt float %tmp591, %conv598      ; <i1> [#uses=1]
  %conv600 = zext i1 %cmp599 to i32               ; <i32> [#uses=1]
  %arraydecay601 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx602 = getelementptr inbounds i32* %arraydecay601, i32 0 ; <i32*> [#uses=2]
  %tmp603 = load i32* %arrayidx602                ; <i32> [#uses=1]
  %add604 = add i32 %tmp603, %conv600             ; <i32> [#uses=1]
  store i32 %add604, i32* %arrayidx602
  %arraydecay605 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx606 = getelementptr inbounds float* %arraydecay605, i32 1 ; <float*> [#uses=1]
  %tmp607 = load float* %arrayidx606              ; <float> [#uses=1]
  %tmp608 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp609 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx610 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp609, i32 %tmp608 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp611 = load <4 x i8> addrspace(3)* %arrayidx610 ; <<4 x i8>> [#uses=1]
  %tmp612 = extractelement <4 x i8> %tmp611, i32 1 ; <i8> [#uses=1]
  %conv613 = zext i8 %tmp612 to i32               ; <i32> [#uses=1]
  %conv614 = sitofp i32 %conv613 to float         ; <float> [#uses=1]
  %cmp615 = fcmp olt float %tmp607, %conv614      ; <i1> [#uses=1]
  %conv616 = zext i1 %cmp615 to i32               ; <i32> [#uses=1]
  %arraydecay617 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx618 = getelementptr inbounds i32* %arraydecay617, i32 1 ; <i32*> [#uses=2]
  %tmp619 = load i32* %arrayidx618                ; <i32> [#uses=1]
  %add620 = add i32 %tmp619, %conv616             ; <i32> [#uses=1]
  store i32 %add620, i32* %arrayidx618
  %arraydecay621 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx622 = getelementptr inbounds float* %arraydecay621, i32 2 ; <float*> [#uses=1]
  %tmp623 = load float* %arrayidx622              ; <float> [#uses=1]
  %tmp624 = load i32* %iLocalPixOffset            ; <i32> [#uses=2]
  %inc625 = add nsw i32 %tmp624, 1                ; <i32> [#uses=1]
  store i32 %inc625, i32* %iLocalPixOffset
  %tmp626 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx627 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp626, i32 %tmp624 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp628 = load <4 x i8> addrspace(3)* %arrayidx627 ; <<4 x i8>> [#uses=1]
  %tmp629 = extractelement <4 x i8> %tmp628, i32 2 ; <i8> [#uses=1]
  %conv630 = zext i8 %tmp629 to i32               ; <i32> [#uses=1]
  %conv631 = sitofp i32 %conv630 to float         ; <float> [#uses=1]
  %cmp632 = fcmp olt float %tmp623, %conv631      ; <i1> [#uses=1]
  %conv633 = zext i1 %cmp632 to i32               ; <i32> [#uses=1]
  %arraydecay634 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx635 = getelementptr inbounds i32* %arraydecay634, i32 2 ; <i32*> [#uses=2]
  %tmp636 = load i32* %arrayidx635                ; <i32> [#uses=1]
  %add637 = add i32 %tmp636, %conv633             ; <i32> [#uses=1]
  store i32 %add637, i32* %arrayidx635
  %arraydecay638 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx639 = getelementptr inbounds float* %arraydecay638, i32 0 ; <float*> [#uses=1]
  %tmp640 = load float* %arrayidx639              ; <float> [#uses=1]
  %tmp641 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp642 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx643 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp642, i32 %tmp641 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp644 = load <4 x i8> addrspace(3)* %arrayidx643 ; <<4 x i8>> [#uses=1]
  %tmp645 = extractelement <4 x i8> %tmp644, i32 0 ; <i8> [#uses=1]
  %conv646 = zext i8 %tmp645 to i32               ; <i32> [#uses=1]
  %conv647 = sitofp i32 %conv646 to float         ; <float> [#uses=1]
  %cmp648 = fcmp olt float %tmp640, %conv647      ; <i1> [#uses=1]
  %conv649 = zext i1 %cmp648 to i32               ; <i32> [#uses=1]
  %arraydecay650 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx651 = getelementptr inbounds i32* %arraydecay650, i32 0 ; <i32*> [#uses=2]
  %tmp652 = load i32* %arrayidx651                ; <i32> [#uses=1]
  %add653 = add i32 %tmp652, %conv649             ; <i32> [#uses=1]
  store i32 %add653, i32* %arrayidx651
  %arraydecay654 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx655 = getelementptr inbounds float* %arraydecay654, i32 1 ; <float*> [#uses=1]
  %tmp656 = load float* %arrayidx655              ; <float> [#uses=1]
  %tmp657 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp658 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx659 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp658, i32 %tmp657 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp660 = load <4 x i8> addrspace(3)* %arrayidx659 ; <<4 x i8>> [#uses=1]
  %tmp661 = extractelement <4 x i8> %tmp660, i32 1 ; <i8> [#uses=1]
  %conv662 = zext i8 %tmp661 to i32               ; <i32> [#uses=1]
  %conv663 = sitofp i32 %conv662 to float         ; <float> [#uses=1]
  %cmp664 = fcmp olt float %tmp656, %conv663      ; <i1> [#uses=1]
  %conv665 = zext i1 %cmp664 to i32               ; <i32> [#uses=1]
  %arraydecay666 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx667 = getelementptr inbounds i32* %arraydecay666, i32 1 ; <i32*> [#uses=2]
  %tmp668 = load i32* %arrayidx667                ; <i32> [#uses=1]
  %add669 = add i32 %tmp668, %conv665             ; <i32> [#uses=1]
  store i32 %add669, i32* %arrayidx667
  %arraydecay670 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx671 = getelementptr inbounds float* %arraydecay670, i32 2 ; <float*> [#uses=1]
  %tmp672 = load float* %arrayidx671              ; <float> [#uses=1]
  %tmp673 = load i32* %iLocalPixOffset            ; <i32> [#uses=1]
  %tmp674 = load <4 x i8> addrspace(3)** %uc4LocalData.addr ; <<4 x i8> addrspace(3)*> [#uses=1]
  %arrayidx675 = getelementptr inbounds <4 x i8> addrspace(3)* %tmp674, i32 %tmp673 ; <<4 x i8> addrspace(3)*> [#uses=1]
  %tmp676 = load <4 x i8> addrspace(3)* %arrayidx675 ; <<4 x i8>> [#uses=1]
  %tmp677 = extractelement <4 x i8> %tmp676, i32 2 ; <i8> [#uses=1]
  %conv678 = zext i8 %tmp677 to i32               ; <i32> [#uses=1]
  %conv679 = sitofp i32 %conv678 to float         ; <float> [#uses=1]
  %cmp680 = fcmp olt float %tmp672, %conv679      ; <i1> [#uses=1]
  %conv681 = zext i1 %cmp680 to i32               ; <i32> [#uses=1]
  %arraydecay682 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx683 = getelementptr inbounds i32* %arraydecay682, i32 2 ; <i32*> [#uses=2]
  %tmp684 = load i32* %arrayidx683                ; <i32> [#uses=1]
  %add685 = add i32 %tmp684, %conv681             ; <i32> [#uses=1]
  store i32 %add685, i32* %arrayidx683
  %arraydecay686 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx687 = getelementptr inbounds i32* %arraydecay686, i32 0 ; <i32*> [#uses=1]
  %tmp688 = load i32* %arrayidx687                ; <i32> [#uses=1]
  %cmp689 = icmp ugt i32 %tmp688, 4               ; <i1> [#uses=1]
  br i1 %cmp689, label %if.then691, label %if.else697

if.then691:                                       ; preds = %for.body
  %arraydecay692 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx693 = getelementptr inbounds float* %arraydecay692, i32 0 ; <float*> [#uses=1]
  %tmp694 = load float* %arrayidx693              ; <float> [#uses=1]
  %arraydecay695 = getelementptr inbounds [3 x float]* %fMinBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx696 = getelementptr inbounds float* %arraydecay695, i32 0 ; <float*> [#uses=1]
  store float %tmp694, float* %arrayidx696
  br label %if.end703

if.else697:                                       ; preds = %for.body
  %arraydecay698 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx699 = getelementptr inbounds float* %arraydecay698, i32 0 ; <float*> [#uses=1]
  %tmp700 = load float* %arrayidx699              ; <float> [#uses=1]
  %arraydecay701 = getelementptr inbounds [3 x float]* %fMaxBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx702 = getelementptr inbounds float* %arraydecay701, i32 0 ; <float*> [#uses=1]
  store float %tmp700, float* %arrayidx702
  br label %if.end703

if.end703:                                        ; preds = %if.else697, %if.then691
  %arraydecay704 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx705 = getelementptr inbounds i32* %arraydecay704, i32 1 ; <i32*> [#uses=1]
  %tmp706 = load i32* %arrayidx705                ; <i32> [#uses=1]
  %cmp707 = icmp ugt i32 %tmp706, 4               ; <i1> [#uses=1]
  br i1 %cmp707, label %if.then709, label %if.else715

if.then709:                                       ; preds = %if.end703
  %arraydecay710 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx711 = getelementptr inbounds float* %arraydecay710, i32 1 ; <float*> [#uses=1]
  %tmp712 = load float* %arrayidx711              ; <float> [#uses=1]
  %arraydecay713 = getelementptr inbounds [3 x float]* %fMinBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx714 = getelementptr inbounds float* %arraydecay713, i32 1 ; <float*> [#uses=1]
  store float %tmp712, float* %arrayidx714
  br label %if.end721

if.else715:                                       ; preds = %if.end703
  %arraydecay716 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx717 = getelementptr inbounds float* %arraydecay716, i32 1 ; <float*> [#uses=1]
  %tmp718 = load float* %arrayidx717              ; <float> [#uses=1]
  %arraydecay719 = getelementptr inbounds [3 x float]* %fMaxBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx720 = getelementptr inbounds float* %arraydecay719, i32 1 ; <float*> [#uses=1]
  store float %tmp718, float* %arrayidx720
  br label %if.end721

if.end721:                                        ; preds = %if.else715, %if.then709
  %arraydecay722 = getelementptr inbounds [3 x i32]* %uiHighCount, i32 0, i32 0 ; <i32*> [#uses=1]
  %arrayidx723 = getelementptr inbounds i32* %arraydecay722, i32 2 ; <i32*> [#uses=1]
  %tmp724 = load i32* %arrayidx723                ; <i32> [#uses=1]
  %cmp725 = icmp ugt i32 %tmp724, 4               ; <i1> [#uses=1]
  br i1 %cmp725, label %if.then727, label %if.else733

if.then727:                                       ; preds = %if.end721
  %arraydecay728 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx729 = getelementptr inbounds float* %arraydecay728, i32 2 ; <float*> [#uses=1]
  %tmp730 = load float* %arrayidx729              ; <float> [#uses=1]
  %arraydecay731 = getelementptr inbounds [3 x float]* %fMinBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx732 = getelementptr inbounds float* %arraydecay731, i32 2 ; <float*> [#uses=1]
  store float %tmp730, float* %arrayidx732
  br label %if.end739

if.else733:                                       ; preds = %if.end721
  %arraydecay734 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx735 = getelementptr inbounds float* %arraydecay734, i32 2 ; <float*> [#uses=1]
  %tmp736 = load float* %arrayidx735              ; <float> [#uses=1]
  %arraydecay737 = getelementptr inbounds [3 x float]* %fMaxBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx738 = getelementptr inbounds float* %arraydecay737, i32 2 ; <float*> [#uses=1]
  store float %tmp736, float* %arrayidx738
  br label %if.end739

if.end739:                                        ; preds = %if.else733, %if.then727
  %arraydecay740 = getelementptr inbounds [3 x float]* %fMaxBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx741 = getelementptr inbounds float* %arraydecay740, i32 0 ; <float*> [#uses=1]
  %tmp742 = load float* %arrayidx741              ; <float> [#uses=1]
  %arraydecay743 = getelementptr inbounds [3 x float]* %fMinBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx744 = getelementptr inbounds float* %arraydecay743, i32 0 ; <float*> [#uses=1]
  %tmp745 = load float* %arrayidx744              ; <float> [#uses=1]
  %add746 = fadd float %tmp742, %tmp745           ; <float> [#uses=1]
  %mul = fmul float %add746, 5.000000e-001        ; <float> [#uses=1]
  %arraydecay747 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx748 = getelementptr inbounds float* %arraydecay747, i32 0 ; <float*> [#uses=1]
  store float %mul, float* %arrayidx748
  %arraydecay749 = getelementptr inbounds [3 x float]* %fMaxBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx750 = getelementptr inbounds float* %arraydecay749, i32 1 ; <float*> [#uses=1]
  %tmp751 = load float* %arrayidx750              ; <float> [#uses=1]
  %arraydecay752 = getelementptr inbounds [3 x float]* %fMinBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx753 = getelementptr inbounds float* %arraydecay752, i32 1 ; <float*> [#uses=1]
  %tmp754 = load float* %arrayidx753              ; <float> [#uses=1]
  %add755 = fadd float %tmp751, %tmp754           ; <float> [#uses=1]
  %mul756 = fmul float %add755, 5.000000e-001     ; <float> [#uses=1]
  %arraydecay757 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx758 = getelementptr inbounds float* %arraydecay757, i32 1 ; <float*> [#uses=1]
  store float %mul756, float* %arrayidx758
  %arraydecay759 = getelementptr inbounds [3 x float]* %fMaxBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx760 = getelementptr inbounds float* %arraydecay759, i32 2 ; <float*> [#uses=1]
  %tmp761 = load float* %arrayidx760              ; <float> [#uses=1]
  %arraydecay762 = getelementptr inbounds [3 x float]* %fMinBound, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx763 = getelementptr inbounds float* %arraydecay762, i32 2 ; <float*> [#uses=1]
  %tmp764 = load float* %arrayidx763              ; <float> [#uses=1]
  %add765 = fadd float %tmp761, %tmp764           ; <float> [#uses=1]
  %mul766 = fmul float %add765, 5.000000e-001     ; <float> [#uses=1]
  %arraydecay767 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx768 = getelementptr inbounds float* %arraydecay767, i32 2 ; <float*> [#uses=1]
  store float %mul766, float* %arrayidx768
  br label %for.inc

for.inc:                                          ; preds = %if.end739
  %tmp769 = load i32* %iSearch                    ; <i32> [#uses=1]
  %inc770 = add nsw i32 %tmp769, 1                ; <i32> [#uses=1]
  store i32 %inc770, i32* %iSearch
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arraydecay772 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx773 = getelementptr inbounds float* %arraydecay772, i32 0 ; <float*> [#uses=1]
  %tmp774 = load float* %arrayidx773              ; <float> [#uses=1]
  %conv775 = fptoui float %tmp774 to i32          ; <i32> [#uses=1]
  %and = and i32 255, %conv775                    ; <i32> [#uses=1]
  store i32 %and, i32* %uiPackedPix
  %arraydecay776 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx777 = getelementptr inbounds float* %arraydecay776, i32 1 ; <float*> [#uses=1]
  %tmp778 = load float* %arrayidx777              ; <float> [#uses=1]
  %conv779 = fptoui float %tmp778 to i32          ; <i32> [#uses=1]
  %shl = shl i32 %conv779, 8                      ; <i32> [#uses=1]
  %and780 = and i32 65280, %shl                   ; <i32> [#uses=1]
  %tmp781 = load i32* %uiPackedPix                ; <i32> [#uses=1]
  %or = or i32 %tmp781, %and780                   ; <i32> [#uses=1]
  store i32 %or, i32* %uiPackedPix
  %arraydecay782 = getelementptr inbounds [3 x float]* %fMedianEstimate, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx783 = getelementptr inbounds float* %arraydecay782, i32 2 ; <float*> [#uses=1]
  %tmp784 = load float* %arrayidx783              ; <float> [#uses=1]
  %conv785 = fptoui float %tmp784 to i32          ; <i32> [#uses=1]
  %shl786 = shl i32 %conv785, 16                  ; <i32> [#uses=1]
  %and787 = and i32 16711680, %shl786             ; <i32> [#uses=1]
  %tmp788 = load i32* %uiPackedPix                ; <i32> [#uses=1]
  %or789 = or i32 %tmp788, %and787                ; <i32> [#uses=1]
  store i32 %or789, i32* %uiPackedPix
  %tmp790 = load i32* %iDevYPrime                 ; <i32> [#uses=1]
  %tmp791 = load i32* %uiDevImageHeight.addr      ; <i32> [#uses=1]
  %cmp792 = icmp slt i32 %tmp790, %tmp791         ; <i1> [#uses=1]
  br i1 %cmp792, label %land.lhs.true794, label %if.end806

land.lhs.true794:                                 ; preds = %for.end
  %tmp795 = load i32* %iImagePosX                 ; <i32> [#uses=1]
  %tmp796 = load i32* %uiImageWidth.addr          ; <i32> [#uses=1]
  %cmp797 = icmp slt i32 %tmp795, %tmp796         ; <i1> [#uses=1]
  br i1 %cmp797, label %if.then799, label %if.end806

if.then799:                                       ; preds = %land.lhs.true794
  %tmp800 = load i32* %uiPackedPix                ; <i32> [#uses=1]
  %tmp801 = load i32* %iDevGMEMOffset             ; <i32> [#uses=1]
  %call802 = call i32 @get_global_size(i32 0)     ; <i32> [#uses=1]
  %add803 = add i32 %tmp801, %call802             ; <i32> [#uses=1]
  %tmp804 = load i32 addrspace(1)** %uiDest.addr  ; <i32 addrspace(1)*> [#uses=1]
  %arrayidx805 = getelementptr inbounds i32 addrspace(1)* %tmp804, i32 %add803 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp800, i32 addrspace(1)* %arrayidx805
  br label %if.end806

if.end806:                                        ; preds = %if.then799, %land.lhs.true794, %for.end
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @_Z5mul24ii(i32, i32)

declare i32 @get_global_size(i32)

declare i32 @get_local_id(i32)

declare i32 @get_local_size(i32)

declare i32 @_Z5mul24jj(i32, i32)

declare i32 @get_group_id(i32)

declare void @barrier(i32)

declare void @llvm.memcpy.i32(i8* nocapture, i8* nocapture, i32, i32) nounwind

declare void @llvm.memset.i32(i8* nocapture, i8, i32, i32) nounwind
