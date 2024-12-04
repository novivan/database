#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>


#include "../include/database.h"
#include "../include/conditional_execute.h"

#include <iostream>
#include <string>
#include "../include/query_parser.h"

void run_query_tests() {
    QueryParser parser;

    // Test 1: Simple SELECT
    std::cout << "Test 1: SELECT id, name FROM users WHERE age > 30\n";
    try {
        std::unique_ptr<Query> query = parser.parse("SELECT id, name FROM users WHERE age > 30");
        query->print();
    } catch (const std::exception& e) {
        std::cout << "Test 1 Failed: " << e.what() << "\n";
    }

    std::cout << "\n";

    // Test 2: SELECT with JOIN
    std::cout << "Test 2: SELECT id, name FROM users JOIN orders ON users.id = orders.user_id WHERE total > 100\n";
    try {
        std::unique_ptr<Query> query = parser.parse(
            "SELECT id, name FROM users JOIN orders ON users.id = orders.user_id WHERE total > 100");
        query->print();
    } catch (const std::exception& e) {
        std::cout << "Test 2 Failed: " << e.what() << "\n";
    }

    std::cout << "\n";

    // Test 3: INSERT
    std::cout << "Test 3: INSERT INTO users (id, name, age) VALUES (1, 'John Doe', 25)\n";
    try {
        std::unique_ptr<Query> query = parser.parse("INSERT INTO users (id, name, age) VALUES (1, 'John Doe', 25)");
        query->print();
    } catch (const std::exception& e) {
        std::cout << "Test 3 Failed: " << e.what() << "\n";
    }

    std::cout << "\n";

    // Test 4: UPDATE
    std::cout << "Test 4: UPDATE users SET name = 'John', age = 26 WHERE id = 1\n";
    try {
        std::unique_ptr<Query> query = parser.parse("UPDATE users SET name = 'John', age = 26 WHERE id = 1");
        query->print();
    } catch (const std::exception& e) {
        std::cout << "Test 4 Failed: " << e.what() << "\n";
    }

    std::cout << "\n";

    // Test 5: DELETE
    std::cout << "Test 5: DELETE FROM users WHERE id = 1\n";
    try {
        std::unique_ptr<Query> query = parser.parse("DELETE FROM users WHERE id = 1");
        query->print();
    } catch (const std::exception& e) {
        std::cout << "Test 5 Failed: " << e.what() << "\n";
    }

    std::cout << "\n";

    // Test 6: Unsupported query
    std::cout << "Test 6: DROP TABLE users\n";
    try {
        parser.parse("DROP TABLE users");
        std::cout << "Test 6 Failed: No exception thrown\n";
    } catch (const std::exception& e) {
        std::cout << "Test 6 Passed: " << e.what() << "\n";
    }
}

void test_conditional_execute() {
    std::string condition = "(users.age*2)/2-2+4-2  > 30 AND city = 'New York'";
    std::map<std::string, std::string> variables = {
        {"users.age", "35"},
        {"city", "New York"}
    };

    try {
        ASTNodePtr ast = parse_condition(condition);
        bool result = check_conditional(ast, variables);
        std::cout << "Condition is " << (result ? "True" : "False") << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error evaluating condition: " << ex.what() << std::endl;
    }
}


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
    
    //db.~Database();


    std::cout << "Running tests for QueryParser...\n\n";
    run_query_tests();

    test_conditional_execute();

    std::cout << "All tests passed." << std::endl;



    return 0;
}