; RUN: llc < %s -mtriple=x86_64-unknown-linux-gnu

define double @Fn(double %r, double %s, double %mul3937, double %mul3938, double %mul3939, double %mul3941) #0 {
if.then3936:
  %mul39371 = fmul fast double %r, %r
  %mul39382 = fmul fast double %r, %r
  %mul39393 = fmul fast double %mul3937, %mul3937
  %mul39414 = fmul fast double %r, %r
  %mul3942 = fmul fast double %mul39371, %r
  %mul3943 = fmul fast double %mul39382, %mul3942
  %mul3944 = fmul fast double %mul39382, %mul39371
  %mul3945 = fmul fast double 0.000000e+00, 0.000000e+00
  %0 = fadd fast double 0.000000e+00, 0.000000e+00
  %sub3948 = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3950.neg = fmul fast double %s, 0xC010EC4EC4EC4EC5
  %mul3952 = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3962 = fmul fast double %mul3943, 0x3FE530CD7C25AC18
  %mul3965 = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3967.neg = fmul fast double %mul3944, 0xBFDAF84B582FF24D
  %mul3970.neg = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3972 = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3974 = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3976.neg = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3979.neg = fmul fast double %mul3942, 0xBFAED29F4036CBC6
  %mul3981 = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3984 = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3986.neg = fmul fast double 0.000000e+00, 0.000000e+00
  %div3989.neg = fmul fast double 0.000000e+00, 0.000000e+00
  %mul3991 = fmul fast double 0.000000e+00, 0.000000e+00
  %reass.add10211 = fsub fast double %mul39393, %mul3943
  %reass.mul10212 = fmul fast double %reass.add10211, 0x401E762762762762
  %reass.add10200 = fadd fast double 0.000000e+00, 0.000000e+00
  %reass.add10201 = fadd fast double 0.000000e+00, 0.000000e+00
  %reass.add10208 = fadd fast double 0.000000e+00, 0.000000e+00
  %reass.mul10209 = fmul fast double 0.000000e+00, 0.000000e+00
  %reass.add10202 = fadd fast double 0.000000e+00, 0.000000e+00
  %reass.add10204 = fadd fast double 0.000000e+00, 0.000000e+00
  %reass.add10205 = fadd fast double 0.000000e+00, %reass.mul10212
  %reass.add10206 = fadd fast double %reass.add10205, %mul3979.neg
  %reass.add10213 = fadd fast double 0.000000e+00, 0.000000e+00
  %reass.add10214 = fadd fast double %reass.add10206, %mul3950.neg
  %reass.mul10215 = fmul fast double 0.000000e+00, %r
  %.neg10197 = fadd fast double 0.000000e+00, 0.000000e+00
  %add3975 = fadd fast double 0.000000e+00, 0.000000e+00
  %sub3980 = fadd fast double 0.000000e+00, 0.000000e+00
  %add3982 = fadd fast double 0.000000e+00, 0.000000e+00
  %add3985 = fadd fast double 0.000000e+00, 0.000000e+00
  %sub3990 = fadd fast double 0.000000e+00, 0x3FE530CD7C25AC18
  %add3992 = fadd fast double %mul3962, %mul3967.neg
  %add3994 = fadd fast double 0.000000e+00, 0.000000e+00
  %sub3995 = fadd fast double 0.000000e+00, 0.000000e+00
  %add3996 = fadd fast double %add3992, %reass.add10214
  br label %if.end5479

if.end5479:                                       ; preds = %if.then3936
  ret double %add3996
}

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare double @llvm.vector.reduce.fadd.v4f64(double, <4 x double>) #1

attributes #0 = { "target-cpu"="skylake-avx512" }
attributes #1 = { nocallback nofree nosync nounwind readnone willreturn }
