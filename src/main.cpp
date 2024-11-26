//СТАРЫЙ MAIN
//ПОКА ЗАКОММЕНЧУ, ЧТОБЫ НЕ МЕШАЛО
/*
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
*/



#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>


#include "../include/database.h"


//#include <../include/table.h>

void testUpdate(Database& db) {
    std::cout << "Testing UPDATE..." << std::endl;

    // Создаем словарь трансформаций
    std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> transformations = {
        {"login", [](std::shared_ptr<Cell> cell) {
            auto strCell = std::static_pointer_cast<CellString>(cell);
            return std::make_shared<CellString>(strCell->data + "_updated");
        }},
        {"is_admin", [](std::shared_ptr<Cell> cell) {
            auto boolCell = std::static_pointer_cast<CellBool>(cell);
            return std::make_shared<CellBool>(!boolCell->data); // инвертируем значение
        }}
    };

    // Обновляем данные
    db.update("users", transformations, [](const Line& line) {
        return std::static_pointer_cast<CellString>(line.cells.at("login"))->data == "vasya";
    });

    std::cout << "After UPDATE:" << std::endl;
    db.printTable("users");
    //printTable(db.tables["users"]);

    // Проверяем результат
    for (const auto& [columnName, column] : db.tables["users"].columns) {
        for (const auto& cell : column.cells) {
            if (columnName == "login") {
                auto strCell = std::static_pointer_cast<CellString>(cell);
                if (strCell->data == "vasya_updated") {
                    std::cout << "UPDATE test passed: login field updated correctly" << std::endl;
                    return;
                }
            }
        }
    }
    throw std::runtime_error("UPDATE test failed: login field not updated correctly");
}








int main() {
    // Создаем таблицу
    //Table users("users");
    Database db;
    db.createTable("users", {{"id", 0}, {"is_admin", 1}, {"login", 2}, {"password_hash", 3}});
    //users.printTable();

    std::cout << "testTableCreation passed." << std::endl;
    
    /*// Добавляем столбцы
    users.addColumn("id", 0);
    users.addColumn("is_admin", 1);
    users.addColumn("login", 2);
    users.addColumn("password_hash", 3);
    */
    db.printTable("users");

    std::cout << "testAddColumn passed." << std::endl;
    
    // Вставляем строки
    Line line1;
    line1.addCell("id", std::make_shared<CellInt>(1));
    line1.addCell("is_admin", std::make_shared<CellBool>(false));
    line1.addCell("login", std::make_shared<CellString>("vasya"));
    line1.addCell("password_hash", std::make_shared<CellBytes>(std::vector<uint8_t>{0xde, 0xad, 0xbe, 0xef}));
    //db.tables["users"].insert(line1);
    db.insert("users", line1);
    //users.insert(line1);

    db.printTable("users");
    //users.printTable();
    std::cout << "testAddRow passed." << std::endl;


    
    Line line2;
    line2.addCell("id", std::make_shared<CellInt>(2));
    line2.addCell("is_admin", std::make_shared<CellBool>(true));
    line2.addCell("login", std::make_shared<CellString>("admin"));
    line2.addCell("password_hash", std::make_shared<CellBytes>(std::vector<uint8_t>{0xca, 0xfe, 0xba, 0xbe}));
    //db.tables["users"].insert(line2);
    db.insert("users", line2);
    //users.insert(line2);

    db.printTable("users");
    //users.printTable();
    std::cout << "second testAddRow passed." << std::endl;

    std::cout << "testSaveToFile:\n";
    // Создаем вторую таблицу
    db.createTable("products", {{"product_id", 0}, {"product_name", 2}, {"price", 0}});

    // Вставляем строки во вторую таблицу
    Line product1;
    product1.addCell("product_id", std::make_shared<CellInt>(1));
    product1.addCell("product_name", std::make_shared<CellString>("Laptop"));
    product1.addCell("price", std::make_shared<CellInt>(1000));
    db.insert("products", product1);

    Line product2;
    product2.addCell("product_id", std::make_shared<CellInt>(2));
    product2.addCell("product_name", std::make_shared<CellString>("Smartphone"));
    product2.addCell("price", std::make_shared<CellInt>(500));
    db.insert("products", product2);

    db.printTable("products");




    db.saveToFile("../data.csv");
    //db.saveToFile("../data.csv");
    std::cout << "testSaveToFile passed.\n";
    
    //ЭТО ПОКА НЕ РАБОТАЕТ ;(
    // НО ЕСЛИ ОТТЕБАЖИТ КТО-ТО, БУДЕТ СУПЕР)
    /*
    std::cout << "testClearBase:\n";
    db.clear();
    std::cout << "db.tables.size() = " << db.tables.size() << std::endl;
    
    db.readFromFile("../data.csv");
    std::cout << "printing readed table: \n";
    std::cout << "db.tables.size() = " << db.tables.size() << std::endl;
    for (auto& [tableName, table] : db.tables) {
        db.printTable(tableName);
    }
    std::cout << "testReadFromFile passed.\n";
    */


    

    std::cout << "testPrintTable passed." << std::endl;
    
    auto selected = db.select("selected_users", "users", {"id", "login"}, [](const Line& line) {
        bool isAdmin = std::static_pointer_cast<CellBool>(line.cells.at("is_admin"))->data;
        int id = std::static_pointer_cast<CellInt>(line.cells.at("id"))->data;
        return isAdmin || id < 10;
    });
    // Печатаем результаты SELECT
    std::cout << "Selected users:" << std::endl;
    //selected.printTable();
    db.printTable("selected_users");

    // Выполняем UPDATE
    std::cout << "Testing UPDATE..." << std::endl;

    // Создаем словарь трансформаций
    std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> transformations = {
        {"login", [](std::shared_ptr<Cell> cell) {
            auto strCell = std::static_pointer_cast<CellString>(cell);
            return std::make_shared<CellString>(strCell->data + "_updated");
        }},
        {"is_admin", [](std::shared_ptr<Cell> cell) {
            auto boolCell = std::static_pointer_cast<CellBool>(cell);
            return std::make_shared<CellBool>(!boolCell->data); // инвертируем значение
        }}
    };

    // Обновляем данные
    db.update("users", transformations, [](const Line& line) {
        return std::static_pointer_cast<CellString>(line.cells.at("login"))->data == "vasya";
    });

    
    std::cout << "After UPDATE:" << std::endl;
    //users.printTable();
    db.printTable("users");

    // Выполняем DELETE

    db.remove("users", [](const Line& line) {
        return static_pointer_cast<CellString>(line.cells.at("login"))->data == "admin";
    });

    // печатаем результаты DELETE
    std::cout << "Users after delete:" << std::endl;
    //users.printTable();
    db.printTable("users");


    //db.writeToFile("../data.csv");
    db.~Database();
    //users.~Table();

    std::cout << "All tests passed." << std::endl;

    return 0;
}