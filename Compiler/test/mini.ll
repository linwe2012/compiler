; ModuleID = 'mini.c'
source_filename = "mini.c"

define i32 @foo(i32 %"0") {
entry:
  br label %"1"

"1":                                              ; preds = %"7", %entry
  br i1 true, label %"2", label %"3"

"2":                                              ; preds = %"1"
  br i1 true, label %"5", label %"6"

"5":                                              ; preds = %"2"
  ret i32 2

"6":                                              ; preds = %"2"
  ret i32 3

"7":                                              ; No predecessors!
  br label %"1"

"3":                                              ; preds = %"1"
  ret i32 0
}

declare void @bar(i32, i32, i32)
