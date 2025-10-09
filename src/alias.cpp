// The MIT License (MIT)
//
// Copyright (c) 2023-2025 Insoft.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "alias.hpp"
#include "common.hpp"

#include <sstream>
#include <regex>

using pplplus::Alias;
using pplplus::Aliases;
using pplplus::Singleton;

static Singleton *singleton = Singleton::shared();

static void parseAlias(const std::string &str, Aliases::TIdentity &identity) {
    std::regex re;
    std::smatch matches;
    
    /*
     eg. name:aliasname
     Group  0 name:alias::name
            1 name
            2 alias::name
     */
    re = R"(([a-zA-Z]\w*):([a-zA-Z_]\w*(?:::[a-zA-Z]\w*)*))";
    
    if (regex_search(str, matches, re)) {
        identity.real = matches[1].str();
        identity.scope = Singleton::shared()->scopeDepth;
        identity.identifier = matches[2].str();
        singleton->aliases.append(identity);
    }
}

static void parseAliases(const std::string &str, Aliases::TIdentity &identity) {
    std::regex re(R"([a-zA-Z]\w*:[a-zA-Z_]\w*(?:::[a-zA-Z]\w*)*)");
    
    for(std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), re); it != std::sregex_iterator(); ++it) {
        parseAlias(it->str(), identity);
    }
}

static void parseFunctionName(const std::string &str) {
    Aliases::TIdentity identity;
    identity.scope = 0;
    identity.type = Aliases::Type::Function;
    parseAlias(str, identity);
}

static void parseParameters(const std::string &str) {
    Aliases::TIdentity identity;
    identity.scope = 1;
    identity.type = Aliases::Type::Argument;
    parseAliases(str, identity);
}

static void parseVariables(const std::string &str) {
    Aliases::TIdentity identity;
    identity.scope = Singleton::shared()->scopeDepth;
    identity.type = Aliases::Type::Variable;
    parseAliases(str, identity);
}

std::string Alias::parse(const std::string &str) {
    std::string s;
    std::regex re;
    std::smatch matches;
    std::ostringstream os;
    std::string output = str;
    
    bool parsed = false;
    
    if (singleton->scopeDepth == 0) {
        /*
         eg. export name:alias::name(p1,p2:alias,a:alias)
         Group  0 export name:alias::name(p1,p2:alias,a:alias)
         1 name:alias::name
         2 p1,p2:alias,a:alias
         */
        re = std::regex(R"(^(?:EXPORT )?((?:[a-zA-Z]\w*:)?[a-zA-Z_]\w*(?:::[a-zA-Z_]\w*)*)\(((?:[a-zA-Z]\w*(?::[a-zA-Z_]\w*)?)(?:,(?:[a-zA-Z]\w*(?::[a-zA-Z_]\w*)?))*)\)$)", std::regex_constants::icase);
        
        if (regex_search(str, matches, re)) {
            parseFunctionName(matches[1].str());
            
            if (!matches[2].str().empty()) {
                parseParameters(matches[2].str());
            }
            parsed = true;
        } else {
            re = R"(\b[a-zA-Z]\w*:[a-zA-Z_]\w*(?:::[a-zA-Z]\w*)*\b)";
            if (regex_search(str, re)) {
                parseVariables(str);
                parsed = true;
            }
        }
    } else {
        re = std::regex(R"(^ *(LOCAL|CONST) +.*)", std::regex_constants::icase);
        if (regex_match(str, re)) {
            parseVariables(str);
            parsed = true;
        }
    }
    
    if (!parsed) return output;
    
    // Now that all aliases, if any, have been parsed, we can remove the :alias from the code.
    output = regex_replace(str, std::regex(R"(([a-zA-Z]\w*):[a-zA-Z_]\w*(?:::[a-zA-Z_]\w*)*)"), "$1");
    
    return output;
}
