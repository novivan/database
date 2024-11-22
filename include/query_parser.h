#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "query.h"
#include "query_condition.h"



#include <string>
#include <tuple>
#include <regex>

namespace memdb {
    class Query;
    class QueryCondition;

    class QueryParser {
       
    public:
        // QueryParser() = default;
        QueryCondition condition_tree(const std::string& condition_str);
        Query parse_query(const std::string& str);
        std::string to_lower_case(const std::string& input){
            std::string result = input;
            std::vector<std::string> keywords = std::vector<std::string>();
            keywords.push_back("SELECT");
            keywords.push_back("FROM");
            keywords.push_back("WHERE");

            for (const auto& keyword : keywords) {
                size_t pos = 0;
                while ((pos = result.find(keyword, pos)) != std::string::npos) {
                    std::string lowercase_keyword = keyword;
                    for (auto& ch : lowercase_keyword) {
                        ch = std::tolower(ch);
                    }
                    result.replace(pos, keyword.size(), lowercase_keyword);
                    pos += lowercase_keyword.size(); 
                }
            }
            return result;
        }
        std::tuple<Query, QueryCondition> split_into_query_cond(std::string str){
            std::string normal_str = to_lower_case(str);

            size_t where_pos = normal_str.find("where");

            std::string query_part = str.substr(0, where_pos);
            std::string condition_part = str.substr(where_pos + 5);

            Query query = parse_query(query_part); // эту функцию надо написать)

            QueryCondition condition = condition_tree(condition_part); // это тоже надо написать)
            
            return std::make_tuple(query, condition);

        } //по сути самое сложное
        //TODO: разбить на запрос и его условие(грубо говоря по слову "where")
        // дальше из условия построить дерево condition'ов
        //query прсто составить надо
        
    };

} // namespace memdb

#endif // QUERY_PARSER_H
