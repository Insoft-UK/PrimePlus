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

static std::vector<std::string> _increments;

using namespace pp;

bool ForNext::parse(std::string& str) {
    std::regex re;
    std::smatch match;
    
    bool parsed = false;
    
    // C style for loop.
    re = R"(\bfor\b(.*);(.*);(.*)\bdo\b)";
    while (std::regex_search(str, match, re)) {
        std::string init, condition, increment, ppl;
        
        init = trim_copy(match[1].str());
        condition = trim_copy(match[2].str());
        increment = trim_copy(match[3].str());
        
        if (!init.empty()) {
            ppl = init + ";";
        }
        ppl.append("WHILE " + (condition.empty() ? "1" : condition) + " DO");
        
        if (!increment.empty()) {
            _increments.push_back(increment + ";");
        }
        
        str = str.replace(match.position(), match.length(), ppl);
        parsed = true;
    }
    
    re = R"(\bnext;)";
    while (regex_search(str, match, re)) {
        if (_increments.empty()) {
            str = str.replace(match.position(), match.length(), "END;");
            continue;
        }
        std::string ppl = _increments.back();
        _increments.pop_back();
        
        if (ppl.empty()) {
            str = str.replace(match.position(), match.length(), "END;");
        } else {
            str = str.replace(match.position(), match.length(), std::string(INDENT_WIDTH, ' ') + ppl + "END;");
        }
        parsed = true;
    }
    return parsed;
}
