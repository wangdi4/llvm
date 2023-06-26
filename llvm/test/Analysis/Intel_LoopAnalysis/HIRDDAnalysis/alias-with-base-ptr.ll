; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region  -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the DD analysis identifies that there is a dependency
; between (%xg)[0][3] and (%arrayidx7)[%4]. The reason is because
; (%arrayidx7)[%4] can have negative index. If we pass GEP location of
; (%arrayidx7)[%4] as '%arrayidx7' instead of '%xg', BasicAA concludes
; independence because it thinks the ref starts with offset %xg[24].

; HIR generated

; BEGIN REGION { }
;       + DO i1 = 0, -12, 1   <DO_LOOP>
;       |   %2 = -1 * i1 + -13  <<  2;
;       |   %4 = -1 * i1 + -13  *  -4;
;       |   @llvm.memcpy.p0.p0.i64(&((%arrayidx7)[%4]),  &((%u8)[%4 + 100]),  %2 + 4,  0);
;       |   %5 = (%xg)[0][3];
;       |   (%xg)[0][3] = %5 + 49;
;       + END LOOP
; END REGION

; Check that the dependencies were found

; CHECK-DAG: (%arrayidx7)[%4] --> (%xg)[0][3] FLOW (*) (?)  
; CHECK-DAG: (%arrayidx7)[%4] --> (%xg)[0][3] OUTPUT (*) (?)  
; CHECK-DAG: (%xg)[0][3] --> (%arrayidx7)[%4] ANTI (*) (?)  
; CHECK-DAG: (%xg)[0][3] --> (%arrayidx7)[%4] OUTPUT (*) (?) 

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo() {
entry:
  %xg = alloca [100 x i32]
  %u8 = alloca [100 x i32]
  br label %for.preheader

for.preheader:
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr %xg, i64 0, i64 24
  %scevgep246 = getelementptr inbounds i8, ptr %u8, i64 100
  br label %for.body

for.body:
  %indvars.iv = phi i64 [35, %for.preheader], [ %indvars.iv.next, %for.body ]
  %0 = sub nsw i64 22, %indvars.iv
  %1 = and i64 %0, 4294967295
  %2 = shl nuw nsw i64 %1, 2
  %3 = add nuw nsw i64 %2, 4
  %4 = mul nsw i64 %1, -4

  %scevgep247 = getelementptr i8, ptr %scevgep246, i64 %4
  %scevgep245 = getelementptr i8, ptr %arrayidx7, i64 %4
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 4 dereferenceable(1) %scevgep245, ptr noundef nonnull align 4 dereferenceable(1) %scevgep247, i64 %3, i1 false)
  %arrayidx72 = getelementptr inbounds [100 x i32], ptr %xg, i64 0, i64 3
  %5 = load i32, ptr %arrayidx72
  %add73 = add i32 %5, 49
  store i32 %add73, ptr %arrayidx72

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 24
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  ret void
}

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)
