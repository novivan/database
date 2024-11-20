#ifndef DATABASE_H
#define DATABASE_H

#include <unordered_map>
#include <string>
#include "table.h"

namespace memdb {

    class Database {
    public:
        std::unordered_map<std::string, Table> tables;

        void create_table(const std::string& name, const TableSchema& schema) {
            if (tables.find(name) != tables.end()) {
                throw std::runtime_error("Table already exists");
            }
            tables[name] = Table{schema};
        }

        // TODO: Реализовать методы drop_table, execute
    };

} // namespace memdb

#endif // DATABASE_H
