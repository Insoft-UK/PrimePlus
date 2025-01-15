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


#include "aliases.hpp"
#include "common.hpp"

#include "singleton.hpp"
#include <regex>
#include <sstream>

using namespace pp;

//MARK: - Functions

static bool compareInterval(Aliases::TIdentity i1, Aliases::TIdentity i2) {
    return (i1.identifier.length() > i2.identifier.length());
}

//MARK: - Public Methods

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
        identity.scope = singleton->scopeDepth.size() == 0 ? Aliases::Scope::Global : Aliases::Scope::Local;
    }
    
    if (identifierExists(identity.identifier)) {
        for (const auto &it : _identities) {
            if (it.identifier == identity.identifier) {
                std::cout
                << MessageType::Warning
                << "redefinition of: " << ANSI::Bold << identity.identifier << ANSI::Default << ", ";
                if (basename(Singleton::shared()->currentPathname()) == basename(it.pathname)) {
                    std::cout << "previous definition on line " << it.line << "\n";
                }
                else {
                    std::cout << "previous definition in '" << ANSI::Green << basename(it.pathname) << ANSI::Default << "' on line " << it.line << "\n";
                }
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
        << (Scope::Local == identity.scope && Type::Macro != identity.type ? ANSI::Default + ANSI::Bold + "local" + ANSI::Default + ":" : "")
        << (Scope::Global == identity.scope && Type::Macro != identity.type ? ANSI::Yellow + "global" + ANSI::Default + ":" : "")
        << (Type::Eenum == identity.type ? " enumerator" : "")
        << (Type::Struct == identity.type ? " structure" : "")
        << (Type::Macro == identity.type ? "macro" : "")
        << (Type::Def == identity.type ? " def" : "")
        << (Type::Unknown == identity.type ? " identifier" : "")
        << " '" << ANSI::Green << identity.identifier << ANSI::Default << "' for '" << ANSI::Green << identity.real << ANSI::Default << "' defined\n";
    return true;
}

void Aliases::removeAllLocalAliases() {
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if (it->scope == Scope::Local) {
            if (verbose) std::cout
                << MessageType::Verbose
                << ANSI::Default << ANSI::Bold << "local" << ANSI::Default << ":"
                << (Type::Eenum == it->type ? " enumerator" : "")
                << (Type::Struct == it->type ? " structure" : "")
                << (Type::Def == it->type ? " def" : "")
                << (Type::Member == it->type ? " identifier" : "")
                << (Type::Unknown == it->type ? " identifier" : "")
                << " '" << ANSI::Green << it->identifier << ANSI::Default << "' removed❗\n";
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
                << (Scope::Local == it->scope && Type::Macro != it->type ? ANSI::Default + ANSI::Bold + "local" + ANSI::Default + ": " : "")
                << (Scope::Global == it->scope && Type::Macro != it->type ? ANSI::Yellow + "global" + ANSI::Default + ": " : "")
                << (Type::Macro == it->type ? "macro" : "")
                << (Type::Eenum == it->type ? "enumerator" : "")
                << (Type::Struct == it->type ? "structure" : "")
                << (Type::Def == it->type ? " def" : "")
                << (Type::Member == it->type ? "identifier" : "")
                << (Type::Unknown == it->type ? "identifier" : "")
                << " '" << ANSI::Green << it->identifier << ANSI::Default << "' removed❗\n";
            _identities.erase(it);
            removeAllAliasesOfType(type);
            break;
        }
    }
}

static std::string resolveMacroFunction(const std::string &str, const std::string &parameters, const std::string &identifier, const std::string &real) {
    std::string result;
    std::regex re;
    std::smatch match;
    std::string pattern;
    
    re = R"(\b)" + identifier + R"(\(([^()]*)\))";
    if (std::regex_search(str, match, re)) {
        result = match[1].str();
        
        re = R"([^,]+(?=[^,]*))";
        std::vector<std::string> arguments;
        for (auto it = std::sregex_iterator(result.begin(), result.end(), re); it != std::sregex_iterator(); ++it) {
            arguments.push_back(it->str());
        }
        
        result = real;
        size_t i = 0;
        for (auto it = std::sregex_iterator(parameters.begin(), parameters.end(), re); it != std::sregex_iterator(); ++it, ++i) {
            if (arguments.empty()) {
                std::cout << MessageType::Error << ANSI::Red << "macro parameters mismatched" << ANSI::Default << '\n';
                break;
            }
            
            pattern = "\\b" + it->str() + "\\b";
            result = std::regex_replace(result, std::regex(pattern), arguments.at(i));
            
            pattern = "\\$" + std::to_string(i + 1);
            result = std::regex_replace(result, std::regex(pattern), arguments.at(i));
            
            pattern = "\\$0";
            result = std::regex_replace(result, std::regex(pattern), identifier);
        }
    }
    
    return result;
}

std::string Aliases::resolveAllAliasesInText(const std::string &str) {
    std::string s = str;
    std::regex re;
    std::smatch match;
    std::string namespaces, pattern;
    
    if (s.empty()) return s;
    
    
    namespaces = namespacePattern();
    
        
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if ('`' == it->identifier.at(0) && '`' == it->identifier.at(it->identifier.length() - 1)) {
            pattern = it->identifier;
        } else {
            if (!namespaces.empty()) {
                pattern = R"(\b)" + namespaces + "?" + regex_replace(it->identifier, std::regex(namespaces), "") + R"(\b)";
            }
            else {
                pattern = R"(\b)" + it->identifier + R"(\b)";
            }
        }
        
        re = pattern;

        if (!it->parameters.empty()) {
            if (namespaces.empty()) {
                re = R"(\b)" + it->identifier + R"(\([^()]*\))";
            }
            else {
                re = R"(\b)" + namespaces + "?" + it->identifier + R"(\([^()]*\))";
            }
            while (regex_search(s, match, re)) {
                if (it->deprecated) std::cout << MessageType::Deprecated << it->identifier << it->message << "\n";
                std::string result = resolveMacroFunction(match.str(), it->parameters, it->identifier, it->real);
                s.replace(match.position(), match.length(), result);
            }
            continue;
        }
        
        if (regex_search(s, re) && it->deprecated)
            std::cout << MessageType::Deprecated << it->identifier << it->message << "\n";
        s = regex_replace(s, re, it->real);
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
                << (Scope::Local == it->scope && Type::Macro != it->type ? ANSI::Default + ANSI::Bold + "local" + ANSI::Default + ": " : "")
                << (Scope::Global == it->scope && Type::Macro != it->type ? ANSI::Yellow + "global" + ANSI::Default + ": " : "")
                << (Type::Macro == it->type ? "macro" : "")
                << (Type::Eenum == it->type ? "enumerator" : "")
                << (Type::Struct == it->type ? "structure" : "")
                << (Type::Def == it->type ? "def" : "")
                << (Type::Member == it->type ? "identifier" : "")
                << (Type::Unknown == it->type ? "identifier" : "")
                << " '" << ANSI::Green << it->identifier << ANSI::Default << "' removed❗\n";
            
            _identities.erase(it);
            break;
        }
    }
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

const Aliases::TIdentity Aliases::getIdentity(const std::string &identifier) {
    TIdentity identity;
    for (auto it = _identities.begin(); it != _identities.end(); ++it) {
        if (it->identifier == identifier) {
            return *it;
        }
    }
    return identity;
}

//MARK: namespace

void Aliases::addNamespace(const std::string &name) {
    // We check to see if namespace allready exists, if it dose we just return.
    for (auto it = _namespaces.begin(); it != _namespaces.end(); ++it) {
        if (name == *it) return;
    }
    _namespaces.push_back(name);
    
    if (Singleton::shared()->scopeDepth.size() == 0) {
        _namespaseCheckpoint = _namespaces.size();
    }
}

void Aliases::removeNamespace(const std::string &name) {
    int index = 0;
    for (auto it = _namespaces.begin(); it != _namespaces.end(); ++it, ++index) {
        if (name != *it) continue;
        _namespaces.erase(it);
        if (index < _namespaseCheckpoint) _namespaseCheckpoint--;
        break;
    }
}

//MARK: - Private Methods

const std::string Aliases::namespacePattern(void) {
    if (_namespaces.size() == 0) {
        return std::string("");
    }
    
    std::string pattern;
    
    pattern = "((";
    for (auto it = _namespaces.begin(); it != _namespaces.end(); ++it) {
        if (it != _namespaces.begin()) {
            pattern += "|";
        }
        pattern += *it;
    }
    pattern += ")(?:::|.))";
    
    return pattern;
}
