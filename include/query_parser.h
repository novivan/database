#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

/*
#include "query.h"
#include "query_condition.h"

#include <string>
#include <tuple>
#include <regex>
#include <vector>
#include <map>



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

        }



class Query;
class QueryCondition;

struct ParsedQuery {
    std::string queryType;                 // Тип запроса: SELECT, INSERT, DELETE, и т.д.
    std::string targetTable;               // Целевая таблица
    std::vector<std::string> columns;      // Колонки для выборки или вставки
    std::vector<std::string> values;       // Значения для вставки (INSERT)
    std::string condition;                 // Условие фильтрации (WHERE)
};



class QueryParser {
    
public:
    QueryCondition condition_tree(const std::string& condition_str);
    Query parse_query(const std::string& str);

    //Кусок Андрея
    ParsedQuery parseQuery(const std::string& query);

    const std::vector<std::string>& getErrors() const;

    QueryParser() = default;

private:
    bool validateSyntax(const std::string& query);

    std::vector<std::string> errors;

    ParsedQuery handleSelect(const std::string& query);
    ParsedQuery handleInsert(const std::string& query);
    ParsedQuery handleDelete(const std::string& query);
    ParsedQuery handleCreateTable(const std::string& query);

    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);
    
};

*/
#endif // QUERY_PARSER_H

//КОД АНДРЕЯ
/*
#include "query_parser.h"
#include <sstream>
#include <algorithm>
#include <cctype>

std::string QueryParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> QueryParser::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

ParsedQuery QueryParser::parseQuery(const std::string& query) {
    errors.clear();
    ParsedQuery result;

    std::string trimmedQuery = trim(query);

    if (trimmedQuery.substr(0, 6) == "SELECT") {
        result = handleSelect(trimmedQuery);
    } else if (trimmedQuery.substr(0, 11) == "INSERT INTO") {
        result = handleInsert(trimmedQuery);
    } else if (trimmedQuery.substr(0, 6) == "DELETE") {
        result = handleDelete(trimmedQuery);
    } else if (trimmedQuery.substr(0, 12) == "CREATE TABLE") {
        result = handleCreateTable(trimmedQuery);
    } else {
        errors.push_back("Unknown query type.");
    }

    return result;
}


const std::vector<std::string>& QueryParser::getErrors() const {
    return errors;
}

ParsedQuery QueryParser::handleSelect(const std::string& query) {
    ParsedQuery parsedQuery;
    parsedQuery.queryType = "SELECT";

    size_t fromPos = query.find(" FROM ");
    size_t wherePos = query.find(" WHERE ");

    if (fromPos == std::string::npos) {
        errors.push_back("Syntax error: missing 'FROM' in SELECT query.");
        return parsedQuery;
    }

    std::string columnsPart = query.substr(7, fromPos - 7); // "name, grade"
    parsedQuery.columns = split(columnsPart, ',');

    if (wherePos != std::string::npos) {
        parsedQuery.targetTable = trim(query.substr(fromPos + 6, wherePos - (fromPos + 6)));
        parsedQuery.condition = trim(query.substr(wherePos + 7)); // "grade > 80"
    } else {
        parsedQuery.targetTable = trim(query.substr(fromPos + 6));
    }

    return parsedQuery;
}

ParsedQuery QueryParser::handleInsert(const std::string& query) {
    ParsedQuery parsedQuery;
    parsedQuery.queryType = "INSERT";

    size_t valuesPos = query.find(" VALUES ");
    if (valuesPos == std::string::npos) {
        errors.push_back("Syntax error: missing 'VALUES' in INSERT query.");
        return parsedQuery;
    }

    size_t tableStart = 12;
    size_t tableEnd = query.find('(', tableStart);
    if (tableEnd == std::string::npos) {
        errors.push_back("Syntax error: missing '(' after table name in INSERT query.");
        return parsedQuery;
    }

    parsedQuery.targetTable = trim(query.substr(tableStart, tableEnd - tableStart));

    size_t columnsEnd = query.find(')', tableEnd);
    if (columnsEnd == std::string::npos) {
        errors.push_back("Syntax error: missing ')' after column list in INSERT query.");
        return parsedQuery;
    }
    std::string columnsPart = query.substr(tableEnd + 1, columnsEnd - tableEnd - 1);
    parsedQuery.columns = split(columnsPart, ',');

    size_t valuesStart = query.find('(', valuesPos);
    size_t valuesEnd = query.find(')', valuesStart);
    if (valuesStart == std::string::npos || valuesEnd == std::string::npos) {
        errors.push_back("Syntax error: missing parentheses in VALUES part of INSERT query.");
        return parsedQuery;
    }
    std::string valuesPart = query.substr(valuesStart + 1, valuesEnd - valuesStart - 1);
    parsedQuery.values = split(valuesPart, ',');

    return parsedQuery;
}
*/