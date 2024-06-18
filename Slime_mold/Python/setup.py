import setuptools  # important
from distutils.core import setup
from Cython.Build import cythonize
import numpy
setup(ext_modules=cythonize('Generative_art_cy.pyx', build_dir="build", annotate=True),
                                           include_dirs=[numpy.get_include()],
                                           script_args=['build'], 
                                           options={'build':{'build_lib':'.'}})

# print(help(cythonize))
# import Generative_art_cy