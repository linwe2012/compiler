; ModuleID = 'mini.c'
source_filename = "mini.c"

define i32 @foo(i32, i32) {
entry:
  br label %cond

cond:                                             ; preds = %entry
  ret void

body:                                             ; No predecessors!

step:                                             ; No predecessors!

after:                                            ; No predecessors!
}
