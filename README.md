# Project **actrie**

English | [简体中文](./README.zh_cn.md)

## What is actrie?

In the beginning, **actrie** is an implementation of Aho-Corasick automation, optimize for large scale multi-pattern.

Now, we support more types of pattern: anti-ambiguity pattern, anti-antonym pattern, distance pattern, alternation pattern, etc. You can combine all of them together.


## Pattern syntax

### 1. **plain** pattern:

> **abc**

### 2. **anti-ambiguity** pattern:

> center **(?&!** ambiguity **)**

### 3. **anti-antonym** pattern:

> **(?<!** antonym **)** center

### 4. **distance** pattern:

> prefix **.{min,max}** suffix

### 5. **alternation** pattern:

> pattern0 **|** pattern1

### 6. **wrapper** pattern:

> **(** pattern **)**


## Build and install

```bash
# download source
git clone --depth=1 --recurse-submodules --shallow-submodules https://github.com/ifplusor/actrie.git

# change directory
cd actrie

# configure cmake project
mkdir build && cd build
cmake ..

# build alib and actrie libraries
make actrie

# change directory
cd ..

# build python wheel package
python setup.py bdist_wheel

# install python package
pip install dist/actrie-*.whl
```


## Python example

### vocab.txt

```text
pattern0
pattern1
pattern2
...
```

### example.py

```python
#!/usr/bin/env python
# coding=utf-8

from actrie import *

pattern = r"f|(a|b).{0,5}(e(?&!ef)|g)
abc
efg"

content = "abcdefg"


def test():
    global pattern, content

    # create matcher by file
    #matcher = Matcher.create_by_file("vocab.txt")

    # create matcher by string
    matcher = Matcher.create_by_string(pattern)

    # iterator
    for matched in matcher.finditer(content):
        print(matched)

    # find all
    all_matched = matcher.findall(content)
    print(all_matched)


if __name__ == "__main__":
    test()

```
