<img src="https://raw.githubusercontent.com/Insoft-UK/PrimeSDK/main/assets/PPL+.png" style="width: 128px" />


## PPL+ for HP Prime Programming Language
**Command Line Tool**

<b>PPL+</b> is a preprocessor command line tool designed to improve code maintainability and readability in the HP Prime Programming Language (PPL). PPL+ also allows one to define regular expressions to assist in the preprocessing workflow. The final output is a compact, optimized PPL program file tailored to the HP Prime’s limited storage capacity.
<br/><br/>

**PPL+**

```
#pragma mode( separator(.,;) integer(h64) )
fn: displayCopyright()
begin
  TEXTOUT_P("Copyright (c) 2023-2025 Insoft. All rights reserved.", 0, 0);
end;

#PYTHON
#END

EXPORT START()
BEGIN
  displayCopyright();
#PPL
  // In PPL+ `=` is treated as `:=` were in PPL `=` is treated as `==`
  // So only PPL code in this section.
  A := B;
#END
  WAIT;
  LOCAL a: alpha = 0;
  LOCAL b: _beta = 1;
  alpha = alpha + _beta;
  RETURN a;
END;
```


**PPL**

```
#pragma mode( separator(.,;) integer(h64) )
fn()
BEGIN
  TEXTOUT_P("Copyright (c) 2023-2025 Insoft. All rights reserved.", 0, 0);
END;

#PYTHON
#END
EXPORT START()
BEGIN
  fn();
  // In PPL+ `=` is treated as `:=` were in PPL `=` is treated as `==`
  // So only PPL code in this section.
  A := B;
  WAIT;
  LOCAL a := 0;
  LOCAL b := 1;
  a := a + b;
  RETURN a;
END;
```

### Regular Expressions
**Example: Extending PPL with Switch-Case Functionality Using Regex**

This example demonstrates how to use **regex** (regular expressions) to add **switch-case** control flow to the PPL language, similar to the switch statements found in other programming languages.

```
regex `\bswitch +([a-zA-Z_]\w*)` LOCAL sw__SCOPE__ := $1;CASE
regex `\bcase +(\-?\d+) +do *$` IF sw\`__SCOPE__-1` == $1 THEN

switch X
    case 0 do
    end;
end;
```
**PPL+ Preprocessor: Switch-Case to PPL Conversion**

The PPL+ preprocessor generates valid PPL code by transforming switch-case statements into standard PPL case statements. This preprocessing step allows you to write more intuitive switch-case syntax while maintaining full compatibility with the PPL language, as the output is pure, valid PPL code.

```
LOCAL sw0 := X;
CASE
  IF sw0 == 0 THEN
  END;
END;
```

### Code Stack

A code stack provides a convenient way to store code snippets that can be retrieved and used later.

**Example: how code stack can be used for a regular expresion to bring a bit of C style to PPL**

```
regex >`\bfor *([^;]+); *([^;]+); *([^;]+) +do\b`i $1; WHILE $2 DO__PUSH__`\t$3;`
regex >`\bend;`i __POP__END;

function()
begin
    for i=0; i<2; i=i+1 do
        A := A+1;
    end;
end;
```

**PPL+ Preprocessor: PPL Converstion**

```
function()
BEGIN
  i := 0; WHILE i<2 DO
    A := A + 1;
  i := i + 1; END;
END;
```


### Implimenting Variable Aliases
Since **regex** is so versitile, the **variable alias** feture when defining variables in PPL+ has been removed in favore of reimplimened it with **regex**

Implimenting **LOCAL a: aliasLongName**, now removed from PPL+ as it can be reimplimened with regex with support for `::`.

```
regex >`\b([a-zA-Z_]\w*) *\: *([a-zA-Z]\w*(?:::[a-zA-Z]\w*)*)` alias $2:=$1;$1

test()
begin
    local a: aliasLongName, b: long::name, v;
    alias aliasName := v;
    
    aliasLongName := long::name + v;
    v := aliasName + 1;
end;
```

**PPL+ Preprocessor: PPL Converstion**
```
test()
BEGIN
  LOCAL a, b, v;
  
  a := b + v;
  v := v + 1;
END;
```

### Implimenting Auto
```
regex >`\bauto\b`i v__++COUNT__
regex >`\b([a-zA-Z_]\w*) *\: *([a-zA-Z]\w*(?:::[a-zA-Z]\w*)*)` alias $2:=$1;$1

test()
begin
    local auto: aliasLongName, b: long::name, v;
    alias aliasName := v;
    
    aliasLongName := long::name + v;
    v := aliasName + 1;
end;
```

**PPL+ Preprocessor: PPL Converstion**
```
test()
BEGIN
  LOCAL v1, b, v;
  
  v1 := b + v;
  v := v + 1;
END;
```

### Assignment Style

In <b>PPL+</b>, the = operator is treated as := (assignment) by default, whereas in standard <b>PPL</b>, = is interpreted as == (equality). This behavior in PPL+ can be explicitly controlled using the directive:

```#pragma mode( assignment(:=) )```

This allows you to use = as equality.

>[!IMPORTANT]
In PPL+ by default `=` is treated as `:=` were in PPL `=` is treated as `==`

## Alias
Added support for defining aliases that include a dot (e.g., alias hp::text := HP.Text).

>[!NOTE]
The PPL+ preprocessor is subject to change but aims to maintain some level of compatibility with previous versions.
