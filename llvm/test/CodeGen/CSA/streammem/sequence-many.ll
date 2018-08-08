; RUN: llc -mtriple=csa < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @func(i64* noalias %a, i64* noalias %b, i64* noalias %c, i64* noalias %d, i64* noalias %e, i64 %N) #0 {
entry:
  br label %loop

loop:
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %loop ]
  %addr.a = getelementptr inbounds i64, i64* %a, i64 %indvar
  %addr.b = getelementptr inbounds i64, i64* %b, i64 %indvar
  %addr.c = getelementptr inbounds i64, i64* %c, i64 %indvar
  %addr.d = getelementptr inbounds i64, i64* %d, i64 %indvar
  %addr.e = getelementptr inbounds i64, i64* %e, i64 %indvar
  %v1 = load i64, i64* %addr.a, align 8
  %v2 = load i64, i64* %addr.b, align 8
  %v3 = load i64, i64* %addr.c, align 8
  %v4 = load i64, i64* %addr.d, align 8
  %tmp.0 = add i64 %v1, %v2
  %tmp.1 = mul i64 %v3, %v4
  %val = sub i64 %tmp.0, %tmp.1
  store i64 %val, i64* %addr.e, align 8
  %indvar.next = add nuw i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %N
  br i1 %exitcond, label %exit, label %loop
; CHECK: .param .lic .i1 %[[INORD:[a-z0-9_]+]]
; CHECK: .param .lic .i64 %[[ADDRA:[a-z0-9_]+]]
; CHECK: .param .lic .i64 %[[ADDRB:[a-z0-9_]+]]
; CHECK: .param .lic .i64 %[[ADDRC:[a-z0-9_]+]]
; CHECK: .param .lic .i64 %[[ADDRD:[a-z0-9_]+]]
; CHECK: .param .lic .i64 %[[ADDRE:[a-z0-9_]+]]
; CHECK: .param .lic .i64 %[[LEN:[a-z0-9_]+]]
; CHECK: cmpne64 %[[OT:[a-z0-9_]+]], %[[LEN]], 0
; CHECK: merge64 %[[SAFELEN:[a-z0-9_]+]], %[[OT]], 1, %[[LEN]]
; CHECK-DAG: sld64 %{{[a-z0-9_]+}}, %[[ADDRA]], %[[SAFELEN]], 1, %ign, %[[INORD]], MEMLEVEL_T0
; CHECK-DAG: sld64 %{{[a-z0-9_]+}}, %[[ADDRB]], %[[SAFELEN]], 1, %ign, %[[INORD]], MEMLEVEL_T0
; CHECK-DAG: sld64 %{{[a-z0-9_]+}}, %[[ADDRC]], %[[SAFELEN]], 1, %ign, %[[INORD]], MEMLEVEL_T0
; CHECK-DAG: sld64 %{{[a-z0-9_]+}}, %[[ADDRD]], %[[SAFELEN]], 1, %ign, %[[INORD]], MEMLEVEL_T0
; CHECK-DAG: sst64 %[[ADDRE]], %[[SAFELEN]], 1, %{{[a-z0-9_]+}}, %{{[a-z0-9_]+}}, %ign, MEMLEVEL_T0

exit:
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
