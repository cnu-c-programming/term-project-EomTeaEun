#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "student.h"

// 새 학생 노드 생성
Student* student_create(int id, const char* name, int score) {
    Student* node = (Student*)malloc(sizeof(Student));
    if (node == NULL) {
        return NULL;
    }
    node->id = id;
    strncpy(node->name, name, NAME_LEN - 1);
    node->name[NAME_LEN - 1] = '\0';
    node->score = score;
    node->next = NULL;
    return node;
}

// 리스트 끝에 학생 추가하기
int student_add(Student** head, int id, const char* name, int score) {
    if (student_find(*head, id) != NULL) {
        return -1; /* 중복 ID */
    }
    Student* node = student_create(id, name, score);
    if (node == NULL) {
        return -2;
    }
    if (*head == NULL) {
        *head = node;
        return 0;
    }
    Student* cur = *head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = node;
    return 0;
}

// id 로 학생 삭제하기
int student_delete(Student** head, int id) {
    Student* cur = *head;
    Student* prev = NULL;
    while (cur != NULL) {
        if (cur->id == id) {
            if (prev == NULL) {
                *head = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur);
            return 0;
        }
        prev = cur;
        cur = cur->next;
    }
    return -1;
}

