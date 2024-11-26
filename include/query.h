#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <functional>
#include "row.h"
#include "value.h"

namespace memdb {
    //для удобства
    //для удобства
    char
    *Create = "create", 
    *Table = "table",
    *Select = "select",
    *Insert = "insert",
    *Update = "update",
    *Delete = "delete",
    *Join = "join",
    *Index = "index",
    *From = "from",
    *Where = "where",
    *Primary = "primary",
    *Key = "key",
    *Unique = "unique",
    *Autoincrement = "autoincrement",
    *To = "to",
    *Set = "set",
    *On = "on",
    *By = "by",
    *Int = "int32",
    *Bool = "bool",
    *String = "string",
    *Bytes = "bytes";

    std::vector <char *> Keywords = 
    {Create, Table, Select, Insert, Update, Delete, Join, Index,
    From, Where, Primary, Key, Unique, Autoincrement, To, Set,
    On, By, Int, Bool, String, Bytes};
    
    class Query {
    public:
        char* type;
        std::vector <std::string> first_operands; //example - select ROW_A, ROW_B, from table_a, table_b (большими буквами то, что идет в переменную)
        std::vector <std::string> second_operands; //example - select row_a, row_b, from TABLE_A, TABLE_B (большими буквами то, что идет в переменную)

        Query() = default;
        Query(std::string & op, std::vector<std::string> &first_param, std::vector <std::string> &sec_param): type(op), first_operands(first_param), second_operands(sec_param){}
        
    };

} // namespace memdb

#endif // QUERY_H
