; ModuleID = 'mini.c'
source_filename = "mini.c"

define i32 @foo(i32, i32) {
entry:
  br label %cond

cond:                                             ; preds = %ifcont, %entry
  br i1 true, label %body, label %after

body:                                             ; preds = %cond
  br i1 true, label %then, label %else

after:                                            ; preds = %cond
  ret i32 0

then:                                             ; preds = %body
  ret i32 2
  br label %ifcont

else:                                             ; preds = %body
  ret i32 3
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  br label %cond
}
