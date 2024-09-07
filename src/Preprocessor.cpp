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

#include "Preprocessor.hpp"
#include "Singleton.hpp"
#include "common.hpp"

#include <regex>
#include <sstream>
#include <fstream>

using namespace pp;

static Singleton* _singleton  = Singleton::shared();



bool Preprocessor::parse(std::string &str) {
    std::string s;
    std::regex r;
    std::smatch m;
    std::sregex_token_iterator it;
    std::sregex_token_iterator end;
    Aliases::TIdentity  identity;
    pathname = std::string("");
    
    
    if (regex_search(str, std::regex(R"(^ *#END\b)", std::regex_constants::icase))) {
        if (std::string::npos != _nesting.back().compare("#PYTHON")) {
            python = false;
        } // #END is for #PYTHON
        
        if (std::string::npos != _nesting.back().compare("#PPL")) {
            ppl = false;
        } // #END is for #PPL
        
        _nesting.pop_back();
        return false;
    }
    
    
    if (regex_search(str, std::regex(R"(^ *#PYTHON\b)"))) {
        _nesting.push_back(std::string("#PYTHON"));
        python=true;
        return true;
    }
    
    if (regex_search(str, std::regex(R"(^ *#PPL\b)"))) {
        _nesting.push_back(std::string("#PPL"));
        ppl=true;
        return true;
    }
    
    
    if (disregard == false) {
        r = R"(^ *#include +)";
        if (regex_search(str, r)) {
            std::sregex_token_iterator it;
            const std::sregex_token_iterator end;
            
            r = R"(^ *#include +<([^<>:"\|\?\*]*)>)";
            it = std::sregex_token_iterator {
                str.begin(), str.end(), r, {1}
            };
            if (it != end) {
                pathname = *it++;
                pathname = path + pathname;
                if (std::string::npos == pathname.rfind('.')) pathname += ".pplib";
                if (verbose) std::cout << MessageType::Verbose << "#include: file named '" << pathname << "'\n";
                return true;
            }
            
            r = R"(^ *#include +"([^<>:"\|\?\*]*)\")";
            it = std::sregex_token_iterator {
                str.begin(), str.end(), r, {1}
            };
            if (it != end) {
                pathname = *it++;
                if (!file_exists(pathname)) {
                    pathname = _singleton->currentPathname().substr(0, _singleton->currentPathname().rfind("/") + 1) + pathname;
                }
                if (verbose) std::cout << MessageType::Verbose << "#include: file named '" << pathname << "'\n";
                return true;
            }
            return false;
        }
        
        /*
         eg. #define NAME(a,b,c) c := a+b
         Group  0 #define NAME(a,b,c) c := a+b
                1 NAME
                2 a,b,c
                3 c := a+b
         */
        r = R"(^ *#define +([A-Za-z_]\w*)(?:\(([A-Za-z_ ,]+)\))? *(.*))";
        it = std::sregex_token_iterator {
            str.begin(), str.end(), r, {1, 2, 3}
        };
        if (it != end) {
            identity.identifier = *it++;
            identity.parameters = *it++;
            strip(identity.parameters);
            identity.real = *it++;
            
            identity.scope = Aliases::Scope::Global;
            identity.type = Aliases::Type::Macro;
            
            _singleton->aliases.append(identity);
            if (verbose) std::cout << MessageType::Verbose << "#define: " << identity.identifier << '\n';
            return true;
        }
 
        /*
         eg. #undef NAME
         Group  0 #undef NAME
                1 NAME
         */
        // #undef
        
        r = R"(^ *#undef +([a-zA-Z_][\w.:]*) *$)";
        it = std::sregex_token_iterator {
            str.begin(), str.end(), r, {1}
        };
        if (it != end) {
            _singleton->aliases.remove(*it);
            if (verbose) std::cout << MessageType::Verbose << "#undef: " << *it << '\n';
            return true;
        }
        
        
        // #pragma
        r = R"((?:^ *#pragma +)\((.*)\) *$)";
        it = std::sregex_token_iterator {
            str.begin(), str.end(), r, {1}
        };
        if (it != end) {
            s = *it;
            r = R"([^,]+(?=[^,]*))";
            for(std::sregex_iterator it = std::sregex_iterator(s.begin(), s.end(), r); it != std::sregex_iterator(); ++it) {
                std::string pragma = trim_copy(it->str());
                
                if (pragma == "bitwise 0" || pragma == "bitwise off") {
                    bitwiseOperators = false;
                }
                
                if (pragma == "bitwise 1" || pragma == "bitwise on") {
                    bitwiseOperators = true;
                }
                
                if (pragma == "verbose aliases") {
                    Singleton::shared()->aliases.verbose = !Singleton::shared()->aliases.verbose;
                }
                
                if (pragma == "unorderedness") {
                    Singleton::shared()->aliases.descendingOrder = false;
                }
                
                if (pragma == "newline") {
                    newline = true;
                }
                
                if (pragma == "indents") {
                    indents = true;
                }
                
                if (pragma == "reduce") {
                    reduce = true;
                }
                
                if (pragma == "messages") {
                    messages = true;
                }
                
                // 'cstyle' Experimental use only: DO NOT USE
                if (pragma == "cstyle") {
                    cstyle = !cstyle;
                    std::cout << MessageType::Verbose << "c/c++ style " << (cstyle ? "enabled!" : "disabled!") << "\n";
                }
                
                if (regex_search(pragma, std::regex(R"(minify (?:\d+|-1))"))) {
                    size_t start = pragma.find(" ") + 1;
                    size_t length = pragma.length() - start;
                    minify = atoi(pragma.substr(start,length).c_str());
                }
                
                if (regex_search(pragma, std::regex(R"(tabsize (?:\d+))"))) {
                    size_t start = pragma.find(" ") + 1;
                    size_t length = pragma.length() - start;
                    Singleton::shared()->tabsize = (unsigned int)atoi(pragma.substr(start,length).c_str());
                }
                
                if (verbose) std::cout << MessageType::Verbose << "#pragma: " << pragma << '\n';
            }
            return true;
        }

        /*
         eg. #ifdef NAME
         Group  0 #ifdef NAME
                1 NAME
         */
        r = R"(^\ *#ifdef +([A-Za-z_]\w*) *$)";
        it = std::sregex_token_iterator {
            str.begin(), str.end(), r, {1}
        };
        if (it != end) {
            identity.identifier = *it;
            disregard = !_singleton->aliases.exists(identity);
            if (verbose) std::cout << MessageType::Verbose << "#ifdef: " << identity.identifier << " is " << (!disregard ? "true" : "false") << '\n';
            return true;
        }
        
        /*
         eg. #ifndef NAME
         Group  0 #ifdef NAME
                1 NAME
         */
        r = R"(^\ *#ifndef +([A-Za-z_]\w*) *$)";
        it = std::sregex_token_iterator {
            str.begin(), str.end(), r, {1}
        };
        if (it != end) {
            identity.identifier = *it;
            
            disregard = _singleton->aliases.exists(identity);
            if (verbose) std::cout << MessageType::Verbose << "#ifndef: " << identity.identifier << " is " << (!disregard ? "true" : "false") << '\n';
            return true;
        }
    }
    
    if (regex_search(str, std::regex(R"(^ *#else\b *((\/\/.*)|)$)"))) {
        disregard = !disregard;
        if (_singleton->aliases.verbose) std::cout << MessageType::Verbose << "#else: " << disregard << '\n';
        return true;
    }
    
    if (regex_search(str, std::regex(R"(^ *#endif\b *((\/\/.*)|)$)"))) {
        disregard = false;
        if (_singleton->aliases.verbose) std::cout << MessageType::Verbose << "#endif: " << disregard << '\n';
        return true;
    }

    return false;
}


