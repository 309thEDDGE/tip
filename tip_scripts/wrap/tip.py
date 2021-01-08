from pathlib import Path
import tip_parse
import tip_translate
import tip_video

class TIP:

    def __init__(self, conf_path):
        self.conf_path = conf_path

    def parse(self, input_path, output_path):

        res = None
        if output_path is None:
            output_path = str(Path(input_path).parent)
            print('output_path:', output_path)

        res = tip_parse.run_parser(input_path, output_path, self.conf_path)

        return res

    def translate(self, input_path, dts_path):

        res = None
        res = tip_translate.run_translator(input_path, dts_path, self.conf_path)

        return res

    def extract_video(self, input_pq_path):

        res = None
        res = tip_video.extract_video(input_pq_path)

        return res