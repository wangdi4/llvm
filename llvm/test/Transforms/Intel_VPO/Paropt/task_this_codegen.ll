; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; It checks whether the OMP backend task outlining can handle
; this pointer or not.
; The function _ZN6kdnode11buildkdtreeEPxS0_S0_S0_S0_xxPPii in the following test case is
; extracted from OMP2012/kdtree.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.kdnode = type <{ i64, %class.kdnode*, %class.kdnode*, [3 x i32], [4 x i8] }>

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #0

; Function Attrs: nounwind uwtable
define dso_local void @_ZN6kdnode11buildkdtreeEPxS0_S0_S0_S0_xxPPii(%class.kdnode* %this, i64* %xn, i64* %yn, i64* %zn, i64* %wn, i64* %tn, i64 %start, i64 %end, i32** %x, i32 %p) #1 align 2 {
entry:
  %xn.addr = alloca i64*, align 8
  %yn.addr = alloca i64*, align 8
  %zn.addr = alloca i64*, align 8
  %wn.addr = alloca i64*, align 8
  %tn.addr = alloca i64*, align 8
  %start.addr = alloca i64, align 8
  %end.addr = alloca i64, align 8
  %x.addr = alloca i32**, align 8
  %p.addr = alloca i32, align 4
  %middle = alloca i64, align 8
  %lower = alloca i64, align 8
  store i64* %xn, i64** %xn.addr, align 8, !tbaa !2
  store i64* %yn, i64** %yn.addr, align 8, !tbaa !2
  store i64* %zn, i64** %zn.addr, align 8, !tbaa !2
  store i64* %wn, i64** %wn.addr, align 8, !tbaa !2
  store i64* %tn, i64** %tn.addr, align 8, !tbaa !2
  store i64 %start, i64* %start.addr, align 8, !tbaa !6
  store i64 %end, i64* %end.addr, align 8, !tbaa !6
  store i32** %x, i32*** %x.addr, align 8, !tbaa !8
  store i32 %p, i32* %p.addr, align 4, !tbaa !10
  %0 = bitcast i64* %middle to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #3
  %1 = bitcast i64* %lower to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %1) #3
  %2 = load i32, i32* %p.addr, align 4, !tbaa !10
  %rem = srem i32 %2, 3
  store i32 %rem, i32* %p.addr, align 4, !tbaa !10
  %3 = load i64, i64* %end.addr, align 8, !tbaa !6
  %4 = load i64, i64* %start.addr, align 8, !tbaa !6
  %cmp = icmp eq i64 %3, %4
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %5 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %arrayidx = getelementptr inbounds i64, i64* %5, i64 %4
  %6 = load i64, i64* %arrayidx, align 8, !tbaa !6
  %n = getelementptr inbounds %class.kdnode, %class.kdnode* %this, i32 0, i32 0
  store i64 %6, i64* %n, align 8, !tbaa !12
  br label %if.end130

if.else:                                          ; preds = %entry
  %add = add nsw i64 %4, 1
  %cmp2 = icmp eq i64 %3, %add
  br i1 %cmp2, label %if.then3, label %if.else38

if.then3:                                         ; preds = %if.else
  %7 = load i32**, i32*** %x.addr, align 8, !tbaa !8
  %8 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %arrayidx4 = getelementptr inbounds i64, i64* %8, i64 %4
  %9 = load i64, i64* %arrayidx4, align 8, !tbaa !6
  %arrayidx5 = getelementptr inbounds i32*, i32** %7, i64 %9
  %10 = load i32*, i32** %arrayidx5, align 8, !tbaa !16
  %idxprom = sext i32 %rem to i64
  %arrayidx6 = getelementptr inbounds i32, i32* %10, i64 %idxprom
  %11 = load i32, i32* %arrayidx6, align 4, !tbaa !10
  %arrayidx7 = getelementptr inbounds i64, i64* %8, i64 %3
  %12 = load i64, i64* %arrayidx7, align 8, !tbaa !6
  %arrayidx8 = getelementptr inbounds i32*, i32** %7, i64 %12
  %13 = load i32*, i32** %arrayidx8, align 8, !tbaa !16
  %arrayidx10 = getelementptr inbounds i32, i32* %13, i64 %idxprom
  %14 = load i32, i32* %arrayidx10, align 4, !tbaa !10
  %cmp11 = icmp slt i32 %11, %14
  br i1 %cmp11, label %if.then12, label %if.else16

if.then12:                                        ; preds = %if.then3
  %n14 = getelementptr inbounds %class.kdnode, %class.kdnode* %this, i32 0, i32 0
  store i64 %12, i64* %n14, align 8, !tbaa !12
  %call = call i8* @_Znwm(i64 40) #5
  %15 = bitcast i8* %call to %class.kdnode*
  %16 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %17 = load i64, i64* %start.addr, align 8, !tbaa !6
  %arrayidx15 = getelementptr inbounds i64, i64* %16, i64 %17
  %18 = load i64, i64* %arrayidx15, align 8, !tbaa !6
  call void @_ZN6kdnodeC1Ex(%class.kdnode* %15, i64 %18)
  %lo = getelementptr inbounds %class.kdnode, %class.kdnode* %this, i32 0, i32 1
  store %class.kdnode* %15, %class.kdnode** %lo, align 8, !tbaa !18
  br label %if.end130

if.else16:                                        ; preds = %if.then3
  %cmp25 = icmp sgt i32 %11, %14
  %n28 = getelementptr inbounds %class.kdnode, %class.kdnode* %this, i32 0, i32 0
  store i64 %9, i64* %n28, align 8, !tbaa !12
  %call29 = call i8* @_Znwm(i64 40) #5
  %19 = bitcast i8* %call29 to %class.kdnode*
  %20 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %21 = load i64, i64* %end.addr, align 8, !tbaa !6
  %arrayidx30 = getelementptr inbounds i64, i64* %20, i64 %21
  %22 = load i64, i64* %arrayidx30, align 8, !tbaa !6
  call void @_ZN6kdnodeC1Ex(%class.kdnode* %19, i64 %22)
  br i1 %cmp25, label %if.then26, label %if.else32

if.then26:                                        ; preds = %if.else16
  %lo31 = getelementptr inbounds %class.kdnode, %class.kdnode* %this, i32 0, i32 1
  store %class.kdnode* %19, %class.kdnode** %lo31, align 8, !tbaa !18
  br label %if.end130

if.else32:                                        ; preds = %if.else16
  %hi = getelementptr inbounds %class.kdnode, %class.kdnode* %this, i32 0, i32 2
  store %class.kdnode* %19, %class.kdnode** %hi, align 8, !tbaa !19
  br label %if.end130

if.else38:                                        ; preds = %if.else
  %add39 = add nsw i64 %4, %3
  %div = sdiv i64 %add39, 2
  store i64 %div, i64* %middle, align 8, !tbaa !6
  %23 = load i32**, i32*** %x.addr, align 8, !tbaa !8
  %24 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %arrayidx40 = getelementptr inbounds i64, i64* %24, i64 %div
  %25 = load i64, i64* %arrayidx40, align 8, !tbaa !6
  %arrayidx41 = getelementptr inbounds i32*, i32** %23, i64 %25
  %26 = load i32*, i32** %arrayidx41, align 8, !tbaa !16
  %27 = load i32, i32* %p.addr, align 4, !tbaa !10
  %idxprom42 = sext i32 %27 to i64
  %arrayidx43 = getelementptr inbounds i32, i32* %26, i64 %idxprom42
  %28 = load i32, i32* %arrayidx43, align 4, !tbaa !10
  %conv = sext i32 %28 to i64
  %sub = sub nsw i64 %div, 1
  br label %for.cond

for.cond:                                         ; preds = %if.else52, %if.else38
  %i.0 = phi i64 [ %sub, %if.else38 ], [ %dec, %if.else52 ]
  %29 = load i64, i64* %start.addr, align 8, !tbaa !6
  %cmp44 = icmp sge i64 %i.0, %29
  br i1 %cmp44, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %30 = load i32**, i32*** %x.addr, align 8, !tbaa !8
  %31 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %arrayidx45 = getelementptr inbounds i64, i64* %31, i64 %i.0
  %32 = load i64, i64* %arrayidx45, align 8, !tbaa !6
  %arrayidx46 = getelementptr inbounds i32*, i32** %30, i64 %32
  %33 = load i32*, i32** %arrayidx46, align 8, !tbaa !16
  %34 = load i32, i32* %p.addr, align 4, !tbaa !10
  %idxprom47 = sext i32 %34 to i64
  %arrayidx48 = getelementptr inbounds i32, i32* %33, i64 %idxprom47
  %35 = load i32, i32* %arrayidx48, align 4, !tbaa !10
  %conv49 = sext i32 %35 to i64
  %cmp50 = icmp slt i64 %conv49, %conv
  br i1 %cmp50, label %for.end, label %if.else52

if.else52:                                        ; preds = %for.body
  store i64 %i.0, i64* %middle, align 8, !tbaa !6
  %dec = add nsw i64 %i.0, -1
  br label %for.cond

for.end:                                          ; preds = %for.body, %for.cond
  %36 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %37 = load i64, i64* %middle, align 8, !tbaa !6
  %arrayidx54 = getelementptr inbounds i64, i64* %36, i64 %37
  %38 = load i64, i64* %arrayidx54, align 8, !tbaa !6
  %n55 = getelementptr inbounds %class.kdnode, %class.kdnode* %this, i32 0, i32 0
  store i64 %38, i64* %n55, align 8, !tbaa !12
  %39 = load i64, i64* %start.addr, align 8, !tbaa !6
  %sub56 = sub nsw i64 %39, 1
  store i64 %sub56, i64* %lower, align 8, !tbaa !6
  %40 = load i64, i64* %middle, align 8, !tbaa !6
  %41 = load i64, i64* %start.addr, align 8, !tbaa !6
  br label %for.cond57

for.cond57:                                       ; preds = %for.inc79, %for.end
  %i.1 = phi i64 [ %41, %for.end ], [ %inc80, %for.inc79 ]
  %upper.0 = phi i64 [ %40, %for.end ], [ %upper.1, %for.inc79 ]
  %42 = load i64, i64* %end.addr, align 8, !tbaa !6
  %cmp58 = icmp sle i64 %i.1, %42
  br i1 %cmp58, label %for.body59, label %for.end81

for.body59:                                       ; preds = %for.cond57
  %43 = load i64*, i64** %yn.addr, align 8, !tbaa !2
  %arrayidx60 = getelementptr inbounds i64, i64* %43, i64 %i.1
  %44 = load i64, i64* %arrayidx60, align 8, !tbaa !6
  %45 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %46 = load i64, i64* %middle, align 8, !tbaa !6
  %arrayidx61 = getelementptr inbounds i64, i64* %45, i64 %46
  %47 = load i64, i64* %arrayidx61, align 8, !tbaa !6
  %cmp62 = icmp ne i64 %44, %47
  br i1 %cmp62, label %if.then63, label %for.inc79

if.then63:                                        ; preds = %for.body59
  %48 = load i32**, i32*** %x.addr, align 8, !tbaa !8
  %arrayidx65 = getelementptr inbounds i32*, i32** %48, i64 %44
  %49 = load i32*, i32** %arrayidx65, align 8, !tbaa !16
  %50 = load i32, i32* %p.addr, align 4, !tbaa !10
  %idxprom66 = sext i32 %50 to i64
  %arrayidx67 = getelementptr inbounds i32, i32* %49, i64 %idxprom66
  %51 = load i32, i32* %arrayidx67, align 4, !tbaa !10
  %conv68 = sext i32 %51 to i64
  %cmp69 = icmp slt i64 %conv68, %conv
  %52 = load i64*, i64** %tn.addr, align 8, !tbaa !2
  br i1 %cmp69, label %if.then70, label %if.else73

if.then70:                                        ; preds = %if.then63
  %53 = load i64, i64* %lower, align 8, !tbaa !6
  %inc = add nsw i64 %53, 1
  store i64 %inc, i64* %lower, align 8, !tbaa !6
  %arrayidx72 = getelementptr inbounds i64, i64* %52, i64 %inc
  store i64 %44, i64* %arrayidx72, align 8, !tbaa !6
  br label %for.inc79

if.else73:                                        ; preds = %if.then63
  %inc75 = add nsw i64 %upper.0, 1
  %arrayidx76 = getelementptr inbounds i64, i64* %52, i64 %inc75
  store i64 %44, i64* %arrayidx76, align 8, !tbaa !6
  br label %for.inc79

for.inc79:                                        ; preds = %if.else73, %if.then70, %for.body59
  %upper.1 = phi i64 [ %upper.0, %if.then70 ], [ %inc75, %if.else73 ], [ %upper.0, %for.body59 ]
  %inc80 = add nsw i64 %i.1, 1
  br label %for.cond57

for.end81:                                        ; preds = %for.cond57
  %54 = load i64, i64* %start.addr, align 8, !tbaa !6
  %sub82 = sub nsw i64 %54, 1
  store i64 %sub82, i64* %lower, align 8, !tbaa !6
  %55 = load i64, i64* %middle, align 8, !tbaa !6
  %56 = load i64, i64* %start.addr, align 8, !tbaa !6
  br label %for.cond83

for.cond83:                                       ; preds = %for.inc106, %for.end81
  %i.2 = phi i64 [ %56, %for.end81 ], [ %inc107, %for.inc106 ]
  %upper.2 = phi i64 [ %55, %for.end81 ], [ %upper.3, %for.inc106 ]
  %57 = load i64, i64* %end.addr, align 8, !tbaa !6
  %cmp84 = icmp sle i64 %i.2, %57
  br i1 %cmp84, label %for.body85, label %for.end108

for.body85:                                       ; preds = %for.cond83
  %58 = load i64*, i64** %zn.addr, align 8, !tbaa !2
  %arrayidx86 = getelementptr inbounds i64, i64* %58, i64 %i.2
  %59 = load i64, i64* %arrayidx86, align 8, !tbaa !6
  %60 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %61 = load i64, i64* %middle, align 8, !tbaa !6
  %arrayidx87 = getelementptr inbounds i64, i64* %60, i64 %61
  %62 = load i64, i64* %arrayidx87, align 8, !tbaa !6
  %cmp88 = icmp ne i64 %59, %62
  br i1 %cmp88, label %if.then89, label %for.inc106

if.then89:                                        ; preds = %for.body85
  %63 = load i32**, i32*** %x.addr, align 8, !tbaa !8
  %arrayidx91 = getelementptr inbounds i32*, i32** %63, i64 %59
  %64 = load i32*, i32** %arrayidx91, align 8, !tbaa !16
  %65 = load i32, i32* %p.addr, align 4, !tbaa !10
  %idxprom92 = sext i32 %65 to i64
  %arrayidx93 = getelementptr inbounds i32, i32* %64, i64 %idxprom92
  %66 = load i32, i32* %arrayidx93, align 4, !tbaa !10
  %conv94 = sext i32 %66 to i64
  %cmp95 = icmp slt i64 %conv94, %conv
  %67 = load i64*, i64** %yn.addr, align 8, !tbaa !2
  br i1 %cmp95, label %if.then96, label %if.else100

if.then96:                                        ; preds = %if.then89
  %68 = load i64, i64* %lower, align 8, !tbaa !6
  %inc98 = add nsw i64 %68, 1
  store i64 %inc98, i64* %lower, align 8, !tbaa !6
  %arrayidx99 = getelementptr inbounds i64, i64* %67, i64 %inc98
  store i64 %59, i64* %arrayidx99, align 8, !tbaa !6
  br label %for.inc106

if.else100:                                       ; preds = %if.then89
  %inc102 = add nsw i64 %upper.2, 1
  %arrayidx103 = getelementptr inbounds i64, i64* %67, i64 %inc102
  store i64 %59, i64* %arrayidx103, align 8, !tbaa !6
  br label %for.inc106

for.inc106:                                       ; preds = %if.else100, %if.then96, %for.body85
  %upper.3 = phi i64 [ %upper.2, %if.then96 ], [ %inc102, %if.else100 ], [ %upper.2, %for.body85 ]
  %inc107 = add nsw i64 %i.2, 1
  br label %for.cond83

for.end108:                                       ; preds = %for.cond83
  %69 = load i64, i64* %lower, align 8, !tbaa !6
  %70 = load i64, i64* %start.addr, align 8, !tbaa !6
  %cmp109 = icmp sge i64 %69, %70
  br i1 %cmp109, label %if.then110, label %if.end117

if.then110:                                       ; preds = %for.end108
  %call111 = call i8* @_Znwm(i64 40) #5
  %71 = bitcast i8* %call111 to %class.kdnode*
  call void @_ZN6kdnodeC1Ev(%class.kdnode* %71)
  %lo112 = getelementptr inbounds %class.kdnode, %class.kdnode* %this, i32 0, i32 1
  store %class.kdnode* %71, %class.kdnode** %lo112, align 8, !tbaa !18
  %72 = load i64, i64* %lower, align 8, !tbaa !6
  %73 = load i64, i64* %start.addr, align 8, !tbaa !6
  %sub113 = sub nsw i64 %72, %73
  %cmp114 = icmp sge i64 %sub113, 16
  br label %DIR.OMP.TASK.1

DIR.OMP.TASK.1:                                   ; preds = %if.then110
  %74 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.IF"(i1 %cmp114), "QUAL.OMP.SHARED"(i64** %tn.addr, i64** %yn.addr, i64** %xn.addr, i64** %wn.addr, i64** %zn.addr, i32*** %x.addr), "QUAL.OMP.FIRSTPRIVATE"(i64* %start.addr), "QUAL.OMP.FIRSTPRIVATE"(i64* %lower), "QUAL.OMP.FIRSTPRIVATE"(i32* %p.addr), "QUAL.OMP.SHARED"(%class.kdnode* %this) ]
  %75 = bitcast %class.kdnode* %this to i8*
  %76 = call i8* @llvm.launder.invariant.group.p0i8(i8* %75)
  %77 = bitcast i8* %76 to %class.kdnode*
  %lo115 = getelementptr inbounds %class.kdnode, %class.kdnode* %77, i32 0, i32 1
  %78 = load %class.kdnode*, %class.kdnode** %lo115, align 8, !tbaa !18
  %79 = load i64*, i64** %tn.addr, align 8, !tbaa !2
  %80 = load i64*, i64** %yn.addr, align 8, !tbaa !2
  %81 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %82 = load i64*, i64** %wn.addr, align 8, !tbaa !2
  %83 = load i64*, i64** %zn.addr, align 8, !tbaa !2
  %84 = load i64, i64* %start.addr, align 8, !tbaa !6
  %85 = load i64, i64* %lower, align 8, !tbaa !6
  %86 = load i32**, i32*** %x.addr, align 8, !tbaa !8
  %87 = load i32, i32* %p.addr, align 4, !tbaa !10
  %add116 = add nsw i32 %87, 1
  call void @_ZN6kdnode11buildkdtreeEPxS0_S0_S0_S0_xxPPii(%class.kdnode* %78, i64* %79, i64* %80, i64* %81, i64* %82, i64* %83, i64 %84, i64 %85, i32** %86, i32 %add116)
  br label %DIR.OMP.END.TASK.3

DIR.OMP.END.TASK.3:                               ; preds = %DIR.OMP.TASK.1
  call void @llvm.directive.region.exit(token %74) [ "DIR.OMP.END.TASK"() ]
  br label %if.end117

if.end117:                                        ; preds = %DIR.OMP.END.TASK.3, %for.end108
  %88 = load i64, i64* %middle, align 8, !tbaa !6
  %cmp118 = icmp sgt i64 %upper.2, %88
  br i1 %cmp118, label %if.then119, label %DIR.OMP.TASKWAIT.7

if.then119:                                       ; preds = %if.end117
  %call120 = call i8* @_Znwm(i64 40) #5
  %89 = bitcast i8* %call120 to %class.kdnode*
  call void @_ZN6kdnodeC1Ev(%class.kdnode* %89)
  %hi121 = getelementptr inbounds %class.kdnode, %class.kdnode* %this, i32 0, i32 2
  store %class.kdnode* %89, %class.kdnode** %hi121, align 8, !tbaa !19
  %90 = load i64, i64* %end.addr, align 8, !tbaa !6
  %91 = load i64, i64* %middle, align 8, !tbaa !6
  %add122 = add nsw i64 %91, 1
  %sub123 = sub nsw i64 %90, %add122
  %cmp124 = icmp sge i64 %sub123, 16
  br label %DIR.OMP.TASK.4

DIR.OMP.TASK.4:                                   ; preds = %if.then119
  %92 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.IF"(i1 %cmp124), "QUAL.OMP.SHARED"(i64** %tn.addr, i64** %yn.addr, i64** %xn.addr, i64** %wn.addr, i64** %zn.addr, i32*** %x.addr), "QUAL.OMP.FIRSTPRIVATE"(i64* %middle), "QUAL.OMP.FIRSTPRIVATE"(i64* %end.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %p.addr), "QUAL.OMP.SHARED"(%class.kdnode* %this) ]
  %93 = bitcast %class.kdnode* %this to i8*
  %94 = call i8* @llvm.launder.invariant.group.p0i8(i8* %93)
  %95 = bitcast i8* %94 to %class.kdnode*
  %hi125 = getelementptr inbounds %class.kdnode, %class.kdnode* %95, i32 0, i32 2
  %96 = load %class.kdnode*, %class.kdnode** %hi125, align 8, !tbaa !19
  %97 = load i64*, i64** %tn.addr, align 8, !tbaa !2
  %98 = load i64*, i64** %yn.addr, align 8, !tbaa !2
  %99 = load i64*, i64** %xn.addr, align 8, !tbaa !2
  %100 = load i64*, i64** %wn.addr, align 8, !tbaa !2
  %101 = load i64*, i64** %zn.addr, align 8, !tbaa !2
  %102 = load i64, i64* %middle, align 8, !tbaa !6
  %add126 = add nsw i64 %102, 1
  %103 = load i64, i64* %end.addr, align 8, !tbaa !6
  %104 = load i32**, i32*** %x.addr, align 8, !tbaa !8
  %105 = load i32, i32* %p.addr, align 4, !tbaa !10
  %add127 = add nsw i32 %105, 1
  call void @_ZN6kdnode11buildkdtreeEPxS0_S0_S0_S0_xxPPii(%class.kdnode* %96, i64* %97, i64* %98, i64* %99, i64* %100, i64* %101, i64 %add126, i64 %103, i32** %104, i32 %add127)
  br label %DIR.OMP.END.TASK.6

DIR.OMP.END.TASK.6:                               ; preds = %DIR.OMP.TASK.4
  call void @llvm.directive.region.exit(token %92) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.TASKWAIT.7

DIR.OMP.TASKWAIT.7:                               ; preds = %DIR.OMP.END.TASK.6, %if.end117
  %106 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"() ]
  br label %DIR.OMP.TASKWAIT.8

DIR.OMP.TASKWAIT.8:                               ; preds = %DIR.OMP.TASKWAIT.7
  call void @llvm.directive.region.exit(token %106) [ "DIR.OMP.END.TASKWAIT"() ]
  br label %if.end130

if.end130:                                        ; preds = %DIR.OMP.TASKWAIT.8, %if.else32, %if.then26, %if.then12, %if.then
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %1) #3
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %0) #3
  ret void
}

; Function Attrs: nobuiltin
declare dso_local noalias i8* @_Znwm(i64) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #4

declare void @_ZN6kdnodeC1Ev(%class.kdnode*)

declare void @_ZN6kdnodeC1Ex(%class.kdnode*, i64)

attributes #0 = { argmemonly nounwind }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { inaccessiblememonly nounwind speculatable }
attributes #5 = { builtin }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPx", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"long long", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPPi", !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !4, i64 0}
!12 = !{!13, !7, i64 0}
!13 = !{!"struct@_ZTS6kdnode", !7, i64 0, !14, i64 8, !14, i64 16, !15, i64 24}
!14 = !{!"unspecified pointer", !4, i64 0}
!15 = !{!"array@_ZTSA3_i", !11, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"pointer@_ZTSPi", !4, i64 0}
!18 = !{!13, !14, i64 8}
!19 = !{!13, !14, i64 16}

; CHECK:  store %class.kdnode* %this, %class.kdnode** %{{.*}}
