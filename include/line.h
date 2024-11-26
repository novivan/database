#include <unordered_map>
#include <string>
#include <memory>

#include "cells.h"

class Line 
{
public:
    std::unordered_map<std::string, std::shared_ptr<Cell>> cells;

    Line() = default;
    Line(std::unordered_map<std::string, std::shared_ptr<Cell>> cls) : cells(cls) {}
    Line(std::vector<std::string> columnNames) 
    {
        for (const auto& columnName : columnNames) 
        {
            cells[columnName] = nullptr;
        }
    }

    std::shared_ptr<Cell>& operator[](const std::string& columnName) 
    {
        return cells[columnName];
    }

    const std::shared_ptr<Cell> operator[](const std::string& columnName) const 
    {   
        if (cells.find(columnName) == cells.end()) 
        {
            throw std::invalid_argument("Column not found: " + columnName);
        }
        return cells.at(columnName);
    }

    void addCell(const std::string& columnName, std::shared_ptr<Cell> cell) 
    {
        cells[columnName] = cell;
    }

    ~Line() 
    {
        for (auto& [columnName, cell] : cells) 
        {
            cell.reset();
        }
    }
};

