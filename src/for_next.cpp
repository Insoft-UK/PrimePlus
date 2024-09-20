/*
 The MIT License (MIT)
 
 Copyright (c) 2023 Insoft. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "for_next.hpp"
#include "singleton.hpp"

#include <regex>

static std::vector<std::string> _stack;

using namespace pp;

bool ForNext::parse(std::string &str) {
    std::regex r;
    std::smatch m;
    
    bool parsed = false;
    
    // C style for loop.
    r = R"(\bfor\b(.*);(.*);(.*)\bdo\b)";
    while (regex_search(str, m, r)) {
        std::string matched = m.str();
        std::sregex_token_iterator it = std::sregex_token_iterator {
            matched.begin(), matched.end(), r, {1, 2, 3}
        };
        std::string statements[3];
        if (it != std::sregex_token_iterator()) {
            if (it->matched) statements[0] = *it++;
            if (it->matched) statements[1] = *it++;
            if (it->matched) statements[2] = *it++;
        }
        trim(statements[0]);
        trim(statements[1]);
        trim(statements[2]);
        if (statements[1].empty()) statements[1] = "1";
        if (statements[0].empty()) {
            str = str.replace(m.position(), m.length(), "WHILE " + statements[1] + " DO");
        } else {
            str = str.replace(m.position(), m.length(), statements[0] + ";WHILE " + statements[1] + " DO");
        }
        if (!statements[2].empty()) _stack.push_back(statements[2] + ";");
        parsed = true;
    }
    
    while (regex_search(str, m, std::regex(R"(\bnext( *)?;?)", std::regex_constants::icase))) {
        if (_stack.empty()) {
            str = str.replace(m.position(), m.length(), "END;");
            continue;
        }
        std::string ppl = _stack.back();
        _stack.pop_back();
        
        if (ppl.empty()) {
            str = str.replace(m.position(), m.length(), "END;");
        } else {
            str = str.replace(m.position(), m.length(), std::string(INDENT_WIDTH, ' ') + ppl + "END;");
        }
        parsed = true;
    }
    return parsed;
}
