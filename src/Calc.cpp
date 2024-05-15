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

using namespace pp;

#define isdigit(str) regex_match(str, std::regex(R"(-?\d*)"))

float Calc::solve(std::vector<std::string>& expression)
{
    struct sOperator
    {
        uint8_t precedence = 0;
        uint8_t arguments = 0;
    };
    
    std::unordered_map<char, sOperator> mapOps;
    mapOps['/'] = { 4, 2 };
    mapOps['*'] = { 3, 2 };
    mapOps['+'] = { 2, 2 };
    mapOps['-'] = { 1, 2 };
    
    
    struct sSymbol
    {
        std::string symbol = "";
        
        enum class Type : uint8_t
        {
            Unknown,
            Literal_Numeric,
            Operator,
            Parenthesis_Open,
            Parenthesis_Close
        } type = Type::Unknown;
        
        sOperator op;
    };
    
    std::deque<sSymbol> stkHolding;
    std::deque<sSymbol> stkOutput;
    
    sSymbol symPrevious = { "0", sSymbol::Type::Literal_Numeric, 0, 0 };
    int pass = 0;
    
    //    for (auto c : expression.b)
    for (auto it = expression.begin(); it != expression.end(); ++it) {
        char c = it->c_str()[0];
        auto s = it->data();
        if (isdigit(it->data()))
        {
            // Push literals straight to output, they are already in order
            stkOutput.push_back({ it->data(), sSymbol::Type::Literal_Numeric });
            symPrevious = stkOutput.back();
        }
        else if (c == '(')
        {
            // Push to holding stack, it acts as a stopper when we back track
            stkHolding.push_front({ std::string(1, '('), sSymbol::Type::Parenthesis_Open });
            symPrevious = stkHolding.front();
        }
        else if (c == ')')
        {
            // Backflush holding stack into output until open parenthesis
            while (!stkHolding.empty() && stkHolding.front().type != sSymbol::Type::Parenthesis_Open)
            {
                stkOutput.push_back(stkHolding.front());
                stkHolding.pop_front();
            }
            
            if (stkHolding.empty())
            {
                std::cout << "!!!!     ERROR! Unexpected parenthesis '" << c << "'\n";
                return 0;
            }
            
            // Remove corresponding open parenthesis from holding stack
            if (!stkHolding.empty() && stkHolding.front().type == sSymbol::Type::Parenthesis_Open)
            {
                stkHolding.pop_front();
            }
            
            symPrevious = { std::string(1, c), sSymbol::Type::Parenthesis_Close };
            
        }
        else if (mapOps.contains(c))
        {
            // Symbol is operator
            sOperator new_op = mapOps[c];
            
            if (c == '-' || c == '+')
            {
                if ((symPrevious.type != sSymbol::Type::Literal_Numeric
                     && symPrevious.type != sSymbol::Type::Parenthesis_Close) || pass == 0)
                {
                    new_op.arguments = 1;
                    new_op.precedence = 100;
                }
            }
            
            while (!stkHolding.empty() && stkHolding.front().type != sSymbol::Type::Parenthesis_Open)
            {
                // Ensure holding stack front is an operator (it might not be later...)
                if (stkHolding.front().type == sSymbol::Type::Operator)
                {
                    const auto& holding_stack_op = stkHolding.front().op;
                    
                    if (holding_stack_op.precedence >= new_op.precedence)
                    {
                        stkOutput.push_back(stkHolding.front());
                        stkHolding.pop_front();
                    }
                    else
                        break;
                }
            }
            
            // Push the new operator onto the holding stack
            stkHolding.push_front({ std::string(1, c), sSymbol::Type::Operator, new_op });
            symPrevious = stkHolding.front();
        } else {
            std::cout << "Bad Symbol: '" << c << "'\n";
            return 0;
        }
        
        pass++;
    }
    
    
    
    // Drain the holding stack
    while (!stkHolding.empty()) {
        stkOutput.push_back(stkHolding.front());
        stkHolding.pop_front();
    }

    
    // Solver
    std::deque<double> stkSolve;
    
    for (const auto& inst : stkOutput) {
        switch (inst.type) {
                
            case sSymbol::Type::Literal_Numeric: {
                stkSolve.push_front(std::stod(inst.symbol));
            } break;
                
            case sSymbol::Type::Operator: {
                std::vector<double> mem(inst.op.arguments);
                for (uint8_t a = 0; a < inst.op.arguments; a++) {
                    if (stkSolve.empty()) {
                        std::cout << "!!!     ERROR! Bad Expression\n";
                    } else {
                        mem[a] = stkSolve[0];
                        stkSolve.pop_front();
                    }
                }
                
                double result = 0.0;
                if (inst.op.arguments == 2) {
                    if (inst.symbol[0] == '/') result = mem[1] / mem[0];
                    if (inst.symbol[0] == '*') result = mem[1] * mem[0];
                    if (inst.symbol[0] == '+') result = mem[1] + mem[0];
                    if (inst.symbol[0] == '-') result = mem[1] - mem[0];
                }
                
                if (inst.op.arguments == 1) {
                    if (inst.symbol[0] == '+') result = +mem[0];
                    if (inst.symbol[0] == '-') result = -mem[0];
                }
                
                stkSolve.push_front(result);
            } break;
                
            default:
                break;
        }
    }
    
    return stkSolve[0];
}
