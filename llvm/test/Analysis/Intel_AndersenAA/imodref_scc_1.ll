; RUN: opt < %s -passes='require<anders-aa>' -disable-output 2>/dev/null


; Test the processing of the SCC propagation when there are multiple
; functions involved in the SCC and one of them makes an indirect
; function call. (based on CQ414941). Test passes if the mod/ref pass
; does not crash.

; The source used to generate this IR was the following
;
; /*
;     Create a SCC containing C, D, B, and A where one of
;     these contains an indirect call, resulting in updates
;     the to ModRef info for all routines in the set.
; */
; extern int A(int);
; extern int B(int);
; extern int C(int);
; extern int D(int);
;
; extern int ((*pD)(void));
;
; int A(int x)
; {
;   return B(x);
; }
;
; int B(int x)
; {
;   return D(--x) + pD();
; }
;
; int C(int x)
; {
;   return A(x);
; }
;
; int D(int x)
; {
;    if (x > 0) {
;       return A(--x);
;    }
;    else {
;        return C(--x);
;    }
; }
;
; int test(int x)
; {
;    return A(5);
; }

@pD = external global i32 ()*, align 8

; Function Attrs: noinline nounwind uwtable
define i32 @A(i32 %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %call = call i32 @B(i32 %0)
  ret i32 %call
}

; Function Attrs: noinline nounwind uwtable
define i32 @B(i32 %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %dec = add nsw i32 %0, -1
  store i32 %dec, i32* %x.addr, align 4
  %call = call i32 @D(i32 %dec)
  %1 = load i32 ()*, i32 ()** @pD, align 8
  %call1 = call i32 %1()
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

; Function Attrs: noinline nounwind uwtable
define i32 @D(i32 %x) #0 {
entry:
  %retval = alloca i32, align 4
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %1 = load i32, i32* %x.addr, align 4
  %dec = add nsw i32 %1, -1
  store i32 %dec, i32* %x.addr, align 4
  %call = call i32 @A(i32 %dec)
  store i32 %call, i32* %retval, align 4
  br label %return

if.else:                                          ; preds = %entry
  %2 = load i32, i32* %x.addr, align 4
  %dec1 = add nsw i32 %2, -1
  store i32 %dec1, i32* %x.addr, align 4
  %call2 = call i32 @C(i32 %dec1)
  store i32 %call2, i32* %retval, align 4
  br label %return

return:                                           ; preds = %if.else, %if.then
  %3 = load i32, i32* %retval, align 4
  ret i32 %3
}

; Function Attrs: noinline nounwind uwtable
define i32 @C(i32 %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %call = call i32 @A(i32 %0)
  ret i32 %call
}

; Function Attrs: noinline nounwind uwtable
define i32 @test(i32 %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %call = call i32 @A(i32 5)
  ret i32 %call
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

