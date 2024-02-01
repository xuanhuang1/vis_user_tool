#include <pybind11/pybind11.h>
#include <string>
#include <vector>

namespace py = pybind11;

void echo(int i);

static std::vector<std::string> init(const std::vector<std::string>& args);
int run_app();

namespace vistool {

    PYBIND11_MODULE(vistool_py, m) {
	// Optional docstring
	m.doc() = "the renderer's py library";
	
	m.def("echo", &echo, "A function that prints a number");
	m.def("init", &init, "init render app");
	m.def("run_app", &run_app, "run render app");
    }
}
