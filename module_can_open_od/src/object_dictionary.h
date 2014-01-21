#ifndef OBJECT_DICTIONARY_H_
#define OBJECT_DICTIONARY_H_

#include "can_open_od_structs.h"

// THIS FILE IS GENERATED - DO NOT EDIT

#define NAME_TABLE_SIZE 1839
#define NUM_VARS 28
#define NUM_ARRAYS 15
#define NUM_RECORDS 12
#define NUM_RECORD_SIS 59
#define NUM_OBJECTS 40

extern char byte_table[];
extern short short_table[];
extern unsigned word_table[];
//Data bytes: 5659

extern char name_table[NAME_TABLE_SIZE];
extern var_t vars[NUM_VARS];
extern array_t arrays[NUM_ARRAYS];
extern record_subindex_t record_si[NUM_RECORD_SIS];
extern record_t records[NUM_RECORDS];
extern object_t objects[NUM_OBJECTS];
#endif
