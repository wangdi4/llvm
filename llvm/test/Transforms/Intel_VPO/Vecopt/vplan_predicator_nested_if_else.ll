; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -S -o /dev/null | FileCheck %s
; REQUIRES: asserts

; Verify the VPlan predicator: nested if-else statements.

; region1
; -------
; BB12
;  |
;  v
; loop16
;  |
;  v
; BB13



; loop16
; ------
; BB11
;  |
;  v
; BB2<-----+
;  |       |
;  v       |
; region17 |
;  |       |
;  v       |
; BB15-----+
;  |
;  v
; BB10

; CHECK: loop{{[0-9]+}}:
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED1:BP[0-9]+]] = AllOnes{{[0-9]+}}
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED2:BP[0-9]+]] = [[BLOCKPRED1]]
; CHECK:   region{{[0-9]+}}:
; CHECK:     [[BLOCKPRED2:BP[0-9]+]] = [[BLOCKPRED1]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED3:BP[0-9]+]] = [[BLOCKPRED2]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED4:BP[0-9]+]] = [[BLOCKPRED3]]

; region17
; --------
;       BB14
;      /    \
;     v      v
;   BB3      BB4
;   / \     /  \
;  v   v   v    v
; BB8 BB9 BB5  BB6
;   \   | |   /
;    \  | |  /
;     v v v v
;       BB7

; CHECK: region{{[0-9]+}}:
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED5:BP[0-9]+]] = [[BLOCKPRED2]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFFALSE18:IfF[0-9]+]] = [[BLOCKPRED5]] && ! VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED10:BP[0-9]+]] = [[IFFALSE18]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFFALSE21:IfF[0-9]+]] = [[BLOCKPRED10]] && ! VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED12:BP[0-9]+]] = [[IFFALSE21]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFTRUE20:IfT[0-9]+]] = [[BLOCKPRED10]] && VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED11:BP[0-9]+]] = [[IFTRUE20]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFTRUE14:IfT[0-9]+]] = [[BLOCKPRED5]] && VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED6:BP[0-9]+]] = [[IFTRUE14]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFFALSE17:IfF[0-9]+]] = [[BLOCKPRED6]] && ! VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED9:BP[0-9]+]] = [[IFFALSE17]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFTRUE16:IfT[0-9]+]] = [[BLOCKPRED6]] && VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED7:BP[0-9]+]] = [[IFTRUE16]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED8:BP[0-9]+]] = [[BLOCKPRED12]] || [[BLOCKPRED11]] || [[BLOCKPRED9]] || [[BLOCKPRED7]]

; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)


; void foo(int * restrict a, int * restrict b, int * restrict c, int N, int M, int K)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (a[i] == 0) 
;       if (b[i] > 5)
;         b[i] = b[i] * 5;
;       else
;         b[i] = c[i] + b[i];
;     else
;       if (a[i] > 100)
;         a[i] = a[i] - b[i];
;       else
;         a[i] = a[i] * c[i];
;   }
; }


; ModuleID = 'pred_nested_if_else_noopt.ll'
source_filename = "pred_nested_if_else.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture readonly %c, i32 %N, i32 %M, i32 %K) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp eq i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else16

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %cmp4 = icmp sgt i32 %1, 5
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %mul = mul nsw i32 %1, 5
  store i32 %mul, i32* %arrayidx3, align 4
  br label %for.inc

if.else:                                          ; preds = %if.then
  %arrayidx11 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx11, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, i32* %arrayidx3, align 4
  br label %for.inc

if.else16:                                        ; preds = %for.body
  %cmp19 = icmp sgt i32 %0, 100
  br i1 %cmp19, label %if.then20, label %if.else27

if.then20:                                        ; preds = %if.else16
  %arrayidx24 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx24, align 4
  %sub = sub nsw i32 %0, %3
  store i32 %sub, i32* %arrayidx, align 4
  br label %for.inc

if.else27:                                        ; preds = %if.else16
  %arrayidx31 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx31, align 4
  %mul32 = mul nsw i32 %4, %0
  store i32 %mul32, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then5, %if.else27, %if.then20
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
