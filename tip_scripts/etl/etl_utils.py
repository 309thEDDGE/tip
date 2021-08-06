from etl.config_defaults import (
    parse_config,
    trans_config,
    tip_parse_conf_schema,
    tip_translate_parse_conf_schema,
    tip_dts1553_schema,
)
from pathlib import Path
import subprocess
import yaml


class tip_jacket:
    def __init__(
        self, tip_root: str = "./", generate_config: bool = True
    ) -> None:
        self.tip_root = Path(tip_root)
        self.generate_config = generate_config
        if self.generate_config:
            self.default_parse_config = parse_config
            self.default_translate_config = trans_config
            self.default_tip_parse_conf_schema = tip_parse_conf_schema
            self.default_tip_translate_parse_conf_schema = (
                tip_translate_parse_conf_schema
            )
            self.default_tip_dts1553_schema = tip_dts1553_schema
            print(
                f"Generating Default config files and directories under {self.tip_root.absolute()}"
            )
            self.init_paths_and_dir()
            self.init_files()

    def init_paths_and_dir(self) -> None:
        self.conf_path = self.tip_root / "conf"
        self.out_path = self.tip_root / "out"
        self.logs_path = self.tip_root / "logs"

        self.parse_out_path = self.out_path / "parse"

        self.translate_out_path = self.out_path / "translate"
        self.yaml_schemas_path = self.conf_path / "yaml_schemas"

        # makes directories
        self.conf_path.mkdir(parents=True, exist_ok=True)
        self.out_path.mkdir(parents=True, exist_ok=True)
        self.logs_path.mkdir(parents=True, exist_ok=True)
        self.parse_out_path.mkdir(parents=True, exist_ok=True)
        self.translate_out_path.mkdir(parents=True, exist_ok=True)
        self.yaml_schemas_path.mkdir(parents=True, exist_ok=True)

        # mandatory to run
        self.parse_conf = self.conf_path / "parse_conf.yaml"
        self.translate_conf = self.conf_path / "translate_conf.yaml"
        self.parse_schema_path = self.yaml_schemas_path / "tip_parse_conf_schema.yaml"
        self.translate_schema_path = (
            self.yaml_schemas_path / "tip_translate_conf_schema.yaml"
        )
        self.dts_schema_path = self.yaml_schemas_path / "tip_dts1553_schema.yaml"

    def init_files(self) -> None:
        tip_jacket.create_yaml_if_not_exists(self.parse_conf, self.default_parse_config)
        tip_jacket.create_yaml_if_not_exists(
            self.translate_conf, self.default_translate_config
        )
        tip_jacket.create_yaml_if_not_exists(
            self.parse_schema_path, self.default_tip_parse_conf_schema
        )
        tip_jacket.create_yaml_if_not_exists(
            self.translate_schema_path, self.default_tip_translate_parse_conf_schema
        )
        tip_jacket.create_yaml_if_not_exists(
            self.dts_schema_path, self.default_tip_dts1553_schema
        )

    @staticmethod
    def create_yaml_if_not_exists(path, config) -> None:
        if not path.exists():
            with open(path, "w") as f:
                yaml.dump(config, f, sort_keys=False, allow_unicode=True)
                print(f"-------Created yaml {path}---------------------")

    def conf_set_up(self, out, conf, logs, *, job_type="parse"):

        if out:
            if job_type == "parse":
                self.parse_out_path = Path(out).absolute()
                self.parse_out_path.mkdir(parents=True, exist_ok=True)
            elif job_type == "translate":
                self.translate_out_path = Path(out).absolute()
                self.translate_out_path.mkdir(parents=True, exist_ok=True)
        if conf:
            self.conf_path = Path(conf).absolute()
            self.conf_path.mkdir(parents=True, exist_ok=True)
        if logs:
            self.logs_path = Path(logs).absolute()
            self.logs_path.mkdir(parents=True, exist_ok=True)

    def parse_ch10(
        self,
        binary_file_path: str,
        out_path: str = None,
        conf_path: str = None,
        logs_path: str = None,
    ) -> None:
        job_type = "parse"
        self.conf_set_up(out_path, conf_path, logs_path, job_type=job_type)

        tip_parse_cmd = "tip_parse {0} {1} {2} {3}"
        parse_path = Path(binary_file_path).absolute()

        out = self.parse_out_path.absolute()
        conf = self.conf_path.absolute()
        logs = self.logs_path.absolute()

        tip_parse_cmd = tip_parse_cmd.format(parse_path, out, conf, logs)
        print(f" Executing | {tip_parse_cmd}")
        process_out = subprocess.run(
            tip_parse_cmd, capture_output=True, shell=True, text=True, check=True
        )
        print(process_out)

    def translate(
        self,
        binary_file_path: str,
        dts_yaml: str,
        out_path: str = None,
        conf_path: str = None,
        logs_path: str = None,
    ) -> None:

        job_type = "translate"
        self.conf_set_up(out_path, conf_path, logs_path, job_type=job_type)

        tip_trans_cmd = "tip_translate {0} {1} {2} {3} {4}"

        trans_path = Path(binary_file_path).absolute()
        dts_path = Path(dts_yaml).absolute()
        out = self.translate_out_path.absolute()
        conf = self.conf_path.absolute()
        logs = self.logs_path.absolute()

        tip_trans_cmd = tip_trans_cmd.format(
            trans_path, dts_path, out, conf, logs
        )
        print(f" Executing | {tip_trans_cmd}")
        process_out = subprocess.run(
            tip_trans_cmd, capture_output=True, shell=True, text=True, check=True
        )
        print(process_out)
