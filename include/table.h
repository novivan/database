#ifndef TABLE_H
#define TABLE_H

#include <vector>
#include <unordered_map>
#include "row.h"
#include "table_schema.h"

namespace memdb {

    class Table {
    public:
        TableSchema schema;
        std::vector<Row> rows;

        void insert(const Row& row) {
            rows.push_back(row);
        }

        // TODO: Реализовать методы update, delete, select
    };

} // namespace memdb

#endif // TABLE_H
