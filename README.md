# Project **actrie**

[![GitHub issues](https://img.shields.io/github/issues/ifplusor/actrie)](https://github.com/ifplusor/actrie/issues)
[![GitHub forks](https://img.shields.io/github/forks/ifplusor/actrie)](https://github.com/ifplusor/actrie/network)
[![GitHub stars](https://img.shields.io/github/stars/ifplusor/actrie)](https://github.com/ifplusor/actrie/stargazers)
[![GitHub license](https://img.shields.io/github/license/ifplusor/actrie)](https://github.com/ifplusor/actrie/blob/master/LICENSE)
![PyPI - Implementation](https://img.shields.io/pypi/implementation/actrie)
![PyPI - Python Version](https://img.shields.io/pypi/pyversions/actrie)
[![PyPI - Wheel](https://img.shields.io/pypi/wheel/actrie)](https://pypi.org/project/actrie/)

English | [简体中文](./README.zh_cn.md)

## What is actrie?

In the beginning, **actrie** is an implementation of Aho-Corasick automation, optimize for large scale multi-pattern.

Now, we support more types of pattern: anti-ambiguity pattern, anti-antonym pattern, distance pattern, alternation pattern, etc. You can combine all of them together.

## Pattern syntax

### 1. **plain** pattern

> **abc**

### 2. **anti-ambiguity** pattern

> center **(?&!** ambiguity **)**

### 3. **anti-antonym** pattern

> **(?<!** antonym **)** center

### 4. **distance** pattern

> prefix **.{min,max}** suffix

### 5. **alternation** pattern

> pattern0 **|** pattern1

### 6. **wrapper** pattern

> **(** pattern **)**

## Build and install

### 1. Build C library

```bash
# download source
git clone --depth=1 --recurse-submodules --shallow-submodules https://github.com/ifplusor/actrie.git

# go to project directory
cd actrie

# make build directory
mkdir build && pushd build

# configure cmake project
cmake -DCMAKE_BUILD_TYPE=Release ..

# configure cmake project if no stdatomic.h in gcc
#cmake -DCMAKE_BUILD_TYPE=Release -D__GCC_ATOMICS__=ON ..

# build alib and actrie libraries
make actrie

popd
```

### 2. Build and install Python package

```bash
# build wheel package
python setup.py bdist_wheel

# install wheel package
pip install dist/actrie-*.whl

# Or install from PyPI
pip install actrie
```

### 3. Build and install Java package

```bash
# go to jni directory
pushd jni

# build nar package
mvn clean package

# install to local maven repository
mvn install

popd
```

## Example

### vocab.txt

```text
f|(a|b).{0,5}(e(?&!ef)|g)	pattern0
abc	pattern1
efg	pattern2
```

### **Python**: example.py

```python
#!/usr/bin/env python
# coding=utf-8

from actrie import *

# with open("vocab.txt") as rf:
#     pattern = rf.read()

pattern = """
f|(a|b).{0,5}(e(?&!ef)|g)
abc
efg
"""

content = "abcdefg"


def test():
    global pattern, content

    # create matcher by file
    # matcher = Matcher.create_by_file("vocab.txt")

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

### **Java**: Example.java

```java
package psn.ifplusor.actrie;

public class Example {

    public static void main(String[] args) {
        String pattern = "f|(a|b).{0,5}(e(?&!ef)|g)\nabc\nefg";
        String content = "abcdefg";

        // create matcher by string
        try (Matcher matcher = Matcher.createByString(pattern)) {
            try (Context context = matcher.match(content)) {
                // iterator
                for (Word word : context) {
                    System.out.println(word);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
```
