#include <iostream>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cmath>

enum class TokenType {
    IDENTIFIER,
    NUMBER,
    STRING,
    OPERATOR,
    PAREN_OPEN,
    PAREN_CLOSE,
    COMMA,
    END
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    Lexer(const std::string& input) : input_(input), pos_(0) {
        advance();
    }

    Token next_token() {
        while (current_char_ != '\0') {
            if (std::isspace(current_char_)) {
                skip_whitespace();
                continue;
            }

            if (current_char_ == '(') {
                advance();
                return Token{TokenType::PAREN_OPEN, "("};
            }

            if (current_char_ == ')') {
                advance();
                return Token{TokenType::PAREN_CLOSE, ")"};
            }

            if (current_char_ == ',') {
                advance();
                return Token{TokenType::COMMA, ","};
            }

            if (is_operator_start(current_char_)) {
                return operator_token();
            }

            if (std::isalpha(current_char_) || current_char_ == '_' || current_char_ == '.') {
                return identifier_or_keyword();
            }

            if (std::isdigit(current_char_) || current_char_ == '.') {
                return number_token();
            }

            if (current_char_ == '\'' || current_char_ == '"') {
                return string_token();
            }

            throw std::runtime_error(std::string("Unexpected character: ") + current_char_);
        }

        return Token{TokenType::END, ""};
    }

private:
    void advance() {
        if (pos_ < input_.size()) {
            current_char_ = input_[pos_++];
        } else {
            current_char_ = '\0';
        }
    }

    void skip_whitespace() {
        while (current_char_ != '\0' && std::isspace(current_char_)) {
            advance();
        }
    }

    bool is_operator_start(char ch) {
        return ch == '=' || ch == '!' || ch == '>' || ch == '<' ||
               ch == '+' || ch == '-' || ch == '*' || ch == '/';
    }

    Token operator_token() {
        std::string op;
        op += current_char_;
        advance();

        if ((op == "!" || op == "=" || op == "<" || op == ">") && current_char_ == '=') {
            op += current_char_;
            advance();
        }

        return Token{TokenType::OPERATOR, op};
    }

    Token identifier_or_keyword() {
        std::string result;
        while (current_char_ != '\0' && (std::isalnum(current_char_) || current_char_ == '_' || current_char_ == '.')) {
            result += current_char_;
            advance();
        }

        return Token{TokenType::IDENTIFIER, result};
    }

    Token number_token() {
        std::string result;
        bool has_decimal = false;

        while (current_char_ != '\0' &&
               (std::isdigit(current_char_) || current_char_ == '.')) {
            if (current_char_ == '.') {
                if (has_decimal) {
                    throw std::runtime_error("Invalid number format");
                }
                has_decimal = true;
            }
            result += current_char_;
            advance();
        }
        return Token{TokenType::NUMBER, result};
    }

    Token string_token() {
        char quote_type = current_char_;
        advance();
        std::string result;
        while (current_char_ != '\0' && current_char_ != quote_type) {
            result += current_char_;
            advance();
        }

        if (current_char_ != quote_type) {
            throw std::runtime_error("Unterminated string literal");
        }

        advance();
        return Token{TokenType::STRING, result};
    }

    const std::string& input_;
    size_t pos_;
    char current_char_;
};

enum class ASTNodeType {
    BINARY_OP,
    UNARY_OP,
    COMPARISON,
    IDENTIFIER,
    LITERAL,
    ARITHMETIC_OP,
    FUNCTION_CALL
};

class Value {
public:
    enum class Type {
        NUMBER,
        STRING
    };

    Value(double number) : type_(Type::NUMBER), number_value_(number) {}
    Value(const std::string& str) : type_(Type::STRING), string_value_(str) {}

    double as_number() const {
        if (type_ == Type::NUMBER) {
            return number_value_;
        } else {
            try {
                return std::stod(string_value_);
            } catch (...) {
                throw std::runtime_error("Cannot convert string to number: " + string_value_);
            }
        }
    }

    const std::string& as_string() const {
        if (type_ == Type::STRING) {
            return string_value_;
        } else {
            static std::string num_str = std::to_string(number_value_);
            return num_str;
        }
    }

    Type get_type() const { return type_; }

    bool operator==(const Value& other) const {
        if (type_ == other.type_) {
            if (type_ == Type::NUMBER) {
                return number_value_ == other.number_value_;
            } else {
                return string_value_ == other.string_value_;
            }
        } else {
            return as_string() == other.as_string();
        }
    }

    bool operator!=(const Value& other) const {
        return !(*this == other);
    }

private:
    Type type_;
    double number_value_ = 0.0;
    std::string string_value_;
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual Value evaluate(const std::map<std::string, std::string>& variables) = 0;
    virtual ASTNodeType get_type() const = 0;
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

class LiteralNode : public ASTNode {
public:
    LiteralNode(const std::string& value, TokenType token_type)
        : value_(value), token_type_(token_type) {}

    Value evaluate(const std::map<std::string, std::string>& /*variables*/) override {
        if (token_type_ == TokenType::NUMBER) {
            return Value(std::stod(value_));
        } else if (token_type_ == TokenType::STRING) {
            return Value(value_);
        } else {
            throw std::runtime_error("Invalid literal token type");
        }
    }

    ASTNodeType get_type() const override { return ASTNodeType::LITERAL; }

private:
    std::string value_;
    TokenType token_type_;
};

class IdentifierNode : public ASTNode {
public:
    IdentifierNode(const std::string& name) : name_(name) {}

    Value evaluate(const std::map<std::string, std::string>& variables) override {
        auto it = variables.find(name_);
        if (it != variables.end()) {
            const std::string& val = it->second;
            try {
                double num = std::stod(val);
                return Value(num);
            } catch (...) {
                return Value(val);
            }
        } else {
            throw std::runtime_error("Variable not found: " + name_);
        }
    }

    ASTNodeType get_type() const override { return ASTNodeType::IDENTIFIER; }

private:
    std::string name_;
};

class ArithmeticOpNode : public ASTNode {
public:
    ArithmeticOpNode(const std::string& op, ASTNodePtr left, ASTNodePtr right)
        : op_(op), left_(std::move(left)), right_(std::move(right)) {}

    Value evaluate(const std::map<std::string, std::string>& variables) override {
        Value left_value = left_->evaluate(variables);
        Value right_value = right_->evaluate(variables);

        double left_num = left_value.as_number();
        double right_num = right_value.as_number();

        if (op_ == "+") {
            return Value(left_num + right_num);
        } else if (op_ == "-") {
            return Value(left_num - right_num);
        } else if (op_ == "*") {
            return Value(left_num * right_num);
        } else if (op_ == "/") {
            if (right_num == 0) {
                throw std::runtime_error("Division by zero");
            }
            return Value(left_num / right_num);
        } else {
            throw std::runtime_error("Unknown arithmetic operator: " + op_);
        }
    }

    ASTNodeType get_type() const override { return ASTNodeType::ARITHMETIC_OP; }

private:
    std::string op_;
    ASTNodePtr left_;
    ASTNodePtr right_;
};

class FunctionCallNode : public ASTNode {
public:
    FunctionCallNode(const std::string& name, std::vector<ASTNodePtr> args)
        : name_(to_upper_case(name)), args_(std::move(args)) {}

    Value evaluate(const std::map<std::string, std::string>& variables) override {
        std::vector<Value> arg_values;
        for (auto& arg : args_) {
            arg_values.push_back(arg->evaluate(variables));
        }

        if (name_ == "ABS") {
            if (arg_values.size() != 1) {
                throw std::runtime_error("Function ABS expects 1 argument");
            }
            return Value(std::abs(arg_values[0].as_number()));
        } else if (name_ == "SIN") {
            if (arg_values.size() != 1) {
                throw std::runtime_error("Function SIN expects 1 argument");
            }
            return Value(std::sin(arg_values[0].as_number()));
        } else if (name_ == "COS") {
            if (arg_values.size() != 1) {
                throw std::runtime_error("Function COS expects 1 argument");
            }
            return Value(std::cos(arg_values[0].as_number()));
        } else if (name_ == "TAN") {
            if (arg_values.size() != 1) {
                throw std::runtime_error("Function TAN expects 1 argument");
            }
            return Value(std::tan(arg_values[0].as_number()));
        } else if (name_ == "EXP") {
            if (arg_values.size() != 1) {
                throw std::runtime_error("Function EXP expects 1 argument");
            }
            return Value(std::exp(arg_values[0].as_number()));
        } else if (name_ == "LOG") {
            if (arg_values.size() != 1) {
                throw std::runtime_error("Function LOG expects 1 argument");
            }
            return Value(std::log(arg_values[0].as_number()));
        } else if (name_ == "ROUND") {
            if (arg_values.size() != 1) {
                throw std::runtime_error("Function ROUND expects 1 argument");
            }
            return Value(std::round(arg_values[0].as_number()));
        } else {
            throw std::runtime_error("Unknown function: " + name_);
        }
    }

    ASTNodeType get_type() const override { return ASTNodeType::FUNCTION_CALL; }

private:
    std::string name_;
    std::vector<ASTNodePtr> args_;

    std::string to_upper_case(const std::string& str) {
        std::string upper_str = str;
        std::transform(
            upper_str.begin(), upper_str.end(), upper_str.begin(),
            [](unsigned char c) { return std::toupper(c); });
        return upper_str;
    }
};

class ComparisonNode : public ASTNode {
public:
    ComparisonNode(const std::string& op, ASTNodePtr left, ASTNodePtr right)
        : op_(op), left_(std::move(left)), right_(std::move(right)) {}

    Value evaluate(const std::map<std::string, std::string>& variables) override {
        Value left_value = left_->evaluate(variables);
        Value right_value = right_->evaluate(variables);

        bool result = false;

        if (op_ == "=" || op_ == "==") {
            result = (left_value == right_value);
        } else if (op_ == "!=") {
            result = (left_value != right_value);
        } else {
            double left_num = left_value.as_number();
            double right_num = right_value.as_number();

            if (op_ == ">") {
                result = left_num > right_num;
            } else if (op_ == "<") {
                result = left_num < right_num;
            } else if (op_ == ">=") {
                result = left_num >= right_num;
            } else if (op_ == "<=") {
                result = left_num <= right_num;
            } else {
                throw std::runtime_error("Unknown comparison operator: " + op_);
            }
        }

        return Value(result ? 1.0 : 0.0);
    }

    ASTNodeType get_type() const override { return ASTNodeType::COMPARISON; }

private:
    std::string op_;
    ASTNodePtr left_;
    ASTNodePtr right_;
};

class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(const std::string& op, ASTNodePtr left, ASTNodePtr right)
        : op_(to_upper_case(op)), left_(std::move(left)), right_(std::move(right)) {}

    Value evaluate(const std::map<std::string, std::string>& variables) override {
        bool left_result = left_->evaluate(variables).as_number() != 0.0;
        bool right_result = right_->evaluate(variables).as_number() != 0.0;

        if (op_ == "AND") {
            return Value((left_result && right_result) ? 1.0 : 0.0);
        } else if (op_ == "OR") {
            return Value((left_result || right_result) ? 1.0 : 0.0);
        } else {
            throw std::runtime_error("Unknown binary operator: " + op_);
        }
    }

    ASTNodeType get_type() const override { return ASTNodeType::BINARY_OP; }

private:
    std::string op_;
    ASTNodePtr left_;
    ASTNodePtr right_;

    std::string to_upper_case(const std::string& str) {
        std::string upper_str = str;
        std::transform(
            upper_str.begin(), upper_str.end(), upper_str.begin(),
            [](unsigned char c) { return std::toupper(c); });
        return upper_str;
    }
};

class UnaryOpNode : public ASTNode {
public:
    UnaryOpNode(const std::string& op, ASTNodePtr operand)
        : op_(to_upper_case(op)), operand_(std::move(operand)) {}

    Value evaluate(const std::map<std::string, std::string>& variables) override {
        bool operand_result = operand_->evaluate(variables).as_number() != 0.0;

        if (op_ == "NOT") {
            return Value(!operand_result ? 1.0 : 0.0);
        } else if (op_ == "+") {
            return operand_->evaluate(variables);
        } else if (op_ == "-") {
            Value val = operand_->evaluate(variables);
            return Value(-val.as_number());
        } else {
            throw std::runtime_error("Unknown unary operator: " + op_);
        }
    }

    ASTNodeType get_type() const override { return ASTNodeType::UNARY_OP; }

private:
    std::string op_;
    ASTNodePtr operand_;

    std::string to_upper_case(const std::string& str) {
        std::string upper_str = str;
        std::transform(
            upper_str.begin(), upper_str.end(), upper_str.begin(),
            [](unsigned char c) { return std::toupper(c); });
        return upper_str;
    }
};

class Parser {
public:
    Parser(Lexer& lexer) : lexer_(lexer) {
        current_token_ = lexer_.next_token();
    }

    ASTNodePtr parse_expression() {
        return parse_logical_or_expression();
    }

private:
    Lexer& lexer_;
    Token current_token_;

    void eat(TokenType type) {
        if (current_token_.type == type) {
            current_token_ = lexer_.next_token();
        } else {
            throw std::runtime_error("Unexpected token: " + current_token_.value);
        }
    }

    ASTNodePtr parse_logical_or_expression() {
        auto node = parse_logical_and_expression();

        while (current_token_.type == TokenType::IDENTIFIER &&
               (to_upper_case(current_token_.value) == "OR")) {
            std::string op = current_token_.value;
            eat(TokenType::IDENTIFIER);
            auto right = parse_logical_and_expression();
            node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
        }

        return node;
    }

    ASTNodePtr parse_logical_and_expression() {
        auto node = parse_equality_expression();

        while (current_token_.type == TokenType::IDENTIFIER &&
               (to_upper_case(current_token_.value) == "AND")) {
            std::string op = current_token_.value;
            eat(TokenType::IDENTIFIER);
            auto right = parse_equality_expression();
            node = std::make_unique<BinaryOpNode>(op, std::move(node), std::move(right));
        }

        return node;
    }

    ASTNodePtr parse_equality_expression() {
        auto node = parse_relational_expression();

        while (current_token_.type == TokenType::OPERATOR &&
               (current_token_.value == "==" || current_token_.value == "!=" ||
                current_token_.value == "=")) {
            std::string op = current_token_.value;
            eat(TokenType::OPERATOR);
            auto right = parse_relational_expression();
            node = std::make_unique<ComparisonNode>(op, std::move(node), std::move(right));
        }

        return node;
    }

    ASTNodePtr parse_relational_expression() {
        auto node = parse_additive_expression();

        while (current_token_.type == TokenType::OPERATOR &&
               (current_token_.value == ">" || current_token_.value == "<" ||
                current_token_.value == ">=" || current_token_.value == "<=")) {
            std::string op = current_token_.value;
            eat(TokenType::OPERATOR);
            auto right = parse_additive_expression();
            node = std::make_unique<ComparisonNode>(op, std::move(node), std::move(right));
        }

        return node;
    }

    ASTNodePtr parse_additive_expression() {
        auto node = parse_multiplicative_expression();

        while (current_token_.type == TokenType::OPERATOR &&
               (current_token_.value == "+" || current_token_.value == "-")) {
            std::string op = current_token_.value;
            eat(TokenType::OPERATOR);
            auto right = parse_multiplicative_expression();
            node = std::make_unique<ArithmeticOpNode>(op, std::move(node), std::move(right));
        }

        return node;
    }

    ASTNodePtr parse_multiplicative_expression() {
        auto node = parse_unary_expression();

        while (current_token_.type == TokenType::OPERATOR &&
               (current_token_.value == "*" || current_token_.value == "/")) {
            std::string op = current_token_.value;
            eat(TokenType::OPERATOR);
            auto right = parse_unary_expression();
            node = std::make_unique<ArithmeticOpNode>(op, std::move(node), std::move(right));
        }

        return node;
    }

    ASTNodePtr parse_unary_expression() {
        if (current_token_.type == TokenType::OPERATOR &&
            (current_token_.value == "+" || current_token_.value == "-")) {
            std::string op = current_token_.value;
            eat(TokenType::OPERATOR);
            auto operand = parse_unary_expression();
            return std::make_unique<UnaryOpNode>(op, std::move(operand));
        } else if (current_token_.type == TokenType::IDENTIFIER &&
                   to_upper_case(current_token_.value) == "NOT") {
            std::string op = current_token_.value;
            eat(TokenType::IDENTIFIER);
            auto operand = parse_unary_expression();
            return std::make_unique<UnaryOpNode>(op, std::move(operand));
        } else {
            return parse_primary_expression();
        }
    }

    ASTNodePtr parse_primary_expression() {
        if (current_token_.type == TokenType::PAREN_OPEN) {
            eat(TokenType::PAREN_OPEN);
            auto node = parse_expression();
            if (current_token_.type != TokenType::PAREN_CLOSE) {
                throw std::runtime_error("Expected closing parenthesis ')'");
            }
            eat(TokenType::PAREN_CLOSE);
            return node;
        } else if (current_token_.type == TokenType::IDENTIFIER) {
            std::string name = current_token_.value;
            eat(TokenType::IDENTIFIER);

            if (current_token_.type == TokenType::PAREN_OPEN) {
                eat(TokenType::PAREN_OPEN);
                std::vector<ASTNodePtr> args;

                if (current_token_.type != TokenType::PAREN_CLOSE) {
                    args.push_back(parse_expression());

                    while (current_token_.type == TokenType::COMMA) {
                        eat(TokenType::COMMA);
                        args.push_back(parse_expression());
                    }
                }

                if (current_token_.type != TokenType::PAREN_CLOSE) {
                    throw std::runtime_error("Expected closing parenthesis ')' in function call");
                }
                eat(TokenType::PAREN_CLOSE);
                return std::make_unique<FunctionCallNode>(name, std::move(args));
            } else {
                return std::make_unique<IdentifierNode>(name);
            }
        } else if (current_token_.type == TokenType::NUMBER ||
                   current_token_.type == TokenType::STRING) {
            std::string value = current_token_.value;
            TokenType type = current_token_.type;
            eat(current_token_.type);
            return std::make_unique<LiteralNode>(value, type);
        } else {
            throw std::runtime_error("Unexpected token in primary expression: " + current_token_.value);
        }
    }

    std::string to_upper_case(const std::string& str) {
        std::string upper_str = str;
        std::transform(
            upper_str.begin(), upper_str.end(), upper_str.begin(),
            [](unsigned char c) { return std::toupper(c); });
        return upper_str;
    }
};

ASTNodePtr parse_condition(const std::string& condition) {
    Lexer lexer(condition);
    Parser parser(lexer);
    return parser.parse_expression();
}

bool check_conditional(const ASTNodePtr& ast, const std::map<std::string, std::string>& variables) {
    Value result = ast->evaluate(variables);
    return result.as_number() != 0.0;
}
