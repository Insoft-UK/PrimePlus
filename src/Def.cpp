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

#include "Def.hpp"
#include "Alias.hpp"
#include <regex>
#include <sstream>
#include <fstream>

#include "common.hpp"

using namespace pp;

static Singleton *singleton = Singleton::shared();

static void eval(std::string &str) {
    str = singleton->aliases.resolveAliasesInText(str);
}

static std::string shell(std::string &cmd) {
    FILE *fp;
    std::string out;
    
    fp = popen(cmd.c_str(),"r");
    if (fp == NULL) {
        std::cout << MessageType::kError << "ERROR!\n" ;
        exit(300);
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
    if (!infile.is_open()) return str;

    std::string ln;
    while (!infile.eof()) {
        std::getline(infile, ln);
        infile.peek();
        if (!infile.eof()) ln += '\n';
        str.append(ln);
    }
    
    infile.close();
    return str;
}

bool Def::parse(std::string &str) {
    std::regex r;
    std::smatch m;
    std::string s;
    
    Singleton *singleton = Singleton::shared();
    
    /*
     def subtitution `name`
     def subtitution name
     def subtitution name(p1,p2,...)
     */
    r = R"(^ *def +(.+) +(`[^`]+`) *;)";
    if (!regex_match(str, r)) {
        /**
          **NEW! 1.6.5
           def functions
         */
        
        /*
         eg. def c := a+b NAME(a,b,c);
         Group  0 def  c := a+b NAME(a,b,c);
                1 c := a+b
                2 NAME
                3 a,b,c
         */
        
        r = R"(^ *def +(.+) +([a-zA-Z][\w.]*(?:(?:::)?[a-zA-Z][\w.]*)*\b) *(?:\(([A-Za-z_ ,]+)\))?(\(\(deprecated(?: *: *\"([^"]*)\")?\)\))?;)";
    }
    
    std::sregex_token_iterator it = std::sregex_token_iterator {
        str.begin(), str.end(), r, {1, 2, 3, 4, 5}
    };
    if (it != std::sregex_token_iterator()) {
        Aliases::TIdentity identity;
        identity.real = *it++;
        identity.identifier = *it++;
        identity.parameters = *it++;
        
        if (it->matched) {
            identity.deprecated = true;
            it++;
            if (it->matched) {
                identity.message = *it;
            }
        }
        
        strip(identity.parameters);
        identity.type = Aliases::Type::kDef;
        identity.scope = Aliases::Scope::kAuto;
        
        if ("shell:" == identity.real.substr(0, 6)) {
            identity.real = identity.real.substr(6, identity.real.length() - 6);
            identity.real = shell(identity.real);
        }
        
        if ("eval:" == identity.real.substr(0, 5)) {
            identity.real = identity.real.substr(5, identity.real.length() - 5);
            eval(identity.real);
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
