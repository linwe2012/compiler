; ModuleID = 'mini.c'
source_filename = "mini.c"

declare i32 @putchar(i32)

declare i32 @foo(i32)

define i32 @main(i32 %"0", i8** %"2") {
entry:
  %"1" = alloca i32
  store i32 %"0", i32* %"1"
  %"3" = alloca i8**
  store i8** %"2", i8*** %"3"
  %h = alloca i32
  store i32 104, i32* %h
  %o = alloca i32
  store i32 111, i32* %o
  %l = alloca i32
  store i32 108, i32* %l
  %a = alloca i32
  store i32 97, i32* %a
  %"4" = load i32, i32* %h
  %"5" = call i32 @putchar(i32 %"4")
  %"6" = load i32, i32* %o
  %"7" = call i32 @putchar(i32 %"6")
  %"8" = load i32, i32* %l
  %"9" = call i32 @putchar(i32 %"8")
  %"10" = load i32, i32* %a
  %"11" = call i32 @putchar(i32 %"10")
  %"12" = call i32 @putchar(i32 10)
  ret i32 0
}
