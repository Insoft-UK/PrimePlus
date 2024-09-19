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


#include "auto.hpp"

#include <sstream>
#include <regex>
#include "singleton.hpp"

using namespace pp;
using namespace std;

bool Auto::parse(string &str) {
    smatch m;
    regex r;
    size_t pos;
    Singleton *singleton = Singleton::shared();
    
    // Functions/Subroutines
    // TODO: ^ *(export +)?\bauto *: *((?:(?:[a-zA-Z_]\w*::)*?)[a-zA-Z][\w.]*) *(?=\()
    
    if (singleton->scope == Singleton::Scope::Global) {
        r = R"(\b(var|local|const) +)";
        if (regex_search(str, m, r)) {
            while ((pos = str.find("auto:")) != string::npos) {
                str.erase(pos, 4);
                ostringstream os;
                os << "g" << base10ToBase32(++_globalCount);
                str.insert(pos, os.str());
            }
        }

        r = R"(\bauto *(?=: *(?:(?:(?:[a-zA-Z_]\w*::)*?)[a-zA-Z][\w.]*) *(?=\()))";
        if (regex_search(str, m, r)) {
            ostringstream os;
            os << "fn" << base10ToBase32(++_fnCount);
            str.replace(m.position(), m.str().length(), os.str());
        }
        
        _paramCount = 0;
        while ((pos = str.find("auto:")) != string::npos) {
            ostringstream os;
            os << "p" << base10ToBase32(++_paramCount);
            str.replace(pos, 4, os.str());
        }
    }
    
    
    // Variables/Constants
    r = R"(\b(var|local|const) +)";
    if (regex_search(str, m, r)) {
        while ((pos = str.find("auto:")) != string::npos) {
            str.erase(pos, 4);
            ostringstream os;
            os << "v" << base10ToBase32(++_varCount);
            str.insert(pos, os.str());
        }
    }
    
    return true;
}

string Auto::base10ToBase32(unsigned int num) {
    if (num == 0) {
        return "0";  // Edge case: if the number is 0, return "0"
    }

    string result;
    const char digits[] = "0123456789ABCDEFGHIJKLMNabcdefgh";  // Base-32 digits
    
    // Keep dividing the number by 32 and store the remainders
    while (num > 0) {
        int remainder = num % 32;  // Get the current base-32 digit
        result += digits[remainder];  // Add the corresponding character
        num /= 32;  // Reduce the number
    }

    // The digits are accumulated in reverse order, so reverse the result string
    reverse(result.begin(), result.end());

    return result;
}



