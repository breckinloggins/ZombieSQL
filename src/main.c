//
//  main.c
//  ZombieSQL
//
//  Created by Benjamin Loggins on 4/20/11.
//  Copyright 2011 GreatFoundry. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zdb.h"

static int testLevel = 0;
#define TEST_INDENT()                   { int i = 0; while (i++ < testLevel) { printf("\t"); } }
#define TEST_START(name)                { TEST_INDENT(); printf("Testing %s\n", name); testLevel++; }
#define TEST_ASSERT(test, stmt)         { if (!(stmt)) { TEST_INDENT(); printf("FAIL (%s)\n", test); exit(1); } }
#define TEST_PASS()                     { testLevel--; TEST_INDENT(); printf("PASS\n"); }

void CreateTestRow(ZdbTable* table, char* name, char* age, char* salary, char* active)
{
    TEST_START("CreateTestRow");

    ZdbRow* r;
    TEST_ASSERT("Insert row", !ZdbEngineInsertRow(table, 5, &r));
    TEST_ASSERT("Update row", ZdbEngineUpdateRow(table, r, 5, NULL, name, age, salary, active) == 1);

    TEST_PASS();
}

ZdbDatabase* CreateTestDatabase()
{
    TEST_START("CreateTestDatabase");

    ZdbDatabase* db = NULL;
    TEST_ASSERT("ZdbEngineCreateDB", !ZdbEngineCreateDB("Company", &db));

    ZdbColumn** columns = malloc(5*sizeof(ZdbColumn*));
    TEST_ASSERT("Create ID Column", !ZdbEngineCreateColumn("ID", ZdbStandardTypes->intType, 1, &(columns[0])));
    TEST_ASSERT("Create Name Column", !ZdbEngineCreateColumn("Name", ZdbStandardTypes->varcharType, 0, &(columns[1])));
    TEST_ASSERT("Create Age Column", !ZdbEngineCreateColumn("Age", ZdbStandardTypes->intType, 0, &(columns[2])));
    TEST_ASSERT("Create Salary Column", !ZdbEngineCreateColumn("Salary", ZdbStandardTypes->floatType, 0, &(columns[3])));
    TEST_ASSERT("Create Active Column", !ZdbEngineCreateColumn("Active", ZdbStandardTypes->booleanType, 0, &(columns[4])));

    ZdbTable* t = NULL;
    TEST_ASSERT("Create Employeees Table", !ZdbEngineCreateTable(db, "Employees", 5, columns, &t));

    CreateTestRow(t, "Breckin", "30", "34000.0", "1");
    CreateTestRow(t, "Bob", "22", "15600.0", "1");
    CreateTestRow(t, "Jane", "45", "45000.0", "1");
    CreateTestRow(t, "John", "35", "95600.0", "0");

    /* Testing that the autoincrement worked */
    ZdbQuery* q;
    ZdbRecordset* rs;
    TEST_ASSERT("create query", !ZdbQueryCreate(db, &q));
    TEST_ASSERT("add table", !ZdbQueryAddTable(q, db->tables[0]));
    TEST_ASSERT("execute", !ZdbQueryExecute(q, &rs));
    int lastId = -1;
    while (ZdbQueryNextResult(rs))
    {
        int id;
        TEST_ASSERT("get value", !ZdbQueryGetInt(rs, 0, &id));
        TEST_ASSERT("autoincrement check", lastId != id);
        lastId = id;
    }
    ZdbQueryFree(q);

    TEST_PASS();

    return db;
}

void PrintQueryResults(ZdbRecordset* rs)
{
    while(ZdbQueryNextResult(rs))
    {
        int id = 0;
        char* name;
        int age = 0;
        float salary = 0.0f;
        int active = 0;

        ZdbQueryGetInt(rs, 0, &id);
        ZdbQueryGetString(rs, 1, &name);
        ZdbQueryGetInt(rs, 2, &age);
        ZdbQueryGetFloat(rs, 3, &salary);
        ZdbQueryGetBoolean(rs, 4, &active);

        printf("Employee %d (%s/%d): $%.2f [%s]\n",
               id,
               name,
               age,
               salary,
               active? "ACTIVE" : "TERMINATED");
    }
}

void TestTypeCopyValue()
{
    int i1, i2;
    float f1, f2;
    int b1, b2;

    char s1[ZDB_LIMIT_VARCHAR], s2[ZDB_LIMIT_VARCHAR];

    TEST_START("TestTypeCopyValue");

    i1 = 10; i2 = 0;
    TEST_ASSERT("int copy", !ZdbTypeCopy(ZdbStandardTypes->intType, &i2, &i1));
    TEST_ASSERT("int check", i2 == i1);

    f1 = 11.2; f2 = 0;
    TEST_ASSERT("float copy", !ZdbTypeCopy(ZdbStandardTypes->floatType, &f2, &f1));
    TEST_ASSERT("float check", abs(f2 - f1) < 0.1);

    b1 = 1; b2 = 0;
    TEST_ASSERT("boolean copy", !ZdbTypeCopy(ZdbStandardTypes->booleanType, &b2, &b1));
    TEST_ASSERT("boolean check", b1 == b2);

    strcpy(s1, "foo"), strcpy(s2, "");
    TEST_ASSERT("varchar copy", !ZdbTypeCopy(ZdbStandardTypes->varcharType, &s2, &s1));
    TEST_ASSERT("varchar check", !strcmp(s2, "foo"));

    TEST_PASS();
}

void TestTypeSystem()
{
    TEST_START("Type System");

    TEST_ASSERT("Initialize", !ZdbTypeInitialize());

    /* TODO: Repeat all tests with all builtin types */

    // Test comparisons
    int a, b, res;

    a = 1, b = 1;
    TEST_ASSERT("int compare ==", !ZdbTypeCompare(ZdbStandardTypes->intType, &a, &b, &res));
    TEST_ASSERT("int == check", !res);

    a = 1, b = 2;
    TEST_ASSERT("int compare <", !ZdbTypeCompare(ZdbStandardTypes->intType, &a, &b, &res));
    TEST_ASSERT("int < check", res < 0);

    a = 2, b = 1;
    TEST_ASSERT("int compare >", !ZdbTypeCompare(ZdbStandardTypes->intType, &a, &b, &res));
    TEST_ASSERT("int > check", res > 0);

    // Test sizeof
    size_t size;
    TEST_ASSERT("sizeof", !ZdbTypeSizeof(ZdbStandardTypes->intType, NULL, &size));
    TEST_ASSERT("sizeof check", size == sizeof(int));
    TEST_ASSERT("sizeof unsupported", ZdbTypeSizeof(ZdbStandardTypes->intType, &a, &size) == ZDB_RESULT_UNSUPPORTED);

    // Test From String
    TEST_ASSERT("from string", !ZdbTypeFromString(ZdbStandardTypes->intType, "42", &a));
    TEST_ASSERT("from string check",  a == 42);

    // Test To String
    char str[7] = {0};
    a = 12345;
    size = 0;
    TEST_ASSERT("to string", !ZdbTypeToString(ZdbStandardTypes->intType, &a, &size, NULL));
    TEST_ASSERT("to string check",  size == 5);
    TEST_ASSERT("to string2", !ZdbTypeToString(ZdbStandardTypes->intType, &a, &size, str));
    TEST_ASSERT("to string2 check",  !strcmp(str, "12345"));

    // Test Next Value
    a = 42;
    TEST_ASSERT("next value", !ZdbTypeNextValue(ZdbStandardTypes->intType, &a, &b));
    TEST_ASSERT("next value check", b == 43);

    TestTypeCopyValue();

    TEST_PASS();
}

void TestBasicQuery(ZdbDatabase* db)
{
    TEST_START("basic query");
    ZdbQuery* q = NULL;
    TEST_ASSERT("query create", !ZdbQueryCreate(db, &q));

    TEST_ASSERT("add table", !ZdbQueryAddTable(q, db->tables[0]));

    ZdbRecordset* rs = NULL;
    TEST_ASSERT("execute", !ZdbQueryExecute(q, &rs));

    int count = 0;
    while(ZdbQueryNextResult(rs))
    {
        ++count;
    }

    TEST_ASSERT("has results", count > 0);

    ZdbQueryFree(q);
    TEST_PASS();
}

void TestOneConditionQuery(ZdbDatabase* db, int column, ZdbQueryConditionType queryType, const char* value, ZdbType* valueType)
{
    char str[255];
    sprintf(str, "OneConditionQuery (%s == %s)", db->tables[0]->columns[column]->name, value);
    TEST_START(str);

    ZdbQuery* q = NULL;
    TEST_ASSERT("create query", !ZdbQueryCreate(db, &q));
    TEST_ASSERT("add table", !ZdbQueryAddTable(q, db->tables[0]));
    TEST_ASSERT("add condition", !ZdbQueryAddCondition(q, queryType, column, valueType, value));

    ZdbRecordset* rs = NULL;
    TEST_ASSERT("execute query", !ZdbQueryExecute(q, &rs));

    while(ZdbQueryNextResult(rs))
    {
        void* v;
        size_t length = 255;
        TEST_ASSERT("get value", !ZdbQueryGetValue(rs, column, valueType, &v));
        TEST_ASSERT("to string", !ZdbTypeToString(valueType, v, &length, str));
        TEST_ASSERT("value equals", !strcmp(value, str));
    }

    ZdbQueryFree(q);
    TEST_PASS();
}

void UpdateRowTestHelper(ZdbDatabase* db, int table, int queryColumn, const char* queryValue, int updateColumn, void* updateValue)
{
    /* Most of this code will likely turn into the Update command code */


    ZdbType* queryColumnType = db->tables[table]->columns[queryColumn]->type;
    ZdbType* updateColumnType = db->tables[table]->columns[updateColumn]->type;

    /* Get some names of things to make diagnostic messages less painful */
    char tableName[ZDB_LIMIT_VARCHAR], columnName[ZDB_LIMIT_VARCHAR], originalValueString[ZDB_LIMIT_VARCHAR], updateValueString[ZDB_LIMIT_VARCHAR], testName[ZDB_LIMIT_VARCHAR];

    strcpy(tableName, db->tables[table]->name);
    strcpy(columnName, db->tables[table]->columns[updateColumn]->name);
    size_t length = ZDB_LIMIT_VARCHAR;
    ZdbTypeToString(updateColumnType, updateValue, &length, updateValueString);

    sprintf(testName, "Update %s(%s) [QUERY]", tableName, columnName);
    TEST_START(testName);

    ZdbQuery* q;
    ZdbRecordset* rs;
    TEST_ASSERT("create query", !ZdbQueryCreate(db, &q));
    TEST_ASSERT("add table", !ZdbQueryAddTable(q, db->tables[table]));
    TEST_ASSERT("add condition", !ZdbQueryAddCondition(q, ZDB_QUERY_CONDITION_EQ, queryColumn, queryColumnType, queryValue));
    TEST_ASSERT("execute query", !ZdbQueryExecute(q, &rs));
    TEST_ASSERT("has results", ZdbQueryNextResult(rs));

    /* Get the current value so we can print it*/
    void* originalValue;
    length = ZDB_LIMIT_VARCHAR;
    TEST_ASSERT("get value", !ZdbQueryGetValue(rs, updateColumn, updateColumnType, &originalValue));
    TEST_ASSERT("to string", !ZdbTypeToString(updateColumnType, originalValue, &length, originalValueString));

    TEST_PASS();

    sprintf(testName, "Update %s => %s", originalValueString, updateValueString);
    TEST_START(testName);

    int rowId = -1;

    /* Collect the values for this row as a string */
    void** values = calloc(db->tables[table]->columnCount, sizeof(void*));

    int i;
    for (i = 0; i < db->tables[table]->columnCount; i++)
    {
        void* value;
        ZdbType* thisType = db->tables[table]->columns[i]->type;
        TEST_ASSERT("get value", !ZdbQueryGetValue(rs, i, thisType, &value));

        if (rowId == -1 && db->tables[table]->columns[i]->autoincrement)
        {
            /* For our test DB, we can assume the first auto increment column is an
              int row id that matches the row index */
            rowId = *(int*)value;
        }

        if (i == updateColumn)
        {
            values[i] = updateValue;
        }
        else
        {
            values[i] = value;
        }
    }


    ZdbRow* row = db->tables[table]->rows[rowId];
    TEST_ASSERT("update row values", ZdbEngineUpdateRowValues(db->tables[table], row, db->tables[table]->columnCount, values) == 1);

    void* updatedValue;
    TEST_ASSERT("get value", !ZdbQueryGetValue(rs, updateColumn, updateColumnType, &updatedValue));
    int result = 0;
    TEST_ASSERT("type compare", !ZdbTypeCompare(updateColumnType, updateValue, updatedValue, &result));
    TEST_ASSERT("update check", result == 0);

    free(values);

    ZdbQueryFree(q);
    TEST_PASS();
}

void TestBasicRowUpdate(ZdbDatabase* db)
{

    /* Legal wants Bob's official name changed to Robert in the DB */
    int i = 1;
    UpdateRowTestHelper(db, 0, 0, "1", 1, "Robert");

    /* John gets rehired */
    i = 3;
    int active = 1;
    UpdateRowTestHelper(db, 0, 0, "3", 4, &active);

    /* Jane got older */
    i = 2;
    int age = 46;
    UpdateRowTestHelper(db, 0, 0, "2", 2, &age);

    /* And I got a raise! */
    i = 0;
    float salary = 80000;
    UpdateRowTestHelper(db, 0, 0, "0", 3, &salary);

}


int main (int argc, const char * argv[])
{
    if (ZdbTypeInitialize() != ZDB_RESULT_SUCCESS)
    {
        printf("Could not initialize type system\n");
    }

    TestTypeSystem();

    ZdbDatabase* db = CreateTestDatabase();

    TestBasicQuery(db);

    TestOneConditionQuery(db, 0, ZDB_QUERY_CONDITION_EQ, "1", ZdbStandardTypes->intType);
    TestOneConditionQuery(db, 1, ZDB_QUERY_CONDITION_EQ, "Jane", ZdbStandardTypes->varcharType);
    TestOneConditionQuery(db, 2, ZDB_QUERY_CONDITION_EQ, "30", ZdbStandardTypes->intType);
    TestOneConditionQuery(db, 3, ZDB_QUERY_CONDITION_EQ, "34000.000000", ZdbStandardTypes->floatType);
    TestOneConditionQuery(db, 4, ZDB_QUERY_CONDITION_EQ, "0", ZdbStandardTypes->booleanType);

    /* Tests multiple results */
    TestOneConditionQuery(db, 4, ZDB_QUERY_CONDITION_EQ, "1", ZdbStandardTypes->booleanType);

    TestBasicRowUpdate(db);

    ZdbEngineDropDB(db);

    return 0;
}

