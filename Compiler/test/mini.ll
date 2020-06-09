; ModuleID = 'mini.c'
source_filename = "mini.c"

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
  ret i32 3

if.after:                                         ; No predecessors!
  br label %while.cond

while.after:                                      ; preds = %while.cond
  ret i32 0
}

declare i32 @bar(i32, i32, i32)
