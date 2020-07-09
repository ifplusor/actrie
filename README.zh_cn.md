# actrie项目

[![GitHub issues](https://img.shields.io/github/issues/ifplusor/actrie)](https://github.com/ifplusor/actrie/issues)
[![GitHub forks](https://img.shields.io/github/forks/ifplusor/actrie)](https://github.com/ifplusor/actrie/network)
[![GitHub stars](https://img.shields.io/github/stars/ifplusor/actrie)](https://github.com/ifplusor/actrie/stargazers)
[![GitHub license](https://img.shields.io/github/license/ifplusor/actrie)](https://github.com/ifplusor/actrie/blob/master/LICENSE)
![PyPI - Implementation](https://img.shields.io/pypi/implementation/actrie)
![PyPI - Python Version](https://img.shields.io/pypi/pyversions/actrie)
[![PyPI - Wheel](https://img.shields.io/pypi/wheel/actrie)](https://pypi.org/project/actrie/)

[English](./README.md) | 简体中文

## 什么是actrie?

actrie是为多模式匹配创建的工具库，支持C和Python，核心算法是AC自动机和双数组字典树，并针对大规模多模式匹配场景做了优化，具有良好的空间和时间性能。

除此之外，actrie还支持高级匹配模式，在简单文本匹配的基础上，增加了歧义、否定、距离、并列等实用结构。这些结构可以任意组合，从而使模式的定义能够满足精确和丰富的语义。


## 匹配模式语法

为降低学习成本，actrie的模式语法参考了正则表达式，并做了必要的扩展和删减。

### 1. 平凡模式

语法：

> 经过必要转译的文本

样例：

> 匹配模式：**北京**
>
> 匹配文本：我去过北京故宫
>
> 匹配结果：北京

说明：文本中包含匹配模式的关键词均会输出。

### 2. 歧义模式

语法：

> 中心词 **(?&!** 歧义词 **)**

样例：

> 匹配模式：**北京(?&!北京大学)**
>
> 匹配文本1：我去过北京故宫
>
> 匹配结果1：北京
>
> 匹配文本2：也去过北京大学
>
> 匹配结果2：

说明：关键词后需紧跟 **(?&!** 歧义词 **)**，匹配到的歧义词（包含关键词的长词）的内容不会输出。

### 3. 否定模式

语法：

> **(?<!** 否定词 **)** 中心词

样例：

> 匹配模式：**(?<!没有)去过北京**
>
> 匹配文本1：我去过北京故宫
>
> 匹配结果1：去过北京
>
> 匹配文本2：但我没有去过北京大学
>
> 匹配结果2：

说明：关键词前需紧跟 **(?<!** 否定词 **)**，匹配到关键词前紧挨着的否定词的内容不会输出。

### 4. 距离模式

语法：

> 前缀词 **.{ 最小间隔距离 , 最大间隔距离 }** 后缀词

### 5. 并列模式

语法：

> 匹配词1 **|** 匹配词2

### 6. 包装模式

语法：

> **(** 匹配词 **)**


## 优先级说明


## 构建和安装

### 1. 构建C库

```bash
# 下载源码
git clone --depth=1 --recurse-submodules --shallow-submodules https://github.com/ifplusor/actrie.git

# 进入源码目录
cd actrie

# 创建build目录并进入
mkdir build && pushd build

# 配置cmake工程
cmake -DCMAKE_BUILD_TYPE=Release ..

# 构建alib和actrie库
make actrie

# 离开build目录
popd
```

### 2. 构建和安装Python包

```bash
# 构建wheel包
python setup.py bdist_wheel

# 安装wheel包
pip install dist/actrie-*.whl

# 也可以从PyPI安装
pip install actrie
```

### 3. 构建和安装Java包

```bash
# 进入jni目录
pushd jni

# 构建nar包
mvn clean package

# 安装到本地maven仓库
mvn install

popd
```

## 使用样例

### **词表文件**: vocab.txt

```text
f|(a|b).{0,5}(e(?&!ef)|g)	pattern0
abc	pattern1
efg	pattern2
```

### **Python样例**: example.py

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

    # 从文件创建匹配器
    # matcher = Matcher.create_by_file("vocab.txt")

    # 从字符串创建匹配器
    matcher = Matcher.create_by_string(pattern)

    # 通过迭代器获取匹配结果
    for matched in matcher.finditer(content):
        print(matched)

    # 获取匹配结果列表
    all_matched = matcher.findall(content)
    print(all_matched)


if __name__ == "__main__":
    test()
```

### **Java样例**: Example.java

```java
package psn.ifplusor.actrie;

public class Example {

    public static void main(String[] args) {
        String pattern = "f|(a|b).{0,5}(e(?&!ef)|g)\nabc\nefg";
        String content = "abcdefg";

        // 从字符串创建匹配器
        try (Matcher matcher = Matcher.createByString(pattern)) {
            try (Context context = matcher.match(content)) {
                // 通过迭代器获取匹配结果
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
