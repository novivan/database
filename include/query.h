#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <functional>
#include "row.h"
#include "value.h"

namespace memdb {
    //для удобства
    enum class Operations{
        SELECT, //если что это #define SELECT 0
        //тут нужно выписать все команды(тк SELECT уже есть FROM не надо) 

    };
    
    class Query {
    public:
        Operations type;
        std::vector <std::string> first_operands; //example - select ROW_A, ROW_B, from table_a, table_b (большими буквами то, что идет в переменную)
        std::vector <std::string> second_operands; //example - select row_a, row_b, from TABLE_A, TABLE_B (большими буквами то, что идет в переменную)

        Query() = default;
        Query(Operations & op, std::vector<std::string> &first_param, std::vector <std::string> &sec_param): type(op), first_operands(first_param), second_operands(sec_param){}
        
    };

} // namespace memdb

#endif // QUERY_H
