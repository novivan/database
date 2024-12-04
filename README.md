# database
---
## инфа для проверяющего
### как собрать
```
mkdir build
cd build
cmake ..
make
```
дальше просто запускаем нужную цель сборки
```
./memorydb
./tests
```

---
## инфа для команды

[memdb.pdf](https://github.com/user-attachments/files/17811270/memdb.pdf)

Schema:
/src
    database.h
    database.cpp
    table.h
    table.cpp
    row.h
    row.cpp
    query_parser.h
    query_parser.cpp
    main.cpp
/build
    (папка для сборки)
/tests
    (тесты для компонентов)
CMakeLists.txt
