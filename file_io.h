// csv 입출력 뼈대

#ifndef FILE_IO_H
#define FILE_IO_H

#include "student.h"

//csv를 읽고 linked list로 만듦
int file_load(const char* path, Student** head);
//linked list를 csv로 저장
int file_save(const char* path, Student* head);

#endif // FILE_IO_H 