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
import os
import sys
import unittest
sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir))
import buildutil.common as common

class MockOsPathExists(object):
  """Mock of os.path.exists.

  Attributes:
    mock_path_exists: Value to return from  __call__().
  """

  def __init__(self, mock_path_exists):
    """Initialize this instance.

    Args:
      mock_path_exists: Value to return from __call__().
    """
    self.mock_path_exists = mock_path_exists

  def __call__(self, unused_path):
    """Mock of os.path.exists.

    Returns:
      Value of mock_path_exists set in the constructor.
    """
    return self.mock_path_exists

class MockFindExecutable(object):
  """Mock of common._find_executable

  Attribtues:
    mock_path: Path to return from __call__().
  """

  def __init__(self, mock_path):
    """Initialize the instance.

    Args:
      mock_path: Path to return from __call__().
    """
    self.mock_path = mock_path

  def __call__(self, unused_name):
    """Mock of common._find_executable().

    Returns:
      Returns the mock_path attribute.
    """
    return self.mock_path


class RunCommandMock(object):
  """Callable object which verifies command lines.

  Attributes:
    expected_args: Expected command line arguments.
    expected_cwd: Expected working directory.
    expected_shell: Expected value of the shell argument.
    stdout: Standard output string that is the result of a call to this object.
    stderr: Standard error string that is the result of a call to this object.
    test_case: unittest.TestCase used to verify arguments.
  """

  def __init__(self, test_case, args=None, cwd=None, stdout=None, stderr=None,
               shell=None):
    """Callable object which verifies command lines.

    Args:
      test_case: unittest.TestCase used to verify arguments.
      args: Expected command line arguments.
      cwd: Expected working directory.
      stdout: Standard output string that is the result of a call to this
        object.
      stderr: Standard error string that is the result of a call to this
        object.
      shell: Expected value of the shell argument.
    """
    self.expected_args = args
    self.expected_cwd = cwd
    self.expected_shell = shell
    self.stdout = stdout
    self.stderr = stderr
    self.test_case = test_case

  def returns(self, stdout, stderr=None):
    """Set the strings values returned by __call__().

    Args:
      stdout: Standard output string that is the result of a call to this
        object.
      stderr: Standard error string that is the result of a call to this
        object.
    """
    self.stdout = stdout
    self.stderr = stderr

  def expect(self, args, cwd=None, shell=None):
    """Set the expected arguments when this class is called.

    Args:
      args: Expected arguments when this class is called.
      cwd: Expected current working directory when this class is called or
         None to ignore this value.
      shell: Expected value of the shell argument.
    """
    self.expected_args = args
    self.expected_cwd = cwd
    if shell is not None:
      self.expected_shell = shell

  def __call__(self, argv, capture=False, cwd=os.getcwd(), shell=False,
               stdin=None):
    """Mock of common.BuildEnvironment.run_subprocess().

    Args:
      argv: Arguments to compare with the expected arguments for this class.
      capture: Whether to return a tuple (self.stdout, self.stderr)
      cwd: Optional path relative to the project directory to run process in
        for commands that do not allow specifying this, such as ant.
      shell: Compared against expected_shell.
      stdin: Unused.

    Returns:
      (self.stdout, self.stderr) if capture is True, None otherwise.
    """
    if self.expected_cwd:
      self.test_case.assertEqual(self.expected_cwd, cwd)
    if self.expected_shell:
      self.test_case.assertEqual(self.expected_shell, shell)
    if issubclass(list, argv.__class__):
      self.test_case.assertListEqual(self.expected_args, argv)
    else:
      self.test_case.assertEqual(str(self.expected_args), argv)
    if capture:
      return (self.stdout, self.stderr)


class RunCommandMockList(object):
  """List of RunCommandMock instances.

  Each call to this class pops the RunCommandMock instance from the top of
  the pending list and calls the object.

  Attributes:
    mock_list: List of RunCommandMock instances.  This list of instances is
      called in sequence on subsequent calls to RunCommandMockList.__call__().
  """

  def __init__(self, mock_list):
    """Initialize this instance.

    Args:
      mock_list: List of RunCommandMock instances.  This list of instances is
        called in sequence on subsequent calls to
        RunCommandMockList.__call__().
    """
    self.mock_list = mock_list

  def __call__(self, argv, capture=False, cwd=os.getcwd(), shell=False):
    """Mock of common.BuildEnvironment.run_subprocess().

    Args:
      argv: Arguments to compare with the expected arguments of the
        RunCommandMock instance at the head of mock_list.
      capture: Whether to return a tuple (self.stdout, self.stderr)
      cwd: Optional path relative to the project directory to run process in
        for commands that do not allow specifying this, such as ant.
      shell: Whether to run the command to run in a shell.

    Returns:
      (self.stdout, self.stderr) if capture is True, None otherwise.
    """
    return self.mock_list.pop(0)(argv, capture=capture, cwd=cwd, shell=shell)


class MakeVerifier(RunCommandMock):
  """Callable object which verifies make command lines.

  Attributes:
    expected: Expected arguments for make.
    test_case: unittest.TestCase used to verify make arguments.
  """
  def __init__(self, test_case, expected):
    """Initialize this instance.

    Args:
      test_case: unittest.TestCase used to verify make arguments.
      expected: Expected arguments for make.
    """
    RunCommandMock.__init__(self, test_case)
    d = common.BuildEnvironment.build_defaults()
    # If the implementation changes arguments, this mock needs to be updated as
    # well.
    self.expect([d[common._MAKE_PATH], '-j', d[common._CPU_COUNT]] + expected)


class CommonBuildUtilTest(unittest.TestCase):
  """Common base buildutil unit tests."""

  def setUp(self):
    self.git_clean_ran = False
    self.git_reset_ran = False
    self.os_path_exists = os.path.exists
    self.find_executable = common._find_executable
    common._find_executable = (
        lambda name, path=None: path if path else os.path.join('a', 'b', name))

  def tearDown(self):
    common._find_executable = self.find_executable
    os.path.exists = self.os_path_exists

  def test_build_defaults(self):
    d = common.BuildEnvironment.build_defaults()
    self.assertIn(common._PROJECT_DIR, d)
    self.assertIn(common._CPU_COUNT, d)
    self.assertIn(common._MAKE_PATH, d)
    self.assertIn(common._GIT_PATH, d)
    self.assertIn(common._MAKE_FLAGS, d)
    self.assertIn(common._GIT_CLEAN, d)
    self.assertIn(common._VERBOSE, d)
    self.assertIn(common._OUTPUT_DIR, d)
    self.assertIn(common._CLEAN, d)

  def test_init(self):
    d = common.BuildEnvironment.build_defaults()
    b = common.BuildEnvironment(d)
    self.assertEqual(b.project_directory, d[common._PROJECT_DIR])
    self.assertEqual(b.cpu_count, d[common._CPU_COUNT])
    self.assertEqual(b.make_path, d[common._MAKE_PATH])
    self.assertEqual(b.git_path, d[common._GIT_PATH])
    self.assertEqual(b.make_flags, d[common._MAKE_FLAGS])
    self.assertEqual(b.enable_git_clean, d[common._GIT_CLEAN])
    self.assertEqual(b.verbose, d[common._VERBOSE])
    self.assertEqual(b.output_directory, d[common._OUTPUT_DIR])
    self.assertEqual(b.clean, d[common._CLEAN])

  def test_add_arguments(self):
    p = argparse.ArgumentParser()
    common.BuildEnvironment.add_arguments(p)
    args = ['--' + common._PROJECT_DIR, 'a',
            '--' + common._CPU_COUNT, 'b',
            '--' + common._MAKE_PATH, 'c',
            '--' + common._GIT_PATH, 'd',
            '--' + common._MAKE_FLAGS, 'e',
            '--' + common._GIT_CLEAN,
            '--' + common._VERBOSE,
            '--' + common._OUTPUT_DIR, 'f',
            '--' + common._CLEAN]

    argobj = p.parse_args(args)
    d = vars(argobj)

    self.assertEqual('a', d[common._PROJECT_DIR])
    self.assertEqual('b', d[common._CPU_COUNT])
    self.assertEqual('c', d[common._MAKE_PATH])
    self.assertEqual('d', d[common._GIT_PATH])
    self.assertEqual('e', d[common._MAKE_FLAGS])
    self.assertTrue(d[common._GIT_CLEAN])
    self.assertTrue(d[common._VERBOSE])
    self.assertEqual('f', d[common._OUTPUT_DIR])
    self.assertTrue(d[common._CLEAN])

  def test_find_path_from_binary(self):
    test_data = [
        (os.path.join(os.path.sep, 'a', 'b', 'c'), 0,
         os.path.join(os.path.sep, 'a', 'b', 'c')),
        (os.path.join(os.path.sep, 'a', 'b', 'c'), 1,
         os.path.join(os.path.sep, 'a', 'b')),
        (os.path.join(os.path.sep, 'a', 'b', 'c'), 2,
         os.path.join(os.path.sep, 'a')),
        (os.path.join(os.path.sep, 'a', 'b', 'c'), 3, os.path.sep),
        (os.path.join(os.path.sep, 'a', 'b', 'c'), 4, None),
        (os.path.join(os.path.sep, 'a', 'b', 'c'), -1,
         os.path.join(os.path.sep, 'a', 'b', 'c')),
        (os.path.join(os.path.sep, 'a'), 0, os.path.join(os.path.sep, 'a')),
        (os.path.join(os.path.sep, 'a'), 1, os.path.sep),
        (os.path.join(os.path.sep, 'a'), 2, None)]

    for (path, levels, expect) in test_data:
      # reset in tearDown
      common._find_executable = MockFindExecutable(path)
      result = common.BuildEnvironment._find_path_from_binary('foo', levels)
      self.assertEqual(result, expect, '"%s" != "%s" (case: %s)' % (
          result, expect, str((path, levels, expect))))

  def test_find_binary_found(self):
    build_environment = common.BuildEnvironment(
        common.BuildEnvironment.build_defaults())
    common._find_executable = lambda name, path=None: None
    with self.assertRaises(common.ToolPathError):
      build_environment._find_binary(common.BuildEnvironment.MAKE)

  def test_find_binary_missing(self):
    build_environment = common.BuildEnvironment(
        common.BuildEnvironment.build_defaults())
    self.assertNotEquals(None, build_environment._find_binary(
        common.BuildEnvironment.GIT))

  def test_run_make(self):
    d = common.BuildEnvironment.build_defaults()
    b = common.BuildEnvironment(d)
    # Mock the call to run_subprocess.
    b.run_subprocess = MakeVerifier(self, ['-C', 'e', 'c', 'd'])
    b.make_flags = 'c d'
    b.project_directory = 'e'
    b.run_make()

  def test_run_make_clean(self):
    d = common.BuildEnvironment.build_defaults()
    b = common.BuildEnvironment(d)
    b.clean = True
    # Mock the call to run_subprocess.
    b.run_subprocess = MakeVerifier(self, ['-C', 'e', 'clean'])
    b.project_directory = 'e'
    b.run_make()

  def test_git_clean(self):
    d = common.BuildEnvironment.build_defaults()
    b = common.BuildEnvironment(d)
    # Mock the call to run_subprocess.
    b.run_subprocess = self.git_clean_verifier
    os.path.exists = MockOsPathExists(False)  # reset in tearDown

    # first, unless enabled, git_clean() should do nothing.
    b.git_clean()
    self.assertFalse(self.git_clean_ran)
    self.assertFalse(self.git_reset_ran)
    b.enable_git_clean = True

    # next, should do nothing if not in git dir
    b.git_clean()
    self.assertFalse(self.git_clean_ran)
    self.assertFalse(self.git_reset_ran)

    os.path.exists = MockOsPathExists(True)  # reset in tearDown

    # finally, should run
    b.project_directory = 'e'
    b.git_clean()
    self.assertTrue(self.git_clean_ran)
    self.assertTrue(self.git_reset_ran)

  def git_clean_verifier(self, args, cwd=None):
    """BuildEnvironment.run_subprocess mock for test_git_clean.

    Args:
      args: Argument list as normally passed to run_subprocess.
      cwd: Working directory arg as normally passed to run_subprocess.
    """
    d = common.BuildEnvironment.build_defaults()
    expected = None
    if 'clean' in args:
      self.git_clean_ran = True
      expected = [d[common._GIT_PATH], '-C', 'e', 'clean', '-d', '-f']
    else:
      if 'reset' in args:
        self.git_reset_ran = True
        expected = [d[common._GIT_PATH], '-C', 'e', 'reset', '--hard']
    self.assertIsNotNone(expected)
    self.assertListEqual(args, expected)
    if cwd:
      self.assertEqual(cwd, d[common._PROJECT_DIR])

  def test_get_project_directory(self):
    b = common.BuildEnvironment(common.BuildEnvironment.build_defaults())
    b.project_directory = os.path.join('examples', 'libfplutil_example')
    expected = os.path.join(os.getcwd(), b.project_directory)
    self.assertEquals(expected, b.get_project_directory())

  def test_get_project_directory_relative(self):
    b = common.BuildEnvironment(common.BuildEnvironment.build_defaults())
    directories = ('examples', 'libfplutil_example')
    b.project_directory = os.path.join(directories[0], directories[1])
    expected = os.path.join(os.getcwd(), directories[0])
    self.assertEquals(expected, b.get_project_directory(path='..'))

  # TBD, these are highly dependent high level functions that may need refactor
  # to unit-test well, as they are currently difficult to mock. At the moment
  # the examples serve as functional tests for these.
  def test_make_archive(self):
    pass

  def test_write_archive(self):
    pass


if __name__ == '__main__':
  unittest.main()
