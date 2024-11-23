#ifndef QUERY_CONDITION_H
#define QUERY_CONDITION_H

#include <string>
#include <functional>
#include <utility>
#include <memory>
#include "row.h"
#include "value.h"

namespace memdb {
    enum class CompareOperator {
        DEFAULT,
        EQUAL,
        NOT_EQUAL,
        GREATER_THAN,
        LESS_THAN,
        GREATER_EQUAL,
        LESS_EQUAL,
    };
    enum class LogicalOperator {
        DEFAULT,
        AND,
        OR,
        NOT
    };

    enum class MathOperator {
        DEFAULT,
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        MOD
    };
    std::unordered_map<std::string, int> priority = {
            {"+", 0},
            {"-", 0},
            {"*", 0},
            {"/", 0},
            {"%", 0},
            {"=", 1},
            {"!=", 1},
            {">", 1},
            {"<", 1},
            {">=", 1},
            {"<=", 1},
            {"NOT", 2},
            {"AND", 3},
            { "OR", 4},
            {"(", 5},
            {")", 5}
    };
    std::unordered_map<std::string, CompareOperator> compare_str_to_enum = {
            {"=", CompareOperator::EQUAL},
            {"!=", CompareOperator::NOT_EQUAL},
            {">", CompareOperator::GREATER_THAN},
            {"<", CompareOperator::LESS_THAN},
            {">=", CompareOperator::GREATER_EQUAL},
            {"<=", CompareOperator::LESS_EQUAL}
    };
    std::unordered_map<std::string, LogicalOperator> logical_str_to_enum = {
            {"AND", LogicalOperator::AND},
            {"OR", LogicalOperator::OR},
            {"NOT", LogicalOperator::NOT}
    };
    std::unordered_map<std::string, MathOperator> math_str_to_enum = {
            {"+", MathOperator::PLUS},
            {"-", MathOperator::MINUS},
            {"*", MathOperator::MULTIPLY},
            {"/", MathOperator::DIVIDE},
            {"%", MathOperator::MOD}
    };
    bool isLogicalOperator(const std::string &token) {
        return token == "AND" || token == "OR" || token == "NOT";
    }
    bool isCompareOperator(const std::string &token) {
        return token == "=" || token == "!=" || token == ">" || token == "<" || token == ">=" || token == "<=";
    }
    bool isMathOperator(const std::string &token) {
        return token == "+" || token == "-" || token == "*" || token == "/" || token == "%";
    }
    class QueryCondition {
    public:
        std::string column_name; //если сейчас в condition название колонки
        Value value; //если сейчас в condition значение
        LogicalOperator logical_op = LogicalOperator::DEFAULT; //если сейчас в condition логический оператор
        CompareOperator op = CompareOperator::DEFAULT; //если сейчас в condition оператор сравнения
        MathOperator math_op = MathOperator::DEFAULT; //если сейчас в condition математический оператор
        std::shared_ptr<QueryCondition> left = nullptr; //если сейчас в condition левая часть
        std::shared_ptr<QueryCondition> right = nullptr; //если сейчас в condition правая часть

        QueryCondition() = default;
        explicit QueryCondition(LogicalOperator logical_op) : logical_op(logical_op) {};
        explicit QueryCondition(CompareOperator op) : op(op) {};
        explicit QueryCondition(MathOperator math_op) : math_op(math_op) {};
        QueryCondition(std::string& val, std::string& position) {
            if (position == "left") {
                column_name = val;
            } else if (position == "right") {
                value = Value(val);
            }
        }
    };


    

} // namespace memdb

#endif // QUERY_CONDITION_H
