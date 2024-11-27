#include <gtest/gtest.h>
#include "database.h"

Database createTestDatabase() {
    Database db;
    db.createTable("users", {{"id", 0}, {"is_admin", 1}, {"login", 2}, {"password_hash", 3}});

    Line line1;
    line1.addCell("id", std::make_shared<CellInt>(1));
    line1.addCell("is_admin", std::make_shared<CellBool>(false));
    line1.addCell("login", std::make_shared<CellString>("ivan"));
    line1.addCell("password_hash", std::make_shared<CellBytes>(std::vector<uint8_t>{0xde, 0xad, 0xbe, 0xef}));
    db.insert("users", line1);

    Line line2;
    line2.addCell("id", std::make_shared<CellInt>(2));
    line2.addCell("is_admin", std::make_shared<CellBool>(true));
    line2.addCell("login", std::make_shared<CellString>("admin"));
    line2.addCell("password_hash", std::make_shared<CellBytes>(std::vector<uint8_t>{0xca, 0xfe, 0xba, 0xbe}));
    db.insert("users", line2);

    return db;
}

TEST(DatabaseTests, CreateTable) {
    Database db;
    db.createTable("users", {{"id", 0}, {"login", 1}, {"password_hash", 2}});

    ASSERT_TRUE(db.tables.find("users") != db.tables.end());
    ASSERT_EQ(db.tables["users"].columns.size(), 3);
}

// тесты для INSERT
TEST(DatabaseTests, Insert) {
    Database db = createTestDatabase();
    Line newLine;
    newLine.addCell("id", std::make_shared<CellInt>(3));
    newLine.addCell("is_admin", std::make_shared<CellBool>(false));
    newLine.addCell("login", std::make_shared<CellString>("new_user"));
    newLine.addCell("password_hash", std::make_shared<CellBytes>(std::vector<uint8_t>{0xbe, 0xef, 0xca, 0xfe}));

    db.insert("users", newLine);

    ASSERT_EQ(db.tables["users"].columns["id"].cells.size(), 3);
    ASSERT_EQ(std::static_pointer_cast<CellString>(db.tables["users"].columns["login"].cells[2])->data, "new_user");
}

TEST(DatabaseTests, Insert_With_Missing_Values) {
    Database db = createTestDatabase();
    Line newLine;
    newLine.addCell("id", std::make_shared<CellInt>(3));

    ASSERT_THROW(db.insert("users", newLine), std::invalid_argument);
}

TEST(DatabaseTests, Insert_With_Null_Values) {
    Database db = createTestDatabase();
    Line newLine;
    newLine.addCell("id", std::make_shared<CellInt>(3));
    newLine.addCell("is_admin", std::make_shared<CellBool>(false));
    newLine.addCell("login", nullptr);
    newLine.addCell("password_hash", nullptr);

    ASSERT_NO_THROW(db.insert("users", newLine));
    ASSERT_EQ(db.tables["users"].columns["id"].cells.size(), 3);
}

TEST(DatabaseTests, Insert_Invalid_Column) {
    Database db = createTestDatabase();
    Line newLine;
    newLine.addCell("nonexistent_column", std::make_shared<CellString>("value"));

    ASSERT_THROW(db.insert("users", newLine), std::invalid_argument);
}

// тесты для UPDATE
TEST(DatabaseTests, Update) {
    Database db = createTestDatabase();
    std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> transformations = {
        {"login", [](std::shared_ptr<Cell> cell) {
            auto strCell = std::static_pointer_cast<CellString>(cell);
            return std::make_shared<CellString>(strCell->data + "_updated");
        }}
    };

    db.update("users", transformations, [](const Line& line) {
        return std::static_pointer_cast<CellString>(line.cells.at("login"))->data == "ivan";
    });

    ASSERT_EQ(std::static_pointer_cast<CellString>(db.tables["users"].columns["login"].cells[0])->data, "ivan_updated");
}

TEST(DatabaseTests, Update_Without_Where) {
    Database db = createTestDatabase();
    std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> transformations = {
        {"is_admin", [](std::shared_ptr<Cell> cell) {
            return std::make_shared<CellBool>(true);
        }}
    };

    db.update("users", transformations, [](const Line&) { return true; });

    ASSERT_TRUE(std::static_pointer_cast<CellBool>(db.tables["users"].columns["is_admin"].cells[0])->data);
    ASSERT_TRUE(std::static_pointer_cast<CellBool>(db.tables["users"].columns["is_admin"].cells[1])->data);
}

TEST(DatabaseTests, Update_With_2_Where) {
    Database db = createTestDatabase();
    std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> transformations = {
        {"login", [](std::shared_ptr<Cell> cell) {
            auto strCell = std::static_pointer_cast<CellString>(cell);
            return std::make_shared<CellString>("updated_" + strCell->data);
        }}
    };

    db.update("users", transformations, [](const Line& line) {
        int id = std::static_pointer_cast<CellInt>(line.cells.at("id"))->data;
        return id == 2;
    });

    ASSERT_EQ(std::static_pointer_cast<CellString>(db.tables["users"].columns["login"].cells[1])->data, "updated_admin");
}

// тесты для DELETE
TEST(DatabaseTests, Delete) {
    Database db = createTestDatabase();
    db.remove("users", [](const Line& line) {
        return std::static_pointer_cast<CellString>(line.cells.at("login"))->data == "admin";
    });

    ASSERT_EQ(db.tables["users"].columns["id"].cells.size(), 1);
    ASSERT_EQ(std::static_pointer_cast<CellString>(db.tables["users"].columns["login"].cells[0])->data, "ivan");
}

TEST(DatabaseTests, Delete_Without_Where) {
    Database db = createTestDatabase();
    db.remove("users", [](const Line&) { return true; });

    ASSERT_EQ(db.tables["users"].columns["id"].cells.size(), 0);
}

TEST(DatabaseTests, Delete_Invalid_Column) {
    Database db = createTestDatabase();

    ASSERT_THROW(db.remove("users", [](const Line& line) {
        return std::static_pointer_cast<CellString>(line.cells.at("nonexistent_column"))->data == "value";
    }), std::out_of_range);
}

TEST(DatabaseTests, Delete_With_2_Where) {
    Database db = createTestDatabase();
    db.remove("users", [](const Line& line) {
        int id = std::static_pointer_cast<CellInt>(line.cells.at("id"))->data;
        return id > 1;
    });

    ASSERT_EQ(db.tables["users"].columns["id"].cells.size(), 1);
    ASSERT_EQ(std::static_pointer_cast<CellString>(db.tables["users"].columns["login"].cells[0])->data, "ivan");
}

// тесты для SELECT
TEST(DatabaseTests, Select) {
    Database db = createTestDatabase();
    auto selected = db.select("selected_users", "users", {"id", "login"}, [](const Line& line) {
        return std::static_pointer_cast<CellBool>(line.cells.at("is_admin"))->data;
    });

    ASSERT_EQ(db.tables["selected_users"].columns["id"].cells.size(), 1);
    ASSERT_EQ(std::static_pointer_cast<CellString>(db.tables["selected_users"].columns["login"].cells[0])->data, "admin");
}

TEST(DatabaseTests, Select_Without_Where) {
    Database db = createTestDatabase();
    auto selected = db.select("selected_users", "users", {"id", "login"}, [](const Line&) { return true; });

    ASSERT_EQ(db.tables["selected_users"].columns["id"].cells.size(), 2);
    ASSERT_EQ(std::static_pointer_cast<CellString>(db.tables["selected_users"].columns["login"].cells[0])->data, "ivan");
    ASSERT_EQ(std::static_pointer_cast<CellString>(db.tables["selected_users"].columns["login"].cells[1])->data, "admin");
}

TEST(DatabaseTests, Select_With_2_Where) {
    Database db = createTestDatabase();
    auto selected = db.select("selected_users", "users", {"id", "login"}, [](const Line& line) {
        int id = std::static_pointer_cast<CellInt>(line.cells.at("id"))->data;
        bool isAdmin = std::static_pointer_cast<CellBool>(line.cells.at("is_admin"))->data;
        return id > 1 || isAdmin;
    });

    ASSERT_EQ(db.tables["selected_users"].columns["id"].cells.size(), 1);
    ASSERT_EQ(std::static_pointer_cast<CellInt>(db.tables["selected_users"].columns["id"].cells[0])->data, 2);
    
}



