; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-tpv -S %s | FileCheck %s
; RUN: opt -passes=vpo-paropt-tpv -S %s | FileCheck %s
;
; It is to test whether a composite instruction such as:
;  %9 = load i32, ptr getelementptr inbounds (i32, ptr getelementptr inbounds (%class.t_class, ptr @a, i64 0, i32 2), i32 1)
; can be broken into a sequence of instructions.

; CHECK: %a.tpv.cached = load ptr, ptr %a.tpv.cached.addr, align 8
; CHECK-LABEL: if.then:
; CHECK: [[I1:%.+]] = getelementptr inbounds %class.t_class, ptr %a.tpv.cached, i64 0, i32 2
; CHECK: {{%.+}} = getelementptr inbounds i32, ptr [[I1]], i32 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.t_class = type { i8, i32, float, ptr }

@a = external dso_local thread_private global %class.t_class, align 8
@.str.4 = external hidden unnamed_addr constant [20 x i8], align 1
@.str.5 = external hidden unnamed_addr constant [82 x i8], align 1
@"@tid.addr" = external local_unnamed_addr global i32
@.gomp_critical_user_.var = external global [8 x i32]
@.kmpc_loc.0.0.11 = external hidden unnamed_addr constant { i32, i32, i32, i32, ptr }
@.kmpc_loc.0.0.15 = external hidden unnamed_addr constant { i32, i32, i32, i32, ptr }

declare dso_local i32 @puts(ptr nocapture readonly) local_unnamed_addr
declare dso_local void @exit(i32) local_unnamed_addr
declare dso_local i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr
declare dso_local i32 @omp_get_thread_num() local_unnamed_addr

define dso_local void @_ZN7t_class3subEv(ptr %this) local_unnamed_addr align 2 {
entry:
  %my.tid26 = load i32, ptr @"@tid.addr", align 4
  call void @__kmpc_barrier(ptr nonnull @.kmpc_loc.0.0.15, i32 %my.tid26)
  %0 = load i8, ptr @a, align 8
  %conv = sext i8 %0 to i32
  %call = call i32 @omp_get_thread_num()
  %add = add nsw i32 %call, 1
  %rem = srem i32 %add, 127
  %cmp = icmp eq i32 %rem, %conv
  br i1 %cmp, label %lor.lhs.false, label %if.then

lor.lhs.false:                                    ; preds = %entry
  %1 = load i32, ptr getelementptr inbounds (%class.t_class, ptr @a, i64 0, i32 1), align 4
  %call4 = call i32 @omp_get_thread_num()
  %add5 = add nsw i32 %call4, 1
  %cmp6 = icmp eq i32 %1, %add5
  br i1 %cmp6, label %lor.lhs.false7, label %if.then

lor.lhs.false7:                                   ; preds = %lor.lhs.false
  %2 = load float, ptr getelementptr inbounds (%class.t_class, ptr @a, i64 0, i32 2), align 8
  %call8 = call i32 @omp_get_thread_num()
  %add9 = add nsw i32 %call8, 1
  %conv10 = sitofp i32 %add9 to float
  %sub = fsub float %2, %conv10
  %3 = call float @llvm.fabs.f32(float %sub)
  %4 = fpext float %3 to double
  %cmp12 = fcmp ogt double %4, 0x3E7AD7F29ABCAF48
  br i1 %cmp12, label %if.then, label %lor.lhs.false13

lor.lhs.false13:                                  ; preds = %lor.lhs.false7
  %5 = load ptr, ptr getelementptr inbounds (%class.t_class, ptr @a, i64 0, i32 3), align 8
  %call14 = call i32 @omp_get_thread_num()
  %add15 = add nsw i32 %call14, 1
  %conv16 = sext i32 %add15 to i64
  %6 = inttoptr i64 %conv16 to ptr
  %cmp17 = icmp eq ptr %5, %6
  br i1 %cmp17, label %if.end, label %if.then

if.then:                                          ; preds = %lor.lhs.false13, %lor.lhs.false7, %lor.lhs.false, %entry
  %call18 = call i32 @omp_get_thread_num()
  %7 = load i8, ptr @a, align 8
  %conv19 = sext i8 %7 to i32
  %8 = load i32, ptr getelementptr inbounds (%class.t_class, ptr @a, i64 0, i32 1), align 4
  %9 = load i32, ptr getelementptr inbounds (i32, ptr getelementptr inbounds (%class.t_class, ptr @a, i64 0, i32 2), i32 1), align 8
  %c1 = bitcast i32 %9 to float
  %conv20 = fpext float %c1 to double
  %10 = load ptr, ptr getelementptr inbounds (%class.t_class, ptr @a, i64 0, i32 3), align 8
  %call21 = call i32 @omp_get_thread_num()
  %add22 = add nsw i32 %call21, 1
  %call23 = call i32 (ptr, ...) @printf(ptr @.str.5, i32 %call18, i32 %conv19, i32 %8, double %conv20, ptr %10, i32 %add22)
  %call24 = call i32 @puts(ptr @.str.4)
  %my.tid = load i32, ptr @"@tid.addr", align 4
  call void @__kmpc_critical(ptr nonnull @.kmpc_loc.0.0.11, i32 %my.tid, ptr nonnull @.gomp_critical_user_.var)
  call void @exit(i32 1)
  unreachable

if.end:                                           ; preds = %lor.lhs.false13
  ret void
}

declare void @__kmpc_critical(ptr, i32, ptr) local_unnamed_addr
declare void @__kmpc_barrier(ptr, i32) local_unnamed_addr
declare float @llvm.fabs.f32(float)
