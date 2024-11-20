#ifndef VALUE_H
#define VALUE_H

#include <variant>
#include <string>

namespace memdb {

    using Value = std::variant<int32_t, int64_t, float, double, std::string>;

} // namespace memdb

#endif // VALUE_H
