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


#include "structs.hpp"
#include <regex>
#include <sstream>

#include "common.hpp"

using namespace pp;

static unsigned counter = 0;
static Singleton *singleton = Singleton::shared();

// MARK: - Private Functions


static bool isStructureVariable(const std::string str) {
    return regex_search(str, std::regex(R"(\bstruct +[A-Za-z]\w* +[A-Za-z][\w*:.]* *(?:;|,))"));
}

static bool hasAliasDeclaration(const std::string& str) {
    std::regex re(R"(\w:\w)");
    return regex_search(str, re);
}

static std::string extractAliasDeclarationName(const std::string& str) {
    std::string identifier;
    size_t pos = str.find(':') + 1;
    identifier = str.substr(pos, str.length() - pos);
    return identifier;
}

static std::string extractDeclarationName(const std::string& str) {
    std::string identifier = str;
    std::regex re(R"([A-Za-z]\w*(?=:))");
    std::smatch match;
//    static unsigned counter = 0;
    
    if (regex_search(str, match, re)) {
        identifier = match.str();
        if (identifier == "auto") {
            identifier = "s" + std::to_string(++counter);
        }
        return identifier;
    }
    
    identifier = "auto";
    return identifier;
}

static std::vector<std::string> extractVariableDeclarationName(const std::string& str) {
    std::vector<std::string> name;
    std::string s;
    std::regex re(R"(([A-Za-z][\w*:.]*))");
    
    s = regex_replace(str, std::regex(R"(struct +[A-Za-z]\w* +)"), "");
    for(auto it = std::sregex_iterator(s.begin(), s.end(), re); it != std::sregex_iterator(); ++it) {
        name.push_back(it->str());
    }
    
    return name;
}

static bool isValidVariableName(const std::string& str) {
    std::regex re;
    
    re = R"(\w:\w)";
    if (regex_search(str, re)) {
        return true;
    }
    re = (R"([:.])");
    return !regex_search(str, re);
}

// MARK: - Public Methods

bool Structs::parse(std::string& str) {
    std::regex re;
    std::smatch match;
    std::string s = str;
    std::ostringstream os;
    
    
    if (!parsing) {
        re = R"(^struct +([A-Za-z][\w:.]*)$)";
        std::sregex_token_iterator it = std::sregex_token_iterator {
            str.begin(), str.end(), re, {1}
        };
        if (it != std::sregex_token_iterator()) {
            // Structure Declaration
            _structure.identifier = *it++;
            _structure.local = Singleton::shared()->scope == Singleton::Scope::Local;
            if (verbose) std::cout << MessageType::Verbose << ANSI::Blue << "struct" << ANSI::Default << ": '" << ANSI::Green << _structure.identifier << ANSI::Default << "' defined\n";
            
            parsing = true;
            return true;
        }
        
        if (isStructureVariable(str)) {
            // Create a Structure Variable
            createStructureVariable(str);
        }
        return false;
    }
    
    // End of Structure Declaration
    re = R"(^end;$)";
    if (regex_match(str, re)) {
        _structures.push_back(_structure);
        _structure.members.clear();
        parsing = false;
        return true;
    }
    
    
    // Structure Members
    re =  R"([^;]+(?=[^;]*))";
    for(std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), re); it != std::sregex_iterator(); ++it) {
        std::string member = it->str();
        trim(member);
        _structure.members.push_back(member);
    }

    return parsing;
}

void Structs::removeAllLocalStructs(void) {
    for (auto it = _structures.begin(); it != _structures.end(); ++it) {
        if (it->local == true) {
            if (verbose) std::cout
                << MessageType::Verbose
                << "struct: '" << it->identifier << "' removed!\n";
            _structures.erase(it);
            removeAllLocalStructs();
            break;
        }
    }
    counter = 0;
}

// MARK: - Private Methods
void Structs::defineStruct(const _Structure& structure, const std::string& real, const std::string& identifier) {
    std::regex re;
    
    Aliases::TIdentity identity = {
        .type = Aliases::Type::Variable,
        .scope = Aliases::Scope::Auto
    };

    
    for (auto member = structure.members.begin(); member != structure.members.end(); ++member) {
        std::string s = member->data();
        
        re = R"([A-Za-z][\w.]*)";
        identity.real = regex_replace(s, re, real);
        
        re = R"(([A-Za-z][\w.]*) *(?:(\[[\d, [\]]*)|(\([\d, ()]*\)))?)";
        std::sregex_token_iterator it = std::sregex_token_iterator {
            s.begin(), s.end(), re, {1}
        };
        
        if (it != std::sregex_token_iterator()) {
            identity.identifier = identifier + "." + (std::string)*it;
        }
        else {
            if (verbose) std::cout << MessageType::Verbose << "struct: syntax error!\n";
            continue;
        }
        
        if (verbose) std::cout << MessageType::Verbose << ANSI::Blue << "struct" << ANSI::Default << ": P+ '" << ANSI::Green << identity.identifier << ANSI::Default << "' subtitution for PPL '" << ANSI::Green << identity.real << ANSI::Default << "' \n";
        singleton->aliases.append(identity);
    }
}

void Structs::createStructureVariable(std::string& str) {
    std::regex re;
    std::smatch match;
    
    Aliases::TIdentity identity = {
        .type = Aliases::Type::Variable,
        .scope = Aliases::Scope::Auto
    };
    
    if (str.empty()) return;
    

    for (auto structure = _structures.begin(); structure != _structures.end(); ++structure) {
        re = R"(\bstruct +)" + structure->identifier + R"( +(?:[A-Za-z][\w:.]*(?: *, *)?)+)";// R"( +[A-Za-z][\w:.]*)";
        if (!regex_search(str, match, re)) continue;
        
        std::string real, identifier;
        std::vector<std::string> names = extractVariableDeclarationName(match.str());
        std::string PPL;

        for (auto name = names.begin(); name < names.end(); ++name) {
            
            if (!isValidVariableName(*name)) {
                *name = "auto:" + *name;
            }
            
            if (hasAliasDeclaration(*name)) {
                identifier = extractAliasDeclarationName(*name);
                real = extractDeclarationName(*name);
                if (verbose) std::cout << MessageType::Verbose << ANSI::Blue << "struct" << ANSI::Default << ": variable '" << ANSI::Green << identifier << ANSI::Default << "' for '" << ANSI::Green << real << ANSI::Default << "' defined\n";
            }
            else {
                identifier = *name;
                real = *name;
                if (verbose) std::cout << MessageType::Verbose << ANSI::Blue << "struct" << ANSI::Default << ": variable '" << ANSI::Green << real << ANSI::Default << "' defined\n";
            }
            
            bool realExists = singleton->aliases.realExists(real);
            
            defineStruct(*structure, real, identifier);
            
            if (realExists) continue;
            
            
            if (!PPL.empty()) PPL += ",";
            PPL += real;
            if (identifier != real) PPL += ":" + identifier;
            PPL += " := {}";
        }
        if (PPL.empty()) return;
        str = regex_replace(str, std::regex(R"(\bstruct +.*)"), "var " + PPL + ";");
    
        return;
    }
}








