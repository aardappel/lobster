# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""@file buildutil/linux.py Linux-specific BuildEnvironment sub-module.
@namespace buildutil.linux

Optional environment variables:

@li CMAKE_PATH = Path to CMake binary. Required if cmake is not in $PATH,
or not passed on command line.
@li CMAKE_FLAGS = String to override the default CMake flags with.
"""


import distutils.spawn
import os
import shlex
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir))
import buildutil.common as common

_CMAKE_PATH_ENV_VAR = 'CMAKE_PATH'
_CMAKE_FLAGS_ENV_VAR = 'CMAKE_FLAGS'
_CMAKE_PATH = 'cmake_path'
_CMAKE_FLAGS = 'cmake_flags'


class BuildEnvironment(common.BuildEnvironment):

  """Class representing a Linux build environment.

  This class adds Linux-specific functionality to the common
  BuildEnvironment.

  Attributes:
    cmake_path: Path to the cmake binary, for cmake-based projects.
    cmake_flags: Flags to pass to cmake, for cmake-based projects.
  """

  CMAKE = 'cmake'

  def __init__(self, arguments):
    """Constructs the BuildEnvironment with basic information needed to build.

    The build properties as set by argument parsing are also available
    to be modified by code using this object after construction.

    It is required to call this function with a valid arguments object,
    obtained either by calling argparse.ArgumentParser.parse_args() after
    adding this modules arguments via buildutils.add_arguments(), or by passing
    in an object returned from buildutils.build_defaults().

    Args:
      arguments: The argument object returned from ArgumentParser.parse_args().
    """

    super(BuildEnvironment, self).__init__(arguments)

    if type(arguments) is dict:
      args = arguments
    else:
      args = vars(arguments)

    self.cmake_path = args[_CMAKE_PATH]
    self.cmake_flags = args[_CMAKE_FLAGS]

  @staticmethod
  def build_defaults():
    """Helper function to set build defaults.

    Returns:
      A dict containing appropriate defaults for a build.
    """
    args = common.BuildEnvironment.build_defaults()

    args[_CMAKE_PATH] = (os.getenv(_CMAKE_PATH_ENV_VAR) or
                         distutils.spawn.find_executable('cmake'))
    args[_CMAKE_FLAGS] = os.getenv(_CMAKE_FLAGS_ENV_VAR)

    return args

  @staticmethod
  def add_arguments(parser):
    """Add module-specific command line arguments to an argparse parser.

    This will take an argument parser and add arguments appropriate for this
    module. It will also set appropriate default values.

    Args:
      parser: The argparse.ArgumentParser instance to use.
    """
    defaults = BuildEnvironment.build_defaults()

    common.BuildEnvironment.add_arguments(parser)

    parser.add_argument('-b', '--' + _CMAKE_PATH,
                        help='Path to CMake binary', dest=_CMAKE_PATH,
                        default=defaults[_CMAKE_PATH])
    parser.add_argument(
        '-F', '--' + _CMAKE_FLAGS, help='Flags to use to override CMake flags',
        dest=_CMAKE_FLAGS, default=defaults[_CMAKE_FLAGS])

  def _find_binary(self, binary, additional_paths=None):
    """Find a binary from the set of binaries managed by this class.

    This method enables the lookup of a binary path using the name of the
    binary to avoid replication of code which searches for binaries.

    This class allows the lookup of...
    * BuildEnvironment.CMAKE

    The _find_binary() method in derived classes may add more binaries.

    Args:
      binary: Name of the binary.
      additional_paths: Additional dictionary to search for binary paths.

    Returns:
      String containing the path of binary.

    Raises:
      ToolPathError: Binary is not at the specified path.
    """
    search_dict = {BuildEnvironment.CMAKE: [self.cmake_path]}
    if additional_paths:
      search_dict.update(additional_paths)
    return BuildEnvironment._check_binary(binary, search_dict[binary])

  def run_cmake(self, gen='Unix Makefiles'):
    """Run cmake based on the specified build environment.

    This will execute cmake using the configured environment, passing it the
    flags specified in the cmake_flags property.

    Args:
      gen: Optional argument to specify CMake project generator (defaults to
        Unix Makefiles)

    Raises:
      SubCommandError: CMake invocation failed or returned an error.
      ToolPathError: CMake not found in configured build environment or $PATH.
    """

    cmake_path = self._find_binary(BuildEnvironment.CMAKE)

    args = [cmake_path, '-G', gen]
    if self.cmake_flags:
      args += shlex.split(self.cmake_flags, posix=self._posix)
    args.append(self.project_directory)

    self.run_subprocess(args, cwd=self.project_directory)
