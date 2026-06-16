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

// id 로 학생 노드를 검색. 없으면 NULL
Student* student_find(Student* head, int id) {
    for (Student* cur = head; cur != NULL; cur = cur->next) {
        if (cur->id == id) {
            return cur;
        }
    }
    return NULL;
}

/* id 학생의 점수를 수정한다. 성공 0, 없으면 -1. */
int student_update(Student* head, int id, int score) {
    Student* node = student_find(head, id);
    if (node == NULL) {
        return -1;
    }
    node->score = score;
    return 0;
}

// 학생 수 반환
int student_count(Student* head) {
    int count = 0;
    for (Student* cur = head; cur != NULL; cur = cur->next) {
        count++;
    }
    return count;
}

// 리스트 전체를 해제, head 를 NULL 로 만듦
void student_free_all(Student** head) {
    Student* cur = *head;
    while (cur != NULL) {
        Student* next = cur->next;
        free(cur);
        cur = next;
    }
    *head = NULL;
}

// 두 노드의 데이터(연결 제외) 교환
static void swap_data(Student* a, Student* b) {
    int tmp_id = a->id;
    char tmp_name[NAME_LEN];
    int tmp_score = a->score;

    a->id = b->id;
    strcpy(tmp_name, a->name);
    strcpy(a->name, b->name);
    strcpy(b->name, tmp_name);
    a->score = b->score;

    b->id = tmp_id;
    b->score = tmp_score;
}

// 이름 오름차순 정렬 
void student_sort_by_name(Student** head) {
    if (*head == NULL) {
        return;
    }
    int swapped;
    do {
        swapped = 0;
        for (Student* cur = *head; cur->next != NULL; cur = cur->next) {
            if (strcmp(cur->name, cur->next->name) > 0) {
                swap_data(cur, cur->next);
                swapped = 1;
            }
        }
    } while (swapped);
}

// 점수 오름차순 정렬 
void student_sort_by_score(Student** head) {
    if (*head == NULL) {
        return;
    }
    int swapped;
    do {
        swapped = 0;
        for (Student* cur = *head; cur->next != NULL; cur = cur->next) {
            if (cur->score > cur->next->score) {
                swap_data(cur, cur->next);
                swapped = 1;
            }
        }
    } while (swapped);
}