//조회 핸들러
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "file_io.h"

// 원본 CSV 경로와 미저장 변경 플래그 + exit 시 경고
char g_csv_path[256] = {0};
static int g_dirty = 0;

//내부 util
// 공백/탭 기준으로 토큰을 분리+ 토큰 개수 반환
static int tokenize(char* s, char* tokens[], int max_tokens) {
    int n = 0;
    char* p = strtok(s, " \t");
    while (p != NULL && n < max_tokens) {
        tokens[n++] = p;
        p = strtok(NULL, " \t");
    }
    return n;
}

// 문자열을 정수로 변환
static int parse_int(const char* s, long* out) {
    char* end;
    long value = strtol(s, &end, 10);
    if (end == s || *end != '\0') {
        return -1;
    }
    *out = value;
    return 0;
}

//조회 명령어 (Admin / Client 공통) 

static ShellResult handle_find(char* args, Student** head) {
    char* tokens[4];
    int n = tokenize(args, tokens, 4);
    if (n < 1) {
        return SHELL_ERR_MISSING_ARGUMENT;
    }
    long id;
    if (parse_int(tokens[0], &id) != 0) {
        return SHELL_ERR_INVALID_ARGUMENT;
    }
    Student* s = student_find(*head, (int)id);
    if (s == NULL) {
        return SHELL_ERR_STUDENT_NOT_FOUND;
    }
    printf("ID: %d\n", s->id);
    printf("Name: %s\n", s->name);
    printf("Score: %d\n", s->score);
    return SHELL_OK;
}

static ShellResult handle_list(char* args, Student** head) {
    (void)args;
    if (*head == NULL) {
        printf("No students found.\n");
        return SHELL_OK;
    }
    printf("%-6s%-12s%s\n", "ID", "Name", "Score");
    for (Student* cur = *head; cur != NULL; cur = cur->next) {
        printf("%-6d%-12s%d\n", cur->id, cur->name, cur->score);
    }
    return SHELL_OK;
}

static ShellResult handle_stats(char* args, Student** head) {
    (void)args;
    if (*head == NULL) {
        printf("No student data available.\n");
        return SHELL_OK;
    }
    int count = 0;
    long sum = 0;
    int max = (*head)->score;
    int min = (*head)->score;
    for (Student* cur = *head; cur != NULL; cur = cur->next) {
        count++;
        sum += cur->score;
        if (cur->score > max) max = cur->score;
        if (cur->score < min) min = cur->score;
    }
    printf("Count: %d\n", count);
    printf("Average: %.1f\n", (double)sum / count);
    printf("Max: %d\n", max);
    printf("Min: %d\n", min);
    return SHELL_OK;
}

static ShellResult handle_reload(char* args, Student** head) {
    (void)args;
    Student* fresh = NULL;
    int count = file_load(g_csv_path, &fresh);
    if (count < 0) {
        return SHELL_ERR_FILE_OPEN;
    }
    student_free_all(head);
    *head = fresh;
    g_dirty = 0;
    printf("Reloaded %d students from %s.\n", count, g_csv_path);
    return SHELL_OK;
}

//Admin 전용 수정 명령어
#ifdef ADMIN_MODE

static ShellResult handle_save(char* args, Student** head) {
    (void)args;
    int count = file_save(g_csv_path, *head);
    if (count < 0) {
        return SHELL_ERR_FILE_WRITE;
    }
    g_dirty = 0;
    printf("Saved %d students to %s.\n", count, g_csv_path);
    return SHELL_OK;
}

static ShellResult handle_add(char* args, Student** head) {
    char* tokens[8];
    int n = tokenize(args, tokens, 8);
    if (n < 3) {
        return SHELL_ERR_MISSING_ARGUMENT; // 인자 부족 
    }
    long id, score;
    if (parse_int(tokens[0], &id) != 0 || id <= 0) {
        return SHELL_ERR_INVALID_ARGUMENT; // 잘못된 ID (비숫자/0/음수)
    }
    //이름 검증
    if (tokens[1][0] == '\0' ||
        strchr(tokens[1], ',') != NULL ||
        strlen(tokens[1]) >= NAME_LEN) {
        return SHELL_ERR_INVALID_ARGUMENT; // 잘못된 이름
    }
    if (parse_int(tokens[2], &score) != 0 || score < 0 || score > 100) {
        return SHELL_ERR_INVALID_SCORE; // 잘못된/범위 밖 점수
    }
    int rc = student_add(head, (int)id, tokens[1], (int)score);
    if (rc == -1) {
        return SHELL_ERR_DUPLICATE_STUDENT; // 중복 ID
    }
    if (rc != 0) {
        return SHELL_ERR_INVALID_ARGUMENT;
    }
    g_dirty = 1;
    printf("Student added.\n");
    return SHELL_OK;
}

static ShellResult handle_delete(char* args, Student** head) {
    char* tokens[4];
    int n = tokenize(args, tokens, 4);
    if (n < 1) {
        return SHELL_ERR_MISSING_ARGUMENT;
    }
    long id;
    if (parse_int(tokens[0], &id) != 0) {
        return SHELL_ERR_INVALID_ARGUMENT;
    }
    if (student_delete(head, (int)id) != 0) {
        return SHELL_ERR_STUDENT_NOT_FOUND;
    }
    g_dirty = 1;
    printf("Student deleted.\n");
    return SHELL_OK;
}

static ShellResult handle_update(char* args, Student** head) {
    char* tokens[4];
    int n = tokenize(args, tokens, 4);
    if (n < 2) {
        return SHELL_ERR_MISSING_ARGUMENT;
    }
    long id, score;
    if (parse_int(tokens[0], &id) != 0) {
        return SHELL_ERR_INVALID_ARGUMENT;
    }
    if (parse_int(tokens[1], &score) != 0 || score < 0 || score > 100) {
        return SHELL_ERR_INVALID_SCORE;
    }
    if (student_update(*head, (int)id, (int)score) != 0) {
        return SHELL_ERR_STUDENT_NOT_FOUND;
    }
    g_dirty = 1;
    printf("Student updated.\n");
    return SHELL_OK;
}

#endif /* ADMIN_MODE */

// help/clear/exit/sort + command table + dispatch + error_message 
// 화면/도움말/종료 (공통)

static ShellResult handle_clear(char* args, Student** head) {
    (void)args;
    (void)head;
    printf("\033[2J\033[H"); 
    return SHELL_OK;
}

static ShellResult handle_help(char* args, Student** head) {
    (void)args;
    (void)head;
    printf("Commands:\n");
#ifdef ADMIN_MODE
    printf("  save                      Save students to CSV\n");
    printf("  reload                    Reload students from CSV\n");
    printf("  add <id> <name> <score>   Add a student\n");
    printf("  delete <id>               Delete a student\n");
    printf("  update <id> <score>       Update student score\n");
    printf("  find <id>                 Find student by ID\n");
    printf("  list                      List all students\n");
    printf("  stats                     Show statistics\n");
    printf("  sort <name|score>         Sort students\n");
    printf("  help                      Show this help\n");
    printf("  clear                     Clear screen\n");
    printf("  exit                      Exit program\n");
#else
    printf("  reload                    Reload students from CSV\n");
    printf("  find <id>                 Find student by ID\n");
    printf("  list                      List all students\n");
    printf("  stats                     Show statistics\n");
    printf("  sort <name|score>         Sort students\n");
    printf("  help                      Show this help\n");
    printf("  clear                     Clear screen\n");
    printf("  exit                      Exit program\n");
#endif
    return SHELL_OK;
}

static ShellResult handle_exit(char* args, Student** head) {
    (void)args;
    (void)head;
    if (g_dirty) {
        // 보너스: 저장하지 않은 변경사항이 있으면 경고
        printf("Warning: there are unsaved changes.\n");
    }
    printf("Goodbye.\n");
    return SHELL_EXIT;
}

// 정렬 (보너스, 공통)

static ShellResult handle_sort(char* args, Student** head) {
    char* tokens[4];
    int n = tokenize(args, tokens, 4);
    if (n < 1) {
        return SHELL_ERR_MISSING_ARGUMENT;
    }
    if (strcmp(tokens[0], "name") == 0) {
        student_sort_by_name(head);
        printf("sorted by name.\n");
        return SHELL_OK;
    }
    if (strcmp(tokens[0], "score") == 0) {
        student_sort_by_score(head);
        printf("sorted by score.\n");
        return SHELL_OK;
    }
    return SHELL_ERR_INVALID_ARGUMENT; // 잘못된 정렬 키 
}

// command table (preprocessing flag 로 분기) 

#ifdef ADMIN_MODE
static const Command g_commands[] = {
    {"save",   handle_save,   "save",                    "Save students to CSV"},
    {"reload", handle_reload, "reload",                  "Reload students from CSV"},
    {"add",    handle_add,    "add <id> <name> <score>", "Add a student"},
    {"delete", handle_delete, "delete <id>",             "Delete a student"},
    {"update", handle_update, "update <id> <score>",     "Update student score"},
    {"find",   handle_find,   "find <id>",               "Find student by ID"},
    {"list",   handle_list,   "list",                    "List students"},
    {"stats",  handle_stats,  "stats",                   "Show statistics"},
    {"sort",   handle_sort,   "sort <name|score>",       "Sort students"},
    {"help",   handle_help,   "help",                    "Show help"},
    {"clear",  handle_clear,  "clear",                   "Clear screen"},
    {"exit",   handle_exit,   "exit",                    "Exit shell"}
};
#else // CLIENT_MODE : 조회 명령어만 포함 
static const Command g_commands[] = {
    {"reload", handle_reload, "reload",            "Reload students from CSV"},
    {"find",   handle_find,   "find <id>",         "Find student by ID"},
    {"list",   handle_list,   "list",              "List students"},
    {"stats",  handle_stats,  "stats",             "Show statistics"},
    {"sort",   handle_sort,   "sort <name|score>", "Sort students"},
    {"help",   handle_help,   "help",              "Show help"},
    {"clear",  handle_clear,  "clear",             "Clear screen"},
    {"exit",   handle_exit,   "exit",              "Exit shell"}
};
#endif

static const int g_command_count =
    (int)(sizeof(g_commands) / sizeof(g_commands[0]));

// 명령어 이름으로 command table 에서 엔트리를 찾는다. 
static const Command* find_command(const char* name) {
    for (int i = 0; i < g_command_count; i++) {
        if (strcmp(g_commands[i].name, name) == 0) {
            return &g_commands[i];
        }
    }
    return NULL;
}

ShellResult command_dispatch(char* line, Student** head) {
    // 앞쪽 공백 제거
    while (*line == ' ' || *line == '\t') {
        line++;
    }
    if (*line == '\0') {
        return SHELL_OK; 
    }

    // 명령어 이름과 인자 분리 
    char* args = line;
    while (*args != '\0' && *args != ' ' && *args != '\t') {
        args++;
    }
    if (*args != '\0') {
        *args = '\0';
        args++;
        while (*args == ' ' || *args == '\t') {
            args++;
        }
    }
    const Command* cmd = find_command(line);
    if (cmd == NULL) {
        return SHELL_ERR_UNKNOWN_COMMAND;
    }
    return cmd->handler(args, head);
}

const char* error_message(ShellResult result) {
    switch (result) {
        case SHELL_ERR_UNKNOWN_COMMAND:
#ifdef CLIENT_MODE
            return "Unknown command or permission denied.";
#else
            return "Unknown command.";
#endif
        case SHELL_ERR_INVALID_ARGUMENT:
            return "Error: invalid argument.";
        case SHELL_ERR_MISSING_ARGUMENT:
            return "Error: missing argument.";
        case SHELL_ERR_FILE_OPEN:
            return "Error: cannot open file.";
        case SHELL_ERR_FILE_WRITE:
            return "Error: cannot write file.";
        case SHELL_ERR_STUDENT_NOT_FOUND:
            return "Error: student not found.";
        case SHELL_ERR_DUPLICATE_STUDENT:
            return "Error: duplicate ID.";
        case SHELL_ERR_INVALID_SCORE:
            return "Error: invalid score.";
        default:
            return "Error.";
    }
}