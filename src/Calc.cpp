/*
 Copyright © 2024 Insoft. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "Calc.hpp"
#include "common.hpp"

#include <regex>
#include <vector>
#include <stack>
#include <sstream>

using namespace pp;

// Function to check if a character is an operator
static bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

// Function to get the precedence of an operator
static int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%') return 2;
    return 0;
}

// Function to perform arithmetic operations
static double applyOperator(const double a, const double b, const char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) {
                std::cout << MessageType::kError << "#[]: division by zero\n";
                return 0;
            }
            return a / b;
        case '%': return fmod(a,b);
        default: 
            std::cout << MessageType::kError << "#[]: unknown '" << op << "' operator\n";
            return 0;
    }
}

static std::string separateExpression(const std::string& expression) {
    std::stringstream separated;
    for (size_t i = 0; i < expression.length(); ++i) {
        if (expression[i] == '(' || expression[i] == ')' || expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/' || expression[i] == '%') {
            // Handle negative numbers (only if '-' is not preceded by a digit)
            if (expression[i] == '-' && (i == 0 || !isdigit(expression[i - 1]))) {
                separated << " " << expression[i];
            } else {
                separated << " " << expression[i] << " ";
            }
        } else {
            separated << expression[i];
        }
    }
    return separated.str();
}

// Function to convert infix expression to postfix (RPN)
static std::vector<std::string> infixToPostfix(const std::string& expression) {
    std::vector<std::string> output;
    std::stack<char> operators;
    
    std::regex r(R"([^ ]+)");
    for(auto it = std::sregex_iterator(expression.begin(), expression.end(), r); it != std::sregex_iterator(); ++it ) {
        std::string result = it->str();
        
        if (isdigit(result[0]) || (result.length() > 1 && result[0] == '-')) {
            output.push_back(it->str());
            continue;
        }
        
        if (isOperator(result.c_str()[0])) {
            while (!operators.empty() && precedence(operators.top()) >= precedence(result.c_str()[0])) {
                output.push_back(std::string(1, operators.top()));
                operators.pop();
            }
            operators.push(result.c_str()[0]);
            continue;
        }
        
        if (result.c_str()[0] == '(') {
            operators.push('(');
            continue;
        }
        
        if (result.c_str()[0] == ')') {
            while (!operators.empty() && operators.top() != '(') {
                output.push_back(std::string(1, operators.top()));
                operators.pop();
            }
            if (operators.empty()) {
                std::cout << MessageType::kError << "#[]: missing '(' in expression '" << expression << "'\n";
                continue;
            }
                
            operators.pop();  // Remove the '(' from stack
            continue;
        }
        
        std::cout << MessageType::kError << "#[]: uknown '" << result << "' in expression '" << expression << "'\n";
    }

    while (!operators.empty()) {
        output.push_back(std::string(1, operators.top()));
        operators.pop();
    }

    return output;
}

// Function to evaluate a postfix expression
static double evaluatePostfix(const std::vector<std::string>& postfix) {
    std::stack<double> values;

    for (const std::string& token : postfix) {
        if (isdigit(token[0]) || (token.length() > 1 && token[0] == '-')) {
            values.push(std::stod(token));
        } else if (isOperator(token[0])) {
            double b = values.top(); values.pop();
            double a = values.top(); values.pop();
            values.push(applyOperator(a, b, token[0]));
        }
    }

    return values.top();
}

// Function to evaluate an infix expression
static double evaluateExpression(const std::string& expression) {
    std::vector<std::string> postfix = infixToPostfix(expression);
    return evaluatePostfix(postfix);
}

bool Calc::parse(std::string &str)
{
    std::regex r;
    std::smatch m;

    /*
     eg. test(#[320/(7+-2*2)]:0);
     Group  0 #[320/(7+-2*2)]:0
            1 320/(7+-2*2)
     Opt!   2 0
    */
    r = R"(#\[(.*)\](?::(\d))?)";
    while (regex_search(str, m, r)) {
        std::string matched = m.str();
        std::string expression;
        int scale = -1;
        
        auto it = std::sregex_token_iterator {
            matched.begin(), matched.end(), r, {1, 2}
        };
        if (it != std::sregex_token_iterator()) {
            expression = *it++;
            if (it->matched) {
                scale = atoi(it->str().c_str());
            } else scale = -1; // -1 means auto scale
        }
        
        strip(expression);
        
        expression = regex_replace(expression, std::regex(R"(MOD)"), "%");
        expression = regex_replace(expression, std::regex(R"(BITAND)"), "&");
        expression = regex_replace(expression, std::regex(R"(BITOR)"), "|");
    
        expression = separateExpression(expression);
        double result = evaluateExpression(expression);
        
        std::stringstream ss;
        ss << std::fixed << std::setprecision(scale > -1 ? scale : 10) << result;
        std::string s = ss.str();
        
        if (scale < 0) {
            s.erase ( s.find_last_not_of('0') + 1, std::string::npos );
            s.erase ( s.find_last_not_of('.') + 1, std::string::npos );
        }
        
        str = str.replace(m.position(), m.length(), s);
    }

    return true;
}




