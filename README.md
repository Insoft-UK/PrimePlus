<img src="../../../Insoft-UK/blob/main/assets/logo.svg" width="80" height="80" />

# P+ for HP Prime

<br />
<img src="https://raw.githubusercontent.com/Insoft-UK/PrimePlus/main/assets/P+_Logo.svg" style="width: 128px" />
P+ is a proprietary programming language designed to mitigate the challenges associated with code maintenance and comprehensibility in the HP Programming Language (PPL). P+ serves as an intermediary language that addresses these issues by introducing support for substitutions and facilitating code organization. The final output is a PPL program file with a minimal footprint, optimized to conserve the limited storage space available on the HP Prime.
<br/><br/>
P+ is essentially the PPL language with additional features, such as substitution capabilities, and requires all keywords to be in lowercase.
<br/><br/>

Please view the test <a href="https://github.com/Insoft-UK/PrimePlus/blob/main/examples/test.pp">code</a> to see what additional features P+ adds to the Prime Programming Language (PPL).

<b><a href="https://github.com/Insoft-UK/GROB">GROB</a> to be intergrated in the future.</b>

```
#pragma mode( separator(.,,) integer(h64) )
#include <pplang>

auto:displayCopyright()
begin
  TEXTOUT_P("Copyright (c) 2023-2025 Insoft. All rights reserved.", 0, 0);
end;

EXPORT START()
BEGIN
  displayCopyright();
  WAIT;
  var a:alpha = 0;
  alpha += 10;
  RETURN a;
END;
```

`p+ project.pp -l pplib`

>[!IMPORTANT]
In P+ `=` is treated as `:=` were in PPL `=` is treated as `==`

>[!NOTE]
The P+ proprietary programming language is susceptible to change, while also maintaining some compatibility with previous versions.
