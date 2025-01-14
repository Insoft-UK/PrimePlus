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


#include "ifte.hpp"

#include <sstream>
#include <regex>
#include "singleton.hpp"

using namespace pp;


bool IFTE::parse(std::string &str) {
    std::smatch match;
    std::regex re;
    
    re = R"(\(([^?]+)\?(.+):(.+)\))";
    while (regex_search(str, match, re)) {
        std::string matched = match.str();
        std::sregex_token_iterator it = std::sregex_token_iterator {
            matched.begin(), matched.end(), re, {1, 2, 3}
        };
        
        if (it != std::sregex_token_iterator()) {
            std::string ppl, expr, trueClause, falseClause;
            expr = trim_copy(*it++);
            trueClause = trim_copy(*it++);
            falseClause = trim_copy(*it++);
            
            parse(trueClause);
            parse(falseClause);
            
            ppl = "IFTE(" + expr + ", " + trueClause + ", " + falseClause + ")";
            str = str.replace(match.position(), match.str().length(), ppl);
            return true;
        }
    }
    
    return false;
}



