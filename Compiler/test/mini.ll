; ModuleID = 'mini.c'
source_filename = "mini.c"

define i32 @foo(i32, i32) {
entry:
  br label %"0"

"0":                                              ; preds = %"6", %entry
  br i1 true, label %"1", label %"2"

"1":                                              ; preds = %"0"
  br i1 true, label %"4", label %"5"

"4":                                              ; preds = %"1"
  ret i32 2
  br label %"6"

"5":                                              ; preds = %"1"
  ret i32 3
  br label %"6"

"6":                                              ; preds = %"5", %"4"
  br label %"0"

"2":                                              ; preds = %"0"
  ret i32 0
}
