; RUN: opt < %s -passes='print<branch-prob>' -disable-output 2>&1 | FileCheck %s

define <4 x double> @weighted_averaging_pixels(ptr %k.init, ptr %pixels.addr.init, i64 %iv.init) {
entry:
  br label %loop0

loop0:
  %result0 = phi <4 x double> [ zeroinitializer, %entry ], [ %result1, %loop1 ]
  %i0 = phi i64 [0, %entry], [%i0.next, %loop1]
  %i0.next = add nsw i64 %i0, 1
  %cond0 = icmp ult i64 %i0, %iv.init
  br i1 %cond0, label %loop1, label %exit

loop1:
  %result1 = phi <4 x double> [ %result0, %loop0 ], [ %result2, %loop2 ]
  %i1 = phi i64 [0, %loop0], [%i1.next, %loop2]
  %i1.next = add nsw i64 %i1, 1
  %cond1 = icmp ult i64 %i1, %iv.init
  br i1 %cond1, label %loop2, label %loop0

loop2:
  %result2 = phi <4 x double> [ %result1, %loop1 ], [ %result3, %loop3 ]
  %i2 = phi i64 [0, %loop1], [%i2.next, %loop3]
  %i2.next = add nsw i64 %i2, 1
  %cond2 = icmp ult i64 %i2, %iv.init
  br i1 %cond2, label %loop3, label %loop1

loop3:
  %result3 = phi <4 x double> [ %result2, %loop2 ], [ %result4.final, %ifmerge ]
  %i3 = phi i64 [0, %loop2], [%i3.next, %ifmerge]
  %i3.next = add nsw i64 %i3, 1
  %cond3 = icmp ult i64 %i3, %iv.init
  br i1 %cond3, label %loop4.preheader, label %loop2

loop4.preheader:
  %cond = icmp ult i64 %iv.init, 2
  br i1 %cond, label %ifmerge, label %loop4

loop4:
  %k.iv = phi ptr [ %k.iv.next, %loop4 ], [ %k.init, %loop4.preheader ]
  %pixels.addr.iv = phi ptr [ %pixels.addr.iv.next, %loop4 ], [ %pixels.addr.init, %loop4.preheader ]
  %iv = phi i64 [ %iv.next, %loop4 ], [ %iv.init, %loop4.preheader ]
  %result.iv = phi <4 x double> [ %result4, %loop4 ], [ %result3, %loop4.preheader ]
  %k0 = load double, ptr %k.iv, align 8
  %k1.addr = getelementptr i8, ptr %k.iv, i64 -8
  %k1 = load double, ptr %k1.addr, align 8
  %k0.insert = insertelement <4 x double> poison, double %k0, i64 0
  %pixels = load <8 x i16>, ptr %pixels.addr.iv, align 2
  %pixels0 = shufflevector <8 x i16> %pixels, <8 x i16> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %pixels1 = shufflevector <8 x i16> %pixels, <8 x i16> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %pixels0.fp = uitofp <4 x i16> %pixels0 to <4 x double>
  %k0.splat = shufflevector <4 x double> %k0.insert, <4 x double> poison, <4 x i32> zeroinitializer
  %mul0 = fmul fast <4 x double> %k0.splat, %pixels0.fp
  %add0 = fadd fast <4 x double> %mul0, %result.iv
  %pixels1.fp = uitofp <4 x i16> %pixels1 to <4 x double>
  %k1.insert = insertelement <4 x double> poison, double %k1, i64 0
  %k1.splat = shufflevector <4 x double> %k1.insert, <4 x double> poison, <4 x i32> zeroinitializer
  %mul1 = fmul fast <4 x double> %k1.splat, %pixels1.fp
  %result4 = fadd fast <4 x double> %mul1, %add0
  %iv.next = add nsw i64 %iv, -1
  %pixels.addr.iv.next = getelementptr i8, ptr %pixels.addr.iv, i64 16
  %k.iv.next = getelementptr i8, ptr %k.iv, i64 -16
  %condloop = icmp eq i64 %iv.next, 0
  br i1 %condloop, label %ifmerge, label %loop4
; CHECK: edge loop4 -> ifmerge probability is 0x00af3ade / 0x80000000 = 0.53%
; CHECK: edge loop4 -> loop4 probability is 0x7f50c522 / 0x80000000 = 99.47% [HOT edge]

ifmerge:
  %result4.final = phi <4 x double> [ %result3, %loop4.preheader ], [ %result4, %loop4 ]
  br label %loop3

exit:
  ret <4 x double> %result0
}
