//
//  query.h
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef QUERY_H
#define QUERY_H

#include "engine.h"

#define ZDB_QUERY_CONDITION_NONE    0
#define ZDB_QUERY_CONDITION_EQ      1

typedef int ZdbQueryConditionType;

typedef struct _ZdbQueryCondition ZdbQueryCondition;
typedef struct _ZdbQuery ZdbQuery;
typedef struct _ZdbRecordset ZdbRecordset;

int ZdbQueryCreate(ZdbDatabase* database, ZdbQuery** query);
int ZdbQueryAddTable(ZdbQuery* query, ZdbTable* table);
int ZdbQueryAddCondition(ZdbQuery* query, ZdbQueryConditionType type, int column, ZdbType* valueType, const char* str);
int ZdbQueryExecute(ZdbQuery* query, ZdbRecordset** recordset);
int ZdbQueryFree(ZdbQuery* query);

int ZdbQueryNextResult(ZdbRecordset* recordset);

int ZdbQueryGetValue(ZdbRecordset* recordset, int column, ZdbType* type, void** value);
int ZdbQueryGetInt(ZdbRecordset* recordset, int column, int* value);
int ZdbQueryGetBoolean(ZdbRecordset* recordset, int column, int* value);
int ZdbQueryGetString(ZdbRecordset* recordset, int column, char** value);  /* Note: You do NOT own this string! */
int ZdbQueryGetFloat(ZdbRecordset* recordset, int column, float* value);

#endif // QUERY_H
