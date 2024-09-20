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

#include <sys/time.h>

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
    std::regex r(R"(\s*([{}[\]()≤≥≠<>=*/+\-▶.,;:!^])\s*)");

    // Replace matches with the operator and no surrounding spaces
    std::string result = std::regex_replace(str, r, "$1");

    return result;
}

// MARK: - C To PPL Translater...

std::string translateCOperatorsToPPL(const std::string &str) {
    std::string s = str;
    std::smatch m;
    
    // Convert C/C++ style shorthand += -= *= /= %= &= |= ^= to PPL longhand
    while (regex_search(s, m, std::regex(R"([a-zA-Z]\w* *(\[.*\] *)*[*\/+\-%&|^]=)"))) {
        char op = m.str().at(m.str().find("=") - 1);
        std::string str = trim_copy(m.str().substr(0, m.str().find("=") - 1));
        s = s.replace(m.position(), m.length(), str + " := " + str + op);
    }
    s = regex_replace(s, std::regex(R"( *\% *)"), " MOD ");
    
    return s;
}

void translateCLogicalOperatorsToPPL(std::string& str) {
    str = regex_replace(str, std::regex(R"(&&)"), " AND ");
    str = regex_replace(str, std::regex(R"(\|\|)"), " OR ");
    str = regex_replace(str, std::regex(R"(!)"), " NOT ");
    str = regex_replace(str, std::regex(R"(\^\^)"), " XOR ");
}


// MARK: - P+ To PPL Translater...
void reformatPPLLine(std::string &str) {
    std::regex r;
    
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
    r = R"(≥|≤|≠|=|:=|\+|-|\*|\/)";
    str = regex_replace(str, r, " $0 ");
    
    // We now hand the issue of Unary Minus/Operator
    
    // Ensuring that `≥`, `≤`, `≠`, `=`, `+`, `-`, `*` and `/` have a whitespace befor `-`.
    r = R"(([≥≤≠=\+|\-|\*|\/]) +- +)";
    str = regex_replace(str, r, "$1 -");
    
    // Ensuring that `-` in  `{ - `, `( - ` and `[ - ` situations have no surrounding whitespace.
    r = R"(([({[]) +- +)";
    str = regex_replace(str, r, "$1-");
    
    if (!regex_search(str, std::regex(R"(LOCAL [A-Za-z]\w* = )"))) {
        // We can now safely convert `=` to `==` without affecting other operators.
        str = regex_replace(str, std::regex(R"( = )"), " == ");
    }
    
    if (Singleton::Scope::Local == Singleton::shared()->scope) {
        if (!regex_search(str, std::regex(R"(\b(BEGIN|IF|CASE|REPEAT|WHILE)\b)", std::regex_constants::icase))) {
            str.insert(0, std::string(Singleton::shared()->nestingLevel * INDENT_WIDTH, ' '));
        } else {
            str.insert(0, std::string((Singleton::shared()->nestingLevel - 1) * INDENT_WIDTH, ' '));
        }
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
    std::regex r;
    
    // We turn any keywords that are in lowercase to uppercase
    r = R"(\b(begin|end|return|kill|if|then|else|xor|or|and|not|case|default|iferr|ifte|for|from|step|downto|to|do|while|repeat|until|break|continue|export|const|local|key)\b)";
    for(std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), r); it != std::sregex_iterator(); ++it) {
        std::string result = it->str();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        str = str.replace(it->position(), it->length(), result);
    }
}


void translatePPlusLine(std::string &ln, std::ofstream &outfile) {
    std::regex r;
    std::smatch m;
    std::ifstream infile;
    
    Singleton *singleton = Singleton::shared();
    
    static bool multiLineComment = false;
    
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
    
    // Remove any leading white spaces before or after.
    trim(ln);
    
    if (ln.length() < 1) {
        ln = std::string("");
        return;
    }
    
    if (ln.substr(0,2) == "//") {
        ln = ln.insert(0, std::string(singleton->nestingLevel * INDENT_WIDTH, ' ') + "\n");
        return;
    }
    
    // Remove any comments.
    singleton->comments.preserveComment(ln);
    singleton->comments.removeComment(ln);
    
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
    
    r = R"(\#pragma mode *\(.*\)$)";
    if (std::regex_match(ln, r)) {
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
    
    ln = regex_replace(ln, std::regex(R"(>=)"), "≥");
    ln = regex_replace(ln, std::regex(R"(<=)"), "≤");
    ln = regex_replace(ln, std::regex(R"(!=)"), "≠");
    ln = regex_replace(ln, std::regex(R"(=>)"), "▶");
    
    ln = singleton->aliases.resolveAliasesInText(ln);

    
    if (structurs.parse(ln)) {
        ln = std::string("");
        return;
    }
    
    if (enumerators.parse(ln)) {
        ln = std::string("");
        return;
    }
    
    
    r = R"(\b(log|cos|sin|tan|ln|min|max)\b)";
    for(std::sregex_iterator it = std::sregex_iterator(ln.begin(), ln.end(), r); it != std::sregex_iterator(); ++it) {
        std::string result = it->str();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        ln = ln.replace(it->position(), it->length(), result);
    }
    
    r = R"(\b(begin|if|for|case|repeat|while|switch)\b)";
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), r); it != std::sregex_iterator(); ++it) {
        singleton->setNestingLevel(singleton->nestingLevel + 1);
    }
    
    r = R"(\b(w?end(if)?|until|next|loop)\b)";
    for(auto it = std::sregex_iterator(ln.begin(), ln.end(), r); it != std::sregex_iterator(); ++it) {
        singleton->setNestingLevel(singleton->nestingLevel - 1);
        if (0 == singleton->nestingLevel) {
            singleton->aliases.removeAllLocalAliases();
            ln += '\n';
        }
        if (singleton->nestingLevel < 0) {
            std::cout << MessageType::Error << "unexpected '" << it->str() << "'\n";
        }
    }
    

    if (singleton->scope == Singleton::Scope::Global) {
        r = R"(^ *(KS?A?_[A-Z\d][a-z]*) *$)";
        std::sregex_token_iterator it = std::sregex_token_iterator {
            ln.begin(), ln.end(), r, {1}
        };
        if (it != std::sregex_token_iterator()) {
            std::string s = *it;
            ln = "KEY " + s + "()";
        }
        
        r = R"(^ *(export +)?([a-zA-Z_]\w*((::)|\.))+[a-zA-Z_]\w* *(?=\())";
        ln = regex_replace(ln, r, "auto:$0");
    }

    ln = regex_replace(ln, std::regex(R"(<int>\((.*)\))"), "IP($1)");
    ln = regex_replace(ln, std::regex(R"(<string>\((.*)\))"), "STRING($1)");
    
    singleton->autoname.parse(ln);
    Alias::parse(ln);
    
    if (singleton->scope == Singleton::Scope::Local) {
        singleton->switches.parse(ln);
        
        if (regex_search(ln, std::regex(R"(\b(IF|WHILE|UNTIL|FOR)\b)", std::regex_constants::icase)))
            translateCLogicalOperatorsToPPL(ln);
        
        IFTE::parse(ln);
        ForNext::parse(ln);
        DoLoop::parse(ln);
        
        ln = regex_replace(ln, std::regex(R"(\btry\b)"), "IFERR");
        ln = regex_replace(ln, std::regex(R"(\bcatch\b)"), "THEN");
        ln = regex_replace(ln, std::regex(R"(\b(w?end(if)?|loop);)", std::regex_constants::icase), "END;");
        ln = regex_replace(ln, std::regex(R"(\.{3})"), " TO ");
    }
    
    
    singleton->calc.parse(ln);
    ln = translateCOperatorsToPPL(ln);
    Bitwise::parse(ln);
    ln = regex_replace(ln, std::regex(R"(=>)"), "▶");
    
    /*
     PPL uses := instead of C's = for assignment. Converting = to := in PPL turns every == into :=:= and := into ::=
     To fix this, we convert all :=:= to == and then all ::= to :=
     */
    ln = regex_replace(ln, std::regex(R"(=)"), ":=");
    ln = regex_replace(ln, std::regex(R"(:=:=)"), "==");
    ln = regex_replace(ln, std::regex(R"(::=)"), ":=");
    
    
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

void translatePPlusAndWriteLine(const std::string& str, std::ofstream &outfile)
{
    Singleton& singleton = *Singleton::shared();
    std::string ln = str;
    translatePPlusLine(ln, outfile);
    if (ln.length() < 2) ln = std::string("");

    writeUTF16Line(ln, outfile);
    
    singleton.incrementLineNumber();
}


void processAndWriteLines(std::istringstream &iss, std::ofstream &outfile)
{
    std::string s;
    
    while(getline(iss, s)) {
        translatePPlusAndWriteLine(s, outfile);
    }
}


void translatePPlusToPPL(const std::string &pathname, std::ofstream &outfile)
{
    Singleton& singleton = *Singleton::shared();
    std::ifstream infile;

    singleton.pushPathname(pathname);
    
    infile.open(pathname,std::ios::in);
    if (!infile.is_open()) exit(2);
    
    
    // Read in the whole of the file into a `std::string`
    std::string utf8;
    
    char c;
    while (!infile.eof()) {
        infile.get(c);
        utf8 += c;
        infile.peek();
    }
    
    /*
     Pre-correct any `THEN`, `DO` or `REPEAT` statements that are followed by other statements on the
     same line by moving the additional statement(s) to the next line. This ensures that the code
     is correctly processed, as it separates the conditional or loop structure from the subsequent
     statements for proper handling.
     */
    
    std::regex r;
    
    r = std::regex(R"(\b(THEN|DO|REPEAT)\b)", std::regex_constants::icase);
    utf8 = regex_replace(utf8, r, "$0\n");
    
    // Make sure all `LOCAL` are on seperate lines.
    r = std::regex(R"(\b(LOCAL|CONST|var)\b)", std::regex_constants::icase);
    utf8 = regex_replace(utf8, r, "\n$0");

    r = std::regex(R"(\b(END|endif|wend|next);)", std::regex_constants::icase);
    utf8 = regex_replace(utf8, r, "\n$0");
    
    r = R"(\/\/.*$;)";
    utf8 = regex_replace(utf8, r, "\n$0");
    
    std::istringstream iss;
    iss.str(utf8);
    
    processAndWriteLines(iss, outfile);
    
    infile.close();
    singleton.popPathname();
}


// MARK: - Command Line
void version(void) {
    std::cout 
    << "P+ Pre-Processor v"
    << (unsigned)__BUILD_NUMBER / 100000 << "."
    << (unsigned)__BUILD_NUMBER / 10000 % 10 << "."
    << (unsigned)__BUILD_NUMBER / 1000 % 10 << "."
    << std::setfill('0') << std::setw(3) << (unsigned)__BUILD_NUMBER % 1000
    << "\n";
}

void error(void) {
    printf("p+: try 'p+ --help' for more information\n");
}

void info(void) {
    std::cout << "Copyright (c) 2023-2024 Insoft. All rights reserved\n";
    int rev = (unsigned)__BUILD_NUMBER / 1000 % 10;
    std::cout << "P+ Pre-Processor v" << (unsigned)__BUILD_NUMBER / 100000 << "." << (unsigned)__BUILD_NUMBER / 10000 % 10 << (rev ? "." + std::to_string(rev) : "") << " BUILD " << std::setfill('0') << std::setw(3) << __BUILD_NUMBER % 1000 << "\n\n";
}

void usage(void) {
    info();
    std::cout << "usage: p+ in-file [-o out-file] [-b flags] [-l] [-m]\n\n";
    std::cout << " -o, --out         file\n";
    std::cout << " -v, --verbose     display detailed processing information\n";
    std::cout << " verbose :- flags\n";
    std::cout << "            a aliases\n";
    std::cout << "            c comments\n";
    std::cout << "            l calc\n";
    std::cout << "            e enumerator\n";
    std::cout << "            p preprocessor\n";
    std::cout << "            w structs\n\n";
    std::cout << " -l, --path        #include <...> search path.\n";
    std::cout << " -m, --min         minify ppl code.\n";
    std::cout << " -t, --tab         tab width in spaces.\n";
    std::cout << " -h, --help        help.\n";
    std::cout << " --version         displays the full version number.\n";
}


// MARK: - Main
int main(int argc, char **argv) {
    bool pragma = false;
    std::string in_filename, out_filename;

    if ( argc == 1 )
    {
        error();
        exit(100);
    }
    
    for( int n = 1; n < argc; n++ ) {
        std::string args(argv[n]);
        
        if ( args == "-o" || args == "--out" )
        {
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
        
        if ( args == "--help" ) {
            usage();
            exit(102);
        }
        
        if ( strcmp( argv[n], "--pragma" ) == 0 ) {
            pragma = true;
        }
        
        if ( strcmp( argv[n], "--version" ) == 0 ) {
            version();
            return 0;
        }
        
        
        
        if ( strcmp( argv[n], "-t" ) == 0 || strcmp( argv[n], "--tab" ) == 0 ) {
            if ( n + 1 >= argc ) {
                error();
                exit(104);
            }
            tabWidth = atoi(argv[n]);
            continue;
        }
        
        if ( args == "-v" || args == "--verbose" ) {
            if ( n + 1 >= argc ) {
                error();
                exit(103);
            }
            args = argv[++n];
            if ( args == "-" ) {
                Singleton::shared()->aliases.verbose = true;
                Singleton::shared()->comments.verbose = true;
                Singleton::shared()->calc.verbose = true;
                enumerators.verbose = true;
                structurs.verbose = true;
                preprocessor.verbose = true;
                Singleton::shared()->switches.verbose = true;
                continue;
            }
            
            if (args.find("a") != std::string::npos) Singleton::shared()->aliases.verbose = true;
            if (args.find("c") != std::string::npos) Singleton::shared()->comments.verbose = true;
            if (args.find("l") != std::string::npos) Singleton::shared()->calc.verbose = true;
            if (args.find("e") != std::string::npos) enumerators.verbose = true;
            if (args.find("s") != std::string::npos) structurs.verbose = true;
            if (args.find("p") != std::string::npos) preprocessor.verbose = true;
            if (args.find("w") != std::string::npos) Singleton::shared()->switches.verbose = true;
        
            continue;
        }
        
        if ( strcmp( argv[n], "-l" ) == 0 || strcmp( argv[n], "--path" ) == 0 ) {
            if ( ++n >= argc ) {
                error();
                return 0;
            }
            preprocessor.path = std::string(argv[n]);
            if (preprocessor.path.at(preprocessor.path.length() - 1) != '/') preprocessor.path.append("/");
            continue;
        }
        
        in_filename = argv[n];
        std::regex r(R"(.\w*$)");
        std::smatch extension;
        if (regex_search(in_filename, extension, r)) {
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
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    std::string str;
    str = "#define __pplus";
    preprocessor.parse(str);
    str = "#define __SCREEN G0";
    preprocessor.parse(str);
    str = R"(#define __LIST_LIMIT 10000)";
    preprocessor.parse(str);
    str = R"(#define __VERSION 2.0)";
    preprocessor.parse(str);
    
    
    
    translatePPlusToPPL(in_filename, outfile);
    
    // Stop measuring time and calculate the elapsed time.
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Display elasps time in secononds.
    double delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("Completed in %.3f seconds.\n", delta_us * 1e-6);
    
    outfile.close();
    
    if (hasErrors() == true) {
        std::cout << "ERRORS!\n";
        remove(out_filename.c_str());
        return 0;
    }

    
    std::cout << "UTF-16LE File '" << out_filename << "' Succefuly Created.\n";
    
    
    return 0;
}
