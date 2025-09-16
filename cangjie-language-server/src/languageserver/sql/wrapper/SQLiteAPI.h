// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "sqlite3.h"

#ifdef SQLITE_EXTENSION_API
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT3
#endif

using sqlite3_destructor_type = void (*)(void *);

namespace sqlite {

extern const int OK;
extern const int Error;
extern const int Internal;
extern const int Perm;
extern const int Abort;
extern const int Busy;
extern const int Locked;
extern const int NoMem;
extern const int ReadOnly;
extern const int Interrupt;
extern const int IOErr;
extern const int Corrupt;
extern const int Full;
extern const int CantOpen;
extern const int Protocol;
extern const int Schema;
extern const int TooBig;
extern const int Constraint;
extern const int Mismatch;
extern const int Misuse;
extern const int NotADB;
extern const int Notice;
extern const int Warning;

extern const int OpenReadOnly;
extern const int OpenReadWrite;
extern const int OpenCreate;
extern const int OpenURI;

extern const int Integer;
extern const int Float;
extern const int Text;
extern const int Blob;
extern const int Null;

extern const int UTF8;

extern const int Deterministic;
extern const int DirectOnly;
extern const int Innocuous;

extern const sqlite3_destructor_type Static;
extern const sqlite3_destructor_type Transient;

int value_type(sqlite3_value *V);

long long value_int64(sqlite3_value *V);
double value_double(sqlite3_value *V);
unsigned value_bytes(sqlite3_value *V);
const void *value_blob(sqlite3_value *V);
const unsigned char *value_text(sqlite3_value *V);

int column_count(sqlite3_stmt *S);

sqlite3_value *column_value(sqlite3_stmt *S, int I);

int bind_null(sqlite3_stmt *S, int I);
int bind_int64(sqlite3_stmt *S, int I, long long V);
int bind_double(sqlite3_stmt *S, int I, double V);
int bind_text64(
    sqlite3_stmt *S, int I, const char *V, unsigned long long N, sqlite3_destructor_type D, unsigned char E);
int bind_blob64(sqlite3_stmt *S, int I, const void *V, unsigned long long N, sqlite3_destructor_type D);

unsigned bind_parameter_index(sqlite3_stmt *S, const char *Name);

void result_error(sqlite3_context *C, const char *V, int N);
void result_int64(sqlite3_context *C, long long V);
void result_double(sqlite3_context *C, double V);
void result_text64(sqlite3_context *C, const char *V, unsigned long long N, sqlite3_destructor_type D, unsigned char E);
void result_blob64(sqlite3_context *C, const void *V, unsigned long long N, sqlite3_destructor_type D);
void result_null(sqlite3_context *C);

int create_scalar(sqlite3 *DB,
    const char *Name,
    int ArgsNum,
    int Flags,
    void *AppData,
    void (*Function)(sqlite3_context *, int, sqlite3_value **),
    void (*Destroy)(void *));

void *user_data(sqlite3_context *C);

int create_aggregate(sqlite3 *DB,
    const char *Name,
    int ArgsNum,
    int Flags,
    void *AppData,
    void (*Step)(sqlite3_context *, int, sqlite3_value **),
    void (*Final)(sqlite3_context *),
    void (*Destroy)(void *));

void *aggregate_context(sqlite3_context *C, int N);

namespace fts5 {

int create_tokenizer(sqlite3 *DB,
    const char *Name,
    void *AppData,
    int (*Tokenize)(
        Fts5Tokenizer *, void *, int, const char *, int, int (*Callback)(void *, int, const char *, int, int, int)),
    void (*Destroy)(void *));

} // namespace fts5
} // namespace sqlite
