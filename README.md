<img src="https://raw.githubusercontent.com/Insoft-UK/PrimeSDK/main/assets/Nightly%20Build.png" style="width: 128px" />


## PPL+ for HP Prime Programming Language
**Command Line Tool**

<b>PPL+</b> is a preprocessor command line tool designed to improve code maintainability and readability in the HP Prime Programming Language (PPL). PPL+ also allows one to define regular expressions to assist in the preprocessing workflow. The final output is a compact, optimized PPL program file tailored to the HP Prime’s limited storage capacity.
<br/><br/>

`Usage: ppl+ <input-file> [-o <output-file>] [-v <flags>]`

<table>
  <thead>
    <tr align="left">
      <th>Options</th>
      <th>Description</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>-o <output-file></td><td>Specify the filename for generated code</td>
    </tr>
    <tr>
      <td>-c or --compress</td><td>Specify if the PPL code should be compressed</td>
    </tr>
    <tr>
      <td>-v or --verbose</td><td>Display detailed processing information</td>
    </tr>
    <tr>
      <td colspan="2"><b>Additional Commands</b></td>
    </tr>
    <tr>
      <td>--version</td><td>Displays the version information</td>
    </tr>
    <tr>
      <td>--build</td><td>Displays the build information</td>
    </tr>
    <tr>
      <td>--help</td><td>Show this help message</td>
    </tr>
  </tbody>
</table>

### Regular Expressions
**Example: Extending PPL with Switch-Case Functionality Using Regex**

This example demonstrates how to use **regex** (regular expressions) to add **switch-case** control flow to the PPL language, similar to the switch statements found in other programming languages.

```
regex >`\bswitch +([a-z_.:]+)`i LOCAL sw__SCOPE__ := $1;\aCASE
regex >`\bcase +([^ ]+) +do\b`i IF sw\`__SCOPE__-1` == $1 THEN
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
regex >`\bfor *([^;]+); *([^;]+); *([^;]+) +do\b`i $1;\aWHILE $2 DO__PUSH__`\i$3;\a`
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


### Implimenting Variable Aliases and Auto
```
regex >`\bauto\b`i v__COUNTER__
regex =`^ *\bauto *: *([a-z]\w*)` g__COUNTER__:$1
regex `\b([a-zA-Z_]\w*) *\: *([a-zA-Z]\w*(?:::[a-zA-Z]\w*)*)` alias $2:=$1;$1

auto : myGlobal := 1;
fnc1: My::Function(p1: argumentOne)
begin
    var auto: i, a: Alpha;
    for i=0; i<2; i=i+1 do
        Alpha := Alpha + 1 * myGlobal * argumentOne;
    end;
    
end;
auto: anotherGlobal := 2;

My::Function();
```

**PPL+ Preprocessor: PPL Converstion**
```
g0 := 1;
fnc1(p1)
BEGIN
  var v0, a;
  v0 := 0; WHILE v0<2 DO
    a := a + 1 * g0 * p1;
  v0 := v0 + 1; END;
END;

g1 := 2;
fnc1();
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
