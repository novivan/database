#ifndef QUERY_CONDITION_H
#define QUERY_CONDITION_H

#include <string>
#include <functional>
#include "row.h"
#include "value.h"

namespace memdb {

    enum class Operator {
        EQUAL,
        NOT_EQUAL,
        GREATER_THAN,
        LESS_THAN,
        GREATER_EQUAL,
        LESS_EQUAL
    };

    enum class LogicalOperator {
        AND,
        OR,
        NOT
    };

    class QueryCondition {
    public:
        std::string column_name;
        Operator op;
        Value value;
        LogicalOperator logical_op = LogicalOperator::AND;
        QueryCondition* left = nullptr;
        QueryCondition* right = nullptr;

        QueryCondition(const std::string& column_name, Operator op, const Value& value)
            : column_name(column_name), op(op), value(value), left(nullptr), right(nullptr) {}

        QueryCondition(LogicalOperator logical_op, QueryCondition* left, QueryCondition* right)
            : logical_op(logical_op), left(left), right(right) {}

        bool evaluate(const Row& row) const {
            if (left && right) {
                switch (logical_op) {
                    case LogicalOperator::AND:
                        return left->evaluate(row) && right->evaluate(row);
                    case LogicalOperator::OR:
                        return left->evaluate(row) || right->evaluate(row);
                    case LogicalOperator::NOT:
                        return !left->evaluate(row);
                }
            }

            auto it = row.values.find(column_name);
            if (it == row.values.end()) {
                throw std::runtime_error("Column not found in row");
            }

            const Value& row_value = it->second;

            switch (op) {
                case Operator::EQUAL:
                    return row_value == value;
                case Operator::NOT_EQUAL:
                    return row_value != value;
                case Operator::GREATER_THAN:
                    return row_value > value;
                case Operator::LESS_THAN:
                    return row_value < value;
                case Operator::GREATER_EQUAL:
                    return row_value >= value;
                case Operator::LESS_EQUAL:
                    return row_value <= value;
                default:
                    throw std::runtime_error("Unknown operator");
            }
        }
    };

} // namespace memdb

#endif // QUERY_CONDITION_H
