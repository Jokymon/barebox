class InvalidOption:
    pass

class ConfigFile:
    def __init__(self):
        self._variables = {}

    def __getitem__(self, key):
        return self._variables[key]

    def read(self, filepath):
        f = open(filepath, "r")
        for l in f.readlines():
            l = l.strip()
            if len(l)>0 and l[0]!='#':
                key, val = l.split("=")
                self._variables[key] = val

    def yes(self, key):
        if not key in self._variables.keys():
            return False
        elif self[key] == "n":
            return False
        elif self[key] == "y":
            return True
        raise InvalidOption

    def append_if_config(self, items, item, config):
        if self.yes(config):
            items.append(item)
