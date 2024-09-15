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

#include "Singleton.hpp"
#include "common.hpp"

#include "Preprocessor.hpp"
#include "Structs.hpp"
#include "Enums.hpp"
#include "Def.hpp"
#include "Alias.hpp"
#include "Strings.hpp"
#include "IFTE.hpp"
#include "Bitwise.hpp"
//#include "Messages.hpp"
#include "Calc.hpp"

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


void preProcess(std::string &ln, std::ofstream &outfile);

// MARK: - Utills

void condence(std::string &str) {
    str = regex_replace(str, std::regex(R"(\s*:=\s*)"), ":=");
    str = regex_replace(str, std::regex(R"(\s*=\s*)"), "=");
    str = regex_replace(str, std::regex(R"(\s*==\s*)"), "==");
    str = regex_replace(str, std::regex(R"(\s*\+\s*)"), "+");
    str = regex_replace(str, std::regex(R"(\s*-\s*)"), "-");
    str = regex_replace(str, std::regex(R"(\s*\/\s*)"), "/");
    str = regex_replace(str, std::regex(R"(\s*\*\s*)"), "*");
    str = regex_replace(str, std::regex(R"(\s*\,\s*)"), ",");
    str = regex_replace(str, std::regex(R"(\s*\{\s*)"), "{");
    str = regex_replace(str, std::regex(R"(\s*\}\s*)"), "}");
    str = regex_replace(str, std::regex(R"(\s*>\s*)"), ">");
    str = regex_replace(str, std::regex(R"(\s*<\s*)"), "<");
    str = regex_replace(str, std::regex(R"(\s*≥\s*)"), "≥");
    str = regex_replace(str, std::regex(R"(\s*≤\s*)"), "≤");
    str = regex_replace(str, std::regex(R"(\s*≠\s*)"), "≠");
    str = regex_replace(str, std::regex(R"(\s*▶\s*)"), "▶");
    
    if (Singleton::shared()->scope == Singleton::Scope::Global) {
        str = std::regex_replace(str, std::regex(R"(^ *LOCAL +)"), "");
    }
    
    long indents = lwspaces(str) / tabWidth * Singleton::shared()->tabsize;
    ltrim(str);
    
    while (indents--) {
        str.insert(0, " ");
    }
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



// MARK: - Translation from C to PPL

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

std::string translateCLogicalOperatorsToPPL(const std::string& str) {
    std::string s = str;
    
    s = regex_replace(s, std::regex(R"( *&& *)"), " AND ");
    s = regex_replace(s, std::regex(R"( *\|\| *)"), " OR ");
    s = regex_replace(s, std::regex(R"( *! *)"), " NOT ");
    s = regex_replace(s, std::regex(R"( *\^\^ *)"), " XOR ");
    
    return s;
}


// MARK: - Pre-Processing...

void processLine(const std::string& str, std::ofstream &outfile)
{
    Singleton& singleton = *Singleton::shared();
    std::string ln = str;
    preProcess(ln, outfile);
    if (ln.length() < 2) ln = std::string("");

    
    
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
    
    singleton.incrementLineNumber();
}

void processLines(std::ifstream &infile, std::ofstream &outfile)
{
    std::string s;
    
    while(getline(infile, s)) {
        processLine(s, outfile);
    }
}

void processStringLines(std::istringstream &iss, std::ofstream &outfile)
{
    std::string s;
    
    while(getline(iss, s)) {
        processLine(s, outfile);
    }
}

void process(const std::string &pathname, std::ofstream &outfile)
{
    Singleton& singleton = *Singleton::shared();
    std::ifstream infile;

    singleton.pushPathname(pathname);
    
    infile.open(pathname,std::ios::in);
    if (!infile.is_open()) exit(2);
    
    processLines(infile, outfile);
    
    infile.close();
    singleton.popPathname();
}


void processString(const std::string &str, std::ofstream &outfile) {
    Singleton &singleton = *Singleton::shared();
    
    std::string ln;
    
    singleton.pushPathname("");

    std::istringstream iss{ str };
    
    processStringLines(iss, outfile);

    singleton.popPathname();
}



std::string filterOutNamedArguments(const std::string &str) {
    std::smatch m;
    std::string s;
    std::regex r;
    
    s = str;
    r = R"(\b\w* *:(?=[^:]))";
    while (regex_search(s, m, r)) {
        s = regex_replace(s, r, "");
    }
    return s;
}

void preProcess(std::string &ln, std::ofstream &outfile) {
    std::regex r;
    std::smatch m;
    std::ifstream infile;
    
    Singleton *singleton = Singleton::shared();
    
    static bool multiLineComment = false;
    
    std::string s("        ");
    s.resize(tabWidth);
    ln = regex_replace(ln, std::regex(R"(\t)"), s); // Tab width = 4 spaces :- future reg-ex will not require to deal with '\t', only spaces.
    
    
    /*
     While parsing the contents, strings may inadvertently undergo parsing, leading to potential disruptions in the string's content.
     To address this issue, we prioritize the preservation of any existing strings. Subsequently, after parsing, any strings that have
     been universally altered can be restored to their original state.
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
    
    // If a comment is in the line, preserve it, remove temporarily, and restore afterward.
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
        
        ln = regex_replace(ln, std::regex(R"(>=)"), "≥");
        ln = regex_replace(ln, std::regex(R"(<=)"), "≤");
        ln = regex_replace(ln, std::regex(R"(!=)"), "≠");
        ln = regex_replace(ln, std::regex(R"(=>)"), "▶");
        
        ln += '\n';
        
        return;
    }
    
    if (preprocessor.python) {
        // We're presently handling Python code.
        preprocessor.parse(ln);
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
            process(preprocessor.pathname, outfile);
        }

        ln = std::string("");
        return;
    }
    

    
    rtrim(ln);
    
    
    if (Def::parse(ln)) return;
#ifdef MESSAGES_HPP
    // Obj-C Style
    if (preprocessor.messages) Messages::parse(ln);
#endif
    ln = singleton->aliases.resolveAliasesInText(ln);

    /**
      **NEW! 1.6.3
       auto now inferred for var & const if name is not a valid PPL name
     */
    r = R"(\b((?:var|const) +)(.*)(?=;))";
    std::sregex_token_iterator it = std::sregex_token_iterator {
        ln.begin(), ln.end(), r, {1, 2}
    };
    if (it != std::sregex_token_iterator()) {
        std::string code = *it++;
        std::string str = *it;
        
        r =  R"([^,;]+)";
        for (auto it = std::sregex_iterator(str.begin(), str.end(), r);;) {
            std::string s = trim_copy(it->str());
            if (regex_search(s, std::regex(R"(^[a-zA-Z]\w*:[a-zA-Z])"))) {
                code.append(s);
            } else {
                if (regex_search(s, std::regex(R"(^[a-zA-Z]\w*(?:(::)|\.))"))) {
                    s.insert(0, "auto:");
                }
                code.append(s);
            }
            
            if (++it == std::sregex_iterator()) break;
            code.append(",");
        }
        ln = regex_replace(ln, std::regex(R"(\b((?:var|const) +)(.*)(?=;))"), code);
    }
    
    if (structurs.parse(ln)) {
        ln = std::string("");
        return;
    }
    
    if (enumerators.parse(ln)) {
        ln = std::string("");
        return;
    }
    
    
    if (regex_match(ln, std::regex(R"(^begin *$)", std::regex_constants::icase))) {
        singleton->scope = Singleton::Scope::Local;
        ln = std::string("BEGIN\n");
        return;
    }
    

    if (Singleton::Scope::Local == singleton->scope) {
        r = R"(^end *;? *$)";
        if (regex_match(ln, std::regex(R"(^(?:end *;?) *$)", std::regex_constants::icase))) {
            singleton->aliases.removeAllLocalAliases();
            singleton->scope = Singleton::Scope::Global;
            ln = std::string("END;\n");
            return;
        }
    }
    
    if (Singleton::Scope::Global == singleton->scope) {
        r = R"(^ *(KS?A?_[A-Z\d][a-z]*) *$)";
        std::sregex_token_iterator it = std::sregex_token_iterator {
            ln.begin(), ln.end(), r, {1}
        };
        if (it != std::sregex_token_iterator()) {
            std::string s = *it;
            ln = "KEY " + s + "()";
        }
    }

    r = R"(<(?:int|string)> *\((.*)\))";
    while (regex_search(ln, m, r)) {
        std::sregex_token_iterator it = std::sregex_token_iterator {
            ln.begin(), ln.end(), r, {1}
        };
        if (it != std::sregex_token_iterator()) {
            if (m.str().substr(0, 5) == "<int>" || m.str().substr(0, 5) == "Int") {
                ln = ln.replace(m.position(), m.length(), "IP(" + trim_copy(*it) + ")");
            } else {
                ln = ln.replace(m.position(), m.length(), "STRING(" + trim_copy(*it) + ")");
            }
        }
    }
    
    
    r = R"(<calc>\(.*\))";
    while (regex_search(ln, m, r)) {
        s = "#[" + m.str().substr(7, m.length() - 7 - 1);
        if (s.at(s.length() - 2) == ':') {
            s.insert(s.length() - 2, "]");
        } else {
            s += "]";
        }
        ln = ln.replace(m.position(), m.length(), s);
    }

    /****
      auto inferred for function names containing :: or . without a name: prefix, it is inferred to have an auto: prefix.
     */
    r = R"(^ *(export +)?([a-zA-Z_]\w*((::)|\.))+[a-zA-Z_]\w* *(?=\())";
    if (regex_search(ln, r) && singleton->scope == Singleton::Scope::Global) {
        ltrim(ln);
        ln.insert(0, "auto:");
    }

    
    singleton->autoname.parse(ln);
    Alias::parse(ln);
    
    
    static std::vector<std::string> stack;
    if (Singleton::Scope::Local == Singleton::shared()->scope) {
        
        
        singleton->switches.parse(ln);
        
        /**
          **NEW! 1.6.0
          guard...else
          end;
         */
        r = R"(\bguard +(.+) +else)";
        while (regex_search(ln, m, r)) {
            std::string matched = m.str();
            std::sregex_token_iterator it = std::sregex_token_iterator {
                matched.begin(), matched.end(), r, {1}
            };
            std::string result = "IF NOT(";
            if (it != std::sregex_token_iterator()) {
                result += *it++;
            }
            result += ") THEN";
            ln = ln.replace(m.position(), m.length(), result);
            std::cout << MessageType::Verbose << "deprecated: guard, recomended to use 'if...then...endif;'\n";
        }
        
        
        
        /**
          **NEW! 1.6.0
          if...do
          else
          endif;
         */
        r = R"(\bif +.+ +do\b *$)";
        while (regex_search(ln, m, r)) {
            std::string s = m.str();
            s = regex_replace(s, std::regex(R"( +do\b)"), " THEN");
            ln = ln.replace(m.position(), m.length(), s);
        }
        
        /**
          **NEW! 1.6.1
          if...then [return, break or continue];
         */
        r = R"(\bif +.+ +then +(?:return|break|continue)\b; *$)";
        while (regex_search(ln, m, r)) {
            std::string result = trim_copy(m.str());
            if (result.at(result.length() - 1) != ';') {
                result = result.append(";");
            }
            result.append(" END;");
            ln = ln.replace(m.position(), m.length(), result);
        }
        
        /**
          **NEW! 1.6.5.181
           IFTE + now supports nested IFTE
           (condition ? true : false); // C & P+
           In P+ '?' & ':' must be seperated by a white space
         */
        IFTE::parse(ln);
        
        r = R"(\bfor +(.+) *; *(.+) *; *(.+)? +do\b)";
        while (regex_search(ln, m, r)) {
            std::string matched = m.str();
            std::sregex_token_iterator it = std::sregex_token_iterator {
                matched.begin(), matched.end(), r, {1, 2, 3}
            };
            std::string statements[3];
            if (it != std::sregex_token_iterator()) {
                statements[0] = *it++;
                statements[1] = *it++;
                if (it->matched) statements[2] = *it++;
            }
            
            ln = ln.replace(m.position(), m.length(), (statements[0].empty() ? "" : statements[0] + "; ") + "WHILE " + statements[1] + " DO");
            if (!statements[2].empty()) stack.push_back(statements[2] + "; ");
        }
        
        while (regex_search(ln, m, std::regex(R"(\bnext( *)?;?)", std::regex_constants::icase))) {
            if (stack.empty()) {
                ln = ln.replace(m.position(), m.length(), "END;");
                continue;
            }
            std::string ppl = stack.back();
            stack.pop_back();
            
            ln = ln.replace(m.position(), m.length(), ppl + "END;");
        }
    }
    
    if (regex_match(ln, std::regex(R"(^ *do\b)"))) {
        ln = regex_replace(ln, std::regex(R"(do\b)"), "WHILE 1 DO");
    }
    
    // We turn any keywords that are in lowercase to uppercase
    r = R"(\b(return|kill|if|then|else|xor|or|and|not|case|default|iferr|ifte|for|from|step|downto|to|do|while|repeat|until|break|continue|export|const|local|key)\b)";
    for(std::sregex_iterator it = std::sregex_iterator(ln.begin(), ln.end(), r); it != std::sregex_iterator(); ++it) {
        std::string result = it->str();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        ln = ln.replace(it->position(), it->length(), result);
    }
    
    /**
      **NEW! 1.7.0.5xx
     We turn any math keywords that are in lowercase to uppercase.
     */
    r = R"(\b(log|cos|sin|tan|ln|min|max)\b)";
    for(std::sregex_iterator it = std::sregex_iterator(ln.begin(), ln.end(), r); it != std::sregex_iterator(); ++it) {
        std::string result = it->str();
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        ln = ln.replace(it->position(), it->length(), result);
    }
    
    /**
      **NEW! 1.6.5
      try ... catch ... end;
     */
    ln = regex_replace(ln, std::regex(R"(\btry\b)"), "IFERR");
    ln = regex_replace(ln, std::regex(R"(\bcatch\b)"), "THEN");
    
    ln = regex_replace(ln, std::regex(R"(\b(end(if)?|wend|next|loop)( *)?;?)", std::regex_constants::icase), "END;");
    ln = regex_replace(ln, std::regex(R"( *\.{3} *)"), " TO ");
    
    
    ln = regex_replace(ln, std::regex(R"(\btrue\b)"), "1");
    ln = regex_replace(ln, std::regex(R"(\bfalse\b)"), "0");
    
    ln = regex_replace(ln, std::regex(R"(>=)"), "≥");
    ln = regex_replace(ln, std::regex(R"(<=)"), "≤");
    ln = regex_replace(ln, std::regex(R"(!=)"), "≠");
    ln = regex_replace(ln, std::regex(R"(\bpi\b)"), "π");
    
    /**
      **NEW! 1.7.03xx
       pre-calc
     */
    singleton->calc.parse(ln);
    
    ln = translateCOperatorsToPPL(ln);
    ln = translateCLogicalOperatorsToPPL(ln);
    if (preprocessor.bitwiseOperators) Bitwise::parse(ln);
    ln = regex_replace(ln, std::regex(R"(=>)"), "▶");
    
    /*
     PPL uses := instead of C's = for assignment. Converting = to := in PPL turns every == into :=:= and := into ::=
     To fix this, we convert all :=:= to == and then all ::= to :=
     */
    ln = regex_replace(ln, std::regex(R"(=)"), ":=");
    ln = regex_replace(ln, std::regex(R"(:=:=)"), "==");
    ln = regex_replace(ln, std::regex(R"(::=)"), ":=");
    

    
    
    condence(ln);
    
    strings.restoreStrings(ln);
    singleton->comments.restoreComment(ln);
    
    ln += '\n';
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
    str = R"(#define __VERSION 1.7)";
    preprocessor.parse(str);
    
    if (pragma) {
        str = "#pragma mode( separator(.,;) integer(h64) )";
        processLine(str, outfile);
    }
    
    process( in_filename, outfile );
    
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
