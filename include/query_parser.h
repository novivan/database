#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H


#include <sstream>
#include <stdexcept>
#include <memory>
#include <algorithm>

#include "query.h"
#include "query_condition.h"



class QueryParser {
public:
    QueryParser() = default;

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
        } else if (query_type == "create") {
            return parse_create(stream);
        } else {
            throw std::invalid_argument("Unsupported query type: " + query_type);
        }
    }

private:
    std::unique_ptr<CreateQuery> parse_create(std::istringstream& stream) {
        std::string table_keyword, table_name;
        stream >> table_keyword >> table_name;

        if (to_lower_case(table_keyword) != "table") {
            throw std::invalid_argument("Invalid CREATE query");
        }

        std::vector<std::pair<std::string, int>> columns;
        std::string column_def;
        while (std::getline(stream, column_def, ',')) {
            column_def.erase(std::remove_if(column_def.begin(), column_def.end(), ::isspace), column_def.end());
            size_t pos = column_def.find(':');
            if (pos == std::string::npos) {
                throw std::invalid_argument("Invalid column definition");
            }
            std::string column_name = column_def.substr(0, pos);
            std::string column_type_str = column_def.substr(pos + 1);
            int column_type;

            //column_type_str = to_lower_case(column_type_str);

            if (column_type_str == "int32") {
                column_type = 0;
            } else if (column_type_str.find("string[") == 0) {
                column_type = 2;
            } else if (column_type_str.find("bytes[") == 0) {
                column_type = 3;
            } else if (column_type_str == "bool" || column_type_str == "bool=true" || column_type_str == "bool=false") {
                column_type = 1;
            } else {
                throw std::invalid_argument("Unknown column type: " + column_type_str);
            }

            columns.emplace_back(column_name, column_type);
        }

        auto query = std::make_unique<CreateQuery>();
        query->set_table(table_name);
        query->set_columns(columns);

        return query;
    }
    std::unique_ptr<SelectQuery> parse_select(std::istringstream& stream) {
        std::string columns_part, from_keyword, table_or_join, where_keyword, condition;

        std::getline(stream, columns_part, ' ');
        while (to_lower_case(columns_part) != "from") {
            std::string next_part;
            stream >> next_part;
            columns_part += " " + next_part;
        }
        columns_part = columns_part.substr(0, columns_part.size() - 5);
        columns_part = trim(columns_part);
        std::vector<std::string> columns = split_by_comma(columns_part);

        std::getline(stream, table_or_join);
        table_or_join = trim(table_or_join);

        auto query = std::make_unique<SelectQuery>();
        query->set_columns(columns);

        if (to_lower_case(table_or_join).find("join") != std::string::npos) {
            parse_join(stream, *query);
        } else {
            query->set_table(table_or_join);
        }

        if (stream >> where_keyword) {
            if (to_lower_case(where_keyword) != "where") {
                throw std::invalid_argument("Unexpected keyword in SELECT query: " + where_keyword);
            }
            std::getline(stream, condition);
            query->set_where(trim(condition));
        }

        return query;
    }

    std::vector<std::string> split_by_comma(const std::string& str) {
        std::vector<std::string> result;
        std::istringstream stream(str);
        std::string token;
        while (std::getline(stream, token, ',')) {
            result.push_back(trim(token));
        }
        return result;
    }

    std::unique_ptr<Query> parse_insert(std::istringstream& stream) {
        std::string into_keyword, table;
        stream >> into_keyword;

        if (to_lower_case(into_keyword) != "into") {
            throw std::invalid_argument("INSERT query missing 'INTO' keyword.");
        }

        stream >> table;

        std::string columns_part, values_part;
        std::getline(stream, columns_part, ')');
        stream >> into_keyword;
        if(to_lower_case(into_keyword) != "values") {
            throw std::invalid_argument("INSERT query missing 'VALUES' keyword.");
        }

        std::getline(stream, values_part, ')');

        columns_part = trim(columns_part);
        values_part = trim(values_part);

        if (columns_part.find('(') == std::string::npos || values_part.find('(') == std::string::npos) {
            throw std::invalid_argument("Invalid INSERT syntax. Expected (columns) VALUES (values).");
        }

        columns_part = columns_part.substr(columns_part.find('(') + 1);
        values_part = values_part.substr(values_part.find('(') + 1);

        std::vector<std::string> columns = split_by_comma(columns_part);
        std::vector<std::string> values = split_by_comma(values_part);

        if (columns.size() != values.size()) {
            throw std::invalid_argument("Number of columns does not match number of values.");
        }

        std::map<std::string, std::string> column_value_map;
        for (size_t i = 0; i < columns.size(); ++i) {
            column_value_map[trim(columns[i])] = remove_quotes_and_commas(trim(values[i]));
        }

        auto query = std::make_unique<InsertQuery>();
        query->set_table(table);
        query->set_values(column_value_map);

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