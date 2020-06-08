; ModuleID = 'mini.c'
source_filename = "mini.c"

define i32 @foo(i32, i32) {
entry:
  br label %cond

cond:                                             ; preds = %body, %entry
  br i1 true, label %body, label %after

body:                                             ; preds = %cond
  br label %cond

after:                                            ; preds = %cond
  ret i64 0
}
