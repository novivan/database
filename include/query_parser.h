#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "query.h"
#include <string>
#include <sstream>
#include <map>
#include <stdexcept>

class QueryParser {
public:
    Query parse(const std::string& input) {
        if (input.empty()) {
            throw std::invalid_argument("Empty query string.");
        }

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

        // Read columns
        if (!std::getline(stream, columns, ' ')) {
            throw std::invalid_argument("Invalid SELECT query: missing columns.");
        }

        // Read FROM and table
        stream >> from >> table;
        if (from != "from" || table.empty()) {
            throw std::invalid_argument("Invalid SELECT query: missing FROM or table.");
        }

        // Check for optional JOIN
        if (stream.peek() != EOF) {
            std::string join_keyword;
            stream >> join_keyword;
            if (join_keyword == "join") {
                std::string join_table, on_keyword, join_condition;
                stream >> join_table >> on_keyword;
                if (join_table.empty() || on_keyword != "on") {
                    throw std::invalid_argument("Invalid SELECT query: malformed JOIN clause.");
                }
                std::getline(stream, join_condition);
                join_clause = "join " + join_table + " on " + join_condition;
            }
        }

        // Read WHERE clause
        std::getline(stream, where_clause);

        return Query("select", {
            {"columns", columns},
            {"table", table},
            {"join", join_clause},
            {"where", where_clause}
        });
    }

    Query parse_insert(std::istringstream& stream) {
        std::string into, table, values_clause;

        stream >> into >> table;
        if (into != "into" || table.empty()) {
            throw std::invalid_argument("Invalid INSERT query: missing INTO or table.");
        }

        if (!std::getline(stream, values_clause)) {
            throw std::invalid_argument("Invalid INSERT query: missing values.");
        }

        return Query("insert", {
            {"table", table},
            {"values", values_clause}
        });
    }

    Query parse_update(std::istringstream& stream) {
        std::string table, set_clause, where_clause;

        stream >> table;
        if (table.empty()) {
            throw std::invalid_argument("Invalid UPDATE query: missing table.");
        }

        if (!std::getline(stream, set_clause, ' ') || set_clause.empty()) {
            throw std::invalid_argument("Invalid UPDATE query: missing SET clause.");
        }

        std::getline(stream, where_clause);

        return Query("update", {
            {"table", table},
            {"set", set_clause},
            {"where", where_clause}
        });
    }

    Query parse_delete(std::istringstream& stream) {
        std::string from, table, where_clause;

        stream >> from >> table;
        if (from != "from" || table.empty()) {
            throw std::invalid_argument("Invalid DELETE query: missing FROM or table.");
        }

        std::getline(stream, where_clause);

        return Query("delete", {
            {"table", table},
            {"where", where_clause}
        });
    }

    std::string to_lower_case(const std::string& input) const {
        std::string result;
        for (char c : input) {
            result += std::tolower(c);
        }
        return result;
    }
};

#endif // QUERY_PARSER_H
