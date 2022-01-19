; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction  -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-dead-store-elimination,print<hir>" -vplan-force-vf=4  < %s 2>&1 | FileCheck %s

; Verify that DSE does not eliminate first masked store by checking
; post-domination with second masked store.

; Before DSE

; CHECK: + DO i1 = 0, 63, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK: |   %.vec5 = undef;
; CHECK: |   %.vec4 = undef;
; CHECK: |   %.vec = i1 + <i32 0, i32 1, i32 2, i32 3> <u 10;
; CHECK: |   %.vec1 = %.vec  ^  -1;
; CHECK: |   %.vec2 = i1 + <i32 0, i32 1, i32 2, i32 3> <u 30;
; CHECK: |   %.vec3 = %.vec1  &  %.vec2;
; CHECK: |   %.vec4 = (<4 x i32>*)(%B)[i1], Mask = @{%.vec3};
; CHECK: |   (<4 x i32>*)(%A)[i1] = %.vec4 + 15, Mask = @{%.vec3};
; CHECK: |   %.vec5 = (<4 x i32>*)(%B)[i1], Mask = @{%.vec};
; CHECK: |   (<4 x i32>*)(%A)[i1] = %.vec5 + 5, Mask = @{%.vec};
; CHECK: + END LOOP

; After DSE

; CHECK: |   (<4 x i32>*)(%A)[i1] = %.vec4 + 15, Mask = @{%.vec3};
; CHECK: |   (<4 x i32>*)(%A)[i1] = %.vec5 + 5, Mask = @{%.vec};

target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* noalias nocapture readonly %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.019 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp1 = icmp ult i32 %i.019, 10
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %ptridx = getelementptr inbounds i32, i32* %B, i32 %i.019
  %0 = load i32, i32* %ptridx, align 4
  %add = add nsw i32 %0, 5
  %ptridx2 = getelementptr inbounds i32, i32* %A, i32 %i.019
  store i32 %add, i32* %ptridx2, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %cmp3 = icmp ult i32 %i.019, 30
  br i1 %cmp3, label %if.then4, label %for.inc

if.then4:                                         ; preds = %if.else
  %ptridx5 = getelementptr inbounds i32, i32* %B, i32 %i.019
  %1 = load i32, i32* %ptridx5, align 4
  %add6 = add nsw i32 %1, 15
  %ptridx7 = getelementptr inbounds i32, i32* %A, i32 %i.019
  store i32 %add6, i32* %ptridx7, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.then4, %if.else
  %inc = add nuw nsw i32 %i.019, 1
  %exitcond = icmp eq i32 %inc, 64
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

attributes #0 = { nofree norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="broadwell" "target-features"="+adx,+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
