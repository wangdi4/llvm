; RUN: llc -mcpu=sandybridge < %s

define  void @test1() nounwind {
deload:
  %Mneg_32638 = xor i32 0, -1
  %temp136_32640 = insertelement <8 x i32> undef, i32 %Mneg_32638, i32 0
  %vector137_32642 = shufflevector <8 x i32> %temp136_32640, <8 x i32> undef, <8 x i32> zeroinitializer
  br label %postload523

 postload523:                                      ; preds = %postload523, %deload
  %vectorPHI135_32650 = phi <8 x i32> [ %vector137_32642, %deload ], [ zeroinitializer, %postload523 ]
  br label %postload523
  ret void
}
