#ifndef QUERY_CONDITION_H
#define QUERY_CONDITION_H
/*
#include <string>
#include <functional>
#include <utility>
#include <memory>
#include <stack>
#include <sstream>

#include "column.h"
#include "cells.h"


class QueryCondition;
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
    QueryCondition(const std::string& val, const std::string& position) {
        if (position == "left") {
            column_name = val;
        } else if (position == "right") {
            value = Value(val);
        }
    }
};

std::vector<std::string> tokenize(const std::string &tokens_in_str) {
    std::vector<std::string> tokens;
    std::istringstream iss(tokens_in_str);
    for (std::string s; iss >> s;) {
        tokens.push_back(s);
    }
    return tokens;
}

std::stack<std::string> get_polish_notation(const std::string &condition_str) {
    std::vector<std::string> tokens = tokenize(condition_str);
    std::stack<std::string> polish_notation;
    std::stack<std::string> operations;
    for (const auto &token: tokens) {
        if (isLogicalOperator(token) || isCompareOperator(token) || isMathOperator(token)) {
            while (!operations.empty() && priority[operations.top()] <= priority[token]) {
                polish_notation.push(operations.top());
                operations.pop();
            }
            operations.push(token);
        } else if (token == "(") {
            operations.push(token);
        } else if (token == ")") {
            while (operations.top() != "(") {
                polish_notation.push(operations.top());
                operations.pop();
            }
            operations.pop();
        } else {
            polish_notation.push(token);
        }
    }
    while (!operations.empty()) {
        polish_notation.push(operations.top());
        operations.pop();
    }
    return polish_notation;
}

std::shared_ptr<QueryCondition> build_condition_tree(std::stack<std::string> &cur, const std::string &which_side = "") {
    if (cur.empty()) {
        return nullptr;
    }
    std::string token = cur.top();
    cur.pop();
    std::shared_ptr<QueryCondition> node;
    if (isLogicalOperator(token)) {
        node = std::make_shared<QueryCondition>(logical_str_to_enum[token]);
        node->left = build_condition_tree(cur, "left");
        node->right = build_condition_tree(cur, "right");
    } else if (isCompareOperator(token)) {
        node = std::make_shared<QueryCondition>(compare_str_to_enum[token]);
        node->left = build_condition_tree(cur, "left");
        node->right = build_condition_tree(cur, "right");
    } else if (isMathOperator(token)) {
        node = std::make_shared<QueryCondition>(math_str_to_enum[token]);
        node->left = build_condition_tree(cur, "left");
        node->right = build_condition_tree(cur, "right");
    } else {
        node = std::make_shared<QueryCondition>(token, which_side);
    }
    return node;
}

std::shared_ptr<QueryCondition> get_condition_tree(const std::string &str) {
    std::stack<std::string> polish_notation = get_polish_notation(str);
    return build_condition_tree(polish_notation);
}

*/
#endif // QUERY_CONDITION_H
