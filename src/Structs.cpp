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

#include "Structs.hpp"
#include <regex>
#include <sstream>

#include "common.hpp"

using namespace pp;

static Singleton *singleton = Singleton::shared();

static bool isStructureVariable(const std::string str) {
    return regex_search(str, std::regex(R"(\bstruct +[A-Za-z]\w* +[A-Za-z][\w*:.]* *;)"));
}

static bool hasAliasDeclaration(const std::string& str) {
    std::regex r(R"(\w:\w)");
    return regex_search(str, r);
}

static std::string extractAliasDeclarationName(const std::string &str) {
    std::string identifier;
    size_t pos = str.find(':') + 1;
    identifier = str.substr(pos, str.length() - pos);
    return identifier;
}

static std::string extractDeclarationName(const std::string &str) {
    std::string identifier = str;
    std::regex r(R"([A-Za-z]\w*(?=:))");
    std::smatch m;
    static unsigned counter = 0;
    
    if (regex_search(str, m, r)) {
        identifier = m.str();
        if (identifier == "auto") {
            std::ostringstream os;
            os << "s" << ++counter;
            identifier = os.str();
        }
        return identifier;
    }
    
    identifier = "auto";
    return identifier;
}

static std::string extractVariableDeclarationName(const std::string &str) {
    std::string name;
    std::regex r(R"(\bstruct +[A-Za-z]\w* +([A-Za-z][\w*:.]*))");
    
    std::sregex_token_iterator it = std::sregex_token_iterator {
        str.begin(), str.end(), r, {1}
    };
    std::sregex_token_iterator end;
    if (it != end) {
        name = *it++;
    }
    
    return name;
}

static bool isValidVariableName(const std::string &str) {
    std::regex r;
    
    r = R"(\w:\w)";
    if (regex_search(str, r)) {
        return true;
    }
    r = (R"([:.])");
    return !regex_search(str, r);
}


void Structs::createStructureVariable(std::string &str) {
    std::string s;
    std::regex r;
    std::smatch m;
    
    Aliases::TIdentity identity = {
        .type = Aliases::Type::kVariable,
        .scope = Aliases::Scope::kAuto
    };
    
    if (str.empty()) return;
    

    for (auto structure = _structures.begin(); structure != _structures.end(); ++structure) {
        r = R"(\bstruct +)" + structure->identifier + R"( +[A-Za-z][\w:.]*)";
        if (!regex_search(str, m, r)) continue;
        
        std::string real, identifier;
        std::string variableName = extractVariableDeclarationName(m.str());
        
        if (!isValidVariableName(variableName)) {
            variableName = "auto:" + variableName;
        }
        
        if (hasAliasDeclaration(variableName)) {
            identifier = extractAliasDeclarationName(variableName);
            real = extractDeclarationName(variableName);
            if (verbose) std::cout << MessageType::kVerbose << "struct: variable '" << identifier << "' for '" << real << "' defined\n";
        } else {
            identifier = variableName;
            real = variableName;
            if (verbose) std::cout << MessageType::kVerbose << "struct: variable '" << real << "' defined\n";
        }
    
        
        r = R"([A-Za-z][\w.]*)";
        for (auto member = structure->members.begin(); member != structure->members.end(); ++member) {
            std::string structureMember = member->data();
            identity.real = regex_replace(structureMember, r, real);
            identity.identifier = identifier + "." + structureMember.substr(0, structureMember.find('['));
            if (verbose) std::cout << MessageType::kVerbose << "struct: " << identity.identifier << " PPL: " << identity.real << "\n";
            singleton->aliases.append(identity);
        }
    
        // Preserving any existing code indenting is achieved by utilizing 'regex_replace'.
        if (identifier == real) {
            str = regex_replace(str, std::regex(R"(\bstruct +.*)"), "var " + real + ";");
        } else str = regex_replace(str, std::regex(R"(\bstruct +.*)"), "var " + real + ":" + identifier + ";");
        return;
    }
}

bool Structs::parse(std::string& str) {
    std::regex r;
    std::smatch m;
    std::string s = str;
    std::ostringstream os;
    
    if (!parsing) {
        r = R"(^ *struct +([A-Za-z][\w:.]*) *$)";
        std::sregex_token_iterator it = std::sregex_token_iterator {
            str.begin(), str.end(), r, {1}
        };
        const std::sregex_token_iterator end;
        if (it != end) {
            // Structure Declaration
            _structure.identifier = *it++;
            if (verbose) std::cout << MessageType::kVerbose << "struct: '" << _structure.identifier << "' defined\n";
            
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
    r = R"(^ *end;)";
    if (regex_match(str, r)) {
        _structures.push_back(_structure);
        _structure.members.clear();
        parsing = false;
        return true;
    }
    
    
    // Structure Members
    r =  R"([^;]+(?=[^;]*))";
    for(std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), r); it != std::sregex_iterator(); ++it) {
        std::string member = it->str();
        trim(member);
        _structure.members.push_back(member);
    }

    return parsing;
}






