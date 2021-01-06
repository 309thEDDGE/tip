import importlib
# Instantiate this class, then check if ready:
#
# t = TIP()
# if t.is_ready():
#     do stuff . . . 
#     t.parse() # see parse.parse.parse
#     t.translate() # see translate.translate.translate
# 

class TIP:

    def __init__(self):
        self.parse = None
        self.translate = None
        self.ready = False
        self.ready = self._import_tip_packages()
        self._assign_funcs()
        
    def is_ready(self):
        return self.ready

    def _import_tip_packages(self):

        #try:
        #    import tip_parse
        #    import tip_translate
        #except ModuleNotFoundError:
        #    return False
        tip_parse_spec = importlib.util.find_spec('tip_parse')
        tip_parse_found = tip_parse_spec is not None

        tip_translate_spec = importlib.util.find_spec('tip_translate')
        tip_translate_found = tip_translate_spec is not None

        if tip_parse_found and tip_translate_found:
            return True
        else:
            return False

    def _assign_funcs(self):

        if self.ready:
            from .parse.parse import parse
            self.parse = parse

            from .translate.translate import translate
            self.translate = translate