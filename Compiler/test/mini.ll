; ModuleID = 'mini.c'
source_filename = "mini.c"

declare i32 @putchar(i32)

define i32 @foo(i32 %"0") {
entry:
  %"1" = alloca i32
  store i32 %"0", i32* %"1"
  br label %while.cond

while.cond:                                       ; preds = %if.after, %entry
  br i1 true, label %while.body, label %while.after

while.body:                                       ; preds = %while.cond
  br i1 true, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %"4" = call i32 @bar(i32 1, i32 2, i32 3)
  ret i32 %"4"

if.else:                                          ; preds = %while.body
  %a = alloca i32
  store i32 10, i32* %a
  %b = alloca float
  store float 1.000000e+00, float* %b
  %load_val = load i32, i32* %a
  %load_val1 = load float, float* %b
  %mul_res = fmul float %load_val1, 3.000000e+00
  %add_res = fadd float 1.000000e+00, %mul_res
  %int32_cast = fptosi float %add_res to i32
  store i32 %int32_cast, i32* %a
  ret i32 0

if.after:                                         ; No predecessors!
  br label %while.cond

while.after:                                      ; preds = %while.cond
  ret i32 0
}

declare i32 @bar(i32, i32, i32)

declare float @test(float)
