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

typedef struct _ZdbQuery ZdbQuery;
typedef struct _ZdbRecordset ZdbRecordset;

ZdbResult ZdbCreateQuery(ZdbDatabase* database, ZdbQuery** query);
ZdbResult ZdbAddQueryTable(ZdbQuery* query, ZdbTable* table);
ZdbResult ZdbExecuteQuery(ZdbQuery* query, ZdbRecordset** recordset);

int ZdbNextResult(ZdbRecordset* recordset);

ZdbResult ZdbGetIntValue(ZdbRecordset* recordset, int column, int* value);
ZdbResult ZdbGetBooleanValue(ZdbRecordset* recordset, int column, int* value);
ZdbResult ZdbGetStringValue(ZdbRecordset* recordset, int column, char** value);  /* Note: You do NOT own this string! */
ZdbResult ZdbGetFloatValue(ZdbRecordset* recordset, int column, float* value);

#endif // QUERY_H