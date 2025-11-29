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

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <ctime>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <string>
#include <unordered_set>

#include "timer.hpp"
#include "singleton.hpp"
#include "common.hpp"

#include "preprocessor.hpp"
#include "dictionary.hpp"
#include "alias.hpp"
#include "base.hpp"
#include "calc.hpp"
#include "utf.hpp"
#include "prgm.hpp"

#include "../version_code.h"

#define NAME "PPL+ Pre-Processor for PPL"
#define COMMAND_NAME "ppl+"
#define INDENT_WIDTH indentation

static unsigned int indentation = 2;

using pplplus::Singleton;
using pplplus::Aliases;
using pplplus::Alias;
using pplplus::Calc;
using pplplus::Dictionary;
using pplplus::Preprocessor;
using pplplus::Base;

using std::regex_replace;
using std::sregex_iterator;
using std::sregex_token_iterator;

namespace fs = std::filesystem;
namespace rc = std::regex_constants;

static Preprocessor preprocessor = Preprocessor();
static std::string assignment = "=";

// MARK: - Extensions

namespace std::filesystem {
    std::filesystem::path expand_tilde(const std::filesystem::path& path) {
        if (!path.empty() && path.string().starts_with("~")) {
#ifdef _WIN32
            const char* home = std::getenv("USERPROFILE");
#else
            const char* home = std::getenv("HOME");
#endif
            
            if (home) {
                return std::filesystem::path(std::string(home) + path.string().substr(1));  // Replace '~' with $HOME
            }
        }
        return path;  // return as-is if no tilde or no HOME
    }
}

std::string extension_lowercased(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return ext;
}

#if __cplusplus >= 202302L
    #include <bit>
    using std::byteswap;
#elif __cplusplus >= 201103L
    #include <cstdint>
    namespace std {
        template <typename T>
        T byteswap(T u)
        {
            
            static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
            
            union
            {
                T u;
                unsigned char u8[sizeof(T)];
            } source, dest;
            
            source.u = u;
            
            for (size_t k = 0; k < sizeof(T); k++)
                dest.u8[k] = source.u8[sizeof(T) - k - 1];
            
            return dest.u;
        }
    }
#else
    #error "C++11 or newer is required"
#endif

// MARK: - Other

void terminator() {
    std::cerr << MessageType::CriticalError << "An internal pre-processing problem occurred. Please review the syntax before this point.\n";
    exit(0);
}
void (*old_terminate)() = std::set_terminate(terminator);

// MARK: - Helper Functions
#include "string_utilities.hpp"

// MARK: - PPL+ To PPL Translater...

std::string translatePPLPlusLine(const std::string& input) {
    std::regex re;
    std::smatch match;
    std::ifstream infile;
    std::string output = input;
    
    // Remove any leading white spaces before or after.

    if (output.empty()) {
        return output;
    }
    
    output = preprocessor.parse(output);

    /*
     While parsing the contents, strings may inadvertently undergo parsing, leading
     to potential disruptions in the string's content as well as comments.
     
     To address this issue, we prioritize the preservation of any existing strings
     and comments. After we prioritize the preservation of any existing strings and
     comments, we blank out the string/s.
     
     Subsequently, after parsing, any strings that have been blanked out can be
     restored to their original state.
     */
    auto strings = preserveStrings(output);
    output = blankOutStrings(output);
 
    std::string comment = extractComment(output);
    output = removeComment(output);
    
    Singleton::shared()->regexp.resolveAllRegularExpression(output);
    output = processEscapes(output);
    
    output = replaceOperators(output);
    
    // PPL by default uses := instead of C's = for assignment. Converting all = to PPL style :=
    if (assignment == "=") {
        output = convertAssignToColonEqual(output);
    } else {
        output = expandAssignmentEquals(output);
    }
    
    output = Singleton::shared()->aliases.resolveAllAliasesInText(output);
   
    /*
     A code stack provides a convenient way to store code snippets
     that can be retrieved and used later.
     */
    output = Singleton::shared()->codeStack.parse(output);

    
    if (Dictionary::isDictionaryDefinition(output)) {
        Dictionary::proccessDictionaryDefinition(output);
        output = Dictionary::removeDictionaryDefinition(output);
        if (output.empty())
            return "";
    }
    
    
    
    // Keywords
    output = capitalizeWords(output, {
        "begin", "end", "return", "kill", "if", "then", "else", "xor", "or", "and", "not",
        "case", "default", "iferr", "ifte", "for", "from", "step", "downto", "to", "do",
        "while", "repeat", "until", "break", "continue", "export", "const", "local", "key"
    });
    
    output = capitalizeWords(output, {"log", "cos", "sin", "tan", "ln", "min", "max"});

    
    //MARK: User Define Alias Parsing
    output = Alias::parse(output);
    
    
    re = R"(\b(BEGIN|IF|FOR|CASE|REPEAT|WHILE|IFERR)\b)";
    for(auto it = sregex_iterator(output.begin(), output.end(), re); it != sregex_iterator(); ++it) {
        Singleton::shared()->increaseScopeDepth();
    }
    
    re = R"(\b(END|UNTIL)\b)";
    for(auto it = sregex_iterator(output.begin(), output.end(), re); it != sregex_iterator(); ++it) {
        Singleton::shared()->decreaseScopeDepth();
        Singleton::shared()->aliases.removeAllOutOfScopeAliases();
        Singleton::shared()->regexp.removeAllOutOfScopeRegexps();
    }
    
    
    if (Singleton::shared()->scopeDepth == 0) {
        re = R"(^ *(KS?A?_[A-Z\d][a-z]*) *$)";
        sregex_token_iterator it = sregex_token_iterator {
            output.begin(), output.end(), re, {1}
        };
        if (it != sregex_token_iterator()) {
            std::string s = *it;
            output = "KEY " + s + "()";
        }
    }
    
    output = Calc::parse(output);
    output = Base::parse(output);
    
    
   
    output = restoreStrings(output, strings);
    
    if (!comment.empty()) output += comment;
    
    if (output.empty())
        return "";
    return output + "\n";
}



void loadRegexLib(const fs::path path, const bool verbose) {
    std::string utf8;
    std::ifstream infile;
    
    infile.open(path, std::ios::in);
    if (!infile.is_open()) {
        return;
    }
    
    if (verbose) std::cerr << "Library " << (path.filename() == ".base.re" ? "base" : path.stem()) << " successfully loaded.\n";
    
    while (getline(infile, utf8)) {
        utf8.insert(0, "regex ");
        Singleton::shared()->regexp.parse(utf8);
    }
    
    infile.close();
}

void loadRegexLibs(const std::filesystem::path& path, const bool verbose) {
    if (path.empty()) return;
    loadRegexLib(path / "base.re", verbose);
    
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::path(entry.path()).extension() != ".re" || fs::path(entry.path()).filename() == "base.re") {
                continue;
            }
            loadRegexLib(entry.path(), verbose);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "error: " << e.what() << '\n';
    }
}

std::string embedPPLCode(const std::filesystem::path& path) {
    std::ifstream is;
    std::string str;
    
    is.open(path, std::ios::in);
    if (!is.is_open()) return str;
    
    if (path.extension() == ".prgm") {
        std::wstring wstr = utf::load(path);
        
        if (!wstr.empty()) {
            str = utf::utf8(wstr);
            str = regex_replace(str, std::regex(R"(^ *#pragma mode *\(.+\) *\n+)"), "");
            is.close();
            return str;
        }
    }
    
    std::string line;
    while (getline(is, line)) {
        line += '\n';
        str += line;
    }
    is.close();
    return str;
}

bool verbose(void) {
    if (Singleton::shared()->aliases.verbose) return true;
    if (preprocessor.verbose) return true;
    
    return false;
}

enum BlockType
{
    BlockType_Python, BlockType_PPL, BlockType_PrimePlus
};

bool isPythonBlock(const std::string& str) {
    return str.find("#PYTHON") != std::string::npos;
}

bool isPPLBlock(const std::string& str) {
    return str.find("#PPL") != std::string::npos;
}

std::string processPPLBlock(std::ifstream& infile) {
    std::string str;
    std::string output;
    
    Singleton::shared()->incrementLineNumber();
    
    while(getline(infile, str)) {
        if (str.find("#END") != std::string::npos) {
            Singleton::shared()->incrementLineNumber();
            return output;
        }
        
        output += str + '\n';
        Singleton::shared()->incrementLineNumber();
    }
    return str;
}

std::string processPythonBlock(std::ifstream& infile, const std::string& input) {
    std::regex re;
    std::string str;
    std::string output;
    std::smatch match;
    
    Aliases aliases;
    aliases.verbose = Singleton::shared()->aliases.verbose;
    
    Singleton::shared()->incrementLineNumber();
    
    str = cleanWhitespace(input);

    size_t start = str.find('(');
    size_t end = str.find(')', start);
        
    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::vector<std::string> arguments = splitCommas(input.substr(start + 1, end - start - 1));
        output = "#PYTHON (";
        int index = 0, n = 0;
        
        Aliases::TIdentity identity = {
            .type = Aliases::Type::Argument
        };
        for (const std::string& argument : arguments) {
            if (index++) output.append(",");
            start = argument.find(':');
            
            if (start != std::string::npos) {
                output.append(argument.substr(0, start));
                identity.identifier = argument.substr(start + 1, argument.length() - start - 1);
                identity.real = "argv[" + std::to_string(n++) + "]";
                aliases.append(identity);
                continue;
            }
            output.append(argument);
        }
        output.append(")\n");
    } else {
        output = "#PYTHON\n";
    }

    while(getline(infile, str)) {
        if (str.find("#END") != std::string::npos) {
            output += "#END\n";
            Singleton::shared()->incrementLineNumber();
            return output;
        }
        
        Singleton::shared()->incrementLineNumber();
        str = aliases.resolveAllAliasesInText(str);
        
        // alias aliasname as realname
        re = R"(^ *alias +([A-Za-z_]\w*) *as *([a-zA-Z][\w\[\]]*) *$)";
        if (regex_search(str, match, re)) {
            Aliases::TIdentity identity;
            identity.identifier = match[1].str();
            identity.real = match[2].str();
            identity.type = Aliases::Type::Alias;
            identity.scope = -1;
            
            aliases.append(identity);
            str = "";
            continue;
        }
        
        output += str + '\n';
    }
    return output;
}

std::string translatePPLPlusToPPL(const fs::path& path) {
    Singleton& singleton = *Singleton::shared();
    std::ifstream infile;
    std::regex re;
    std::string input;
    std::string output;
    std::smatch match;

    bool pragma = false;
    
    singleton.pushPath(path);
    
    infile.open(path,std::ios::in);
    if (!infile.is_open()) {
        return output;
    }
    
    while (getline(infile, input)) {
        /*
         Handle any escape lines `\` by continuing to read line joining them all up as one long line.
         */
        
        if (!input.empty()) {
            while (input.at(input.length() - 1) == '\\' && !input.empty()) {
                input.resize(input.length() - 1);
                std::string s;
                getline(infile, s);
                input.append(s);
                Singleton::shared()->incrementLineNumber();
                if (s.empty()) break;
            }
        } else {
            Singleton::shared()->incrementLineNumber();
            output += "\n";
            continue;
        }
        
        input = removeTripleSlashComment(input);
        
        input = regex_replace(input, std::regex(R"(\t)"), std::string(INDENT_WIDTH, ' '));
        
        if (input.find("#EXIT") != std::string::npos) {
            break;
        }
        
        while (preprocessor.disregard == true) {
            input = preprocessor.parse(input);
            Singleton::shared()->incrementLineNumber();
            getline(infile, input);
        }
        
        if (isPythonBlock(input)) {
            output += processPythonBlock(infile, input);
            continue;
        }
        
        if (isPPLBlock(input)) {
            output += processPPLBlock(infile);
            continue;
        }
        
        // Handle `#pragma mode` for PPL+
        if (input.find("#pragma mode") != std::string::npos) {
            if (pragma) {
                Singleton::shared()->incrementLineNumber();
                continue;
            }
            pragma = true;
            re = R"(([a-zA-Z]\w*)\(([^()]*)\))";
            std::string s = input;
            input = "#pragma mode( ";
            for(auto it = sregex_iterator(s.begin(), s.end(), re); it != sregex_iterator(); ++it) {
                if (it->str(1) == "assignment") {
                    if (it->str(2) != ":=" && it->str(2) != "=") {
                        std::cerr << MessageType::Warning << "#pragma mode: for '" << it->str() << "' invalid.\n";
                    }
                    if (it->str(2) == ":=") assignment = ":=";
                    if (it->str(2) == "=") assignment = "=";
                    continue;
                }
                if (it->str(1) == "indentation") {
                    indentation = atoi(it->str(2).c_str());
                    continue;
                }
               
                input.append(it->str() + " ");
            }
            input.append(")");
            output += input + '\n';
            Singleton::shared()->incrementLineNumber();
            continue;
        }
        
        
        if (preprocessor.isQuotedInclude(input)) {
            Singleton::shared()->incrementLineNumber();
            
            std::filesystem::path path = preprocessor.extractIncludePath(input);
            if (path.parent_path().empty() && fs::exists(path) == false) {
                path = singleton.getMainSourceDir().string() + "/" + path.filename().string();
            }
            if (path.extension() == ".prgm") {
                output += embedPPLCode(path.string());
                continue;
            }
            if (!(fs::exists(path))) {
                std::cerr << MessageType::Verbose << path.filename() << " file not found\n";
            } else {
                output += translatePPLPlusToPPL(path.string());
            }
            continue;
        }
        
        if (preprocessor.isAngleInclude(input)) {
            Singleton::shared()->incrementLineNumber();
            std::filesystem::path path = preprocessor.extractIncludePath(input);
            if (path.extension().empty()) {
                path.replace_extension(".prgm+");
            }
            for (fs::path systemIncludePath : preprocessor.systemIncludePath) {
                if (fs::exists(systemIncludePath.string() + "/" + path.filename().string())) {
                    path = systemIncludePath.string() + "/" + path.filename().string();
                    break;
                }
            }
            if (!(fs::exists(path))) {
                std::cerr << MessageType::Verbose << path.filename() << " file not found\n";
            } else {
                output += translatePPLPlusToPPL(path.string());
            }
            continue;
        }
        
        
        if (Singleton::shared()->regexp.parse(input)) {
            input = regex_replace(input, std::regex(R"(^ *\bregex +([@<>=≠≤≥~])?`([^`]*)`(i)? *(.*)$)"), "");
            Singleton::shared()->incrementLineNumber();
            continue;
        }
        
        std::istringstream iss;
        iss.str(input);
        std::string str;
        

        while(getline(iss, str)) {
            std::string s = translatePPLPlusLine(str);
            if (is_all_whitespace(s)) {
                continue;
            }
            output += s;
        }
        
        Singleton::shared()->incrementLineNumber();
    }
    
    
    
    infile.close();
    singleton.popPath();
    
    return output;
}


// MARK: - Command Line
void error(void) {
    std::cerr << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}


void help(void) {
    using namespace std;
    std::cerr
    << "Copyright (C) 2023-" << YEAR << " Insoft.\n"
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << BUNDLE_VERSION << ")\n"
    << "\n"
    << "Usage: " << COMMAND_NAME << " <input-file> [-o <output-file>] [-v=<flags>]\n"
    << "\n"
    << "Options:\n"
    << "  -o <output-file>        Specify the filename for generated PPL code.\n"
    << "  -v=<flags>              Display detailed processing information.\n"
    << "\n"
    << "  Verbose Flags:\n"
    << "     a                    Aliases\n"
    << "     p                    Preprocessor\n"
    << "     r                    Regular Expression\n"
    << "\n"
    << "Additional Commands:\n"
    << "  " << COMMAND_NAME << " {--version | --help }\n"
    << "    --version              Display the version information.\n"
    << "    --help                 Show this help message.\n";
}

fs::path resolveAndValidateInputFile(const char *input_file) {
    fs::path path;
    
    path = input_file;
    if (path == "/dev/stdin") return path;
    
    path = fs::expand_tilde(path);
    if (path.parent_path().empty()) path = fs::path("./") / path;
    
    if (path.has_extension() == false) {
        // default extension
        path.replace_extension("prgm+");
    }
        
    
    // • Validates the extension and encoding (UTF-8 BOM)
    if (utf::bom(path) != utf::BOMnone) {
        std::cerr << "❌ error: " << path.filename() << " file must be utf8.\n";
        exit(0);
    }
    
    if (!fs::exists(path)) {
        std::cerr << "❓File " << path.filename() << " not found at " << path.parent_path() << " location.\n";
        exit(0);
    }
    
    return path;
}

fs::path resolveOutputFile(const char *output_file) {
    fs::path path;
    
    path = output_file;
    if (path == "/dev/stdout") return path;
    
    path = fs::expand_tilde(path);
    
    return path;
}

fs::path resolveOutputPath(const fs::path& inpath, const fs::path& outpath) {
    fs::path path = outpath;
    
    if (path == "/dev/stdout") return path;
    
    if (path.empty()) {
        // User did not specify specify an output filename, use the input filename with a .prgm extension.
        path = inpath;
        path.replace_extension(".prgm");
        return path;
    }
    
    if (fs::is_directory(path)) {
        /* User did not specify specify an output filename but has specified a path, so append
         with the input filename and subtitute the extension with .prgm
         */
        path = path / inpath.stem();
        path.replace_extension("prgm");
        return path;
    }
    
    if (!path.has_extension()) path.replace_extension("prgm");
    if (path.parent_path().empty()) path = inpath.parent_path() / path;
    
    return path;
}

// MARK: - Main
int main(int argc, char **argv) {
    fs::path inpath, outpath;
    
    if (argc == 1) {
        error();
        exit(100);
    }
    
    bool verbose = false;
    
    
    std::string args(argv[0]);
    
    for (int n = 1; n < argc; n++) {
        args = argv[n];
        
        if (args == "-o") {
            if ( ++n >= argc ) {
                error();
                exit(0);
            }
            outpath = resolveOutputFile(argv[n]);
            continue;
        }
    
        if ( args == "--help" ) {
            help();
            return 0;
        }
        
        if (args == "--version") {
            std::cout << VERSION_NUMBER << "\n";
            return 0;
        }
        
        if (args == "--build") {
            std::cout << NUMERIC_BUILD << "\n";
            return 0;
        }
        
        if (args.starts_with("-v=")) {
            if (args.find("a") != std::string::npos) Singleton::shared()->aliases.verbose = true;
            if (args.find("p") != std::string::npos) preprocessor.verbose = true;
            if (args.find("r") != std::string::npos) Singleton::shared()->regexp.verbose = true;
            if (args.find("l") != std::string::npos) verbose = true;
                
            continue;
        }
        
        if (args.starts_with("-I")) {
            fs::path path = fs::path(args.substr(2)).has_filename() ? fs::path(args.substr(2)) : fs::path(args.substr(2)).parent_path();
            path = fs::expand_tilde(path);
            preprocessor.systemIncludePath.push_front(path);
            continue;
        }
        
        if (args.starts_with("-L")) {
            fs::path path = fs::path(args.substr(2)).has_filename() ? fs::path(args.substr(2)) : fs::path(args.substr(2)).parent_path();
            loadRegexLibs(fs::expand_tilde(path), verbose);
            continue;
        }
        
        inpath = resolveAndValidateInputFile(argv[n]);
    }
    
    outpath = resolveOutputPath(inpath, outpath);

    if (outpath == inpath) {
        std::cerr << "❌ error: Input file and output file cannot be the same. Choose a different output path.\n";
        exit(0);
        return 0;
    }
    
    
    std::string str;
    
    str = "#define __pplplus";
    preprocessor.parse(str);
    
    str = R"(#define __LIST_LIMIT 10000)";
    preprocessor.parse(str);
    
    str = R"(#define __VERSION )" + std::to_string(NUMERIC_BUILD / 100);
    preprocessor.parse(str);
    
    str = R"(#define __NUMERIC_BUILD )" + std::to_string(NUMERIC_BUILD);
    preprocessor.parse(str);
    
    // Start measuring time
    Timer timer;
    std::string output = translatePPLPlusToPPL(inpath);
    
    // Stop measuring time and calculate the elapsed time.
    long long elapsed_time = timer.elapsed();
    
    // Display elasps time in secononds.
    if (elapsed_time / 1e9 < 1.0) {
        std::cerr << "📣 Completed in " << std::fixed << std::setprecision(2) << elapsed_time / 1e6 << " milliseconds\n";
    } else {
        std::cerr << "📣 Completed in " << std::fixed << std::setprecision(2) << elapsed_time / 1e9 << " seconds\n";
    }
    
    
    if (outpath == "/dev/stdout") {
        std::cout << output;
    }
    
    auto ext = extension_lowercased(outpath);
    if (ext == ".hpprgm") {
        auto programName = inpath.stem().string();
        auto prgmSource = prgm::loadPrgm(inpath);
        
        prgm::buildHPPrgm(outpath, programName, prgmSource);
    } else {
        if (!utf::save(outpath, utf::utf16(output), utf::BOMle)) {
            std::cerr << "❌ Unable to create file " << outpath.filename() << ".\n";
            return 0;
        }
    }
    
    if (hasErrors() == true) {
        std::cerr << "🛑 errors!" << "\n";
    }
    
    if (outpath != "/dev/stdout")
        std::cerr << "✅ File " << outpath.filename() << " succefuly created.\n";
    
    
    return 0;
}
