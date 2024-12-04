#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <sstream>
#include <memory>
#include <stack>
#include <stdexcept>
#include <cassert>

#include "table.h"
#include "query_parser.h"



int select_counter = 0;

std::function<bool(const Line&)> parse_select_condition(const std::string& condition);
std::function<bool(const Line&, const Line&)> parse_join_condition(const std::string& condition, const std::string& table1_name, const std::string& table2_name);
std::unordered_map<std::string, std::shared_ptr<Cell>> dump_map(std::map<std::string, std::string> values);
std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> parse_transformations(const std::map<std::string, std::string>& transformations);

class Database
{
public:
    std::unordered_map<std::string, Table> tables;

    QueryParser parser;

    Database() = default;

    void clear() 
    {
        tables.clear();
    }

    void readFromFile(const std::string& csv_filename)
    {
        // Очищаем текущую базу данных
        clear();

        std::ifstream file(csv_filename);
        if (!file.is_open()) 
        {
            throw std::runtime_error("Could not open file");
        }

        std::string line;
        std::getline(file, line);
        int numTables = std::stoi(line);

        for (int t = 0; t < numTables; ++t) 
        {
            // Читаем заголовок таблицы
            std::getline(file, line);
            std::istringstream ss(line);
            std::string tableName;
            std::getline(ss, tableName, ',');

            std::string numColumnsStr, numRowsStr;
            std::getline(ss, numColumnsStr, ',');
            int numColumns = std::stoi(numColumnsStr);
            std::getline(ss, numRowsStr);
            int numRows = std::stoi(numRowsStr);

            std::vector<std::pair<std::string, int>> columns;
            for (int c = 0; c < numColumns; ++c) 
            {
                std::string columnName;
                int columnType;
                std::getline(ss, columnName, ',');
                std::string columnTypeStr;
                std::getline(ss, columnTypeStr, ',');
                columnType = std::stoi(columnTypeStr);
                columns.emplace_back(columnName, columnType);
            }

            createTable(tableName, columns);

            // Читаем данные таблицы
            for (int r = 0; r < numRows; ++r) 
            {
                std::getline(file, line);
                std::istringstream dataStream(line);
                Line dataLine;
                for (const auto& [columnName, columnType] : columns) 
                {
                    std::string cellData;
                    std::getline(dataStream, cellData, ',');
                    if (columnType == 0) {
                        dataLine.addCell(columnName, std::make_shared<CellInt>(std::stoi(cellData)));
                    } else if (columnType == 1) {
                        dataLine.addCell(columnName, std::make_shared<CellBool>(cellData == "1"));
                    } else if (columnType == 2) {
                        dataLine.addCell(columnName, std::make_shared<CellString>(cellData));
                    } else if (columnType == 3) {
                        std::string bytes = std::string(cellData.begin(), cellData.end());
                        /*for (size_t i = 0; i < cellData.size(); i += 2) {
                            bytes.push_back(cellData[i]);
                        }*/
                        dataLine.addCell(columnName, std::make_shared<CellBytes>(bytes));
                    }
                }
                insert(tableName, dataLine);
            }
            // Пропускаем пустую строку между таблицами
            std::getline(file, line);
        }

        file.close();
    }

    void saveToFile(const std::string& filename) 
    {
        std::ofstream file(filename);
        if (!file.is_open()) 
        {
            throw std::runtime_error("Could not open file");
        }

        // Записываем количество таблиц
        file << tables.size() << "\n";

        for (const auto& [tableName, table] : tables) 
        {
            // Записываем заголовок таблицы
            file << tableName << "," << table.columns.size() << "," << table.columns.begin()->second.cells.size() << "\n";

            for (const auto& [columnName, column] : table.columns) 
            {
                file << columnName << "," << column.type << ",";
            }
            file << "\n";

            // Записываем данные таблицы
            size_t numRows = table.columns.begin()->second.cells.size();
            for (size_t i = 0; i < numRows; ++i) 
            {
                for (const auto& [columnName, column] : table.columns) 
                {
                    if (column.type == 0) {
                        file << std::static_pointer_cast<CellInt>(column.cells[i])->data;
                    } else if (column.type == 1) {
                        file << std::static_pointer_cast<CellBool>(column.cells[i])->data;
                    } else if (column.type == 2) {
                        file << std::static_pointer_cast<CellString>(column.cells[i])->data;
                    } else if (column.type == 3) {
                        auto bytes = std::static_pointer_cast<CellBytes>(column.cells[i])->data;
                        file << std::string(bytes.begin(), bytes.end());
                        /*for (auto byte : bytes) {
                            file << static_cast<int>(byte);
                        }*/
                    }
                    file << ",";
                }
                file << "\n";
            }
            file << "\n"; // Разделяем таблицы пустой строкой
        }

        file.close();
    }

    /* Database(const std::string& csv_filename) 
    {
        readFromFile(csv_filename);
    }*/

    ~Database() = default;

    void printTable(const std::string& tableName) 
    {
        if (tables.find(tableName) == tables.end()) 
        {
            throw std::invalid_argument("Table not found: " + tableName);
        }
        tables.at(tableName).printTable();
    }

    Table& createTable(const std::string tableName, const std::vector <std::pair<std::string, int>>& colums)
    {   
        tables[tableName] = Table(tableName);
        for (auto& [columnName, columnType] : colums) 
        {
            tables[tableName].addColumn(columnName, columnType);
        }
        return tables[tableName];
    }

    Table& select(const std::string& newTablename, const std::string& tableName, std::vector <std::string> columnNames, const std::function<bool(const Line&)>& condition)
    {
        return tables[newTablename] = tables.at(tableName).select(newTablename, columnNames, condition);
    }

    Table& insert(const std::string& tableName, Line& line)
    {
        tables.at(tableName).insert(line);
        return tables[tableName];
    }

    Table& update(const std::string& tableName, const std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>>& transformations, const std::function<bool(const Line&)>& condition)
    {
        tables.at(tableName).update(transformations, condition);
        return tables[tableName];
    }

    Table& remove(const std::string& tableName, const std::function<bool(const Line&)>& condition)
    {
        tables.at(tableName).remove(condition);
        return tables[tableName];
    }

    Table& join(const std::string& newTableName, const std::string& tableName1, const std::string& tableName2, const std::function<bool(const Line&, const Line&)>& condition)
    {   
        if (tables.find(tableName1) == tables.end()) {
            throw std::invalid_argument("Table not found: " + tableName1);
        }
        if (tables.find(tableName2) == tables.end()) {
            throw std::invalid_argument("Table not found: " + tableName2);
        }
        return tables[newTableName] = tables.at(tableName1).join(newTableName, tables.at(tableName2), condition);
    }

    
    Table& translate_n_execute(std::string query) {
        std::unique_ptr<Query> qry;
        try {
            qry = parser.parse(query);
        } catch (const std::exception& e) {
            std::cout << "Invalid query: " << e.what() << "\n";
            return tables[""];
        }

        std::vector<const std::type_info*> QueryTypes(4);
        QueryTypes[0] = &typeid(SelectQuery);
        QueryTypes[1] = &typeid(InsertQuery);
        QueryTypes[2] = &typeid(UpdateQuery);
        QueryTypes[3] = &typeid(DeleteQuery);

        int query_type = std::find(QueryTypes.begin(), QueryTypes.end(), &typeid(*qry)) - QueryTypes.begin();
        if (query_type == 0) { // SELECT
            std::unique_ptr<SelectQuery> select_query = std::make_unique<SelectQuery>(std::move(qry));
            if (select_query->joins.empty()) {
                return select("Select_number_" + std::to_string(select_counter++), select_query->table, select_query->columns, parse_select_condition(select_query->where_conditions));
            } else {
                join(select_query->joins[0].table1 + "&" + select_query->joins[0].table2, select_query->joins[0].table1, select_query->joins[0].table2, parse_join_condition(select_query->joins[0].condition, select_query->joins[0].table1, select_query->joins[0].table2));
                return select("Select_number_" + std::to_string(select_counter++), select_query->joins[0].table1 + "&" + select_query->joins[0].table2, select_query->columns, parse_select_condition(select_query->where_conditions));
            }
        } else if (query_type == 1) { // INSERT
            std::unique_ptr<InsertQuery> insert_query = std::make_unique<InsertQuery>(std::move(qry));
            std::unordered_map<std::string, std::shared_ptr<Cell>> values = dump_map(insert_query->values);
            Line insertline(values);
            insert(insert_query->table, insertline);
            return tables[insert_query->table];
        } else if (query_type == 2) { // UPDATE
            std::unique_ptr<UpdateQuery> update_query = std::make_unique<UpdateQuery>(std::move(qry));
            std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> transformations = parse_transformations(update_query->assignments);
            update(update_query->table, transformations, parse_select_condition(update_query->where_conditions));
            return tables[update_query->table];
        } else if (query_type == 3) { // DELETE
            std::unique_ptr<DeleteQuery> delete_query = std::make_unique<DeleteQuery>(std::move(qry));
            remove(delete_query->table, parse_select_condition(delete_query->where_conditions));
            return tables[delete_query->table];
        } else {
            throw std::runtime_error("Неизвестный тип запроса");
        }
    }

   
    
};


std::unordered_map<std::string, std::shared_ptr<Cell>> dump_map(std::map<std::string, std::string> values) {
    std::unordered_map<std::string, std::shared_ptr<Cell>> result;
    for (auto& [key, value] : values) {
        if (value == "true" || value == "false") {
            result[key] = std::make_shared<CellBool>(value == "true");
        } else if (value.front() == '\'' && value.back() == '\'') {
            result[key] = std::make_shared<CellString>(value.substr(1, value.size() - 2));
        } else {
            result[key] = std::make_shared<CellInt>(std::stoi(value));
        }
    }
    return result;
}


// Функция для преобразования строки в дерево условий для двух Line
std::function<bool(const Line&, const Line&)> parse_join_condition(const std::string& condition, const std::string& table1_name, const std::string& table2_name) {
    // Функция для вычисления значения выражения
    auto evaluate_expression = [table1_name, table2_name](const std::string& expr, const Line& line1, const Line& line2) -> std::string {
        std::istringstream stream(expr);
        std::stack<std::string> values;
        std::stack<char> ops;

        auto apply_op = [](const std::string& a, const std::string& b, char op) -> std::string {
            if (a.front() == '\'' && a.back() == '\'' && b.front() == '\'' && b.back() == '\'') {
                // Обработка строковых операций
                std::string str_a = a.substr(1, a.size() - 2);
                std::string str_b = b.substr(1, b.size() - 2);
                switch (op) {
                    case '+': return "'" + str_a + str_b + "'";
                    default: throw std::invalid_argument("Unsupported operator for strings");
                }
            } else {
                // Обработка числовых операций
                int int_a = std::stoi(a);
                int int_b = std::stoi(b);
                switch (op) {
                    case '+': return std::to_string(int_a + int_b);
                    case '-': return std::to_string(int_a - int_b);
                    case '*': return std::to_string(int_a * int_b);
                    case '/': return std::to_string(int_a / int_b);
                    default: throw std::invalid_argument("Unsupported operator for integers");
                }
            }
        };

        auto precedence = [](char op) -> int {
            switch (op) {
                case '+':
                case '-': return 1;
                case '*':
                case '/': return 2;
                default: return 0;
            }
        };

        auto process_operator = [&]() {
            std::string b = values.top(); values.pop();
            std::string a = values.top(); values.pop();
            char op = ops.top(); ops.pop();
            values.push(apply_op(a, b, op));
        };

        std::string token;
        while (stream >> token) {
            if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
                values.push(token);
            } else if (token.front() == '\'' && token.back() == '\'') {
                values.push(token);
            } else if (token[0] == '(') {
                ops.push('(');
            } else if (token[0] == ')') {
                while (!ops.empty() && ops.top() != '(') {
                    process_operator();
                }
                ops.pop();
            } else if (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/') {
                while (!ops.empty() && precedence(ops.top()) >= precedence(token[0])) {
                    process_operator();
                }
                ops.push(token[0]);
            } else {
                // Предполагаем, что это имя столбца с префиксом таблицы или без него
                auto dot_pos = token.find('.');
                std::string table_name, column_name;
                if (dot_pos != std::string::npos) {
                    table_name = token.substr(0, dot_pos);
                    column_name = token.substr(dot_pos + 1);
                } else {
                    column_name = token;
                }

                auto cell = (table_name == table1_name || (table_name.empty() && line1.cells.count(column_name))) ? line1.cells.at(column_name) : line2.cells.at(column_name);
                if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                    values.push(std::to_string(cellInt->data));
                } else if (auto cellString = std::dynamic_pointer_cast<CellString>(cell)) {
                    values.push("'" + cellString->data + "'");
                } else {
                    throw std::invalid_argument("Unsupported cell type in expression");
                }
            }
        }

        while (!ops.empty()) {
            process_operator();
        }

        return values.top();
    };

    // Функция для парсинга условий
    return [condition, evaluate_expression, table1_name, table2_name](const Line& line1, const Line& line2) -> bool {
        std::istringstream stream(condition);
        std::stack<bool> values;
        std::stack<std::string> ops;

        auto apply_op = [](bool a, bool b, const std::string& op) -> bool {
            if (op == "&&") return a && b;
            if (op == "||") return a || b;
            throw std::invalid_argument("Unsupported logical operator");
        };

        auto precedence = [](const std::string& op) -> int {
            if (op == "||") return 1;
            if (op == "&&") return 2;
            return 0;
        };

        auto process_operator = [&]() {
            bool b = values.top(); values.pop();
            bool a = values.top(); values.pop();
            std::string op = ops.top(); ops.pop();
            values.push(apply_op(a, b, op));
        };

        std::string token;
        while (stream >> token) {
            if (token == "&&" || token == "||") {
                while (!ops.empty() && precedence(ops.top()) >= precedence(token)) {
                    process_operator();
                }
                ops.push(token);
            } else if (token == "(") {
                ops.push("(");
            } else if (token == ")") {
                while (!ops.empty() && ops.top() != "(") {
                    process_operator();
                }
                ops.pop();
            } else {
                // Предполагаем, что это условие вида "table1.col op value" или "value op table2.col"
                std::string column, op, value;
                std::istringstream cond_stream(token);
                cond_stream >> column >> op >> value;

                // Удаляем кавычки из значения, если они есть
                if (!value.empty() && value.front() == '\'' && value.back() == '\'') {
                    value = value.substr(1, value.size() - 2);
                }

                auto dot_pos = column.find('.');
                std::string table_name, column_name;
                if (dot_pos != std::string::npos) {
                    table_name = column.substr(0, dot_pos);
                    column_name = column.substr(dot_pos + 1);
                } else {
                    column_name = column;
                }

                auto cell = (table_name == table1_name || (table_name.empty() && line1.cells.count(column_name))) ? line1.cells.at(column_name) : line2.cells.at(column_name);
                bool result = false;

                if (op == "=") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data == std::stoi(value);
                    } else if (auto cellBool = std::dynamic_pointer_cast<CellBool>(cell)) {
                        result = cellBool->data == (value == "true" || value == "1");
                    } else if (auto cellString = std::dynamic_pointer_cast<CellString>(cell)) {
                        result = cellString->data == value;
                    } else if (auto cellBytes = std::dynamic_pointer_cast<CellBytes>(cell)) {
                        result = cellBytes->data == value;
                    }
                } else if (op == "!=") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data != std::stoi(value);
                    } else if (auto cellBool = std::dynamic_pointer_cast<CellBool>(cell)) {
                        result = cellBool->data != (value == "true" || value == "1");
                    } else if (auto cellString = std::dynamic_pointer_cast<CellString>(cell)) {
                        result = cellString->data != value;
                    } else if (auto cellBytes = std::dynamic_pointer_cast<CellBytes>(cell)) {
                        result = cellBytes->data != value;
                    }
                } else if (op == "<") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data < std::stoi(value);
                    }
                } else if (op == "<=") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data <= std::stoi(value);
                    }
                } else if (op == ">") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data > std::stoi(value);
                    }
                } else if (op == ">=") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data >= std::stoi(value);
                    }
                } else {
                    throw std::invalid_argument("Unsupported operation or type in condition: " + condition);
                }

                values.push(result);
            }
        }

        while (!ops.empty()) {
            process_operator();
        }

        return values.top();
    };
}

// Функция для преобразования строки в дерево условий для where(select, update, delete)
std::function<bool(const Line&)> parse_select_condition(const std::string& condition) {
    // Функция для вычисления значения выражения
    auto evaluate_expression = [](const std::string& expr, const Line& line) -> std::string {
        std::istringstream stream(expr);
        std::stack<std::string> values;
        std::stack<char> ops;

        auto apply_op = [](const std::string& a, const std::string& b, char op) -> std::string {
            if (a.front() == '\'' && a.back() == '\'' && b.front() == '\'' && b.back() == '\'') {
                // Обработка строковых операций
                std::string str_a = a.substr(1, a.size() - 2);
                std::string str_b = b.substr(1, b.size() - 2);
                switch (op) {
                    case '+': return "'" + str_a + str_b + "'";
                    default: throw std::invalid_argument("Unsupported operator for strings");
                }
            } else {
                // Обработка числовых операций
                int int_a = std::stoi(a);
                int int_b = std::stoi(b);
                switch (op) {
                    case '+': return std::to_string(int_a + int_b);
                    case '-': return std::to_string(int_a - int_b);
                    case '*': return std::to_string(int_a * int_b);
                    case '/': return std::to_string(int_a / int_b);
                    default: throw std::invalid_argument("Unsupported operator for integers");
                }
            }
        };

        auto precedence = [](char op) -> int {
            switch (op) {
                case '+':
                case '-': return 1;
                case '*':
                case '/': return 2;
                default: return 0;
            }
        };

        auto process_operator = [&]() {
            std::string b = values.top(); values.pop();
            std::string a = values.top(); values.pop();
            char op = ops.top(); ops.pop();
            values.push(apply_op(a, b, op));
        };

        std::string token;
        while (stream >> token) {
            if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
                values.push(token);
            } else if (token.front() == '\'' && token.back() == '\'') {
                values.push(token);
            } else if (token[0] == '(') {
                ops.push('(');
            } else if (token[0] == ')') {
                while (!ops.empty() && ops.top() != '(') {
                    process_operator();
                }
                ops.pop();
            } else if (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/') {
                while (!ops.empty() && precedence(ops.top()) >= precedence(token[0])) {
                    process_operator();
                }
                ops.push(token[0]);
            } else {
                // Предполагаем, что это имя столбца
                auto cell = line.cells.at(token);
                if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                    values.push(std::to_string(cellInt->data));
                } else if (auto cellString = std::dynamic_pointer_cast<CellString>(cell)) {
                    values.push("'" + cellString->data + "'");
                } else {
                    throw std::invalid_argument("Unsupported cell type in expression");
                }
            }
        }

        while (!ops.empty()) {
            process_operator();
        }

        return values.top();
    };

    // Функция для парсинга условий
    return [condition, evaluate_expression](const Line& line) -> bool {
        std::istringstream stream(condition);
        std::stack<bool> values;
        std::stack<std::string> ops;

        auto apply_op = [](bool a, bool b, const std::string& op) -> bool {
            if (op == "&&") return a && b;
            if (op == "||") return a || b;
            throw std::invalid_argument("Unsupported logical operator");
        };

        auto precedence = [](const std::string& op) -> int {
            if (op == "||") return 1;
            if (op == "&&") return 2;
            return 0;
        };

        auto process_operator = [&]() {
            bool b = values.top(); values.pop();
            bool a = values.top(); values.pop();
            std::string op = ops.top(); ops.pop();
            values.push(apply_op(a, b, op));
        };

        std::string token;
        while (stream >> token) {
            if (token == "&&" || token == "||") {
                while (!ops.empty() && precedence(ops.top()) >= precedence(token)) {
                    process_operator();
                }
                ops.push(token);
            } else if (token == "(") {
                ops.push("(");
            } else if (token == ")") {
                while (!ops.empty() && ops.top() != "(") {
                    process_operator();
                }
                ops.pop();
            } else {
                // Предполагаем, что это условие вида "col op value"
                std::string column, op, value;
                std::istringstream cond_stream(token);
                cond_stream >> column >> op >> value;

                // Удаляем кавычки из значения, если они есть
                if (!value.empty() && value.front() == '\'' && value.back() == '\'') {
                    value = value.substr(1, value.size() - 2);
                }

                auto cell = line.cells.at(column);
                bool result = false;

                if (op == "=") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data == std::stoi(value);
                    } else if (auto cellBool = std::dynamic_pointer_cast<CellBool>(cell)) {
                        result = cellBool->data == (value == "true" || value == "1");
                    } else if (auto cellString = std::dynamic_pointer_cast<CellString>(cell)) {
                        result = cellString->data == value;
                    } else if (auto cellBytes = std::dynamic_pointer_cast<CellBytes>(cell)) {
                        result = cellBytes->data == value;
                    }
                } else if (op == "!=") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data != std::stoi(value);
                    } else if (auto cellBool = std::dynamic_pointer_cast<CellBool>(cell)) {
                        result = cellBool->data != (value == "true" || value == "1");
                    } else if (auto cellString = std::dynamic_pointer_cast<CellString>(cell)) {
                        result = cellString->data != value;
                    } else if (auto cellBytes = std::dynamic_pointer_cast<CellBytes>(cell)) {
                        result = cellBytes->data != value;
                    }
                } else if (op == "<") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data < std::stoi(value);
                    }
                } else if (op == "<=") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data <= std::stoi(value);
                    }
                } else if (op == ">") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data > std::stoi(value);
                    }
                } else if (op == ">=") {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        result = cellInt->data >= std::stoi(value);
                    }
                } else {
                    throw std::invalid_argument("Unsupported operation or type in condition: " + condition);
                }

                values.push(result);
            }
        }

        while (!ops.empty()) {
            process_operator();
        }

        return values.top();
    };
}




std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> parse_transformations(const std::map<std::string, std::string>& transformations) {
    auto evaluate_expression = [](const std::string& expr, const std::shared_ptr<Cell>& cell) -> std::shared_ptr<Cell> {
        std::istringstream stream(expr);
        std::stack<std::string> values;
        std::stack<char> ops;

        auto apply_op = [](const std::string& a, const std::string& b, char op) -> std::string {
            if (a.front() == '\'' && a.back() == '\'' && b.front() == '\'' && b.back() == '\'') {
                // Обработка строковых операций
                std::string str_a = a.substr(1, a.size() - 2);
                std::string str_b = b.substr(1, b.size() - 2);
                switch (op) {
                    case '+': return "'" + str_a + str_b + "'";
                    default: throw std::invalid_argument("Unsupported operator for strings");
                }
            } else {
                // Обработка числовых операций
                int int_a = std::stoi(a);
                int int_b = std::stoi(b);
                switch (op) {
                    case '+': return std::to_string(int_a + int_b);
                    case '-': return std::to_string(int_a - int_b);
                    case '*': return std::to_string(int_a * int_b);
                    case '/': return std::to_string(int_a / int_b);
                    default: throw std::invalid_argument("Unsupported operator for integers");
                }
            }
        };

        auto precedence = [](char op) -> int {
            switch (op) {
                case '+':
                case '-': return 1;
                case '*':
                case '/': return 2;
                default: return 0;
            }
        };

        auto process_operator = [&]() {
            std::string b = values.top(); values.pop();
            std::string a = values.top(); values.pop();
            char op = ops.top(); ops.pop();
            values.push(apply_op(a, b, op));
        };

        std::string token;
        while (stream >> token) {
            if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
                values.push(token);
            } else if (token.front() == '\'' && token.back() == '\'') {
                values.push(token);
            } else if (token[0] == '(') {
                ops.push('(');
            } else if (token[0] == ')') {
                while (!ops.empty() && ops.top() != '(') {
                    process_operator();
                }
                ops.pop();
            } else if (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/') {
                while (!ops.empty() && precedence(ops.top()) >= precedence(token[0])) {
                    process_operator();
                }
                ops.push(token[0]);
            } else {
                // Предполагаем, что это имя столбца или строковое значение
                if (token.front() == '\'' && token.back() == '\'') {
                    values.push(token);
                } else {
                    if (auto cellInt = std::dynamic_pointer_cast<CellInt>(cell)) {
                        values.push(std::to_string(cellInt->data));
                    } else if (auto cellString = std::dynamic_pointer_cast<CellString>(cell)) {
                        values.push("'" + cellString->data + "'");
                    } else {
                        throw std::invalid_argument("Unsupported cell type in expression");
                    }
                }
            }
        }

        while (!ops.empty()) {
            process_operator();
        }

        std::string result = values.top();
        if (result.front() == '\'' && result.back() == '\'') {
            // Обработка строкового результата
            return std::make_shared<CellString>(result.substr(1, result.size() - 2));
        } else {
            // Обработка числового результата
            return std::make_shared<CellInt>(std::stoi(result));
        }
    };

    std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> result;
    for (const auto& [column, expr] : transformations) {
        result[column] = [expr, evaluate_expression](std::shared_ptr<Cell> cell) -> std::shared_ptr<Cell> {
            return evaluate_expression(expr, cell);
        };
    }

    return result;
}
