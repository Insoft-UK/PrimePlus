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

#include "Alias.hpp"
#include "common.hpp"

#include <sstream>
#include <regex>

using namespace pp;

static Singleton *singleton = Singleton::shared();

static void parseAlias(const std::string &str, Aliases::TIdentity &identity) {
    /*
     eg. name:alias
     Group  0 name:alias
            1 name
            2 alias
     */
    std::regex r(R"(([a-zA-Z_]\w*):([a-zA-Z][\w.]*((?:::)?[a-zA-Z][\w.]*)*))");
    std::sregex_token_iterator it = std::sregex_token_iterator {
        str.begin(), str.end(), r, {1, 2}
    };
    if (it != std::sregex_token_iterator()) {
        identity.real = *it++;
        identity.identifier = *it;
        singleton->aliases.append(identity);
    }
}

static void parseAliases(const std::string &str, Aliases::TIdentity &identity) {
    std::regex r(R"([a-zA-Z_]\w*:[a-zA-Z][\w.]*((?:::)?[a-zA-Z][\w.]*)*)");
    
    for(std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), r); it != std::sregex_iterator(); ++it) {
        parseAlias(it->str(), identity);
    }
}

static void parseFunctionName(const std::string &str) {
    Aliases::TIdentity identity;
    identity.scope = Aliases::Scope::kGlobal;
    identity.type = Aliases::Type::kFunction;
    parseAlias(str, identity);
}

static void parseParameters(const std::string &str) {
    Aliases::TIdentity identity;
    identity.scope = Aliases::Scope::kLocal;
    identity.type = Aliases::Type::kUnknown;
    parseAliases(str, identity);
}

static void parseVariables(const std::string &str) {
    Aliases::TIdentity identity;
    identity.scope = Aliases::Scope::kAuto;
    identity.type = Aliases::Type::kVariable;
    parseAliases(str, identity);
}

bool Alias::parse(std::string &str) {
    std::string s;
    std::regex r;
    std::smatch m;
    std::ostringstream os;
    
    std::sregex_token_iterator it;
    const std::sregex_token_iterator end;
    bool parsed = false;
    
    /*
     eg. fnName:alias(p1, p2:alias, auto:alias)
     Group  0 fnName:alias(p1, p2:alias, auto:alias)
            1 fnName:alias
            2 p1, p2:alias, auto:alias
     */
    r = R"(^ *([a-zA-Z][\w:]*) *\(([\w:, ]*?)\))";
    it = std::sregex_token_iterator {
        str.begin(), str.end(), r, {1, 2}
    };
    if (it != end && singleton->scope == Singleton::Scope::kGlobal) {
        parseFunctionName(*it++);
        parseParameters(*it++);
        parsed = true;
    }
    
    r = R"(^ *(var|const) +.*)";
    if (regex_match(str, r)) {
        parseVariables(str);
        str = regex_replace(str, std::regex(R"(\bvar\b)"), "LOCAL");
        str = regex_replace(str, std::regex(R"(\bconst\b)"), "CONST");
        parsed = true;
    }
    
    if (!parsed) return false;
    str = regex_replace(str, std::regex(R"(:[a-zA-Z][\w.]*((?:::)?[a-zA-Z][\w.]*)*)"), "");
    
    return true;
}
