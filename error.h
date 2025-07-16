// Error.h
#ifndef ERROR_H
#define ERROR_H
#include <string>
#include <vector>
enum class ErrorType {
    UNDEFINED_VARIABLE,     // 未定义变量
    DUPLICATE_DECLARATION,  // 重复声明
    TYPE_MISMATCH,          // 类型不匹配
    INVALID_OPERATION,      // 无效操作（如对字符串做减法）
    SYNTAX_ERROR,           // 语法错误
    // 其他错误类型...
};

struct Error {
    ErrorType type;
    int line;       // 错误行号
    int column;     // 错误列号
    std::string message;  // 错误信息
};
// ErrorManager.h
class ErrorManager {
public:
    static ErrorManager& instance() {
        static ErrorManager instance;
        return instance;
    }

    void addError(ErrorType type, int line, int column, const std::string& msg) {
        errors.push_back({type, line, column, msg});
    }

    const std::vector<Error>& getErrors() const {
        return errors;
    }

    bool hasErrors() const {
        return !errors.empty();
    }

    void clear() {
        errors.clear();
    }

private:
    std::vector<Error> errors;
    ErrorManager() = default;
    ErrorManager(const ErrorManager&) = delete;
    ErrorManager& operator=(const ErrorManager&) = delete;
};
#endif // ERROR_H
