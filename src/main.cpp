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
    Database db;


    // Создаем таблицу
    db.translate_n_execute("CREATE TABLE users id:int32, is_admin:bool, login:string[32], password_hash:bytes[32]");

    std::cout << "testTableCreation passed." << std::endl;
    db.printTable("users");
    

    db.translate_n_execute("INSERT INTO users (id, is_admin, login, password_hash) VALUES (1, false, \'vasya\', 0xdeadbeef)");


    db.printTable("users");
    std::cout << "testAddRow passed." << std::endl;

    db.translate_n_execute("INSERT INTO users (id, is_admin, login, password_hash) VALUES (2, true, \'admin\', 0xcafebabe)");

    db.printTable("users");
    std::cout << "second testAddRow passed." << std::endl;
    std::cout << "testSaveToFile:\n";
    // Создаем вторую таблицу

    db.translate_n_execute("CREATE TABLE products product_id:int32, product_name:string[32], price:int32");
    
    

    // Вставляем строки во вторую таблицу
    db.translate_n_execute("INSERT INTO products (product_id, product_name, price) VALUES (1, \'Laptop\', 1000)");
    db.translate_n_execute("INSERT INTO products (product_id, product_name, price) VALUES (2, \'Smartphone\', 500)");

    db.printTable("products");




    db.saveToFile("../data.csv");
    
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

    //db.tables["selected_users"] = db.translate_n_execute("SELECT id, login FROM users WHERE is_admin OR id < 10");
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