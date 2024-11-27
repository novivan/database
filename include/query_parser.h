#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "query.h"
#include <sstream>
#include <stdexcept>
#include <memory>
#include <algorithm>

class QueryParser {
public:
    std::unique_ptr<Query> parse(const std::string& query) {
        std::istringstream stream(query);
        std::string query_type;
        stream >> query_type;
        query_type = to_lower_case(query_type);


        if (query_type == "select") {
            return parse_select(stream);
        } else if (query_type == "insert") {
            return parse_insert(stream);
        } else if (query_type == "update") {
            return parse_update(stream);
        } else if (query_type == "delete") {
            return parse_delete(stream);

        } else {
            throw std::invalid_argument("Unsupported query type: " + query_type);
        }
    }

private:
    std::unique_ptr<Query> parse_select(std::istringstream& stream) {
        std::vector<std::string> columns;
        std::string word;

        while (stream >> word) {
            if (to_lower_case(word) == "from") {
                break;
            }
            columns.push_back(remove_quotes_and_commas(word));
        }

        if (columns.empty()) {
            throw std::invalid_argument("SELECT query missing columns.");
        }

        std::string table;
        stream >> table;

        auto query = std::make_unique<SelectQuery>();
        query->set_columns(columns);
        query->set_table(table);

        std::string next_keyword;
        while (stream >> next_keyword) {
            next_keyword = to_lower_case(next_keyword);
            if (next_keyword == "join") {
                parse_join(stream, *query);
            } else if (next_keyword == "where") {
                parse_where(stream, *query);
                break;
            } else {
                throw std::invalid_argument("Unexpected keyword in SELECT query: " + next_keyword);
            }
        }

        return query;
    }

    std::unique_ptr<Query> parse_insert(std::istringstream& stream) {
        std::string into_keyword, table, values_str;
        stream >> into_keyword;

        if (to_lower_case(into_keyword) != "into") {
            throw std::invalid_argument("INSERT query missing 'INTO' keyword.");
        }

        stream >> table;
        std::getline(stream, values_str);

        auto query = std::make_unique<InsertQuery>();
        query->set_table(table);
        query->set_values(parse_values(values_str));

        return query;
    }

    std::string remove_quotes_and_commas(const std::string& str) {
        if (str.empty()) {
            return str;
        }

        size_t start = 0;
        size_t end = str.size() - 1;

        while (start <= end && (str[start] == '\'' || str[start] == '"' || str[start] == ',')) {
            start++;
        }

        while (end >= start && (str[end] == '\'' || str[end] == '"' || str[end] == ',')) {
            end--;
        }

        return str.substr(start, end - start + 1);
    }



    std::unique_ptr<Query> parse_update(std::istringstream& stream) {
        std::string table, word;
        stream >> table;

        auto query = std::make_unique<UpdateQuery>();
        query->set_table(table);

        std::map<std::string, std::string> assignments;
        while (stream >> word) {
            if (to_lower_case(word) == "set") {
                continue;
            }

            if (to_lower_case(word) == "where") {
                parse_where(stream, *query);
                break;
            }

            size_t eq_pos = word.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = trim(word.substr(0, eq_pos));
                std::string value = trim(word.substr(eq_pos + 1));
                assignments[key] = remove_quotes_and_commas(value);
            } else {
                char eq;
                stream >> eq;
                std::string value;
                stream >> value;
                assignments[word] = remove_quotes_and_commas(value);
            }
        }

        if (assignments.empty()) {
            throw std::invalid_argument("UPDATE query missing assignments.");
        }

        query->set_assignments(assignments);

        return query;
    }

    std::unique_ptr<Query> parse_delete(std::istringstream& stream) {
        std::string from_keyword, table;
        stream >> from_keyword;

        if (to_lower_case(from_keyword) != "from") {
            throw std::invalid_argument("DELETE query missing 'FROM' keyword.");
        }

        stream >> table;
        auto query = std::make_unique<DeleteQuery>();
        query->set_table(table);

        std::string next_keyword;
        while (stream >> next_keyword) {
            if (to_lower_case(next_keyword) == "where") {
                parse_where(stream, *query);
                break;
            }
        }

        return query;
    }

    void parse_join(std::istringstream& stream, SelectQuery& query) {
        std::string table2, on_keyword, condition;

        stream >> table2 >> on_keyword;

        if (to_lower_case(on_keyword) != "on") {
            throw std::invalid_argument("JOIN clause missing 'ON' keyword.");
        }

        std::ostringstream condition_stream;
        std::string word;

        while (stream >> word) {
            if (to_lower_case(word) == "where") {
                parse_where(stream, query);
                break;
            }
            condition_stream << word << " ";
        }

        query.set_join(query.table, table2, trim(condition_stream.str()));
    }


    void parse_where(std::istringstream& stream, Query& query) {
        std::ostringstream conditions;
        std::string word;

        while (stream >> word) {
            if (word.back() == ',') {
                word.pop_back();
            }
            conditions << word << " ";
        }

        query.set_where(trim(conditions.str()));
    }

    std::map<std::string, std::string> parse_values(const std::string& values_str) {
        std::map<std::string, std::string> values;
        std::istringstream stream(values_str);
        std::string key_value;
        while (std::getline(stream, key_value, ',')) {
            size_t eq_pos = key_value.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = trim(key_value.substr(0, eq_pos));
                std::string value = trim(key_value.substr(eq_pos + 1));
                values[key] = value;
            }
        }
        return values;
    }

    std::string to_lower_case(const std::string& str) {
        std::string lower_str = str;
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
        return lower_str;
    }

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
    }
};

#endif // QUERY_PARSER_H