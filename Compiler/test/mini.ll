; ModuleID = 'mini.c'
source_filename = "mini.c"

%"struct Demo" = type { i32, i8*, i8, %"struct K", %"struct Demo"* }
%"struct K" = type { i32, double }
%"struct A" = type { i32, double }

@a = external global i32
@"2" = private unnamed_addr constant [16 x i8] c"\22\5CnHello world\22\00"
@"4" = private unnamed_addr constant [48 x i8] c"\22My age is %d\5Cn, Im %f m, and my hobby is %d\5Cn\22\00"
@"5" = private unnamed_addr constant [10 x i8] c"\22Running\22\00"
@g_a = global i32 10

declare i32 @putchar(i32)

declare x86_stdcallcc i32 @SetConsoleTextAttribute(i8*, i16)

declare i32 @printf(i8*, ...)

define i32 @gcd(i32 %a, i32 %b) {
entry:
  %"0" = alloca i32
  store i32 %a, i32* %"0"
  %"1" = alloca i32
  store i32 %b, i32* %"1"
  %a1 = load i32, i32* %"0"
  %b2 = load i32, i32* %"1"
  %mod_res = srem i32 %a1, %b2
  %equal_res = icmp eq i32 %mod_res, 0
  %"2" = icmp ne i1 %equal_res, false
  br i1 %"2", label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %b3 = load i32, i32* %"1"
  ret i32 %b3

if.else:                                          ; preds = %entry
  br label %if.after

if.after:                                         ; preds = %if.else
  %b4 = load i32, i32* %"1"
  %a5 = load i32, i32* %"0"
  %b6 = load i32, i32* %"1"
  %mod_res7 = srem i32 %a5, %b6
  %"3" = call i32 @gcd(i32 %b4, i32 %mod_res7)
  ret i32 %"3"
}

define void @disp_num(i32 %n) {
entry:
  %"0" = alloca i32
  store i32 %n, i32* %"0"
  %n1 = load i32, i32* %"0"
  %less_res = icmp slt i32 %n1, 0
  %"1" = icmp ne i1 %less_res, false
  br i1 %"1", label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %n2 = load i32, i32* %"0"
  %n3 = load i32, i32* %"0"
  %neg_res = sub i32 0, %n3
  store i32 %neg_res, i32* %"0"
  %"2" = call i32 @putchar(i32 45)
  br label %if.after

if.then5:                                         ; preds = %if.else
  ret void

if.else6:                                         ; preds = %if.else
  br label %if.after7

if.after7:                                        ; preds = %if.else6
  br label %if.after

if.else:                                          ; preds = %entry
  %n4 = load i32, i32* %"0"
  %equal_res = icmp eq i32 %n4, 0
  %"3" = icmp ne i1 %equal_res, false
  br i1 %"3", label %if.then5, label %if.else6

if.after:                                         ; preds = %if.after7, %if.then
  %n8 = load i32, i32* %"0"
  %div_res = sdiv exact i32 %n8, 10
  call void @disp_num(i32 %div_res)
  %n9 = load i32, i32* %"0"
  %mod_res = srem i32 %n9, 10
  %add_res = add i32 %mod_res, 48
  %"4" = call i32 @putchar(i32 %add_res)
  ret void
}

define i32 @main() {
entry:
  %demo_b = alloca %"struct Demo"
  %st_A = alloca %"struct A"
  %nb = alloca i32
  store i32 16, i32* %nb
  %ss1 = alloca i32
  store i32 101, i32* %ss1
  %"0" = call i32 @gcd(i32 36, i32 60)
  call void @disp_num(i32 %"0")
  %"1" = call i32 @putchar(i32 10)
  %"3" = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @"2", i32 0, i32 0))
  %"6" = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([48 x i8], [48 x i8]* @"4", i32 0, i32 0), i32 22, float 0x3FFCCCCCC0000000, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @"5", i32 0, i32 0))
  ret i32 0
}

define void @test() {
entry:
  %g_a = load i32, i32* @g_a
  store i32 100, i32* @g_a
  %a = alloca [10 x i32]
  %arr = alloca [10 x [2 x i32]]
  %gep_res = getelementptr [10 x i32], [10 x i32]* %a, i64 0, i32 1
  %arr_res = load i32, i32* %gep_res
  %gep_res1 = getelementptr [10 x [2 x i32]], [10 x [2 x i32]]* %arr, i64 0, i32 2
  %gep_res2 = getelementptr [2 x i32], [2 x i32]* %gep_res1, i64 0, i32 1
  %arr_res3 = load i32, i32* %gep_res2
  %gep_res4 = getelementptr [10 x i32], [10 x i32]* %a, i64 0, i32 1
  store i32 %arr_res3, i32* %gep_res4
  %gep_res5 = getelementptr [10 x [2 x i32]], [10 x [2 x i32]]* %arr, i64 0, i32 3
  %gep_res6 = getelementptr [2 x i32], [2 x i32]* %gep_res5, i64 0, i32 1
  %arr_res7 = load i32, i32* %gep_res6
  %gep_res8 = getelementptr [10 x i32], [10 x i32]* %a, i64 0, i32 2
  %arr_res9 = load i32, i32* %gep_res8
  %gep_res10 = getelementptr [10 x [2 x i32]], [10 x [2 x i32]]* %arr, i64 0, i32 3
  %gep_res11 = getelementptr [2 x i32], [2 x i32]* %gep_res10, i64 0, i32 1
  store i32 %arr_res9, i32* %gep_res11
  %d = alloca [3 x [10 x double]]
  %na = alloca i32
  %a12 = load [10 x i32], [10 x i32]* %a
  store i32 trunc (i64 mul nuw (i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64), i64 10) to i32), i32* %na
  %nd = alloca i32
  %d13 = load [3 x [10 x double]], [3 x [10 x double]]* %d
  store i32 trunc (i64 mul (i64 ptrtoint (double* getelementptr (double, double* null, i32 1) to i64), i64 30) to i32), i32* %nd
  ret void
}
