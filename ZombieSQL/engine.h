//
//  engine.h
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef ENGINE_H
#define ENGINE_H

#define ZDB_LIMIT_VARCHAR       255
#define ZDB_LIMIT_COLUMNS       32

#define ZDB_COLTYPE_BOOLEAN         1
#define ZDB_COLTYPE_INT             2
#define ZDB_COLTYPE_FLOAT           3
#define ZDB_COLTYPE_VARCHAR         4
#define ZDB_COLTYPE_AUTOINCREMENT   5

#define ZDB_ROW_CHUNKS          128
#define ZDB_TABLE_CHUNKS        32

#define ZDB_RESULT_SUCCESS             0        /* No error ocurred */
#define ZDB_RESULT_ERR_AUTOINCREMENT   -1       /* Attempt to update or set an autoincrement value */

typedef int ZdbColumnType;
typedef int ZdbResult;

typedef struct
{
    int ignored;                        /* Set if the database should ignore whatever the value is (e.g. for autoincrement) */
    union 
    {
        char boolVal;
        int intVal;
        float floatVal;
        char varcharVal[ZDB_LIMIT_VARCHAR];        
    };
} ZdbColumnVal;

typedef struct
{
    ZdbColumnType type;
    char name[ZDB_LIMIT_VARCHAR];
    ZdbColumnVal* lastInsertedValue;       /* Used mainly to track the last autoincrement number */
} ZdbColumn;

typedef struct
{
    ZdbColumnVal values[ZDB_LIMIT_COLUMNS];
} ZdbRow;


typedef struct
{
    char name[ZDB_LIMIT_VARCHAR];
    int columnCount;
    int rowCount;
    int freeRowsLeft;
    
    ZdbColumn** columns;
    ZdbRow** rows;
} ZdbTable;

typedef struct
{
    char name[ZDB_LIMIT_VARCHAR];
    int tableCount;
    int freeTablesLeft;
    
    ZdbTable** tables;
} ZdbDatabase;

ZdbResult ZdbCreateColumn(char* name, ZdbColumnType type, ZdbColumn** column);
ZdbResult ZdbCreateTable(ZdbDatabase* db, char* name, int columnCount, ZdbColumn** columnDefs, ZdbTable** table);
ZdbResult ZdbCreateDatabase(char* name, ZdbDatabase** database);
ZdbResult ZdbDropTable(ZdbTable* table);
ZdbResult ZdbDropDatabase(ZdbDatabase* db);
ZdbResult ZdbInsertRow(ZdbTable* table, int columnCount, ZdbColumnVal* values, ZdbRow** row);

void ZdbPrintColumn(ZdbColumn* column);
void ZdbPrintColumnValue(ZdbColumnType type, ZdbColumnVal* value);
void ZdbPrintRow(ZdbRow* row, ZdbColumn** columns, int columnCount);
void ZdbPrintTable(ZdbTable* table);
void ZdbPrintDatabase(ZdbDatabase* db);









#endif // ENGINE_H