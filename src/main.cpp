#include <iostream>
#include "database.h"

int main() {
    memdb::Database db;

    memdb::TableSchema schema;
    schema.columns = {{"id", "INT32", true}, {"name", "STRING"}};
    schema.primary_key = "id";

    db.create_table("users", schema);

    memdb::Row row;
    row["id"] = 1;
    row["name"] = "John Doe";

    db.tables["users"].insert(row);

    for (const auto& row : db.tables["users"].rows) {
        std::cout << std::get<int>(row["id"]) << " " << std::get<std::string>(row["name"]) << "\n";
    }

    return 0;
}
