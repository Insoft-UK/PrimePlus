/*
 The MIT License (MIT)
 
 Copyright (c) 2024 Insoft. All rights reserved.
 
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



#include "Switch.hpp"
#include "common.hpp"

#include <regex>
#include <sstream>
#include "Alias.hpp"

using namespace pp;

bool Switch::parse(std::string &str) {
    std::regex r;
    std::smatch m;
    
    if (Singleton::shared()->scope == Singleton::Scope::Global) {
        _sw = 0;
        return false;
    }
    
    /*
     eg. switch expresion
     Group  0 switch expresion
            1 expresion
     */
    r = R"(\bswitch +(.+);?)";
    if (regex_search(str, m, r)) {
        std::string s = m.str();
        
        auto it = std::sregex_token_iterator {
            s.begin(), s.end(), r, {1}
        };
        if (it != std::sregex_token_iterator()) {
            std::ostringstream oss;
            oss << "LOCAL sw" << ++_sw << " := " << *it << "; CASE";
            str.replace(m.position(), m.str().length(), oss.str());
            oss.str("");
            oss << "sw" << _sw;
            _expressions.push_back({oss.str(), lwspaces(str)});
            if (verbose) std::cout
                << MessageType::Verbose
                << "switch"
                << ": '" << *it << "' for expression defined\n";
                
        }
        return true;
    }
    
    if (!_expressions.size()) return false;
    TExpression exp = _expressions.front();
    
    r = R"(\bcase +([\w#.]*) +do\b)";
    if (regex_search(str, m, r)) {
        std::string s = m.str();
        
        
        auto it = std::sregex_token_iterator {
            s.begin(), s.end(), r, {1}
        };
        if (it != std::sregex_token_iterator()) {
            str.replace(m.position(), m.str().length(), "if " + exp.expression + " == " + (std::string)*it + " then");
        }
        return true;
    }
    
    r = R"(^ *end;)";
    if (regex_match(str, r)) {
        if (lwspaces(str) == exp.indeted) {
            if (verbose) std::cout
                << MessageType::Verbose
                << "switch"
                << ": '" << _expressions.front().expression << "' expression removed!\n";
            _expressions.pop_back();
        }
    }
    
    return false;
}
