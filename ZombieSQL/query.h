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

ZdbResult ZdbQueryCreate(ZdbDatabase* database, ZdbQuery** query);
ZdbResult ZdbQueryAddTable(ZdbQuery* query, ZdbTable* table);
ZdbResult ZdbQueryAddCondition(ZdbQuery* query, ZdbQueryConditionType type, int column, ZdbColumnType valueType, ZdbColumnVal value);
ZdbResult ZdbQueryExecute(ZdbQuery* query, ZdbRecordset** recordset);

int ZdbQueryNextResult(ZdbRecordset* recordset);

ZdbResult ZdbQueryGetInt(ZdbRecordset* recordset, int column, int* value);
ZdbResult ZdbQueryGetBoolean(ZdbRecordset* recordset, int column, int* value);
ZdbResult ZdbQueryGetString(ZdbRecordset* recordset, int column, char** value);  /* Note: You do NOT own this string! */
ZdbResult ZdbQueryGetFloat(ZdbRecordset* recordset, int column, float* value);

#endif // QUERY_H