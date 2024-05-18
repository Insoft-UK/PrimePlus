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
#include <regex>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "common.hpp"

using namespace pp;

#define isdigit(str) regex_match(str, std::regex(R"(-?\d+(?:\.\d+)?)"))

static double solve(std::vector<std::string>& expression)
{
    struct Operator
    {
        uint8_t precedence = 0;
        uint8_t arguments = 0;
    };
    
    std::unordered_map<char, Operator> mapOps;
    mapOps['%'] = { 2, 2 };
    mapOps['/'] = { 2, 2 };
    mapOps['*'] = { 2, 2 };
    mapOps['+'] = { 1, 2 };
    mapOps['-'] = { 1, 2 };
    
    
    struct Symbol
    {
        std::string symbol = "";
        
        enum class Type : uint8_t
        {
            Unknown,
            LiteralNumeric,
            Operator,
            ParenthesisOpen,
            ParenthesisClose
        } type = Type::Unknown;
        
        Operator op;
    };
    
    std::deque<Symbol> holdingStack;
    std::deque<Symbol> RPN;
    

    for (auto it = expression.begin(); it != expression.end(); ++it) {
        char c = it->c_str()[0];
        if (isdigit(it->data()))
        {
            RPN.push_back({ it->data(), Symbol::Type::LiteralNumeric });
        }
        else if (c == '(')
        {
            // Push to holding stack, it acts as a stopper when we back track
            holdingStack.push_front({ std::string(1, '('), Symbol::Type::ParenthesisOpen });
        }
        else if (c == ')')
        {
            // Backflush holding stack into output until open parenthesis
            while (!holdingStack.empty() && holdingStack.front().type != Symbol::Type::ParenthesisOpen)
            {
                RPN.push_back(holdingStack.front());
                holdingStack.pop_front();
            }
            
            if (holdingStack.empty())
            {
                std::cout << MessageType::kError << "#[]: Unexpected Parenthesis: '" << c << "'\n";
                return 0;
            }
            
            // Remove corresponding open parenthesis from holding stack
            if (!holdingStack.empty() && holdingStack.front().type == Symbol::Type::ParenthesisOpen)
            {
                holdingStack.pop_front();
            }
            
            
        }
        else if (mapOps.contains(c))
        {
            // Symbol is operator
            Operator newOp = mapOps[c];

            
            while (!holdingStack.empty() && holdingStack.front().type != Symbol::Type::ParenthesisOpen)
            {
                // Ensure holding stack front is an operator (it might not be later...)
                if (holdingStack.front().type == Symbol::Type::Operator)
                {
                    const auto& holdingStackOp = holdingStack.front().op;
                    
                    if (holdingStackOp.precedence >= newOp.precedence)
                    {
                        RPN.push_back(holdingStack.front());
                        holdingStack.pop_front();
                    }
                    else
                        break;
                }
            }
            
            // Push the new operator onto the holding stack
            holdingStack.push_front({ std::string(1, c), Symbol::Type::Operator, newOp });

        } else {
            std::cout << MessageType::kError << "#[]: Bad Symbol: '" << c << "'\n";
            return 0;
        }
    }
    
    
    // Drain the holding stack
    while (!holdingStack.empty()) {
        RPN.push_back(holdingStack.front());
        holdingStack.pop_front();
    }

    
    // Solver
    std::deque<double> solveStack;
    
    for (const auto& inst : RPN) {
        switch (inst.type) {
                
            case Symbol::Type::LiteralNumeric: {
                solveStack.push_front(std::stod(inst.symbol));
            } break;
                
            case Symbol::Type::Operator: {
                std::vector<double> mem(inst.op.arguments);
                for (uint8_t a = 0; a < inst.op.arguments; a++) {
                    if (solveStack.empty()) {
                        std::cout << MessageType::kError << "#[]: Bad Expression\n";
                    } else {
                        mem[a] = solveStack[0];
                        solveStack.pop_front();
                    }
                }
                
                double result = 0.0;
                if (inst.op.arguments == 2) {
                    if (inst.symbol[0] == '%') result = fmod(mem[1], mem[0]);
                    if (inst.symbol[0] == '/') result = mem[1] / mem[0];
                    if (inst.symbol[0] == '*') result = mem[1] * mem[0];
                    if (inst.symbol[0] == '+') result = mem[1] + mem[0];
                    if (inst.symbol[0] == '-') result = mem[1] - mem[0];
                }
                
                if (inst.op.arguments == 1) {
                    if (inst.symbol[0] == '+') result = +mem[0];
                    if (inst.symbol[0] == '-') result = -mem[0];
                }
                
                solveStack.push_front(result);
            } break;
                
            default:
                break;
        }
    }
    
    return solveStack[0];
}

bool Calc::parse(std::string &str)
{
    std::regex r;
    std::string s;
    std::smatch m;
    int scale = -1;
    
    r = R"(\bMOD\b)";
    str = regex_replace(str, r, "%");

    /*
     eg. test(#[320/(7+-2*2)]:0);
     Group  0 #[320/(7+-2*2)]:0
            1 320/(7+-2*2)
     Opt!   2 0
    */
    r = R"(#\[([\d\/*+\-(). %]*)\](?::(\d))?)";
    while (regex_search(str, m, r)) {
        std::string matched = m.str();
        
        auto it = std::sregex_token_iterator {
            matched.begin(), matched.end(), r, {1, 2}
        };
        if (it != std::sregex_token_iterator()) {
            s = *it++;
            if (it->matched) {
                scale = atoi(it->str().c_str());
            } else scale = -1; // -1 means auto scale
        }
        
        std::vector<std::string> expression;
        std::regex re(R"((?:-?\d+(?:.\d+)?)|[\/*+\-()%])");
        for(auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); ++it ) {
            expression.push_back(it->str());
        }
        
        double number = solve(expression);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(scale > -1 ? scale : 9) << number;
        s = ss.str();
        
        if (scale < 0) {
            s.erase ( s.find_last_not_of('0') + 1, std::string::npos );
            s.erase ( s.find_last_not_of('.') + 1, std::string::npos );
        }
        
        str = str.replace(m.position(), m.length(), s);
    }
    
    
    return true;
}
