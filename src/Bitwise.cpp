/*
 Copyright © 2023 Insoft. All rights reserved.
 
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

#include "Bitwise.hpp"

#include <sstream>
#include <regex>
#include "Singleton.hpp"

using namespace pp;


bool Bitwise::parse(std::string &str) {
    std::string s, line = str;
    std::smatch m;
    std::string result;
    std::regex r;
    
    bool parsed = false;
    
    r = R"((?:\()? *([\w#:→]+(?: *\(.*\))?) *([&|\|]|<<|>>) *([\w#:→]+(?: *\(.*\))?)(?:\))?)";
    while (regex_search(line, m, r)) {
        std::string matched = m.str();
        std::sregex_token_iterator it = std::sregex_token_iterator {
            matched.begin(), matched.end(), r, {1,2,3}
        };
        std::string ppl;
        if (it != std::sregex_token_iterator()) {
            std::string firstOperand,operation,secondOperand;
            
            firstOperand = *it++;
            operation = *it++;
            secondOperand = *it++;
            
            if (operation == "<<") ppl = "BITSL";
            if (operation == ">>") ppl = "BITSR";
            if (operation == "&") ppl = "BITAND";
            if (operation == "|") ppl = "BITOR";
            if (operation == "^") ppl = "BITXOR";
            
            line = line.replace(m.position(), m.str().length(), ppl + "(" + firstOperand + "," + secondOperand + ")");
            parsed = true;
        } else line = line.replace(m.position(), m.str().length(), "(BITWISE PARSING ERROR!)");
    }
    
    r = R"(~ *(?:\()?([\w#:→]+(?: *\(.*\))?)(?:\))?)";
    while (regex_search(line, m, r)) {
        std::string matched = m.str();
        std::sregex_token_iterator it = std::sregex_token_iterator {
            matched.begin(), matched.end(), r, {1}
        };
        if (it != std::sregex_token_iterator()) {
            line = line.replace(m.position(), m.str().length(), "BITNOT(" + (std::string)*it + ")");
            parsed = true;
        } else line = line.replace(m.position(), m.str().length(), "(BITWISE NOT PARSING ERROR!)");
    }

    str = line;
    
    return parsed;
}



