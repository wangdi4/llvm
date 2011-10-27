; RUN: llc -mcpu=sandybridge < %s | grep vinsertf128 | count 1 

define <8 x i32> @test(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
  %1 = add <8 x i32> %v1, %v2
  %2 = add <8 x i32> %1, %v1
  ret <8 x i32> %2
}
