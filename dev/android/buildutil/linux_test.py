#!/usr/bin/python
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

import argparse
import distutils.spawn
import os
import sys
import unittest
sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir))
import buildutil.android as android
import buildutil.common as common
import buildutil.common_test as common_test
import buildutil.linux as linux


class CMakeMock(common_test.RunCommandMock):
  """Verifies cmake command lines."""

  def __init__(self, test_case, expected, cwd):
    """Callable object which verifies cmake command lines.

    Args:
      test_case: unitetst.TestCase used to verify arguments.
      expected: Expected arguments for cmake.
      cwd: Expected working directory.
    """
    common_test.RunCommandMock.__init__(self, test_case)
    build_environment_defaults = linux.BuildEnvironment.build_defaults()
    self.expect([build_environment_defaults[linux._CMAKE_PATH]] + expected,
                cwd)


class LinuxBuildUtilTest(unittest.TestCase):
  """Linux-specific unit tests."""

  def setUp(self):
    self.find_executable = common._find_executable
    # Mock out find_executable so all binaries are found.
    common._find_executable = (
      lambda name, path=None: path if path else os.path.join('a', 'b', name))

  def tearDown(self):
    common._find_executable = self.find_executable

  def test_build_defaults(self):
    d = linux.BuildEnvironment.build_defaults()
    # Verify that the linux ones are set.
    self.assertIn(linux._CMAKE_FLAGS, d)
    self.assertIn(linux._CMAKE_PATH, d)
    # Verify that a mandatory superclass one gets set.
    self.assertIn(common._PROJECT_DIR, d)
    # Verify that a required Android one does not get set.
    self.assertNotIn(android._NDK_HOME, d)

  def test_init(self):
    d = linux.BuildEnvironment.build_defaults()
    b = linux.BuildEnvironment(d)
    # Verify that the linux ones are set.
    self.assertEqual(b.cmake_flags, d[linux._CMAKE_FLAGS])
    self.assertEqual(b.cmake_path, d[linux._CMAKE_PATH])
    # Verify that a mandatory superclass one gets set.
    self.assertEqual(b.project_directory, d[common._PROJECT_DIR])
    # Verify that a required Android one does not get set.
    self.assertNotIn(android._NDK_HOME, vars(b))

  def test_add_arguments(self):
    p = argparse.ArgumentParser()
    linux.BuildEnvironment.add_arguments(p)
    args = ['--' + linux._CMAKE_FLAGS, 'a', '--' + linux._CMAKE_PATH, 'b']
    argobj = p.parse_args(args)
    d = vars(argobj)
    self.assertEqual('a', d[linux._CMAKE_FLAGS])
    self.assertEqual('b', d[linux._CMAKE_PATH])
    self.assertEqual(os.getcwd(), d[common._PROJECT_DIR])
    self.assertNotIn(android._NDK_HOME, d)

  def test_run_cmake(self):
    d = linux.BuildEnvironment.build_defaults()
    b = linux.BuildEnvironment(d)
    # Mock the call to run_subprocess.
    b.run_subprocess = CMakeMock(self, ['-G', 'b', 'c', 'd', 'e'], 'e')
    b.cmake_flags = 'c d'
    b.project_directory = 'e'
    b.run_cmake(gen='b')

if __name__ == '__main__':
  unittest.main()
