#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "query.h"
#include "query_condition.h"



#include <string>
#include <tuple>


namespace memdb {
    class Query;
    class QueryCondition;

    class QueryParser {
       
    public:
        // QueryParser() = default;
        std::tuple<Query, QueryCondition> split_into_query_cond(std::string str); //по сути самое сложное
        //TODO: разбить на запрос и его условие(грубо говоря по слову "where")
        // дальше из условия построить дерево condition'ов
        //query прсто составить надо
        
    };

} // namespace memdb

#endif // QUERY_PARSER_H
