; ModuleID = 'ref.c'
source_filename = "ref.c"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%struct.Ch = type { i32 }
%struct.A = type { i32, i8, double, %struct.D, %union.U, %struct.A* }
%struct.D = type { i32, %struct.E }
%struct.E = type { i32 }
%union.U = type { double, [8 x i8] }

@__const.main.ch = private unnamed_addr constant %struct.Ch { i32 10 }, align 4

; Function Attrs: noinline nounwind optnone
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %ch = alloca %struct.Ch, align 4
  %A_ins = alloca %struct.A, align 8
  %u = alloca i32, align 4
  %x = alloca i32, align 4
  %ck = alloca i32, align 4
  %arr = alloca [10 x i8], align 1
  %a = alloca i32, align 4
  %c = alloca [10 x [8 x i32]], align 16
  store i32 0, i32* %retval, align 4
  %0 = bitcast %struct.Ch* %ch to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %0, i8* align 4 bitcast (%struct.Ch* @__const.main.ch to i8*), i64 4, i1 false)
  %d = getelementptr inbounds %struct.A, %struct.A* %A_ins, i32 0, i32 3
  %e = getelementptr inbounds %struct.D, %struct.D* %d, i32 0, i32 1
  %k = getelementptr inbounds %struct.E, %struct.E* %e, i32 0, i32 0
  store i32 10, i32* %k, align 4
  %d1 = getelementptr inbounds %struct.A, %struct.A* %A_ins, i32 0, i32 3
  %e2 = getelementptr inbounds %struct.D, %struct.D* %d1, i32 0, i32 1
  %k3 = getelementptr inbounds %struct.E, %struct.E* %e2, i32 0, i32 0
  %1 = load i32, i32* %k3, align 4
  store i32 %1, i32* %u, align 4
  store i32 -10, i32* %x, align 4
  %2 = load i32, i32* %x, align 4
  %or = or i32 %2, 2
  store i32 %or, i32* %ck, align 4
  %3 = bitcast [10 x i8]* %arr to i8*
  call void @llvm.memset.p0i8.i64(i8* align 1 %3, i8 0, i64 10, i1 false)
  %arraydecay = getelementptr inbounds [10 x i8], [10 x i8]* %arr, i64 0, i64 0
  %add.ptr = getelementptr inbounds i8, i8* %arraydecay, i64 8
  %4 = bitcast i8* %add.ptr to i32*
  store i32 10, i32* %4, align 4
  call void @putchar(i32 104)
  call void @putchar(i32 111)
  call void @putchar(i32 108)
  call void @putchar(i32 97)
  call void @putchar(i32 10)
  call void @dummy(i32 1, float 0x3FB99999A0000000)
  %coerce.dive = getelementptr inbounds %struct.Ch, %struct.Ch* %ch, i32 0, i32 0
  %5 = load i32, i32* %coerce.dive, align 4
  call void @dummy2(i32 8, i32 %5, double 8.000000e-01)
  store i32 10, i32* %a, align 4
  %6 = load i32, i32* %ck, align 4
  %cmp = icmp eq i32 %6, 1
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %7 = load i32, i32* %ck, align 4
  %cmp4 = icmp eq i32 %7, 2
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %8 = load i32, i32* %a, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32* %a, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %entry
  %9 = load i32, i32* %ck, align 4
  %cmp5 = icmp eq i32 %9, 1
  br i1 %cmp5, label %if.then7, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %if.end
  %10 = load i32, i32* %ck, align 4
  %cmp6 = icmp eq i32 %10, 2
  br i1 %cmp6, label %if.then7, label %if.end9

if.then7:                                         ; preds = %lor.lhs.false, %if.end
  %11 = load i32, i32* %a, align 4
  %inc8 = add nsw i32 %11, 1
  store i32 %inc8, i32* %a, align 4
  br label %if.end9

if.end9:                                          ; preds = %if.then7, %lor.lhs.false
  %12 = bitcast [10 x [8 x i32]]* %c to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %12, i8 0, i64 320, i1 false)
  %arrayinit.begin = getelementptr inbounds [10 x [8 x i32]], [10 x [8 x i32]]* %c, i64 0, i64 0
  %arrayinit.begin10 = getelementptr inbounds [8 x i32], [8 x i32]* %arrayinit.begin, i64 0, i64 0
  store i32 2, i32* %arrayinit.begin10, align 4
  %arrayinit.element = getelementptr inbounds i32, i32* %arrayinit.begin10, i64 1
  %13 = load i32, i32* %a, align 4
  store i32 %13, i32* %arrayinit.element, align 4
  ret i32 0
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

declare dso_local void @putchar(i32) #2

declare dso_local void @dummy(i32, float) #2

declare dso_local void @dummy2(i32, i32, double) #2

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cx8,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cx8,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{!"clang version 10.0.0 "}
