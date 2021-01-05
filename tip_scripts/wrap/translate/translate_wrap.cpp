//#include "translator_helper_funcs.h"
#include <cstdio>
#include "Python.h"

// Module name is tip_translate

int RunTranslator(char* in_path, char* dts_in_path, char* tip_path)
{
	printf("before create str input path\n");
	//std::string str_input_path(in_path);
	//printf("before create mp input path\n");
	//ManagedPath input_path(str_input_path);

	//std::string str_dts_in_path(dts_in_path);
	//ManagedPath dts_path(str_dts_in_path);

	//TranslationConfigParams config;
	//std::string root_path(tip_path);
	//printf("before init config\n");
	//if (!InitializeConfig(root_path, config))
	//	return 1;
	//uint8_t thread_count = config.translate_thread_count_;

	//printf("DTS1553 path: %s\n", dts_path.RawString().c_str());
	//printf("Input: %s\n", input_path.RawString().c_str());
	//printf("Thread count: %hhu\n", thread_count);

	//DTS1553 dts1553;
	//std::map<uint64_t, std::string> chanid_to_bus_name_map;
	//std::set<uint64_t> excluded_channel_ids = std::set<uint64_t>();
	//if (!PrepareICDAndBusMap(dts1553, input_path, dts_path, config.stop_after_bus_map_,
	//	config.prompt_user_, config.vote_threshold_, config.vote_method_checks_tmats_,
	//	config.bus_name_exclusions_, config.tmats_busname_corrections_, config.use_tmats_busmap_,
	//	chanid_to_bus_name_map, excluded_channel_ids))
	//{
	//	return 0;
	//}

	//// Start translation routine for multi-threaded use case (or single-threaded using the threading framework
	//// if thread_count = 1 is specified).
	//if (thread_count > 0)
	//{
	//	if (!MTTranslate(config, input_path, dts1553.GetICDData(),
	//		dts_path, chanid_to_bus_name_map, excluded_channel_ids))
	//		return 1;
	//}
	//// Start the translation routine that doesn't use threading.
	//else
	//{
	//	if (!Translate(config, input_path, dts1553.GetICDData(),
	//		dts_path, chanid_to_bus_name_map, excluded_channel_ids))
	//		return 1;
	//}

	return 0;
}

extern "C"
{
	static PyObject* run_translator(PyObject* self, PyObject* args)
	{
		char* input_path;
		char* dts_path;
		char* tip_root_path;

		if (!PyArg_ParseTuple(args, "sss", &input_path, &dts_path, &tip_root_path))
		{
			Py_RETURN_NONE;
		}
		printf("before RunTranslator\n");
		int ret = RunTranslator(input_path, dts_path, tip_root_path);
		long long_ret = long(ret);
		return PyLong_FromLong(long_ret);
	}

	// Method definition object for this extension, these argumens mean:
	// ml_name: The name of the method
	// ml_meth: Function pointer to the method implementation
	// ml_flags: Flags indicating special features of this method, such as
	//          accepting arguments, accepting keyword arguments, being a
	//          class method, or being a static method of a class.
	// ml_doc:  Contents of this method's docstring
	static PyMethodDef tip_translate_methods[] = {
		{
			"run_translator", run_translator, METH_VARARGS,
			"Print 'run_translator' from a method defined in a C extension."
		}
	};

	// Module definition
	// The arguments of this structure tell Python what to call your extension,
	// what it's methods are and where to look for it's method definitions
	static struct PyModuleDef tip_translate_definition = {
		PyModuleDef_HEAD_INIT,
		"tip_translate",
		"A Python module that allows parsing of ch10 data",
		-1,
		tip_translate_methods
	};

	// Module initialization
	// Python calls this function when importing your extension. It is important
	// that this function is named PyInit_[[your_module_name]] exactly, and matches
	// the name keyword argument in setup.py's setup() call.
	PyMODINIT_FUNC PyInit_tip_translate(void) {
		Py_Initialize();
		return PyModule_Create(&tip_translate_definition);
	}
}