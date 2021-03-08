# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
import subprocess
sys.path.insert(0, os.path.abspath('../bin'))
os.environ['PATH'] += "%s%s" % (os.pathsep, os.path.abspath('../bin'))

# -- Build wkt subdir first -------------------------------------------
# This generates mesh.rst from wkt_mesh.h, IF Lua is available.  Lua is not
# available and can't be installed on readthedocs.org, so we check mesh.rst
# into the repo and use the one that's there if we can't build it.
try:
    subprocess.check_call(["lua", "-v"])
    subprocess.call(["make", "-C", "wkt"])
except FileNotFoundError:
    print("No Lua found in this environment. Skipping WKT build")

# --  information -----------------------------------------------------

project = 'IREP'
copyright = '2016-2021, Lawrence Livermore National Laboratory'
author = 'Lee Busby'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.graphviz',
    'sphinxcontrib.programoutput',
]

# The name of the Pygments (syntax highlighting) style to use. We use our
# own extension of the default style with a few modifications mainly, the
# default RTD green background is hideous, so we want something nicer.
from pygments.style import Style
from pygments.styles.default import DefaultStyle
from pygments.token import Generic, Comment, Text

class IrepStyle(DefaultStyle):
    styles = DefaultStyle.styles.copy()
    background_color       = "#f4f4f8"
    styles[Generic.Output] = "#355"
    styles[Generic.Prompt] = "bold #346ec9"

import pkg_resources
dist = pkg_resources.Distribution(__file__)
sys.path.append('.')  # make 'conf' module findable
ep = pkg_resources.EntryPoint.parse('irep = conf:IrepStyle', dist=dist)
dist._ep_map = {'pygments.styles': {'plugin1': ep}}
pkg_resources.working_set.add(dist)

pygments_style = 'irep'

# Set default graphviz options
graphviz_dot_args = [
    '-Gbgcolor=transparent', '-Nstyle=rounded',
    '-Nshape=box', '-Nfontname=Helvetica', '-Nfontsize=10']

# Get nice vector graphics
graphviz_output_format = "svg"

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']
