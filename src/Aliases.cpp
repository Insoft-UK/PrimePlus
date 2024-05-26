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

#include "Aliases.hpp"
#include "common.hpp"

#include "Singleton.hpp"
#include <regex>
#include <sstream>

using namespace pp;

bool compareInterval(Aliases::TIdentity i1, Aliases::TIdentity i2) {
    return (i1.identifier.length() > i2.identifier.length());
}

bool Aliases::append(const TIdentity &idty) {
    TIdentity identity = idty;
    Singleton *singleton = Singleton::shared();
    
    if (identity.identifier.empty()) return false;
    
    trim(identity.identifier);
    trim(identity.real);
    identity.pathname = singleton->currentPathname();
    identity.line = singleton->currentLineNumber();
    
    if (!identity.message.empty()) {
        trim(identity.message);
        identity.message.insert(0, ", ");
    }
    
    if (Scope::Auto == identity.scope) {
        identity.scope = singleton->scope == Singleton::Scope::Global ? Aliases::Scope::Global : Aliases::Scope::Local;
    }
    
    if ('_' == identity.identifier.at(0) && '_' != identity.identifier.at(1)) {
        identity.identifier = identity.identifier.substr(1, identity.identifier.length() - 1);
        identity.type = Type::Property;
    }
    
    if (exists(identity) == true) {
        for (const auto &it : identities) {
            if (it.identifier == identity.identifier) {
                std::cout
                << MessageType::Warning
                << "redefinition of: "
                << (Type::Eenum == identity.type ? "enumerator" : "")
                << (Type::Struct == identity.type ? "structure" : "")
                << (Type::Macro == identity.type ? "macro" : "")
                << (Type::Def == identity.type ? "def" : "")
                << "'" << identity.identifier << "' as "
                << (Scope::Local == identity.scope && Type::Macro != identity.type ? "local: " : "")
                << (Scope::Global == identity.scope && Type::Macro != identity.type ? "global: " : "")
                << "type was previous definition at " << it.pathname << ":" << it.line << '\n';
                break;
            }
        }
        return false;
    }
    
    identities.push_back(identity);
    
    if (descendingOrder) std::sort(identities.begin(), identities.end(), compareInterval);
    
    if (verbose) std::cout
        << MessageType::Verbose
        << (Scope::Local == identity.scope && Type::Macro != identity.type ? "local:" : "")
        << (Scope::Global == identity.scope && Type::Macro != identity.type ? "global:" : "")
        << (Type::Eenum == identity.type ? " enumerator" : "")
        << (Type::Struct == identity.type ? " structure" : "")
        << (Type::Macro == identity.type ? "macro" : "")
        << (Type::Def == identity.type ? " def" : "")
        << (Type::Unknown == identity.type ? " identifier" : "")
        << " '" << identity.identifier << "' for '" << identity.real << "' defined\n";
    return true;
}

void Aliases::removeAllLocalAliases() {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->scope == Scope::Local) {
            if (verbose) std::cout
                << MessageType::Verbose
                << "local:"
                << (Type::Eenum == it->type ? " enumerator" : "")
                << (Type::Struct == it->type ? " structure" : "")
                << (Type::Def == it->type ? "def" : "")
                << (Type::Member == it->type ? " identifier" : "")
                << (Type::Unknown == it->type ? " identifier" : "")
                << " '" << it->identifier << "' removed!\n";
            identities.erase(it);
            removeAllLocalAliases();
            break;
        }
    }
}

void Aliases::removeAllAliasesOfType(const Type type) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->type == type) {
            if (verbose) std::cout
                << MessageType::Verbose
                << (Scope::Local == it->scope && Type::Macro != it->type ? "local: " : "")
                << (Scope::Global == it->scope && Type::Macro != it->type ? "global: " : "")
                << (Type::Macro == it->type ? "macro" : "")
                << (Type::Eenum == it->type ? "enumerator" : "")
                << (Type::Struct == it->type ? "structure" : "")
                << (Type::Def == it->type ? "def" : "")
                << (Type::Member == it->type ? "identifier" : "")
                << (Type::Unknown == it->type ? "identifier" : "")
                << " '" << it->identifier << "' removed!\n";
            identities.erase(it);
            removeAllLocalAliases();
            break;
        }
    }
}

static std::string resolveMacroFunction(const std::string &str, const std::string &parameters, const std::string &identifier, const std::string &real) {
    std::string s;
    std::regex r;
    std::smatch m;
    
    /*
     eg. NAME(a,b,c)
     Group  0 NAME(a,b,c)
            1 a,b,c
     */
    r = R"(\b)" + identifier + R"( *\((.+)\))";
    std::sregex_token_iterator it = std::sregex_token_iterator {
        str.begin(), str.end(), r, {1}
    };
    if (it != std::sregex_token_iterator()) {
        r = R"([^,]+(?=[^,]*))";
        s = *it++;
        std::vector<std::string> arguments;
        for(std::sregex_iterator it = std::sregex_iterator(s.begin(), s.end(), r); it != std::sregex_iterator(); ++it) {
            arguments.push_back(it->str());
        }
        s = real;
        size_t argumentIndex = 0;
        for(std::sregex_iterator it = std::sregex_iterator(parameters.begin(), parameters.end(), r); it != std::sregex_iterator(); ++it) {
            if (arguments.empty()) {
                std::cout << MessageType::Error << "macro parameters mismatched" << '\n';
                break;
            }
            std::ostringstream os;
            
            if (it->str().at(0) != '_') os << R"(\b)";
            os << it->str();
            if (it->str().at(it->str().length() - 1) != '_') os << R"(\b)";
            
            s = std::regex_replace(s, std::regex(os.str()), arguments.at(argumentIndex++));
        }
    }
    
    return s;
}

std::string Aliases::resolveAliasesInText(const std::string &str) {
    std::string s = str;
    std::regex r;
    std::smatch m;
    
    if (s.empty()) return s;
        
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if ('`' == it->identifier.at(0) && '`' == it->identifier.at(it->identifier.length() - 1)) {
            r = it->identifier;
        } else {
            r = R"(\b)" + it->identifier + R"(\b)";
        }

        if (!it->parameters.empty()) {
            r = R"(\b)" + it->identifier + R"( *\(.+\))";
            while (regex_search(s, m, r)) {
                if (it->deprecated) std::cout << MessageType::Deprecated << it->identifier << it->message << "\n";
                std::string result = resolveMacroFunction(m.str(), it->parameters, it->identifier, it->real);
                s.replace(m.position(), m.length(), result);
            }
            continue;
        }
        
        if (regex_search(s, r) && it->deprecated)
            std::cout << MessageType::Deprecated << it->identifier << it->message << "\n";
        s = regex_replace(s, r, it->real);
    }
    
    return s;
}

void Aliases::remove(const std::string &identifier) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->identifier == identifier) {
            if (verbose) std::cout
                << MessageType::Verbose
                << (Scope::Local == it->scope && Type::Macro != it->type ? "local: " : "")
                << (Scope::Global == it->scope && Type::Macro != it->type ? "global: " : "")
                << (Type::Macro == it->type ? "macro" : "")
                << (Type::Eenum == it->type ? "enumerator" : "")
                << (Type::Struct == it->type ? "structure" : "")
                << (Type::Def == it->type ? "def" : "")
                << (Type::Member == it->type ? "identifier" : "")
                << (Type::Unknown == it->type ? "identifier" : "")
                << " '" << it->identifier << "' removed!\n";
            
            identities.erase(it);
            return;
        }
    }
}

bool Aliases::exists(const TIdentity &identity) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->identifier == identity.identifier) {
            return true;
        }
    }
    return false;
}

bool Aliases::identifierExists(const std::string &identifier) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->identifier == identifier) {
            return true;
        }
    }
    return false;
}

bool Aliases::realExists(const std::string &real) {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (it->real == real) {
            return true;
        }
    }
    return false;
}

void Aliases::dumpIdentities() {
    for (auto it = identities.begin(); it != identities.end(); ++it) {
        if (verbose) std::cout << "identities : " << it->identifier << " = " << it->real << "\n";
    }
}
