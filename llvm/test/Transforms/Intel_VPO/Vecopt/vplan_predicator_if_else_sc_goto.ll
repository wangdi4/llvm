; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -S -o /dev/null | FileCheck %s
; REQUIRES: asserts

; Verify the VPlan predicator: if with short-circuit condition (goto)

; region1
; -------
; BB9
;  |
;  v
; loop13
;  |
;  v
; BB10


; loop13
; ------
; BB8
;  |
;  v
; BB2<-----+
;  |       |
;  v       |
; region14 |
;  |       |
;  v       |
; BB12-----+
;  |
;  v
; BB7

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


; region14
; --------
; BB11
;  |\
;  v \
; BB3 \
;  | \ \
;  v  v v
; BB5 BB4
;  |  /
;  v v
; BB6

; CHECK: region{{[0-9]+}}:
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED5:BP[0-9]+]] = [[BLOCKPRED2]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFTRUE11:IfT[0-9]+]] = [[BLOCKPRED5]] && VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED6:BP[0-9]+]] = [[IFTRUE11]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFFALSE15:IfF[0-9]+]] = [[BLOCKPRED6]] && ! VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED9:BP[0-9]+]] = [[IFFALSE15]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFTRUE14:IfT[0-9]+]] = [[BLOCKPRED6]] && VBR{{[0-9]+}}
; CHECK:     [[IFFALSE12:IfF[0-9]+]] = [[BLOCKPRED5]] && ! VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED7:BP[0-9]+]] = [[IFFALSE12]] || [[IFTRUE14]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED8:BP[0-9]+]] = [[BLOCKPRED9]] || [[BLOCKPRED7]]


; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)

; void foo(int * restrict a, int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (b[i] <= 100 && a[i] != 0) 
;       b[i] = b[i] * 5;
;     else
;       a[i] = a[i] + 5;
; 
;     c[i] = c[i] * N;
;   }
; }


; ModuleID = 'pred_if_else_sc_goto_noopt.ll'
source_filename = "pred_if_else_sc_goto.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp slt i32 %0, 101
  %arrayidx3 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  br i1 %cmp1, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %for.body
  %cmp4 = icmp eq i32 %1, 0
  br i1 %cmp4, label %if.else, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %mul = mul nsw i32 %0, 5
  store i32 %mul, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body, %land.lhs.true
  %2 = phi i32 [ 0, %land.lhs.true ], [ %1, %for.body ]
  %add = add nsw i32 %2, 5
  store i32 %add, i32* %arrayidx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %arrayidx14 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx14, align 4
  %mul15 = mul nsw i32 %3, %N
  store i32 %mul15, i32* %arrayidx14, align 4
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
