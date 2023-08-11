# mysql-component-uuid_v7

Extending MySQL using the Component Infrastructure - Adding a UUID v7 generator user function.

```
 MySQL > install component "file://component_uuid_v7";
 Query OK, 0 rows affected (0.0005 sec)
 
 MySQL > select uuid_v7();
+--------------------------------------+
| uuid_v7()                            |
+--------------------------------------+
| 0186c285-2890-79d1-a8f0-6229ba440ade |
+--------------------------------------+
1 row in set (0.0008 sec)

 MySQL > select uuid_v7_to_timestamp("0186c285-2890-79d1-a8f0-6229ba440ade");
+--------------------------------------------------------------+
| uuid_v7_to_timestamp("0186c285-2890-79d1-a8f0-6229ba440ade") |
+--------------------------------------------------------------+
| 2023-03-08 21:39:12.016                                      |
+--------------------------------------------------------------+
1 row in set (0.0003 sec)

 MySQL > select uuid_v7_to_timestamp("0189e190-8e91-7d81-9f8d-d9a1edbf955a");
+--------------------------------------------------------------+
| uuid_v7_to_timestamp("0189e190-8e91-7d81-9f8d-d9a1edbf955a") |
+--------------------------------------------------------------+
| 2023-08-11 01:28:14.609                                      |
+--------------------------------------------------------------+
1 row in set (0.0005 sec)

 MySQL > select uuid_v7_to_unixtime("0189e190-8e91-7d81-9f8d-d9a1edbf955a");
+-------------------------------------------------------------+
| uuid_v7_to_unixtime("0189e190-8e91-7d81-9f8d-d9a1edbf955a") |
+-------------------------------------------------------------+
|                                                  1691706494 |
+-------------------------------------------------------------+
1 row in set (0.0006 sec)

 MySQL > select from_unixtime(uuid_v7_to_unixtime("0189e190-8e91-7d81-9f8d-d9a1edbf955a"));
+----------------------------------------------------------------------------+
| from_unixtime(uuid_v7_to_unixtime("0189e190-8e91-7d81-9f8d-d9a1edbf955a")) |
+----------------------------------------------------------------------------+
| 2023-08-11 01:28:14                                                        |
+----------------------------------------------------------------------------+
1 row in set (0.0006 sec)
 
 MySQL > select from_unixtime(uuid_v7_to_unixtime(uuid_v7()));
+-----------------------------------------------+
| from_unixtime(uuid_v7_to_unixtime(uuid_v7())) |
+-----------------------------------------------+
| 2023-08-11 11:33:22                           |
+-----------------------------------------------+
1 row in set (0.0005 sec)
 
 MySQL > select uuid_v7_to_timestamp_long("0189e190-8e91-7d81-9f8d-d9a1edbf955a");
+-------------------------------------------------------------------+
| uuid_v7_to_timestamp_long("0189e190-8e91-7d81-9f8d-d9a1edbf955a") |
+-------------------------------------------------------------------+
| Fri Aug 11 01:28:14 2023 GMT                                      |
+-------------------------------------------------------------------+
1 row in set (0.0007 sec) 

 MySQL > select uuid_v7_to_timestamp("0189e190-8e91-4d81-9f8d-d9a1edbf955a");
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; This is not a UUID v7

 MySQL > select uuid_v7_to_timestamp("0189e190-8e91-4d81-9f8d-d9a1edbf955a1");
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; Invalid UUID string length

 MySQL > select uuid_v7_to_timestamp("aaa");
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; Invalid UUID string length

 MySQL > select uuid_v7_to_timestamp("0189e190-8e91-4d81-9f8d-d9a1edbf955a1",1);
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; this function requires only 1 parameteter

 MySQL > select uuid_v7_to_timestamp();
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; this function requires 1 parameteter
```

## How to use a UUID v7 as primary key on a table

```
 MySQL > create table t1 (uuid varbinary(16) primary key, 
         name varchar(20), created timestamp default current_timestamp());

 MySQL > insert into t1 (uuid, name) values (UUID_TO_BIN(uuid_v7()), "fred");
 MySQL > insert into t1 (uuid, name) values (UUID_TO_BIN(uuid_v7()), "scott");
 MySQL > insert into t1 (uuid, name) values (UUID_TO_BIN(uuid_v7()), "lenka");

 MySQL > select * from t1;
+------------------------------------+-------+---------------------+
| uuid                               | name  | created             |
+------------------------------------+-------+---------------------+
| 0x0189E3CEE2A677819EE703856F8135C0 | fred  | 2023-08-11 11:55:33 |
| 0x0189E3CEF8027A58951D745F60AF0068 | scott | 2023-08-11 11:55:39 |
| 0x0189E3CF84BB766EAB94A674678379DC | lenka | 2023-08-11 11:56:15 |
+------------------------------------+-------+---------------------+
3 rows in set (0.0007 sec)

 MySQL > select BIN_TO_UUID(uuid), name, created from t1;
+--------------------------------------+-------+---------------------+
| BIN_TO_UUID(uuid)                    | name  | created             |
+--------------------------------------+-------+---------------------+
| 0189e3ce-e2a6-7781-9ee7-03856f8135c0 | fred  | 2023-08-11 11:55:33 |
| 0189e3ce-f802-7a58-951d-745f60af0068 | scott | 2023-08-11 11:55:39 |
| 0189e3cf-84bb-766e-ab94-a674678379dc | lenka | 2023-08-11 11:56:15 |
+--------------------------------------+-------+---------------------+
3 rows in set (0.0003 sec)

 MySQL > select BIN_TO_UUID(uuid), created, 
       uuid_v7_to_timestamp_long(BIN_TO_UUID(uuid)) uuid_ts from t1;
+--------------------------------------+---------------------+------------------------------+
| BIN_TO_UUID(uuid)                    | created             | uuid_ts                      |
+--------------------------------------+---------------------+------------------------------+
| 0189e3ce-e2a6-7781-9ee7-03856f8135c0 | 2023-08-11 11:55:33 | Fri Aug 11 11:55:33 2023 GMT |
| 0189e3ce-f802-7a58-951d-745f60af0068 | 2023-08-11 11:55:39 | Fri Aug 11 11:55:39 2023 GMT |
| 0189e3cf-84bb-766e-ab94-a674678379dc | 2023-08-11 11:56:15 | Fri Aug 11 11:56:15 2023 GMT |
+--------------------------------------+---------------------+------------------------------+

 MySQL > explain select * from t1 
    where uuid=UUID_TO_BIN('0189e3ce-f802-7a58-951d-745f60af0068')\G
*************************** 1. row ***************************
           id: 1
  select_type: SIMPLE
        table: t1
   partitions: NULL
         type: const
possible_keys: PRIMARY
          key: PRIMARY
      key_len: 18
          ref: const
         rows: 1
     filtered: 100
        Extra: NULL
1 row in set, 1 warning (0.0011 sec)

 MySQL > explain format=tree select * from t1 
    where uuid=UUID_TO_BIN('0189e3ce-f802-7a58-951d-745f60af0068')\G
*************************** 1. row ***************************
EXPLAIN: -> Rows fetched before execution  (cost=0..0 rows=1

 MySQL > explain format=json select * from t1 
    where uuid=UUID_TO_BIN('0189e3ce-f802-7a58-951d-745f60af0068')\G
*************************** 1. row ***************************
EXPLAIN: {
  "query_block": {
    "select_id": 1,
    "cost_info": {
      "query_cost": "1.00"
    },
    "table": {
      "table_name": "t1",
      "access_type": "const",
      "possible_keys": [
        "PRIMARY"
      ],
      "key": "PRIMARY",
      "used_key_parts": [
        "uuid"
      ],
      "key_length": "18",
      "ref": [
        "const"
      ],
      "rows_examined_per_scan": 1,
      "rows_produced_per_join": 1,
      "filtered": "100.00",
      "cost_info": {
        "read_cost": "0.00",
        "eval_cost": "0.10",
        "prefix_cost": "0.00",
        "data_read_per_join": "104"
      },
      "used_columns": [
        "uuid",
        "name",
        "created"
      ]
    }
  }
}

 MySQL > ALTER table t1 add column uuid_text char(36) 
         generated always as (BIN_TO_UUID(uuid)) VIRTUAL;

 MySQL > select * from t1 ;
+------------------------------------+-------+---------------------+--------------------------------------+
| uuid                               | name  | created             | uuid_text                            |
+------------------------------------+-------+---------------------+--------------------------------------+
| 0x0189E3CEE2A677819EE703856F8135C0 | fred  | 2023-08-11 11:55:33 | 0189e3ce-e2a6-7781-9ee7-03856f8135c0 |
| 0x0189E3CEF8027A58951D745F60AF0068 | scott | 2023-08-11 11:55:39 | 0189e3ce-f802-7a58-951d-745f60af0068 |
| 0x0189E3CF84BB766EAB94A674678379DC | lenka | 2023-08-11 11:56:15 | 0189e3cf-84bb-766e-ab94-a674678379dc |
+------------------------------------+-------+---------------------+--------------------------------------+         
```

The only caveat is that such a component function cannot be used
as default value for any column.
