#ifndef TABLE_SCHEMA_H
#define TABLE_SCHEMA_H

#include <string>
#include <vector>

namespace memdb {

    struct Column {
        std::string name;
        std::string type;
        bool is_primary_key = false;
        size_t size = 0; // Для строк фиксированной длины
    };

    class TableSchema {
    public:
        std::vector<Column> columns;
        std::string primary_key;

        Column get_column(const std::string& name) const {
            for (const auto& col : columns) {
                if (col.name == name) {
                    return col;
                }
            }
            throw std::runtime_error("Column not found");
        }
    };

} // namespace memdb

#endif // TABLE_SCHEMA_H
