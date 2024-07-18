#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <iostream>

namespace py = pybind11;

void echo(int i) {
    std::cout << i <<std::endl;
}



PYBIND11_MODULE(vistool_gl_py, m) {
    // Optional docstring
    m.doc() = "the renderer's py library";
        
    m.def("echo", &echo, "echo test func");
}
