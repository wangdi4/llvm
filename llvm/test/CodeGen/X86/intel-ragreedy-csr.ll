; RUN: llc < %s -mtriple=x86_64-pc-linux --enable-intel-advanced-opts -O3 -regalloc=greedy | FileCheck %s

; CHECK: _ZN3povL23All_Plane_IntersectionsEPNS_13Object_StructEPNS_10Ray_StructEPNS_13istack_structE.2582
; CHECK-NOT: pushq

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.pov::Object_Struct.99" = type { %"struct.pov::Method_Struct.74"*, i32, %"struct.pov::Object_Struct.99"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Interior_Struct"*, %"struct.pov::Object_Struct.99"*, %"struct.pov::Object_Struct.99"*, %"struct.pov::Light_Source_Struct.98"*, %"struct.pov::Bounding_Box_Struct", %"struct.pov::Transform_Struct"*, %"struct.pov::Transform_Struct"*, float, i32 }
%"struct.pov::Method_Struct.74" = type { {}*, i32 (double*, %"struct.pov::Object_Struct.99"*)*, void (double*, %"struct.pov::Object_Struct.99"*, %"struct.pov::istk_entry.72"*)*, void (double*, %"struct.pov::Object_Struct.99"*, %"struct.pov::istk_entry.72"*)*, i8* (%"struct.pov::Object_Struct.99"*)*, void (%"struct.pov::Object_Struct.99"*, double*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct.99"*, double*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct.99"*, double*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct.99"*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct.99"*)*, void (%"struct.pov::Object_Struct.99"*)* }
%"struct.pov::istk_entry.72" = type { double, [3 x double], [3 x double], [3 x double], [2 x double], %"struct.pov::Object_Struct.99"*, i32, i32, double, double, double, double, double, double, double, double, double, i8*, i8* }
%"struct.pov::Texture_Struct" = type { i16, i16, i16, i32, float, float, float, %"struct.pov::Warps_Struct"*, %"struct.pov::Pattern_Struct"*, %"struct.pov::Blend_Map_Struct"*, %union.anon.17, %"struct.pov::Texture_Struct"*, %"struct.pov::Pigment_Struct"*, %"struct.pov::Tnormal_Struct"*, %"struct.pov::Finish_Struct"*, %"struct.pov::Texture_Struct"*, i32 }
%"struct.pov::Warps_Struct" = type { i16, %"struct.pov::Warps_Struct"*, %"struct.pov::Warps_Struct"* }
%"struct.pov::Pattern_Struct" = type { i16, i16, i16, i32, float, float, float, %"struct.pov::Warps_Struct"*, %"struct.pov::Pattern_Struct"*, %"struct.pov::Blend_Map_Struct"*, %union.anon.17 }
%"struct.pov::Blend_Map_Struct" = type { i32, i16, i8, i8, %"struct.pov::Blend_Map_Entry"* }
%"struct.pov::Blend_Map_Entry" = type { float, i8, %union.anon }
%union.anon = type { [2 x double], [8 x i8] }
%union.anon.17 = type { %struct.anon.21 }
%struct.anon.21 = type { [3 x double], [3 x double], double, double, i16, [3 x double]*, i32, [3 x double] }
%"struct.pov::Pigment_Struct" = type { i16, i16, i16, i32, float, float, float, %"struct.pov::Warps_Struct"*, %"struct.pov::Pattern_Struct"*, %"struct.pov::Blend_Map_Struct"*, %union.anon.17, [5 x float] }
%"struct.pov::Tnormal_Struct" = type { i16, i16, i16, i32, float, float, float, %"struct.pov::Warps_Struct"*, %"struct.pov::Pattern_Struct"*, %"struct.pov::Blend_Map_Struct"*, %union.anon.17, float, float }
%"struct.pov::Finish_Struct" = type { float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, [3 x float], [3 x float], [3 x float], float, i32, float, i32 }
%"struct.pov::Interior_Struct" = type { i32, i32, i32, float, float, float, float, float, float, [5 x float], %"struct.pov::Media_Struct"* }
%"struct.pov::Media_Struct" = type { i32, i32, i32, i32, i32, double, i32, double, double, i32, i32, i32, i32, [5 x float], [5 x float], [5 x float], [5 x float], double, double, double, double*, double, i32, i32, %"struct.pov::Pigment_Struct"*, %"struct.pov::Media_Struct"* }
%"struct.pov::Light_Source_Struct.98" = type { %"struct.pov::Method_Struct.74"*, i32, %"struct.pov::Object_Struct.99"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Interior_Struct"*, %"struct.pov::Object_Struct.99"*, %"struct.pov::Object_Struct.99"*, %"struct.pov::Light_Source_Struct.98"*, %"struct.pov::Bounding_Box_Struct", %"struct.pov::Transform_Struct"*, %"struct.pov::Transform_Struct"*, float, i32, %"struct.pov::Object_Struct.99"*, [5 x float], [3 x double], [3 x double], [3 x double], [3 x double], [3 x double], double, double, double, double, double, %"struct.pov::Light_Source_Struct.98"*, i8, i8, i8, i8, i8, i8, i8, i8, i32, i32, i32, i32, i32, [5 x float]**, %"struct.pov::Object_Struct.99"*, %"struct.pov::Object_Struct.99"*, %"struct.pov::Blend_Map_Struct"*, [6 x %"struct.pov::Project_Tree_Node_Struct"*] }
%"struct.pov::Project_Tree_Node_Struct" = type { i16, %"struct.pov::BBox_Tree_Struct"*, %"struct.pov::Project_Struct", i16, %"struct.pov::Project_Tree_Node_Struct"** }
%"struct.pov::BBox_Tree_Struct" = type { i16, i16, %"struct.pov::Bounding_Box_Struct", %"struct.pov::BBox_Tree_Struct"** }
%"struct.pov::Project_Struct" = type { i32, i32, i32, i32 }
%"struct.pov::Bounding_Box_Struct" = type { [3 x float], [3 x float] }
%"struct.pov::Transform_Struct" = type { [4 x [4 x double]], [4 x [4 x double]] }
%"struct.pov::Ray_Struct" = type { [3 x double], [3 x double], i32, i32, [100 x %"struct.pov::Interior_Struct"*] }
%"__DTRT_struct.pov::istack_struct.101" = type { %"__DTRT_struct.pov::istack_struct.101"*, %"struct.pov::istk_entry.72"*, i32, i32 }
%"struct.pov::Object_Struct" = type { %"struct.pov::Method_Struct"*, i32, %"struct.pov::Object_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Interior_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Light_Source_Struct"*, %"struct.pov::Bounding_Box_Struct", %"struct.pov::Transform_Struct"*, %"struct.pov::Transform_Struct"*, float, i32 }
%"struct.pov::Method_Struct" = type { i32 (%"struct.pov::Object_Struct"*, %"struct.pov::Ray_Struct"*, %"struct.pov::istack_struct"*)*, i32 (double*, %"struct.pov::Object_Struct"*)*, void (double*, %"struct.pov::Object_Struct"*, %"struct.pov::istk_entry"*)*, void (double*, %"struct.pov::Object_Struct"*, %"struct.pov::istk_entry"*)*, i8* (%"struct.pov::Object_Struct"*)*, void (%"struct.pov::Object_Struct"*, double*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct"*, double*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct"*, double*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct"*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct"*)*, void (%"struct.pov::Object_Struct"*)* }
%"struct.pov::istack_struct" = type { %"struct.pov::istack_struct"*, %"struct.pov::istk_entry"*, i32, i32 }
%"struct.pov::istk_entry" = type { double, [3 x double], [3 x double], [3 x double], [2 x double], %"struct.pov::Object_Struct"*, i32, i32, double, double, double, double, double, double, double, double, double, i8*, i8* }
%"struct.pov::Light_Source_Struct" = type { %"struct.pov::Method_Struct"*, i32, %"struct.pov::Object_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Interior_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Light_Source_Struct"*, %"struct.pov::Bounding_Box_Struct", %"struct.pov::Transform_Struct"*, %"struct.pov::Transform_Struct"*, float, i32, %"struct.pov::Object_Struct"*, [5 x float], [3 x double], [3 x double], [3 x double], [3 x double], [3 x double], double, double, double, double, double, %"struct.pov::Light_Source_Struct"*, i8, i8, i8, i8, i8, i8, i8, i8, i32, i32, i32, i32, i32, [5 x float]**, %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Blend_Map_Struct"*, [6 x %"struct.pov::Project_Tree_Node_Struct"*] }

@_ZN3pov5statsE = external hidden global [123 x i64], align 32
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0
; Function Attrs: nounwind readnone speculatable willreturn
declare double @llvm.fabs.f64(double) #1
; Function Attrs: uwtable
define hidden i32 @_ZN3povL23All_Plane_IntersectionsEPNS_13Object_StructEPNS_10Ray_StructEPNS_13istack_structE.2582(%"struct.pov::Object_Struct.99"* %0, %"struct.pov::Ray_Struct"* nocapture readonly %1, %"__DTRT_struct.pov::istack_struct.101"* nocapture %2) #2 {
  %4 = alloca [3 x double], align 16
  %5 = bitcast [3 x double]* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %5) #4
  %6 = load i64, i64* getelementptr inbounds ([123 x i64], [123 x i64]* @_ZN3pov5statsE, i64 0, i64 54), align 16, !tbaa !5
  %7 = add nsw i64 %6, 1
  store i64 %7, i64* getelementptr inbounds ([123 x i64], [123 x i64]* @_ZN3pov5statsE, i64 0, i64 54), align 16, !tbaa !5
  %8 = getelementptr inbounds %"struct.pov::Object_Struct.99", %"struct.pov::Object_Struct.99"* %0, i64 0, i32 10
  %9 = load %"struct.pov::Transform_Struct"*, %"struct.pov::Transform_Struct"** %8, align 8, !tbaa !9
  %10 = icmp eq %"struct.pov::Transform_Struct"* %9, null
  br i1 %10, label %11, label %45

11:                                               ; preds = %3
  %12 = getelementptr inbounds %"struct.pov::Object_Struct.99", %"struct.pov::Object_Struct.99"* %0, i64 1
  %13 = getelementptr inbounds %"struct.pov::Ray_Struct", %"struct.pov::Ray_Struct"* %1, i64 0, i32 1, i64 0
  %14 = bitcast double* %13 to <2 x double>*
  %15 = load <2 x double>, <2 x double>* %14, align 8, !tbaa !23
  %16 = bitcast %"struct.pov::Object_Struct.99"* %12 to <2 x double>*
  %17 = load <2 x double>, <2 x double>* %16, align 8, !tbaa !23
  %18 = fmul fast <2 x double> %15, %17
  %19 = extractelement <2 x double> %18, i32 0
  %20 = extractelement <2 x double> %18, i32 1
  %21 = fadd fast double %20, %19
  %22 = getelementptr inbounds %"struct.pov::Object_Struct.99", %"struct.pov::Object_Struct.99"* %0, i64 1, i32 2
  %23 = bitcast %"struct.pov::Object_Struct.99"** %22 to double*
  %24 = load double, double* %23, align 8, !tbaa !23
  %25 = getelementptr inbounds %"struct.pov::Ray_Struct", %"struct.pov::Ray_Struct"* %1, i64 0, i32 1, i64 2
  %26 = load double, double* %25, align 8, !tbaa !23
  %27 = fmul fast double %26, %24
  %28 = fadd fast double %21, %27
  %29 = tail call fast double @llvm.fabs.f64(double %28)
  %30 = fcmp fast olt double %29, 0x3E7AD7F29ABCAF48
  br i1 %30, label %223, label %31

31:                                               ; preds = %11
  %32 = bitcast %"struct.pov::Ray_Struct"* %1 to <2 x double>*
  %33 = load <2 x double>, <2 x double>* %32, align 8, !tbaa !23
  %34 = extractelement <2 x double> %33, i32 0
  %35 = extractelement <2 x double> %17, i32 0
  %36 = fmul fast double %34, %35
  %37 = extractelement <2 x double> %33, i32 1
  %38 = extractelement <2 x double> %17, i32 1
  %39 = fmul fast double %37, %38
  %40 = fadd fast double %39, %36
  %41 = getelementptr inbounds %"struct.pov::Ray_Struct", %"struct.pov::Ray_Struct"* %1, i64 0, i32 0, i64 2
  %42 = load double, double* %41, align 8, !tbaa !23
  %43 = fmul fast double %42, %24
  %44 = fadd fast double %40, %43
  br label %148

45:                                               ; preds = %3
  %46 = bitcast %"struct.pov::Ray_Struct"* %1 to <2 x double>*
  %47 = load <2 x double>, <2 x double>* %46, align 8, !tbaa !23
  %48 = getelementptr inbounds %"struct.pov::Ray_Struct", %"struct.pov::Ray_Struct"* %1, i64 0, i32 0, i64 2
  %49 = load double, double* %48, align 8, !tbaa !23
  %50 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 0, i64 0
  %51 = bitcast double* %50 to <2 x double>*
  %52 = load <2 x double>, <2 x double>* %51, align 8, !tbaa !24
  %53 = shufflevector <2 x double> %47, <2 x double> undef, <2 x i32> zeroinitializer
  %54 = fmul fast <2 x double> %52, %53
  %55 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 1, i64 0
  %56 = bitcast double* %55 to <2 x double>*
  %57 = load <2 x double>, <2 x double>* %56, align 8, !tbaa !24
  %58 = shufflevector <2 x double> %47, <2 x double> undef, <2 x i32> <i32 1, i32 1>
  %59 = fmul fast <2 x double> %57, %58
  %60 = fadd fast <2 x double> %59, %54
  %61 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 2, i64 0
  %62 = bitcast double* %61 to <2 x double>*
  %63 = load <2 x double>, <2 x double>* %62, align 8, !tbaa !24
  %64 = insertelement <2 x double> undef, double %49, i32 0
  %65 = shufflevector <2 x double> %64, <2 x double> undef, <2 x i32> zeroinitializer
  %66 = fmul fast <2 x double> %63, %65
  %67 = fadd fast <2 x double> %60, %66
  %68 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 3, i64 0
  %69 = bitcast double* %68 to <2 x double>*
  %70 = load <2 x double>, <2 x double>* %69, align 8, !tbaa !24
  %71 = fadd fast <2 x double> %67, %70
  %72 = getelementptr inbounds %"struct.pov::Ray_Struct", %"struct.pov::Ray_Struct"* %1, i64 0, i32 1, i64 0
  %73 = bitcast double* %72 to <2 x double>*
  %74 = load <2 x double>, <2 x double>* %73, align 8, !tbaa !23
  %75 = getelementptr inbounds %"struct.pov::Ray_Struct", %"struct.pov::Ray_Struct"* %1, i64 0, i32 1, i64 2
  %76 = load double, double* %75, align 8, !tbaa !23
  %77 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 0, i64 0
  %78 = bitcast double* %77 to <2 x double>*
  %79 = load <2 x double>, <2 x double>* %78, align 8, !tbaa !24
  %80 = extractelement <2 x double> %74, i32 0
  %81 = shufflevector <2 x double> %74, <2 x double> undef, <2 x i32> zeroinitializer
  %82 = fmul fast <2 x double> %79, %81
  %83 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 1, i64 0
  %84 = bitcast double* %83 to <2 x double>*
  %85 = load <2 x double>, <2 x double>* %84, align 8, !tbaa !24
  %86 = extractelement <2 x double> %74, i32 1
  %87 = shufflevector <2 x double> %74, <2 x double> undef, <2 x i32> <i32 1, i32 1>
  %88 = fmul fast <2 x double> %85, %87
  %89 = fadd fast <2 x double> %88, %82
  %90 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 2, i64 0
  %91 = bitcast double* %90 to <2 x double>*
  %92 = load <2 x double>, <2 x double>* %91, align 8, !tbaa !24
  %93 = insertelement <2 x double> undef, double %76, i32 0
  %94 = shufflevector <2 x double> %93, <2 x double> undef, <2 x i32> zeroinitializer
  %95 = fmul fast <2 x double> %92, %94
  %96 = fadd fast <2 x double> %89, %95
  %97 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 0, i64 2
  %98 = load double, double* %97, align 8, !tbaa !24
  %99 = fmul fast double %98, %80
  %100 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 1, i64 2
  %101 = load double, double* %100, align 8, !tbaa !24
  %102 = fmul fast double %101, %86
  %103 = fadd fast double %102, %99
  %104 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 2, i64 2
  %105 = load double, double* %104, align 8, !tbaa !24
  %106 = fmul fast double %105, %76
  %107 = fadd fast double %103, %106
  %108 = getelementptr inbounds %"struct.pov::Object_Struct.99", %"struct.pov::Object_Struct.99"* %0, i64 1
  %109 = bitcast %"struct.pov::Object_Struct.99"* %108 to <2 x double>*
  %110 = load <2 x double>, <2 x double>* %109, align 8, !tbaa !23
  %111 = fmul fast <2 x double> %96, %110
  %112 = extractelement <2 x double> %111, i32 0
  %113 = extractelement <2 x double> %111, i32 1
  %114 = fadd fast double %113, %112
  %115 = getelementptr inbounds %"struct.pov::Object_Struct.99", %"struct.pov::Object_Struct.99"* %0, i64 1, i32 2
  %116 = bitcast %"struct.pov::Object_Struct.99"** %115 to double*
  %117 = load double, double* %116, align 8, !tbaa !23
  %118 = fmul fast double %107, %117
  %119 = fadd fast double %114, %118
  %120 = tail call fast double @llvm.fabs.f64(double %119)
  %121 = fcmp fast olt double %120, 0x3E7AD7F29ABCAF48
  br i1 %121, label %223, label %122

122:                                              ; preds = %45
  %123 = extractelement <2 x double> %71, i32 1
  %124 = extractelement <2 x double> %71, i32 0
  %125 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 1, i64 2
  %126 = load double, double* %125, align 8, !tbaa !24
  %127 = extractelement <2 x double> %47, i32 1
  %128 = fmul fast double %126, %127
  %129 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 0, i64 2
  %130 = load double, double* %129, align 8, !tbaa !24
  %131 = extractelement <2 x double> %47, i32 0
  %132 = fmul fast double %130, %131
  %133 = fadd fast double %128, %132
  %134 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 2, i64 2
  %135 = load double, double* %134, align 8, !tbaa !24
  %136 = fmul fast double %135, %49
  %137 = fadd fast double %133, %136
  %138 = getelementptr inbounds %"struct.pov::Transform_Struct", %"struct.pov::Transform_Struct"* %9, i64 0, i32 1, i64 3, i64 2
  %139 = load double, double* %138, align 8, !tbaa !24
  %140 = fadd fast double %137, %139
  %141 = extractelement <2 x double> %110, i32 0
  %142 = fmul fast double %124, %141
  %143 = extractelement <2 x double> %110, i32 1
  %144 = fmul fast double %123, %143
  %145 = fadd fast double %144, %142
  %146 = fmul fast double %140, %117
  %147 = fadd fast double %145, %146
  br label %148

148:                                              ; preds = %122, %31
  %149 = phi double [ %26, %31 ], [ %76, %122 ]
  %150 = phi double [ %42, %31 ], [ %49, %122 ]
  %151 = phi double [ %44, %31 ], [ %147, %122 ]
  %152 = phi double [ %28, %31 ], [ %119, %122 ]
  %153 = phi <2 x double> [ %15, %31 ], [ %74, %122 ]
  %154 = phi <2 x double> [ %33, %31 ], [ %47, %122 ]
  %155 = getelementptr inbounds %"struct.pov::Object_Struct.99", %"struct.pov::Object_Struct.99"* %0, i64 1, i32 3
  %156 = bitcast %"struct.pov::Texture_Struct"** %155 to double*
  %157 = load double, double* %156, align 8, !tbaa !28
  %158 = fadd fast double %157, %151
  %159 = fneg fast double %158
  %160 = fdiv fast double %159, %152
  %161 = fcmp fast ult double %160, 0x3EB0C6F7A0B5ED8D
  %162 = fcmp fast ugt double %160, 1.000000e+07
  %163 = or i1 %161, %162
  br i1 %163, label %223, label %164

164:                                              ; preds = %148
  %165 = load i64, i64* getelementptr inbounds ([123 x i64], [123 x i64]* @_ZN3pov5statsE, i64 0, i64 55), align 8, !tbaa !5
  %166 = add nsw i64 %165, 1
  store i64 %166, i64* getelementptr inbounds ([123 x i64], [123 x i64]* @_ZN3pov5statsE, i64 0, i64 55), align 8, !tbaa !5
  %167 = insertelement <2 x double> undef, double %160, i32 0
  %168 = shufflevector <2 x double> %167, <2 x double> undef, <2 x i32> zeroinitializer
  %169 = fmul fast <2 x double> %153, %168
  %170 = fadd fast <2 x double> %169, %154
  %171 = bitcast [3 x double]* %4 to <2 x double>*
  store <2 x double> %170, <2 x double>* %171, align 16, !tbaa !23
  %172 = fmul fast double %149, %160
  %173 = fadd fast double %172, %150
  %174 = getelementptr inbounds [3 x double], [3 x double]* %4, i64 0, i64 2
  store double %173, double* %174, align 16, !tbaa !23
  %175 = getelementptr inbounds %"struct.pov::Object_Struct.99", %"struct.pov::Object_Struct.99"* %0, i64 0, i32 7, !intel-tbaa !29
  %176 = bitcast %"struct.pov::Object_Struct.99"** %175 to %"struct.pov::Object_Struct"**
  %177 = load %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"** %176, align 8, !tbaa !29
  %178 = icmp eq %"struct.pov::Object_Struct"* %177, null
  br i1 %178, label %182, label %179

179:                                              ; preds = %164
  %180 = bitcast [3 x double]* %4 to double*
  %181 = call fastcc i1 @_ZN3pov13Point_In_ClipEPdPNS_13Object_StructE.2745.extracted(%"struct.pov::Object_Struct"* nonnull %177, double* nonnull %180)
  br i1 %181, label %223, label %182, !prof !31

182:                                              ; preds = %179, %164
  %183 = getelementptr inbounds %"__DTRT_struct.pov::istack_struct.101", %"__DTRT_struct.pov::istack_struct.101"* %2, i64 0, i32 1, !intel-tbaa !32
  %184 = load %"struct.pov::istk_entry.72"*, %"struct.pov::istk_entry.72"** %183, align 8, !tbaa !32
  %185 = getelementptr inbounds %"__DTRT_struct.pov::istack_struct.101", %"__DTRT_struct.pov::istack_struct.101"* %2, i64 0, i32 3
  %186 = load i32, i32* %185, align 4, !tbaa !36
  %187 = zext i32 %186 to i64
  %188 = getelementptr inbounds %"struct.pov::istk_entry.72", %"struct.pov::istk_entry.72"* %184, i64 %187, i32 0
  store double %160, double* %188, align 8, !tbaa !37
  %189 = getelementptr inbounds %"struct.pov::istk_entry.72", %"struct.pov::istk_entry.72"* %184, i64 %187, i32 5
  store %"struct.pov::Object_Struct.99"* %0, %"struct.pov::Object_Struct.99"** %189, align 8, !tbaa !41
  %190 = getelementptr inbounds %"struct.pov::istk_entry.72", %"struct.pov::istk_entry.72"* %184, i64 %187, i32 1
  %191 = bitcast [3 x double]* %4 to i64*
  %192 = load i64, i64* %191, align 16, !tbaa !23
  %193 = bitcast [3 x double]* %190 to i64*
  store i64 %192, i64* %193, align 8, !tbaa !23
  %194 = bitcast [3 x double]* %4 to i8*
  %195 = getelementptr inbounds i8, i8* %194, i64 8
  %196 = bitcast i8* %195 to i64*
  %197 = load i64, i64* %196, align 8, !tbaa !23
  %198 = getelementptr inbounds %"struct.pov::istk_entry.72", %"struct.pov::istk_entry.72"* %184, i64 %187, i32 1, i64 1
  %199 = bitcast double* %198 to i64*
  store i64 %197, i64* %199, align 8, !tbaa !23
  %200 = bitcast [3 x double]* %4 to i8*
  %201 = getelementptr inbounds i8, i8* %200, i64 16
  %202 = bitcast i8* %201 to i64*
  %203 = load i64, i64* %202, align 16, !tbaa !23
  %204 = getelementptr inbounds %"struct.pov::istk_entry.72", %"struct.pov::istk_entry.72"* %184, i64 %187, i32 1, i64 2
  %205 = bitcast double* %204 to i64*
  store i64 %203, i64* %205, align 8, !tbaa !23
  %206 = getelementptr inbounds %"struct.pov::istk_entry.72", %"struct.pov::istk_entry.72"* %184, i64 %187, i32 4
  %207 = load i64, i64* %191, align 16, !tbaa !23
  %208 = bitcast [2 x double]* %206 to i64*
  store i64 %207, i64* %208, align 8, !tbaa !23
  %209 = load i64, i64* %196, align 8, !tbaa !23
  %210 = getelementptr inbounds %"struct.pov::istk_entry.72", %"struct.pov::istk_entry.72"* %184, i64 %187, i32 4, i64 1
  %211 = bitcast double* %210 to i64*
  store i64 %209, i64* %211, align 8, !tbaa !23
  %212 = getelementptr inbounds %"struct.pov::istk_entry.72", %"struct.pov::istk_entry.72"* %184, i64 %187, i32 18
  store i8* null, i8** %212, align 8, !tbaa !42
  %213 = add i32 %186, 1
  store i32 %213, i32* %185, align 4, !tbaa !36
  %214 = getelementptr inbounds %"__DTRT_struct.pov::istack_struct.101", %"__DTRT_struct.pov::istack_struct.101"* %2, i64 0, i32 2
  %215 = load i32, i32* %214, align 8, !tbaa !43
  %216 = icmp ult i32 %213, %215
  br i1 %216, label %223, label %217

217:                                              ; preds = %182
  %218 = bitcast %"__DTRT_struct.pov::istack_struct.101"* %2 to i8*
  %219 = getelementptr inbounds i8, i8* %218, i64 20
  %220 = bitcast i8* %219 to i32*
  store i32 %186, i32* %220, align 4, !tbaa !36
  %221 = load i64, i64* getelementptr inbounds ([123 x i64], [123 x i64]* @_ZN3pov5statsE, i64 0, i64 7), align 8, !tbaa !5
  %222 = add nsw i64 %221, 1
  store i64 %222, i64* getelementptr inbounds ([123 x i64], [123 x i64]* @_ZN3pov5statsE, i64 0, i64 7), align 8, !tbaa !5
  br label %223

223:                                              ; preds = %217, %182, %179, %148, %45, %11
  %224 = phi i32 [ 0, %179 ], [ 1, %182 ], [ 1, %217 ], [ 0, %148 ], [ 0, %45 ], [ 0, %11 ]
  %225 = bitcast [3 x double]* %4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %225) #4
  ret i32 %224
}
; Function Attrs: uwtable
declare hidden fastcc i1 @_ZN3pov13Point_In_ClipEPdPNS_13Object_StructE.2745.extracted(%"struct.pov::Object_Struct"*, double*) unnamed_addr #3

attributes #0 = { argmemonly nounwind willreturn }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "prefer-partial-inline-outlined-func" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{i32 1, !"LTOPostLink", i32 1}
!5 = !{!6, !6, i64 0}
!6 = !{!"long long", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!10, !20, i64 96}
!10 = !{!"struct@_ZTSN3pov12Plane_StructE", !11, i64 0, !12, i64 8, !13, i64 16, !14, i64 24, !14, i64 32, !15, i64 40, !13, i64 48, !13, i64 56, !16, i64 64, !17, i64 72, !20, i64 96, !20, i64 104, !19, i64 112, !12, i64 116, !21, i64 120, !22, i64 144}
!11 = !{!"pointer@_ZTSPN3pov13Method_StructE", !7, i64 0}
!12 = !{!"int", !7, i64 0}
!13 = !{!"pointer@_ZTSPN3pov13Object_StructE", !7, i64 0}
!14 = !{!"pointer@_ZTSPN3pov14Texture_StructE", !7, i64 0}
!15 = !{!"pointer@_ZTSPN3pov15Interior_StructE", !7, i64 0}
!16 = !{!"pointer@_ZTSPN3pov19Light_Source_StructE", !7, i64 0}
!17 = !{!"struct@_ZTSN3pov19Bounding_Box_StructE", !18, i64 0, !18, i64 12}
!18 = !{!"array@_ZTSA3_f", !19, i64 0}
!19 = !{!"float", !7, i64 0}
!20 = !{!"pointer@_ZTSPN3pov16Transform_StructE", !7, i64 0}
!21 = !{!"array@_ZTSA3_d", !22, i64 0}
!22 = !{!"double", !7, i64 0}
!23 = !{!22, !22, i64 0}
!24 = !{!25, !22, i64 128}
!25 = !{!"struct@_ZTSN3pov16Transform_StructE", !26, i64 0, !26, i64 128}
!26 = !{!"array@_ZTSA4_A4_d", !27, i64 0}
!27 = !{!"array@_ZTSA4_d", !22, i64 0}
!28 = !{!10, !22, i64 144}
!29 = !{!30, !13, i64 56}
!30 = !{!"struct@_ZTSN3pov13Object_StructE", !11, i64 0, !12, i64 8, !13, i64 16, !14, i64 24, !14, i64 32, !15, i64 40, !13, i64 48, !13, i64 56, !16, i64 64, !17, i64 72, !20, i64 96, !20, i64 104, !19, i64 112, !12, i64 116}
!31 = !{!"branch_weights", i32 2, i32 2}
!32 = !{!33, !35, i64 8}
!33 = !{!"struct@_ZTSN3pov13istack_structE", !34, i64 0, !35, i64 8, !12, i64 16, !12, i64 20}
!34 = !{!"pointer@_ZTSPN3pov13istack_structE", !7, i64 0}
!35 = !{!"pointer@_ZTSPN3pov10istk_entryE", !7, i64 0}
!36 = !{!33, !12, i64 20}
!37 = !{!38, !22, i64 0}
!38 = !{!"struct@_ZTSN3pov10istk_entryE", !22, i64 0, !21, i64 8, !21, i64 32, !21, i64 56, !39, i64 80, !13, i64 96, !12, i64 104, !12, i64 108, !22, i64 112, !22, i64 120, !22, i64 128, !22, i64 136, !22, i64 144, !22, i64 152, !22, i64 160, !22, i64 168, !22, i64 176, !40, i64 184, !40, i64 192}
!39 = !{!"array@_ZTSA2_d", !22, i64 0}
!40 = !{!"pointer@_ZTSPv", !7, i64 0}
!41 = !{!38, !13, i64 96}
!42 = !{!38, !40, i64 192}
!43 = !{!33, !12, i64 16}
