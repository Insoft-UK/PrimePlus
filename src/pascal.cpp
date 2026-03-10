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


#include "pascal.hpp"

#include <iostream>
#include <regex>
#include <string>
#include <sstream>
#include <unordered_set>

namespace pplplus::pascal {
    static std::string lowercased(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    static std::string replaceWords(const std::string& input, const std::vector<std::string>& words, const std::string& replacement) {
        // Create lowercase word set
        std::unordered_set<std::string> wordSet;
        for (const auto& w : words) {
            wordSet.insert(lowercased(w));
        }

        std::string result;
        size_t i = 0;
        
        while (i < input.size()) {
            if (!isalpha(static_cast<unsigned char>(input[i])) && input[i] != '_') {
                result += input[i];
                ++i;
                continue;
            }
            size_t start = i;
            
            while (i < input.size() && (isalpha(static_cast<unsigned char>(input[i])) || input[i] == '_')) {
                ++i;
            }
            
            std::string word = input.substr(start, i - start);
            std::string lowercase = lowercased(word);
            
            if (wordSet.count(lowercase)) {
                result += replacement;
                continue;
            }
            
            result += word;
        }
        
        return result;
    }
    
    static std::string trim(const std::string& s)
    {
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        return s.substr(a, b - a + 1);
    }

    static std::vector<std::string> split(const std::string& s, char c)
    {
        std::vector<std::string> r;
        std::stringstream ss(s);
        std::string item;

        while (std::getline(ss, item, c))
            r.push_back(trim(item));

        return r;
    }

    std::vector<std::string> getInterfaceRoutines(const std::string& source)
    {
        std::vector<std::string> result;

        std::regex interfaceRe(R"(\binterface\b([\s\S]*?)\bimplementation\b)", std::regex::icase);
        std::smatch interfaceMatch;

        if (!std::regex_search(source, interfaceMatch, interfaceRe))
            return result;

        std::string interfaceBlock = interfaceMatch[1].str();

        std::regex routineRe(R"(\b(?:procedure|function)\s+([A-Za-z_][A-Za-z0-9_]*)\b)", std::regex::icase);

        auto it = std::sregex_iterator(interfaceBlock.begin(), interfaceBlock.end(), routineRe);
        auto end = std::sregex_iterator();

        for (; it != end; ++it)
            result.push_back((*it)[1].str());

        return result;
    }

    static std::string addExportToImplementation(
        const std::string& source,
        const std::vector<std::string>& routines)
    {
        std::string result = source;

        // find implementation section
        std::regex implRe(R"(\bimplementation\b([\s\S]*))", std::regex::icase);
        std::smatch implMatch;

        if (!std::regex_search(result, implMatch, implRe))
            return result;

        std::string implBlock = implMatch[1].str();
        size_t implPos = implMatch.position(1);

        for (const auto& name : routines)
        {
            std::regex routineRe(
                "\\b(procedure|function)\\s+" + name + "\\b",
                std::regex::icase);

            implBlock = std::regex_replace(
                implBlock,
                routineRe,
                "EXPORT $1 " + name);
        }

        result.replace(implPos, implMatch.length(1), implBlock);

        return result;
    }

    static bool startsWithWord(const std::string& s, size_t pos, const std::string& word)
    {
        if (pos + word.size() > s.size())
            return false;

        for (size_t i = 0; i < word.size(); i++)
            if (tolower(s[pos + i]) != tolower(word[i]))
                return false;

        if (pos > 0 && std::isalnum((unsigned char)s[pos - 1]))
            return false;

        if (pos + word.size() < s.size() && std::isalnum((unsigned char)s[pos + word.size()]))
            return false;

        return true;
    }

    static bool isDecl(const std::string& s, size_t pos)
    {
        static const char* words[] = { "label","const","type","var","threadvar" };

        for (auto w : words)
            if (startsWithWord(s, pos, w))
                return true;

        return false;
    }

    static std::string moveBeginBeforeDeclarations(const std::string& src)
    {
        size_t beginPos = std::string::npos;

        for (size_t i = 0; i < src.size(); i++)
            if (startsWithWord(src, i, "begin"))
            {
                beginPos = i;
                break;
            }

        if (beginPos == std::string::npos)
            return src;

        size_t declStart = std::string::npos;

        for (size_t i = 0; i < beginPos; i++)
            if (isDecl(src, i))
            {
                declStart = i;
                break;
            }

        if (declStart == std::string::npos)
            return src;

        std::string beforeDecl = src.substr(0, declStart);
        std::string declBlock = src.substr(declStart, beginPos - declStart);
        std::string afterBegin = src.substr(beginPos + 5);

        std::string result;
        result += beforeDecl;
        result += "begin\n";
        result += declBlock;
        result += afterBegin;

        return result;
    }

    static std::string convertPascalCase(const std::string& src)
    {
        std::stringstream in(src);
        std::stringstream out;

        std::string line;
        std::string expr;
        bool inCase = false;

        while (getline(in, line))
        {
            std::string t = trim(line);

            if (!inCase)
            {
                size_t p = t.find("case ");
                if (p != std::string::npos && t.find(" of") != std::string::npos)
                {
                    expr = trim(t.substr(5, t.find("of") - 5));
                    out << "CASE\n";
                    inCase = true;
                }
                else
                    out << line << "\n";

                continue;
            }

            if (t == "end;" || t == "end")
            {
                out << "END;\n";
                inCase = false;
                continue;
            }

            if (t.rfind("else",0)==0)
            {
                std::string stmt = trim(t.substr(4));

                out << "DEFAULT\n";
                if (!stmt.empty())
                    out << stmt << "\n";

                continue;
            }

            size_t colon = t.find(':');
            if (colon == std::string::npos)
                continue;

            std::string selector = trim(t.substr(0, colon));
            std::string statement = trim(t.substr(colon+1));

            std::string cond;

            if (selector.find("..") != std::string::npos)
            {
                auto parts = split(selector,'.');
                cond = expr + " >= " + parts[0] + " AND " +
                       expr + " <= " + parts[2];
            }
            else if (selector.find(',') != std::string::npos)
            {
                auto vals = split(selector,',');

                for (size_t i=0;i<vals.size();i++)
                {
                    if (i) cond += " OR ";
                    cond += expr + " == " + vals[i];
                }
            }
            else
            {
                cond = expr + " == " + selector;
            }

            out << "IF " << cond << " THEN\n";
            out << statement << "\n";
            out << "END;\n";
        }

        return out.str();
    }


    static std::string removePascalTypes(const std::string& input)
    {
        std::regex re(R"(:\s*[^;]+;)");
        return std::regex_replace(input, re, ";");
    }

    static std::string convertPascalToPPL(const std::string& input)
    {
        std::string result = input;

        // function NAME(params): Type;
        std::regex funcRegex(
            R"(\bfunction\s+([A-Za-z_][A-Za-z0-9_]*)\s*(\([^)]*\))?\s*:\s*[A-Za-z0-9_]+\s*;)",
            std::regex::icase
        );

        // procedure NAME(params);
        std::regex procRegex(
            R"(\bprocedure\s+([A-Za-z_][A-Za-z0-9_]*)\s*(\([^)]*\))?\s*;)",
            std::regex::icase
        );

        result = std::regex_replace(result, funcRegex, "$1()");
        result = std::regex_replace(result, procRegex, "$1()");

        return result;
    }
    
    std::string convertPascalSyntax(const std::string &code) {
        std::string s;
        std::regex re;
        std::smatch matches;
        std::string output = code;
        
        auto routines = getInterfaceRoutines(output);
        output = addExportToImplementation(output, routines);
        output = moveBeginBeforeDeclarations(output);
        output = convertPascalCase(code);
        output = replaceWords(output, {"interface", "implementation"}, "");
        output = convertPascalToPPL(output);
        output = removePascalTypes(output);
        
        return output;
    }
}
