; When inlining a function that contains an alloca outside the first block,
; the inliner normally inserts a stacksave/stackrestore to simulate the
; original callee function's stack allocation behavior. This is incorrect
; on Intel GPU targets because there is no traditional stack.
; RUN: opt -inline -S -o - %s | FileCheck %s
; CHECK-NOT: stacksave
; CHECK-NOT: stackrestore

target triple = "spir64"
target device_triples = "spir64"

$_Z6calleev = comdat any

; Function Attrs: uwtable
define dso_local i32 @_Z6callerv() #0 {
entry:
  %call = call i32 @_Z6calleev()
  ret i32 %call
}

; Function Attrs: alwaysinline nounwind uwtable
define linkonce_odr dso_local i32 @_Z6calleev() #1 comdat {
entry:
  %j = alloca i32, align 4
  %0 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 4, i32* %j, align 4
  %1 = load i32, i32* %j, align 4
  %tobool = icmp ne i32 %1, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %2 = load i32, i32* %j, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* %j, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %3 = load i32, i32* %j, align 4
  %dec = add nsw i32 %3, -1
  store i32 %dec, i32* %j, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %k = alloca i32, align 4
  %4 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #3
  store i32 5, i32* %k, align 4
  %5 = load i32, i32* %j, align 4
  %6 = load i32, i32* %k, align 4
  %add = add nsw i32 %5, %6
  %7 = bitcast i32* %k to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7) #3
  %8 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %8) #3
  ret i32 %add
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

