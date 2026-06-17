//command table 타입/enum 선언
#ifndef COMMAND_H
#define COMMAND_H

#include "student.h"

// 명령어 처리 결과 코드 
typedef enum {
    SHELL_OK = 0,
    SHELL_EXIT,
    SHELL_ERR_UNKNOWN_COMMAND,
    SHELL_ERR_INVALID_ARGUMENT,
    SHELL_ERR_MISSING_ARGUMENT,
    SHELL_ERR_FILE_OPEN,
    SHELL_ERR_FILE_WRITE,
    SHELL_ERR_STUDENT_NOT_FOUND,
    SHELL_ERR_DUPLICATE_STUDENT,
    SHELL_ERR_INVALID_SCORE
} ShellResult;

// 명령어 핸들러 함수 포인터 타입
typedef ShellResult (*CommandHandler)(char* args, Student** head);

// command table 엔트리
typedef struct {
    const char* name;
    CommandHandler handler;
    const char* usage;
    const char* description;
} Command;

// 원본 CSV 경로 (save/reload 시 사용, main.c 에서 초기화)
extern char g_csv_path[256];

// 한 줄을 파싱해 해당 명령어 핸들러를 실행
ShellResult command_dispatch(char* line, Student** head);

// ShellResult 에 대응하는 에러 메시지 문자열을 반환
const char* error_message(ShellResult result);

#endif /* COMMAND_H */