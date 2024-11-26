#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <vector>

struct Query {
    std::string type;                    // Тип запроса (например, "select")
    std::vector<std::string> columns;    // Список столбцов
    std::string table;                   // Имя таблицы
    std::vector<std::string> join;       // Условия соединения (JOIN)
    std::vector<std::string> where;      // Условия фильтрации (WHERE)

    // Конструктор с параметрами
    Query(const std::string& query_type,
          const std::vector<std::string>& query_columns,
          const std::string& query_table,
          const std::vector<std::string>& query_join,
          const std::vector<std::string>& query_where)
        : type(query_type), columns(query_columns), table(query_table), join(query_join), where(query_where) {}

    // Конструктор по умолчанию
    Query() : type(""), table("") {}

    // Метод для отладки и вывода данных
    void print() const {
        std::cout << "Query type: " << type << "\n";
        std::cout << "Columns: ";
        for (const auto& col : columns) {
            std::cout << col << " ";
        }
        std::cout << "\nTable: " << table << "\n";
        std::cout << "Join: ";
        for (const auto& j : join) {
            std::cout << j << " ";
        }
        std::cout << "\nWhere: ";
        for (const auto& w : where) {
            std::cout << w << " ";
        }
        std::cout << "\n";
    }
};

#endif // QUERY_H