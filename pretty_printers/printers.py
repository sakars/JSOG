import gdb

class TextRange:
    def __init__(self, val):
        self.val = val
        if val is None:
            self.start = 0
            self.end = 0
            self.has_value = False
        else:
            self.start = val['first']
            self.end = val['afterLast']
            self.has_value = True if int(self.start) != 0 and int(self.end) != 0 else False

    # def __str__(self):
    #     return self.to_string()

    def display_hint(self):
        return 'string'

    def to_string(self):
        start_addr = int(self.start)
        end_addr = int(self.end)
        if start_addr == 0 or end_addr == 0:
            return "(empty)"
        length = end_addr - start_addr
        try:
            return gdb.selected_inferior().read_memory(start_addr, length).tobytes().decode('utf-8')
        except gdb.error:
            return "(invalid memory)"


class UriUriStructA:
    def __init__(self, val):
        self.val = val
        self.scheme = TextRange(val['scheme'])
        self.userInfo = TextRange(val['userInfo'])
        self.hostText = TextRange(val['hostText'])
        self.portText = TextRange(val['portText'])
        self.query = TextRange(val['query'])
        self.fragment = TextRange(val['fragment'])
        self.absolutePath = val['absolutePath']
        self.owner = val['owner']

    def to_string(self):
        scheme = self.scheme.to_string() + "://" if self.scheme.has_value else ""
        userInfo = self.userInfo.to_string() + "@" if self.userInfo.has_value else ""
        hostText = self.hostText.to_string() if self.hostText.has_value else ""
        portText = ":" + self.portText.to_string() if self.portText.has_value else ""
        query = "?" + self.query.to_string() if self.query.has_value else ""
        fragment = "#" + self.fragment.to_string() if self.fragment.has_value else ""
        return f"{scheme}{userInfo}{hostText}{portText}{query}{fragment}"

    def display_hint(self):
        return 'array'

    def children(self):
        return [
            ('scheme', self.val['scheme']),
            ('userInfo', self.val['userInfo']),
            ('hostText', self.val['hostText']),
            ('portText', self.val['portText']),
            ('query', self.val['query']),
            ('fragment', self.val['fragment']),
            ('absolutePath', self.val['absolutePath']),
            ('owner', self.val['owner'])
        ]

class UriWrapper:
    def __init__(self, val):
        self.val = val
        self.uri_ = UriUriStructA(val['uri_'])

    def to_string(self):
        return self.uri_.to_string()

    def display_hint(self):
        return 'array'

    def children(self):
        return [
            ('uri_', self.val['uri_'])
        ]


def uri_struct_printer(val):
    try:
        if str(val.type) == 'UriUriStructA *' and val:
            return UriUriStructA(val.dereference())
        if str(val.type) == 'UriUriStructA':
            return UriUriStructA(val)
        if str(val.type) == 'UriTextRangeA *' and val:
            return TextRange(val.dereference())
        if str(val.type) == 'UriTextRangeA':
            return TextRange(val)
        if str(val.type) == 'UriWrapper':
            return UriWrapper(val)
        if str(val.type) == 'UriWrapper *':
            return UriWrapper(val)
    except gdb.error:
        return None
    return None


if __name__ == "__main__":
    gdb.pretty_printers.append(uri_struct_printer)