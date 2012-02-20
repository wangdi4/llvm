; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc
;

define void @A(i32 %c, i32* nocapture %y) nounwind {
; <label>:0
  br label %1

; <label>:1                                       
  %2 = phi i32 [ 0, %0 ], [ %5, %1 ]
  %x.01 = phi i32 [ undef, %0 ], [ %4, %1 ]
  %3 = icmp eq i32 %2, 0
  %x.0.op = add i32 %x.01, 1
  %4 = select i1 %3, i32 11, i32 %x.0.op
  %5 = add nsw i32 %2, 1
  %exitcond = icmp eq i32 %5, 87
  br i1 %exitcond, label %6, label %1

; <label>:6                                       
  store i32 %4, i32* %y, align 4
  ret void
}

define void @B(i64 %c, i64* nocapture %y) nounwind {
; <label>:0
  br label %1

; <label>:1                                       
  %2 = phi i64 [ 0, %0 ], [ %5, %1 ]
  %x.01 = phi i64 [ undef, %0 ], [ %4, %1 ]
  %3 = icmp eq i64 %2, 0
  %x.0.op = add i64 %x.01, 1
  %4 = select i1 %3, i64 11, i64 %x.0.op
  %5 = add nsw i64 %2, 1
  %exitcond = icmp eq i64 %5, 87
  br i1 %exitcond, label %6, label %1

; <label>:6                                       
  store i64 %4, i64* %y, align 4
  ret void
}

define void @C(float %c, float* nocapture %y) nounwind {
; <label>:0
  br label %1

; <label>:1                                       
  %2 = phi float [ 0.0, %0 ], [ %5, %1 ]
  %x.01 = phi float [ undef, %0 ], [ %4, %1 ]
  %3 = fcmp eq float %2, 0.0
  %x.0.op = fadd float %x.01, 1.0
  %4 = select i1 %3, float 11.0, float %x.0.op
  %5 = fadd float %2, 1.0
  %exitcond = fcmp eq float %5, 87.0
  br i1 %exitcond, label %6, label %1

; <label>:6                                       
  store float %4, float* %y, align 4
  ret void
}

define void @D(double %c, double* nocapture %y) nounwind {
; <label>:0
  br label %1

; <label>:1                                       
  %2 = phi double [ 0.0, %0 ], [ %5, %1 ]
  %x.01 = phi double [ undef, %0 ], [ %4, %1 ]
  %3 = fcmp eq double %2, 0.0
  %x.0.op = fadd double %x.01, 1.0
  %4 = select i1 %3, double 11.0, double %x.0.op
  %5 = fadd double %2, 1.0
  %exitcond = fcmp eq double %5, 87.0
  br i1 %exitcond, label %6, label %1

; <label>:6                                       
  store double %4, double* %y, align 4
  ret void
}
