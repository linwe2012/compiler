; ModuleID = 'mini.c'
source_filename = "mini.c"

declare i32 @putchar(i32)

define i32 @foo(i32 %"0") {
entry:
  %"1" = alloca i32
  store i32 %"0", i32* %"1"
  %y = alloca float
  %load_val = load float, float* %y
  store float 1.000000e+00, float* %y
  %a = alloca i32
  store i32 10, i32* %a
  br label %while.cond

while.cond:                                       ; preds = %if.after, %entry
  br i1 true, label %while.body, label %while.after

while.body:                                       ; preds = %while.cond
  br i1 true, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %"4" = call i32 @bar(i32 1, i32 2, i32 3)
  ret i32 %"4"

if.else:                                          ; preds = %while.body
  %load_val1 = load i32, i32* %a
  %load_val2 = load float, float* %y
  %less_res = fcmp olt float %load_val2, 2.000000e+00
  %int32_cast = zext i1 %less_res to i32
  store i32 %int32_cast, i32* %a
  ret i32 0

if.after:                                         ; No predecessors!
  br label %while.cond

while.after:                                      ; preds = %while.cond
  ret i32 0
}

declare i32 @bar(i32, i32, i32)

declare float @test(float)
