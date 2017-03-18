; RUN: opt %s -VPlanDriver -vplan-predicator-report -vplan-driver -vplan-enable-subregions -vplan-predicator -S -o /dev/null | FileCheck %s
; Verify VPlan predicator: two nested if-else statements.

; region1
; -------
; BB11
;  |
;  v
; loop16
;  |
;  v
; BB12


; loop16
; ------
; BB10
;  |
;  v
; BB2<-----+
;  |       |
;  v       |
; region17 |
;  |       |
;  v       |
; region18 |
;  |       |
;  v       |
; BB15-----+
;  |
;  v
; BB9

; CHECK: loop{{[0-9]+}}:
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED1:BP[0-9]+]] = AllOnes{{[0-9]+}}
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED2:BP[0-9]+]] = [[BLOCKPRED1]]
; CHECK:   region{{[0-9]+}}:
; CHECK:     [[BLOCKPRED2:BP[0-9]+]] = [[BLOCKPRED1]]
; CHECK:   region{{[0-9]+}}:
; CHECK:     [[BLOCKPRED2:BP[0-9]+]] = [[BLOCKPRED1]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED3:BP[0-9]+]] = [[BLOCKPRED2]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED4:BP[0-9]+]] = [[BLOCKPRED3]]


; region18
; --------
; BB14
;  | \
;  v  v
; BB6 BB7
;  |  /
;  v v
; BB8

; CHECK: region{{[0-9]+}}:
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED5:BP[0-9]+]] = [[BLOCKPRED2]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFFALSE11:IfF[0-9]+]] = [[BLOCKPRED5]] && ! VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED8:BP[0-9]+]] = [[IFFALSE11]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFTRUE10:IfT[0-9]+]] = [[BLOCKPRED5]] && VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED6:BP[0-9]+]] = [[IFTRUE10]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED7:BP[0-9]+]] = [[BLOCKPRED8]] || [[BLOCKPRED6]]


; region17
; --------
; BB13
;  |  \
;  v   v
; BB3 BB4
;  |  /
;  v v
; BB5

; CHECK: region{{[0-9]+}}:
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED12:BP[0-9]+]] = [[BLOCKPRED2]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFFALSE18:IfF[0-9]+]] = [[BLOCKPRED12]] && ! VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED15:BP[0-9]+]] = [[IFFALSE18]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[IFTRUE17:IfT[0-9]+]] = [[BLOCKPRED12]] && VBR{{[0-9]+}}
; CHECK:     [[BLOCKPRED13:BP[0-9]+]] = [[IFTRUE17]]
; CHECK:   BB{{[0-9]+}}:
; CHECK:     [[BLOCKPRED14:BP[0-9]+]] = [[BLOCKPRED15]] || [[BLOCKPRED13]]


; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)


; void foo(int * restrict a, int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (a[i] > 0) 
;       b[i] = b[i] * 5;
;     else
;       a[i] = a[i] + 5;
; 
;     c[i] = c[i] * N;
; 
;     if (c[i] > 0) 
;       a[i] = c[i] - a[i];
;     else
;       b[i] = a[i] * c[i];
;   }
; }

; ModuleID = 'pred_2x_if_else_noopt.ll'
source_filename = "pred_2x_if_else.c"
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
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %mul = mul nsw i32 %1, 5
  store i32 %mul, i32* %arrayidx3, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %add = add nsw i32 %0, 5
  store i32 %add, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %2 = phi i32 [ %add, %if.else ], [ %0, %if.then ]
  %arrayidx11 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx11, align 4
  %mul12 = mul nsw i32 %3, %N
  store i32 %mul12, i32* %arrayidx11, align 4
  %cmp17 = icmp sgt i32 %mul12, 0
  br i1 %cmp17, label %if.then18, label %if.else25

if.then18:                                        ; preds = %if.end
  %sub = sub nsw i32 %mul12, %2
  store i32 %sub, i32* %arrayidx, align 4
  br label %for.inc

if.else25:                                        ; preds = %if.end
  %mul30 = mul nsw i32 %2, %mul12
  %arrayidx32 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  store i32 %mul30, i32* %arrayidx32, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then18, %if.else25
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
