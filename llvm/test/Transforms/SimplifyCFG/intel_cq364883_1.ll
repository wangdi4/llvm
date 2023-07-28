; Test whether the 3-entry phi node is folded into a 2-entry phi node.
; Fails if the 3-entry phi node still exists, passes otherwise.
;
; RUN: opt -passes="simplifycfg" < %s -S | FileCheck %s
; INTEL
; RUN: opt < %s -S -passes=convert-to-subscript | opt -passes="simplifycfg" -S | FileCheck %s
;

@x = common global [1024 x double] zeroinitializer, align 16

define double @foo(i32 %i) #0 {

entry:
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds [1024 x double], ptr @x, i64 0, i64 %idxprom
  %0 = load double, ptr %arrayidx, align 8
  %max1_cmp = fcmp ogt double %0, -3.276800e+04
  br i1 %max1_cmp, label %max1.true, label %max1.end

max1.true:
  %min_cmp = fcmp olt double %0, 3.276700e+04
  br label %max1.end

max1.end:
  %cond = phi i1 [ %min_cmp, %max1.true ], [ true, %entry ]
  br i1 %cond, label %min.true, label %end

min.true:
  %1 = load double, ptr %arrayidx, align 8
  %max2_cmp = fcmp ogt double %1, -3.276800e+04
  br i1 %max2_cmp, label %max2.true, label %end

max2.true:
  br label %end

end:
; CHECK-NOT: phi double [{{.*}}], [{{.*}}], [{{.*}}]
  %res = phi double [ %1, %max2.true ], [ -3.276800e+04, %min.true ], [ 3.276700e+04, %max1.end]
  ret double %res
}
