// Symbol.h
#include <string>
#include <vector>
#include <unordered_map>
#include <QDebug>
#include "error.h"
struct Symbol {
    std::string name;      // 变量/函数名（如a、main）
    std::string type;      // 类型（如int、char*、void）
    std::string scope;     // 作用域（如global、main、block_1）
    bool is_function = false; // 是否是函数
    bool is_initialized = false; // 变量是否初始化（仅用于变量）
    std::vector<std::pair<std::string, std::string>> params; // 参数名-类型对（仅用于函数）
};
class SymbolTable {
public:
    SymbolTable() {
        enterScope("global");
        registerBuiltinFunctions(); // 调用批量注册方法
    }

    // 进入新作用域（如函数、代码块）
    void enterScope(const std::string& scopeName) {
        scopes.emplace_back(); // 压入新作用域（空哈希表）
        scopeNames.push_back(scopeName); // 添加作用域名称到栈
        currentScopeName = scopeName;
    }

    std::string getCurrentScope() const {
        return currentScopeName;
    }

    // 退出当前作用域
    void leaveScope() {
        if (!scopes.empty()) {
            scopes.pop_back();
            scopeNames.pop_back(); // 弹出作用域名称栈
            // 安全获取当前作用域名称
            currentScopeName = scopeNames.empty() ? "global" : scopeNames.back();
        }
    }


    // 插入符号（声明时调用，返回false表示重复声明）
    bool insert(const Symbol& sym) {
        if (scopes.empty()) return false;
        auto& current = scopes.back();
        if (current.count(sym.name)) return false; // 同一作用域重复声明
        current[sym.name] = sym;
        return true;
    }

    // 查找符号（从当前作用域往上找，返回nullptr表示未找到）
    Symbol* lookup(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) {
                return &it->at(name);
            }
        }
        return nullptr;
    }


    // 调试：打印当前符号表
    void dump() const {
        int level = scopes.size();
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it, --level) {
            qDebug() << "Scope [" << level << "]: " << currentScopeName << "\n";
            for (const auto& [name, sym] : *it) {
                qDebug() << "  " << name << " : " << sym.type
                          << (sym.is_function ? " (func)" : "") << "\n";
            }
        }
    }
// 按名称和参数类型查找函数
    Symbol* lookupFunction(const std::string& name, const std::vector<std::string>& argTypes) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto range = it->equal_range(name);
            for (auto iter = range.first; iter != range.second; ++iter) {
                if (iter->second.is_function) {
                    // 比较参数类型
                    if (argTypes.size() != iter->second.params.size()) continue;
                    bool match = true;
                    for (size_t i = 0; i < argTypes.size(); ++i) {
                        if (argTypes[i] != iter->second.params[i].second) {
                            match = false;
                            break;
                        }
                    }
                    if (match) return &iter->second;
                }
            }
        }
        return nullptr;
    }
    int getScopeCount() const{return scopes.size();}
private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes; // 作用域栈
    std::vector<std::string> scopeNames;
    std::string currentScopeName = "global"; // 当前作用域名
    struct BuiltinFunction {
        std::string name;          // 函数名
        std::string return_type;   // 返回类型
        std::vector<std::pair<std::string, std::string>> params; // 参数列表(名称-类型)
    };
    void registerBuiltinFunctions() {
        // 定义所有内置函数的元数据列表
        const std::vector<BuiltinFunction> builtins = {
            // IO函数
            {"printf", "int", {{"format", "string"}, {"args", "vararg"}}},
            {"scanf", "int", {{"format", "string"}, {"args", "vararg"}}},
            {"puts", "int", {{"str", "string"}}},
            {"gets", "string", {{"str", "string"}}},

            // 字符串函数
            {"strlen", "int", {{"str", "string"}}},
            {"strcmp", "int", {{"str1", "string"}, {"str2", "string"}}},
            {"strcpy", "string", {{"dest", "string"}, {"src", "string"}}},

            // 数学函数
            {"abs", "int", {{"num", "int"}}},
            {"sqrt", "double", {{"num", "double"}}},
            {"pow", "double", {{"base", "double"}, {"exponent", "double"}}}
        };

        // 批量注册函数到全局作用域
        for (const auto& func : builtins) {
            Symbol sym;
            sym.name = func.name;
            sym.type = func.return_type;
            sym.is_function = true;
            sym.scope = "global";
            sym.params = func.params;
            scopes.back().insert({sym.name, sym});
        }
    }
};
