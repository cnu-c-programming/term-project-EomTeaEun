/*
 * main.c  –  Mini Student Shell
 *
 * TODO: Implement admin_shell and client_shell.
 *
 * Build:
 *   make admin   →  admin_shell  (compiled with -DADMIN_MODE)
 *   make client  →  client_shell (compiled with -DCLIENT_MODE)
 *
 * Usage:
 *   ./admin_shell [students.csv]
 *   ./admin_shell -f commands.txt [students.csv]
 *   ./client_shell [students.csv]
 *   ./client_shell -f commands.txt [students.csv]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* TODO: Add your own header includes here */
#include "student.h"
#include "file_io.h"
#include "command.h"

#ifdef ADMIN_MODE
#define PROGRAM_TITLE "[Admin Program]"
#define PROMPT "admin> "
#define EXEC_NAME "./admin_shell"
#else
#define PROGRAM_TITLE "[Client Program]"
#define PROMPT "client> "
#define EXEC_NAME "./client_shell"
#endif

#define LINE_BUF 256

// 입력 줄 끝의 개행 문자를 제거
static void chomp(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[--len] = '\0';
    }
}

// 에러 결과를 출력. 명령어 파일 모드면 "Skipped line N." 덧붙임.
static void report_result(ShellResult result, int from_file, int line_no) {
    if (result == SHELL_OK || result == SHELL_EXIT) {
        return;
    }
    if (from_file) {
        printf("%s Skipped line %d.\n", error_message(result), line_no);
    } else {
        printf("%s\n", error_message(result));
    }
}

// CSV 파일을 읽어 linked list 를 구성하고 시작 메시지 출력
// 성공 시 0, 실패 시 -1 을 반환
static int load_and_announce(const char *csv_path, Student **head) {
    int loaded = file_load(csv_path, head);
    if (loaded == -1) {
        printf("Error: cannot open file %s.\n", csv_path);
        return -1;
    }
    if (loaded == -2) {
        printf("Error: invalid CSV header in %s.\n", csv_path);
        return -1;
    }
    // save/reload 핸들러가 사용할 원본 CSV 경로를 저장
    strncpy(g_csv_path, csv_path, sizeof(g_csv_path) - 1);
    g_csv_path[sizeof(g_csv_path) - 1] = '\0';

    printf("%s\n", PROGRAM_TITLE);
    printf("Loaded %d students from %s.\n", loaded, csv_path);
    return 0;
}


/* ---------------------------------------------------------------
 * TODO: Implement the interactive shell loop.
 *   - Print a prompt and read a line from stdin.
 *   - Parse the line into a command and arguments.
 *   - Dispatch to the appropriate handler function.
 *   - Loop until the user types "exit" or EOF.
 * --------------------------------------------------------------- */
void run_shell(const char *csv_path) {
    Student *head = NULL;
    if (load_and_announce(csv_path, &head) != 0) {
        return;
    }

    char line[LINE_BUF];
    for (;;) {
        printf("%s", PROMPT);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break; 
        }
        chomp(line);
        ShellResult result = command_dispatch(line, &head);
        if (result == SHELL_EXIT) {
            break;
        }
        report_result(result, 0, 0);
    }

    student_free_all(&head); // 종료 전 동적 메모리 모두 해제 
}


/* ---------------------------------------------------------------
 * TODO: Implement batch mode – read commands from a file.
 *   - Open cmd_file for reading.
 *   - Execute each line as a command (same logic as run_shell).
 *   - Close the file when done.
 * --------------------------------------------------------------- */
void run_command_file(const char *cmd_file, const char *csv_path) {
    Student *head = NULL;
    if (load_and_announce(csv_path, &head) != 0) {
        return;
    }

    FILE *fp = fopen(cmd_file, "r");
    if (fp == NULL) {
        printf("Error: cannot open command file %s.\n", cmd_file);
        student_free_all(&head);
        return;
    }

    char line[LINE_BUF];
    int line_no = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_no++;
        chomp(line);
        char *p = line;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '\0' || *p == '#') {
            continue; // 빈 줄과 # 주석은 건너뜀 
        }
        printf("[command file:%d] %s\n", line_no, p);
        ShellResult result = command_dispatch(p, &head);
        if (result == SHELL_EXIT) {
            break;
        }
        report_result(result, 1, line_no); // 오류 시 "Skipped line N."
    }

    fclose(fp);
    student_free_all(&head); //종료 전 동적 메모리 해제 
}


int main(int argc, char *argv[]) {
    const char *csv_path  = NULL; 
    const char *cmd_file  = NULL;           /* -f <file> argument */

    /* TODO: Parse command-line arguments.
     *   Supported flags:
     *     -f <file>   run commands from <file> instead of stdin
     *   Remaining positional argument (if any): path to students CSV.
     *
     *   Example parsing skeleton:
     *
     *   for (int i = 1; i < argc; i++) {
     *       if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
     *           cmd_file = argv[++i];
     *       } else {
     *           csv_path = argv[i];
     *       }
     *   }
     */
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            cmd_file = argv[++i];
        } else {
            csv_path = argv[i];
        }
    }

    if (csv_path == NULL) {
        printf("Usage: %s <csv_file> [-f command_file]\n", EXEC_NAME);
        return 1;
    }

#ifdef ADMIN_MODE
    /* Admin shell: supports add, delete, update, save, load, sort, list, find, help, exit */
    if (cmd_file) {
        run_command_file(cmd_file, csv_path);
    } else {
        run_shell(csv_path);
    }

#elif defined(CLIENT_MODE)
    /* Client shell: supports find, list, help, exit  (read-only) */
    if (cmd_file) {
        run_command_file(cmd_file, csv_path);
    } else {
        run_shell(csv_path);
    }

#else
#error "Define either -DADMIN_MODE or -DCLIENT_MODE when compiling."
#endif

    return 0;
}
