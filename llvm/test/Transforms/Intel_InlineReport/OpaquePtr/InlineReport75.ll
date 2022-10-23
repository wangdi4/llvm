; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt -opaque-pointers -passes=inline -disable-output -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost < %s 2>&1 | FileCheck %s

; Ensure that inlining of
; _ZN3pov12Ray_In_BoundEPNS_10Ray_StructEPNS_13Object_StructE
; is inhibited on the compile step so that it can participate in Intel Partial
; inlining on the link step.

; CHECK: COMPILE FUNC: _ZN3pov12IntersectionEPNS_10istk_entryEPNS_13Object_StructEPNS_10Ray_StructE
; CHECK: _ZN3pov12Ray_In_BoundEPNS_10Ray_StructEPNS_13Object_StructE{{.*}}Inline decision is delayed until link time

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.pov::Object_Struct" = type { ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, %"struct.pov::Bounding_Box_Struct", ptr, ptr, float, i32 }
%"struct.pov::Bounding_Box_Struct" = type { [3 x float], [3 x float] }
%"struct.pov::Method_Struct" = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%"struct.pov::istk_entry" = type { double, [3 x double], [3 x double], [3 x double], [2 x double], ptr, i32, i32, double, double, double, double, double, double, double, double, double, ptr, ptr }
%"struct.pov::Ray_Struct" = type { [3 x double], [3 x double], i32, i32, [100 x ptr] }
%"struct.pov::istack_struct" = type { ptr, ptr, i32, i32 }

$_ZN3pov9pop_entryEPNS_13istack_structE = comdat any

$_ZN3pov16Increase_CounterERx = comdat any

@_ZN3pov17Number_of_istacksE = dso_local local_unnamed_addr global i32 0, align 4
@_ZN3pov17Max_IntersectionsE = dso_local local_unnamed_addr global i32 64, align 4
@_ZN3pov11free_istackE = dso_local local_unnamed_addr global ptr null, align 8
@_ZN3pov5statsE = external dso_local global [123 x i64], align 16
@.str = private unnamed_addr constant [12 x i8] c"objects.cpp\00", align 1
@.str.1 = private unnamed_addr constant [7 x i8] c"istack\00", align 1
@.str.2 = private unnamed_addr constant [15 x i8] c"istack entries\00", align 1

define dso_local zeroext i1 @_ZN3pov12IntersectionEPNS_10istk_entryEPNS_13Object_StructEPNS_10Ray_StructE(ptr %Ray_Intersection, ptr %Object, ptr %Ray) local_unnamed_addr {
entry:
  %cmp = icmp eq ptr %Object, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %Bound = getelementptr inbounds %"struct.pov::Object_Struct", ptr %Object, i64 0, i32 6
  %i = load ptr, ptr %Bound, align 8
  %call = call zeroext i1 @_ZN3pov12Ray_In_BoundEPNS_10Ray_StructEPNS_13Object_StructE(ptr %Ray, ptr %i)
  br i1 %call, label %if.end2, label %cleanup

if.end2:                                          ; preds = %if.end
  %call3 = call ptr @_ZN3pov11open_istackEv()
  %Methods = getelementptr inbounds %"struct.pov::Object_Struct", ptr %Object, i64 0, i32 0
  %i1 = load ptr, ptr %Methods, align 8
  %All_Intersections_Method = getelementptr inbounds %"struct.pov::Method_Struct", ptr %i1, i64 0, i32 0
  %i2 = load ptr, ptr %All_Intersections_Method, align 8
  %call4 = call i32 %i2(ptr nonnull %Object, ptr %Ray, ptr %call3)
  %tobool = icmp eq i32 %call4, 0
  br i1 %tobool, label %if.else, label %while.cond

while.cond:                                       ; preds = %if.end11, %if.end2
  %Closest.0 = phi double [ 0x7FF0000000000000, %if.end2 ], [ %Closest.1, %if.end11 ]
  %call6 = call ptr @_ZN3pov9pop_entryEPNS_13istack_structE(ptr %call3)
  %cmp7 = icmp eq ptr %call6, null
  br i1 %cmp7, label %while.end, label %while.body

while.body:                                       ; preds = %while.cond
  %Depth = getelementptr inbounds %"struct.pov::istk_entry", ptr %call6, i64 0, i32 0
  %i3 = load double, ptr %Depth, align 8
  %cmp8 = fcmp fast olt double %i3, %Closest.0
  br i1 %cmp8, label %if.then9, label %if.end11

if.then9:                                         ; preds = %while.body
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 dereferenceable(200) %Ray_Intersection, ptr nonnull align 8 dereferenceable(200) %call6, i64 200, i1 false)
  %i6 = load double, ptr %Depth, align 8
  br label %if.end11

if.end11:                                         ; preds = %if.then9, %while.body
  %Closest.1 = phi double [ %i6, %if.then9 ], [ %Closest.0, %while.body ]
  br label %while.cond

while.end:                                        ; preds = %while.cond
  call void @_ZN3pov12close_istackEPNS_13istack_structE(ptr %call3)
  br label %cleanup

if.else:                                          ; preds = %if.end2
  call void @_ZN3pov12close_istackEPNS_13istack_structE(ptr %call3)
  br label %cleanup

cleanup:                                          ; preds = %if.else, %while.end, %if.end, %entry
  %retval.0 = phi i1 [ true, %while.end ], [ false, %if.else ], [ false, %entry ], [ false, %if.end ]
  ret i1 %retval.0
}

define dso_local zeroext i1 @_ZN3pov12Ray_In_BoundEPNS_10Ray_StructEPNS_13Object_StructE(ptr %Ray, ptr %Bounding_Object) local_unnamed_addr {
entry:
  %Local = alloca %"struct.pov::istk_entry", align 8
  call void @llvm.lifetime.start.p0(i64 200, ptr nonnull %Local)
  br label %for.cond

for.cond:                                         ; preds = %if.end3, %entry
  %Bound.0 = phi ptr [ %Bounding_Object, %entry ], [ %i1, %if.end3 ]
  %cmp = icmp eq ptr %Bound.0, null
  br i1 %cmp, label %cleanup, label %for.body

for.body:                                         ; preds = %for.cond
  call void @_ZN3pov16Increase_CounterERx(ptr nonnull align 8 dereferenceable(8) getelementptr inbounds ([123 x i64], ptr @_ZN3pov5statsE, i64 0, i64 88))
  %call = call zeroext i1 @_ZN3pov12IntersectionEPNS_10istk_entryEPNS_13Object_StructEPNS_10Ray_StructE(ptr nonnull %Local, ptr nonnull %Bound.0, ptr %Ray)
  br i1 %call, label %if.end3, label %if.then

if.then:                                          ; preds = %for.body
  %Initial = getelementptr inbounds %"struct.pov::Ray_Struct", ptr %Ray, i64 0, i32 0
  %arraydecay = getelementptr inbounds [3 x double], ptr %Initial, i64 0, i64 0
  %call1 = call zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE(ptr %arraydecay, ptr nonnull %Bound.0)
  br i1 %call1, label %if.end3, label %cleanup

if.end3:                                          ; preds = %if.then, %for.body
  call void @_ZN3pov16Increase_CounterERx(ptr nonnull align 8 dereferenceable(8) getelementptr inbounds ([123 x i64], ptr @_ZN3pov5statsE, i64 0, i64 89))
  %Sibling = getelementptr inbounds %"struct.pov::Object_Struct", ptr %Bound.0, i64 0, i32 2
  %i1 = load ptr, ptr %Sibling, align 8
  br label %for.cond

cleanup:                                          ; preds = %if.then, %for.cond
  call void @llvm.lifetime.end.p0(i64 200, ptr nonnull %Local)
  ret i1 %cmp
}

define dso_local ptr @_ZN3pov11open_istackEv() local_unnamed_addr {
entry:
  %i = load ptr, ptr @_ZN3pov11free_istackE, align 8
  %cmp = icmp eq ptr %i, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call fastcc void @_ZN3povL13create_istackEv()
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %i1 = load ptr, ptr @_ZN3pov11free_istackE, align 8
  %i3 = load i64, ptr %i1, align 8
  store i64 %i3, ptr @_ZN3pov11free_istackE, align 8
  %top_entry = getelementptr inbounds %"struct.pov::istack_struct", ptr %i1, i64 0, i32 3
  store i32 0, ptr %top_entry, align 4
  ret ptr %i1
}

define linkonce_odr dso_local ptr @_ZN3pov9pop_entryEPNS_13istack_structE(ptr %i) local_unnamed_addr comdat {
entry:
  %top_entry = getelementptr inbounds %"struct.pov::istack_struct", ptr %i, i64 0, i32 3
  %i1 = load i32, ptr %top_entry, align 4
  %cmp = icmp eq i32 %i1, 0
  br i1 %cmp, label %cond.end, label %cond.true

cond.true:                                        ; preds = %entry
  %istack = getelementptr inbounds %"struct.pov::istack_struct", ptr %i, i64 0, i32 1
  %i2 = load ptr, ptr %istack, align 8
  %dec = add i32 %i1, -1
  store i32 %dec, ptr %top_entry, align 4
  %idxprom = zext i32 %dec to i64
  %ptridx = getelementptr inbounds %"struct.pov::istk_entry", ptr %i2, i64 %idxprom
  br label %cond.end

cond.end:                                         ; preds = %cond.true, %entry
  %cond = phi ptr [ %ptridx, %cond.true ], [ null, %entry ]
  ret ptr %cond
}

define dso_local void @_ZN3pov12close_istackEPNS_13istack_structE(ptr %istk) local_unnamed_addr {
entry:
  %i = load ptr, ptr @_ZN3pov11free_istackE, align 8
  %next = getelementptr inbounds %"struct.pov::istack_struct", ptr %istk, i64 0, i32 0
  store ptr %i, ptr %next, align 8
  store ptr %istk, ptr @_ZN3pov11free_istackE, align 8
  ret void
}

define dso_local zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE(ptr %IPoint, ptr %Object) local_unnamed_addr {
entry:
  %Clip = getelementptr inbounds %"struct.pov::Object_Struct", ptr %Object, i64 0, i32 7
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %Sib.0.in = phi ptr [ %Clip, %entry ], [ %Sibling, %for.inc ]
  %Sib.0 = load ptr, ptr %Sib.0.in, align 8
  %cmp = icmp eq ptr %Sib.0, null
  br i1 %cmp, label %for.end, label %for.body

for.body:                                         ; preds = %for.cond
  %call = call zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE(ptr %IPoint, ptr nonnull %Sib.0)
  br i1 %call, label %for.inc, label %cleanup

for.inc:                                          ; preds = %for.body
  %Sibling = getelementptr inbounds %"struct.pov::Object_Struct", ptr %Sib.0, i64 0, i32 2
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %Methods = getelementptr inbounds %"struct.pov::Object_Struct", ptr %Object, i64 0, i32 0
  %i = load ptr, ptr %Methods, align 8
  %Inside_Method = getelementptr inbounds %"struct.pov::Method_Struct", ptr %i, i64 0, i32 1
  %i1 = load ptr, ptr %Inside_Method, align 8
  %call1 = call i32 %i1(ptr %IPoint, ptr %Object)
  %tobool = icmp ne i32 %call1, 0
  br label %cleanup

cleanup:                                          ; preds = %for.end, %for.body
  %retval.0 = phi i1 [ %tobool, %for.end ], [ false, %for.body ]
  ret i1 %retval.0
}

define linkonce_odr dso_local void @_ZN3pov16Increase_CounterERx(ptr nonnull align 8 dereferenceable(8) %x) local_unnamed_addr comdat {
entry:
  %i = load i64, ptr %x, align 8
  %inc = add nsw i64 %i, 1
  store i64 %inc, ptr %x, align 8
  ret void
}

define internal fastcc void @_ZN3povL13create_istackEv() unnamed_addr {
entry:
  %call = call ptr @_ZN3pov10pov_mallocEmPKciS1_(i64 24, ptr @.str, i32 834, ptr @.str.1)
  %i1 = load ptr, ptr @_ZN3pov11free_istackE, align 8
  %next = getelementptr inbounds %"struct.pov::istack_struct", ptr %call, i64 0, i32 0
  store ptr %i1, ptr %next, align 8
  store ptr %call, ptr @_ZN3pov11free_istackE, align 8
  %i2 = load i32, ptr @_ZN3pov17Max_IntersectionsE, align 4
  %conv = zext i32 %i2 to i64
  %mul = mul nuw nsw i64 %conv, 200
  %call1 = call ptr @_ZN3pov10pov_mallocEmPKciS1_(i64 %mul, ptr @.str, i32 840, ptr @.str.2)
  %istack = getelementptr inbounds %"struct.pov::istack_struct", ptr %call, i64 0, i32 1
  store ptr %call1, ptr %istack, align 8
  %i4 = load i32, ptr @_ZN3pov17Max_IntersectionsE, align 4
  %max_entries = getelementptr inbounds %"struct.pov::istack_struct", ptr %call, i64 0, i32 2
  store i32 %i4, ptr %max_entries, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %i5 = load i32, ptr %max_entries, align 8
  %cmp = icmp ult i32 %i.0, %i5
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %i6 = load ptr, ptr %istack, align 8
  %idxprom = zext i32 %i.0 to i64
  %ptridx = getelementptr inbounds %"struct.pov::istk_entry", ptr %i6, i64 %idxprom
  %Object = getelementptr inbounds %"struct.pov::istk_entry", ptr %ptridx, i64 0, i32 5
  store ptr null, ptr %Object, align 8
  %inc = add i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %i7 = load i32, ptr @_ZN3pov17Number_of_istacksE, align 4
  %inc4 = add i32 %i7, 1
  store i32 %inc4, ptr @_ZN3pov17Number_of_istacksE, align 4
  ret void
}

declare dso_local ptr @_ZN3pov10pov_mallocEmPKciS1_(i64, ptr, i32, ptr) local_unnamed_addr

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { argmemonly nofree nounwind willreturn }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
; end INTEL_FEATURE_SW_ADVANCED
