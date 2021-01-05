#include "parser_helper_funcs.h"
#include "Python.h"

// Module name is tip_parse

int RunParser(char* input_path, char* output_path, char* tip_path)
{
	ParserConfigParams config;
	if (!ValidateConfig(config, tip_path))
		return 1;

	ManagedPath mp_input_path;
	ManagedPath mp_output_path;
	if (!ValidatePaths(input_path, output_path, mp_input_path, mp_output_path))
		return 1;

	if(!StartParse(mp_input_path, mp_output_path, config))
		return 1;

	return 0;
}

extern "C"
{
	static PyObject* run_parser(PyObject* self, PyObject* args)
	{
		char* input_path;
		char* output_path;
		char* tip_root_path;

		if (!PyArg_ParseTuple(args, "sss", &input_path, &output_path, &tip_root_path))
		{
			Py_RETURN_NONE;
		}

		int ret = RunParser(input_path, output_path, tip_root_path);
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
	static PyMethodDef tip_parse_methods[] = {
		{
			"run_parser", run_parser, METH_VARARGS,
			"Print 'run_parser' from a method defined in a C extension."
		}
	};

	// Module definition
	// The arguments of this structure tell Python what to call your extension,
	// what it's methods are and where to look for it's method definitions
	static struct PyModuleDef tip_parse_definition = {
		PyModuleDef_HEAD_INIT,
		"tip_parse",
		"A Python module that allows parsing of ch10 data",
		-1,
		tip_parse_methods
	};

	// Module initialization
	// Python calls this function when importing your extension. It is important
	// that this function is named PyInit_[[your_module_name]] exactly, and matches
	// the name keyword argument in setup.py's setup() call.
	PyMODINIT_FUNC PyInit_tip_parse(void) {
		Py_Initialize();
		return PyModule_Create(&tip_parse_definition);
	}
}