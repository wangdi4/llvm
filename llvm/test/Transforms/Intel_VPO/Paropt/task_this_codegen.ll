; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; It checks whether the OMP backend task outlining can handle
; this pointer or not.
; The function _ZN6kdnode11buildkdtreeEPxS0_S0_S0_S0_xxPPii in the following test case is
; extracted from OMP2012/kdtree.

; CHECK: %__struct.shared.t = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr }
; CHECK: [[THIS_SHR_GEP:%.+]] = getelementptr inbounds %__struct.shared.t, ptr %{{.*}}, i32 0, i32 6
; CHECK: store ptr %this, ptr [[THIS_SHR_GEP]], align 8

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.kdnode = type <{ i64, ptr, ptr, [3 x i32], [4 x i8] }>

declare void @llvm.lifetime.start.p0(i64, ptr nocapture)

declare void @llvm.lifetime.end.p0(i64, ptr nocapture)

define dso_local void @_ZN6kdnode11buildkdtreeEPxS0_S0_S0_S0_xxPPii(ptr %this, ptr %xn, ptr %yn, ptr %zn, ptr %wn, ptr %tn, i64 %start, i64 %end, ptr %x, i32 %p) align 2 {
entry:
  %xn.addr = alloca ptr, align 8
  %yn.addr = alloca ptr, align 8
  %zn.addr = alloca ptr, align 8
  %wn.addr = alloca ptr, align 8
  %tn.addr = alloca ptr, align 8
  %start.addr = alloca i64, align 8
  %end.addr = alloca i64, align 8
  %x.addr = alloca ptr, align 8
  %p.addr = alloca i32, align 4
  %middle = alloca i64, align 8
  %lower = alloca i64, align 8
  store ptr %xn, ptr %xn.addr, align 8
  store ptr %yn, ptr %yn.addr, align 8
  store ptr %zn, ptr %zn.addr, align 8
  store ptr %wn, ptr %wn.addr, align 8
  store ptr %tn, ptr %tn.addr, align 8
  store i64 %start, ptr %start.addr, align 8
  store i64 %end, ptr %end.addr, align 8
  store ptr %x, ptr %x.addr, align 8
  store i32 %p, ptr %p.addr, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr %middle)
  call void @llvm.lifetime.start.p0(i64 8, ptr %lower)
  %0 = load i32, ptr %p.addr, align 4
  %rem = srem i32 %0, 3
  store i32 %rem, ptr %p.addr, align 4
  %1 = load i64, ptr %end.addr, align 8
  %2 = load i64, ptr %start.addr, align 8
  %cmp = icmp eq i64 %1, %2
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %3 = load ptr, ptr %xn.addr, align 8
  %arrayidx = getelementptr inbounds i64, ptr %3, i64 %2
  %4 = load i64, ptr %arrayidx, align 8
  store i64 %4, ptr %this, align 8
  br label %if.end130

if.else:                                          ; preds = %entry
  %add = add nsw i64 %2, 1
  %cmp2 = icmp eq i64 %1, %add
  br i1 %cmp2, label %if.then3, label %if.else38

if.then3:                                         ; preds = %if.else
  %5 = load ptr, ptr %x.addr, align 8
  %6 = load ptr, ptr %xn.addr, align 8
  %arrayidx4 = getelementptr inbounds i64, ptr %6, i64 %2
  %7 = load i64, ptr %arrayidx4, align 8
  %arrayidx5 = getelementptr inbounds ptr, ptr %5, i64 %7
  %8 = load ptr, ptr %arrayidx5, align 8
  %idxprom = sext i32 %rem to i64
  %arrayidx6 = getelementptr inbounds i32, ptr %8, i64 %idxprom
  %9 = load i32, ptr %arrayidx6, align 4
  %arrayidx7 = getelementptr inbounds i64, ptr %6, i64 %1
  %10 = load i64, ptr %arrayidx7, align 8
  %arrayidx8 = getelementptr inbounds ptr, ptr %5, i64 %10
  %11 = load ptr, ptr %arrayidx8, align 8
  %arrayidx10 = getelementptr inbounds i32, ptr %11, i64 %idxprom
  %12 = load i32, ptr %arrayidx10, align 4
  %cmp11 = icmp slt i32 %9, %12
  br i1 %cmp11, label %if.then12, label %if.else16

if.then12:                                        ; preds = %if.then3
  store i64 %10, ptr %this, align 8
  %call = call ptr @_Znwm(i64 40)
  %13 = load ptr, ptr %xn.addr, align 8
  %14 = load i64, ptr %start.addr, align 8
  %arrayidx15 = getelementptr inbounds i64, ptr %13, i64 %14
  %15 = load i64, ptr %arrayidx15, align 8
  call void @_ZN6kdnodeC1Ex(ptr %call, i64 %15)
  %lo = getelementptr inbounds %class.kdnode, ptr %this, i32 0, i32 1
  store ptr %call, ptr %lo, align 8
  br label %if.end130

if.else16:                                        ; preds = %if.then3
  %cmp25 = icmp sgt i32 %9, %12
  store i64 %7, ptr %this, align 8
  %call29 = call ptr @_Znwm(i64 40)
  %16 = load ptr, ptr %xn.addr, align 8
  %17 = load i64, ptr %end.addr, align 8
  %arrayidx30 = getelementptr inbounds i64, ptr %16, i64 %17
  %18 = load i64, ptr %arrayidx30, align 8
  call void @_ZN6kdnodeC1Ex(ptr %call29, i64 %18)
  br i1 %cmp25, label %if.then26, label %if.else32

if.then26:                                        ; preds = %if.else16
  %lo31 = getelementptr inbounds %class.kdnode, ptr %this, i32 0, i32 1
  store ptr %call29, ptr %lo31, align 8
  br label %if.end130

if.else32:                                        ; preds = %if.else16
  %hi = getelementptr inbounds %class.kdnode, ptr %this, i32 0, i32 2
  store ptr %call29, ptr %hi, align 8
  br label %if.end130

if.else38:                                        ; preds = %if.else
  %add39 = add nsw i64 %2, %1
  %div = sdiv i64 %add39, 2
  store i64 %div, ptr %middle, align 8
  %19 = load ptr, ptr %x.addr, align 8
  %20 = load ptr, ptr %xn.addr, align 8
  %arrayidx40 = getelementptr inbounds i64, ptr %20, i64 %div
  %21 = load i64, ptr %arrayidx40, align 8
  %arrayidx41 = getelementptr inbounds ptr, ptr %19, i64 %21
  %22 = load ptr, ptr %arrayidx41, align 8
  %23 = load i32, ptr %p.addr, align 4
  %idxprom42 = sext i32 %23 to i64
  %arrayidx43 = getelementptr inbounds i32, ptr %22, i64 %idxprom42
  %24 = load i32, ptr %arrayidx43, align 4
  %conv = sext i32 %24 to i64
  %sub = sub nsw i64 %div, 1
  br label %for.cond

for.cond:                                         ; preds = %if.else52, %if.else38
  %i.0 = phi i64 [ %sub, %if.else38 ], [ %dec, %if.else52 ]
  %25 = load i64, ptr %start.addr, align 8
  %cmp44 = icmp sge i64 %i.0, %25
  br i1 %cmp44, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %26 = load ptr, ptr %x.addr, align 8
  %27 = load ptr, ptr %xn.addr, align 8
  %arrayidx45 = getelementptr inbounds i64, ptr %27, i64 %i.0
  %28 = load i64, ptr %arrayidx45, align 8
  %arrayidx46 = getelementptr inbounds ptr, ptr %26, i64 %28
  %29 = load ptr, ptr %arrayidx46, align 8
  %30 = load i32, ptr %p.addr, align 4
  %idxprom47 = sext i32 %30 to i64
  %arrayidx48 = getelementptr inbounds i32, ptr %29, i64 %idxprom47
  %31 = load i32, ptr %arrayidx48, align 4
  %conv49 = sext i32 %31 to i64
  %cmp50 = icmp slt i64 %conv49, %conv
  br i1 %cmp50, label %for.end, label %if.else52

if.else52:                                        ; preds = %for.body
  store i64 %i.0, ptr %middle, align 8
  %dec = add nsw i64 %i.0, -1
  br label %for.cond

for.end:                                          ; preds = %for.body, %for.cond
  %32 = load ptr, ptr %xn.addr, align 8
  %33 = load i64, ptr %middle, align 8
  %arrayidx54 = getelementptr inbounds i64, ptr %32, i64 %33
  %34 = load i64, ptr %arrayidx54, align 8
  store i64 %34, ptr %this, align 8
  %35 = load i64, ptr %start.addr, align 8
  %sub56 = sub nsw i64 %35, 1
  store i64 %sub56, ptr %lower, align 8
  %36 = load i64, ptr %middle, align 8
  %37 = load i64, ptr %start.addr, align 8
  br label %for.cond57

for.cond57:                                       ; preds = %for.inc79, %for.end
  %i.1 = phi i64 [ %37, %for.end ], [ %inc80, %for.inc79 ]
  %upper.0 = phi i64 [ %36, %for.end ], [ %upper.1, %for.inc79 ]
  %38 = load i64, ptr %end.addr, align 8
  %cmp58 = icmp sle i64 %i.1, %38
  br i1 %cmp58, label %for.body59, label %for.end81

for.body59:                                       ; preds = %for.cond57
  %39 = load ptr, ptr %yn.addr, align 8
  %arrayidx60 = getelementptr inbounds i64, ptr %39, i64 %i.1
  %40 = load i64, ptr %arrayidx60, align 8
  %41 = load ptr, ptr %xn.addr, align 8
  %42 = load i64, ptr %middle, align 8
  %arrayidx61 = getelementptr inbounds i64, ptr %41, i64 %42
  %43 = load i64, ptr %arrayidx61, align 8
  %cmp62 = icmp ne i64 %40, %43
  br i1 %cmp62, label %if.then63, label %for.inc79

if.then63:                                        ; preds = %for.body59
  %44 = load ptr, ptr %x.addr, align 8
  %arrayidx65 = getelementptr inbounds ptr, ptr %44, i64 %40
  %45 = load ptr, ptr %arrayidx65, align 8
  %46 = load i32, ptr %p.addr, align 4
  %idxprom66 = sext i32 %46 to i64
  %arrayidx67 = getelementptr inbounds i32, ptr %45, i64 %idxprom66
  %47 = load i32, ptr %arrayidx67, align 4
  %conv68 = sext i32 %47 to i64
  %cmp69 = icmp slt i64 %conv68, %conv
  %48 = load ptr, ptr %tn.addr, align 8
  br i1 %cmp69, label %if.then70, label %if.else73

if.then70:                                        ; preds = %if.then63
  %49 = load i64, ptr %lower, align 8
  %inc = add nsw i64 %49, 1
  store i64 %inc, ptr %lower, align 8
  %arrayidx72 = getelementptr inbounds i64, ptr %48, i64 %inc
  store i64 %40, ptr %arrayidx72, align 8
  br label %for.inc79

if.else73:                                        ; preds = %if.then63
  %inc75 = add nsw i64 %upper.0, 1
  %arrayidx76 = getelementptr inbounds i64, ptr %48, i64 %inc75
  store i64 %40, ptr %arrayidx76, align 8
  br label %for.inc79

for.inc79:                                        ; preds = %if.else73, %if.then70, %for.body59
  %upper.1 = phi i64 [ %upper.0, %if.then70 ], [ %inc75, %if.else73 ], [ %upper.0, %for.body59 ]
  %inc80 = add nsw i64 %i.1, 1
  br label %for.cond57

for.end81:                                        ; preds = %for.cond57
  %50 = load i64, ptr %start.addr, align 8
  %sub82 = sub nsw i64 %50, 1
  store i64 %sub82, ptr %lower, align 8
  %51 = load i64, ptr %middle, align 8
  %52 = load i64, ptr %start.addr, align 8
  br label %for.cond83

for.cond83:                                       ; preds = %for.inc106, %for.end81
  %i.2 = phi i64 [ %52, %for.end81 ], [ %inc107, %for.inc106 ]
  %upper.2 = phi i64 [ %51, %for.end81 ], [ %upper.3, %for.inc106 ]
  %53 = load i64, ptr %end.addr, align 8
  %cmp84 = icmp sle i64 %i.2, %53
  br i1 %cmp84, label %for.body85, label %for.end108

for.body85:                                       ; preds = %for.cond83
  %54 = load ptr, ptr %zn.addr, align 8
  %arrayidx86 = getelementptr inbounds i64, ptr %54, i64 %i.2
  %55 = load i64, ptr %arrayidx86, align 8
  %56 = load ptr, ptr %xn.addr, align 8
  %57 = load i64, ptr %middle, align 8
  %arrayidx87 = getelementptr inbounds i64, ptr %56, i64 %57
  %58 = load i64, ptr %arrayidx87, align 8
  %cmp88 = icmp ne i64 %55, %58
  br i1 %cmp88, label %if.then89, label %for.inc106

if.then89:                                        ; preds = %for.body85
  %59 = load ptr, ptr %x.addr, align 8
  %arrayidx91 = getelementptr inbounds ptr, ptr %59, i64 %55
  %60 = load ptr, ptr %arrayidx91, align 8
  %61 = load i32, ptr %p.addr, align 4
  %idxprom92 = sext i32 %61 to i64
  %arrayidx93 = getelementptr inbounds i32, ptr %60, i64 %idxprom92
  %62 = load i32, ptr %arrayidx93, align 4
  %conv94 = sext i32 %62 to i64
  %cmp95 = icmp slt i64 %conv94, %conv
  %63 = load ptr, ptr %yn.addr, align 8
  br i1 %cmp95, label %if.then96, label %if.else100

if.then96:                                        ; preds = %if.then89
  %64 = load i64, ptr %lower, align 8
  %inc98 = add nsw i64 %64, 1
  store i64 %inc98, ptr %lower, align 8
  %arrayidx99 = getelementptr inbounds i64, ptr %63, i64 %inc98
  store i64 %55, ptr %arrayidx99, align 8
  br label %for.inc106

if.else100:                                       ; preds = %if.then89
  %inc102 = add nsw i64 %upper.2, 1
  %arrayidx103 = getelementptr inbounds i64, ptr %63, i64 %inc102
  store i64 %55, ptr %arrayidx103, align 8
  br label %for.inc106

for.inc106:                                       ; preds = %if.else100, %if.then96, %for.body85
  %upper.3 = phi i64 [ %upper.2, %if.then96 ], [ %inc102, %if.else100 ], [ %upper.2, %for.body85 ]
  %inc107 = add nsw i64 %i.2, 1
  br label %for.cond83

for.end108:                                       ; preds = %for.cond83
  %65 = load i64, ptr %lower, align 8
  %66 = load i64, ptr %start.addr, align 8
  %cmp109 = icmp sge i64 %65, %66
  br i1 %cmp109, label %if.then110, label %if.end117

if.then110:                                       ; preds = %for.end108
  %call111 = call ptr @_Znwm(i64 40)
  call void @_ZN6kdnodeC1Ev(ptr %call111)
  %lo112 = getelementptr inbounds %class.kdnode, ptr %this, i32 0, i32 1
  store ptr %call111, ptr %lo112, align 8
  %67 = load i64, ptr %lower, align 8
  %68 = load i64, ptr %start.addr, align 8
  %sub113 = sub nsw i64 %67, %68
  %cmp114 = icmp sge i64 %sub113, 16
  br label %DIR.OMP.TASK.1

DIR.OMP.TASK.1:                                   ; preds = %if.then110
  %69 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IF"(i1 %cmp114),
    "QUAL.OMP.SHARED:TYPED"(ptr %tn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %yn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %xn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %wn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %zn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %x.addr, ptr null, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %start.addr, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %lower, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %p.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %this, %class.kdnode zeroinitializer, i32 1) ]

  %70 = call ptr @llvm.launder.invariant.group.p0(ptr %this)
  %lo115 = getelementptr inbounds %class.kdnode, ptr %70, i32 0, i32 1
  %71 = load ptr, ptr %lo115, align 8
  %72 = load ptr, ptr %tn.addr, align 8
  %73 = load ptr, ptr %yn.addr, align 8
  %74 = load ptr, ptr %xn.addr, align 8
  %75 = load ptr, ptr %wn.addr, align 8
  %76 = load ptr, ptr %zn.addr, align 8
  %77 = load i64, ptr %start.addr, align 8
  %78 = load i64, ptr %lower, align 8
  %79 = load ptr, ptr %x.addr, align 8
  %80 = load i32, ptr %p.addr, align 4
  %add116 = add nsw i32 %80, 1
  call void @_ZN6kdnode11buildkdtreeEPxS0_S0_S0_S0_xxPPii(ptr %71, ptr %72, ptr %73, ptr %74, ptr %75, ptr %76, i64 %77, i64 %78, ptr %79, i32 %add116)
  br label %DIR.OMP.END.TASK.3

DIR.OMP.END.TASK.3:                               ; preds = %DIR.OMP.TASK.1
  call void @llvm.directive.region.exit(token %69) [ "DIR.OMP.END.TASK"() ]
  br label %if.end117

if.end117:                                        ; preds = %DIR.OMP.END.TASK.3, %for.end108
  %81 = load i64, ptr %middle, align 8
  %cmp118 = icmp sgt i64 %upper.2, %81
  br i1 %cmp118, label %if.then119, label %DIR.OMP.TASKWAIT.7

if.then119:                                       ; preds = %if.end117
  %call120 = call ptr @_Znwm(i64 40)
  call void @_ZN6kdnodeC1Ev(ptr %call120)
  %hi121 = getelementptr inbounds %class.kdnode, ptr %this, i32 0, i32 2
  store ptr %call120, ptr %hi121, align 8
  %82 = load i64, ptr %end.addr, align 8
  %83 = load i64, ptr %middle, align 8
  %add122 = add nsw i64 %83, 1
  %sub123 = sub nsw i64 %82, %add122
  %cmp124 = icmp sge i64 %sub123, 16
  br label %DIR.OMP.TASK.4

DIR.OMP.TASK.4:                                   ; preds = %if.then119
  %84 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IF"(i1 %cmp124),
    "QUAL.OMP.SHARED:TYPED"(ptr %tn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %yn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %xn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %wn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %zn.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %x.addr, ptr null, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %middle, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %end.addr, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %p.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %this, %class.kdnode zeroinitializer, i32 1) ]

  %85 = call ptr @llvm.launder.invariant.group.p0(ptr %this)
  %hi125 = getelementptr inbounds %class.kdnode, ptr %85, i32 0, i32 2
  %86 = load ptr, ptr %hi125, align 8
  %87 = load ptr, ptr %tn.addr, align 8
  %88 = load ptr, ptr %yn.addr, align 8
  %89 = load ptr, ptr %xn.addr, align 8
  %90 = load ptr, ptr %wn.addr, align 8
  %91 = load ptr, ptr %zn.addr, align 8
  %92 = load i64, ptr %middle, align 8
  %add126 = add nsw i64 %92, 1
  %93 = load i64, ptr %end.addr, align 8
  %94 = load ptr, ptr %x.addr, align 8
  %95 = load i32, ptr %p.addr, align 4
  %add127 = add nsw i32 %95, 1
  call void @_ZN6kdnode11buildkdtreeEPxS0_S0_S0_S0_xxPPii(ptr %86, ptr %87, ptr %88, ptr %89, ptr %90, ptr %91, i64 %add126, i64 %93, ptr %94, i32 %add127)
  br label %DIR.OMP.END.TASK.6

DIR.OMP.END.TASK.6:                               ; preds = %DIR.OMP.TASK.4
  call void @llvm.directive.region.exit(token %84) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.TASKWAIT.7

DIR.OMP.TASKWAIT.7:                               ; preds = %DIR.OMP.END.TASK.6, %if.end117
  %96 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"() ]
  br label %DIR.OMP.TASKWAIT.8

DIR.OMP.TASKWAIT.8:                               ; preds = %DIR.OMP.TASKWAIT.7
  call void @llvm.directive.region.exit(token %96) [ "DIR.OMP.END.TASKWAIT"() ]
  br label %if.end130

if.end130:                                        ; preds = %DIR.OMP.TASKWAIT.8, %if.else32, %if.then26, %if.then12, %if.then
  call void @llvm.lifetime.end.p0(i64 8, ptr %lower)
  call void @llvm.lifetime.end.p0(i64 8, ptr %middle)
  ret void
}

declare dso_local noalias ptr @_Znwm(i64)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare ptr @llvm.launder.invariant.group.p0(ptr)
declare void @_ZN6kdnodeC1Ev(ptr)
declare void @_ZN6kdnodeC1Ex(ptr, i64)
