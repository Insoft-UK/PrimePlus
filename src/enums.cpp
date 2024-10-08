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


#include "enums.hpp"
#include <regex>
#include <sstream>

#include "common.hpp"

using namespace pp;

static std::string name;


static bool parseEnum(const std::string& str) {
    std::regex re;
    std::smatch match;
    
    re = R"(enum ([A-Za-z]\w*)$)";
    if (std::regex_search(str, match, re)) {
        name = match.str(1);
        return true;
    }
    
    return false;
}

bool Enums::parse(std::string& str) {
    std::regex re;
    
    if (!parsing) {
        if (parseEnum(str)) {
            _identities.clear();
            parsing = true;
        }
        return parsing;
    }
    
    re = R"(^end;$)";
    if (regex_match(str, re)) {
        parsing = false;
        return true;
    }
    
    parseEnumItems(str);
    
    return parsing;
}

void Enums::parseEnumItems(const std::string& str) {
    std::regex re(R"(([A-Za-z]\w*) :?= ((?:#[\dA-F]+(?::\d+)?h)|(?:#\d+(?::\d+)?d)|(?:#[0-1]+(?::\d+)?b)|(?:#[0-7]+(?::\d+)?o)|(?:\d+(?:\.\d+)?)))");
    
    // Create an iterator to go over all matches
    auto matches_begin = std::sregex_iterator(str.begin(), str.end(), re);
    auto matches_end = std::sregex_iterator();
    
    Aliases::TIdentity identity;
    identity.type = Aliases::Type::Eenum;
    identity.scope = Aliases::Scope::Auto;
    
    // Iterate over all matches
    for (std::sregex_iterator it = matches_begin; it != matches_end; ++it) {
        std::smatch match = *it;  // Dereference the iterator to get the match
        
        identity.real = match[2].str();
        // Old
        identity.identifier = name + "." + match[1].str();
        _singleton->aliases.append(identity);
        // New v2.0.5
        identity.identifier = name + "::" + match[1].str();
        _singleton->aliases.append(identity);
    }
}






