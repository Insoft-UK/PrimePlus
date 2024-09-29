PPL()
BEGIN
    // PPL & P+
    LOCAL a;
    var b:beta;
    
    
    IF a==beta THEN
        a▶beta;
        if a==1 then
            a += 1;
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
unuse namespace std;

Var()
begin
    var a, b; var c, d;
    auto iAmVerryLong;
end;

fn1:Function1()
begin
end;

auto:Function2()
begin
    Function1();
end;

Parameter(a:alpha, auto:iAmUknown)
begin
    alpha += iAmUknown;
    a += iAmUknown;
end;

def Theme[1] theme.color;
Def()
begin
    theme.color = 2;
    theme.shade = 1;
end;

Comments()
begin
    // blar...blar...blar..
    var a; /* variable
    */
    return "blar...blar...blar.."; // blar...blar...blar..
end;

Implied::AutoNameForPPL()
begin
    return 0;
end;

ValidPPLFunctionName()
begin
    Implied::AutoNameForPPL();
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
    
    do
        break;
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
    struct LocalStruct
        first[1];
        second[2];
    end;
    struct LocalStruct auto:localStruct;
    
    localStruct.first = 1;
    localStruct.second = 2;
end;

struct GlobalStruct
    first[1,1];
    second[2,1];
end;

Struct_2()
begin
    struct GlobalStruct auto:globalStruct;
    
    globalStruct.first = 1;
    globalStruct.second = 2;
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
#end





