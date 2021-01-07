#include "arrow/status.h"
#include "parquet_video_extraction.h"
#include <ctime>
#include <chrono>
#include "Python.h"

// Module name is tip_video

int ExtractVideo(char* input_path, double& duration)
{
	auto t1 = std::chrono::high_resolution_clock::now();

	ParquetVideoExtraction pe;

	std::string input_pq_video_dir(input_path);
	if (!pe.Initialize(ManagedPath(input_pq_video_dir)))
		return 1;

	if (!pe.ExtractTS())
		return 1;

	auto t2 = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();

	return 0;
}

extern "C"
{
	static PyObject* extract_video(PyObject* self, PyObject* args)
	{
		char* input_path;

		if (!PyArg_ParseTuple(args, "s", &input_path))
		{
			Py_BuildValue("[id]", -1, 0.0);
		}

		double duration = 0.0;
		int ret = ExtractVideo(input_path, duration);
		return Py_BuildValue("[id]", ret, duration);
	}

	// Method definition object for this extension, these argumens mean:
	// ml_name: The name of the method
	// ml_meth: Function pointer to the method implementation
	// ml_flags: Flags indicating special features of this method, such as
	//          accepting arguments, accepting keyword arguments, being a
	//          class method, or being a static method of a class.
	// ml_doc:  Contents of this method's docstring
	static PyMethodDef vextract_methods[] = {
		{
			"extract_video", extract_video, METH_VARARGS,
			"Print 'extract_video' from a method defined in a C extension."
		},
		{NULL, NULL, 0, NULL} // This is used to delimit the array
	};

	// Module definition
	// The arguments of this structure tell Python what to call your extension,
	// what it's methods are and where to look for it's method definitions
	static struct PyModuleDef vextract_definition = {
		PyModuleDef_HEAD_INIT,
		"tip_video",
		"A Python module that extracts video from TIP Parquet video archives",
		-1,
		vextract_methods
	};

	// Module initialization
	// Python calls this function when importing your extension. It is important
	// that this function is named PyInit_[[your_module_name]] exactly, and matches
	// the name keyword argument in setup.py's setup() call.
	PyMODINIT_FUNC PyInit_tip_video(void) {
		Py_Initialize();
		return PyModule_Create(&vextract_definition);
	}
}