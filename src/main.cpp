#include <iostream>
#include "../include/database.h"
#include "../include/query_condition.h"

void print_table(const memdb::Table& table) {
    for (const auto& row : table.rows) {
        std::cout << "id: " << std::get<int>(row["id"])
                  << ", name: " << std::get<std::string>(row["name"])
                  << ", age: " << std::get<int>(row["age"]) << std::endl;
    }
    std::cout << "-----------------------" << std::endl;
}

int main() {
    memdb::Database db;

    memdb::TableSchema schema;
    schema.columns = {{"id", "INT32", true}, {"name", "STRING"}, {"age", "INT32"}};
    schema.primary_key = "id";

    db.create_table("users", schema);

    memdb::Row row;
    row["id"] = 1;
    row["name"] = "John Doe";
    row["age"] = 25;
    db.tables["users"].insert(row);

    row["id"] = 2;
    row["name"] = "Jane Doe";
    row["age"] = 30;
    db.tables["users"].insert(row);

    row["id"] = 3;
    row["name"] = "Alice";
    row["age"] = 22;
    db.tables["users"].insert(row);

    print_table(db.tables["users"]);

    // TODO не работает :(
    memdb::QueryCondition condition_select("age", memdb::Operator::GREATER_THAN, 24);

    print_table(db.tables["users"].select(condition_select));

    memdb::QueryCondition condition_update("id", memdb::Operator::EQUAL, 2);
    memdb::Row updated_values;
    updated_values["name"] = "Updated Jane";
    db.tables["users"].update(condition_update, updated_values);

    print_table(db.tables["users"]);

    memdb::QueryCondition condition_delete("age", memdb::Operator::LESS_THAN, 25);
    db.tables["users"].delete_rows(condition_delete);

    print_table(db.tables["users"]);

    return 0;
}
