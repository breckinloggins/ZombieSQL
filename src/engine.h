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

#define ZDB_RESULT_SUCCESS              0       /* The operation completed successfully */
#define ZDB_RESULT_VALUE_ERROR          -1      /* There was a problem setting a value in a table */
#define ZDB_RESULT_INVALID_CAST         -2      /* The specified cast is invalid */
#define ZDB_RESULT_INVALID_OPERATION    -3      /* The operation was invalid */
#define ZDB_RESULT_INVALID_NULL         -4      /* Invalid use of NULL */
#define ZDB_RESULT_UNSUPPORTED          -5      /* The attempted operation is not supported */

typedef struct _ZdbType ZdbType;

typedef struct
{
    ZdbType* type;
    char name[ZDB_LIMIT_VARCHAR];
    int autoincrement;                     /* Whether values autoincrement */
    void* lastInsertedValue;       /* Used mainly to track the last autoincrement number */
} ZdbColumn;

typedef struct
{
    void* data;
    
    /* Insert additional row properties here */
    
    char _rowdata[0];               /* This MUST be the last member of the struct */
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

int ZdbEngineCreateColumn(char* name, ZdbType *type, int autoincrement, ZdbColumn** column);
int ZdbEngineCreateTable(ZdbDatabase* db, char* name, int columnCount, ZdbColumn** columnDefs, ZdbTable** table);
int ZdbEngineCreateDB(char* name, ZdbDatabase** database);
int ZdbEngineDropTable(ZdbTable* table);
int ZdbEngineDropDB(ZdbDatabase* db);
int ZdbEngineInsertRow(ZdbTable* table, int columnCount, ZdbRow** row);
int ZdbEngineGetRowDataSize(ZdbTable* table, int columnCount, size_t* size);
int ZdbEngineUpdateRowValues(ZdbTable* table, ZdbRow* row, int valueCount, void** values);
int ZdbEngineUpdateRow(ZdbTable* table, ZdbRow* row, int valueCount, ...);
int ZdbEngineGetValue(ZdbTable* table, ZdbRow* row, int column, void** value);

/* TO BE RENAMED AND MOVED TO DESCRIBE MODULE */
void ZdbPrintColumn(ZdbColumn* column);
void ZdbPrintColumnValue(ZdbType* type, void* value);
void ZdbPrintRow(ZdbTable* table, ZdbRow* row, int columnCount);
void ZdbPrintTable(ZdbTable* table);
void ZdbPrintDatabase(ZdbDatabase* db);
/* ... */

#endif // ENGINE_H
