#ifndef DEFINE_H
#define DEFINE_H

#include "sqlite3.h"

#ifdef __cplusplus
extern "C" {
#endif

int define_manage_init(sqlite3* db);
int define_eval_init(sqlite3* db);

#ifdef __cplusplus
}
#endif

#endif // DEFINE_H
