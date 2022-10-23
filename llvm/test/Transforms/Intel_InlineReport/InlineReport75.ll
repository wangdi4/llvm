; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt -passes=inline -disable-output -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost < %s 2>&1 | FileCheck %s

; Ensure that inlining of
; _ZN3pov12Ray_In_BoundEPNS_10Ray_StructEPNS_13Object_StructE
; is inhibited on the compile step so that it can participate in Intel Partial
; inlining on the link step.

; CHECK: COMPILE FUNC: _ZN3pov12IntersectionEPNS_10istk_entryEPNS_13Object_StructEPNS_10Ray_StructE
; CHECK: _ZN3pov12Ray_In_BoundEPNS_10Ray_StructEPNS_13Object_StructE{{.*}}Inline decision is delayed until link time

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.pov::istack_struct" = type { %"struct.pov::istack_struct"*, %"struct.pov::istk_entry"*, i32, i32 }
%"struct.pov::istk_entry" = type { double, [3 x double], [3 x double], [3 x double], [2 x double], %"struct.pov::Object_Struct"*, i32, i32, double, double, double, double, double, double, double, double, double, i8*, i8* }
%"struct.pov::Object_Struct" = type { %"struct.pov::Method_Struct"*, i32, %"struct.pov::Object_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Interior_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Light_Source_Struct"*, %"struct.pov::Bounding_Box_Struct", %"struct.pov::Transform_Struct"*, %"struct.pov::Transform_Struct"*, float, i32 }
%"struct.pov::Method_Struct" = type { i32 (%"struct.pov::Object_Struct"*, %"struct.pov::Ray_Struct"*, %"struct.pov::istack_struct"*)*, i32 (double*, %"struct.pov::Object_Struct"*)*, void (double*, %"struct.pov::Object_Struct"*, %"struct.pov::istk_entry"*)*, void (double*, %"struct.pov::Object_Struct"*, %"struct.pov::istk_entry"*)*, i8* (%"struct.pov::Object_Struct"*)*, void (%"struct.pov::Object_Struct"*, double*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct"*, double*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct"*, double*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct"*, %"struct.pov::Transform_Struct"*)*, void (%"struct.pov::Object_Struct"*)*, void (%"struct.pov::Object_Struct"*)* }
%"struct.pov::Ray_Struct" = type { [3 x double], [3 x double], i32, i32, [100 x %"struct.pov::Interior_Struct"*] }
%"struct.pov::Texture_Struct" = type { i16, i16, i16, i32, float, float, float, %"struct.pov::Warps_Struct"*, %"struct.pov::Pattern_Struct"*, %"struct.pov::Blend_Map_Struct"*, %union.anon.9, %"struct.pov::Texture_Struct"*, %"struct.pov::Pigment_Struct"*, %"struct.pov::Tnormal_Struct"*, %"struct.pov::Finish_Struct"*, %"struct.pov::Texture_Struct"*, i32 }
%"struct.pov::Warps_Struct" = type { i16, %"struct.pov::Warps_Struct"*, %"struct.pov::Warps_Struct"* }
%"struct.pov::Pattern_Struct" = type { i16, i16, i16, i32, float, float, float, %"struct.pov::Warps_Struct"*, %"struct.pov::Pattern_Struct"*, %"struct.pov::Blend_Map_Struct"*, %union.anon.17 }
%union.anon.17 = type { %struct.anon.21 }
%struct.anon.21 = type { [3 x double], [3 x double], double, double, i16, [3 x double]*, i32, [3 x double] }
%"struct.pov::Blend_Map_Struct" = type { i32, i16, i8, i8, %"struct.pov::Blend_Map_Entry"* }
%"struct.pov::Blend_Map_Entry" = type { float, i8, %union.anon }
%union.anon = type { [2 x double], [8 x i8] }
%union.anon.9 = type { %struct.anon.13 }
%struct.anon.13 = type { [3 x double], [3 x double], double, double, i16, [3 x double]*, i32, [3 x double] }
%"struct.pov::Pigment_Struct" = type { i16, i16, i16, i32, float, float, float, %"struct.pov::Warps_Struct"*, %"struct.pov::Pattern_Struct"*, %"struct.pov::Blend_Map_Struct"*, %union.anon.25, [5 x float] }
%union.anon.25 = type { %struct.anon.29 }
%struct.anon.29 = type { [3 x double], [3 x double], double, double, i16, [3 x double]*, i32, [3 x double] }
%"struct.pov::Tnormal_Struct" = type { i16, i16, i16, i32, float, float, float, %"struct.pov::Warps_Struct"*, %"struct.pov::Pattern_Struct"*, %"struct.pov::Blend_Map_Struct"*, %union.anon.0, float, float }
%union.anon.0 = type { %struct.anon.5 }
%struct.anon.5 = type { [3 x double], [3 x double], double, double, i16, [3 x double]*, i32, [3 x double] }
%"struct.pov::Finish_Struct" = type { float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, [3 x float], [3 x float], [3 x float], float, i32, float, i32 }
%"struct.pov::Interior_Struct" = type { i32, i32, i32, float, float, float, float, float, float, [5 x float], %"struct.pov::Media_Struct"* }
%"struct.pov::Media_Struct" = type { i32, i32, i32, i32, i32, double, i32, double, double, i32, i32, i32, i32, [5 x float], [5 x float], [5 x float], [5 x float], double, double, double, double*, double, i32, i32, %"struct.pov::Pigment_Struct"*, %"struct.pov::Media_Struct"* }
%"struct.pov::Light_Source_Struct" = type { %"struct.pov::Method_Struct"*, i32, %"struct.pov::Object_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Texture_Struct"*, %"struct.pov::Interior_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Light_Source_Struct"*, %"struct.pov::Bounding_Box_Struct", %"struct.pov::Transform_Struct"*, %"struct.pov::Transform_Struct"*, float, i32, %"struct.pov::Object_Struct"*, [5 x float], [3 x double], [3 x double], [3 x double], [3 x double], [3 x double], double, double, double, double, double, %"struct.pov::Light_Source_Struct"*, i8, i8, i8, i8, i8, i8, i8, i8, i32, i32, i32, i32, i32, [5 x float]**, %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"*, %"struct.pov::Blend_Map_Struct"*, [6 x %"struct.pov::Project_Tree_Node_Struct"*] }
%"struct.pov::Project_Tree_Node_Struct" = type { i16, %"struct.pov::BBox_Tree_Struct"*, %"struct.pov::Project_Struct", i16, %"struct.pov::Project_Tree_Node_Struct"** }
%"struct.pov::BBox_Tree_Struct" = type { i16, i16, %"struct.pov::Bounding_Box_Struct", %"struct.pov::BBox_Tree_Struct"** }
%"struct.pov::Project_Struct" = type { i32, i32, i32, i32 }
%"struct.pov::Bounding_Box_Struct" = type { [3 x float], [3 x float] }
%"struct.pov::Transform_Struct" = type { [4 x [4 x double]], [4 x [4 x double]] }

$_ZN3pov9pop_entryEPNS_13istack_structE = comdat any

$_ZN3pov16Increase_CounterERx = comdat any

@_ZN3pov17Number_of_istacksE = dso_local local_unnamed_addr global i32 0, align 4
@_ZN3pov17Max_IntersectionsE = dso_local local_unnamed_addr global i32 64, align 4
@_ZN3pov11free_istackE = dso_local local_unnamed_addr global %"struct.pov::istack_struct"* null, align 8
@_ZN3pov5statsE = external dso_local global [123 x i64], align 16
@.str = private unnamed_addr constant [12 x i8] c"objects.cpp\00", align 1
@.str.1 = private unnamed_addr constant [7 x i8] c"istack\00", align 1
@.str.2 = private unnamed_addr constant [15 x i8] c"istack entries\00", align 1

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly %0, i8* noalias nocapture readonly %1, i64 %2, i1 immarg %3)

define dso_local zeroext i1 @_ZN3pov12IntersectionEPNS_10istk_entryEPNS_13Object_StructEPNS_10Ray_StructE(%"struct.pov::istk_entry"* %Ray_Intersection, %"struct.pov::Object_Struct"* %Object, %"struct.pov::Ray_Struct"* %Ray) local_unnamed_addr {
entry:
  %cmp = icmp eq %"struct.pov::Object_Struct"* %Object, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %Bound = getelementptr inbounds %"struct.pov::Object_Struct", %"struct.pov::Object_Struct"* %Object, i64 0, i32 6
  %0 = load %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"** %Bound, align 8
  %call = call zeroext i1 @_ZN3pov12Ray_In_BoundEPNS_10Ray_StructEPNS_13Object_StructE(%"struct.pov::Ray_Struct"* %Ray, %"struct.pov::Object_Struct"* %0)
  br i1 %call, label %if.end2, label %cleanup

if.end2:                                          ; preds = %if.end
  %call3 = call %"struct.pov::istack_struct"* @_ZN3pov11open_istackEv()
  %Methods = getelementptr inbounds %"struct.pov::Object_Struct", %"struct.pov::Object_Struct"* %Object, i64 0, i32 0
  %1 = load %"struct.pov::Method_Struct"*, %"struct.pov::Method_Struct"** %Methods, align 8
  %All_Intersections_Method = getelementptr inbounds %"struct.pov::Method_Struct", %"struct.pov::Method_Struct"* %1, i64 0, i32 0
  %2 = load i32 (%"struct.pov::Object_Struct"*, %"struct.pov::Ray_Struct"*, %"struct.pov::istack_struct"*)*, i32 (%"struct.pov::Object_Struct"*, %"struct.pov::Ray_Struct"*, %"struct.pov::istack_struct"*)** %All_Intersections_Method, align 8
  %call4 = call i32 %2(%"struct.pov::Object_Struct"* nonnull %Object, %"struct.pov::Ray_Struct"* %Ray, %"struct.pov::istack_struct"* %call3)
  %tobool = icmp eq i32 %call4, 0
  br i1 %tobool, label %if.else, label %while.cond

while.cond:                                       ; preds = %if.end2, %if.end11
  %Closest.0 = phi double [ 0x7FF0000000000000, %if.end2 ], [ %Closest.1, %if.end11 ]
  %call6 = call %"struct.pov::istk_entry"* @_ZN3pov9pop_entryEPNS_13istack_structE(%"struct.pov::istack_struct"* %call3)
  %cmp7 = icmp eq %"struct.pov::istk_entry"* %call6, null
  br i1 %cmp7, label %while.end, label %while.body

while.body:                                       ; preds = %while.cond
  %Depth = getelementptr inbounds %"struct.pov::istk_entry", %"struct.pov::istk_entry"* %call6, i64 0, i32 0
  %3 = load double, double* %Depth, align 8
  %cmp8 = fcmp fast olt double %3, %Closest.0
  br i1 %cmp8, label %if.then9, label %if.end11

if.then9:                                         ; preds = %while.body
  %4 = bitcast %"struct.pov::istk_entry"* %Ray_Intersection to i8*
  %5 = bitcast %"struct.pov::istk_entry"* %call6 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 dereferenceable(200) %4, i8* nonnull align 8 dereferenceable(200) %5, i64 200, i1 false)
  %6 = load double, double* %Depth, align 8
  br label %if.end11

if.end11:                                         ; preds = %if.then9, %while.body
  %Closest.1 = phi double [ %6, %if.then9 ], [ %Closest.0, %while.body ]
  br label %while.cond

while.end:                                        ; preds = %while.cond
  call void @_ZN3pov12close_istackEPNS_13istack_structE(%"struct.pov::istack_struct"* %call3)
  br label %cleanup

if.else:                                          ; preds = %if.end2
  call void @_ZN3pov12close_istackEPNS_13istack_structE(%"struct.pov::istack_struct"* %call3)
  br label %cleanup

cleanup:                                          ; preds = %if.end, %entry, %if.else, %while.end
  %retval.0 = phi i1 [ true, %while.end ], [ false, %if.else ], [ false, %entry ], [ false, %if.end ]
  ret i1 %retval.0
}

declare void @llvm.lifetime.start.p0i8(i64 immarg %0, i8* nocapture %1)

define dso_local zeroext i1 @_ZN3pov12Ray_In_BoundEPNS_10Ray_StructEPNS_13Object_StructE(%"struct.pov::Ray_Struct"* %Ray, %"struct.pov::Object_Struct"* %Bounding_Object) local_unnamed_addr {
entry:
  %Local = alloca %"struct.pov::istk_entry", align 8
  %0 = bitcast %"struct.pov::istk_entry"* %Local to i8*
  call void @llvm.lifetime.start.p0i8(i64 200, i8* nonnull %0)
  br label %for.cond

for.cond:                                         ; preds = %if.end3, %entry
  %Bound.0 = phi %"struct.pov::Object_Struct"* [ %Bounding_Object, %entry ], [ %1, %if.end3 ]
  %cmp = icmp eq %"struct.pov::Object_Struct"* %Bound.0, null
  br i1 %cmp, label %cleanup, label %for.body

for.body:                                         ; preds = %for.cond
  call void @_ZN3pov16Increase_CounterERx(i64* nonnull align 8 dereferenceable(8) getelementptr inbounds ([123 x i64], [123 x i64]* @_ZN3pov5statsE, i64 0, i64 88))
  %call = call zeroext i1 @_ZN3pov12IntersectionEPNS_10istk_entryEPNS_13Object_StructEPNS_10Ray_StructE(%"struct.pov::istk_entry"* nonnull %Local, %"struct.pov::Object_Struct"* nonnull %Bound.0, %"struct.pov::Ray_Struct"* %Ray)
  br i1 %call, label %if.end3, label %if.then

if.then:                                          ; preds = %for.body
  %Initial = getelementptr inbounds %"struct.pov::Ray_Struct", %"struct.pov::Ray_Struct"* %Ray, i64 0, i32 0
  %arraydecay = getelementptr inbounds [3 x double], [3 x double]* %Initial, i64 0, i64 0
  %call1 = call zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE(double* %arraydecay, %"struct.pov::Object_Struct"* nonnull %Bound.0)
  br i1 %call1, label %if.end3, label %cleanup

if.end3:                                          ; preds = %if.then, %for.body
  call void @_ZN3pov16Increase_CounterERx(i64* nonnull align 8 dereferenceable(8) getelementptr inbounds ([123 x i64], [123 x i64]* @_ZN3pov5statsE, i64 0, i64 89))
  %Sibling = getelementptr inbounds %"struct.pov::Object_Struct", %"struct.pov::Object_Struct"* %Bound.0, i64 0, i32 2
  %1 = load %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"** %Sibling, align 8
  br label %for.cond

cleanup:                                          ; preds = %for.cond, %if.then
  call void @llvm.lifetime.end.p0i8(i64 200, i8* nonnull %0)
  ret i1 %cmp
}

define dso_local %"struct.pov::istack_struct"* @_ZN3pov11open_istackEv() local_unnamed_addr {
entry:
  %0 = load %"struct.pov::istack_struct"*, %"struct.pov::istack_struct"** @_ZN3pov11free_istackE, align 8
  %cmp = icmp eq %"struct.pov::istack_struct"* %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call fastcc void @_ZN3povL13create_istackEv()
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %1 = load %"struct.pov::istack_struct"*, %"struct.pov::istack_struct"** @_ZN3pov11free_istackE, align 8
  %2 = bitcast %"struct.pov::istack_struct"* %1 to i64*
  %3 = load i64, i64* %2, align 8
  store i64 %3, i64* bitcast (%"struct.pov::istack_struct"** @_ZN3pov11free_istackE to i64*), align 8
  %top_entry = getelementptr inbounds %"struct.pov::istack_struct", %"struct.pov::istack_struct"* %1, i64 0, i32 3
  store i32 0, i32* %top_entry, align 4
  ret %"struct.pov::istack_struct"* %1
}

define linkonce_odr dso_local %"struct.pov::istk_entry"* @_ZN3pov9pop_entryEPNS_13istack_structE(%"struct.pov::istack_struct"* %i) local_unnamed_addr comdat {
entry:
  %top_entry = getelementptr inbounds %"struct.pov::istack_struct", %"struct.pov::istack_struct"* %i, i64 0, i32 3
  %0 = load i32, i32* %top_entry, align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %cond.end, label %cond.true

cond.true:                                        ; preds = %entry
  %istack = getelementptr inbounds %"struct.pov::istack_struct", %"struct.pov::istack_struct"* %i, i64 0, i32 1
  %1 = load %"struct.pov::istk_entry"*, %"struct.pov::istk_entry"** %istack, align 8
  %dec = add i32 %0, -1
  store i32 %dec, i32* %top_entry, align 4
  %idxprom = zext i32 %dec to i64
  %ptridx = getelementptr inbounds %"struct.pov::istk_entry", %"struct.pov::istk_entry"* %1, i64 %idxprom
  br label %cond.end

cond.end:                                         ; preds = %entry, %cond.true
  %cond = phi %"struct.pov::istk_entry"* [ %ptridx, %cond.true ], [ null, %entry ]
  ret %"struct.pov::istk_entry"* %cond
}

define dso_local void @_ZN3pov12close_istackEPNS_13istack_structE(%"struct.pov::istack_struct"* %istk) local_unnamed_addr {
entry:
  %0 = load %"struct.pov::istack_struct"*, %"struct.pov::istack_struct"** @_ZN3pov11free_istackE, align 8
  %next = getelementptr inbounds %"struct.pov::istack_struct", %"struct.pov::istack_struct"* %istk, i64 0, i32 0
  store %"struct.pov::istack_struct"* %0, %"struct.pov::istack_struct"** %next, align 8
  store %"struct.pov::istack_struct"* %istk, %"struct.pov::istack_struct"** @_ZN3pov11free_istackE, align 8
  ret void
}

declare void @llvm.lifetime.end.p0i8(i64 immarg %0, i8* nocapture %1)

define dso_local zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE(double* %IPoint, %"struct.pov::Object_Struct"* %Object) local_unnamed_addr {
entry:
  %Clip = getelementptr inbounds %"struct.pov::Object_Struct", %"struct.pov::Object_Struct"* %Object, i64 0, i32 7
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %Sib.0.in = phi %"struct.pov::Object_Struct"** [ %Clip, %entry ], [ %Sibling, %for.inc ]
  %Sib.0 = load %"struct.pov::Object_Struct"*, %"struct.pov::Object_Struct"** %Sib.0.in, align 8
  %cmp = icmp eq %"struct.pov::Object_Struct"* %Sib.0, null
  br i1 %cmp, label %for.end, label %for.body

for.body:                                         ; preds = %for.cond
  %call = call zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE(double* %IPoint, %"struct.pov::Object_Struct"* nonnull %Sib.0)
  br i1 %call, label %for.inc, label %cleanup

for.inc:                                          ; preds = %for.body
  %Sibling = getelementptr inbounds %"struct.pov::Object_Struct", %"struct.pov::Object_Struct"* %Sib.0, i64 0, i32 2
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %Methods = getelementptr inbounds %"struct.pov::Object_Struct", %"struct.pov::Object_Struct"* %Object, i64 0, i32 0
  %0 = load %"struct.pov::Method_Struct"*, %"struct.pov::Method_Struct"** %Methods, align 8
  %Inside_Method = getelementptr inbounds %"struct.pov::Method_Struct", %"struct.pov::Method_Struct"* %0, i64 0, i32 1
  %1 = load i32 (double*, %"struct.pov::Object_Struct"*)*, i32 (double*, %"struct.pov::Object_Struct"*)** %Inside_Method, align 8
  %call1 = call i32 %1(double* %IPoint, %"struct.pov::Object_Struct"* %Object)
  %tobool = icmp ne i32 %call1, 0
  br label %cleanup

cleanup:                                          ; preds = %for.body, %for.end
  %retval.0 = phi i1 [ %tobool, %for.end ], [ false, %for.body ]
  ret i1 %retval.0
}

define linkonce_odr dso_local void @_ZN3pov16Increase_CounterERx(i64* nonnull align 8 dereferenceable(8) %x) local_unnamed_addr comdat {
entry:
  %0 = load i64, i64* %x, align 8
  %inc = add nsw i64 %0, 1
  store i64 %inc, i64* %x, align 8
  ret void
}

define internal fastcc void @_ZN3povL13create_istackEv() unnamed_addr {
entry:
  %call = call i8* @_ZN3pov10pov_mallocEmPKciS1_(i64 24, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i64 0, i64 0), i32 834, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.1, i64 0, i64 0))
  %0 = bitcast i8* %call to %"struct.pov::istack_struct"*
  %1 = load %"struct.pov::istack_struct"*, %"struct.pov::istack_struct"** @_ZN3pov11free_istackE, align 8
  %next = getelementptr inbounds %"struct.pov::istack_struct", %"struct.pov::istack_struct"* %0, i64 0, i32 0
  store %"struct.pov::istack_struct"* %1, %"struct.pov::istack_struct"** %next, align 8
  store %"struct.pov::istack_struct"* %0, %"struct.pov::istack_struct"** @_ZN3pov11free_istackE, align 8
  %2 = load i32, i32* @_ZN3pov17Max_IntersectionsE, align 4
  %conv = zext i32 %2 to i64
  %mul = mul nuw nsw i64 %conv, 200
  %call1 = call i8* @_ZN3pov10pov_mallocEmPKciS1_(i64 %mul, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i64 0, i64 0), i32 840, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str.2, i64 0, i64 0))
  %3 = bitcast i8* %call1 to %"struct.pov::istk_entry"*
  %istack = getelementptr inbounds %"struct.pov::istack_struct", %"struct.pov::istack_struct"* %0, i64 0, i32 1
  store %"struct.pov::istk_entry"* %3, %"struct.pov::istk_entry"** %istack, align 8
  %4 = load i32, i32* @_ZN3pov17Max_IntersectionsE, align 4
  %max_entries = getelementptr inbounds %"struct.pov::istack_struct", %"struct.pov::istack_struct"* %0, i64 0, i32 2
  store i32 %4, i32* %max_entries, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %5 = load i32, i32* %max_entries, align 8
  %cmp = icmp ult i32 %i.0, %5
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %6 = load %"struct.pov::istk_entry"*, %"struct.pov::istk_entry"** %istack, align 8
  %idxprom = zext i32 %i.0 to i64
  %ptridx = getelementptr inbounds %"struct.pov::istk_entry", %"struct.pov::istk_entry"* %6, i64 %idxprom
  %Object = getelementptr inbounds %"struct.pov::istk_entry", %"struct.pov::istk_entry"* %ptridx, i64 0, i32 5
  store %"struct.pov::Object_Struct"* null, %"struct.pov::Object_Struct"** %Object, align 8
  %inc = add i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %7 = load i32, i32* @_ZN3pov17Number_of_istacksE, align 4
  %inc4 = add i32 %7, 1
  store i32 %inc4, i32* @_ZN3pov17Number_of_istacksE, align 4
  ret void
}

declare dso_local i8* @_ZN3pov10pov_mallocEmPKciS1_(i64 %0, i8* %1, i32 %2, i8* %3) local_unnamed_addr

; end INTEL_FEATURE_SW_ADVANCED
