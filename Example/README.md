# Example `test.c`

*Compiled on Ubuntu 16.04.1 LTS 64 bit*

This example covers the basics of `dress`.

First, `a.out` was compiled by:

> gcc test.c

Then, a stripped binary, `b.out`, was created:

> strip a.out -o b.out

Now, running readelf only has the symbols used for dynamic linking:

```
$ readelf -s b.out
Symbol table '.dynsym' contains 5 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@GLIBC_2.2.5 (2)
     2: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.2.5 (2)
     3: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.2.5 (2)
     4: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
```

With the use of Binary Ninja or IDA, we can infer and guess what the original function and global variable names were. From this, we create a symbol file to that lists the symbols to add back into the binary. Functions are indicated with parentheses and the address is listed after the @ sign and an asterisk:

**a.sym**
``` javascript
counter @ *0x00601044
main() @ *0x00400593
test() @ *0x00400566
```

Finally, we can compile the symbols into the binary:

> ./dress b.out c.out a.sym

This is using `b.out` as the input file, `c.out` as the output file, and `a.sym` as the symbol file. All three arguments are required for dress to work. Now, we can check to make sure the symbols were added back in properly:

```
$ readelf -s c.out
Symbol table '.dynsym' contains 5 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@GLIBC_2.2.5 (2)
     2: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.2.5 (2)
     3: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.2.5 (2)
     4: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__

Symbol table '.symtab' contains 3 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000601044     0 OBJECT  LOCAL  DEFAULT   25 counter
     1: 0000000000400593     0 FUNC    LOCAL  DEFAULT   14 main
     2: 0000000000400566     0 FUNC    LOCAL  DEFAULT   14 test
```
