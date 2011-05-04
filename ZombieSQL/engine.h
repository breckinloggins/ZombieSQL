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

#define ZDB_ROW_CHUNKS          128
#define ZDB_TABLE_CHUNKS        32

#define ZDB_RESULT_SUCCESS             0        /* No error ocurred */
#define ZDB_RESULT_ERR_AUTOINCREMENT   -1       /* Attempt to update or set an autoincrement value */
#define ZDB_RESULT_ERR_INVALID_STATE   -2       /* A parameter resulted in an invalid or unsupported state */
#define ZDB_RESULT_ERR_INVALID_CAST    -3       /* Attempt was made to get or set a value in the database of a different type than the column's datatype */
#define ZDB_RESULT_ERR_UNSUPPORTED     -4       /* Operation is not supported */

typedef int ZdbResult;
typedef struct _ZdbType ZdbType;

typedef struct
{
    union 
    {
        int boolVal;
        int intVal;
        float floatVal;
        char varcharVal[ZDB_LIMIT_VARCHAR];        
    };
} ZdbColumnVal;

typedef struct
{
    ZdbType* type;
    char name[ZDB_LIMIT_VARCHAR];
    int autoincrement;                     /* Whether values autoincrement */
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

ZdbResult ZdbEngineCreateColumn(char* name, ZdbType *type, int autoincrement, ZdbColumn** column);
ZdbResult ZdbEngineCreateTable(ZdbDatabase* db, char* name, int columnCount, ZdbColumn** columnDefs, ZdbTable** table);
ZdbResult ZdbEngineCreateDB(char* name, ZdbDatabase** database);
ZdbResult ZdbEngineDropTable(ZdbTable* table);
ZdbResult ZdbEngineDropDB(ZdbDatabase* db);
ZdbResult ZdbEngineInsertRow(ZdbTable* table, int columnCount, ZdbColumnVal* values, ZdbRow** row);

/* TO BE RENAMED AND MOVED TO DESCRIBE MODULE */
void ZdbPrintColumn(ZdbColumn* column);
void ZdbPrintColumnValue(ZdbType* type, ZdbColumnVal* value);
void ZdbPrintRow(ZdbRow* row, ZdbColumn** columns, int columnCount);
void ZdbPrintTable(ZdbTable* table);
void ZdbPrintDatabase(ZdbDatabase* db);
/* ... */

#endif // ENGINE_H