#ifndef DATABASE_H
#define DATABASE_H

#include <unordered_map>
#include <string>
#include "table.h"
#include "query_parser.h"



namespace memdb {

    class Database {
    public:
        
        std::unordered_map<std::string, Table> tables;

        QueryParser parser;

        void create_table(const std::string& name, const TableSchema& schema) {
            if (tables.find(name) != tables.end()) {
                throw std::runtime_error("Table already exists");
            }
            tables[name] = (Table){schema};
        }

        void translate_n_execute(std::string query) {
            Query qry;
            std::shared_ptr<QueryCondition> cond;
            std::tie(qry, cond) = parser.split_into_query_cond(query);

            // для всего, что выдает query нужно проверить условие (вот пример из main):
            /*
                db.tables["users"].select(condition_select);
                по сути, просто команду применить
            */
            // и выдать новую таблицу
            
        }

        // TODO: Реализовать методы drop_table, execute
    };

} // namespace memdb

#endif // DATABASE_H
