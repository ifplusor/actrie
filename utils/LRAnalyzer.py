#!/bin/env python3
# coding=utf-8


class IdGenerator:
    def __init__(self):
        self.id = -1

    def next(self):
        self.id += 1
        return self.id


class Queue(list):
    def pop(self, idx=0):
        return list.pop(self, idx)

    def push(self, obj):
        list.append(self, obj)

    def empty(self):
        return len(self) == 0


class UniQueue(Queue):
    def __init__(self):
        super(UniQueue, self).__init__()
        self.lookup = set()

    def push(self, obj):
        if obj not in self.lookup:
            self.lookup.add(obj)
            Queue.push(self, obj)


class Item:
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
        """
        next_token - item 的下一个文法符号
        :return:
        """
        if self.is_end():
            return None
        else:
            return self.pdct.tokens[self.anchor]

    def skip_token(self):
        if self.is_end():
            raise Exception("item index out of range")
        elif self.anchor + 1 == self.pdct.length:
            return None
        else:
            return self.pdct.tokens[self.anchor + 1]

    @staticmethod
    def identifier(pdct, anchor, forward=None):
        if anchor >= pdct.length:
            _str = pdct.gen + " -> " + " ".join(pdct.tokens) + " ."
        else:
            _str = pdct.gen + " -> " + " ".join(pdct.tokens[:anchor]) + " . " + " ".join(pdct.tokens[anchor:])
        if forward is not None:
            _str += ", " + forward
        return _str

    def __str__(self):
        if self._str is None:
            self._str = self.identifier(self.pdct, self.anchor, self.forward)
        return self._str

    def __eq__(self, other):
        return str(self) == str(other)

    def __hash__(self):
        return str(self).__hash__()


class ItemClosure:
    def __init__(self, analyzer, closure):
        self.analyzer = analyzer
        self.id = self.analyzer.closure_id.next()
        self._str = None

        self.closure = closure
        self._goto = dict()

        for item in closure:
            if item.is_end():
                self.have_reduce = True
                break
        else:
            self.have_reduce = False

        self.gen = set()
        if self.have_reduce:
            for item in closure:
                if item.is_end():
                    self.gen.add(item.pdct.gen)

    def build_goto(self, get_closure):
        direct_next = dict()
        for item in self.closure:
            if item.is_end():
                continue
            token = item.next_token()
            s = direct_next.get(token, None)
            if s is None:
                s = set()
                direct_next[token] = s
            s.add(self.analyzer.get_item(item.pdct, item.anchor+1, item.forward))
        for token in direct_next:
            self._goto[token] = get_closure(direct_next[token])

    def next_tokens(self):
        return set(self._goto.keys())

    def goto(self, token):
        return self._goto.get(token, None)

    def is_conflict(self):
        return self.have_reduce and (len(self._goto) >= 1 or len(self.closure) >= 2)

    def need_reduce(self):
        return self.have_reduce

    def reduce_item(self):
        return list(self.closure)[0]

    def reduce_item_set(self):
        lst = []
        for item in self.closure:
            if item.is_end():
                lst.append(item)
        return lst

    def is_reduce(self):
        return self.have_reduce and len(self._goto) == 0

    def __iter__(self):
        return self.closure.__iter__()

    def __len__(self):
        return len(self.closure)

    @staticmethod
    def identifier(closure_set):
        if len(closure_set) == 0:
            return ""
        elif len(closure_set) == 1:
            return str(list(closure_set)[0])
        else:
            lst = list(closure_set)
            lst.sort(lambda a, b: -cmp(str(a), str(b)))
            return "\n".join([str(item) for item in lst])

    def __str__(self):
        if self._str is None:
            self._str = self.identifier(self.closure)
        return self._str

    def __eq__(self, other):
        return str(self) == str(other)

    def __hash__(self):
        return str(self).__hash__()


class Production:
    def __init__(self, analyzer, text):
        self.analyzer = analyzer
        self.id = self.analyzer.pdct_id.next()
        self._str = None

        left, right = text.split("->")
        self.gen = left.strip()  # 产生式左部符号
        self.tokens = [x for x in right.split() if x]  # 产生式的右部符号序列
        self.length = len(self.tokens)

        # 生成 item
        # self.items = [Item(self.analyzer.item_id, self, anchor) for anchor in range(self.length + 1)]
        # for i in range(self.length):
        #     self.items[i].nitem = self.items[i+1]

    def __str__(self):
        if self._str is None:
            self._str = self.gen + " -> " + " ".join(self.tokens)
        return self._str

    def __eq__(self, other):
        return str(self) == str(other)

    def __hash__(self):
        return str(self).__hash__()


class LRAnalyzer:

    def __init__(self, path):
        # 产生式
        self.pdct_list = list()
        self.pdct_dict = dict()

        # 符号表
        self.terminator = set()
        self.non_terminator = set()

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

        self.read_pdcts(path)
        self.gen_dfa()

    def is_terminator(self, token):
        return token in self.terminator

    def is_non_terminator(self, token):
        return token in self.non_terminator

    def read_pdcts(self, path):
        with open(path) as fp:
            for line in fp.readlines():
                line = line.strip()
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
                    print("encounter error when read productions '%s', skipped..." % line)
                    pass

        self.non_terminator = set([pdct.gen for pdct in self.pdct_list])
        self.terminator = reduce(lambda s, pdct: s.union(pdct.tokens), self.pdct_list, set()) - self.non_terminator
        self.terminator.add("$")

    def get_first(self, gen, forward=None):
        """ get_first - 获取 FIRST(gen, forward) 集
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
            firsts = set()
            self.first_cache[gen] = firsts

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
        """ get_follow - 获取 FOLLOW(gen) 集
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
                                if pdct == self.pdct_dict["term"][0]:
                                    follows.add("$")
                                else:
                                    queue.push(pdct.gen)

        return follows

    def get_item(self, pdct, anchor, forward=None):
        id = Item.identifier(pdct, anchor, forward)
        item = self.item_cache.get(id, None)
        if item is None:
            item = Item(self, pdct, anchor, forward)
            self.item_cache[id] = item
        return item

    def get_closure(self, item_set, new_queue):
        """
        get_closure - 获取 item 集的 closure 集
        :param item_set:
        :param new_queue: 新 closure 队列
        :return:
        """
        closure = set(item_set)

        queue = UniQueue()
        for item in item_set:
            if item.is_end():
                continue

            # item 的期望的下一个 token 是未检查过的非终结符
            token = item.next_token()
            if self.is_non_terminator(token):
                queue.push(token)

            while not queue.empty():  # A -> alpha . B beta, a
                token = queue.pop()
                plist = self.pdct_dict[token]  # token 的产生式
                for pdct in plist:  # B -> gamma
                    beta = item.skip_token()
                    for first in self.get_first(beta, item.forward):
                        closure.add(self.get_item(pdct, 0, first))

                    token0 = pdct.tokens[0]
                    if self.is_non_terminator(token0):
                        queue.push(token0)

        id = ItemClosure.identifier(closure)
        item_closure = self.closure_cache.get(id, None)
        if item_closure is None:
            item_closure = ItemClosure(self, closure)
            self.closure_cache[id] = item_closure
            new_queue.push(item_closure)
        return item_closure

    def gen_dfa(self):
        # 初态
        term_pdct = self.pdct_dict["term"][0]
        # start_item = term_pdct.items[0]
        # accept_item = start_item.nitem

        start_item = self.get_item(term_pdct, 0, "$")
        accept_item = self.get_item(term_pdct, 1, "$")

        # 构造 closure
        queue = Queue()
        root_closure = self.get_closure({start_item}, queue)
        while not queue.empty():
            closure = queue.pop()
            self.closure_list.append(closure)
            closure.build_goto(lambda x: self.get_closure(x, queue))

        # 生成 lr 分析表
        for closure in self.closure_list:
            lr_analyze_item = dict()
            self.lr_analyze_table.append(lr_analyze_item)

            next_tokens = closure.next_tokens()
            for next_token in next_tokens:
                pass

            for item in closure:
                token = item.next_token()
                if token is None:
                    if item.forward == "$" and item.pdct.gen == "term":
                        lr_analyze_item["$"] = ("acpt", -1)
                    else:
                        forward = item.forward
                        if forward not in lr_analyze_item:
                            lr_analyze_item[forward] = ("rduc", item.pdct.id)
                        else:
                            action = lr_analyze_item[forward]
                            if action[0] != "rduc":
                                print("closure#%d have shift-reduce conflict:\n%s\n" % (closure.id, str(closure)))
                                exit(-1)  # conflict!
                            elif action[1] != item.pdct.id:
                                print("closure#%d have reduce-reduce conflict\n%s\n" % (closure.id, str(closure)))
                                exit(-1)  # conflict!
                else:
                    if token not in lr_analyze_item:
                        lr_analyze_item[token] = ("shft", closure.goto(token).id)
                    else:
                        action = lr_analyze_item[token]
                        if action[0] != "shft":
                            print("closure#%d have shift-reduce conflict:\n%s\n" % (closure.id, str(closure)))
                            exit(-1)  # conflict!

    def output(self, path):
        """
        output - 生成 C 语法的 LR 分析表
        :param path: 文件生成路径
        """
        with open(path, "w") as fp:
            fp.write("""/**
 * %s - LR(1) analyze table
 *
 * NOTE: this file is auto-generated by LRAnalyzer
 */
""" % path)

            terminator = [("text", "TOKEN_TEXT"), ("--", "TOKEN_ERR"), ("$", "TOKEN_EOF"),
                          ("(", "TOKEN_SUBS"), (")", "TOKEN_SUBE"), ("(?&!", "TOKEN_AMBI"),
                          ("(?<!", "TOKEN_ANTO"), (".{m,n}", "TOKEN_DIST"), ("|", "TOKEN_ALT")]

            non_terminator = list(self.non_terminator)
            non_to_id = dict()
            for i, token in enumerate(non_terminator):
                non_to_id[token] = i

            # write productions
            fp.write("""\n
/**
 * productions:
""")
            for i, pdct in enumerate(self.pdct_list):
                fp.write(" *   %2d: %s\n" % (i, str(pdct)))

            fp.write(" */\n")

            fp.write("#define LR_PDCT_NUM %d\n" % len(self.pdct_list))

            # write production to non-terminator index
            fp.write("""
int lr_pdct2nonid[LR_PDCT_NUM] = {
  """)

            for pdct in self.pdct_list:
                fp.write("%d, " % non_to_id[pdct.gen])

            fp.write("\n};\n")


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
                        fp.write("{ %s,%3d }, " % (analyze_item[token][0], analyze_item[token][1]))
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
                        fp.write("{ %s,%3d }, " % (analyze_item[token][0], analyze_item[token][1]))
                    else:
                        fp.write("{ deny,%3d }, " % -1)
                fp.write(" },\n")

            fp.write("};\n")

            fp.write("\n")


if __name__ == "__main__":
    lr_analyzer = LRAnalyzer("utils/productions.txt")
    lr_analyzer.output("src/lr_table.h")
