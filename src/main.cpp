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

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <ctime>

#include "timer.hpp"
#include "singleton.hpp"
#include "common.hpp"

#include "preprocessor.hpp"
#include "structs.hpp"
#include "enums.hpp"
#include "def.hpp"
#include "alias.hpp"
#include "strings.hpp"
#include "ifte.hpp"
#include "bitwise.hpp"
#include "for_next.hpp"
#include "do_loop.hpp"
#include "calc.hpp"

#include "build.h"

#define PPLUS_TAB_WIDTH  4

int tabWidth = PPLUS_TAB_WIDTH;

using namespace pp;

static Preprocessor preprocessor = Preprocessor();
static Enums enumerators = Enums();
static Structs structurs = Structs();
static Strings strings = Strings();


void terminator() {
  std::cout << MessageType::Error << "An internal preprocessing problem occurred. Please review the syntax before this point.\n";
  exit(-1);
}
 
void (*old_terminate)() = std::set_terminate(terminator);


void translatePPlusToPPL(const std::string &pathname, std::ofstream &outfile);

// MARK: - Utills



/*
 The decimalToBase24 function converts a given
 base 10 integer into its base 24 representation using a
 specific set of characters. The character set is
 comprised of the following 24 symbols:

     •    Numbers: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
     •    Letters: C, D, F, H, J, K, M, N, R, U, V, W, X, Y
     
 Character Selection:
 The choice of characters was made to avoid confusion
 with common alphanumeric representations, ensuring
 that each character is visually distinct and easily
 recognizable. This set excludes characters that closely
 resemble each other or numerical digits, promoting
 clarity in representation.
 */
static std::string decimalToBase24(long num) {
    if (num == 0) {
        return "C";
    }

    const std::string base24Chars = "0123456789CDFHJKMNRUVWXY";
    std::string base24;

    while (num > 0) {
        int remainder = num % 24;
        base24 = base24Chars[remainder] + base24; // Prepend character
        num /= 24; // Integer division
    }

    return base24;
}

/*
 The `base24ToDecimal` function converts a
 base 24 string (using the characters
 `"0123456789CDFHJKMNRUVWXY"`) into its
 decimal (base 10) equivalent. It validates the
 input for empty strings and invalid characters,
 making it the inverse of the
 `decimalToBase24` function.
 */
//static long base24ToDecimal(const std::string& base24) {
//    if (base24.empty()) {
//        throw std::invalid_argument("Input string is empty.");
//    }
//
//    const std::string base24Chars = "0123456789CDFHJKMNRUVWXY";
//    long decimalValue = 0;
//
//    for (size_t i = 0; i < base24.length(); ++i) {
//        char currentChar = base24[base24.length() - 1 - i]; // Reverse position
//        size_t index = base24Chars.find(currentChar);
//
//        if (index == std::string::npos) {
//            throw std::invalid_argument("Invalid character in base 24 string.");
//        }
//
//        decimalValue += index * static_cast<long>(pow(24, i)); // Calculate value
//    }
//
//    return decimalValue;
//}

static std::string getBuildCode(void) {
    std::string str;
    int majorVersionNumber = BUILD_NUMBER / 100000;
    str = std::to_string(majorVersionNumber) + decimalToBase24(BUILD_NUMBER - majorVersionNumber * 100000);
    return str;
}

uint32_t utf8_to_utf16(const char *str) {
    uint8_t *utf8 = (uint8_t *)str;
    uint16_t utf16 = *utf8;
    
    if ((utf8[0] & 0b11110000) == 0b11100000) {
        utf16 = utf8[0] & 0b11111;
        utf16 <<= 6;
        utf16 |= utf8[1] & 0b111111;
        utf16 <<= 6;
        utf16 |= utf8[2] & 0b111111;
        return utf16;
    }
    
    // 110xxxxx 10xxxxxx
    if ((utf8[0] & 0b11100000) == 0b11000000) {
        utf16 = utf8[0] & 0b11111;
        utf16 <<= 6;
        utf16 |= utf8[1] & 0b111111;
        return utf16;
    }
    
    return utf16;
}

// Function to remove whitespaces around specific operators using regular expressions
std::string removeWhitespaceAroundOperators(const std::string& str) {
    // Regular expression pattern to match spaces around the specified operators
    // Operators: {}[]()≤≥≠<>=*/+-▶.,;:!^
    std::regex re(R"(\s*([{}[\]()≤≥≠<>=*\/+\-▶.,;:!^&|%])\s*)");

    // Replace matches with the operator and no surrounding spaces
    std::string result = std::regex_replace(str, re, "$1");
    
    return result;
}

// MARK: - C To PPL Translater...

/*
 Parses and converts C-style expressions like a+=b to PPL form like a := a + b
 */
std::string expandAssignment(const std::string& expression) {
    std::string str = expression;
    std::regex re(R"(([A-Za-z]\w*(?:\[.*\])*)([*\/+\-&|^%]|(?:>>|<<))=)");
    str = regex_replace(str, re, "$1:=$1$2");
    
    re = R"(%)";
    str = regex_replace(str, re, " MOD ");
    
    return str;
}

void translateCLogicalOperatorsToPPL(std::string& str) {
    str = regex_replace(str, std::regex(R"(&&)"), " AND ");
    str = regex_replace(str, std::regex(R"(\|\|)"), " OR ");
    str = regex_replace(str, std::regex(R"(!)"), " NOT ");
    str = regex_replace(str, std::regex(R"(\^\^)"), " XOR ");
}


// MARK: - P+ To PPL Translater...
void reformatPPLLine(std::string &str) {
    std::regex re;
    
    str = removeWhitespaceAroundOperators(str);
    
    str = regex_replace(str, std::regex(R"(,)"), ", ");
    str = regex_replace(str, std::regex(R"(\{)"), "{ ");
    str = regex_replace(str, std::regex(R"(\})"), " }");
    str = regex_replace(str, std::regex(R"(^ +(\} *;))"), "$1\n");
    str = regex_replace(str, std::regex(R"(\{ +\})"), "{}");
    
    /*
     To prevent correcting over-modifications, first replace all double `==` with a single `=`.
     
     Converting standalone `=` to `==` initially can lead to unintended changes like `<=`, `>=`,
     `:=`, and `==` turning into `<==`, `>==`, `:==`, and `===`.
     
     By first reverting all double `==` back to a single `=`, and ensuring that only standalone
     `=` or `:=` with surrounding whitespace are targeted, we can then safely convert `=` to `==`
     without affecting other operators.
     */
    str = regex_replace(str, std::regex(R"(==)"), "=");

    // Ensuring that standalone `≥`, `≤`, `≠`, `=`, `:=`, `+`, `-`, `*` and `/` have surrounding whitespace.
    re = R"(≥|≤|≠|=|:=|\+|-|\*|\/|▶)";
    str = regex_replace(str, re, " $0 ");
    
    // We now hand the issue of Unary Minus/Operator
    
    // Ensuring that `≥`, `≤`, `≠`, `=`, `+`, `-`, `*` and `/` have a whitespace befor `-`.
    re = R"(([≥≤≠=\+|\-|\*|\/]) +- +)";
    str = regex_replace(str, re, "$1 -");
    
    // Ensuring that `-` in  `{ - `, `( - ` and `[ - ` situations have no surrounding whitespace.
    re = R"(([({[]) +- +)";
    str = regex_replace(str, re, "$1-");
    
    if (!regex_search(str, std::regex(R"(LOCAL [A-Za-z]\w* = )"))) {
        // We can now safely convert `=` to `==` without affecting other operators.
        str = regex_replace(str, std::regex(R"( = )"), " == ");
    }
    
    if (Singleton::Scope::Local == Singleton::shared()->scope) {
        if (!regex_search(str, std::regex(R"(\b(BEGIN|IF|CASE|REPEAT|WHILE|FOR|ELSE|IFERR)\b)"))) {
            str.insert(0, std::string(Singleton::shared()->nestingLevel * INDENT_WIDTH, ' '));
        } else {
            str.insert(0, std::string((Singleton::shared()->nestingLevel - 1) * INDENT_WIDTH, ' '));
        }
        
        re = std::regex(R"(^ *\b(THEN)\b)", std::regex_constants::icase);
        str = regex_replace(str, re, std::string((Singleton::shared()->nestingLevel - 1) * INDENT_WIDTH, ' ') + "$1");
        
    
        str = regex_replace(str, std::regex(R"(\(\s*\))"), "");
        
        if (regex_search(str, std::regex(R"(\bEND;$)"))) {
            str = regex_replace(str, std::regex(R"(;(.+))"), ";\n" + std::string(Singleton::shared()->nestingLevel * INDENT_WIDTH, ' ') + "$1");
        } else {
            str = regex_replace(str, std::regex(R"(;(.+))"), ";\n" + std::string((Singleton::shared()->nestingLevel - 1) * INDENT_WIDTH, ' ') + "$1");
        }
    }
    
    if (Singleton::Scope::Global == Singleton::shared()->scope) {
        str = regex_replace(str, std::regex(R"(END;)"), "$0\n");
        str = regex_replace(str, std::regex(R"(LOCAL )"), "");
    }
}

void capitalizeKeywords(std::string &str) {
    std::string result = str;
    std::regex re;
    
    // We turn any keywords that are in lowercase to uppercase
    re = R"(\b(begin|end|return|kill|if|then|else|xor|or|and|not|case|default|iferr|ifte|for|from|step|downto|to|do|while|repeat|until|break|continue|export|const|local|key)\b)";
    for(std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), re); it != std::sregex_iterator(); ++it) {
        std::string result = it->str();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        str = str.replace(it->position(), it->length(), result);
    }
}


void translatePPlusLine(std::string &ln, std::ofstream &outfile) {
    std::regex re;
    std::smatch m;
    std::ifstream infile;
    
    Singleton *singleton = Singleton::shared();
    
    static bool multiLineComment = false;
    static int consecutiveBlankLines = 0;
    
    ln = regex_replace(ln, std::regex(R"(\t)"), " "); // Tab to space, future reg-ex will not require to deal with '\t', only spaces.
    
    /*
     While parsing the contents, strings may inadvertently undergo parsing, leading
     to potential disruptions in the string's content.
     To address this issue, we prioritize the preservation of any existing strings.
     Subsequently, after parsing, any strings that have been universally altered can
     be restored to their original state.
     */
    strings.preserveStrings(ln);
    
    
    
    if (multiLineComment) {
        multiLineComment = !regex_search(ln, std::regex(R"(\*\/ *$)"));
        ln = std::string("");
        return;
    }
    
    if (std::regex_match(ln, std::regex(R"(^\ *\/\*)"))) {
        multiLineComment = true;
        ln = std::string("");
        return;
    }
    
    if (preprocessor.disregard == true) {
        preprocessor.parse(ln);
        ln = std::string("");
        return;
    }
    
    if (preprocessor.ppl) {
        // We're presently handling PPL code.
        preprocessor.parse(ln);
        if (!preprocessor.ppl) {
            // Signaling PPL code ending with the #end preprocessor, we clear the line to discard #end, and return to the calling function.
            ln = std::string("");
            return;
        }
        ln += '\n';
        return;
    }
    
    if (preprocessor.python) {
        // We're presently handling Python code.
        preprocessor.parse(ln);
        ln += '\n';
        return;
    }
    
    // Remove any leading white spaces before or after.
    trim(ln);
    
    if (ln.empty()) {
        ln = "";
        if (!consecutiveBlankLines++) {
            ln += '\n';
        }
        return;
    }
    consecutiveBlankLines = 0;
    
    if (ln.substr(0,2) == "//") {
        ln = ln.insert(0, std::string(singleton->nestingLevel * INDENT_WIDTH, ' '));
        ln += '\n';
        return;
    }
    
    // Remove any comments.
    re = R"(\/\* *(.*)\*\/ *)";
    ln = regex_replace(ln, re, "// $1\n" + std::string(singleton->nestingLevel * INDENT_WIDTH, ' ')); // Convert any C style `/* comment */` to PPL style `// comment` first.
    singleton->comments.preserveComment(ln);
    singleton->comments.removeComment(ln);
    
   
    
    re = R"(\#pragma mode *\(.*\)$)";
    if (std::regex_match(ln, re)) {
        ln += '\n';
        return;
    }
    
    if (preprocessor.parse(ln)) {
        if (preprocessor.python) {
            // Indicating Python code ahead with the #PYTHON preprocessor, we maintain the line unchanged and return to the calling function.
            ln += '\n';
            return;
        }

        if (!preprocessor.pathname.empty()) {
            // Flagged with #include preprocessor for file inclusion, we process it before continuing.
            translatePPlusToPPL(preprocessor.pathname, outfile);
        }

        ln = std::string("");
        return;
    }
    
    if (Def::parse(ln)) return;
    ln = removeWhitespaceAroundOperators(ln);
     
    /*
     In C++, the standard library provides support for regular expressions
     through the <regex> library, but it does not support lookbehind
     assertions (such as (?<!...)) directly, as they are not part of the
     regular expressions supported by the C++ Standard Library.

     However, we can work around this limitation by adjusting your regular
     expression to achieve the same result using alternative techniques.
     
     This approach doesn’t fully replicate lookbehind functionality, but
     it can be effective for simpler cases where a limited lookbehind is
     required.
     */
    std::string::const_iterator it(ln.cbegin());
    re = R"((?:[^<>=]|^)(>=|!=|<>|<=|=>)(?!=[<>=]))";
    
    //TODO: Remove the Hack
    int hack = 0; // a hack for now to avoid any infinite loop
    while (std::regex_search(it, ln.cend(), m, re)) {
        // We will convert any >= != <= or => to PPLs ≥ ≠ ≤ and ▶
        std::string s = m.str(1);
        
        // Replace the operator with the appropriate PPL symbol.
        if (s == ">=") s = "≥";
        if (s == "!=") s = "≠";
        if (s == "<>") s = "≠";
        if (s == "<=") s = "≤";
        if (s == "=>") s = "▶";
        
        ln = ln.replace(m.position(1), m.length(1), s);
        
        // Reset the iterator to the beginning
        it = ln.cbegin();
        
        // Advance the iterator to the position just after the current match
//        std::advance(it, m.position(1) + s.length() + std::distance(ln.cbegin(), it));
        if (++hack > 100) break;
        
    }
    
    //MARK: - namespace parsing
    
    re = R"(^using namespace ([A-Za-z](?:\w+|::[A-Za-z]+)*);$)";
    if (regex_search(ln, m, re)) {
        singleton->aliases.addNamespace(m[1].str());
        ln = "";
        return;
    }
    
    re = R"(^unuse namespace ([A-Za-z](?:\w+|::[A-Za-z]+)*);$)";
    if (regex_search(ln, m, re)) {
        singleton->aliases.removeNamespace(m[1].str());
        ln = "";
        return;
    }
    
    //MARK: -
    
    ln = expandAssignment(ln);
    Bitwise::parse(ln);
    
    // PPL uses := instead of C's = for assignment. Converting all = to PPL style :=
    re = R"(([^:=]|^)(?:=)(?!=))";
    ln = std::regex_replace(ln, re, "$1 := ");
    
    ln = singleton->aliases.resolveAllAliasesInText(ln);

    
    if (structurs.parse(ln)) {
        ln = std::string("");
        return;
    }
    
    if (enumerators.parse(ln)) {
        ln = std::string("");
        return;
    }
    
    
    re = R"(\b(log|cos|sin|tan|ln|min|max)\b)";
    for(std::sregex_iterator it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        std::string result = it->str();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        ln = ln.replace(it->position(), it->length(), result);
    }
    
    re = R"(\b(begin|if|for|case|repeat|while|switch|try)\b)";
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->setNestingLevel(singleton->nestingLevel + 1);
    }
    
    re = R"(\b(w?end(if)?|until|next|loop)\b)";
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), re); it != std::sregex_iterator(); ++it) {
        singleton->setNestingLevel(singleton->nestingLevel - 1);
        if (0 == singleton->nestingLevel) {
            singleton->aliases.removeAllLocalAliases();
            structurs.removeAllLocalStructs();
            ln += '\n';
        }
        if (singleton->nestingLevel < 0) {
            std::cout << MessageType::Error << "unexpected '" << it->str() << "'\n";
        }
    }
    

    if (singleton->scope == Singleton::Scope::Global) {
        re = R"(^ *(KS?A?_[A-Z\d][a-z]*) *$)";
        std::sregex_token_iterator it = std::sregex_token_iterator {
            ln.begin(), ln.end(), re, {1}
        };
        if (it != std::sregex_token_iterator()) {
            std::string s = *it;
            ln = "KEY " + s + "()";
        }
        
        re = R"(^ *(export +)?([a-zA-Z_]\w*((::)|\.))+[a-zA-Z_]\w* *(?=\())";
        ln = regex_replace(ln, re, "auto:$0");
    }

    ln = regex_replace(ln, std::regex(R"(<int>\((.*)\))"), "IP($1)");
    ln = regex_replace(ln, std::regex(R"(<string>\((.*)\))"), "STRING($1)");
    
    singleton->autoname.parse(ln);
    Alias::parse(ln);
    
    if (singleton->scope == Singleton::Scope::Local) {
        singleton->switches.parse(ln);
        
        if (regex_search(ln, std::regex(R"(\b(IF|WHILE|UNTIL|FOR|TRY)\b)", std::regex_constants::icase)))
            translateCLogicalOperatorsToPPL(ln);
        
        IFTE::parse(ln);
        ForNext::parse(ln);
        DoLoop::parse(ln);
        
        ln = regex_replace(ln, std::regex(R"(\btry\b)"), "IFERR");
        ln = regex_replace(ln, std::regex(R"(\bcatch\b)"), "THEN");
        ln = regex_replace(ln, std::regex(R"(\b(w?end(if)?|loop);)", std::regex_constants::icase), "END;");
        ln = regex_replace(ln, std::regex(R"(\.{3})"), " TO ");
    }
    
    
    Calc::parse(ln);
    
    capitalizeKeywords(ln);
    reformatPPLLine(ln);
    
    strings.restoreStrings(ln);
    singleton->comments.restoreComment(ln);
    
    ln += '\n';
}

void writeUTF16Line(const std::string& ln, std::ofstream& outfile) {
    for ( int n = 0; n < ln.length(); n++) {
        uint8_t *ascii = (uint8_t *)&ln.at(n);
        if (ln.at(n) == '\r') continue;
        
        // Output as UTF-16LE
        if (*ascii >= 0x80) {
            uint16_t utf16 = utf8_to_utf16(&ln.at(n));
            
#ifndef __LITTLE_ENDIAN__
            utf16 = utf16 >> 8 | utf16 << 8;
#endif
            outfile.write((const char *)&utf16, 2);
            if ((*ascii & 0b11100000) == 0b11000000) n++;
            if ((*ascii & 0b11110000) == 0b11100000) n+=2;
            if ((*ascii & 0b11111000) == 0b11110000) n+=3;
        } else {
            outfile.put(ln.at(n));
            outfile.put('\0');
        }
    }
}

void translatePPlusAndWriteLine(const std::string& str, std::ofstream& outfile)
{
    std::string ln = str;
    translatePPlusLine(ln, outfile);
    writeUTF16Line(ln, outfile);
}


void processAndWriteLines(std::istringstream& iss, std::ofstream& outfile)
{
    std::string str;
    
    while(getline(iss, str)) {
        translatePPlusAndWriteLine(str, outfile);
    }
}


void translatePPlusToPPL(const std::string& pathname, std::ofstream& outfile)
{
    Singleton& singleton = *Singleton::shared();
    std::ifstream infile;
    std::regex re;
    std::smatch m;
    std::string utf8;

    singleton.pushPathname(pathname);
    
    infile.open(pathname,std::ios::in);
    if (!infile.is_open()) exit(2);
    
    while(getline(infile, utf8)) {
        std::istringstream iss;
        
        if (!preprocessor.ppl && !preprocessor.python) {
            re = std::regex(R"(\b(THEN|DO|REPEAT)\b(.*\S+))", std::regex_constants::icase);
            // Adds a newline only if there is content after THEN, DO, or REPEAT
            utf8 = std::regex_replace(utf8, re, "$1\n$2");
            
            // Make sure all `LOCAL` are on seperate lines.
            re = std::regex(R"((\S+.*)\b(LOCAL|CONST|var)\b)", std::regex_constants::icase);
            utf8 = regex_replace(utf8, re, "$1\n$2");
            
            // All `END`, `endif`, `wend`, and `next` must also be on separate lines,
            // but no newline is added if they are already at the start of the line.
            re = std::regex(R"((\S+.*)\b((?:END|endif|wend|next);))", std::regex_constants::icase);
            utf8 = std::regex_replace(utf8, re, "$1\n$2");
            
            re = R"(\/\/.*$;)";
            utf8 = regex_replace(utf8, re, "\n$0");
        }
        
        iss.str(utf8);
            
        processAndWriteLines(iss, outfile);
        Singleton::shared()->incrementLineNumber();
    }
    
    infile.close();
    singleton.popPathname();
}


// MARK: - Command Line
void version(void) {
    int major = BUILD_NUMBER / 100000;
    int minor = BUILD_NUMBER / 10000 % 10;
    int revision = BUILD_NUMBER / 1000 % 10;
    
    std::cout << "Insoft P+ Pre-Processor, version "
    << major << "."
    << minor << "."
    << revision
    << " (BUILD " << getBuildCode() << ")\n";
    
    std::cout << "Copyright (C) 2024 Insoft. All rights reserved.\n";
    std::cout << "Built on: " << CURRENT_DATE << "\n";
    std::cout << "Licence: MIT License\n\n";
    std::cout << "For more information, visit: http://www.insoft.uk\n";
}

void error(void)
{
    std::cout << "p+: try 'p+ -help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout << "Copyright (c) 2024 Insoft. All rights reserved.\n";
    int rev = BUILD_NUMBER / 1000 % 10;
    std::cout << "Insoft P+ Pre-Processor version " << BUILD_NUMBER / 100000 << "." << BUILD_NUMBER / 10000 % 10 << (rev ? "." + std::to_string(rev) : "")
    << " (BUILD " << getBuildCode() << "-" << decimalToBase24(BUILD_DATE) << ")\n\n";
}

void help(void) {
    int rev = BUILD_NUMBER / 1000 % 10;
    
    std::cout << "Copyright (C) 2024 Insoft. All rights reserved.\n";
    std::cout << "Insoft P+ Pre-Processor version " << BUILD_NUMBER / 100000 << "." << BUILD_NUMBER / 10000 % 10 << (rev ? "." + std::to_string(rev) : "")
    << " (BUILD " << getBuildCode() << "-" << decimalToBase24(BUILD_DATE) << ")\n\n";
    std::cout << "Usage: p+ <input-file> [-o <output-file>] [-b <flags>] [-l <pathname>]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output-file>        Specify the filename for generated PPL code.\n";
    std::cout << "  -v                      Display detailed processing information.\n\n";
    std::cout << "  Verbose Flags:\n";
    std::cout << "    a                     Aliases\n";
    std::cout << "    e                     Enumerator\n";
    std::cout << "    p                     Preprocessor\n";
    std::cout << "    s                     Structs\n\n";
    std::cout << "\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  ansiart {-version | -help}\n";
    std::cout << "    -version              Display the version information.\n";
    std::cout << "    -help                 Show this help message.\n";
}


// MARK: - Main
int main(int argc, char **argv) {
    std::string in_filename, out_filename;

    if (argc == 1) {
        error();
        exit(100);
    }
    
    for (int n = 1; n < argc; n++) {
        std::string args(argv[n]);
        
        if (args == "-o") {
            if ( n + 1 >= argc ) {
                error();
                exit(101);
            }
            out_filename = argv[n + 1];
            if (out_filename.substr(out_filename.length() - 7).compare(".hpprgm") != 0) {
                out_filename += ".hpprgm";
            }

            n++;
            continue;
        }
        
        if ( args == "-help" ) {
            help();
            return 0;
        }

        
        if ( strcmp( argv[n], "-version" ) == 0 ) {
            version();
            return 0;
        }
        
        
        if ( args == "-v" ) {
            if ( n + 1 >= argc ) {
                error();
                exit(103);
            }
            args = argv[++n];
            if ( args == "-" ) {
                Singleton::shared()->aliases.verbose = true;
                Singleton::shared()->comments.verbose = true;
                enumerators.verbose = true;
                structurs.verbose = true;
                preprocessor.verbose = true;
                Singleton::shared()->switches.verbose = true;
                continue;
            }
            
            if (args.find("a") != std::string::npos) Singleton::shared()->aliases.verbose = true;
            if (args.find("e") != std::string::npos) enumerators.verbose = true;
            if (args.find("s") != std::string::npos) structurs.verbose = true;
            if (args.find("p") != std::string::npos) preprocessor.verbose = true;
        
            continue;
        }
        
        if (args == "-l") {
            if (++n >= argc) {
                error();
                return 0;
            }
            preprocessor.path = std::string(argv[n]);
            if (preprocessor.path.at(preprocessor.path.length() - 1) != '/') preprocessor.path.append("/");
            continue;
        }
        
        in_filename = argv[n];
        std::regex re(R"(.\w*$)");
        std::smatch extension;
        if (regex_search(in_filename, extension, re)) {
            if (".ppl" == extension.str()) preprocessor.ppl = true;
        }
    }
    
    if (!out_filename.length()) {
        out_filename = in_filename;
        if (out_filename.rfind(".")) {
            out_filename.replace(out_filename.rfind("."), out_filename.length() - out_filename.rfind("."), ".hpprgm");
        }
    }
    
    info();
    
    
    std::ofstream outfile;
    outfile.open(out_filename, std::ios::out | std::ios::binary);
    if(!outfile.is_open())
    {
        error();
        return 0;
    }
    
    // The "hpprgm" file format requires UTF-16LE.
    outfile.put(0xFF);
    outfile.put(0xFE);
    
    // Start measuring time
    Timer timer;
    
    std::string str;
    str = "#define __pplus";
    preprocessor.parse(str);
    str = "#define __SCREEN G0";
    preprocessor.parse(str);
    str = R"(#define __LIST_LIMIT 10000)";
    preprocessor.parse(str);
    str = R"(#define __VERSION )" + std::to_string(BUILD_NUMBER / 100000) + std::to_string(BUILD_NUMBER / 10000 % 10) + std::to_string(BUILD_NUMBER / 1000 % 10);
    preprocessor.parse(str);
    
//    Singleton::shared()->aliases.addNamespace("std");
    
    translatePPlusToPPL(in_filename, outfile);
    
    // Stop measuring time and calculate the elapsed time.
    long long elapsed_time = timer.elapsed();

    
    outfile.close();
    
    if (hasErrors() == true) {
        std::cout << "ERRORS!\n";
        remove(out_filename.c_str());
        return 0;
    }

    // Display elasps time in secononds.
    std::cout << "Completed in " << std::fixed << std::setprecision(2) << elapsed_time / 1e9 << " seconds\n";
    std::cout << "UTF-16LE File '" << out_filename << "' Succefuly Created.\n";
    
    
    return 0;
}
