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
    std::stack<std::string> ans = memdb::get_polish_notation("id > 5 AND ( name = 'Vasya' OR age < 18 )");
    std::shared_ptr<memdb::QueryCondition> condition = memdb::build_condition_tree(ans);
    return 0;
}
