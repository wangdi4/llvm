; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATISobelFilter.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [3 x i8] c"82\00"		; <[3 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @sobel_filter
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %land.lhs.true10, %if.then
; CHECK: ret

define void @sobel_filter(i8 addrspace(2)* %inputImage, i8 addrspace(1)* %outputImage, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %call1 = call i32 @get_global_id(i32 1) nounwind
  %call2 = call i32 @get_global_size(i32 0) nounwind
  %call3 = call i32 @get_global_size(i32 1) nounwind
  %cmp = icmp ugt i32 %call, 3
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %sub = add i32 %call2, -4
  %cmp6 = icmp uge i32 %call, %sub
  %cmp9 = icmp eq i32 %call1, 0
  %or.cond = or i1 %cmp6, %cmp9
  br i1 %or.cond, label %if.end, label %land.lhs.true10

land.lhs.true10:                                  ; preds = %land.lhs.true
  %sub13 = add i32 %call3, -1
  %cmp14 = icmp ult i32 %call1, %sub13
  br i1 %cmp14, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true10
  %sub16 = add i32 %call, -4
  %sub19 = add i32 %call1, -1
  %mul = mul i32 %call2, %sub19
  %add = add i32 %mul, %sub16
  %arrayidx = getelementptr i8 addrspace(2)* %inputImage, i32 %add
  %tmp21 = load i8 addrspace(2)* %arrayidx, align 1
  %conv = zext i8 %tmp21 to i32
  %add27 = add i32 %mul, %call
  %arrayidx29 = getelementptr i8 addrspace(2)* %inputImage, i32 %add27
  %tmp30 = load i8 addrspace(2)* %arrayidx29, align 1
  %conv31 = zext i8 %tmp30 to i32
  %mul32 = shl nuw nsw i32 %conv31, 1
  %add35 = add i32 %call, 4
  %add40 = add i32 %mul, %add35
  %arrayidx42 = getelementptr i8 addrspace(2)* %inputImage, i32 %add40
  %tmp43 = load i8 addrspace(2)* %arrayidx42, align 1
  %conv44 = zext i8 %tmp43 to i32
  %add50 = add i32 %call1, 1
  %mul51 = mul i32 %call2, %add50
  %add52 = add i32 %mul51, %sub16
  %arrayidx54 = getelementptr i8 addrspace(2)* %inputImage, i32 %add52
  %tmp55 = load i8 addrspace(2)* %arrayidx54, align 1
  %conv56 = zext i8 %tmp55 to i32
  %add63 = add i32 %mul51, %call
  %arrayidx65 = getelementptr i8 addrspace(2)* %inputImage, i32 %add63
  %tmp66 = load i8 addrspace(2)* %arrayidx65, align 1
  %conv67 = zext i8 %tmp66 to i32
  %mul68 = shl nuw nsw i32 %conv67, 1
  %add76 = add i32 %mul51, %add35
  %arrayidx78 = getelementptr i8 addrspace(2)* %inputImage, i32 %add76
  %tmp79 = load i8 addrspace(2)* %arrayidx78, align 1
  %conv80 = zext i8 %tmp79 to i32
  %add33 = add i32 %conv44, %conv
  %add45 = add i32 %add33, %mul32
  %sub57 = sub i32 %add45, %conv56
  %sub69 = sub i32 %sub57, %conv80
  %sub81 = sub i32 %sub69, %mul68
  %mul122 = mul i32 %call2, %call1
  %add123 = add i32 %mul122, %sub16
  %arrayidx125 = getelementptr i8 addrspace(2)* %inputImage, i32 %add123
  %tmp126 = load i8 addrspace(2)* %arrayidx125, align 1
  %conv127 = zext i8 %tmp126 to i32
  %mul128 = shl nuw nsw i32 %conv127, 1
  %add148 = add i32 %mul122, %add35
  %arrayidx150 = getelementptr i8 addrspace(2)* %inputImage, i32 %add148
  %tmp151 = load i8 addrspace(2)* %arrayidx150, align 1
  %conv152 = zext i8 %tmp151 to i32
  %mul153 = shl nuw nsw i32 %conv152, 1
  %sub116 = sub i32 %conv, %conv44
  %add141 = add i32 %sub116, %conv56
  %sub154 = sub i32 %add141, %conv80
  %add178 = add i32 %sub154, %mul128
  %sub190 = sub i32 %add178, %mul153
  %add195 = add i32 %mul122, %call
  %arrayidx197 = getelementptr i8 addrspace(1)* %outputImage, i32 %add195
  %conv199 = sitofp i32 %sub81 to float
  %conv201 = sitofp i32 %sub190 to float
  %call202 = call float @__hypotf(float %conv199, float %conv201) nounwind
  %div = fmul float %call202, 5.000000e-01
  %conv203 = fptoui float %div to i8
  store i8 %conv203, i8 addrspace(1)* %arrayidx197, align 1
  br label %if.end

if.end:                                           ; preds = %land.lhs.true, %if.then, %land.lhs.true10, %entry
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

declare float @__hypotf(float, float)
