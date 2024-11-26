#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "query.h"
#include "query_condition.h"
#include <string>
#include <regex>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>


class QueryParser {
public:
    Query parse(const std::string& input) {
        std::string lower_input = to_lower_case(input);
        std::istringstream stream(lower_input);
        std::string keyword;
        stream >> keyword;

        if (keyword == "select") {
            return parse_select(stream);
        } else if (keyword == "insert") {
            return parse_insert(stream);
        } else if (keyword == "update") {
            return parse_update(stream);
        } else if (keyword == "delete") {
            return parse_delete(stream);
        } else {
            throw std::invalid_argument("Unsupported query type: " + keyword);
        }
    }

private:
    Query parse_select(std::istringstream& stream) {
        std::string columns, from, table, join_clause, where_clause;
        std::getline(stream, columns, ' ');
        stream >> from >> table;

        if (stream.peek() != EOF) {
            std::string join_keyword;
            stream >> join_keyword;
            if (join_keyword == "join") {
                std::string join_table, on_keyword, join_condition;
                stream >> join_table >> on_keyword;
                std::getline(stream, join_condition);
                join_clause = "join " + join_table + " on " + join_condition;
            }
        }

        std::getline(stream, where_clause);

        return Query{
            "select",
            {{"columns", columns}, {"table", table}, {"join", join_clause}, {"where", where_clause}}
        };
    }

    Query parse_insert(std::istringstream& stream) {
        std::string into, table, values_clause;
        stream >> into >> table;
        std::getline(stream, values_clause);

        return Query{
            "insert",
            {{"table", table}, {"values", values_clause}}
        };
    }

    Query parse_update(std::istringstream& stream) {
        std::string table, set_clause, where_clause;
        stream >> table;
        std::getline(stream, set_clause, ' ');
        std::getline(stream, where_clause);

        return Query{
            "update",
            {{"table", table}, {"set", set_clause}, {"where", where_clause}}
        };
    }

    Query parse_delete(std::istringstream& stream) {
        std::string from, table, where_clause;
        stream >> from >> table;
        std::getline(stream, where_clause);

        return Query{
            "delete",
            {{"table", table}, {"where", where_clause}}
        };
    }

    std::string to_lower_case(const std::string& input) {
        std::string result;
        for (char c : input) {
            result += std::tolower(c);
        }
        return result;
    }
};

#endif // QUERY_PARSER_H
