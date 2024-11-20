#ifndef TABLE_H
#define TABLE_H

#include <vector>
#include <unordered_map>
#include "row.h"
#include "table_schema.h"
#include "query_condition.h"

namespace memdb {

    class Table {
    public:
        TableSchema schema;
        std::vector<Row> rows;

        void insert(const Row& row) {
            rows.push_back(row);
        }

        size_t update(const QueryCondition& condition, const Row& new_values) {
            size_t updated_count = 0;
            for (auto& row : rows) {
                if (condition.evaluate(row)) {
                    for (const auto& [key, value] : new_values.values) {
                        row[key] = value;
                    }
                    ++updated_count;
                }
            }
            return updated_count;
        }

        size_t delete_rows(const QueryCondition& condition) {
            size_t initial_size = rows.size();
            rows.erase(
                std::remove_if(rows.begin(), rows.end(),
                               [&](const Row& row) { return condition.evaluate(row); }),
                rows.end());
            return initial_size - rows.size();
        }

        Table select(const QueryCondition& condition) const {
            Table result_table;
            result_table.schema = schema; // Копируем схему

            for (const auto& row : rows) {
                if (condition.evaluate(row)) {
                    result_table.rows.push_back(row); // Добавляем строку в новый объект
                }
            }

            return result_table;
        }
    };

} // namespace memdb

#endif // TABLE_H
