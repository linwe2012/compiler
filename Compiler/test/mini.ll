; ModuleID = 'mini.c'
source_filename = "mini.c"

declare i32 @putchar(i32)

define i32 @gcd(i32 %a, i32 %b) {
entry:
  %"0" = alloca i32
  store i32 %a, i32* %"0"
  %"1" = alloca i32
  store i32 %b, i32* %"1"
  %load_val = load i32, i32* %"0"
  %load_val1 = load i32, i32* %"1"
  %mod_res = srem i32 %load_val, %load_val1
  %equal_res = icmp eq i32 %mod_res, 0
  %"2" = icmp ne i1 %equal_res, false
  br i1 %"2", label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %"3" = load i32, i32* %"1"
  ret i32 %"3"

if.else:                                          ; preds = %entry
  br label %if.after

if.after:                                         ; preds = %if.else
  %"4" = load i32, i32* %"1"
  %load_val2 = load i32, i32* %"0"
  %load_val3 = load i32, i32* %"1"
  %mod_res4 = srem i32 %load_val2, %load_val3
  %"5" = call i32 @gcd(i32 %"4", i32 %mod_res4)
  ret i32 %"5"
}

define void @disp_num(i32 %n) {
entry:
  %"0" = alloca i32
  store i32 %n, i32* %"0"
  %load_val = load i32, i32* %"0"
  %less_res = icmp slt i32 %load_val, 0
  %"1" = icmp ne i1 %less_res, false
  br i1 %"1", label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %load_val1 = load i32, i32* %"0"
  %load_val2 = load i32, i32* %"0"
  %neg_res = sub i32 0, %load_val2
  store i32 %neg_res, i32* %"0"
  %"2" = call i32 @putchar(i32 45)
  br label %if.after

if.then4:                                         ; preds = %if.else
  ret void

if.else5:                                         ; preds = %if.else
  br label %if.after6

if.after6:                                        ; preds = %if.else5
  br label %if.after

if.else:                                          ; preds = %entry
  %load_val3 = load i32, i32* %"0"
  %equal_res = icmp eq i32 %load_val3, 0
  %"3" = icmp ne i1 %equal_res, false
  br i1 %"3", label %if.then4, label %if.else5

if.after:                                         ; preds = %if.after6, %if.then
  %load_val7 = load i32, i32* %"0"
  %div_res = sdiv exact i32 %load_val7, 10
  call void @disp_num(i32 %div_res)
  %load_val8 = load i32, i32* %"0"
  %mod_res = srem i32 %load_val8, 10
  %add_res = add i32 %mod_res, 48
  %"4" = call i32 @putchar(i32 %add_res)
  ret void
}

define i32 @main() {
entry:
  %"0" = call i32 @gcd(i32 36, i32 60)
  call void @disp_num(i32 %"0")
  %"1" = call i32 @putchar(i32 10)
  ret i32 0
}
