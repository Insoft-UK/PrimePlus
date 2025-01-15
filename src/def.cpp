// The MIT License (MIT)
//
// Copyright (c) 2023-2025 Insoft. All rights reserved.
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


#include "def.hpp"
#include "alias.hpp"
#include <regex>
#include <sstream>
#include <fstream>

#include "common.hpp"

using namespace pp;

static Singleton *singleton = Singleton::shared();

static void eval(std::string &str) {
    str = singleton->aliases.resolveAllAliasesInText(str);
}

static std::string shell(std::string &cmd) {
    FILE *fp;
    std::string out;
    
    fp = popen(cmd.c_str(),"re");
    if (fp == NULL) {
        std::cout << MessageType::CriticalError << "ERROR!\n" ;
    }
    
    char buf[2048];
    /* Read the output a line at a time - output it. */
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        out += std::string(buf);
    }
    
    pclose(fp);
    return out;
}

static std::ifstream openAsBinary(const std::string &filename) {
    std::ifstream infile;
    Singleton *singleton = Singleton::shared();
    std::string pathname = filename;
    
    if (filename.rfind("/") == std::string::npos) {
        pathname.insert(0, singleton->getPath());
    }
    infile.open(pathname, std::ios::in | std::ios::binary);
    
    return infile;
}

static std::string load(std::string &filename) {
    std::ifstream infile;
    std::string str;
    
    infile = openAsBinary(filename);
    if (!infile.is_open()) {
        return str;
    }

    std::string ln;
    while (!infile.eof()) {
        std::getline(infile, ln);
        infile.peek();
        if (!infile.eof()) {
            ln += '\n';
        }
        str.append(ln);
    }
    
    infile.close();
    return str;
}

static void removeUnnecessaryWhitespace(std::string &str) {
    // Regular expression pattern to match spaces around the specified operators
    // Operators: {}[]()≤≥≠<>=*/+-▶.,;:!^
    std::regex re(R"(\s*([{}[\]()≤≥≠<>=*\/+\-▶.,;:!^&|%])\s*)");
    
    // Replace matches with the operator and no surrounding spaces
    str = std::regex_replace(str, re, "$1");
    
    auto pos = str.find_last_of("]");
    if (pos != std::string::npos) str.insert(pos + 1, " ");
    
    return;
}

static bool isDef(const std::string &str) {
    return regex_search(str, std::regex("^def (.*);"));
}




bool Def::parse(std::string &str) {
    std::regex re;
    std::smatch match;
    std::string s;
    
    Singleton *singleton = Singleton::shared();
    
    re = R"(^undef (.*);)";
    if (regex_match(str, match, re)) {
        if (!Singleton::shared()->aliases.identifierExists(match[1].str())) return false;
        const Aliases::TIdentity identity = Singleton::shared()->aliases.getIdentity(match[1].str());
        if (identity.type != Aliases::Type::Def) return false;
        Singleton::shared()->aliases.remove(identity.identifier);
        return true;
    }
    
    if (!isDef(str)) return false;
    
    removeUnnecessaryWhitespace(str);
    re = R"(^def +(?:([a-z]+)(?:<(global|local)>)?:)?(.+) ((?:`[\w.:]+`)|[\w.:]+)(?:\(([A-Za-z_ ,]+)\))?;$)";
    if (!regex_search(str, match, re)) return false;
    
    Aliases::TIdentity identity;
    identity.type = Aliases::Type::Def;
    Aliases::Scope scope = Aliases::Scope::Auto;
    
    if (!match[2].str().empty()) {
        scope = match[2].str() == "global" ? Aliases::Scope::Global : Aliases::Scope::Local;
    }
    
    if (match[1] == "dictionary") {
        re = R"(([a-zA-Z]\w*(?:(?:[a-zA-Z_]\w*)|(?:::)|\.)*)(?:(\[#?[\dA-F]+(?::-?\d{0,2}[bodh])?(?:,#?[\dA-F]+(?::-?\d{0,2}[bodh])?)*\])|(?:=(#?[\dA-F]+(?::-?\d{0,2}[bodh])?)))?)";
        s = match[3];
        for (auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); it++) {
            identity.identifier = match[4].str() + "." + it->str(1);
            
            
            if (!it->str(2).empty()) {
                // List
                identity.real = match[4].str() + it->str(2);
            }
            if (!it->str(3).empty()) {
                identity.real = it->str(3);
            }
            
            if (it->str(2).empty() && it->str(3).empty()) {
                identity.real = match[4].str();
            }
            
            singleton->aliases.append(identity);
        }
        str = std::string("");
        return true;
    }
    
    
    
    re = R"(^def (.+) +(`[^`]+`)() *(@ *deprecated(?: *\"([^"]*)\")?)?;)";
    if (!regex_match(str, re)) {
        re = R"(^def (.+) +([a-zA-Z][\w.]*(?:(?:::)?[a-zA-Z][\w.]*)*\b)(?:\(([A-Za-z_ ,]+)\))? *(@ *deprecated(?: *\"([^"]*)\")?)?;)";
    }

    if (std::regex_search(str, match, re)) {
        identity.real = match[1].str();
        identity.identifier = match[2].str();
        identity.parameters = match[3].str();
        
        if (!match[4].str().empty()) {
            identity.deprecated = true;
            if (!match[5].str().empty()) {
                identity.message = match[5].str();
            }
        }
        
        strip(identity.parameters);
        identity.type = Aliases::Type::Def;
        identity.scope = Aliases::Scope::Auto;
        
        if ("shell:" == identity.real.substr(0, 6)) {
            identity.real = identity.real.substr(6, identity.real.length() - 6);
            identity.real = shell(identity.real);
        }
        
        
        if ('"' == identity.real.at(0) && '"' == identity.real.at(identity.real.length() - 1)) {
            s = identity.real.substr(1, identity.real.length() - 2);
            identity.real = load(s);
        }
        
        singleton->aliases.append(identity);
        str = std::string("");
        return true;
    }
    
    return false;
}
