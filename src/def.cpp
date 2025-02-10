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
    
    if (std::filesystem::path(filename).parent_path().empty()) {
        pathname.insert(0, singleton->getProjectPath());
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

bool Def::isDefine(const std::string &str) {
    return regex_search(str, std::regex("^ *(@global )? *(def|undef) (.*);"));
}


bool Def::processDefine(const std::string &str) {
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
    
    
    Aliases::TIdentity identity;
    identity.type = Aliases::Type::Def;
    identity.scope = Aliases::Scope::Auto;
    
    
    re = R"(^ *(?:@(global) )? *def (.+) +(`[^`]+`)() *;)";
    if (!regex_match(str, re)) {
        re = R"(^ *(?:@(global) )? *def (.+) +([a-zA-Z][\w.]*(?:(?:::)?[a-zA-Z][\w.]*)*\b)(?:\(([A-Za-z_ ,]+)\))? *;)";
    }

    if (std::regex_search(str, match, re)) {
        identity.real = match.str(2);
        identity.identifier = match.str(3);
        identity.parameters = match.str(4);
        
        strip(identity.parameters);
        identity.type = Aliases::Type::Def;
        identity.scope = match[1].matched ? Aliases::Scope::Global : Aliases::Scope::Auto;
        
        
        if ("shell:" == identity.real.substr(0, 6)) {
            identity.real = identity.real.substr(6, identity.real.length() - 6);
            identity.real = shell(identity.real);
        }
        
        
        if ('"' == identity.real.at(0) && '"' == identity.real.at(identity.real.length() - 1)) {
            s = identity.real.substr(1, identity.real.length() - 2);
            identity.real = load(s);
        }
        
        singleton->aliases.append(identity);
        return true;
    }
    
    return false;
}
