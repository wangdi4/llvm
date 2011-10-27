define <8 x i64> @broadcast1(i64 %A, i64 %B) nounwind readonly {
; CHECK: broadcast1
; CHECK: vpbroadcastq
  %C = add i64 %A, %B
  %broadcast1 = insertelement <8 x i64> undef, i64 %C, i32 0
  %broadcast2 = shufflevector <8 x i64> %broadcast1, <8 x i64> undef, <8 x i32> zeroinitializer
  %D = add <8 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  %E = and <8 x i64> %D, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  ret <8 x i64> %E
  }

define <4 x i64> @broadcast4xi64(i64 %A, i64 %B) nounwind readonly {
; CHECK: broadcast4xi64
; CHECK: vpbroadcastq
  %C = add i64 %A, %B
  %D = insertelement <4 x i64> undef, i64 %C, i32 0 
  ret < 4 x i64> %D
  }

  define <4 x i32> @broadcast4xi32(i32 %A, i32 %B) nounwind readonly {
; CHECK: broadcast4xi32
; CHECK: vpbroadcastd
  %C = add i32 %A, %B
  %D = insertelement <4 x i32> undef, i32 %C, i32 0 
  ret < 4 x i32> %D
  }
  
define <16 x i16> @broadcast16xi16(i16 %A, i16 %B) nounwind readonly {
; CHECK: broadcast16xi16
; CHECK: vpbroadcastw
  %C = add i16 %A, %B
  %D = insertelement <16 x i16> undef, i16 %C, i32 0 
  ret < 16 x i16> %D
  }  