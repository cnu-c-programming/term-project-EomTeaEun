//csv입출력 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_io.h"

#define LINE_BUF 256

// 문자열 양 끝의 공백/개행 문자를 제거
static char* trim(char* s) {
    char* end;
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
        s++;
    }
    end = s + strlen(s);
    while (end > s &&
           (end[-1] == ' ' || end[-1] == '\t' ||
            end[-1] == '\r' || end[-1] == '\n')) {
        *--end = '\0';
    }
    return s;
}

// 문자열이 정수인지 검사
static int is_integer(const char* s) {
    if (s == NULL || *s == '\0') {
        return 0;
    }
    if (*s == '+' || *s == '-') {
        s++;
    }
    if (*s == '\0') {
        return 0;
    }
    for (; *s != '\0'; s++) {
        if (*s < '0' || *s > '9') {
            return 0;
        }
    }
    return 1;
}

int file_load(const char* path, Student** head) {
    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        return -1; /* 파일 열기 실패 */
    }

    char line[LINE_BUF];

    //header 줄 검사
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return -2; /* 빈 파일 -> header 없음 */
    }
    if (strcmp(trim(line), "id,name,score") != 0) {
        fclose(fp);
        return -2; /* 잘못된 header */
    }

    //데이터 줄 파싱
    int count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        char* row = trim(line);
        if (*row == '\0') {
            continue; /* 빈 줄 무시 */
        }

        char* sid = strtok(row, ",");
        char* sname = strtok(NULL, ",");
        char* sscore = strtok(NULL, ",");

        if (sid == NULL || sname == NULL || sscore == NULL) {
            continue; /* 형식 오류 줄은 건너뜀 */
        }
        sid = trim(sid);
        sname = trim(sname);
        sscore = trim(sscore);

        if (!is_integer(sid) || !is_integer(sscore) || *sname == '\0') {
            continue;
        }
        int id = atoi(sid);
        int score = atoi(sscore);
        if (score < 0 || score > 100) {
            continue;
        }
        if (student_add(head, id, sname, score) == 0) {
            count++;
        }
    }

    fclose(fp);
    return count;
}

int file_save(const char* path, Student* head) {
    FILE* fp = fopen(path, "w");
    if (fp == NULL) {
        return -1; /* 파일 쓰기 실패 */
    }

    fprintf(fp, "id,name,score\n");
    int count = 0;
    for (Student* cur = head; cur != NULL; cur = cur->next) {
        fprintf(fp, "%d,%s,%d\n", cur->id, cur->name, cur->score);
        count++;
    }

    fclose(fp);
    return count;
}