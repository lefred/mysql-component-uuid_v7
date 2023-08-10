# mysql-component-uuid_v7

Extending MySQL using the Component Infrastructure - Adding a UUID v7 generator user fonction.

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

 MySQL > select uuid_v7_to_timestamp("0189e190-8e91-4d81-9f8d-d9a1edbf955a");
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; This is not a UUID v7

 MySQL > select uuid_v7_to_timestamp("0189e190-8e91-4d81-9f8d-d9a1edbf955a1");
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; Invalid UUID string length

 MySQL > select uuid_v7_to_timestamp("aaa");
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; Invalid UUID string length

 MySQL > select uuid_v7_to_timestamp("0189e190-8e91-4d81-9f8d-d9a1edbf955a1",1);
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; this function requires only 1 parameteter!

 MySQL > select uuid_v7_to_timestamp();
ERROR: 3200 (HY000): uuid_v7_to_timestamp UDF failed; this function requires 1 parameteter!
```


