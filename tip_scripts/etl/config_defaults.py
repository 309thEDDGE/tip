parse_config = {
    "ch10_packet_type": {
        "MILSTD1553_FORMAT1": True,
        "VIDEO_FORMAT0": True,
        "ETHERNET_DATA0": True,
    },
    "parse_chunk_bytes": 200,
    "parse_thread_count": 2,
    "max_chunk_read_count": 1000,
    "worker_offset_wait_ms": 200,
    "worker_shift_wait_ms": 200,
}
trans_config = {
    "translate_thread_count": 1,
    "use_tmats_busmap": False,
    "tmats_busname_corrections": {},
    "prompt_user": False,
    "stop_after_bus_map": False,
    "vote_threshold": 1,
    "vote_method_checks_tmats": False,
    "bus_name_exclusions": [],
    "select_specific_messages": [],
    "exit_after_table_creation": False,
}

tip_parse_conf_schema = {
    "ch10_packet_type": {"MILSTD1553_FORMAT1": "BOOL", "VIDEO_FORMAT0": "BOOL"},
    "parse_chunk_bytes": "INT",
    "parse_thread_count": "INT",
    "max_chunk_read_count": "INT",
    "worker_offset_wait_ms": "INT",
    "worker_shift_wait_ms": "INT",
}

tip_translate_parse_conf_schema = {
    "translate_thread_count": "INT",
    "use_tmats_busmap": "BOOL",
    "tmats_busname_corrections": {"_NOT_DEFINED_OPT_": "STR"},
    "prompt_user": "BOOL",
    "stop_after_bus_map": "BOOL",
    "vote_threshold": "INT",
    "vote_method_checks_tmats": "BOOL",
    "bus_name_exclusions": ["OPTSTR"],
    "select_specific_messages": ["OPTSTR"],
    "exit_after_table_creation": "BOOL",
}
tip_dts1553_schema = {
    "translatable_message_definitions": {
        "_NOT_DEFINED_": {
            "msg_data": {
                "command": ["INT"],
                "lru_addr": ["INT"],
                "lru_subaddr": ["INT"],
                "lru_name": ["STR"],
                "bus": "STR",
                "wrdcnt": "INT",
                "rate": "FLT",
                "mode_code": "BOOL",
                "desc": "STR",
            },
            "word_elem": {
                "_NOT_DEFINED_OPT_": {
                    "off": "INT",
                    "cnt": "INT",
                    "schema": "STR",
                    "msbval": "FLT",
                    "desc": "STR",
                    "uom": "STR",
                    "multifmt": "BOOL",
                    "class": "INT",
                }
            },
            "bit_elem": {
                "_NOT_DEFINED_OPT_": {
                    "off": "INT",
                    "cnt": "INT",
                    "schema": "STR",
                    "msbval": "FLT",
                    "desc": "STR",
                    "uom": "STR",
                    "multifmt": "BOOL",
                    "class": "INT",
                    "msb": "INT",
                    "lsb": "INT",
                    "bitcnt": "INT",
                }
            },
        }
    },
    "supplemental_bus_map_command_words": {"_NOT_DEFINED_OPT_": [["INT"]]},
}
