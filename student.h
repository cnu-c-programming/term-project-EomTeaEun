#ifndef STUDENT_H
#define STUDENT_H

#define NAME_LEN 32

typedef struct Student {
    int id;
    char name[NAME_LEN];
    int score;
    struct Student* next;
} Student;

// 노드 생성 추가 삭제 검색 수정 카운트 전체해제  선언 해놓기
Student* student_create(int id, const char* name, int score);
int      student_add(Student** head, int id, const char* name, int score);
int      student_delete(Student** head, int id);
Student* student_find(Student* head, int id);
int      student_update(Student* head, int id, int score);
int      student_count(Student* head);
void     student_free_all(Student** head);

//보너스를 위한 정렬기능 선언
void student_sort_by_name(Student** head);
void student_sort_by_score(Student** head);

#endif // STUDENT_H 