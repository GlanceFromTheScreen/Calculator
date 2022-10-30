#include <iostream>
#include <windows.h>
#include <functional>
#include <vector>
#include <string>
#include <stack>
#include <exception>
#include <list>
#include <filesystem>
#include <memory>
#include "Operation.h"
#include "Calculator.h"

using std::vector;
using std::function;
using std::string;
using std::stack;
using std::cout;
using std::cin;
using std::list;

Calculator::Calculator() {
    operations.push_back(new Operation("+", 1, true, [](stack<double>& s) {
        if (s.size() < 2) {
            throw std::exception("addition error\n");
        }
        double x = s.top();
        s.pop();
        s.top() += x;
        }));

    operations.push_back(new Operation("-", 1, true, [](stack<double>& s) {
        if (s.size() < 2) {
            throw std::exception("subtraction error\n");
        }
        double x = s.top();
        s.pop();
        s.top() -= x;
        }));

    operations.push_back(new Operation("*", 2, true, [](stack<double>& s) {
        if (s.size() < 2) {
            throw std::exception("multiply error\n");
        }
        double x = s.top();
        s.pop();
        s.top() *= x;
        }));

    operations.push_back(new Operation("/", 2, true, [](stack<double>& s) {
        if (s.size() < 2 || s.top() == 0) {
            throw std::exception("division error\n");
        }
        double x = s.top();
        s.pop();
        s.top() /= x;
        }));

    operations.push_back(new Operation("(", 0, false, nullptr));
    operations.push_back(new Operation(")", 0, false, nullptr));
}

Calculator::~Calculator() {
    for (auto item : operations) {
        delete item;
    }

    for (auto dlls : dll_files) {
        FreeLibrary(dlls);
    }
}

void Calculator::SetExpression(string const& str) {
    expression = str;
}

void Calculator::RemoveGaps() {
    for (int i = 0; i < expression.size(); ++i) {
        if (expression[i] == ' ') {
            expression.erase(i, 1);
            i--;
        }
    }
}

void Calculator::MakePolishNotation() {
    /*
    * Reading expression:
    * 1) number? -number? .number?
    * 2) ',' ?
    * 3) operation? name among operations stack? priority?
    * 4) not to forget left in stack operations (after loop)
    */
    stack<Operation*> operations_stack;
    RemoveGaps();
    ImportDll();

    int i = 0;
    while (i <= expression.size() - 1) {
        if (expression[i] >= '0' && expression[i] <= '9' || expression[i] == '-' && (i == 0 || (expression.size() != 1 && expression[i - 1] == '('))) {
            string buffer;
            buffer += expression[i];
            int j = 1;
            while (i <= expression.size() - 1 && (expression[i + 1] >= '0' && expression[i + 1] <= '9' || expression[i + 1] == '.')) {
                buffer += expression[i + 1];
                j++;
                i++;
            }
            i++;
            polish_notation.push_back(buffer);
        }
        else if (expression[i] == ',') {
            while (!operations_stack.empty() && operations_stack.top()->name != "(") {
                polish_notation.push_back(operations_stack.top()->name);
                operations_stack.pop();
            }
            i++;
        }
        else {
            bool totalFonud = false;
            for (auto const& item : operations) {
                bool isFound = true;
                for (int k = 0; k < item->name.size(); ++k) {
                    if (item->name[k] != expression[i + k]) {
                        isFound = false;
                        break;
                    }
                }
                if (isFound) {
                    totalFonud = true;
                    i += item->name.size();
                    if (operations_stack.empty() || item->name == "(")
                        operations_stack.push(item);
                    else {
                        while (!operations_stack.empty() && operations_stack.top()->priority >= item->priority && operations_stack.top()->name != "(") {
                            polish_notation.push_back(operations_stack.top()->name);
                            operations_stack.pop();
                        }
                        if (item->name != ")") {
                            operations_stack.push(item);
                        }
                        else {
                            operations_stack.pop();
                        }
                    }
                    break;
                }
            }
            if (!totalFonud) {
                throw std::exception("Operation or symbor is not correct\n");
                return;
            }
        }
    }
    while (!operations_stack.empty()) {
        polish_notation.push_back(operations_stack.top()->name);
        operations_stack.pop();
    }
}

void Calculator::SolvePolishNotation() {
    /*
    * now expression is in Polish Notation
    * Read from left to the right
    * Convert string to double
    * Apply Arithmetic operations (check if unary or binary)
    */
    stack<double> solution;
    while (!polish_notation.empty()) {
        if (polish_notation.front()[0] >= '0' && polish_notation.front()[0] <= '9' || (polish_notation.front()[0] == '-' && polish_notation.front().size() > 1)) {
            solution.push(std::stod(polish_notation.front()));
            polish_notation.pop_front();
        }
        else {
            for (auto const& item : operations) {
                if (item->name == polish_notation.front()) {
                    try {
                        item->run(solution);  //  operation applying
                    }
                    catch (std::exception& e) {
                        cout << e.what();
                        return;
                    }
                    break;
                }
                //  =>  error if we did not break
                //  no, there can not be unrecornized operations
            }
            polish_notation.pop_front();
        }
    }
    if (solution.size() == 1) {
        cout << "Answer: " << solution.top() << std::endl;
        cout << "type \"q\" to finish program\n";
    }
    else {
        throw std::exception("operation is missed\n");
    }
}

void Calculator::ImportDll() {

    std::filesystem::path PluginsPath = std::filesystem::current_path().parent_path();
    PluginsPath /= "plugins";

    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

    for (const auto& File : recursive_directory_iterator(PluginsPath)) {

        std::string filename = File.path().string();
        HMODULE hLib;
        hLib = LoadLibraryA(filename.c_str());
        dll_files.push_back(hLib);

        if (hLib != nullptr) {

            char* func_name;
            (FARPROC&)func_name = GetProcAddress(hLib, "name");

            bool* func_binary;
            (FARPROC&)func_binary = GetProcAddress(hLib, "isBinary");

            int* func_priority;
            (FARPROC&)func_priority = GetProcAddress(hLib, "priority");

            operations.push_back(new Operation(func_name, *func_priority, *func_binary, [hLib, func_binary](stack<double>& s) {
                if (*func_binary && s.size() >= 2) {
                    double (*pFunction)(double, double);
                    (FARPROC&)pFunction = GetProcAddress(hLib, "func");
                    double x = s.top();
                    s.pop();
                    double y = pFunction(s.top(), x);
                    s.top() = y;

                }
                else if (!*func_binary && s.size() >= 1) {
                    double (*pFunction)(double);
                    (FARPROC&)pFunction = GetProcAddress(hLib, "func");
                    s.top() = pFunction(s.top());

                }
                else {
                    throw std::exception("one of dll operation can not be computed\n");
                }
                }));
        }
    }
}

int main()
{
    Calculator c;
    cout << "enter the expression\n";
    string input;
    while (input != "q") {
        input.clear();
        c.polish_notation.clear();
        c.expression.clear();
        std::getline(cin, input);
        if (input == "q")
            break;

        c.SetExpression(input);

        try {
            c.MakePolishNotation();
            c.SolvePolishNotation();
        }
        catch (std::exception& e) {
            cout << e.what();
        }
    }

    return 0;
}

