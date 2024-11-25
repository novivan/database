#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <string>
#include <vector>
#include <map>

struct ParsedQuery {
    std::string queryType;                 // Тип запроса: SELECT, INSERT, DELETE, и т.д.
    std::string targetTable;               // Целевая таблица
    std::vector<std::string> columns;      // Колонки для выборки или вставки
    std::vector<std::string> values;       // Значения для вставки (INSERT)
    std::string condition;                 // Условие фильтрации (WHERE)
};

class QueryParser {
public:
    ParsedQuery parseQuery(const std::string& query);

    const std::vector<std::string>& getErrors() const;

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

#endif // QUERY_PARSER_H
