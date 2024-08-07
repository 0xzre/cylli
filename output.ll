; ModuleID = 'CylliLLVM'
source_filename = "CylliLLVM"

@0 = private unnamed_addr constant [25 x i8] c"I like number %d!! siu \0A\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @0, i32 0, i32 0), i32 7)
  ret i32 0
}
