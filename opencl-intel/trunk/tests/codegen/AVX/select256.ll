; RUN: llc < %s

define  x86_ocl_kernelcc <8 x float> @test_v8f32(<8 x float> %a, <8 x float> %b, i32 %c) nounwind {
deload:
  br label %postload1554
postload1554:                                    
  %bit6509 = icmp slt i32 %c, 0
  %out_sel2461238_comp = select i1 %bit6509, <8 x float> %a, <8 x float> %b
  ret <8 x float> %out_sel2461238_comp
}

define  x86_ocl_kernelcc <4 x double> @test_v4f64(<4 x double> %a, <4 x double> %b, i32 %c) nounwind {
deload:
  br label %postload1554
postload1554:                                    
  %bit6509 = icmp slt i32 %c, 0
  %out_sel2461238_comp = select i1 %bit6509, <4 x double> %a, <4 x double> %b
  ret <4 x double> %out_sel2461238_comp
}

define  x86_ocl_kernelcc <4 x i64> @test_v4i64(<4 x i64> %a, <4 x i64> %b, i32 %c) nounwind {
deload:
  br label %postload1554
postload1554:                                    
  %bit6509 = icmp slt i32 %c, 0
  %out_sel2461238_comp = select i1 %bit6509, <4 x i64> %a, <4 x i64> %b
  ret <4 x i64> %out_sel2461238_comp
}

define  x86_ocl_kernelcc <8 x i32> @test_v8i32(<8 x i32> %a, <8 x i32> %b, i32 %c) nounwind {
deload:
  br label %postload1554
postload1554:                                    
  %bit6509 = icmp slt i32 %c, 0
  %out_sel2461238_comp = select i1 %bit6509, <8 x i32> %a, <8 x i32> %b
  ret <8 x i32> %out_sel2461238_comp
}

define  x86_ocl_kernelcc <16 x i16> @test_v16i16(<16 x i16> %a, <16 x i16> %b, i32 %c) nounwind {
deload:
  br label %postload1554
postload1554:                                    
  %bit6509 = icmp slt i32 %c, 0
  %out_sel2461238_comp = select i1 %bit6509, <16 x i16> %a, <16 x i16> %b
  ret <16 x i16> %out_sel2461238_comp
}


define  x86_ocl_kernelcc <32 x i8> @test_v32i8(<32 x i8> %a, <32 x i8> %b, i32 %c) nounwind {
deload:
  br label %postload1554
postload1554:                                    
  %bit6509 = icmp slt i32 %c, 0
  %out_sel2461238_comp = select i1 %bit6509, <32 x i8> %a, <32 x i8> %b
  ret <32 x i8> %out_sel2461238_comp
}


