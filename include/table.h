#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <functional>

#include "column.h"
#include "line.h"

//TODO: добавть smart ptrs
// потому что несколько таблиц могут ссылаться на одни и те же ячейки, и при удалении таблицы,
// вторая таблица будет ссылаться на уже удаленные ячейки

//C этим нужно будет очень внимательно порабоать
// во все файлы в ниже в иерархии (line.h, column.h, cells.h) переписать на умные указатели
class Table 
{
public:
    std::string name;
    std::unordered_map<std::string, Column> columns;

    Table() = default;

    Table(const std::string& tableName) : name(tableName) {
        columns.clear();
    }

    void addColumn(const std::string& columnName, int type) 
    {
        columns[columnName] = Column(type);
    }

    void insert(Line& line) 
    {
        for (auto& [columnName, column] : columns) 
        {
            if (line.cells.find(columnName) != line.cells.end()) 
            {
                column.cells.push_back(line.cells.at(columnName));
            } 
            else 
            {
                throw std::invalid_argument("Missing value for column: " + columnName);
            }
        }
    }

    Table select(const std::string& newTableName, const std::vector<std::string>& columnNames, const std::function<bool(const Line&)>& condition) 
    {
        Table result(newTableName);

        // Добавляем столбцы в результирующую таблицу
        for (const auto& columnName : columnNames) 
        {
            if (columns.find(columnName) != columns.end()) 
            {
                result.addColumn(columnName, columns[columnName].type);
            } 
            else 
            {
                throw std::invalid_argument("Column not found: " + columnName);
            }
        }

        size_t rowCount = columns.begin()->second.cells.size();

        for (size_t i = 0; i < rowCount; ++i) 
        {
            Line line;
            for (const auto& [columnName, column] : columns) 
            {
                line.addCell(columnName, column.cells[i]);
            }
            
            if (condition(line)) 
            {
                Line resultLine;
                for (const auto& columnName : columnNames) 
                {
                    resultLine.addCell(columnName, line.cells[columnName]);
                }
                result.insert(resultLine);
            }
        }
        return result;
    }

    void update(const std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>>& transformations, 
            const std::function<bool(const Line&)>& condition) 
    {
        size_t rowCount = columns.begin()->second.cells.size();

        for (size_t i = 0; i < rowCount; ++i) 
        {
            Line line;
            for (auto& [columnName, column] : columns) 
            {
                line.addCell(columnName, column.cells[i]);
            }
            if (condition(line)) 
            {
                for (auto& [columnName, transform] : transformations) 
                {
                    columns[columnName].cells[i] = transform(columns[columnName].cells[i]);
                }
            }
        }
    }

    void remove(const std::function<bool(const Line&)>& condition) 
    {
        size_t rowCount = columns.begin()->second.cells.size();

        for (size_t i = 0; i < rowCount; ++i) 
        {
            Line line;
            for (auto& [columnName, column] : columns) 
            {
                line.addCell(columnName, column.cells[i]);
            }
            if (condition(line)) 
            {
                for (auto& [columnName, column] : columns) 
                {
                    column.cells[i].reset();
                    //column.cells.erase(column.cells.begin() + i);
                }
                --i;
                --rowCount;
            }
        }
    }
    
    //сейчас нет обработки того, что таблицы можно объединять по совпадающему столбцу
    //кстати, тут создаются новые экземпляры ячеек, а не копируются указатели (это надо исправить)
    Table join(const std::string& newTableName, const Table& other, const std::function<bool(const Line&, const Line&)>& condition) 
    {
        Table result(newTableName);

        for (const auto& [columnName, column] : columns) 
        {
            result.addColumn(name + "." + columnName, column.type);
        }
        for (const auto& [columnName, column] : other.columns) 
        {
            result.addColumn(other.name + "." + columnName, column.type);
        }

        size_t rowCount1 = columns.begin()->second.cells.size();
        size_t rowCount2 = other.columns.begin()->second.cells.size();

        for (size_t i = 0; i < rowCount1; ++i) 
        {
            Line line1;
            for (const auto& [columnName, column] : columns) 
            {
                line1.addCell(name + "." + columnName, column.cells[i]);
            }
            for (size_t j = 0; j < rowCount2; ++j) 
            {
                Line line2;
                for (const auto& [columnName, column] : other.columns) 
                {
                    line2.addCell(other.name + "." + columnName, column.cells[j]);
                }
                if (condition(line1, line2)) 
                {
                    Line joinedLine = line1;
                    for (const auto& [columnName, cell] : line2.cells) 
                    {
                        joinedLine.addCell(columnName, cell);
                    }
                    for (const auto& [columnName, cell] : joinedLine.cells) 
                    {
                        result.columns[columnName].cells.push_back(cell);
                    }
                }
            }
        }

        return result;
    }

    void printTable() {
        std::cout << "Table: " << name << std::endl;
        for (const auto& column : columns) {
            std::cout << column.first << "  ";
            if (column.second.type == 3) {
                std:: cout << "                                                      ";
            }
        }
        std::cout << std::endl;

        size_t numRows = columns.begin()->second.cells.size();
        for (size_t i = 0; i < numRows; ++i) {
            for (const auto& column : columns) {
                if (column.second.cells[i] == nullptr) {
                    std::cerr << "Error: nullptr detected in column '" << column.first << "' at row " << i << std::endl;
                    continue;
                }
                if (column.second.type == 0) {
                    std::cout << std::static_pointer_cast<CellInt>(column.second.cells[i])->data << "\t";
                } else if (column.second.type == 1) {
                    std::cout << std::static_pointer_cast<CellBool>(column.second.cells[i])->data << "\t";
                } else if (column.second.type == 2) {
                    std::cout << std::static_pointer_cast<CellString>(column.second.cells[i])->data << "\t";
                } else if (column.second.type == 3) {
                    auto bytes = std::static_pointer_cast<CellBytes>(column.second.cells[i])->data;
                    for (auto byte : bytes) {
                        //std::cout << std::hex << static_cast<int>(byte);
                        std::cout << byte;
                    }
                    std::cout << "\t";
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl << std::endl << std::endl << std::endl;
    }


    ~Table() = default;
};


