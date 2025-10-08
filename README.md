<img src="https://raw.githubusercontent.com/Insoft-UK/PrimeSDK/main/assets/PPL+.png" style="width: 128px" />


## PPL+ for HP Prime Programming Language
**Command Line Tool**

<b>PPL+</b> is a preprocessor command line tool designed to improve code maintainability and readability in the HP Prime Programming Language (PPL). PPL+ also allows one to define regular expressions to assist in the preprocessing workflow. The final output is a compact, optimized PPL program file tailored to the HP Prime’s limited storage capacity.
<br/><br/>

**PPL+**

```
#pragma mode( separator(.,;) integer(h64) )
auto: displayCopyright()
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
  LOCAL _beta = 1;
  alpha = alpha + _beta;
  RETURN a;
END;
```


**PPL**

```
#pragma mode( separator(.,;) integer(h64) )
fn1()
BEGIN
  TEXTOUT_P("Copyright (c) 2023-2025 Insoft. All rights reserved.", 0, 0);
END;

#PYTHON
#END
EXPORT START()
BEGIN
  fn1;
  // In PPL+ `=` is treated as `:=` were in PPL `=` is treated as `==`
  // So only PPL code in this section.
  A := B;
  WAIT;
  LOCAL a := 0;
  LOCAL v1 := 1;
  a := a + v1;
  RETURN a;
END;
```

### Regular Expressions
#### switch
**PPL+**
```
regex `\bswitch +([a-zA-Z_]\w*)` LOCAL sw__SCOPE__ := $1;CASE
regex `\bcase +(\-?\d+) +do *$` IF sw\`__SCOPE__-1` == $1 THEN

switch X
    case 0 do
    end;
end;
```

**PPL**
```
LOCAL sw0 := X;
CASE
  IF sw0 == 0 THEN
  END;
END;
```

### Code Stack

A code stack provides a convenient way to store code snippets that can be retrieved and used later.

**PPL+**
```
__PUSH__`i := i + 1;`
local i := 8;
__POP__
```
**PPL**
```
LOCAL i := 8;
i := i + 1;
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
