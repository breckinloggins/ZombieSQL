//  engine.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "engine.h"
#include "types.h"

/*
 * Private helper methods
 */

int _insertTableIntoDatabase(ZdbDatabase* db, ZdbTable* table)
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

size_t _calculateRowSize(int columnCount, ZdbColumn** columns)
{
   /* TODO: Should memoize this */
   size_t size = 0;
   for (int i = 0; i < columnCount; i++)
   {
      size_t thisSize = 0;
      ZdbTypeSizeof(columns[i]->type, NULL, &thisSize);
      size += thisSize;
   }

   return size;
}

size_t _calculateRowOffset(ZdbColumn** columns, int column)
{
   /* TODO: Should memoize this */
   size_t offset = 0;
   for (int i = 1; i <= column; i++)
   {
      size_t thisSize = 0;
      ZdbTypeSizeof(columns[i-1]->type, NULL, &thisSize);

      offset += thisSize;
   }

   return offset;
}

/*
 * Public Interface Methods
 */

int ZdbEngineCreateColumn(char* name, ZdbType* type, int autoincrement, ZdbColumn** column)
{
   ZdbColumn* c = malloc(sizeof(ZdbColumn));
   c->type = type;
   strcpy(c->name, name);
   c->autoincrement = autoincrement;
   c->lastInsertedValue = NULL;

   *column = c;
   return ZDB_RESULT_SUCCESS;
}

int ZdbEngineCreateTable(ZdbDatabase* db, char* name, int columnCount, ZdbColumn** columnDefs, ZdbTable** table)
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

int ZdbEngineCreateDB(char* name, ZdbDatabase** database)
{
   ZdbDatabase* db = malloc(sizeof(ZdbDatabase));
   strcpy(db->name, name);
   db->tables = NULL;
   db->tableCount = 0;
   db->freeTablesLeft = 0;

   *database = db;
   return ZDB_RESULT_SUCCESS;
}

int ZdbEngineDropTable(ZdbTable* table)
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

int ZdbEngineDropDB(ZdbDatabase* db)
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

int ZdbEngineUpdateRowValues(ZdbTable* table, ZdbRow* row, int valueCount, void** values)
{
   int newRow = row->data == NULL;
   if (newRow)
   {
      row->data = row->_rowdata;
   }

	for (int i = 0; i < valueCount; i++)
	{
      void* value = row->data + _calculateRowOffset(table->columns, i);

      if (table->columns[i]->autoincrement)
      {
         if (!newRow)
         {
             /* We don't auto increment on an existing row */
             continue ;
         }

         /* Automatically incrementing column */
         if (values[i] != NULL)
         {
             /* Cannot specify explicit value for autoincrement columns */
             return ZDB_RESULT_VALUE_ERROR;
         }

         int result = ZdbTypeNextValue(table->columns[i]->type, table->columns[i]->lastInsertedValue, value);
         if (result != ZDB_RESULT_SUCCESS)
         {
             return result;
         }
      }
      else
      {
         /* Normal column value */
         int result = ZdbTypeCopy(table->columns[i]->type, value, values[i]);
         if (result != ZDB_RESULT_SUCCESS)
         {
             return result;
         }
      }

      table->columns[i]->lastInsertedValue = value;
	}

   return 1;        /* Number of rows affected */
}

int ZdbEngineUpdateRow(ZdbTable* table, ZdbRow* row, int valueCount, ...)
{
   va_list argp;
	va_start(argp, valueCount);

   size_t rowSize;
   if (ZdbEngineGetRowDataSize(table, table->columnCount, &rowSize) != ZDB_RESULT_SUCCESS)
   {
      /* Could not calculate the row size */
      return ZDB_RESULT_INVALID_OPERATION;
   }

   void** values = calloc(valueCount, sizeof(void*));
   void* valueData = calloc(1, rowSize);

   for (int i = 0; i < valueCount; i++)
   {
      char* str = va_arg(argp, char*);

      if (str != NULL)
      {
         /* Normal column value */
         int result = ZdbTypeFromString(table->columns[i]->type, str, valueData + _calculateRowOffset(table->columns, i));
         if (result != ZDB_RESULT_SUCCESS)
         {
             return result;
         }

         values[i] = valueData + _calculateRowOffset(table->columns, i);
      }
      else
      {
         values[i] = NULL;
      }
   }

   int result = ZdbEngineUpdateRowValues(table, row, valueCount, values);

   free(values);
   free(valueData);

	va_end(argp);

   return result;
}

int ZdbEngineGetRowDataSize(ZdbTable* table, int columnCount, size_t* size)
{
   if (table == NULL)
   {
      /* Need a table definition */
      return ZDB_RESULT_INVALID_NULL;
   }

   *size = _calculateRowSize(columnCount, table->columns);

   return ZDB_RESULT_SUCCESS;
}

int ZdbEngineInsertRow(ZdbTable* table, int columnCount, ZdbRow** row)
{
   ZdbRow* r = calloc(1, sizeof(ZdbRow) + _calculateRowSize(columnCount, table->columns));

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

int ZdbEngineGetValue(ZdbTable* table, ZdbRow* row, int column, void** value)
{
   if (table == NULL || row == NULL || value == NULL)
   {
      /* Invalid parameters to call */
      return ZDB_RESULT_INVALID_NULL;
   }

   size_t offset = _calculateRowOffset(table->columns, column);
   *value = row->data + offset;

   return ZDB_RESULT_SUCCESS;
}

void ZdbPrintColumn(ZdbColumn* column)
{
   printf("%s", column->name);
}

void ZdbPrintColumnValue(ZdbType* type, void* value)
{
   if (!ZdbTypeSupportsToString(type))
   {
      /* Print something to indicate there's something there, but we can't print it */
      printf("%s", "{}");
      return;
   }

   size_t length = 0;
   ZdbTypeToString(type, value, &length, NULL);
   if (length == 0)
   {
      printf(" ");
      return;
   }

   char* s = malloc((length + 1) * sizeof(char));
   int result = ZdbTypeToString(type, value, &length, s);
   if (result != ZDB_RESULT_SUCCESS)
   {
      printf("ERROR");
      return;
   }

   printf("%s", s);
   free(s);
}

void ZdbPrintRow(ZdbTable* table, ZdbRow* row, int columnCount)
{
   int i;
   for (i = 0; i < columnCount; i++)
   {
      void* value;
      if (ZdbEngineGetValue(table, row, i, &value) != ZDB_RESULT_SUCCESS)
      {
         printf("ERROR");
      }
      else
      {
         ZdbPrintColumnValue(table->columns[i]->type, value);
      }

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
      ZdbPrintRow(table, table->rows[i], table->columnCount);
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
