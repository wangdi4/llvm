; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd < %s 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; RUN: opt -enable-intel-advanced-opts -disable-simplify-libcalls -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd < %s 2>&1 | FileCheck %s --check-prefix="NOLIBCALL"
; No disable-simplify-libcalls support in new pass manager.
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed=false -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd < %s 2>&1 | FileCheck %s --check-prefix="NOLIBCALL"
; RUN: opt -enable-intel-advanced-opts  -intel-libirc-allowed=false -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s --check-prefix="NOLIBCALL"


; Check runtime dd multiversioning with library call

;     BEGIN REGION { }
;          + DO i1 = 0, 99, 1   <DO_LOOP>
;          |   %t19 = (%s)[i1];
;          |   %t20 = (%r)[i1];
;          |   (%r)[i1] = %t19 + %t20;
;          |   %t21 = (%q)[i1];
;          |   (%q)[i1] = %t19 + %t20 + %t21;
;          |   %t22 = (%p)[i1];
;          |   (%p)[i1] = %t19 + %t20 + %t21 + %t22;
;          |   %t23 = (%o)[i1];
;          |   (%o)[i1] = %t19 + %t20 + %t21 + %t22 + %t23;
;          |   %t24 = (%n)[i1];
;          |   (%n)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24;
;          |   %t25 = (%m)[i1];
;          |   (%m)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25;
;          |   %t26 = (%l)[i1];
;          |   (%l)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26;
;          |   %t27 = (%k)[i1];
;          |   (%k)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27;
;          |   %t28 = (%j)[i1];
;          |   (%j)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28;
;          |   %t29 = (%i)[i1];
;          |   (%i)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28 + %t29;
;          |   %t30 = (%h)[i1];
;          |   (%h)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28 + %t29 + %t30;
;          |   %t31 = (%g)[i1];
;          |   (%g)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28 + %t29 + %t30 + %t31;
;          |   %t32 = (%f)[i1];
;          |   (%f)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28 + %t29 + %t30 + %t31 + %t32;
;          |   %t33 = (%e)[i1];
;          |   (%e)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28 + %t29 + %t30 + %t31 + %t32 + %t33;
;          |   %t34 = (%d)[i1];
;          |   (%d)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28 + %t29 + %t30 + %t31 + %t32 + %t33 + %t34;
;          |   %t35 = (%c)[i1];
;          |   (%c)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28 + %t29 + %t30 + %t31 + %t32 + %t33 + %t34 + %t35;
;          |   %t36 = (%b)[i1];
;          |   (%b)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28 + %t29 + %t30 + %t31 + %t32 + %t33 + %t34 + %t35 + %t36;
;          |   %t37 = (%a)[i1];
;          |   (%a)[i1] = %t19 + %t20 + %t21 + %t22 + %t23 + %t24 + %t25 + %t26 + %t27 + %t28 + %t29 + %t30 + %t31 + %t32 + %t33 + %t34 + %t35 + %t36 + %t37;
;          + END LOOP
;     END REGION

; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%s)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%s)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%r)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%r)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%q)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%q)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%p)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%p)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%o)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%o)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%n)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%n)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%m)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%m)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%l)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%l)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%k)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%k)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%j)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%j)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%i)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%i)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%h)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%h)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%g)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%g)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%f)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%f)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%e)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%e)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%d)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%d)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%c)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%c)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%b)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%b)[99]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].0 = &((%a)[0]);
; CHECK-DAG: (%dd)[0][{{[0-9]+}}].1 = &((%a)[99]);
; CHECK: %call = @__intel_rtdd_indep(&((i8*)(%dd)[0]),  19);
; CHECK: if (%call == 0)

; NOLIBCALL-NOT: __intel_rtdd_indep

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local global [16000 x i8] zeroinitializer, align 16

define dso_local i32 @foo(i32* %a, i32* %b, i32* %c, i32* %d, i32* %e, i32* %f, i32* %g, i32* %h, i32* %i, i32* %j, i32* %k, i32* %l, i32* %m, i32* %n, i32* %o, i32* %p, i32* %q, i32* %r, i32* %s) #0 {
entry:
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %indvars.iv.i = phi i64 [ 0, %entry ], [ %indvars.iv.next.i, %for.body.i ]
  %arrayidx.i = getelementptr inbounds i32, i32* %s, i64 %indvars.iv.i
  %t19 = load i32, i32* %arrayidx.i, align 4
  %arrayidx2.i = getelementptr inbounds i32, i32* %r, i64 %indvars.iv.i
  %t20 = load i32, i32* %arrayidx2.i, align 4
  %add.i = add nsw i32 %t20, %t19
  store i32 %add.i, i32* %arrayidx2.i, align 4
  %arrayidx4.i = getelementptr inbounds i32, i32* %q, i64 %indvars.iv.i
  %t21 = load i32, i32* %arrayidx4.i, align 4
  %add5.i = add nsw i32 %t21, %add.i
  store i32 %add5.i, i32* %arrayidx4.i, align 4
  %arrayidx7.i = getelementptr inbounds i32, i32* %p, i64 %indvars.iv.i
  %t22 = load i32, i32* %arrayidx7.i, align 4
  %add8.i = add nsw i32 %t22, %add5.i
  store i32 %add8.i, i32* %arrayidx7.i, align 4
  %arrayidx10.i = getelementptr inbounds i32, i32* %o, i64 %indvars.iv.i
  %t23 = load i32, i32* %arrayidx10.i, align 4
  %add11.i = add nsw i32 %t23, %add8.i
  store i32 %add11.i, i32* %arrayidx10.i, align 4
  %arrayidx13.i = getelementptr inbounds i32, i32* %n, i64 %indvars.iv.i
  %t24 = load i32, i32* %arrayidx13.i, align 4
  %add14.i = add nsw i32 %t24, %add11.i
  store i32 %add14.i, i32* %arrayidx13.i, align 4
  %arrayidx16.i = getelementptr inbounds i32, i32* %m, i64 %indvars.iv.i
  %t25 = load i32, i32* %arrayidx16.i, align 4
  %add17.i = add nsw i32 %t25, %add14.i
  store i32 %add17.i, i32* %arrayidx16.i, align 4
  %arrayidx19.i = getelementptr inbounds i32, i32* %l, i64 %indvars.iv.i
  %t26 = load i32, i32* %arrayidx19.i, align 4
  %add20.i = add nsw i32 %t26, %add17.i
  store i32 %add20.i, i32* %arrayidx19.i, align 4
  %arrayidx22.i = getelementptr inbounds i32, i32* %k, i64 %indvars.iv.i
  %t27 = load i32, i32* %arrayidx22.i, align 4
  %add23.i = add nsw i32 %t27, %add20.i
  store i32 %add23.i, i32* %arrayidx22.i, align 4
  %arrayidx25.i = getelementptr inbounds i32, i32* %j, i64 %indvars.iv.i
  %t28 = load i32, i32* %arrayidx25.i, align 4
  %add26.i = add nsw i32 %t28, %add23.i
  store i32 %add26.i, i32* %arrayidx25.i, align 4
  %arrayidx28.i = getelementptr inbounds i32, i32* %i, i64 %indvars.iv.i
  %t29 = load i32, i32* %arrayidx28.i, align 4
  %add29.i = add nsw i32 %t29, %add26.i
  store i32 %add29.i, i32* %arrayidx28.i, align 4
  %arrayidx31.i = getelementptr inbounds i32, i32* %h, i64 %indvars.iv.i
  %t30 = load i32, i32* %arrayidx31.i, align 4
  %add32.i = add nsw i32 %t30, %add29.i
  store i32 %add32.i, i32* %arrayidx31.i, align 4
  %arrayidx34.i = getelementptr inbounds i32, i32* %g, i64 %indvars.iv.i
  %t31 = load i32, i32* %arrayidx34.i, align 4
  %add35.i = add nsw i32 %t31, %add32.i
  store i32 %add35.i, i32* %arrayidx34.i, align 4
  %arrayidx37.i = getelementptr inbounds i32, i32* %f, i64 %indvars.iv.i
  %t32 = load i32, i32* %arrayidx37.i, align 4
  %add38.i = add nsw i32 %t32, %add35.i
  store i32 %add38.i, i32* %arrayidx37.i, align 4
  %arrayidx40.i = getelementptr inbounds i32, i32* %e, i64 %indvars.iv.i
  %t33 = load i32, i32* %arrayidx40.i, align 4
  %add41.i = add nsw i32 %t33, %add38.i
  store i32 %add41.i, i32* %arrayidx40.i, align 4
  %arrayidx43.i = getelementptr inbounds i32, i32* %d, i64 %indvars.iv.i
  %t34 = load i32, i32* %arrayidx43.i, align 4
  %add44.i = add nsw i32 %t34, %add41.i
  store i32 %add44.i, i32* %arrayidx43.i, align 4
  %arrayidx46.i = getelementptr inbounds i32, i32* %c, i64 %indvars.iv.i
  %t35 = load i32, i32* %arrayidx46.i, align 4
  %add47.i = add nsw i32 %t35, %add44.i
  store i32 %add47.i, i32* %arrayidx46.i, align 4
  %arrayidx49.i = getelementptr inbounds i32, i32* %b, i64 %indvars.iv.i
  %t36 = load i32, i32* %arrayidx49.i, align 4
  %add50.i = add nsw i32 %t36, %add47.i
  store i32 %add50.i, i32* %arrayidx49.i, align 4
  %arrayidx52.i = getelementptr inbounds i32, i32* %a, i64 %indvars.iv.i
  %t37 = load i32, i32* %arrayidx52.i, align 4
  %add53.i = add nsw i32 %t37, %add50.i
  store i32 %add53.i, i32* %arrayidx52.i, align 4
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.i = icmp eq i64 %indvars.iv.next.i, 100
  br i1 %exitcond.i, label %foo.exit, label %for.body.i

foo.exit:                                         ; preds = %for.body.i
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

