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

#include "Enums.hpp"
#include <regex>
#include <sstream>

#include "common.hpp"

using namespace pp;

static std::string name;
static std::string prefix;

static std::string prefixFromName(const std::string &str) {
    std::string s;
    std::regex r(R"([A-Z])");
    for(std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), r); it != std::sregex_iterator(); ++it) {
        s += it->str();
    }
    return s;
}

static bool parseEnum(const std::string &str) {
    /*
     eg. enum Name : AB
     Group  0 enum Name : AB
            1 Name
            2 AB
     */
    std::regex r(R"(^ *enum +([A-Za-z]\w*) *:? *([A-Z]*) *$)");
    std::sregex_token_iterator it = std::sregex_token_iterator {
        str.begin(), str.end(), r, {1, 2}
    };
    const std::sregex_token_iterator end;
    if (it != end) {
        name = *it++;
        prefix = *it;
        if (prefix.empty()) prefix = prefixFromName(name);
        return true;
    }
    return false;
}

static void parseEnumItems(const std::string &str, std::vector<Aliases::TIdentity> &identities) {
    std::regex r(R"(([a-zA-Z]\w*) *:?= *(#?[0-9A-F]*:?\d?\d?h?d?b?o?))");
    std::sregex_token_iterator it = std::sregex_token_iterator {
                str.begin(), str.end(), r, {1, 2}
            };
    std::sregex_token_iterator end;
    while (it != end) {
        Aliases::TIdentity identity;
        std::string identifier = *it++;
        identity.type = Aliases::Type::kEenum;
        identity.scope = Aliases::Scope::kAuto;
        identity.real = *it++;
        
        identity.identifier = name + "." + identifier;
        identities.push_back(identity);
        identity.identifier = prefix + "_" + identifier;
        identities.push_back(identity);
    }
}


bool Enums::parse(std::string &str) {
    std::regex r;
    
    if (!parsing) {
        if (parseEnum(str)) {
            _identities.clear();
            parsing = true;
        }
        return parsing;
    }
    
    r = R"(^ *end;)";
    if (regex_match(str, r)) {
        for (auto identity=_identities.begin(); identity != _identities.end(); ++identity) {
            _singleton->aliases.append(*identity);
        }
        parsing = false;
        return true;
    }
    
    parseEnumItems(str, _identities);
    return parsing;
}






