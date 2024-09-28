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


#include "aliases.hpp"
#include "common.hpp"

#include "singleton.hpp"
#include <regex>
#include <sstream>

using namespace pp;

bool compareInterval(Aliases::TIdentity i1, Aliases::TIdentity i2) {
    return (i1.identifier.length() > i2.identifier.length());
}

bool Aliases::append(const TIdentity& idty) {
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
        for (const auto &it : _identities) {
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
    
    _identities.push_back(identity);
    
    // Resort in descending order
    std::sort(_identities.begin(), _identities.end(), compareInterval);
    
    if (verbose) std::cout
        << MessageType::Verbose
        << (Scope::Local == identity.scope && Type::Macro != identity.type ? "local:" : "")
        << (Scope::Global == identity.scope && Type::Macro != identity.type ? "global:" : "")
        << (Type::Eenum == identity.type ? " enumerator" : "")
        << (Type::Struct == identity.type ? " structure" : "")
        << (Type::Macro == identity.type ? " macro" : "")
        << (Type::Def == identity.type ? " def" : "")
        << (Type::Unknown == identity.type ? " identifier" : "")
        << " '" << identity.identifier << "' for '" << identity.real << "' defined\n";
    return true;
}

void Aliases::removeAllLocalAliases() {
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if (it->scope == Scope::Local) {
            if (verbose) std::cout
                << MessageType::Verbose
                << "local:"
                << (Type::Eenum == it->type ? " enumerator" : "")
                << (Type::Struct == it->type ? " structure" : "")
                << (Type::Def == it->type ? " def" : "")
                << (Type::Member == it->type ? " identifier" : "")
                << (Type::Unknown == it->type ? " identifier" : "")
                << " '" << it->identifier << "' removed!\n";
            _identities.erase(it);
            removeAllLocalAliases();
            break;
        }
    }
    
    while (_namespaseCheckpoint != _namespaces.size()) {
        _namespaces.resize(_namespaseCheckpoint);
    }
}

void Aliases::removeAllAliasesOfType(const Type type) {
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if (it->type == type) {
            if (verbose) std::cout
                << MessageType::Verbose
                << (Scope::Local == it->scope && Type::Macro != it->type ? "local: " : "")
                << (Scope::Global == it->scope && Type::Macro != it->type ? "global: " : "")
                << (Type::Macro == it->type ? "macro" : "")
                << (Type::Eenum == it->type ? "enumerator" : "")
                << (Type::Struct == it->type ? "structure" : "")
                << (Type::Def == it->type ? " def" : "")
                << (Type::Member == it->type ? "identifier" : "")
                << (Type::Unknown == it->type ? "identifier" : "")
                << " '" << it->identifier << "' removed!\n";
            _identities.erase(it);
            removeAllAliasesOfType(type);
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
    r = R"(\b)" + identifier + R"(\(([^()]*)\))";
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

std::string Aliases::resolveAllAliasesInText(const std::string& str) {
    std::string s = str;
    std::regex r;
    std::smatch m;
    std::string namespaces, pattern;
    
    namespaces = "((";
    for (auto it = _namespaces.begin(); it != _namespaces.end(); ++it) {
        if (it != _namespaces.begin()) {
            namespaces += "|";
        }
        namespaces += *it;
    }
    namespaces += ")::)";
    
    if (s.empty()) return s;
        
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if ('`' == it->identifier.at(0) && '`' == it->identifier.at(it->identifier.length() - 1)) {
            pattern = it->identifier;
        } else {
            if (_namespaces.size()) {
                pattern = R"(\b)" + namespaces + "?" + regex_replace(it->identifier, std::regex(namespaces), "") + R"(\b)";
            }
            else {
                pattern = R"(\b)" + it->identifier + R"(\b)";
            }
        }
        
        r = pattern;

        if (!it->parameters.empty()) {
            r = R"(\b)" + namespaces + "?" + it->identifier + R"(\([^()]*\))";
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
    
  //TODO: Rework to remove this hack!
    /*
     To ensures proper resolution in cases where one alias refers to another.
     It nessasary to perform another check, only if the first check was an alias.
     */
    if (s != str) {
        s = resolveAllAliasesInText(s);
    }
    
    return s;
}

void Aliases::remove(const std::string &identifier) {
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
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
            
            _identities.erase(it);
            break;
        }
    }
}

bool Aliases::exists(const TIdentity &identity) {
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if (it->identifier == identity.identifier) {
            return true;
        }
    }
    return false;
}

bool Aliases::identifierExists(const std::string &identifier) {
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if (it->identifier == identifier) {
            return true;
        }
    }
    return false;
}

bool Aliases::realExists(const std::string &real) {
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if (it->real == real) {
            return true;
        }
    }
    return false;
}

void Aliases::dumpIdentities() {
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if (verbose) std::cout << "_identities : " << it->identifier << " = " << it->real << "\n";
    }
}

Aliases::TIdentity Aliases::getIdentity(const std::string& identifier) {
    TIdentity identity;
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if (it->identifier == identifier) {
            memcpy(&identity, &*it, sizeof(TIdentity));
            break;
        }
    }
    return identity;
}

void Aliases::addNamespace(const std::string& name) {
    for (auto it = _namespaces.begin(); it != _namespaces.end(); ++it) {
        if (name == *it) return;
    }
    _namespaces.push_back(name);
    
    if (Singleton::shared()->scope == Singleton::Scope::Global) {
        _namespaseCheckpoint = _namespaces.size();
    }
}

void Aliases::removeNamespace(const std::string& name) {
    int index = 0;
    for (auto it = _namespaces.begin(); it != _namespaces.end(); ++it, ++index) {
        if (name != *it) continue;
        _namespaces.erase(it);
        if (index < _namespaseCheckpoint) _namespaseCheckpoint--;
        break;
    }
}
