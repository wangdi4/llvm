; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='function(sroa),cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(sroa),cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; CHECK: Begin
; CHECK: double callsite
; CHECK: End Inlining Report

; ModuleID = 'convolution.cpp'
source_filename = "convolution.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.Image = type { i32, i32, i32, i8* }

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
define void @_Z10blur_imagePK5ImagePS_(%class.Image* %src, %class.Image* %dst) #0 {
entry:
  %src.addr = alloca %class.Image*, align 8
  %dst.addr = alloca %class.Image*, align 8
  store %class.Image* %src, %class.Image** %src.addr, align 8
  store %class.Image* %dst, %class.Image** %dst.addr, align 8
  %0 = load %class.Image*, %class.Image** %src.addr, align 8
  %1 = load %class.Image*, %class.Image** %dst.addr, align 8
  call void @_ZN12_GLOBAL__N_18convolveEPK5ImagePKffmPS0_(%class.Image* %0, float* getelementptr inbounds ([25 x float], [25 x float]* @_ZZ10blur_imagePK5ImagePS_E6kernel, i32 0, i32 0), float 2.560000e+02, i64 5, %class.Image* %1)
  ret void
}

; Function Attrs: inlinehint uwtable
define internal void @_ZN12_GLOBAL__N_18convolveEPK5ImagePKffmPS0_(%class.Image* %src, float* %kernel, float %factor, i64 %size, %class.Image* %dst) #1 {
entry:
  %src.addr = alloca %class.Image*, align 8
  %kernel.addr = alloca float*, align 8
  %factor.addr = alloca float, align 4
  %size.addr = alloca i64, align 8
  %dst.addr = alloca %class.Image*, align 8
  %border_size = alloca i64, align 8
  %src_width = alloca i64, align 8
  %src_height = alloca i64, align 8
  %src_channels = alloca i64, align 8
  %dst_width = alloca i64, align 8
  %dst_height = alloca i64, align 8
  %dst_channels = alloca i64, align 8
  %src_data = alloca i8*, align 8
  %dst_data = alloca i8*, align 8
  %i = alloca i64, align 8
  %j = alloca i64, align 8
  %r = alloca float, align 4
  %g = alloca float, align 4
  %b = alloca float, align 4
  %u = alloca i64, align 8
  %v = alloca i64, align 8
  %src_offset = alloca i64, align 8
  %src_pixel = alloca i8*, align 8
  %dst_offset = alloca i64, align 8
  %dst_pixel = alloca i8*, align 8
  store %class.Image* %src, %class.Image** %src.addr, align 8
  store float* %kernel, float** %kernel.addr, align 8
  store float %factor, float* %factor.addr, align 4
  store i64 %size, i64* %size.addr, align 8
  store %class.Image* %dst, %class.Image** %dst.addr, align 8
  %0 = load %class.Image*, %class.Image** %src.addr, align 8
  %call = call i32 @_ZNK5Image8channelsEv(%class.Image* %0)
  %cmp = icmp eq i32 %call, 3
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  call void @__assert_fail(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str.1, i32 0, i32 0), i32 17, i8* getelementptr inbounds ([91 x i8], [91 x i8]* @__PRETTY_FUNCTION__._ZN12_GLOBAL__N_18convolveEPK5ImagePKffmPS0_, i32 0, i32 0)) #6
  unreachable
                                                  ; No predecessors!
  br label %cond.end

cond.end:                                         ; preds = %1, %cond.true
  %2 = load i64, i64* %size.addr, align 8
  %div = udiv i64 %2, 2
  store i64 %div, i64* %border_size, align 8
  %3 = load %class.Image*, %class.Image** %src.addr, align 8
  %call1 = call i32 @_ZNK5Image5widthEv(%class.Image* %3)
  %conv = zext i32 %call1 to i64
  store i64 %conv, i64* %src_width, align 8
  %4 = load %class.Image*, %class.Image** %src.addr, align 8
  %call2 = call i32 @_ZNK5Image6heightEv(%class.Image* %4)
  %conv3 = zext i32 %call2 to i64
  store i64 %conv3, i64* %src_height, align 8
  %5 = load %class.Image*, %class.Image** %src.addr, align 8
  %call4 = call i32 @_ZNK5Image8channelsEv(%class.Image* %5)
  %conv5 = zext i32 %call4 to i64
  store i64 %conv5, i64* %src_channels, align 8
  %6 = load %class.Image*, %class.Image** %src.addr, align 8
  %call6 = call i32 @_ZNK5Image5widthEv(%class.Image* %6)
  %conv7 = zext i32 %call6 to i64
  %7 = load i64, i64* %border_size, align 8
  %mul = mul i64 %7, 2
  %sub = sub i64 %conv7, %mul
  store i64 %sub, i64* %dst_width, align 8
  %8 = load %class.Image*, %class.Image** %src.addr, align 8
  %call8 = call i32 @_ZNK5Image6heightEv(%class.Image* %8)
  %conv9 = zext i32 %call8 to i64
  %9 = load i64, i64* %border_size, align 8
  %mul10 = mul i64 %9, 2
  %sub11 = sub i64 %conv9, %mul10
  store i64 %sub11, i64* %dst_height, align 8
  %10 = load %class.Image*, %class.Image** %src.addr, align 8
  %call12 = call i32 @_ZNK5Image8channelsEv(%class.Image* %10)
  %conv13 = zext i32 %call12 to i64
  store i64 %conv13, i64* %dst_channels, align 8
  %11 = load %class.Image*, %class.Image** %dst.addr, align 8
  %12 = load i64, i64* %dst_width, align 8
  %conv14 = trunc i64 %12 to i32
  %13 = load i64, i64* %dst_height, align 8
  %conv15 = trunc i64 %13 to i32
  %14 = load i64, i64* %dst_channels, align 8
  %conv16 = trunc i64 %14 to i32
  call void @_ZN5Image5resetEjjj(%class.Image* %11, i32 %conv14, i32 %conv15, i32 %conv16)
  %15 = load %class.Image*, %class.Image** %src.addr, align 8
  %call17 = call i8* @_ZNK5Image4dataEv(%class.Image* %15)
  store i8* %call17, i8** %src_data, align 8
  %16 = load %class.Image*, %class.Image** %dst.addr, align 8
  %call18 = call i8* @_ZN5Image4dataEv(%class.Image* %16)
  store i8* %call18, i8** %dst_data, align 8
  %17 = load i64, i64* %border_size, align 8
  store i64 %17, i64* %i, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc82, %cond.end
  %18 = load i64, i64* %i, align 8
  %19 = load i64, i64* %src_height, align 8
  %20 = load i64, i64* %border_size, align 8
  %sub19 = sub i64 %19, %20
  %cmp20 = icmp ult i64 %18, %sub19
  br i1 %cmp20, label %for.body, label %for.end84

for.body:                                         ; preds = %for.cond
  %21 = load i64, i64* %border_size, align 8
  store i64 %21, i64* %j, align 8
  br label %for.cond21

for.cond21:                                       ; preds = %for.inc79, %for.body
  %22 = load i64, i64* %j, align 8
  %23 = load i64, i64* %src_width, align 8
  %24 = load i64, i64* %border_size, align 8
  %sub22 = sub i64 %23, %24
  %cmp23 = icmp ult i64 %22, %sub22
  br i1 %cmp23, label %for.body24, label %for.end81

for.body24:                                       ; preds = %for.cond21
  store float 0.000000e+00, float* %r, align 4
  store float 0.000000e+00, float* %g, align 4
  store float 0.000000e+00, float* %b, align 4
  store i64 0, i64* %u, align 8
  br label %for.cond25

for.cond25:                                       ; preds = %for.inc58, %for.body24
  %25 = load i64, i64* %u, align 8
  %26 = load i64, i64* %size.addr, align 8
  %cmp26 = icmp ult i64 %25, %26
  br i1 %cmp26, label %for.body27, label %for.end60

for.body27:                                       ; preds = %for.cond25
  store i64 0, i64* %v, align 8
  br label %for.cond28

for.cond28:                                       ; preds = %for.inc, %for.body27
  %27 = load i64, i64* %v, align 8
  %28 = load i64, i64* %size.addr, align 8
  %cmp29 = icmp ult i64 %27, %28
  br i1 %cmp29, label %for.body30, label %for.end

for.body30:                                       ; preds = %for.cond28
  %29 = load i64, i64* %i, align 8
  %30 = load i64, i64* %u, align 8
  %add = add i64 %29, %30
  %31 = load i64, i64* %border_size, align 8
  %sub31 = sub i64 %add, %31
  %32 = load i64, i64* %src_width, align 8
  %mul32 = mul i64 %sub31, %32
  %33 = load i64, i64* %j, align 8
  %34 = load i64, i64* %v, align 8
  %add33 = add i64 %33, %34
  %35 = load i64, i64* %border_size, align 8
  %sub34 = sub i64 %add33, %35
  %add35 = add i64 %mul32, %sub34
  %36 = load i64, i64* %src_channels, align 8
  %mul36 = mul i64 %add35, %36
  store i64 %mul36, i64* %src_offset, align 8
  %37 = load i64, i64* %src_offset, align 8
  %38 = load i8*, i8** %src_data, align 8
  %arrayidx = getelementptr inbounds i8, i8* %38, i64 %37
  store i8* %arrayidx, i8** %src_pixel, align 8
  %39 = load i8*, i8** %src_pixel, align 8
  %arrayidx37 = getelementptr inbounds i8, i8* %39, i64 0
  %40 = load i8, i8* %arrayidx37, align 1
  %conv38 = uitofp i8 %40 to float
  %41 = load i64, i64* %u, align 8
  %42 = load i64, i64* %size.addr, align 8
  %mul39 = mul i64 %41, %42
  %43 = load i64, i64* %v, align 8
  %add40 = add i64 %mul39, %43
  %44 = load float*, float** %kernel.addr, align 8
  %arrayidx41 = getelementptr inbounds float, float* %44, i64 %add40
  %45 = load float, float* %arrayidx41, align 4
  %mul42 = fmul float %conv38, %45
  %46 = load float, float* %r, align 4
  %add43 = fadd float %46, %mul42
  store float %add43, float* %r, align 4
  %47 = load i8*, i8** %src_pixel, align 8
  %arrayidx44 = getelementptr inbounds i8, i8* %47, i64 1
  %48 = load i8, i8* %arrayidx44, align 1
  %conv45 = uitofp i8 %48 to float
  %49 = load i64, i64* %u, align 8
  %50 = load i64, i64* %size.addr, align 8
  %mul46 = mul i64 %49, %50
  %51 = load i64, i64* %v, align 8
  %add47 = add i64 %mul46, %51
  %52 = load float*, float** %kernel.addr, align 8
  %arrayidx48 = getelementptr inbounds float, float* %52, i64 %add47
  %53 = load float, float* %arrayidx48, align 4
  %mul49 = fmul float %conv45, %53
  %54 = load float, float* %g, align 4
  %add50 = fadd float %54, %mul49
  store float %add50, float* %g, align 4
  %55 = load i8*, i8** %src_pixel, align 8
  %arrayidx51 = getelementptr inbounds i8, i8* %55, i64 2
  %56 = load i8, i8* %arrayidx51, align 1
  %conv52 = uitofp i8 %56 to float
  %57 = load i64, i64* %u, align 8
  %58 = load i64, i64* %size.addr, align 8
  %mul53 = mul i64 %57, %58
  %59 = load i64, i64* %v, align 8
  %add54 = add i64 %mul53, %59
  %60 = load float*, float** %kernel.addr, align 8
  %arrayidx55 = getelementptr inbounds float, float* %60, i64 %add54
  %61 = load float, float* %arrayidx55, align 4
  %mul56 = fmul float %conv52, %61
  %62 = load float, float* %b, align 4
  %add57 = fadd float %62, %mul56
  store float %add57, float* %b, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body30
  %63 = load i64, i64* %v, align 8
  %inc = add i64 %63, 1
  store i64 %inc, i64* %v, align 8
  br label %for.cond28

for.end:                                          ; preds = %for.cond28
  br label %for.inc58

for.inc58:                                        ; preds = %for.end
  %64 = load i64, i64* %u, align 8
  %inc59 = add i64 %64, 1
  store i64 %inc59, i64* %u, align 8
  br label %for.cond25

for.end60:                                        ; preds = %for.cond25
  %65 = load float, float* %r, align 4
  %66 = load float, float* %factor.addr, align 4
  %div61 = fdiv float %65, %66
  store float %div61, float* %r, align 4
  %67 = load float, float* %g, align 4
  %68 = load float, float* %factor.addr, align 4
  %div62 = fdiv float %67, %68
  store float %div62, float* %g, align 4
  %69 = load float, float* %b, align 4
  %70 = load float, float* %factor.addr, align 4
  %div63 = fdiv float %69, %70
  store float %div63, float* %b, align 4
  %71 = load float, float* %r, align 4
  %call64 = call float @_Z5clampIfET_S0_S0_S0_(float %71, float 0.000000e+00, float 2.550000e+02)
  store float %call64, float* %r, align 4
  %72 = load float, float* %g, align 4
  %call65 = call float @_Z5clampIfET_S0_S0_S0_(float %72, float 0.000000e+00, float 2.550000e+02)
  store float %call65, float* %g, align 4
  %73 = load float, float* %b, align 4
  %call66 = call float @_Z5clampIfET_S0_S0_S0_(float %73, float 0.000000e+00, float 2.550000e+02)
  store float %call66, float* %b, align 4
  %74 = load i64, i64* %i, align 8
  %75 = load i64, i64* %border_size, align 8
  %sub67 = sub i64 %74, %75
  %76 = load i64, i64* %dst_width, align 8
  %mul68 = mul i64 %sub67, %76
  %77 = load i64, i64* %j, align 8
  %78 = load i64, i64* %border_size, align 8
  %sub69 = sub i64 %77, %78
  %add70 = add i64 %mul68, %sub69
  %79 = load i64, i64* %dst_channels, align 8
  %mul71 = mul i64 %add70, %79
  store i64 %mul71, i64* %dst_offset, align 8
  %80 = load i64, i64* %dst_offset, align 8
  %81 = load i8*, i8** %dst_data, align 8
  %arrayidx72 = getelementptr inbounds i8, i8* %81, i64 %80
  store i8* %arrayidx72, i8** %dst_pixel, align 8
  %82 = load float, float* %r, align 4
  %conv73 = fptoui float %82 to i8
  %83 = load i8*, i8** %dst_pixel, align 8
  %arrayidx74 = getelementptr inbounds i8, i8* %83, i64 0
  store i8 %conv73, i8* %arrayidx74, align 1
  %84 = load float, float* %g, align 4
  %conv75 = fptoui float %84 to i8
  %85 = load i8*, i8** %dst_pixel, align 8
  %arrayidx76 = getelementptr inbounds i8, i8* %85, i64 1
  store i8 %conv75, i8* %arrayidx76, align 1
  %86 = load float, float* %b, align 4
  %conv77 = fptoui float %86 to i8
  %87 = load i8*, i8** %dst_pixel, align 8
  %arrayidx78 = getelementptr inbounds i8, i8* %87, i64 2
  store i8 %conv77, i8* %arrayidx78, align 1
  br label %for.inc79

for.inc79:                                        ; preds = %for.end60
  %88 = load i64, i64* %j, align 8
  %inc80 = add i64 %88, 1
  store i64 %inc80, i64* %j, align 8
  br label %for.cond21

for.end81:                                        ; preds = %for.cond21
  br label %for.inc82

for.inc82:                                        ; preds = %for.end81
  %89 = load i64, i64* %i, align 8
  %inc83 = add i64 %89, 1
  store i64 %inc83, i64* %i, align 8
  br label %for.cond

for.end84:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: uwtable
define void @_Z13sharpen_imagePK5ImagePS_(%class.Image* %src, %class.Image* %dst) #0 {
entry:
  %src.addr = alloca %class.Image*, align 8
  %dst.addr = alloca %class.Image*, align 8
  store %class.Image* %src, %class.Image** %src.addr, align 8
  store %class.Image* %dst, %class.Image** %dst.addr, align 8
  %0 = load %class.Image*, %class.Image** %src.addr, align 8
  %1 = load %class.Image*, %class.Image** %dst.addr, align 8
  call void @_ZN12_GLOBAL__N_18convolveEPK5ImagePKffmPS0_(%class.Image* %0, float* getelementptr inbounds ([9 x float], [9 x float]* @_ZZ13sharpen_imagePK5ImagePS_E6kernel, i32 0, i32 0), float 1.000000e+00, i64 3, %class.Image* %1)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr i32 @_ZNK5Image8channelsEv(%class.Image* %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %class.Image*, align 8
  store %class.Image* %this, %class.Image** %this.addr, align 8
  %this1 = load %class.Image*, %class.Image** %this.addr, align 8
  %channels_ = getelementptr inbounds %class.Image, %class.Image* %this1, i32 0, i32 2
  %0 = load i32, i32* %channels_, align 8
  ret i32 %0
}

; Function Attrs: noreturn nounwind
declare void @__assert_fail(i8*, i8*, i32, i8*) #3

; Function Attrs: nounwind uwtable
define linkonce_odr i32 @_ZNK5Image5widthEv(%class.Image* %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %class.Image*, align 8
  store %class.Image* %this, %class.Image** %this.addr, align 8
  %this1 = load %class.Image*, %class.Image** %this.addr, align 8
  %width_ = getelementptr inbounds %class.Image, %class.Image* %this1, i32 0, i32 0
  %0 = load i32, i32* %width_, align 8
  ret i32 %0
}

; Function Attrs: nounwind uwtable
define linkonce_odr i32 @_ZNK5Image6heightEv(%class.Image* %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %class.Image*, align 8
  store %class.Image* %this, %class.Image** %this.addr, align 8
  %this1 = load %class.Image*, %class.Image** %this.addr, align 8
  %height_ = getelementptr inbounds %class.Image, %class.Image* %this1, i32 0, i32 1
  %0 = load i32, i32* %height_, align 4
  ret i32 %0
}

declare void @_ZN5Image5resetEjjj(%class.Image*, i32, i32, i32) #4

; Function Attrs: nounwind uwtable
define linkonce_odr i8* @_ZNK5Image4dataEv(%class.Image* %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %class.Image*, align 8
  store %class.Image* %this, %class.Image** %this.addr, align 8
  %this1 = load %class.Image*, %class.Image** %this.addr, align 8
  %data_ = getelementptr inbounds %class.Image, %class.Image* %this1, i32 0, i32 3
  %0 = load i8*, i8** %data_, align 8
  ret i8* %0
}

; Function Attrs: nounwind uwtable
define linkonce_odr i8* @_ZN5Image4dataEv(%class.Image* %this) #2 comdat align 2 {
entry:
  %this.addr = alloca %class.Image*, align 8
  store %class.Image* %this, %class.Image** %this.addr, align 8
  %this1 = load %class.Image*, %class.Image** %this.addr, align 8
  %data_ = getelementptr inbounds %class.Image, %class.Image* %this1, i32 0, i32 3
  %0 = load i8*, i8** %data_, align 8
  ret i8* %0
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr float @_Z5clampIfET_S0_S0_S0_(float %value, float %minimum_value, float %maximum_value) #5 comdat {
entry:
  %retval = alloca float, align 4
  %value.addr = alloca float, align 4
  %minimum_value.addr = alloca float, align 4
  %maximum_value.addr = alloca float, align 4
  store float %value, float* %value.addr, align 4
  store float %minimum_value, float* %minimum_value.addr, align 4
  store float %maximum_value, float* %maximum_value.addr, align 4
  %0 = load float, float* %value.addr, align 4
  %1 = load float, float* %maximum_value.addr, align 4
  %cmp = fcmp ogt float %0, %1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = load float, float* %maximum_value.addr, align 4
  store float %2, float* %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  %3 = load float, float* %value.addr, align 4
  %4 = load float, float* %minimum_value.addr, align 4
  %cmp1 = fcmp olt float %3, %4
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  %5 = load float, float* %minimum_value.addr, align 4
  store float %5, float* %retval, align 4
  br label %return

if.end3:                                          ; preds = %if.end
  %6 = load float, float* %value.addr, align 4
  store float %6, float* %retval, align 4
  br label %return

return:                                           ; preds = %if.end3, %if.then2, %if.then
  %7 = load float, float* %retval, align 4
  ret float %7
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
