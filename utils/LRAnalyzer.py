#!/bin/env python3
# coding=utf-8

from functools import reduce
import sys

is_py3k = bool(sys.version_info[0] == 3)


class IdGenerator:
    """id 生成器"""

    def __init__(self):
        self.id = -1

    def next(self):
        self.id += 1
        return self.id


class Queue(list):
    """普通队列"""

    def pop(self, idx=0):
        return super(Queue, self).pop(idx)

    def push(self, obj):
        super(Queue, self).append(obj)

    def empty(self):
        return len(self) == 0


class UniQueue(Queue):
    """记忆队列，元素仅可入队一次"""

    def __init__(self):
        super(UniQueue, self).__init__()
        self.lookup = set()

    def push(self, obj):
        if obj not in self.lookup:
            self.lookup.add(obj)
            super(UniQueue, self).push(obj)


class Item:
    """项目"""

    def __init__(self, analyzer, pdct, anchor, forward=None):
        self.analyzer = analyzer
        self.id = self.analyzer.item_id.next()
        self._str = None

        self.pdct = pdct
        self.anchor = anchor
        self.forward = forward
        self.nitem = None

    def is_end(self):
        return self.anchor >= self.pdct.length

    def next_token(self):
        """next_token - item 的下一个文法符号
        :return: 下一个文法符号
        """
        if self.is_end():
            return None
        else:
            return self.pdct.tokens[self.anchor]

    def skip_token(self):
        if self.is_end():
            raise Exception("item index out of range")
        elif self.anchor + 1 >= self.pdct.length:
            return None
        else:
            return self.pdct.tokens[self.anchor + 1]

    @staticmethod
    def identifier(pdct, anchor, forward=None):
        """identifier - 生成 item 的符号表示
        :return: item 的符号表示
        """
        if anchor >= pdct.length:
            _str = "{} -> {} #".format(pdct.gen, " ".join(pdct.tokens))
        elif anchor <= 0:
            _str = "{} -> # {}".format(pdct.gen, " ".join(pdct.tokens))
        else:
            _str = "{} -> {} # {}".format(pdct.gen, " ".join(pdct.tokens[:anchor]), " ".join(pdct.tokens[anchor:]))
        if forward is not None:
            _str += "; {}".format(forward)
        return _str

    def __str__(self):
        if self._str is None:
            self._str = self.identifier(self.pdct, self.anchor, self.forward)
        return self._str

    def __eq__(self, other):
        return str(self) == str(other)
        # Dear husband, I love u forever. - by Amaris

    def __hash__(self):
        return str(self).__hash__()


class ItemClosure:
    """项目集（闭包）"""

    def __init__(self, analyzer, items):
        self.analyzer = analyzer
        self.id = self.analyzer.closure_id.next()
        self._str = None

        self.prefix = LRAnalyzer.token_eof
        self.items = items  # item 集合
        self._goto = dict()

        for item in items:
            if item.is_end():
                self.has_reduce = True
                break
        else:
            self.has_reduce = False

        self.gen = set()  # 可规约的左部符号集合
        if self.has_reduce:
            for item in self.items:
                if item.is_end():
                    self.gen.add(item.pdct.gen)

    def build_goto(self, get_closure):
        """build_goto - 构造状态转换表"""
        direct_next = dict()
        for item in self.items:
            if item.is_end():
                continue
            token = item.next_token()
            s = direct_next.get(token, None)
            if s is None:
                direct_next[token] = s = set()
            s.add(self.analyzer.get_item(item.pdct, item.anchor+1, item.forward))
        for token in direct_next:
            self._goto[token] = get_closure(direct_next[token])
            self._goto[token].prefix = token

    def next_tokens(self):
        return set(self._goto.keys())

    def goto(self, token):
        return self._goto.get(token, None)

    def is_conflict(self):
        return self.has_reduce and (len(self._goto) >= 1 or len(self.items) >= 2)

    def need_reduce(self):
        return self.has_reduce

    def reduce_item(self):
        return list(self.closure)[0]

    def reduce_item_set(self):
        lst = []
        for item in self.items:
            if item.is_end():
                lst.append(item)
        return lst

    def is_reduce(self):
        return self.has_reduce and len(self._goto) == 0

    def __iter__(self):
        return self.items.__iter__()

    def __len__(self):
        return len(self.items)

    @staticmethod
    def identifier(items):
        """identifier - 生成 closure 的符号表示
        :return: closure 的符号表示
        """
        if len(items) == 0:
            return ""
        elif len(items) == 1:
            return str(list(items)[0])
        else:
            lst = list(items)
            lst.sort(key=str)
            return "\n".join([str(item) for item in lst])

    def __str__(self):
        if self._str is None:
            self._str = self.identifier(self.items)
        return self._str

    def __eq__(self, other):
        return str(self) == str(other)

    def __hash__(self):
        return str(self).__hash__()


class Production:
    """产生式"""

    def __init__(self, analyzer, text):
        self.analyzer = analyzer
        self.id = self.analyzer.pdct_id.next()
        self._str = None

        left, right = text.split("->")
        self.gen = left.strip()  # 产生式左部符号
        self.tokens = right.split()  # 产生式的右部符号序列
        self.length = len(self.tokens)

    def __str__(self):
        if self._str is None:
            self._str = self.gen + " -> " + " ".join(self.tokens)
        return self._str

    def __eq__(self, other):
        return str(self) == str(other)

    def __hash__(self):
        return str(self).__hash__()


class LRAnalyzer:
    """LR(1) 分析器"""

    gen_super = "pattern"
    gen_term = "term"
    gen_wrap = "wrap"
    gen_alter = "alter"
    gen_join = "join"
    gen_dist = "dist"
    gen_ddist = "ddist"
    gen_anti_anto = "anti-anto"
    gen_anto = "anto"
    gen_anti_ambi = "anti-ambi"
    gen_ambi = "ambi"
    gen_plain = "plain"

    token_text = "text"
    token_eof = "$"
    token_err = "--"
    token_subs = "("
    token_sube = ")"
    token_ambi = "(?&!"
    token_anto = "(?<!"
    token_any = "."
    token_num = "\\d"
    token_rept = "{m,n}"
    token_alt = "|"

    action_deny = "deny"
    action_accept = "acpt"
    action_shift = "shft"
    action_reduce = "rduc"

    def __init__(self, source_path, priority_path):
        # 产生式
        self.pdct_list = list()
        self.pdct_dict = dict()

        # 符号表
        self.terminator = set()  # 终结符
        self.non_terminator = set()  # 非终结符

        self.item_cache = dict()
        self.item_list = list()

        self.closure_cache = dict()
        self.closure_list = list()

        self.lr_analyze_table = list()

        self.follow_cache = dict()
        self.first_cache = dict()

        # id 生成器
        self.pdct_id = IdGenerator()
        self.item_id = IdGenerator()
        self.closure_id = IdGenerator()

        self.priority_list = list()
        self.priority_set = set()

        self.read_pdcts(source_path)
        self.read_priority(priority_path)
        self.build_dfa()

    def is_terminator(self, token):
        return token in self.terminator

    def is_non_terminator(self, token):
        return token in self.non_terminator

    def read_pdcts(self, path):
        """read_pdcts - 从文件中读取产生式
        :param path: 文件路径
        :return:
        """
        if is_py3k:
            def fopen(fpath): return open(fpath, encoding="utf-8")
        else:
            def fopen(fpath): return open(fpath)

        with fopen(path) as fp:
            for line in fp.readlines():
                line = line.split("#")[0].strip()
                if not line:
                    continue
                try:
                    pdct = Production(self, line)
                    self.pdct_list.append(pdct)
                    plist = self.pdct_dict.get(pdct.gen, None)
                    if plist is None:
                        self.pdct_dict[pdct.gen] = plist = []
                    plist.append(pdct)
                except Exception as e:
                    print("encounter error when read productions '{}', skipped...".format(line))
                    exit(-1)

        self.non_terminator = set([pdct.gen for pdct in self.pdct_list])
        self.terminator = reduce(lambda s, pdct: s.union(
            pdct.tokens), self.pdct_list, set()) - self.non_terminator
        self.terminator.add(LRAnalyzer.token_eof)

    def read_priority(self, path):
        if not path:
            return

        if is_py3k:
            def fopen(fpath): return open(fpath, encoding="utf-8")
        else:
            def fopen(fpath): return open(fpath)

        with fopen(path) as fp:
            self.priority_list = filter(lambda line: len(line) > 0, map(lambda line: line.split("#")[0].strip(), fp.readlines()))
            self.priority_set = set(self.priority_list)

    def get_first(self, gen, forward=None):
        """get_first - 获取 FIRST(gen, forward) 集
        :param gen: 期待的文法符号
        :param forward: 终结符
        :return: 终结符集合
        """
        if gen in self.terminator:  # 终结符，直接返回
            return {gen}
        elif gen is None:
            return {forward}

        firsts = self.first_cache.get(gen, None)  # cache
        if firsts is None:
            self.first_cache[gen] = firsts = set()

            queue = UniQueue()
            queue.push(gen)
            while not queue.empty():
                gen = queue.pop()
                for pdct in self.pdct_dict[gen]:
                    token = pdct.tokens[0]
                    if token in self.terminator:  # 终结符
                        firsts.add(token)
                    elif token in self.non_terminator:  # 非终结符
                        queue.push(token)

        if forward is None or len(firsts) > 0:
            return firsts
        else:
            return {forward}

    def get_follow(self, gen):
        """get_follow - 获取 FOLLOW(gen) 集
        :param gen: 非终结符
        :return: 终结符集合
        """
        follows = self.follow_cache.get(gen, None)  # cache
        if follows is None:
            follows = set()
            self.follow_cache[gen] = follows

            queue = UniQueue()
            queue.push(gen)

            while not queue.empty():
                gen = queue.pop()
                for pdct in self.pdct_list:
                    for i, token in enumerate(pdct.tokens):
                        if token == gen:
                            if i < pdct.length - 1:
                                next_token = pdct.tokens[i + 1]
                                if next_token in self.terminator:
                                    follows.add(next_token)
                                else:
                                    follows.update(self.get_first(token))
                            else:
                                if pdct == self.pdct_dict[LRAnalyzer.gen_super][0]:
                                    follows.add(LRAnalyzer.token_eof)
                                else:
                                    queue.push(pdct.gen)

        return follows

    def get_item(self, pdct, anchor, forward=None):
        item_id = Item.identifier(pdct, anchor, forward)
        item = self.item_cache.get(item_id, None)
        if item is None:
            item = Item(self, pdct, anchor, forward)
            self.item_cache[item_id] = item
        return item

    def expand_items_to_closure(self, item_set):
        """expand_items_to_closure - 扩展 item 集合为 closure
        :param item_set: item 的集合
        :return: closure 的 item 集合
        """
        queue = UniQueue()
        for item in item_set:
            queue.push(item)

        while not queue.empty():
            item_A = queue.pop()  # A -> alpha . B beta; a

            # item(A -> alpha . B beta; a) 的期望的下一个 token(B) 是非终结符
            token_B = item_A.next_token()
            if self.is_non_terminator(token_B):
                token_beta = item_A.skip_token()
                plist = self.pdct_dict[token_B]  # B 的产生式
                for pdct_B in plist:  # B -> gamma
                    for first in self.get_first(token_beta, item_A.forward):
                        extend_item = self.get_item(pdct_B, 0, first)
                        queue.push(extend_item)

        return queue.lookup

    def get_closure(self, item_set, closure_queue):
        """get_closure - 获取 item 集的 closure 集
        :param item_set:
        :param new_queue: 接收新 closure 的队列
        :return:
        """
        items_in_closure = self.expand_items_to_closure(item_set)
        closure_id = ItemClosure.identifier(items_in_closure)
        closure = self.closure_cache.get(closure_id, None)
        if closure is None:
            closure = ItemClosure(self, items_in_closure)
            self.closure_cache[closure_id] = closure
            if closure_queue is not None:
                closure_queue.push(closure)
        return closure

    def reduce_over_shift(self, pdct, token):
        try:
            pdct_priority = self.priority_list.index(pdct.gen)
            token_priority = self.priority_list.index(token)
        except:
            # 未出现在优先级表中的符号默认优先级低
            raise Exception("Priority not recognized.\n production: {}\n token: {}".format(pdct, token))

        if pdct_priority > token_priority:  # 左结合
            print("priority: '{}' > '{}'".format(pdct, token))
            return True
        else:
            print("priority: '{}' < '{}'".format(pdct, token))
            return False

    def build_dfa(self):
        # 初态, 增广规则
        super_pdct = self.pdct_dict[LRAnalyzer.gen_super][0]

        start_item = self.get_item(super_pdct, 0, LRAnalyzer.token_eof)
        accept_item = self.get_item(super_pdct, 1, LRAnalyzer.token_eof)

        # 构造 closure
        closure_queue = Queue()
        root_closure = self.get_closure({start_item}, closure_queue)
        while not closure_queue.empty():
            closure = closure_queue.pop()
            self.closure_list.append(closure)
            closure.build_goto(lambda items, _queue=closure_queue: self.get_closure(items, _queue))

        # 生成 lr 分析表
        for closure in self.closure_list:
            # new table item
            lr_analyze_item = dict()
            self.lr_analyze_table.append(lr_analyze_item)

            for item in closure:
                token = item.next_token()
                if token is None:
                    # will reduce
                    if item.id == accept_item.id:
                        # accept
                        lr_analyze_item[LRAnalyzer.token_eof] = (LRAnalyzer.action_accept, -1)
                    else:
                        forward = item.forward
                        if forward not in lr_analyze_item:
                            # reduce
                            lr_analyze_item[forward] = (LRAnalyzer.action_reduce, item.pdct.id)
                        else:
                            action, pdct_or_closure_id = lr_analyze_item[forward]
                            if action == LRAnalyzer.action_shift:
                                # shift-reduce conflict
                                if self.reduce_over_shift(item.pdct, forward):  # check priority
                                    # overwrite by reduce
                                    lr_analyze_item[forward] = (LRAnalyzer.action_reduce, item.pdct.id)
                            elif action == LRAnalyzer.action_reduce:
                                if item.pdct.id != pdct_or_closure_id:
                                    # reduce-reduce conflict
                                    print("closure#{} have reduce-reduce conflict\n{}\ncurrent item:{}\n".format(closure.id, closure, item))
                                    exit(-1)
                            else:
                                print("closure#{} have unknown conflict\n{}\ncurrent item:{}\n".format(closure.id, closure, item))
                                exit(-1)
                else:
                    # will shift
                    if token not in lr_analyze_item:
                        # shift
                        lr_analyze_item[token] = (LRAnalyzer.action_shift, closure.goto(token).id)
                    else:
                        action, pdct_or_closure_id = lr_analyze_item[token]
                        if action == LRAnalyzer.action_reduce:
                            # shift-reduce conflict
                            if not self.reduce_over_shift(self.pdct_list[pdct_or_closure_id], token):  # check priority
                                # overwrite by shift
                                lr_analyze_item[token] = (LRAnalyzer.action_shift, closure.goto(token).id)
                        elif action != LRAnalyzer.action_shift:
                            print("closure#{} have unknown conflict\n{}\ncurrent item:{}\n".format(closure.id, closure, item))
                            exit(-1)

    def output(self, path):
        """
        output - 生成 C 语法的 LR 分析表
        :param path: 文件生成路径
        """
        with open(path, "w") as fp:
            fp.write("""/**
 * %s - LR(1) analyze table
 *
 * NOTE: this file is auto-generated by LRAnalyzer.py
 */

#ifndef _LR_ANALYZER_TABLE_H_
#define _LR_ANALYZER_TABLE_H_

#include "lr_reduce.h"

// clang-format off
""" % path)

            terminator = [
                (LRAnalyzer.token_text, "TOKEN_TEXT"),
                (LRAnalyzer.token_eof,  "TOKEN_EOF"),
                (LRAnalyzer.token_err,  "TOKEN_ERR"),
                (LRAnalyzer.token_subs, "TOKEN_SUBS"),
                (LRAnalyzer.token_sube, "TOKEN_SUBE"),
                (LRAnalyzer.token_ambi, "TOKEN_AMBI"),
                (LRAnalyzer.token_anto, "TOKEN_ANTO"),
                (LRAnalyzer.token_any,  "TOKEN_ANY"),
                (LRAnalyzer.token_num,  "TOKEN_NUM"),
                (LRAnalyzer.token_rept, "TOKEN_REPT"),
                (LRAnalyzer.token_alt,  "TOKEN_ALT")
            ]

            non_terminator = list(self.non_terminator)
            non_to_id = dict()
            for i, token in enumerate(non_terminator):
                non_to_id[token] = i

            # write productions
            fp.write("""
/**
 * productions:
""")
            for i, pdct in enumerate(self.pdct_list):
                fp.write(" *   %2d: %s\n" % (i, str(pdct)))

            fp.write(" */\n")

            fp.write("""
#define LR_PDCT_NUM %d
""" % len(self.pdct_list))

            # write production to non-terminator index
            fp.write("""
static const int lr_pdct2nonid[LR_PDCT_NUM] = {
  """)

            for pdct in self.pdct_list:
                fp.write("%d, " % non_to_id[pdct.gen])

            fp.write("""
};
""")

            # write production to reduce function
            fp.write("""
static const lr_reduce_func lr_reduce_func_table[LR_PDCT_NUM] = {
""")

            for pdct in self.pdct_list:
                if pdct.gen == LRAnalyzer.gen_super:
                    fp.write("    NULL,\n")
                elif pdct.gen == LRAnalyzer.gen_term:
                    fp.write("    reduce_only_pop,\n")
                elif pdct.gen == LRAnalyzer.gen_plain:
                    fp.write("    reduce_text2pure,\n")
                elif pdct.gen == LRAnalyzer.gen_alter:
                    fp.write("    reduce_alter,\n")
                elif pdct.gen == LRAnalyzer.gen_join:
                    fp.write("    reduce_join,\n")
                elif pdct.gen in {LRAnalyzer.gen_dist, LRAnalyzer.gen_ddist}:
                    fp.write("    reduce_dist,\n")
                elif pdct.gen == LRAnalyzer.gen_anti_anto:
                    fp.write("    reduce_anto,\n")
                elif pdct.gen == LRAnalyzer.gen_anti_ambi:
                    fp.write("    reduce_ambi,\n")
                elif pdct.gen in {LRAnalyzer.gen_wrap, LRAnalyzer.gen_anto, LRAnalyzer.gen_ambi}:
                    fp.write("    reduce_unwrap,\n")
                else:
                    raise Exception("error production")

            fp.write("""
};
""")

            fp.write("""\n
typedef enum _lr_action {
  deny = 0, // deny
  acpt,     // accept
  shft,     // shift
  rduc      // reduce
} lr_action_e;
""")

            fp.write("""\n
typedef struct _lr_table_item {
  lr_action_e action;
  int index;
} lr_item_s, *lr_item_t;
""")

            # write action table
            fp.write("""\n
lr_item_s lr_action_table[%d][%d] = {
""" % (len(self.lr_analyze_table), len(terminator)))

            fp.write("/*       ")
            for _, desc in terminator:
                fp.write("   %10s," % desc)
            fp.write(" */\n")

            for i, analyze_item in enumerate(self.lr_analyze_table):
                fp.write("/* %3d */ { " % i)
                for token, _ in terminator:
                    if token in analyze_item:
                        fp.write("{ %s,%3d }, " % (
                            analyze_item[token][0], analyze_item[token][1]))
                    else:
                        fp.write("{ deny,%3d }, " % -1)
                fp.write(" },\n")

            fp.write("};\n")

            # write goto table
            fp.write("""\n
lr_item_s lr_goto_table[%d][%d] = {
""" % (len(self.lr_analyze_table), len(non_terminator)))

            for i, analyze_item in enumerate(self.lr_analyze_table):
                fp.write("/* %3d */ { " % i)
                for token in non_terminator:
                    if token in analyze_item:
                        fp.write("{ %s,%3d }, " % (
                            analyze_item[token][0], analyze_item[token][1]))
                    else:
                        fp.write("{ deny,%3d }, " % -1)
                fp.write(" },\n")

            fp.write("};\n")

            fp.write("""
// clang-format on

#endif  // _LR_ANALYZER_TABLE_H_
""")


if __name__ == "__main__":
    import sys
    import optparse

    usage = "usage: %prog [options]"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("-s", "--source", dest="source", type=str, metavar="PATH", help="productions.syntax path")
    parser.add_option("-o", "--output", dest="output", type=str, metavar="PATH", help="lr_table.c output path")
    parser.add_option("-p", "--priority", dest="priority", type=str, metavar="PATH", help="priority.syntax path")
    options, args = parser.parse_args(args=sys.argv[1:])

    print("generate LR(1) table, source='{}', output='{}'".format(options.source, options.output))

    lr_analyzer = LRAnalyzer(options.source, options.priority)
    lr_analyzer.output(options.output)
