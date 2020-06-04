; ModuleID = 'mini'
source_filename = "mini"

define void @foo(i32) {
entry:
  br i1 true, label %then, label %else

then:                                             ; preds = %entry
  br label %ifcont

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  ret void
}
