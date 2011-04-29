//
//  engine.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"

/*
 * Private helper methods
 */

ZdbResult _insertTableIntoDatabase(ZdbDatabase* db, ZdbTable* table)
{
    if (db->freeTablesLeft == 0)
    {
        /* We're out of free tables, so allocate some more */
        db->tables = realloc(db->tables, (db->tableCount + ZDB_TABLE_CHUNKS)*sizeof(ZdbTable*));
        db->freeTablesLeft = ZDB_TABLE_CHUNKS;
    }
    
    db->tables[db->tableCount++] = table;
    db->freeTablesLeft--;
    
    return ZDB_RESULT_SUCCESS;
}

/*
 * Public Interface Methods
 */

ZdbResult ZdbEngineCreateColumn(char* name, ZdbColumnType type, ZdbColumn** column)
{
    ZdbColumn* c = malloc(sizeof(ZdbColumn));
    c->type = type;
    strcpy(c->name, name);
    c->lastInsertedValue = NULL;
    
    *column = c;
    return ZDB_RESULT_SUCCESS;
}

ZdbResult ZdbEngineCreateTable(ZdbDatabase* db, char* name, int columnCount, ZdbColumn** columnDefs, ZdbTable** table)
{
    int i;
    ZdbTable* t = malloc(sizeof(ZdbTable));
    
    strcpy(t->name, name);
    t->columnCount = columnCount;
    
    t->columns = malloc(columnCount * sizeof(ZdbColumn*));
    for (i = 0; i < columnCount; i++)
    {
        t->columns[i] = columnDefs[i];
    }
    
    t->rows = NULL; 
    t->freeRowsLeft = 0;
    t->rowCount = 0;
    
    _insertTableIntoDatabase(db, t);
    
    *table = t;
    return ZDB_RESULT_SUCCESS;
}

ZdbResult ZdbEngineCreateDB(char* name, ZdbDatabase** database)
{
    ZdbDatabase* db = malloc(sizeof(ZdbDatabase));
    strcpy(db->name, name);
    db->tables = NULL;
    db->tableCount = 0;
    db->freeTablesLeft = 0;
    
    *database = db;
    return ZDB_RESULT_SUCCESS;
}

ZdbResult ZdbEngineDropTable(ZdbTable* table)
{
    int i;
    for (i = 0; i < table->columnCount; i++)
    {
        free(table->columns[i]);
    }
    free(table->columns);
    table->columns = NULL;
    table->columnCount = 0;
    
    for (i = 0; i < table->rowCount; i++)
    {
        free(table->rows[i]);
    }
    free(table->rows);
    table->rows = NULL;
    table->rowCount = 0;
    table->freeRowsLeft = ZDB_ROW_CHUNKS;
    
    return ZDB_RESULT_SUCCESS;
}

ZdbResult ZdbEngineDropDB(ZdbDatabase* db)
{
    int i;
    for (i = 0; i < db->tableCount; i++)
    {
        ZdbEngineDropTable(db->tables[i]);
        free(db->tables[i]);
        db->tables[i] = NULL;
    }
    free(db->tables);
    db->tables = NULL;
    db->tableCount = 0;
    db->freeTablesLeft = ZDB_TABLE_CHUNKS;
    
    return ZDB_RESULT_SUCCESS;
}

ZdbResult ZdbEngineInsertRow(ZdbTable* table, int columnCount, ZdbColumnVal* values, ZdbRow** row)
{
    int i;
    ZdbRow* r = malloc(sizeof(ZdbRow));
    
    for (i = 0; i < columnCount; i++)
    {
        if (table->columns[i]->type == ZDB_COLTYPE_AUTOINCREMENT && !values[i].ignored)
        {
            /* You can't set an auto increment column */
            free(r);
            return ZDB_RESULT_ERR_AUTOINCREMENT;
        }
        else if (table->columns[i]->type == ZDB_COLTYPE_AUTOINCREMENT)
        {
            int lastValue = -1;
            if (table->columns[i]->lastInsertedValue != NULL)
            {
                lastValue = table->columns[i]->lastInsertedValue->intVal;
            }
            
            values[i].intVal = ++lastValue;
            r->values[i] = values[i];
        }
        else if (!values[i].ignored)
        {
            r->values[i] = values[i];
        }
        
        table->columns[i]->lastInsertedValue = &(values[i]);
    }
    
    if (table->freeRowsLeft == 0)
    {
        /* We're out of free rows, so allocate some more */
        table->rows = realloc(table->rows, (table->rowCount + ZDB_ROW_CHUNKS)*sizeof(ZdbRow*));
        table->freeRowsLeft = ZDB_ROW_CHUNKS;
    }
    
    table->rows[table->rowCount++] = r;
    table->freeRowsLeft--;
    
    *row = r;
    return ZDB_RESULT_SUCCESS;
}

ZdbColumnType ZdbGetCanonicalType(ZdbColumnType type)
{
    if (type == ZDB_COLTYPE_AUTOINCREMENT)
    {
        return ZDB_COLTYPE_INT;
    }
    
    return type;
}

int ZdbTypesCompatible(ZdbColumnType type1, ZdbColumnType type2)
{
    return ZdbGetCanonicalType(type1) == ZdbGetCanonicalType(type2);
}

void ZdbPrintColumn(ZdbColumn* column)
{
    printf("%s", column->name);
}

void ZdbPrintColumnValue(ZdbColumnType type, ZdbColumnVal* value)
{
    ZdbColumnType canonicalType = ZdbGetCanonicalType(type);
    switch(canonicalType)
    {
        case ZDB_COLTYPE_BOOLEAN:
            printf("%s", value->boolVal? "TRUE" : "FALSE");
            break;
        case ZDB_COLTYPE_INT:
            printf("%d", value->intVal);
            break;
        case ZDB_COLTYPE_FLOAT:
            printf("%f", value->floatVal);
            break;
        case ZDB_COLTYPE_VARCHAR:
            printf("%s", value->varcharVal);
            break;
        default:
            printf("ERROR IN PRINTCOLUMNVALUE\n");
            break;
    }
}

void ZdbPrintRow(ZdbRow* row, ZdbColumn** columns, int columnCount)
{
    int i;
    for (i = 0; i < columnCount; i++)
    {
        ZdbPrintColumnValue(columns[i]->type, &row->values[i]);
        printf("\t");
    }
}

void ZdbPrintTable(ZdbTable* table)
{
    printf("%s\n", table->name);
    
    int i;
    for (i = 0; i < table->columnCount; i++)
    {
        ZdbPrintColumn(table->columns[i]);
        printf("\t");
    }
    
    printf("\n\n");
    
    for (i = 0; i < table->rowCount; i++)
    {
        ZdbPrintRow(table->rows[i], table->columns, table->columnCount);
        printf("\n");
    }
}

void ZdbPrintDatabase(ZdbDatabase* db)
{
    int i;
    printf("Database: %s\n\n", db->name);
    for (i = 0; i < db->tableCount; i++)
    {
        ZdbPrintTable(db->tables[i]);
        printf("\n\n");
    }
}
