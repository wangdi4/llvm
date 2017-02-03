; RUN: llc -O1 -csa-opt-df-pass=1 -csa-seq-opt=2 -csa-seq-break-memdep=2 -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK

; ModuleID = 'BlackScholes_csa_loop.cpp'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @BlackScholesOpt(i32 %OptPerThread, double* nocapture readonly %OptionYears, double* nocapture readonly %OptionStrike, double* nocapture readonly %StockPrice, double* nocapture %CallResult, double* nocapture %PutResult) #0 {

; CSA_CHECK-DAG: repeat1
; CSA_CHECK-DAG: onend
; CSA_CHECK-DAG: seqotne32
; CSA_CHECK-DAG: stride64

entry:
  %cmp.77 = icmp sgt i32 %OptPerThread, 0
  br i1 %cmp.77, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, double* %OptionYears, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8, !tbaa !1
  %arrayidx2 = getelementptr inbounds double, double* %OptionStrike, i64 %indvars.iv
  %1 = load double, double* %arrayidx2, align 8, !tbaa !1
  %mul = fmul double %0, 0xBF9D8BE0817F5FFE
  %call = tail call double @exp2(double %mul) #2
  %mul3 = fmul double %1, %call
  %arrayidx5 = getelementptr inbounds double, double* %StockPrice, i64 %indvars.iv
  %2 = load double, double* %arrayidx5, align 8, !tbaa !1
  %call6 = tail call double @sqrt(double %0) #2
  %div = fdiv double %2, %1
  %call7 = tail call double @log2(double %div) #2
  %mul8 = fmul double %call6, 0x3FDBB322796769FD
  %div9 = fdiv double %call7, %mul8
  %mul10 = fmul double %call6, 0x3FCBBBBBBBBBBBBC
  %add = fadd double %mul10, %div9
  %cmp.i = fcmp olt double %add, 0.000000e+00
  %sub.i = fsub double -0.000000e+00, %add
  %InputX.addr.0.i = select i1 %cmp.i, double %sub.i, double %add
  %mul.i = fmul double %InputX.addr.0.i, %InputX.addr.0.i
  %mul4.i = fmul double %mul.i, 0xBFE71547652B82FE
  %call.i = tail call double @exp2(double %mul4.i) #2
  %mul5.i = fmul double %call.i, 0x3FD9884533D43651
  %mul7.i = fmul double %InputX.addr.0.i, 2.316419e-01
  %add.i = fadd double %mul7.i, 1.000000e+00
  %div.i = fdiv double 1.000000e+00, %add.i
  %mul8.i = fmul double %div.i, %div.i
  %mul9.i = fmul double %div.i, %mul8.i
  %mul10.i = fmul double %div.i, %mul9.i
  %mul11.i = fmul double %div.i, %mul10.i
  %mul12.i = fmul double %div.i, 0x3FD470BF3A92F8EC
  %mul13.i = fmul double %mul8.i, 0x3FD6D1F0E5A8325B
  %mul14.i = fmul double %mul9.i, 0x3FFC80EF025F5E68
  %add1547.i = fsub double %mul14.i, %mul13.i
  %mul16.i = fmul double %mul10.i, 0x3FFD23DD4EF278D0
  %add1748.i = fsub double %add1547.i, %mul16.i
  %mul18.i = fmul double %mul11.i, 0x3FF548CDD6F42943
  %add19.i = fadd double %add1748.i, %mul18.i
  %add20.i = fadd double %mul12.i, %add19.i
  %mul21.i = fmul double %mul5.i, %add20.i
  %sub22.i = fsub double 1.000000e+00, %mul21.i
  %sub24.i = fsub double 1.000000e+00, %sub22.i
  %sub24.sub22.i = select i1 %cmp.i, double %sub24.i, double %sub22.i
  %mul12 = fmul double %call6, 3.000000e-01
  %sub = fsub double %add, %mul12
  %cmp.i.50 = fcmp olt double %sub, 0.000000e+00
  %sub.i.51 = fsub double -0.000000e+00, %sub
  %InputX.addr.0.i.52 = select i1 %cmp.i.50, double %sub.i.51, double %sub
  %mul.i.53 = fmul double %InputX.addr.0.i.52, %InputX.addr.0.i.52
  %mul4.i.54 = fmul double %mul.i.53, 0xBFE71547652B82FE
  %call.i.55 = tail call double @exp2(double %mul4.i.54) #2
  %mul5.i.56 = fmul double %call.i.55, 0x3FD9884533D43651
  %mul7.i.57 = fmul double %InputX.addr.0.i.52, 2.316419e-01
  %add.i.58 = fadd double %mul7.i.57, 1.000000e+00
  %div.i.59 = fdiv double 1.000000e+00, %add.i.58
  %mul8.i.60 = fmul double %div.i.59, %div.i.59
  %mul9.i.61 = fmul double %div.i.59, %mul8.i.60
  %mul10.i.62 = fmul double %div.i.59, %mul9.i.61
  %mul11.i.63 = fmul double %div.i.59, %mul10.i.62
  %mul12.i.64 = fmul double %div.i.59, 0x3FD470BF3A92F8EC
  %mul13.i.65 = fmul double %mul8.i.60, 0x3FD6D1F0E5A8325B
  %mul14.i.66 = fmul double %mul9.i.61, 0x3FFC80EF025F5E68
  %add1547.i.67 = fsub double %mul14.i.66, %mul13.i.65
  %mul16.i.68 = fmul double %mul10.i.62, 0x3FFD23DD4EF278D0
  %add1748.i.69 = fsub double %add1547.i.67, %mul16.i.68
  %mul18.i.70 = fmul double %mul11.i.63, 0x3FF548CDD6F42943
  %add19.i.71 = fadd double %add1748.i.69, %mul18.i.70
  %add20.i.72 = fadd double %mul12.i.64, %add19.i.71
  %mul21.i.73 = fmul double %mul5.i.56, %add20.i.72
  %sub22.i.74 = fsub double 1.000000e+00, %mul21.i.73
  %sub24.i.75 = fsub double 1.000000e+00, %sub22.i.74
  %sub24.sub22.i.76 = select i1 %cmp.i.50, double %sub24.i.75, double %sub22.i.74
  %mul14 = fmul double %2, %sub24.sub22.i
  %mul15 = fmul double %mul3, %sub24.sub22.i.76
  %sub16 = fsub double %mul14, %mul15
  %add17 = fadd double %mul3, %sub16
  %sub18 = fsub double %add17, %2
  %arrayidx20 = getelementptr inbounds double, double* %CallResult, i64 %indvars.iv
  store double %sub16, double* %arrayidx20, align 8, !tbaa !1
  %arrayidx22 = getelementptr inbounds double, double* %PutResult, i64 %indvars.iv
  store double %sub18, double* %arrayidx22, align 8, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %OptPerThread
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind readnone
declare double @exp2(double) #1

; Function Attrs: nounwind readnone
declare double @sqrt(double) #1

; Function Attrs: nounwind readnone
declare double @log2(double) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 "}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
