//
//  query.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>

#include "query.h"

struct _ZdbQuery {
    ZdbDatabase* database;      /* The database this query will operate on */
    ZdbTable* table;            /* Query subject table */
};

struct _ZdbRecordset {
    ZdbQuery* query;            /* The query that created this recordset */
    int rowIndex;
};

/*
 * Internal functions
 */

ZdbResult _getValue(ZdbRecordset* recordset, int column, ZdbColumnType type, void* value)
{
    if (column < 0 || column >= recordset->query->table->columnCount)
    {
        /* Invalid column specified */
        return ZDB_RESULT_ERR_INVALID_STATE;
    }
    
    if (type != recordset->query->table->columns[column]->type)
    {
        /* Attempt to cast result to an incompatible type */
        return ZDB_RESULT_ERR_INVALID_CAST;
    }
    
    ZdbRow* resultRow = recordset->query->table->rows[recordset->rowIndex];
    ZdbColumnVal *val = &resultRow->values[column];
    
    switch (type)
    {
        case ZDB_COLTYPE_BOOLEAN:
            *(int*)value = val->boolVal;
            break;
        case ZDB_COLTYPE_INT:
        case ZDB_COLTYPE_AUTOINCREMENT:
            *(int*)value = val->intVal;
            break;
        case ZDB_COLTYPE_VARCHAR:
            *(char**)value = val->varcharVal;
            break;
        case ZDB_COLTYPE_FLOAT:
            *(float*)value = val->floatVal;
            break;
        default:
            /* Did you forget to add a type case here? */
            return ZDB_RESULT_ERR_INVALID_STATE;
            break;
    }
    
    return ZDB_RESULT_SUCCESS;
}

/*
 * Public functions
 */

ZdbResult ZdbCreateQuery(ZdbDatabase* database, ZdbQuery** query)
{
    ZdbQuery* q = malloc(sizeof(ZdbQuery));
    q->database = database;
    q->table = NULL;
    
    *query = q;
    return ZDB_RESULT_SUCCESS;
}

ZdbResult ZdbAddQueryTable(ZdbQuery* query, ZdbTable* table)
{
    if (query->table != NULL)
    {
        // Multiple tables in query is not currently supported
        return ZDB_RESULT_ERR_INVALID_STATE;
    }
    
    query->table = table;
    return ZDB_RESULT_SUCCESS;
}

ZdbResult ZdbExecuteQuery(ZdbQuery* query, ZdbRecordset** recordset)
{
    ZdbRecordset* rs = malloc(sizeof(recordset));
    rs->query = query;
    rs->rowIndex = -1;
    
    *recordset = rs;
    return ZDB_RESULT_SUCCESS;
}

int ZdbNextResult(ZdbRecordset* recordset)
{
    ++recordset->rowIndex;
    if (recordset->rowIndex >= recordset->query->table->rowCount)
    {
        /* Done! */
        return 0;
    }
    
    /* There are more rows available */
    return 1;
}

ZdbResult ZdbGetIntValue(ZdbRecordset* recordset, int column, int* value)
{
    int v;
    ZdbResult result = _getValue(recordset, column, ZDB_COLTYPE_INT, &v);
    if (result == ZDB_RESULT_ERR_INVALID_CAST)
    {
        /* HACK! Special case where we need to try the auto increment type
         * TODO: Give column types more structure and/or introduce the notion of compatible types
         */
        if(_getValue(recordset, column, ZDB_COLTYPE_AUTOINCREMENT, &v) != ZDB_RESULT_SUCCESS)
        {
            /* Must have been some other problem */
            return result;
        }
        else
        {
            result = ZDB_RESULT_SUCCESS;
        }
    }
    
    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = v;
    }
    
    return result;
}

ZdbResult ZdbGetBooleanValue(ZdbRecordset* recordset, int column, int* value)
{
    int v;
    ZdbResult result = _getValue(recordset, column, ZDB_COLTYPE_BOOLEAN, &v);
     
    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = v;
    }
    
    return result;
}

ZdbResult ZdbGetStringValue(ZdbRecordset* recordset, int column, char** value)
{
    char* v;
    ZdbResult result = _getValue(recordset, column, ZDB_COLTYPE_VARCHAR, &v);
    
    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = v;
    }
    
    return result;    
}

ZdbResult ZdbGetFloatValue(ZdbRecordset* recordset, int column, float* value)
{
    float v;
    ZdbResult result = _getValue(recordset, column, ZDB_COLTYPE_FLOAT, &v);
    
    if (result == ZDB_RESULT_SUCCESS)
    {
        *value = v;
    }
    
    return result;
}




