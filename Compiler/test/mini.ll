; ModuleID = 'mini.c'
source_filename = "mini.c"

declare i32 @putchar(i32)

declare i32 @foo(i32)

define i32 @main() {
entry:
  %h = alloca i32
  store i32 104, i32* %h
  %o = alloca i32
  store i32 111, i32* %o
  %l = alloca i32
  store i32 108, i32* %l
  %a = alloca i32
  store i32 97, i32* %a
  %"0" = load i32, i32* %h
  %"1" = call i32 @putchar(i32 %"0")
  %"2" = load i32, i32* %o
  %"3" = call i32 @putchar(i32 %"2")
  %"4" = load i32, i32* %l
  %"5" = call i32 @putchar(i32 %"4")
  %"6" = load i32, i32* %a
  %"7" = call i32 @putchar(i32 %"6")
  %"8" = call i32 @putchar(i32 10)
  ret i32 0
}
