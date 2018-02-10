from distutils.core import setup, Extension
import numpy

print(numpy.get_include())

module = Extension('rcsv',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = [numpy.get_include()],
                    libraries = [],
                    library_dirs = [],
                    sources = ['rcsvmodule.c', 'rcsv.c'])

setup (name = 'RCSV',
       version = '1.0',
       description = 'CSV Reader/Writer',
       author = 'Remi Lespinet',
       author_email = 'remi.lespinet@gmail.com',
       url = '',
       long_description = '''
Fast CSV Reader/Writer
''',
       ext_modules = [module])
