# mysql-component-uuid_v7

Extending MySQL using the Component Infrastructure - Adding a UUID v4 generator user fonction.

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
```


