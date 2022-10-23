; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -passes='function(sroa),cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(sroa),cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; CHECK: Begin
; CHECK: double callsite
; CHECK: End Inlining Report

; ModuleID = 'convolution.cpp'
source_filename = "convolution.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.Image = type { i32, i32, i32, ptr }

$_ZNK5Image8channelsEv = comdat any

$_ZNK5Image5widthEv = comdat any

$_ZNK5Image6heightEv = comdat any

$_ZNK5Image4dataEv = comdat any

$_ZN5Image4dataEv = comdat any

$_Z5clampIfET_S0_S0_S0_ = comdat any

@_ZZ10blur_imagePK5ImagePS_E6kernel = internal constant [25 x float] [float 1.000000e+00, float 4.000000e+00, float 6.000000e+00, float 4.000000e+00, float 1.000000e+00, float 4.000000e+00, float 1.600000e+01, float 2.400000e+01, float 1.600000e+01, float 4.000000e+00, float 6.000000e+00, float 2.400000e+01, float 3.600000e+01, float 2.400000e+01, float 6.000000e+00, float 4.000000e+00, float 1.600000e+01, float 2.400000e+01, float 1.600000e+01, float 4.000000e+00, float 1.000000e+00, float 4.000000e+00, float 6.000000e+00, float 4.000000e+00, float 1.000000e+00], align 16
@_ZZ10blur_imagePK5ImagePS_E6factor = internal constant float 2.560000e+02, align 4
@_ZZ13sharpen_imagePK5ImagePS_E6kernel = internal constant [9 x float] [float 0.000000e+00, float -1.000000e+00, float 0.000000e+00, float -1.000000e+00, float 5.000000e+00, float -1.000000e+00, float 0.000000e+00, float -1.000000e+00, float 0.000000e+00], align 16
@_ZZ13sharpen_imagePK5ImagePS_E6factor = internal constant float 1.000000e+00, align 4
@.str = private unnamed_addr constant [21 x i8] c"src->channels() == 3\00", align 1
@.str.1 = private unnamed_addr constant [16 x i8] c"convolution.cpp\00", align 1
@__PRETTY_FUNCTION__._ZN12_GLOBAL__N_18convolveEPK5ImagePKffmPS0_ = private unnamed_addr constant [91 x i8] c"void (anonymous namespace)::convolve(const Image *, const float *, float, size_t, Image *)\00", align 1

; Function Attrs: uwtable
define void @_Z10blur_imagePK5ImagePS_(ptr %src, ptr %dst) #0 {
entry:
  %src.addr = alloca ptr, align 8
  %dst.addr = alloca ptr, align 8
  store ptr %src, ptr %src.addr, align 8
  store ptr %dst, ptr %dst.addr, align 8
  %i = load ptr, ptr %src.addr, align 8
  %i1 = load ptr, ptr %dst.addr, align 8
  call void @_ZN12_GLOBAL__N_18convolveEPK5ImagePKffmPS0_(ptr %i, ptr @_ZZ10blur_imagePK5ImagePS_E6kernel, float 2.560000e+02, i64 5, ptr %i1)
  ret void
}

; Function Attrs: inlinehint uwtable
define internal void @_ZN12_GLOBAL__N_18convolveEPK5ImagePKffmPS0_(ptr %src, ptr %kernel, float %factor, i64 %size, ptr %dst) #1 {
entry:
  %src.addr = alloca ptr, align 8
  %kernel.addr = alloca ptr, align 8
  %factor.addr = alloca float, align 4
  %size.addr = alloca i64, align 8
  %dst.addr = alloca ptr, align 8
  %border_size = alloca i64, align 8
  %src_width = alloca i64, align 8
  %src_height = alloca i64, align 8
  %src_channels = alloca i64, align 8
  %dst_width = alloca i64, align 8
  %dst_height = alloca i64, align 8
  %dst_channels = alloca i64, align 8
  %src_data = alloca ptr, align 8
  %dst_data = alloca ptr, align 8
  %i = alloca i64, align 8
  %j = alloca i64, align 8
  %r = alloca float, align 4
  %g = alloca float, align 4
  %b = alloca float, align 4
  %u = alloca i64, align 8
  %v = alloca i64, align 8
  %src_offset = alloca i64, align 8
  %src_pixel = alloca ptr, align 8
  %dst_offset = alloca i64, align 8
  %dst_pixel = alloca ptr, align 8
  store ptr %src, ptr %src.addr, align 8
  store ptr %kernel, ptr %kernel.addr, align 8
  store float %factor, ptr %factor.addr, align 4
  store i64 %size, ptr %size.addr, align 8
  store ptr %dst, ptr %dst.addr, align 8
  %i1 = load ptr, ptr %src.addr, align 8
  %call = call i32 @_ZNK5Image8channelsEv(ptr %i1)
  %cmp = icmp eq i32 %call, 3
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  call void @__assert_fail(ptr @.str, ptr @.str.1, i32 17, ptr @__PRETTY_FUNCTION__._ZN12_GLOBAL__N_18convolveEPK5ImagePKffmPS0_) #6
  unreachable

bb:                                               ; No predecessors!
  br label %cond.end

cond.end:                                         ; preds = %bb, %cond.true
  %i2 = load i64, ptr %size.addr, align 8
  %div = udiv i64 %i2, 2
  store i64 %div, ptr %border_size, align 8
  %i3 = load ptr, ptr %src.addr, align 8
  %call1 = call i32 @_ZNK5Image5widthEv(ptr %i3)
  %conv = zext i32 %call1 to i64
  store i64 %conv, ptr %src_width, align 8
  %i4 = load ptr, ptr %src.addr, align 8
  %call2 = call i32 @_ZNK5Image6heightEv(ptr %i4)
  %conv3 = zext i32 %call2 to i64
  store i64 %conv3, ptr %src_height, align 8
  %i5 = load ptr, ptr %src.addr, align 8
  %call4 = call i32 @_ZNK5Image8channelsEv(ptr %i5)
  %conv5 = zext i32 %call4 to i64
  store i64 %conv5, ptr %src_channels, align 8
  %i6 = load ptr, ptr %src.addr, align 8
  %call6 = call i32 @_ZNK5Image5widthEv(ptr %i6)
  %conv7 = zext i32 %call6 to i64
  %i7 = load i64, ptr %border_size, align 8
  %mul = mul i64 %i7, 2
  %sub = sub i64 %conv7, %mul
  store i64 %sub, ptr %dst_width, align 8
  %i8 = load ptr, ptr %src.addr, align 8
  %call8 = call i32 @_ZNK5Image6heightEv(ptr %i8)
  %conv9 = zext i32 %call8 to i64
  %i9 = load i64, ptr %border_size, align 8
  %mul10 = mul i64 %i9, 2
  %sub11 = sub i64 %conv9, %mul10
  store i64 %sub11, ptr %dst_height, align 8
  %i10 = load ptr, ptr %src.addr, align 8
  %call12 = call i32 @_ZNK5Image8channelsEv(ptr %i10)
  %conv13 = zext i32 %call12 to i64
  store i64 %conv13, ptr %dst_channels, align 8
  %i11 = load ptr, ptr %dst.addr, align 8
  %i12 = load i64, ptr %dst_width, align 8
  %conv14 = trunc i64 %i12 to i32
  %i13 = load i64, ptr %dst_height, align 8
  %conv15 = trunc i64 %i13 to i32
  %i14 = load i64, ptr %dst_channels, align 8
  %conv16 = trunc i64 %i14 to i32
  call void @_ZN5Image5resetEjjj(ptr %i11, i32 %conv14, i32 %conv15, i32 %conv16)
  %i15 = load ptr, ptr %src.addr, align 8
  %call17 = call ptr @_ZNK5Image4dataEv(ptr %i15)
  store ptr %call17, ptr %src_data, align 8
  %i16 = load ptr, ptr %dst.addr, align 8
  %call18 = call ptr @_ZN5Image4dataEv(ptr %i16)
  store ptr %call18, ptr %dst_data, align 8
  %i17 = load i64, ptr %border_size, align 8
  store i64 %i17, ptr %i, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc82, %cond.end
  %i18 = load i64, ptr %i, align 8
  %i19 = load i64, ptr %src_height, align 8
  %i20 = load i64, ptr %border_size, align 8
  %sub19 = sub i64 %i19, %i20
  %cmp20 = icmp ult i64 %i18, %sub19
  br i1 %cmp20, label %for.body, label %for.end84

for.body:                                         ; preds = %for.cond
  %i21 = load i64, ptr %border_size, align 8
  store i64 %i21, ptr %j, align 8
  br label %for.cond21

for.cond21:                                       ; preds = %for.inc79, %for.body
  %i22 = load i64, ptr %j, align 8
  %i23 = load i64, ptr %src_width, align 8
  %i24 = load i64, ptr %border_size, align 8
  %sub22 = sub i64 %i23, %i24
  %cmp23 = icmp ult i64 %i22, %sub22
  br i1 %cmp23, label %for.body24, label %for.end81

for.body24:                                       ; preds = %for.cond21
  store float 0.000000e+00, ptr %r, align 4
  store float 0.000000e+00, ptr %g, align 4
  store float 0.000000e+00, ptr %b, align 4
  store i64 0, ptr %u, align 8
  br label %for.cond25

for.cond25:                                       ; preds = %for.inc58, %for.body24
  %i25 = load i64, ptr %u, align 8
  %i26 = load i64, ptr %size.addr, align 8
  %cmp26 = icmp ult i64 %i25, %i26
  br i1 %cmp26, label %for.body27, label %for.end60

for.body27:                                       ; preds = %for.cond25
  store i64 0, ptr %v, align 8
  br label %for.cond28

for.cond28:                                       ; preds = %for.inc, %for.body27
  %i27 = load i64, ptr %v, align 8
  %i28 = load i64, ptr %size.addr, align 8
  %cmp29 = icmp ult i64 %i27, %i28
  br i1 %cmp29, label %for.body30, label %for.end

for.body30:                                       ; preds = %for.cond28
  %i29 = load i64, ptr %i, align 8
  %i30 = load i64, ptr %u, align 8
  %add = add i64 %i29, %i30
  %i31 = load i64, ptr %border_size, align 8
  %sub31 = sub i64 %add, %i31
  %i32 = load i64, ptr %src_width, align 8
  %mul32 = mul i64 %sub31, %i32
  %i33 = load i64, ptr %j, align 8
  %i34 = load i64, ptr %v, align 8
  %add33 = add i64 %i33, %i34
  %i35 = load i64, ptr %border_size, align 8
  %sub34 = sub i64 %add33, %i35
  %add35 = add i64 %mul32, %sub34
  %i36 = load i64, ptr %src_channels, align 8
  %mul36 = mul i64 %add35, %i36
  store i64 %mul36, ptr %src_offset, align 8
  %i37 = load i64, ptr %src_offset, align 8
  %i38 = load ptr, ptr %src_data, align 8
  %arrayidx = getelementptr inbounds i8, ptr %i38, i64 %i37
  store ptr %arrayidx, ptr %src_pixel, align 8
  %i39 = load ptr, ptr %src_pixel, align 8
  %arrayidx37 = getelementptr inbounds i8, ptr %i39, i64 0
  %i40 = load i8, ptr %arrayidx37, align 1
  %conv38 = uitofp i8 %i40 to float
  %i41 = load i64, ptr %u, align 8
  %i42 = load i64, ptr %size.addr, align 8
  %mul39 = mul i64 %i41, %i42
  %i43 = load i64, ptr %v, align 8
  %add40 = add i64 %mul39, %i43
  %i44 = load ptr, ptr %kernel.addr, align 8
  %arrayidx41 = getelementptr inbounds float, ptr %i44, i64 %add40
  %i45 = load float, ptr %arrayidx41, align 4
  %mul42 = fmul float %conv38, %i45
  %i46 = load float, ptr %r, align 4
  %add43 = fadd float %i46, %mul42
  store float %add43, ptr %r, align 4
  %i47 = load ptr, ptr %src_pixel, align 8
  %arrayidx44 = getelementptr inbounds i8, ptr %i47, i64 1
  %i48 = load i8, ptr %arrayidx44, align 1
  %conv45 = uitofp i8 %i48 to float
  %i49 = load i64, ptr %u, align 8
  %i50 = load i64, ptr %size.addr, align 8
  %mul46 = mul i64 %i49, %i50
  %i51 = load i64, ptr %v, align 8
  %add47 = add i64 %mul46, %i51
  %i52 = load ptr, ptr %kernel.addr, align 8
  %arrayidx48 = getelementptr inbounds float, ptr %i52, i64 %add47
  %i53 = load float, ptr %arrayidx48, align 4
  %mul49 = fmul float %conv45, %i53
  %i54 = load float, ptr %g, align 4
  %add50 = fadd float %i54, %mul49
  store float %add50, ptr %g, align 4
  %i55 = load ptr, ptr %src_pixel, align 8
  %arrayidx51 = getelementptr inbounds i8, ptr %i55, i64 2
  %i56 = load i8, ptr %arrayidx51, align 1
  %conv52 = uitofp i8 %i56 to float
  %i57 = load i64, ptr %u, align 8
  %i58 = load i64, ptr %size.addr, align 8
  %mul53 = mul i64 %i57, %i58
  %i59 = load i64, ptr %v, align 8
  %add54 = add i64 %mul53, %i59
  %i60 = load ptr, ptr %kernel.addr, align 8
  %arrayidx55 = getelementptr inbounds float, ptr %i60, i64 %add54
  %i61 = load float, ptr %arrayidx55, align 4
  %mul56 = fmul float %conv52, %i61
  %i62 = load float, ptr %b, align 4
  %add57 = fadd float %i62, %mul56
  store float %add57, ptr %b, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body30
  %i63 = load i64, ptr %v, align 8
  %inc = add i64 %i63, 1
  store i64 %inc, ptr %v, align 8
  br label %for.cond28

for.end:                                          ; preds = %for.cond28
  br label %for.inc58

for.inc58:                                        ; preds = %for.end
  %i64 = load i64, ptr %u, align 8
  %inc59 = add i64 %i64, 1
  store i64 %inc59, ptr %u, align 8
  br label %for.cond25

for.end60:                                        ; preds = %for.cond25
  %i65 = load float, ptr %r, align 4
  %i66 = load float, ptr %factor.addr, align 4
  %div61 = fdiv float %i65, %i66
  store float %div61, ptr %r, align 4
  %i67 = load float, ptr %g, align 4
  %i68 = load float, ptr %factor.addr, align 4
  %div62 = fdiv float %i67, %i68
  store float %div62, ptr %g, align 4
  %i69 = load float, ptr %b, align 4
  %i70 = load float, ptr %factor.addr, align 4
  %div63 = fdiv float %i69, %i70
  store float %div63, ptr %b, align 4
  %i71 = load float, ptr %r, align 4
  %call64 = call float @_Z5clampIfET_S0_S0_S0_(float %i71, float 0.000000e+00, float 2.550000e+02)
  store float %call64, ptr %r, align 4
  %i72 = load float, ptr %g, align 4
  %call65 = call float @_Z5clampIfET_S0_S0_S0_(float %i72, float 0.000000e+00, float 2.550000e+02)
  store float %call65, ptr %g, align 4
  %i73 = load float, ptr %b, align 4
  %call66 = call float @_Z5clampIfET_S0_S0_S0_(float %i73, float 0.000000e+00, float 2.550000e+02)
  store float %call66, ptr %b, align 4
  %i74 = load i64, ptr %i, align 8
  %i75 = load i64, ptr %border_size, align 8
  %sub67 = sub i64 %i74, %i75
  %i76 = load i64, ptr %dst_width, align 8
  %mul68 = mul i64 %sub67, %i76
  %i77 = load i64, ptr %j, align 8
  %i78 = load i64, ptr %border_size, align 8
  %sub69 = sub i64 %i77, %i78
  %add70 = add i64 %mul68, %sub69
  %i79 = load i64, ptr %dst_channels, align 8
  %mul71 = mul i64 %add70, %i79
  store i64 %mul71, ptr %dst_offset, align 8
  %i80 = load i64, ptr %dst_offset, align 8
  %i81 = load ptr, ptr %dst_data, align 8
  %arrayidx72 = getelementptr inbounds i8, ptr %i81, i64 %i80
  store ptr %arrayidx72, ptr %dst_pixel, align 8
  %i82 = load float, ptr %r, align 4
  %conv73 = fptoui float %i82 to i8
  %i83 = load ptr, ptr %dst_pixel, align 8
  %arrayidx74 = getelementptr inbounds i8, ptr %i83, i64 0
  store i8 %conv73, ptr %arrayidx74, align 1
  %i84 = load float, ptr %g, align 4
  %conv75 = fptoui float %i84 to i8
  %i85 = load ptr, ptr %dst_pixel, align 8
  %arrayidx76 = getelementptr inbounds i8, ptr %i85, i64 1
  store i8 %conv75, ptr %arrayidx76, align 1
  %i86 = load float, ptr %b, align 4
  %conv77 = fptoui float %i86 to i8
  %i87 = load ptr, ptr %dst_pixel, align 8
  %arrayidx78 = getelementptr inbounds i8, ptr %i87, i64 2
  store i8 %conv77, ptr %arrayidx78, align 1
  br label %for.inc79

for.inc79:                                        ; preds = %for.end60
  %i88 = load i64, ptr %j, align 8
  %inc80 = add i64 %i88, 1
  store i64 %inc80, ptr %j, align 8
  br label %for.cond21

for.end81:                                        ; preds = %for.cond21
  br label %for.inc82

for.inc82:                                        ; preds = %for.end81
  %i89 = load i64, ptr %i, align 8
  %inc83 = add i64 %i89, 1
  store i64 %inc83, ptr %i, align 8
  br label %for.cond

for.end84:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: uwtable
define void @_Z13sharpen_imagePK5ImagePS_(ptr %src, ptr %dst) #0 {
entry:
  %src.addr = alloca ptr, align 8
  %dst.addr = alloca ptr, align 8
  store ptr %src, ptr %src.addr, align 8
  store ptr %dst, ptr %dst.addr, align 8
  %i = load ptr, ptr %src.addr, align 8
  %i1 = load ptr, ptr %dst.addr, align 8
  call void @_ZN12_GLOBAL__N_18convolveEPK5ImagePKffmPS0_(ptr %i, ptr @_ZZ13sharpen_imagePK5ImagePS_E6kernel, float 1.000000e+00, i64 3, ptr %i1)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr i32 @_ZNK5Image8channelsEv(ptr %this) #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %channels_ = getelementptr inbounds %class.Image, ptr %this1, i32 0, i32 2
  %i = load i32, ptr %channels_, align 8
  ret i32 %i
}

; Function Attrs: noreturn nounwind
declare void @__assert_fail(ptr, ptr, i32, ptr) #3

; Function Attrs: nounwind uwtable
define linkonce_odr i32 @_ZNK5Image5widthEv(ptr %this) #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %width_ = getelementptr inbounds %class.Image, ptr %this1, i32 0, i32 0
  %i = load i32, ptr %width_, align 8
  ret i32 %i
}

; Function Attrs: nounwind uwtable
define linkonce_odr i32 @_ZNK5Image6heightEv(ptr %this) #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %height_ = getelementptr inbounds %class.Image, ptr %this1, i32 0, i32 1
  %i = load i32, ptr %height_, align 4
  ret i32 %i
}

declare void @_ZN5Image5resetEjjj(ptr, i32, i32, i32) #4

; Function Attrs: nounwind uwtable
define linkonce_odr ptr @_ZNK5Image4dataEv(ptr %this) #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %data_ = getelementptr inbounds %class.Image, ptr %this1, i32 0, i32 3
  %i = load ptr, ptr %data_, align 8
  ret ptr %i
}

; Function Attrs: nounwind uwtable
define linkonce_odr ptr @_ZN5Image4dataEv(ptr %this) #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %data_ = getelementptr inbounds %class.Image, ptr %this1, i32 0, i32 3
  %i = load ptr, ptr %data_, align 8
  ret ptr %i
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr float @_Z5clampIfET_S0_S0_S0_(float %value, float %minimum_value, float %maximum_value) #5 comdat {
entry:
  %retval = alloca float, align 4
  %value.addr = alloca float, align 4
  %minimum_value.addr = alloca float, align 4
  %maximum_value.addr = alloca float, align 4
  store float %value, ptr %value.addr, align 4
  store float %minimum_value, ptr %minimum_value.addr, align 4
  store float %maximum_value, ptr %maximum_value.addr, align 4
  %i = load float, ptr %value.addr, align 4
  %i1 = load float, ptr %maximum_value.addr, align 4
  %cmp = fcmp ogt float %i, %i1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %i2 = load float, ptr %maximum_value.addr, align 4
  store float %i2, ptr %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  %i3 = load float, ptr %value.addr, align 4
  %i4 = load float, ptr %minimum_value.addr, align 4
  %cmp1 = fcmp olt float %i3, %i4
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  %i5 = load float, ptr %minimum_value.addr, align 4
  store float %i5, ptr %retval, align 4
  br label %return

if.end3:                                          ; preds = %if.end
  %i6 = load float, ptr %value.addr, align 4
  store float %i6, ptr %retval, align 4
  br label %return

return:                                           ; preds = %if.end3, %if.then2, %if.then
  %i7 = load float, ptr %retval, align 4
  ret float %i7
}

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { inlinehint uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { inlinehint nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { noreturn nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17975)"}
; end INTEL_FEATURE_SW_ADVANCED
