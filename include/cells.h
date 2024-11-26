#ifndef CELLS_H
#define CELLS_H

#include <string>
#include <typeinfo>

//абстрактный
class Cell{
public:
    virtual ~Cell() = 0;
    virtual const void* get_data() const = 0;
    virtual bool operator==(Cell &other) const = 0;
};

inline Cell::~Cell() {}

class CellInt : public Cell
{
public:
    int data;
    CellInt() = default;
    CellInt(int num): data(num) {}

    bool operator==(Cell &other) const override 
    {
        return typeid(CellInt) == typeid(other) &&  data == dynamic_cast<CellInt&>(other).data;
    }
    void operator=(Cell &other) 
    {
        if (typeid(CellInt) != typeid(other))
        {
            throw "can't exchange values between different types\n";
            return;
        }
        data = (int)(dynamic_cast<CellInt&>(other).data);
        return;
    }

    const void* get_data() const override {
        return &data;
    }
};

class CellBool : public Cell 
{
public:
    bool data;
    CellBool() = default;
    CellBool(bool state): data(state) {}

    bool operator==(Cell &other) const override 
    {
        return typeid(CellBool) == typeid(other) &&  data == dynamic_cast<CellBool&>(other).data;
    }
    void operator=(Cell &other) 
    {
        if (typeid(CellBool) != typeid(other)) 
        {
            throw "can't exchange values between different types\n";
            return;
        }
        data = (bool)(dynamic_cast<CellBool&>(other).data);
        return;
    }

    const void* get_data() const override {
        return &data;
    }
    
};

class CellString : public Cell
{
public:
    std::string data;
    CellString() = default;
    CellString(std::string s): data(s) {}

    bool operator==(Cell &other) const override 
    {
        return typeid(CellString) == typeid(other) &&  data == dynamic_cast<CellString&>(other).data;
    }
    void operator=(Cell &other) 
    {
        if (typeid(CellString) != typeid(other)) 
        {
            throw "can't exchange values between different types\n";
            return;
        }
        data = (std::string)(dynamic_cast<CellString&>(other).data);
        return;
    }

    const void* get_data() const override {
        return &data;
    }
    
};

class CellBytes : public Cell
{
public:
    std::string data; //просто в байте будем хранить биты, если что
                      //ну или переделаем без потерь памяти с глиномесием
    CellBytes() = default;
    CellBytes(std::string str) : data(str){}
    CellBytes(std::vector<uint8_t> bytes) { //в числах переданы байты
        data.reserve(8 * bytes.size());
        data.clear();
        for (uint8_t byte : bytes) {
            for (int j = 7; j >= 0; --j) {
                data.push_back((byte & (1 << j)) ? '1' : '0');
            }
        }
    }

    bool operator==(Cell &other) const override 
    {
        return typeid(CellBytes) == typeid(other) &&  data == dynamic_cast<CellBytes&>(other).data;
    }
    void operator=(Cell &other) 
    {
        if (typeid(CellBytes) != typeid(other)) 
        {
            throw "can't exchange values between different types\n";
            return;
        }
        data = (std::string)(dynamic_cast<CellBytes&>(other).data);
        return;
    }

    const void* get_data() const override {
        return &data;
    }
    
};

#endif //CELLS_H