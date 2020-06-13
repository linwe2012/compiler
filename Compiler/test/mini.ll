; ModuleID = 'mini.c'
source_filename = "mini.c"

declare i32 @putchar(i32)

define void @test() {
entry:
  %a = alloca [10 x i32]
  %gep_res = getelementptr [10 x i32], [10 x i32]* %a, i32 1, i32 0
  %arr_res = load i32, i32* %gep_res
  %gep_res1 = getelementptr [10 x i32], [10 x i32]* %a, i32 1, i32 0
  store i32 10, i32* %gep_res1
  ret void
}
