#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <query.h>
class QueryParser {
public:
    Query parse(const std::string& input) {
        if (input.empty()) {
            throw std::invalid_argument("Empty query string.");
        }

        std::istringstream stream(input);
        std::string keyword;
        stream >> keyword;
        keyword = to_lower_case(keyword);

        if (keyword == "select") {
            return parse_select(stream);
        } else if (keyword == "insert" || keyword == "update" || keyword == "delete") {
            throw std::invalid_argument("This parser currently supports SELECT queries only.");
        } else {
            throw std::invalid_argument("Unsupported query type: " + keyword);
        }
    }

private:
    Query parse_select(std::istringstream& stream) {
        std::string columns_str, table, join_keyword, join_table, on_keyword, join_condition, where_keyword, where_str;

        // Extract columns
        std::getline(stream, columns_str, 'F'); // Read until "FROM"
        columns_str = trim(columns_str);
        std::vector<std::string> columns = split_by_comma(columns_str);
        if (columns.empty()) {
            throw std::invalid_argument("Invalid SELECT query: missing columns.");
        }

        // Extract table after "FROM"
        stream.ignore(4); // Skip "FROM"
        stream >> table;
        if (table.empty()) {
            throw std::invalid_argument("Invalid SELECT query: missing table.");
        }

        // Check for optional JOIN
        std::vector<std::string> join_parts;
        if (stream.peek() != EOF) {
            stream >> join_keyword;
            if (to_lower_case(join_keyword) == "join") {
                stream >> join_table >> on_keyword;
                on_keyword = to_lower_case(on_keyword);
                if (join_table.empty() || on_keyword != "on") {
                    throw std::invalid_argument("Invalid SELECT query: malformed JOIN clause.");
                }
                std::getline(stream, join_condition, 'W'); // Read until "WHERE"
                join_parts.push_back("join " + join_table + " on " + trim(join_condition));
            } else {
                // If not JOIN, revert stream position for WHERE handling
                stream.putback(join_keyword.back());
            }
        }

        // Extract WHERE clause
        std::vector<std::string> where_parts;
        if (stream.peek() != EOF) {
            std::getline(stream, where_keyword, ' '); // Read "WHERE"
            if (to_lower_case(where_keyword) == "where") {
                std::getline(stream, where_str); // Read the rest of the WHERE clause
                where_parts = split_by_comma(trim(where_str));
            }
        }

        return Query("select", columns, table, join_parts, where_parts);
    }

    std::string to_lower_case(const std::string& input) const {
        std::string result;
        std::transform(input.begin(), input.end(), std::back_inserter(result), [](unsigned char c) {
            return std::tolower(c);
        });
        return result;
    }

    std::string trim(const std::string& str) const {
        const char* whitespace = " \t\n\r\f\v";
        size_t start = str.find_first_not_of(whitespace);
        if (start == std::string::npos) {
            return ""; // All whitespace
        }
        size_t end = str.find_last_not_of(whitespace);
        return str.substr(start, end - start + 1);
    }

    std::vector<std::string> split_by_comma(const std::string& str) const {
        std::vector<std::string> result;
        std::istringstream stream(str);
        std::string token;
        while (std::getline(stream, token, ',')) {
            result.push_back(trim(token));
        }
        return result;
    }
};

#endif // QUERY_PARSER_H
