#ifndef ROW_H
#define ROW_H

#include <unordered_map>
#include <string>
#include "value.h"

namespace memdb {

    class Row {
    public:
        std::unordered_map<std::string, Value> values;

        Value& operator[](const std::string& column_name) {
            return values[column_name];
        }

        const Value& operator[](const std::string& column_name) const {
            return values.at(column_name);
        }
    };

} // namespace memdb

#endif // ROW_H
