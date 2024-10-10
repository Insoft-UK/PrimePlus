/*
 The MIT License (MIT)
 
 Copyright (c) 2023-2024 Insoft. All rights reserved.
 
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
 
#pragma mode( separator(.,,) integer(h64) )

/*
 When defining a macro with parameters, issues can arise if an argument
 name is adjacent to a letter, causing conflicts. To resolve this, you
 can use numbered placeholders like `1$` for the first argument, `2$` for
 the second, `3$` for the third, and so on. This ensures the macro
 arguments remain uniquely identifiable and avoid conflicts.
*/
#define MacroList(n)      L$1

#define COPYWRITE "Copyright (c) 2023-2024 Insoft. All rights reserved."

#PYTHON

var
#END
#PPL

local

g
k
#END

local

// Synthetic Class
def using namespace class;
def remove namespace endof;

class MyClass;

auto a;
var MyClass.b;

MyClass::method()
begin
    a = b = 0;
end;

endof MyClass;

MyClass.b = 0;
MyClass::method();


#if __VERSION >= 204
    var text = {"This is some text...   ... ...", "If I see you", "It means it's working."};
#else
    var text {"I never want to see you.", "If I do, we have a BUG!"}
#endif

fn2:Function1()
begin
    var auto:copywriteText = COPYWRITE;
end;

auto:Function2()
begin
    Function1();
end;

Implied::AutoNameForPPL()
begin
    return 0;
end;

ValidPPLFunctionName()
begin
    Implied::AutoNameForPPL();
end;

TRY:Try(a, b)
begin
    var _c;
    try
        a /= b;
    catch
        a = -1;
    end;
    
    return a;
end;

Ifte(a)
begin
    var b = 1;
    
    a = Try(a, b);
    b = (a == 0 ? 0 : 1);
end;

FOR_NEXT:forNext()
begin
    var a, b;
    
    def M$1 matrix(n) @ deprecated "just testing deprecated message!";
    for a = 0; a <= 10; a += 1 do
        b += 1;
    next;
    
    for ; a != 10; a = 10 do
    next;
    
    for ;; do
    next;
    
    a = matrix(1);
end;

export Parameters(auto:a, b:beta)
begin
    def L$1 DefList(n);
    
    auto alpha;
    var auto:beta;
    var z:zero;
    
    alpha += beta + zero;
    
    MacroList(1) = 0;
    DefList(1) = 0;
end;

enum MyEnum
   First = 1, Second = 2
end;

def MyEnum.Second ME_Second;
PPL()
BEGIN
    // PPL & P+
    LOCAL a;
    var b:beta;
    
    
    IF a==beta THEN
        a▶beta;
        if a==1 then
            a += MyEnum.First;
            a += ME_Second;
            a += MyEnum_Second;
        endif;
    END;
END;

using namespace std;
Namespace()
begin
    def RECT std::cartesian::rect;
    std::cartesian::rect(0);
    cartesian::rect(0);
end;
remove namespace std;

Var()
begin
    var a, b; var c, d;
    auto iAmVerryLong;
end;

Parameter(a:alpha, auto:iAmUknown)
begin
    alpha += iAmUknown;
    a += iAmUknown;
end;

def Theme[1] theme.color;
def Theme[2] theme.shade;
Def()
begin
    theme.color = 2;
    theme.shade = 1;
end;

Comments()
begin
    // blar...blar...blar..
    var a; /* variable */
    return "blar...blar...   blar.."; // blar...blar...blar..
end;

Operators()
begin
    var a, b;
    
    a >> b;
    a >> b;
    a << b;
    
    a & b;
    a | b;
    
    a = b;
    
    a += b;
    a -= b;
    a /= b;
    a %= b;

    a &= b;
    a |= b;

    // In mathematics, the symbol ^ represents exponentiation.
    // It is used to denote that a number (the base) is raised
    // to the power of another number (the exponent).
    a ^= b;

    a <<= b;
    a >>= b;
end;

Branch()
begin
    var a, b;
    
    if a == b then
        a += 1;
    else
        a = 10;
    endif;
    
    if a <= b && b > 10 then a += 1; endif;
    
    if a != b && b > 10 then a += 1; endif;
    
    if a <> b && b > 10 then a += 1; endif;
    
    if a >= b || b < 20 then a += 1; endif;
    
    if a == b || b < 20 then a += 1; endif;
end;

Loop()
begin
    var a, b = 10;
    
    for a = 0; a <= 10; a += 1 do
        b += 1;
    next;
    
    for a := 0 step 1 do
        b - 1 ▶ b;
    end;
    
    while a != b do
        a = b;
    wend;
    
    repeat
        a += 1;
    until a == b;
    
    a = 0;
    do
        if a > 10 then break; endif;
        a += 1;
    loop;
end;

Auto()
begin
    auto iWasAVeryLongName, b;
    auto iWas::Invalid = 0; // invalid because it's asigning a value.
    var a:thisIsMyLongName, auto:letYouDecideMyShortName;
    
    iWasAVeryLongName = thisIsMyLongName + b;
    a = thisIsMyLongName + b;
    iWasAVeryLongName *= letYouDecideMyShortName;
end;

Switch()
begin
    var a = 0;
    
    switch a
        case 0 do
            a += 1;
        end;
        
        case 1 do
            a -= 1;
        end;
        
        default
            switch b
                case 0 do
                    b =1;
                end;
            end;
    end;
    
    case
        if a == 0 then a += 1; end;
        if a == 1 then a -= 1; end;
    end;
end;

Struct_1()
begin
    struct MyStruct
        first[1];
        second[2];
    end;
    struct MyStruct auto:myStruct;
    
    myStruct.first = 1;
    myStruct.second = 2;
end;

struct MyGlobalStruct
    first[1,1];
    second[2,1];
end;

Struct_2()
begin
    struct MyGlobalStruct auto:myGlobalStruct;
    
    myGlobalStruct.first = 1;
    myGlobalStruct.second = 2;
end;

Try()
begin
    var a, b = 0;
    try
        a += 1;
        a /= b;
    catch
        return -1;
    else
        return 0;
    end;
end;

PreCalc()
begin
    var ppl_numbers = \[320 / #2d];
    var old = #[ 1 + 2 * 4 / 2 ]:2;
#if __VERSION >= 202
    var new = \ 2 [ 1 + 2 * 4 / 2 ];
#endif
end;

#PPL
I_AM_PPL_CODE()
BEGIN
    LOCAL a; LOCAL b;
    var b; // I am not PPL Code!
END;
#END
