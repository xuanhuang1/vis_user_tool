#!/usr/bin/env python
import sys, os
scriptdir = os.path.split(__file__)[0]
sys.path.insert(0, os.path.join(scriptdir, '..'))

import numpy
import vistool_py

vistool_py.init_app(sys.argv)
vistool_py.run_app()
