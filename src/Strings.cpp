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

#include "Strings.hpp"
#include <regex>

using namespace pp;

void Strings::preserveStrings(const std::string &str) {
    _original = str;
}

void Strings::restoreStrings(std::string &altered) {
    std::regex r;
    std::list<std::string> s;
    
    r = R"("[^"]*")";
    if (!regex_search(_original, r)) return;
    
    for( std::sregex_iterator it = std::sregex_iterator(_original.begin(), _original.end(), r); it != std::sregex_iterator(); ++it ) {
        s.push_back(it->str());
    }
    
    std::string replace;
    std::string result;
    
    auto iter = std::sregex_iterator(altered.begin(), altered.end(), r);
    auto stop = std::sregex_iterator();
    auto last_iter = iter;
    
    auto out = back_inserter(result);
    
    for(;s.size() && iter != stop; ++iter)
    {
        out = copy(iter->prefix().first, iter->prefix().second, out);
        replace = s.front();
        s.pop_front();
        out = iter->format(out, replace);
        last_iter = iter;
    }
    
    out = copy(last_iter->suffix().first, last_iter->suffix().second, out);
    altered = result;
}
