

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allOne_v2(<2 x i1> %pred) {
entry:
  %elem0 = extractelement <2 x i1> %pred, i32 0
  %elem1 = extractelement <2 x i1> %pred, i32 1
  %res = and i1 %elem0, %elem1
  ret i1 %res
}

define i1 @allOne_v4(<4 x i1> %pred) {
entry:
  %elem0 = extractelement <4 x i1> %pred, i32 0
  %elem1 = extractelement <4 x i1> %pred, i32 1
  %elem2 = extractelement <4 x i1> %pred, i32 2
  %elem3 = extractelement <4 x i1> %pred, i32 3

  %res1 = and i1 %elem0, %elem1
  %res2 = and i1 %elem2, %elem3

  %res = and i1 %res1, %res2
  ret i1 %res
}

define i1 @allOne_v8(<8 x i1> %pred) {
entry:
  %elem0 = extractelement <8 x i1> %pred, i32 0
  %elem1 = extractelement <8 x i1> %pred, i32 1
  %elem2 = extractelement <8 x i1> %pred, i32 2
  %elem3 = extractelement <8 x i1> %pred, i32 3
  %elem4 = extractelement <8 x i1> %pred, i32 4
  %elem5 = extractelement <8 x i1> %pred, i32 5
  %elem6 = extractelement <8 x i1> %pred, i32 6
  %elem7 = extractelement <8 x i1> %pred, i32 7

  %res1 = and i1 %elem0, %elem1
  %res2 = and i1 %elem2, %elem3
  %res3 = and i1 %elem4, %elem5
  %res4 = and i1 %elem6, %elem7

  %res5 = and i1 %res1, %res2
  %res6 = and i1 %res3, %res4

  %res = and i1 %res5, %res6
  ret i1 %res
}

define i1 @allOne_v16(<16 x i1> %pred) {
entry:
  %elem0 = extractelement <16 x i1> %pred, i32 0
  %elem1 = extractelement <16 x i1> %pred, i32 1
  %elem2 = extractelement <16 x i1> %pred, i32 2
  %elem3 = extractelement <16 x i1> %pred, i32 3
  %elem4 = extractelement <16 x i1> %pred, i32 4
  %elem5 = extractelement <16 x i1> %pred, i32 5
  %elem6 = extractelement <16 x i1> %pred, i32 6
  %elem7 = extractelement <16 x i1> %pred, i32 7
  %elem8 = extractelement <16 x i1> %pred, i32 8
  %elem9 = extractelement <16 x i1> %pred, i32 9
  %elem10 = extractelement <16 x i1> %pred, i32 10
  %elem11 = extractelement <16 x i1> %pred, i32 11
  %elem12 = extractelement <16 x i1> %pred, i32 12
  %elem13 = extractelement <16 x i1> %pred, i32 13
  %elem14 = extractelement <16 x i1> %pred, i32 14
  %elem15 = extractelement <16 x i1> %pred, i32 15

  %res1 = and i1 %elem0, %elem1
  %res2 = and i1 %elem2, %elem3
  %res3 = and i1 %elem4, %elem5
  %res4 = and i1 %elem6, %elem7
  %res5 = and i1 %elem8, %elem9
  %res6 = and i1 %elem10, %elem11
  %res7 = and i1 %elem12, %elem13
  %res8 = and i1 %elem14, %elem15

  %res9 = and i1 %res1, %res2
  %res10 = and i1 %res3, %res4
  %res11 = and i1 %res5, %res6
  %res12 = and i1 %res7, %res8

  %res13 = and i1 %res9, %res10
  %res14 = and i1 %res11, %res12
  
  %res = and i1 %res13, %res14
  ret i1 %res
}





define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %t
}

define i1 @allZero_v2(<2 x i1> %t) {
entry:
  %pred = xor <2 x i1> %t, <i1 true, i1 true>
  %elem0 = extractelement <2 x i1> %pred, i32 0
  %elem1 = extractelement <2 x i1> %pred, i32 1
  %res = and i1 %elem0, %elem1
  ret i1 %res
}

define i1 @allZero_v4(<4 x i1> %t) {
entry:
  %pred = xor <4 x i1> %t, <i1 true, i1 true, i1 true, i1 true>
  %elem0 = extractelement <4 x i1> %pred, i32 0
  %elem1 = extractelement <4 x i1> %pred, i32 1
  %elem2 = extractelement <4 x i1> %pred, i32 2
  %elem3 = extractelement <4 x i1> %pred, i32 3

  %res1 = and i1 %elem0, %elem1
  %res2 = and i1 %elem2, %elem3

  %res = and i1 %res1, %res2
  ret i1 %res
}

define i1 @allZero_v8(<8 x i1> %t) {
entry:
  %pred = xor <8 x i1> %t, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %elem0 = extractelement <8 x i1> %pred, i32 0
  %elem1 = extractelement <8 x i1> %pred, i32 1
  %elem2 = extractelement <8 x i1> %pred, i32 2
  %elem3 = extractelement <8 x i1> %pred, i32 3
  %elem4 = extractelement <8 x i1> %pred, i32 4
  %elem5 = extractelement <8 x i1> %pred, i32 5
  %elem6 = extractelement <8 x i1> %pred, i32 6
  %elem7 = extractelement <8 x i1> %pred, i32 7

  %res1 = and i1 %elem0, %elem1
  %res2 = and i1 %elem2, %elem3
  %res3 = and i1 %elem4, %elem5
  %res4 = and i1 %elem6, %elem7

  %res5 = and i1 %res1, %res2
  %res6 = and i1 %res3, %res4

  %res = and i1 %res5, %res6
  ret i1 %res
}

define i1 @allZero_v16(<16 x i1> %t) {
entry:
  %pred = xor <16 x i1> %t, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %elem0 = extractelement <16 x i1> %pred, i32 0
  %elem1 = extractelement <16 x i1> %pred, i32 1
  %elem2 = extractelement <16 x i1> %pred, i32 2
  %elem3 = extractelement <16 x i1> %pred, i32 3
  %elem4 = extractelement <16 x i1> %pred, i32 4
  %elem5 = extractelement <16 x i1> %pred, i32 5
  %elem6 = extractelement <16 x i1> %pred, i32 6
  %elem7 = extractelement <16 x i1> %pred, i32 7
  %elem8 = extractelement <16 x i1> %pred, i32 8
  %elem9 = extractelement <16 x i1> %pred, i32 9
  %elem10 = extractelement <16 x i1> %pred, i32 10
  %elem11 = extractelement <16 x i1> %pred, i32 11
  %elem12 = extractelement <16 x i1> %pred, i32 12
  %elem13 = extractelement <16 x i1> %pred, i32 13
  %elem14 = extractelement <16 x i1> %pred, i32 14
  %elem15 = extractelement <16 x i1> %pred, i32 15

  %res1 = and i1 %elem0, %elem1
  %res2 = and i1 %elem2, %elem3
  %res3 = and i1 %elem4, %elem5
  %res4 = and i1 %elem6, %elem7
  %res5 = and i1 %elem8, %elem9
  %res6 = and i1 %elem10, %elem11
  %res7 = and i1 %elem12, %elem13
  %res8 = and i1 %elem14, %elem15

  %res9 = and i1 %res1, %res2
  %res10 = and i1 %res3, %res4
  %res11 = and i1 %res5, %res6
  %res12 = and i1 %res7, %res8

  %res13 = and i1 %res9, %res10
  %res14 = and i1 %res11, %res12
  
  %res = and i1 %res13, %res14
  ret i1 %res
}

; Scalar
declare void @barrier(i32 %type);

define void @barrier_v1(i1 %mask, i32 %type) {
  br i1 %mask, label %do, label %dont
do:
  call void @barrier(i32 %type)
  br label %dont
dont:
  ret void
}

define void @barrier_v2(<2 x i1> %mask, <2 x i32> %type) {
  %m0 = extractelement <2 x i1> %mask, i32 0
  br i1 %m0, label %do, label %dont
do:
  %t0 = extractelement <2 x i32> %type, i32 0
  call void @barrier(i32 %t0)
  br label %dont
dont:
  ret void
}

define void @barrier_v4(<4 x i1> %mask, <4 x i32> %type) {
  %m0 = extractelement <4 x i1> %mask, i32 0
  br i1 %m0, label %do, label %dont
do:
  %t0 = extractelement <4 x i32> %type, i32 0
  call void @barrier(i32 %t0)
  br label %dont
dont:
  ret void
}

define void @barrier_v8(<8 x i1> %mask, <8 x i32> %type) {
  %m0 = extractelement <8 x i1> %mask, i32 0
  br i1 %m0, label %do, label %dont
do:
  %t0 = extractelement <8 x i32> %type, i32 0
  call void @barrier(i32 %t0)
  br label %dont
dont:
  ret void
}

define void @barrier_v16(<16 x i1> %mask, <16 x i32> %type) {
  %m0 = extractelement <16 x i1> %mask, i32 0
  br i1 %m0, label %do, label %dont
do:
  %t0 = extractelement <16 x i32> %type, i32 0
  call void @barrier(i32 %t0)
  br label %dont
dont:
  ret void
}

