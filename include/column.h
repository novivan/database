#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

#include "cells.h"

/*
std::vector<const std::type_info*> CellTypes(4);
        CellTypes[0] = &typeid(CellInt);
        CellTypes[1] =  &typeid(CellBool);
        CellTypes[2] =  &typeid(CellString);
        CellTypes[3] =  &typeid(CellBytes);
*/

class Column 
{
public:
    int type; //смотрите выше
    std::vector <std::shared_ptr<Cell>> cells;

    bool is_key, is_unique, is_autoincrement;

    Column() : type(0), is_key(false), is_unique(false), is_autoincrement(false) {}

    Column (int tp): type(tp){}

    Column(Column *other): type(other->type), is_key(other->is_key), is_unique(other->is_unique), is_autoincrement(other->is_autoincrement){
        for (const auto& cell : other->cells) {
            if (type == 0) {
                cells.push_back(std::make_shared<CellInt>(*std::static_pointer_cast<CellInt>(cell)));
            } else if (type == 1) {
                cells.push_back(std::make_shared<CellBool>(*std::static_pointer_cast<CellBool>(cell)));
            } else if (type == 2) {
                cells.push_back(std::make_shared<CellString>(*std::static_pointer_cast<CellString>(cell)));
            } else if (type == 3) {
                cells.push_back(std::make_shared<CellBytes>(*std::static_pointer_cast<CellBytes>(cell)));
            } else {
                throw std::runtime_error("Unknown cell type");
            }
        }
    }


    //возможно, понадобится для select или join
    //Column(Column &&other): type(other.type), cells(std::move(other.cells)), is_key(other.is_key), is_unique(other.is_unique), is_autoincrement(other.is_autoincrement){}

    Column(Cell& cell) 
    {   
        std::vector<const std::type_info*> CellTypes(4);
        CellTypes[0] = &typeid(CellInt);
        CellTypes[1] =  &typeid(CellBool);
        CellTypes[2] =  &typeid(CellString);
        CellTypes[3] =  &typeid(CellBytes);

        type = std::find(CellTypes.begin(), CellTypes.end(), &typeid(cell)) - CellTypes.begin();


        if (type == 0) {
            cells.push_back(std::make_shared<CellInt>(dynamic_cast<CellInt&>(cell)));  
        } else if (type == 1) {
            cells.push_back(std::make_shared<CellBool>(dynamic_cast<CellBool&>(cell)));  
        } else if (type == 2) {
            cells.push_back(std::make_shared<CellString>(dynamic_cast<CellString&>(cell)));  
        } else if (type == 3) {
            cells.push_back(std::make_shared<CellBytes>(dynamic_cast<CellBytes&>(cell))); 
        } else {
            throw std::runtime_error("Unknown cell type");
        }
    }

    Cell& get_cell(int index) 
    {   
        if (index >= cells.size()) 
        {
            throw "there's no such cell in this column\n";
        }
        return *cells[index];
    }

    int get_cell_index(Cell &cell) 
    {   
        std::vector<const std::type_info*> CellTypes(4);
        CellTypes[0] = &typeid(CellInt);
        CellTypes[1] =  &typeid(CellBool);
        CellTypes[2] =  &typeid(CellString);
        CellTypes[3] =  &typeid(CellBytes);

        if (type != std::find(CellTypes.begin(), CellTypes.end(), &typeid(cell)) - CellTypes.begin()) 
        {
            throw "there's no cell in this column with such type\n";
            return -1;
        }
        for (int i = 0; i < cells.size(); i++) 
        {
            if (*cells[i] == cell) 
            {
                return i;
            }
        }
        throw "there's no such cell in this column\n";
        return -1;
    }

    void add_cell(Cell& cell) 
    {
        std::vector<const std::type_info*> CellTypes(4);
        CellTypes[0] = &typeid(CellInt);
        CellTypes[1] =  &typeid(CellBool);
        CellTypes[2] =  &typeid(CellString);
        CellTypes[3] =  &typeid(CellBytes);

        if (type != std::find(CellTypes.begin(), CellTypes.end(), &typeid(cell)) - CellTypes.begin()) 
        {
            throw "Unknown cell type\n";
            return;
        }
        if (type == 0) {
            cells.push_back(std::make_shared<CellInt>(dynamic_cast<CellInt&>(cell)));  
        } else if (type == 1) {
            cells.push_back(std::make_shared<CellBool>(dynamic_cast<CellBool&>(cell)));  
        } else if (type == 2) {
            cells.push_back(std::make_shared<CellString>(dynamic_cast<CellString&>(cell))); 
        } else if (type == 3) {
            cells.push_back(std::make_shared<CellBytes>(dynamic_cast<CellBytes&>(cell)));  
        } else {
            throw std::runtime_error("Unknown cell type");
        }
    }

    ~Column() = default;
};