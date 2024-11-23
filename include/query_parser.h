#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "query.h"
#include "query_condition.h"


#include <string>
#include <tuple>
#include <regex>

namespace memdb {
    class Query;


    class QueryCondition;

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

    std::shared_ptr<QueryCondition> build_condition_tree(std::stack<std::string> &cur, std::string which_side = "") {
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

    Query parse_query(const std::string &str);

    class QueryParser {

    public:
        // QueryParser() = default;

        std::string to_lower_case(const std::string &input) {
            std::string result = input;
            std::vector<std::string> keywords = std::vector<std::string>();
            keywords.push_back("SELECT");
            keywords.push_back("FROM");
            keywords.push_back("WHERE");

            for (const auto &keyword: keywords) {
                size_t pos = 0;
                while ((pos = result.find(keyword, pos)) != std::string::npos) {
                    std::string lowercase_keyword = keyword;
                    for (auto &ch: lowercase_keyword) {
                        ch = std::tolower(ch);
                    }
                    result.replace(pos, keyword.size(), lowercase_keyword);
                    pos += lowercase_keyword.size();
                }
            }
            return result;
        }

        std::tuple<Query, std::shared_ptr<QueryCondition>> split_into_query_cond(std::string str) {
            std::string normal_str = to_lower_case(str);

            size_t where_pos = normal_str.find("where");

            std::string query_part = str.substr(0, where_pos);
            std::string condition_part = str.substr(where_pos + 5);

            Query query = parse_query(query_part); // эту функцию надо написать)

            std::shared_ptr<QueryCondition> condition = get_condition_tree(condition_part); // это тоже надо написать)

            return std::make_tuple(query, condition);

        } //по сути самое сложное
        //TODO: разбить на запрос и его условие(грубо говоря по слову "where")
        // дальше из условия построить дерево condition'ов
        //query прсто составить надо

    };

} // namespace memdb

#endif // QUERY_PARSER_H
