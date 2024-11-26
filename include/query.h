#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <map>

class Query {
public:
    std::string type;
    std::map<std::string, std::string> data;

    Query(const std::string& query_type, const std::map<std::string, std::string>& query_data)
        : type(query_type), data(query_data) {}

    Query() = default;

    std::string get(const std::string& key) const {
        auto it = data.find(key);
        if (it != data.end()) {
            return it->second;
        }
        return {};
    }

    void set(const std::string& key, const std::string& value) {
        data[key] = value;
    }

    bool has(const std::string& key) const {
        return data.find(key) != data.end();
    }
};

#endif // QUERY_H
