; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test verifies that the dtrans::ResolveTypes correctly combines
; types that have the same base name but different suffixes if the types
; have the same layout but does not combine them if the layout of types
; named this way is different.

; These types should all be combined.
%struct.test01 = type { i32, i64, i32 }
%struct.test01.123 = type { i32, i64, i32 }
%struct.test01.456 = type { i32, i64, i32 }

; CHECK-LABEL: %__DTRT_struct.test01 = type { i32, i64, i32 }
; CHECK-NOT: %struct.test01 = type { i32, i64, i32 }
; CHECK-NOT: %struct.test01.123 = type { i32, i64, i32 }
; CHECK-NOT: %struct.test01.456 = type { i32, i64, i32 }

; These types should not be combined
%struct.test02 = type { i32, i32 }
%struct.test02.1 = type { i16, i16, i32 }

; CHECK-LABEL: %struct.test02 = type { i32, i32 }
; CHECK: %struct.test02.1 = type { i16, i16, i32 }

; These types with nested pointers should be combined.
%struct.test03.a = type { i32, %struct.test03.b* }
%struct.test03.b = type { i32, i32 }
%struct.test03.a.789 = type { i32, %struct.test03.b.135* }
%struct.test03.b.135 = type { i32, i32 }

; CHECK: %__DTRT_struct.test03.a = type { i32, %__DTRT_struct.test03.b* }
; CHECK: %__DTRT_struct.test03.b = type { i32, i32 }
; CHECK-NOT: %struct.test03.a = type { i32, %struct.test03.b* }
; CHECK-NOT: %struct.test03.b = type { i32, i32 }
; CHECK-NOT: %struct.test03.a.789 = type { i32, %struct.test03.b.135* }
; CHECK-NOT: %struct.test03.b.135 = type { i32, i32 }

; These types with nested pointers should not be combined.
%struct.test04.a = type { i32, %struct.test04.b* }
%struct.test04.b = type { i32, i32 }
%struct.test04.a.987 = type { i32, %struct.test04.b.531* }
%struct.test04.b.531 = type { i16, i16, i32 }

; CHECK-LABEL: %struct.test04.a = type { i32, %struct.test04.b* }
; CHECK: %struct.test04.b = type { i32, i32 }
; CHECK: %struct.test04.a.987 = type { i32, %struct.test04.b.531* }
; CHECK: %struct.test04.b.531 = type { i16, i16, i32 }

; These types with pointers to each other should be combined.
%struct.test05.a = type { i32, %struct.test05.b* }
%struct.test05.b = type { i32, %struct.test05.a* }
%struct.test05.a.11 = type { i32, %struct.test05.b.22* }
%struct.test05.b.22 = type { i32, %struct.test05.a.11* }

; CHECK: %__DTRT_struct.test05.a = type { i32, %__DTRT_struct.test05.b* }
; CHECK: %__DTRT_struct.test05.b = type { i32, %__DTRT_struct.test05.a* }
; CHECK-NOT: %struct.test05.a = type { i32, %struct.test05.b* }
; CHECK-NOT: %struct.test05.b = type { i32, %struct.test05.a* }
; CHECK-NOT: %struct.test05.a.11 = type { i32, %struct.test05.b.22* }
; CHECK-NOT: %struct.test05.b.22 = type { i32, %struct.test05.a.11* }

; These types with nested pointers should be combined.
; These differ from test03 in that the called functions reference the
; nested pointers.
%struct.test06.a = type { i32, %struct.test06.b* }
%struct.test06.b = type { i32, i32 }
%struct.test06.a.33 = type { i32, %struct.test06.b.44* }
%struct.test06.b.44 = type { i32, i32 }

; CHECK: %__DTRT_struct.test06.a = type { i32, %__DTRT_struct.test06.b* }
; CHECK: %__DTRT_struct.test06.b = type { i32, i32 }
; CHECK-NOT: %struct.test06.a = type { i32, %struct.test06.b* }
; CHECK-NOT: %struct.test06.b = type { i32, i32 }
; CHECK-NOT: %struct.test06.a.33 = type { i32, %struct.test06.b.44* }
; CHECK-NOT: %struct.test06.b.44 = type { i32, i32 }

; These types with nested pointers that differ only by type base name
; should not be combined.
%struct.test07.a = type { i32, %struct.test07.b* }
%struct.test07.b = type { i32, i32 }
%struct.test07.a.55 = type { i32, %struct.test07.b2.66* }
%struct.test07.b2.66 = type { i32, i32 }

; CHECK-LABEL: %struct.test07.a = type { i32, %struct.test07.b* }
; CHECK: %struct.test07.b = type { i32, i32 }
; Note that %struct.test07.b2.66 gets renamed. That's OK.
; CHECK: %struct.test07.a.55 = type { i32, %struct.test07.b2* }
; CHECK: %struct.test07.b2 = type { i32, i32 }

; The second and third types here should be combined with one another but
; not with the first type.
%struct.test08 = type { i32, i32, i32, i32 }
%struct.test08.77 = type { i32, i64, i32 }
%struct.test08.88 = type { i32, i64, i32 }

; CHECK-LABEL: %struct.test08 = type { i32, i32, i32, i32 }
; CHECK: %__DTRT_struct.test08.77 = type { i32, i64, i32 }
; CHECK-NOT: %struct.test08.77 = type { i32, i64, i32 }
; CHECK-NOT: %struct.test08.88 = type { i32, i64, i32 }

; These types should all be combined.
%struct.test09 = type { i32, i64, i32 }
%struct.test09.1 = type { i32, i64, i32 }
%struct.test09.2.3 = type { i32, i64, i32 }

; CHECK-LABEL: %__DTRT_struct.test09 = type { i32, i64, i32 }
; CHECK-NOT: %struct.test09 = type { i32, i64, i32 }
; CHECK-NOT: %struct.test09.1 = type { i32, i64, i32 }
; CHECK-NOT: %struct.test09.2.3 = type { i32, i64, i32 }

; The call interfaces are the important thing in the tests. We don't actually
; need to do anything with the elements.

define void @test01_a(%struct.test01* %p) {
  ret void
}
; CHECK-NOT: void @test01_a(%struct.test01* %p)

define void @test01_b(%struct.test01.123* %p) {
  ret void
}
; CHECK-NOT: void @test01_b(%struct.test01.123* %p)

define void @test01_c(%struct.test01.456* %p) {
  ret void
}
; CHECK-NOT: void @test01_c(%struct.test01.456* %p)

define void @test01() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test01*
  call void @test01_a(%struct.test01* %p)
  call void bitcast (void (%struct.test01.123*)* @test01_b
              to void (%struct.test01*)*) (%struct.test01* %p)
  call void bitcast (void (%struct.test01.456*)* @test01_c
              to void (%struct.test01*)*) (%struct.test01* %p)
  call void @free(i8* %buf)
  ret void
}

; CHECK-LABEL: void @test01()
; CHECK:  %buf = call i8* @malloc(i32 16)
; CHECK:  %p = bitcast i8* %buf to %__DTRT_struct.test01*
; CHECK:  call void @test01_a.1(%__DTRT_struct.test01* %p)
; CHECK:  call void @test01_b.2(%__DTRT_struct.test01* %p)
; CHECK:  call void @test01_c.3(%__DTRT_struct.test01* %p)
; CHECK:  call void @free(i8* %buf)

define void @test02_a(%struct.test02* %p) {
  ret void
}
; CHECK-LABEL: void @test02_a(%struct.test02* %p)

define void @test02_b(%struct.test02.1* %p) {
  ret void
}
; CHECK: void @test02_b(%struct.test02.1* %p)

define void @test02() {
  %buf = call i8* @malloc(i32 8)
  %p = bitcast i8* %buf to %struct.test02*
  call void @test02_a(%struct.test02* %p)
  call void @free(i8* %buf)

  %buf2 = call i8* @malloc(i32 8)
  %p2 = bitcast i8* %buf2 to %struct.test02.1*
  call void @test02_b(%struct.test02.1* %p2)
  call void @free(i8* %buf2)

  ret void
}

; CHECK-LABEL: void @test02()
; CHECK:  %buf = call i8* @malloc(i32 8)
; CHECK:  %p = bitcast i8* %buf to %struct.test02*
; CHECK:  call void @test02_a(%struct.test02* %p)
; CHECK:  call void @free(i8* %buf)
; CHECK:  %buf2 = call i8* @malloc(i32 8)
; CHECK:  %p2 = bitcast i8* %buf2 to %struct.test02.1*
; CHECK:  call void @test02_b(%struct.test02.1* %p2)
; CHECK:  call void @free(i8* %buf2)

define void @test03_a(%struct.test03.a* %p) {
  ret void
}
; CHECK-NOT: void @test03_a(%struct.test03.a* %p)

define void @test03_b(%struct.test03.a.789* %p) {
  ret void
}
; CHECK-NOT: void @test03_b(%struct.test03.a.789* %p)

define void @test03() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test03.a*
  call void @test03_a(%struct.test03.a* %p)
  call void bitcast (void (%struct.test03.a.789*)* @test03_b
              to void (%struct.test03.a*)*) (%struct.test03.a* %p)
  call void @free(i8* %buf)
  ret void
}

; CHECK-LABEL: void @test03()
; CHECK:  %buf = call i8* @malloc(i32 16)
; CHECK:  %p = bitcast i8* %buf to %__DTRT_struct.test03.a*
; CHECK:  call void @test03_a.4(%__DTRT_struct.test03.a* %p)
; CHECK:  call void @test03_b.5(%__DTRT_struct.test03.a* %p)
; CHECK:  call void @free(i8* %buf)

define void @test04_a(%struct.test04.a* %p) {
  ret void
}
; CHECK-LABEL: void @test04_a(%struct.test04.a* %p)

define void @test04_b(%struct.test04.a.987* %p) {
  ret void
}
; CHECK: void @test04_b(%struct.test04.a.987* %p)

define void @test04() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test04.a*
  call void @test04_a(%struct.test04.a* %p)
  call void @free(i8* %buf)

  %buf2 = call i8* @malloc(i32 16)
  %p2 = bitcast i8* %buf2 to %struct.test04.a.987*
  call void @test04_b(%struct.test04.a.987* %p2)
  call void @free(i8* %buf2)

  ret void
}

; CHECK-LABEL: void @test04()
; CHECK:  %buf = call i8* @malloc(i32 16)
; CHECK:  %p = bitcast i8* %buf to %struct.test04.a*
; CHECK:  call void @test04_a(%struct.test04.a* %p)
; CHECK:  call void @free(i8* %buf)
; CHECK:  %buf2 = call i8* @malloc(i32 16)
; CHECK:  %p2 = bitcast i8* %buf2 to %struct.test04.a.987*
; CHECK:  call void @test04_b(%struct.test04.a.987* %p2)
; CHECK:  call void @free(i8* %buf2)

define void @test05_a(%struct.test05.a* %p) {
  ret void
}
; CHECK-NOT: void @test05_a(%struct.test05.a* %p)

define void @test05_b(%struct.test05.a.11* %p) {
  ret void
}
; CHECK-NOT: void @test05_b(%struct.test05.a.11* %p)

define void @test05() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test05.a*
  call void @test05_a(%struct.test05.a* %p)
  call void bitcast (void (%struct.test05.a.11*)* @test05_b
              to void (%struct.test05.a*)*) (%struct.test05.a* %p)
  call void @free(i8* %buf)
  ret void
}

; CHECK-LABEL: void @test05()
; CHECK:  %buf = call i8* @malloc(i32 16)
; CHECK:  %p = bitcast i8* %buf to %__DTRT_struct.test05.a*
; CHECK:  call void @test05_a.6(%__DTRT_struct.test05.a* %p)
; CHECK:  call void @test05_b.7(%__DTRT_struct.test05.a* %p)
; CHECK:  call void @free(i8* %buf)

define void @test06_a(%struct.test06.a* %p) {
  %pp = getelementptr %struct.test06.a, %struct.test06.a* %p, i64 0, i32 1
  %p.b = load %struct.test06.b*, %struct.test06.b** %pp
  call void @test06_c(%struct.test06.b* %p.b)
  ret void
}
; CHECK-NOT: void @test06_a(%struct.test06.a* %p)

define void @test06_b(%struct.test06.a.33* %p) {
  %pp = getelementptr %struct.test06.a.33, %struct.test06.a.33* %p, i64 0, i32 1
  %p.b = load %struct.test06.b.44*, %struct.test06.b.44** %pp
  call void @test06_d(%struct.test06.b.44* %p.b)
  ret void
}
; CHECK-NOT: void @test06_b(%struct.test06.a.33* %p)

define void @test06_c(%struct.test06.b* %p) {
  ret void
}
; CHECK-NOT: void @test06_c(%struct.test06.b* %p)

define void @test06_d(%struct.test06.b.44* %p) {
  ret void
}
; CHECK-NOT: void @test06_d(%struct.test06.b.44* %p)

define void @test06() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test06.a*
  call void @test06_a(%struct.test06.a* %p)
  call void bitcast (void (%struct.test06.a.33*)* @test06_b
              to void (%struct.test06.a*)*) (%struct.test06.a* %p)
  call void @free(i8* %buf)
  ret void
}

; CHECK-LABEL: void @test06()
; CHECK:  %buf = call i8* @malloc(i32 16)
; CHECK:  %p = bitcast i8* %buf to %__DTRT_struct.test06.a*
; CHECK:  call void @test06_a.8(%__DTRT_struct.test06.a* %p)
; CHECK:  call void @test06_b.9(%__DTRT_struct.test06.a* %p)
; CHECK:  call void @free(i8* %buf)

define void @test07_a(%struct.test07.a* %p) {
  ret void
}
; CHECK-LABEL: void @test07_a(%struct.test07.a* %p)

define void @test07_b(%struct.test07.a.55* %p) {
  ret void
}
; CHECK: void @test07_b(%struct.test07.a.55* %p)

define void @test07() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test07.a*
  call void @test07_a(%struct.test07.a* %p)
  call void @free(i8* %buf)

  %buf2 = call i8* @malloc(i32 16)
  %p2 = bitcast i8* %buf2 to %struct.test07.a.55*
  call void @test07_b(%struct.test07.a.55* %p2)
  call void @free(i8* %buf2)

  ret void
}

; CHECK-LABEL: void @test07()
; CHECK:  %buf = call i8* @malloc(i32 16)
; CHECK:  %p = bitcast i8* %buf to %struct.test07.a*
; CHECK:  call void @test07_a(%struct.test07.a* %p)
; CHECK:  call void @free(i8* %buf)
; CHECK:  %buf2 = call i8* @malloc(i32 16)
; CHECK:  %p2 = bitcast i8* %buf2 to %struct.test07.a.55*
; CHECK:  call void @test07_b(%struct.test07.a.55* %p2)
; CHECK:  call void @free(i8* %buf2)

define void @test08_a(%struct.test08* %p) {
  ret void
}
; CHECK-LABEL: void @test08_a(%struct.test08* %p)

define void @test08_b(%struct.test08.77* %p) {
  ret void
}
; CHECK-NOT: void @test08_b(%struct.test08.77* %p)

define void @test08_c(%struct.test08.88* %p) {
  ret void
}
; CHECK-NOT: void @test08_c(%struct.test08.88* %p)

define void @test08() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test08*
  call void @test08_a(%struct.test08* %p)
  call void @free(i8* %buf)

  %buf2 = call i8* @malloc(i32 16)
  %p2 = bitcast i8* %buf2 to %struct.test08.77*
  call void @test08_b(%struct.test08.77* %p2)
  call void bitcast (void (%struct.test08.88*)* @test08_c
              to void (%struct.test08.77*)*) (%struct.test08.77* %p2)
  call void @free(i8* %buf2)
  ret void
}

; CHECK-LABEL: void @test08()
; CHECK:  %buf = call i8* @malloc(i32 16)
; CHECK:  %p = bitcast i8* %buf to %struct.test08*
; CHECK:  call void @test08_a(%struct.test08* %p)
; CHECK:  call void @free(i8* %buf)
; CHECK:  %buf2 = call i8* @malloc(i32 16)
; CHECK:  %p2 = bitcast i8* %buf2 to %__DTRT_struct.test08.77*
; CHECK:  call void @test08_b.12(%__DTRT_struct.test08.77* %p2)
; CHECK:  call void @test08_c.13(%__DTRT_struct.test08.77* %p2)
; CHECK:  call void @free(i8* %buf2)

define void @test09_a(%struct.test09* %p) {
  ret void
}
; CHECK-NOT: void @test09_a(%struct.test09* %p)

define void @test09_b(%struct.test09.1* %p) {
  ret void
}
; CHECK-NOT: void @test09_b(%struct.test09.1* %p)

define void @test09_c(%struct.test09.2.3* %p) {
  ret void
}
; CHECK-NOT: void @test09_c(%struct.test09.2.3* %p)

define void @test09() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test09*
  call void @test09_a(%struct.test09* %p)
  call void bitcast (void (%struct.test09.1*)* @test09_b
              to void (%struct.test09*)*) (%struct.test09* %p)
  call void bitcast (void (%struct.test09.2.3*)* @test09_c
              to void (%struct.test09*)*) (%struct.test09* %p)
  call void @free(i8* %buf)
  ret void
}

; CHECK-LABEL: void @test09()
; CHECK:  %buf = call i8* @malloc(i32 16)
; CHECK:  %p = bitcast i8* %buf to %__DTRT_struct.test09*
; CHECK:  call void @test09_a.14(%__DTRT_struct.test09* %p)
; CHECK:  call void @test09_b.15(%__DTRT_struct.test09* %p)
; CHECK:  call void @test09_c.16(%__DTRT_struct.test09* %p)
; CHECK:  call void @free(i8* %buf)

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  call void @test02()
  call void @test03()
  call void @test04()
  call void @test05()
  call void @test06()
  call void @test07()
  call void @test08()
  call void @test09()
  ret i32 0
}

declare i8* @malloc(i32)
declare void @free(i8*)

; Cloned functions get added at the end.

; CHECK-LABEL: void @test01_a.1(%__DTRT_struct.test01* %p)
; CHECK: void @test01_b.2(%__DTRT_struct.test01* %p)
; CHECK: void @test01_c.3(%__DTRT_struct.test01* %p)

; CHECK-LABEL: void @test03_a.4(%__DTRT_struct.test03.a* %p)
; CHECK: void @test03_b.5(%__DTRT_struct.test03.a* %p)

; CHECK-LABEL: void @test05_a.6(%__DTRT_struct.test05.a* %p)
; CHECK: void @test05_b.7(%__DTRT_struct.test05.a* %p)

; CHECK-LABEL: void @test06_a.8(%__DTRT_struct.test06.a* %p)
; CHECK:  %pp = getelementptr %__DTRT_struct.test06.a,
; CHECK-SAME:                 %__DTRT_struct.test06.a* %p, i64 0, i32 1
; CHECK:  %p.b = load %__DTRT_struct.test06.b*, %__DTRT_struct.test06.b** %pp
; CHECK:  call void @test06_c.10(%__DTRT_struct.test06.b* %p.b)
; CHECK: void @test06_b.9(%__DTRT_struct.test06.a* %p)
; CHECK:  %pp = getelementptr %__DTRT_struct.test06.a,
; CHECK-SAME:                 %__DTRT_struct.test06.a* %p, i64 0, i32 1
; CHECK:  %p.b = load %__DTRT_struct.test06.b*, %__DTRT_struct.test06.b** %pp
; CHECK:  call void @test06_d.11(%__DTRT_struct.test06.b* %p.b)
; CHECK: void @test06_c.10(%__DTRT_struct.test06.b* %p)
; CHECK: void @test06_d.11(%__DTRT_struct.test06.b* %p)

; CHECK-LABEL: void @test08_b.12(%__DTRT_struct.test08.77* %p)
; CHECK: void @test08_c.13(%__DTRT_struct.test08.77* %p)

; CHECK-LABEL: void @test09_a.14(%__DTRT_struct.test09* %p)
; CHECK: void @test09_b.15(%__DTRT_struct.test09* %p)
; CHECK: void @test09_c.16(%__DTRT_struct.test09* %p)
