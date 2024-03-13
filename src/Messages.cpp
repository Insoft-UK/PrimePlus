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

#include "Messages.hpp"

#include <sstream>
#include <regex>
#include "Singleton.hpp"

using namespace pp;

static Singleton *singleton = Singleton::shared();

bool Messages::parse(std::string &str) {
    std::smatch m;
    std::regex r;
    
    r = R"(\[ *([a-zA-Z]\w*) +([a-zA-Z]\w*) *:?(.*)\])";
    while (regex_search(str, m, r)) {
        std::string matched = m.str();
        std::sregex_token_iterator it = std::sregex_token_iterator {
            matched.begin(), matched.end(), r, {1, 2, 3}
        };
        
        if (it != std::sregex_token_iterator()) {
            std::vector<std::string> groups;
            std::string s;
            
            groups.push_back(trim_copy(*it++));
            groups.push_back(trim_copy(*it++));
            
            if (singleton->aliases.identifierExists(groups.at(0) + "::" + groups.at(1)) == false) {
                groups.at(1) = regex_replace(groups.at(1), std::regex(R"((By|With|From|Named|At|For)(?:[A-Z]+[a-zA-Z]*)?$)"), "");
            }
            
            if (it->matched) {
                s = trim_copy(*it);
                
                if (!parse(s)) {;
                    s = regex_replace(s, std::regex(R"(\w+ *: *)"), ",");
                }
                
                if (!s.empty()) {
                    s.insert(0, "(");
                    s.append(")");
                }
            }
            
            s.insert(0, groups.at(0) + "::" + groups.at(1));
            str = str.replace(m.position(), m.str().length(), s);
            return true;
        }
    }
    
    return false;
}



