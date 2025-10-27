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


#include "common.hpp"
#include "singleton.hpp"

#include <sstream>
#include <algorithm>
#include <regex>


using pplplus::Singleton;

static bool _failed = false;

bool hasErrors(void) {
    return _failed;
}

std::ostream &operator<<(std::ostream &os, MessageType type) {
    Singleton *singlenton = Singleton::shared();

    if (!singlenton->currentSourceFilePath().empty()) {
        os << "📄 " << singlenton->currentSourceFilePath().filename().string() << ":";
        os << singlenton->currentLineNumber() << " ";
    }

    switch (type) {
        case MessageType::Error:
            os << "❌ error: ";
            _failed = true;
            break;
            
        case MessageType::CriticalError:
            os << "🛑 critical error: ";
            _failed = true;
            break;

        case MessageType::Warning:
            os << "⚠️ warning: ";
            break;
            
        case MessageType::Deprecated:
            os << "🪦 deprecated: ";
            break;

        default:
            os << "";
            break;
    }

    return os;
}

std::string &ltrim(std::string &str)
{
  auto it2 =  std::find_if( str.begin() , str.end() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
  str.erase( str.begin() , it2);
  return str;
}

std::string &rtrim(std::string &str) {
  auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
  str.erase( it1.base() , str.end() );
  return str;
}

std::string &trim(std::string &str) {
   return ltrim(rtrim(str));
}

std::string trim_copy(const std::string &str) {
   auto s = str;
   return ltrim(rtrim(s));
}

std::string &strip(std::string &str) {
    str.erase(remove(str.begin(), str.end(), ' '), str.end());
    return str;
}

std::string strip_copy(const std::string &str) {
    auto s = str;
    s.erase(remove(s.begin(), s.end(), ' '), s.end());
    return s;
}




int countLeadingCharacters(const std::string &str, const char character) {
    int count = 0;
    for (const char ch : str) {  // Declare 'ch' as const
        if (ch == character) {
            count++;
        } else {
            break;
        }
    }
    return count;
}



std::string normalizeWhitespace(const std::string& input) {
    std::string output;
    output.reserve(input.size());  // Optimize memory allocation

    bool in_whitespace = false;

    for (char ch : input) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!in_whitespace) {
                output += ' ';
                in_whitespace = true;
            }
        } else {
            output += ch;
            in_whitespace = false;
        }
    }

    return output;
}
