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

import __builtin__
import argparse
import os
import platform
import re
import StringIO
import subprocess
import sys
import unittest
import uuid
sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir))
import buildutil.android as android
import buildutil.common as common
import buildutil.common_test as common_test
import buildutil.linux as linux


class FileMock(object):
  """Light mock of a file.

  Attributes:
    string: String returned by read().
  """

  def __init__(self, string):
    """Initialize the mock file.

    Args:
      string: String returned by read().
    """
    self.string = string

  def close(self):
    pass

  def read(self, nbytes=None):
    """Return nbytes characters of the "string" attribute.

    Args:
      nbytes: Number of bytes to read from the file or None returns the entire
        file.

    Returns:
      An nbytes slice of the "string" attribute.
    """
    if nbytes is None:
      nbytes = len(self.string)
    r = self.string[0:nbytes]
    self.string = self.string[nbytes:]
    return r

  def readlines(self):
    """Return a line at a time from the "string" attribute.

    Returns:
      A list of lines from the string attribute.
    """
    return self.string.splitlines()

  def __enter__(self):
    """Empty file open."""
    return self

  def __exit__(self, exc_type, exc_val, exc_tb):
    """Empty file close."""
    pass


class FileSetMockFactory(object):
  """Factory for FileMock instances which can be used to override open.

  Attributes:
    test_case: Test case used to verify contents are found for open files.
    filename_contents: Dictionary containing file contents string indexed by
      filenames.  A FileMock is returned by __call__ initialized with the
      contents of the specified filename.  If the filename isn't present, the
      contents indexed by the key None will be returned.
  """

  file_open = __builtin__.open

  def __init__(self, test_case, filename_contents):
    """Initialize the mock file factory.

    Args:
      test_case: Test case used to verify contents are found for open files.
      filename_contents: Dictionary containing file contents string indexed by
        filenames.  A FileMock is returned by __call__ initialized with the
        contents of the specified filename.  If the filename isn't present, the
        contents indexed by the key None will be returned.
    """
    self.test_case = test_case
    self.filename_contents = filename_contents

  def __call__(self, filename, *argv):
    """Mock file open().

    Args:
      filename: Name of the file used to lookup the contents string in
        "filename_contents" which initializes the returned FileMock.
      *argv: Additional arguments for file open, ignored by the mock
        implementation.

    Returns:
      FileMock instance that will return "contents" on MockFile.read().
    """
    # Special case which allows the exception handler to open files to
    # annotate tracebacks.
    if filename and filename.endswith('.py'):
      return self.__class__.file_open(filename, *argv)

    contents = self.filename_contents.get(
        filename, self.filename_contents.get(None))
    self.test_case.assertNotEquals(None, contents)
    return FileMock(contents)


class FileMockFactory(FileSetMockFactory):
  """Factory for FileMock which can be used to override open."""

  def __init__(self, test_case, contents):
    """Initialize the mock file factory.

    Args:
      test_case: Test case used to verify contents are found for open files.
      contents: String which contains the contents of the file returned by
        FileMock.read().
    """
    super(FileMockFactory, self).__init__(test_case, {None: contents})

  def __call__(self, unused_filename, *unused_argv):
    """Mock file open().

    Returns:
      FileMock instance that will return "contents" on MockFile.read().
    """
    return super(FileMockFactory, self).__call__(None)


class BuildAndroidLibrariesMock(object):

  def __init__(self, test):
    self.test = test
    self.subprojects = []
    self.output = None

  def __call__(self, subprojects, output=None):
    self.test.assertEqual(self.output, output)
    self.test.assertListEqual(sorted(self.subprojects), sorted(subprojects))

  def expect(self, subprojects, output='libs'):
    self.subprojects = subprojects
    self.output = output


class BuildAndroidAPKMock(object):

  def __init__(self, test):
    self.test = test
    self.path = None
    self.output = None
    self.fail_if_called = False

  def fail(self, fail=True):
    self.fail_if_called = fail

  def __call__(self, path='.', output=None):
    self.test.assertFalse(self.fail_if_called)
    self.test.assertEqual(self.output, output)
    self.test.assertEqual(self.path, path)

  def expect(self, path, output='apks'):
    self.path = path
    self.output = output


class FileNode(object):

  def __init__(self, name, parent=None):
    self.name = name
    self.parent = parent
    if parent:
      self.name = os.path.join(parent.name, name)

    self.files = []
    self.dirs = []

  def add_files(self, namelist):
    self.files += namelist

  def add_subdir(self, name):
    node = FileNode(name, self)
    self.dirs.append(node)
    return node


class OsWalkMock(object):

  def __init__(self, test):
    self.root = None
    self.test = test
    self.project = None

  def expect(self, path):
    self.project = path

  def set_root(self, root):
    self.root = root

  def walk(self, path, **unused_kwargs):
    self.test.assertEqual(self.project, path)
    # Perform a BFS traversal as a generator to mimic how topdown os.walk works.
    nodes = [self.root]
    while nodes:
      n = nodes.pop(0)
      rc = (n.name, [os.path.basename(m.name) for m in n.dirs], n.files)
      yield rc
      # Pick up changes to the dirlist, per how os.walk works in real life.
      _, dirs, _ = rc
      nodes += [d for d in n.dirs if os.path.basename(d.name) in dirs]


class SubprocessMockStdOut(object):

  def __init__(self, test, args=None, stdout=''):
    self.test = test
    self.stdout = StringIO.StringIO(stdout)
    self.expected_args = args

  def __call__(self, args=None, bufsize=None, stdout=None):
    self.test.assertEquals(self.expected_args, args)
    return self

  def kill(self):
    pass


class AndroidBuildUtilTest(unittest.TestCase):
  """Android-specific unit tests."""

  def setUp(self):
    # Save methods that are mocked out.
    self.os_walk = os.walk
    self.os_path_exists = os.path.exists
    self.os_path_getmtime = os.path.getmtime
    self.find_executable = common._find_executable
    self.file_open = __builtin__.open
    self.subprocess_popen = subprocess.Popen
    # Mock out find_executable so all binaries are found.
    common._find_executable = lambda name, path=None: (
        os.path.join(path, name) if path else os.path.join(
            'a', 'b', name))
    self.uuid4 = uuid.uuid4

  def tearDown(self):
    # Undo mocks.
    os.walk = self.os_walk
    os.path.exists = self.os_path_exists
    os.path.getmtime = self.os_path_getmtime
    common._find_executable = self.find_executable
    __builtin__.open = self.file_open
    subprocess.Popen = self.subprocess_popen
    uuid.uuid4 = self.uuid4

  def test_build_defaults(self):
    d = android.BuildEnvironment.build_defaults()
    # Verify that the android ones are set.
    self.assertIn(android._NDK_HOME, d)
    self.assertIn(android._SDK_HOME, d)
    self.assertIn(android._ANT_PATH, d)
    self.assertIn(android._ANT_FLAGS, d)
    self.assertIn(android._ANT_TARGET, d)
    self.assertIn(android._APK_KEYSTORE, d)
    self.assertIn(android._APK_PASSFILE, d)
    self.assertIn(android._APK_KEYALIAS, d)
    self.assertIn(android._APK_KEYPK8, d)
    self.assertIn(android._APK_KEYPEM, d)
    self.assertIn(android._SIGN_APK, d)
    # Verify that a mandatory superclass one gets set.
    self.assertIn(common._PROJECT_DIR, d)
    # Verify that the Linux ones do not get set.
    self.assertNotIn(linux._CMAKE_FLAGS, d)
    self.assertNotIn(linux._CMAKE_PATH, d)

  def test_init(self):
    d = android.BuildEnvironment.build_defaults()
    b = android.BuildEnvironment(d)
    # Verify that the Android ones are set.
    self.assertEqual(b.ndk_home, d[android._NDK_HOME])
    self.assertEqual(b.sdk_home, d[android._SDK_HOME])
    self.assertEqual(b.ant_path, d[android._ANT_PATH])
    self.assertEqual(b.ant_flags, d[android._ANT_FLAGS])
    self.assertEqual(b.ant_target, d[android._ANT_TARGET])
    self.assertEqual(b.apk_keystore, d[android._APK_KEYSTORE])
    self.assertEqual(b.apk_passfile, d[android._APK_PASSFILE])
    self.assertEqual(b.apk_keyalias, d[android._APK_KEYALIAS])
    self.assertEqual(b.sign_apk, d[android._SIGN_APK])
    # Verify that a mandatory superclass one gets set.
    self.assertEqual(b.project_directory, d[common._PROJECT_DIR])
    # Verify that the Linux ones do not get set.
    self.assertNotIn(linux._CMAKE_FLAGS, vars(b))
    self.assertNotIn(linux._CMAKE_PATH, vars(b))

  def test_add_arguments(self):
    p = argparse.ArgumentParser()
    android.BuildEnvironment.add_arguments(p)
    args = ['--' + android._SDK_HOME, 'a',
            '--' + android._NDK_HOME, 'b',
            '--' + android._ANT_PATH, 'c',
            '--' + android._ANT_FLAGS, 'd',
            '--' + android._ANT_TARGET, 'e',
            '--' + android._APK_KEYSTORE, 'f',
            '--' + android._APK_PASSFILE, 'g',
            '--' + android._APK_KEYALIAS, 'h',
            '--' + android._APK_KEYPK8, 'i',
            '--' + android._APK_KEYPEM, 'j',
            '--' + android._SIGN_APK]
    argobj = p.parse_args(args)
    d = vars(argobj)

    self.assertEqual('a', d[android._SDK_HOME])
    self.assertEqual('b', d[android._NDK_HOME])
    self.assertEqual('c', d[android._ANT_PATH])
    self.assertEqual('d', d[android._ANT_FLAGS])
    self.assertEqual('e', d[android._ANT_TARGET])
    self.assertEqual('f', d[android._APK_KEYSTORE])
    self.assertEqual('g', d[android._APK_PASSFILE])
    self.assertEqual('h', d[android._APK_KEYALIAS])
    self.assertEqual('i', d[android._APK_KEYPK8])
    self.assertEqual('j', d[android._APK_KEYPEM])
    self.assertTrue(d[android._SIGN_APK])

    self.assertEqual(os.getcwd(), d[common._PROJECT_DIR])
    # Verify that the Linux ones do not get set.
    self.assertNotIn(linux._CMAKE_FLAGS, d)
    self.assertNotIn(linux._CMAKE_PATH, d)

  def test_construct_android_manifest(self):
    m = android.AndroidManifest(None)
    self.assertEqual(m.min_sdk, 0)
    self.assertEqual(m.target_sdk, 0)
    self.assertIsNone(m.path)
    with self.assertRaises(common.ConfigurationError):
      android.AndroidManifest('/non existent/bogus_path')

  def test_manifest_parse_trivial(self):
    f = FileMock(
        '<manifest '
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <uses-sdk android:minSdkVersion="1"/>\n'
        '  <application>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '      <meta-data android:name="android.app.lib_name"\n'
        '                 android:value="test"/>\n'
        '      <intent-filter>\n'
        '       <action android:name="android.intent.action.MAIN"/>\n'
        '       <category android:name="android.intent.category.LAUNCHER"/>\n'
        '      </intent-filter>\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    m = android.AndroidManifest(None)
    m._parse(f)
    self.assertEqual(m.min_sdk, 1)
    self.assertEqual(m.target_sdk, m.min_sdk)

  def test_manifest_parse_native_activity_no_lib(self):
    f = FileMock(
        '<manifest '
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <uses-sdk android:minSdkVersion="1"/>\n'
        '  <application>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    m = android.AndroidManifest(None)
    with self.assertRaises(common.ConfigurationError):
      m._parse(f)

  def test_manifest_parse_with_target(self):
    f = FileMock(
        '<manifest '
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <uses-sdk android:minSdkVersion="1" '
        '            android:targetSdkVersion="2"/>\n'
        '  <application>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '      <meta-data android:name="android.app.lib_name"\n'
        '                 android:value="test"/>\n'
        '      <intent-filter>\n'
        '       <action android:name="android.intent.action.MAIN"/>\n'
        '       <category android:name="android.intent.category.LAUNCHER"/>\n'
        '      </intent-filter>\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    m = android.AndroidManifest(None)
    m._parse(f)
    self.assertEqual(m.min_sdk, 1)
    self.assertEqual(m.target_sdk, 2)

  def test_manifest_parse_with_bad_target(self):
    f = FileMock(
        '<manifest \n'
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <uses-sdk android:minSdkVersion="1" '
        '            android:targetSdkVersion="-2"/>\n'
        '  <application>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '      <meta-data android:name="android.app.lib_name"\n'
        '                 android:value="test"/>\n'
        '      <intent-filter>\n'
        '       <action android:name="android.intent.action.MAIN"/>\n'
        '       <category android:name="android.intent.category.LAUNCHER"/>\n'
        '      </intent-filter>\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    m = android.AndroidManifest(None)
    m._parse(f)
    self.assertEqual(m.min_sdk, 1)
    # this is an error but we want to catch in processing, not parsing
    self.assertEqual(m.target_sdk, -2)

  def test_manifest_parse_missing_min_version(self):
    f = FileMock(
        '<manifest '
        'xmlns:android="http://schemas.android.com/apk/res/android">\n'
        '<uses-sdk/>\n'
        '</manifest>')
    m = android.AndroidManifest(None)
    with self.assertRaises(common.ConfigurationError):
      m._parse(f)

  def test_manifest_parse_expecting_missing_sdk(self):
    f = FileMock(
        '<manifest '
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <application>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '      <meta-data android:name="android.app.lib_name"\n'
        '                 android:value="test"/>\n'
        '      <intent-filter>\n'
        '       <action android:name="android.intent.action.MAIN"/>\n'
        '       <category android:name="android.intent.category.LAUNCHER"/>\n'
        '      </intent-filter>\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    m = android.AndroidManifest(None, ignore_sdk_version_missing=True)
    m._parse(f)
    self.assertEqual(m.min_sdk, 0)
    self.assertEqual(m.target_sdk, 0)

  def test_manifest_parse_expecting_missing_min_ver(self):
    f = FileMock(
        '<manifest '
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <application>\n'
        '    <uses-sdk/>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '      <meta-data android:name="android.app.lib_name"\n'
        '                 android:value="test"/>\n'
        '      <intent-filter>\n'
        '       <action android:name="android.intent.action.MAIN"/>\n'
        '       <category android:name="android.intent.category.LAUNCHER"/>\n'
        '      </intent-filter>\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    m = android.AndroidManifest(None, ignore_sdk_version_missing=True)
    m._parse(f)
    self.assertEqual(m.min_sdk, 0)
    self.assertEqual(m.target_sdk, 0)

  def test_manifest_parse_missing_uses_sdk(self):
    f = FileMock(
        '<manifest '
        'xmlns:android="http://schemas.android.com/apk/res/android">\n'
        '</manifest>')
    m = android.AndroidManifest(None)
    with self.assertRaises(common.ConfigurationError):
      m._parse(f)

  def test_get_android_manifest_path(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    self.assertEquals(os.path.join(os.getcwd(), 'AndroidManifest.xml'),
                      build_environment.get_manifest_path())

  def test_manifest_parse(self):
    os.path.exists = lambda unused_filename: True
    __builtin__.open = FileMockFactory(
        self,
        '<manifest '
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <uses-sdk android:minSdkVersion="1"/>\n'
        '  <application>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '      <meta-data android:name="android.app.lib_name"\n'
        '                 android:value="test"/>\n'
        '      <intent-filter>\n'
        '       <action android:name="android.intent.action.MAIN"/>\n'
        '       <category android:name="android.intent.category.LAUNCHER"/>\n'
        '      </intent-filter>\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    manifest = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults()).parse_manifest()
    self.assertEqual(manifest.min_sdk, 1)
    self.assertEqual(manifest.target_sdk, manifest.min_sdk)

  def test_manifest_parse_missing_main_activity(self):
    os.path.exists = lambda unused_filename: True
    # This mock file has everything but the launcher category.
    __builtin__.open = FileMockFactory(
        self,
        '<manifest '
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <uses-sdk android:minSdkVersion="1"/>\n'
        '  <application>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '      <meta-data android:name="android.app.lib_name"\n'
        '                 android:value="test"/>\n'
        '    <intent-filter>\n'
        '      <action android:name="android.intent.action.MAIN"/>\n'
        '    </intent-filter>\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    defaults = android.BuildEnvironment.build_defaults()
    self.assertRaises(android.AndroidManifest.MissingActivityError,
                      android.BuildEnvironment(defaults).parse_manifest)

  def test_manifest_parse_error(self):
    f = FileMock('<manifest ')
    m = android.AndroidManifest(None)
    with self.assertRaises(common.ConfigurationError):
      m._parse(f)

  def test_construct_buildxml(self):
    b = android.BuildXml(None)
    self.assertIsNone(b.path)
    self.assertIsNone(b.project_name)
    with self.assertRaises(common.ConfigurationError):
      android.BuildXml('/non existent/bogus_path')

  def test_buildxml_parse_trivial(self):
    f = FileMock('<project name="foo"/>')
    b = android.BuildXml(None)
    b._parse(f)
    self.assertEqual(b.project_name, 'foo')
    self.assertIsNone(b.path)

  def test_buildxml_missing_name(self):
    f = FileMock('<project/>')
    b = android.BuildXml(None)
    with self.assertRaises(common.ConfigurationError):
      b._parse(f)

  def test_buildxml_missing_project(self):
    f = FileMock('<not-project name="foo"/>')
    b = android.BuildXml(None)
    with self.assertRaises(common.ConfigurationError):
      b._parse(f)

  def test_parse_adb_devices_list(self):
    device = android.AdbDevice(
        '06d8bd43               device usb:2-3.3 product:razor model:Nexus_7 '
        'device:flo')
    self.assertEquals(device.serial, '06d8bd43')
    self.assertEquals(device.type, 'device')
    self.assertEquals(device.usb, '2-3.3')
    self.assertEquals(device.product, 'razor')
    self.assertEquals(device.model, 'Nexus_7')
    self.assertEquals(device.device, 'flo')

  def test_adb_device_to_string(self):
    device = android.AdbDevice()
    device.serial = '06d8bd43'
    device.type = 'device'
    device.usb = '2-3.3'
    device.product = 'razor'
    device.model = 'Nexus_7'
    device.device = 'flo'
    self.assertEquals('device:flo model:Nexus_7 product:razor serial:06d8bd43 '
                      'type:device usb:2-3.3', str(device))

  def test_adb_device_from_device_line_to_string(self):
    device = android.AdbDevice(
        '06d8bd43 device usb:2-3.3 product:razor model:Nexus_7 device:flo')
    self.assertEquals('device:flo model:Nexus_7 product:razor serial:06d8bd43 '
                      'type:device usb:2-3.3',
                      str(device))
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    self.assertEquals('06d8bd43',
                      build_environment.get_adb_device_name(device))

    build_environment.verbose = True
    self.assertEquals('device:flo model:Nexus_7 product:razor serial:06d8bd43 '
                      'type:device usb:2-3.3',
                      build_environment.get_adb_device_name(device))

  def test_build_libraries(self):
    d = android.BuildEnvironment.build_defaults()
    b = android.BuildEnvironment(d)
    m = common_test.RunCommandMock(self)
    b.run_subprocess = m
    ndk_build = os.path.join(b.ndk_home, 'ndk-build')
    l = 'libfoo'
    lpath = os.path.abspath(os.path.join(b.project_directory, l))
    expect = [ndk_build, '-j' + str(b.cpu_count), '-C', lpath]
    m.expect(expect)
    b.build_android_libraries([l])
    b.verbose = True
    expect.append('V=1')
    m.expect(expect)
    b.build_android_libraries([l])
    expect.append('NDK_OUT=%s' % lpath)
    m.expect(expect)
    b.build_android_libraries([l], output=l)
    b.make_flags = '-DFOO -DBAR -DBAZ'
    flaglist = ['-DFOO', '-DBAR', '-DBAZ']
    expect += flaglist
    m.expect(expect)
    b.build_android_libraries([l], output=l)
    common._find_executable = self.find_executable
    b.ndk_home = '/dev/null'
    with self.assertRaises(common.ToolPathError):
      b.build_android_libraries([l], output=l)

  def test_clean_libraries(self):
    d = android.BuildEnvironment.build_defaults()
    b = android.BuildEnvironment(d)
    b.clean = True
    m = common_test.RunCommandMock(self)
    b.run_subprocess = m
    ndk_build = os.path.join(b.ndk_home, 'ndk-build')
    l = 'libfoo'
    lpath = os.path.abspath(os.path.join(b.project_directory, l))
    expect = [ndk_build, '-j' + str(platform.mac_ver()[0] and 1 or
                                    b.cpu_count),
              '-C', lpath, 'clean']
    m.expect(expect)
    b.build_android_libraries([l])
    b.verbose = True
    expect.append('V=1')
    m.expect(expect)
    b.build_android_libraries([l])
    expect.append('NDK_OUT=%s' % lpath)
    m.expect(expect)
    b.build_android_libraries([l], output=l)
    b.make_flags = '-DFOO -DBAR -DBAZ'
    flaglist = ['-DFOO', '-DBAR', '-DBAZ']
    expect += flaglist
    m.expect(expect)
    b.build_android_libraries([l], output=l)
    common._find_executable = self.find_executable
    b.ndk_home = '/dev/null'
    with self.assertRaises(common.ToolPathError):
      b.build_android_libraries([l], output=l)

  def test_find_android_sdk(self):
    d = android.BuildEnvironment.build_defaults()
    b = android.BuildEnvironment(d)
    m = common_test.RunCommandMock(self)
    b.run_subprocess = m
    expect = ['android', 'list', 'target', '--compact']
    m.expect(expect)
    m.returns('android-3\n'
              'android-5\n'
              'meaningless\n'
              'android-10\n'
              'android-L\n')
    got = b._find_best_android_sdk('android', 1, 5)
    self.assertEqual(got, 'android-5')
    got = b._find_best_android_sdk('android', 5, 15)
    self.assertEqual(got, 'android-10')
    got = b._find_best_android_sdk('android', 1, 2)
    self.assertEqual(got, 'android-10')
    with self.assertRaises(common.ConfigurationError):
      b._find_best_android_sdk('android', 11, 20)
    m.returns('android-10\n'
              'android-15\n'
              'android-7\n')
    got = b._find_best_android_sdk('android', 5, 15)
    self.assertEqual(got, 'android-15')

  def _find_projects_test_setup(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    walk_mock = OsWalkMock(self)
    os.walk = walk_mock.walk
    return (build_environment, walk_mock)

  def test_find_projects_trivial(self):
    (b, walk_mock) = self._find_projects_test_setup()
    project = b.project_directory
    tree = FileNode(project)
    jni = tree.add_subdir('jni')
    jni.add_files(['Android.mk', 'Application.mk'])
    walk_mock.expect(project)
    walk_mock.set_root(tree)
    b.find_projects()

  def test_find_projects_exclude(self):
    (b, walk_mock) = self._find_projects_test_setup()
    project = b.project_directory
    tree = FileNode(project)
    jni = tree.add_subdir('jni')
    jni.add_files(['Android.mk', 'Application.mk'])
    fooz = tree.add_subdir('fooz')
    fooz.add_files(['Android.mk', 'Application.mk'])
    walk_mock.expect(project)
    walk_mock.set_root(tree)
    b.find_projects(exclude_dirs=['fooz'])

  def test_find_projects_exclude_defaults(self):
    (b, walk_mock) = self._find_projects_test_setup()
    project = b.project_directory
    tree = FileNode(project)
    jni = tree.add_subdir('jni')
    jni.add_files(['Android.mk', 'Application.mk'])
    for name in ['apks', 'libs', 'bin', 'obj', 'res']:
      n = tree.add_subdir(name)
      n.add_files(['Android.mk', 'Application.mk'])
    walk_mock.expect(project)
    walk_mock.set_root(tree)
    b.find_projects()

  def test_find_projects_even_more_trivial(self):
    (b, walk_mock) = self._find_projects_test_setup()
    project = b.project_directory
    tree = FileNode(project)
    tree.add_files(['Android.mk'])  # test handling top level Android.mk
    walk_mock.expect(project)
    walk_mock.set_root(tree)
    b.find_projects()

  def _create_update_build_xml_setup(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    build_environment.sdk_home = ''

    build_environment._find_best_android_sdk = (
        lambda unused_android, unused_min_sdk, unused_target_sdk: 10)

    project_directory = build_environment.get_project_directory()
    buildxml_filename = os.path.join(project_directory, 'build.xml')

    manifest = android.AndroidManifest(None)
    manifest.path = os.path.join(project_directory, 'AndroidManifest.xml')
    manifest._parse(FileMock(
        '<manifest '
        ' xmlns:android="http://schemas.android.com/apk/res/android"\n'
        ' package="com.google.fpl.libfplutil_test">\n'
        ' <uses-sdk android:minSdkVersion="1"/>\n'
        ' <application>\n'
        '  <activity android:name="android.app.NativeActivity">\n'
        '    <intent-filter>\n'
        '      <action android:name="android.intent.action.MAIN"/>\n'
        '      <category android:name="android.intent.category.LAUNCHER"/>\n'
        '    </intent-filter>\n'
        '    <meta-data android:name="android.app.lib_name"\n'
        '               android:value="test"/>\n'
        '   </activity>\n'
        ' </application>\n'
        '</manifest>'))

    __builtin__.open = FileMockFactory(
        self, ('<?xml version="1.0" encoding="UTF-8"?>\n'
               '<project name="libfplutil_test" default="help">\n'
               '</project>\n'))

    return build_environment, manifest, buildxml_filename

  def test_create_update_build_xml_update(self):
    build_environment, manifest, buildxml_filename = (
        self._create_update_build_xml_setup())

    os.path.exists = lambda unused_filename: True
    os.path.getmtime = lambda filename: (
        {manifest.path: 10, buildxml_filename: 5}[filename])

    run_command_mock = common_test.RunCommandMock(self)
    run_command_mock.expect([os.path.join('sdk_path', 'tools', 'android'),
                             'update', 'project', '--path',
                             build_environment.get_project_directory(),
                             '--target', 10, '--name', 'libfplutil_test'])
    build_environment.run_subprocess = run_command_mock
    build_environment.ndk_home = 'ndk_path'
    build_environment.sdk_home = 'sdk_path'
    buildxml = build_environment.create_update_build_xml(manifest)
    self.assertEqual('libfplutil_test', buildxml.project_name)

  def test_create_update_build_xml_create(self):
    build_environment, manifest, buildxml_filename = (
        self._create_update_build_xml_setup())

    missing_files = set(buildxml_filename)
    os.path.exists = lambda f: (
        True if f not in missing_files else missing_files.remove(f) or False)
    os.path.getmtime = lambda filename: (
        {manifest.path: 10, buildxml_filename: 5}[filename])

    run_command_mock = common_test.RunCommandMock(self)
    run_command_mock.expect([os.path.join('sdk_path', 'tools', 'android'),
                             'update', 'project', '--path',
                             build_environment.get_project_directory(),
                             '--target', 10, '--name', 'libfplutil_test'])
    build_environment.run_subprocess = run_command_mock
    build_environment.ndk_home = 'ndk_path'
    build_environment.sdk_home = 'sdk_path'
    buildxml = build_environment.create_update_build_xml(manifest)
    self.assertEqual('libfplutil_test', buildxml.project_name)

  def test_create_update_build_xml_up_to_date(self):
    build_environment, manifest, buildxml_filename = (
        self._create_update_build_xml_setup())

    os.path.exists = lambda unused_filename: True
    os.path.getmtime = lambda filename: (
        {manifest.path: 5, buildxml_filename: 5}[filename])
    build_environment.ndk_home = 'ndk_path'
    build_environment.sdk_home = 'sdk_path'
    buildxml = build_environment.create_update_build_xml(manifest)
    self.assertEqual('libfplutil_test', buildxml.project_name)

  def test_get_apk_filenames(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    signed_apk, unsigned_apk = build_environment.get_apk_filenames(
        'libfplutil_example')
    self.assertEquals(os.path.join(os.getcwd(), 'bin',
                                   'libfplutil_example-release-unsigned.apk'),
                      unsigned_apk)
    self.assertEquals(os.path.join(os.getcwd(), 'bin',
                                   'libfplutil_example.apk'),
                      signed_apk)

  def test_get_apk_filenames_debug(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    build_environment.ant_target = 'debug'
    signed_apk, unsigned_apk = build_environment.get_apk_filenames(
        'libfplutil_example')
    expected_apk = os.path.join(os.getcwd(), 'bin',
                                'libfplutil_example-debug.apk')
    self.assertEquals(expected_apk, unsigned_apk)
    self.assertEquals(expected_apk, signed_apk)

  def test_build_android_apk_unsigned(self):
    build_environment, manifest, _ = (
        self._create_update_build_xml_setup())
    os.path.exists = lambda unused_filename: True
    os.path.getmtime = lambda unused_filename: 1
    build_environment.ant_path = 'ant'
    build_environment.ant_flags = 'a b "c d"'
    run_command_mock = common_test.RunCommandMock(self)
    run_command_mock.expect(['ant', 'release', 'a', 'b', 'c d'], None)
    build_environment.run_subprocess = run_command_mock
    build_environment.ndk_home = 'ndk_path'
    build_environment.sdk_home = 'sdk_path'
    build_environment.build_android_apk(manifest=manifest)

  def test_clean_android_apk(self):
    build_environment, manifest, _ = (
        self._create_update_build_xml_setup())
    os.path.exists = lambda unused_filename: True
    os.path.getmtime = lambda unused_filename: 1
    build_environment.clean = True
    build_environment.ant_path = 'ant'
    run_command_mock = common_test.RunCommandMock(self)
    run_command_mock.expect(['ant', 'clean', '-quiet'], None)
    build_environment.run_subprocess = run_command_mock
    build_environment.ndk_home = 'ndk_path'
    build_environment.sdk_home = 'sdk_path'
    build_environment.build_android_apk(manifest=manifest)

  def _build_all_test_setup(self):
    b, walk_mock = self._find_projects_test_setup()
    apk_mock = BuildAndroidAPKMock(self)
    lib_mock = BuildAndroidLibrariesMock(self)
    b.build_android_libraries = lib_mock
    b.build_android_apk = apk_mock
    return (b, apk_mock, lib_mock, walk_mock)

  def test_build_all_even_more_trivial(self):
    (b, apk_mock, lib_mock, walk_mock) = self._build_all_test_setup()
    project = b.project_directory
    tree = FileNode(project)
    tree.add_files(['Android.mk'])  # test handling top level Android.mk
    walk_mock.expect(project)
    walk_mock.set_root(tree)
    lib_mock.expect([tree.name])
    apk_mock.fail()  # should not be called
    b.build_all()

  def test_build_all_app(self):
    (b, apk_mock, lib_mock, walk_mock) = self._build_all_test_setup()
    project = b.project_directory
    tree = FileNode(project)
    tree.add_files(['AndroidManifest.xml'])
    jni = tree.add_subdir('jni')
    jni.add_files(['Android.mk', 'Application.mk'])
    walk_mock.expect(project)
    walk_mock.set_root(tree)
    lib_mock.expect([tree.name])
    apk_mock.expect(tree.name)
    b.build_all()

  def test_build_all_both(self):
    (b, apk_mock, lib_mock, walk_mock) = self._build_all_test_setup()
    project = b.project_directory
    tree = FileNode(project)
    app = tree.add_subdir('app')
    app.add_files(['AndroidManifest.xml'])
    jni = app.add_subdir('jni')
    jni.add_files(['Android.mk', 'Application.mk'])
    src = tree.add_subdir('src')
    jni = src.add_subdir('jni')
    jni.add_files(['Android.mk', 'Application.mk'])
    walk_mock.expect(project)
    walk_mock.set_root(tree)
    lib_mock.expect([src.name, app.name])
    apk_mock.expect(app.name)
    b.build_all()

  def test_check_adb_devices_no_devices(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    build_environment.sdk_home = 'sdk_home'
    os.path.exists = lambda unused_filename: True
    build_environment.run_subprocess = common_test.RunCommandMock(
        self, args='%s devices -l' % (
            build_environment._find_binary(android.BuildEnvironment.ADB)),
        stdout='List of devices attached\n\n', shell=True)
    with self.assertRaises(common.AdbError):
      build_environment.check_adb_devices()

  def test_check_adb_devices_one_device(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    build_environment.sdk_home = 'sdk_home'
    os.path.exists = lambda unused_filename: True
    build_environment.run_subprocess = common_test.RunCommandMock(
        self, args='%s devices -l' % (
            build_environment._find_binary(android.BuildEnvironment.ADB)),
        stdout=('List of devices attached\n'
                '06d8bd43               device usb:2-3.3 product:razor '
                'model:Nexus_7 device:flo\n'), shell=True)
    device = build_environment.check_adb_devices()
    self.assertEquals('06d8bd43', device.serial)

  def test_check_adb_devices_multiple_devices(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    build_environment.sdk_home = 'sdk_home'
    os.path.exists = lambda unused_filename: True
    build_environment.run_subprocess = common_test.RunCommandMock(
        self, args='%s devices -l' % (
            build_environment._find_binary(android.BuildEnvironment.ADB)),
        stdout=('Stuff to ignore\n'
                'More stuff to ignore\n'
                'List of devices attached\n'
                '06d8bd43               device usb:2-3.3 product:razor '
                'model:Nexus_7 device:flo\n'
                '98aaffe123             device usb:2-3.3 product:fishy '
                'model:Nexus_55 device:jelly\n'), shell=True)
    with self.assertRaises(common.AdbError):
      build_environment.check_adb_devices()

  def test_check_adb_devices_specified_device_found(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    build_environment.sdk_home = 'sdk_home'
    os.path.exists = lambda unused_filename: True
    build_environment.run_subprocess = common_test.RunCommandMock(
        self, args='%s devices -l' % (
            build_environment._find_binary(android.BuildEnvironment.ADB)),
        stdout=('List of devices attached\n'
                '06d8bd43               device usb:2-3.3 product:razor '
                'model:Nexus_7 device:flo\n'
                '98aaffe123             device usb:2-3.3 product:fishy '
                'model:Nexus_55 device:jelly\n'), shell=True)
    build_environment.check_adb_devices('98aaffe123')

  def test_check_adb_devices_specified_device_not_found(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    build_environment.sdk_home = 'sdk_home'
    os.path.exists = lambda unused_filename: True
    build_environment.run_subprocess = common_test.RunCommandMock(
        self, args='%s devices -l' % (
            build_environment._find_binary(android.BuildEnvironment.ADB)),
        stdout=('List of devices attached\n'
                '06d8bd43               device usb:2-3.3 product:razor '
                'model:Nexus_7 device:flo\n'
                '98aaffe123             device usb:2-3.3 product:fishy '
                'model:Nexus_55 device:jelly\n'), shell=True)
    with self.assertRaises(common.AdbError):
      build_environment.check_adb_devices('0000')

  def test_get_adb_device_argument(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    self.assertEquals('-s 06d8bd43',
                      build_environment.get_adb_device_argument('06d8bd43'))

  def test_get_adb_device_argument_no_device(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    self.assertEquals('', build_environment.get_adb_device_argument())

  def test_list_installed_packages(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    build_environment.sdk_home = 'sdk_home'
    os.path.exists = lambda unused_filename: True
    adb_path = build_environment._find_binary(android.BuildEnvironment.ADB)
    build_environment.run_subprocess = common_test.RunCommandMock(
        self, args='%s -s 06d8bd43 shell pm list packages' % adb_path,
        stdout=('package:com.google.earth\n'
                'junk\n'
                'package:com.google.android.gsf\n'
                'package:com.android.keyguard\n'
                'nothing useful\n'))
    expected = ['com.google.earth', 'com.google.android.gsf',
                'com.android.keyguard']
    self.assertListEqual(expected, build_environment.list_installed_packages(
        '06d8bd43'))

  def test_install_android_apk(self):
    class MockBuildEnvironment(android.BuildEnvironment):

      def __init__(self, defaults):
        android.BuildEnvironment.__init__(self, defaults)

      def parse_manifest(self, path=None):
        manifest = android.AndroidManifest(None)
        manifest._parse(FileMock(
            '<manifest '
            ' xmlns:android="http://schemas.android.com/apk/res/android"\n'
            ' package="com.google.fpl.libfplutil_test">\n'
            ' <uses-sdk android:minSdkVersion="1"/>\n'
            ' <application>\n'
            '  <activity android:name="android.app.NativeActivity">\n'
            '   <meta-data android:name="android.app.lib_name"\n'
            '              android:value="test"/>\n'
            '   <intent-filter>\n'
            '    <action android:name="android.intent.action.MAIN"/>\n'
            '    <category android:name="android.intent.category.LAUNCHER"/>\n'
            '   </intent-filter>\n'
            '  </activity>\n'
            ' </application>\n'
            '</manifest>'))
        return manifest

      def create_update_build_xml(self, manifest, path=None):
        buildxml = android.BuildXml(None)
        buildxml._parse(FileMock(
            '<?xml version="1.0" encoding="UTF-8"?>\n'
            '<project name="libfplutil_test" default="help">\n'
            '</project>\n'))
        return buildxml

      def list_installed_packages(self, adb_device=None, check_devices=True):
        return ['com.google.earth',
                'com.google.android.gsf',
                'com.google.fpl.libfplutil_test',
                'com.android.keyguard']

    build_environment = MockBuildEnvironment(
        android.BuildEnvironment.build_defaults())
    build_environment.sdk_home = ''

    os.path.exists = lambda unused_filename: True
    build_environment.sdk_home = 'sdk_home'
    adb_path = build_environment._find_binary(android.BuildEnvironment.ADB)

    build_environment.run_subprocess = common_test.RunCommandMockList(
        [common_test.RunCommandMock(
            self, args='%s devices -l' % adb_path,
            stdout=('List of devices attached\n'
                    '06d8bd43               device usb:2-3.3 product:razor '
                    'model:Nexus_7 device:flo\n')),
         common_test.RunCommandMock(
             self, args=('%s -s 06d8bd43 uninstall '
                         'com.google.fpl.libfplutil_test' % adb_path)),
         common_test.RunCommandMock(
             self, args=(
                 '%s -s 06d8bd43 install %s' % (adb_path, os.path.join(
                     build_environment.get_project_directory(), 'bin',
                     'libfplutil_test.apk'))))])

    build_environment.install_android_apk(adb_device='06d8bd43')

  def test_run_android_apk(self):
    os.path.exists = lambda unused_filename: True
    __builtin__.open = FileMockFactory(
        self,
        '<manifest '
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <uses-sdk android:minSdkVersion="1"/>\n'
        '  <application>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '      <meta-data android:name="android.app.lib_name"\n'
        '                 android:value="test"/>\n'
        '      <intent-filter>\n'
        '       <action android:name="android.intent.action.MAIN"/>\n'
        '       <category android:name="android.intent.category.LAUNCHER"/>\n'
        '      </intent-filter>\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())

    expected_logoutput = (
        'Random log output\n'
        'Some other log output\n'
        'Another application log output.\n'
        'Displayed com.google.fpl.libfplutil_test/'
        'android.app.NativeActivity\n'
        )
    logoutput = expected_logoutput + 'Line noise\n'

    # Configure the set of expected commands executed by run_android_apk.
    os.path.exists = lambda unused_filename: True
    build_environment.sdk_home = 'sdk_home'
    package = 'com.google.fpl.libfplutil_test'
    activity = 'android.app.NativeActivity'
    adb_path = build_environment._find_binary(android.BuildEnvironment.ADB)
    run_command_mock = common_test.RunCommandMockList(
        [common_test.RunCommandMock(
            self, args='%s devices -l' % adb_path,
            stdout=('List of devices attached\n'
                    '123456\tdevice\tusb:2-3.4\tproduct:razor\tmodel:Nexus_7\t'
                    'device:flo\n')),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 shell am force-stop %s' %
                         (adb_path, package))),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 logcat -c' %
                         (adb_path))),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 shell am start -n %s/%s' %
                         (adb_path, package, activity)),
             stdout=(''))])
    build_environment.run_subprocess = run_command_mock

    subprocess.Popen = SubprocessMockStdOut(
        self, args=[adb_path, '-s', '123456', 'logcat'],
        stdout=logoutput)
    self.assertEquals(expected_logoutput,
                      build_environment.run_android_apk(adb_device='123456'))

  def test_adb_device_with_poorly_formatted_device_line(self):
    adb_device = android.AdbDevice('serial')
    self.assertEquals(adb_device.serial, '')

  def test_adb_device_with_short_device_line(self):
    adb_device = android.AdbDevice('serial type')
    self.assertEquals(adb_device.serial, 'serial')
    self.assertEquals(adb_device.type, 'type')

  def test_run_android_apk_custom_endpoint(self):
    os.path.exists = lambda unused_filename: True
    __builtin__.open = FileMockFactory(
        self,
        '<manifest '
        '  xmlns:android="http://schemas.android.com/apk/res/android"\n'
        '  package="com.google.fpl.libfplutil_test">\n'
        '  <uses-sdk android:minSdkVersion="1"/>\n'
        '  <application>\n'
        '    <activity android:name="android.app.NativeActivity">\n'
        '      <meta-data android:name="android.app.lib_name"\n'
        '                 android:value="test"/>\n'
        '      <intent-filter>\n'
        '       <action android:name="android.intent.action.MAIN"/>\n'
        '       <category android:name="android.intent.category.LAUNCHER"/>\n'
        '      </intent-filter>\n'
        '    </activity>\n'
        '  </application>\n'
        '</manifest>')
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())

    end_str = 'This is the end condition!'
    end_cond = re.compile(end_str)

    expected_logoutput = (
        'Random log output\n'
        'Some other log output\n'
        'Another application log output.\n' + end_str + '\n')

    logoutput = expected_logoutput + '\nLine noise\n'

    # Configure the set of expected commands executed by run_android_apk.
    os.path.exists = lambda unused_filename: True
    build_environment.sdk_home = 'sdk_home'
    adb_path = build_environment._find_binary(android.BuildEnvironment.ADB)
    package = 'com.google.fpl.libfplutil_test'
    activity = 'android.app.NativeActivity'
    run_command_mock = common_test.RunCommandMockList(
        [common_test.RunCommandMock(
            self, args='%s devices -l' % adb_path,
            stdout=('List of devices attached\n'
                    '123456\tdevice\tusb:2-3.4\tproduct:razor\tmodel:Nexus_7\t'
                    'device:flo\n')),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 shell am force-stop %s' %
                         (adb_path, package))),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 logcat -c' %
                         (adb_path))),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 shell am start -n %s/%s' %
                         (adb_path, package, activity)),
             stdout=(''))])
    build_environment.run_subprocess = run_command_mock
    subprocess.Popen = SubprocessMockStdOut(
        self, args=[adb_path, '-s', '123456', 'logcat'],
        stdout=logoutput)
    self.assertEquals(expected_logoutput,
                      build_environment.run_android_apk(adb_device='123456',
                                                        end_match=end_cond))
  def test_take_screencap(self):
    uuid.uuid4 = lambda: 'test_uuid.png'
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    adb_path = build_environment._find_binary(android.BuildEnvironment.ADB)
    test_dest = '/fake/directory/structure.png'
    run_command_mock = common_test.RunCommandMockList(
        [common_test.RunCommandMock(
            self, args='%s devices -l' % adb_path,
            stdout=('List of devices attached\n'
                    '123456\tdevice\tusb:2-3.4\tproduct:razor\tmodel:Nexus_7\t'
                    'device:flo\n')),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 shell screencap -p /sdcard/%s' %
                         (adb_path, uuid.uuid4()))),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 pull /sdcard/%s %s' %
                         (adb_path, uuid.uuid4(), test_dest))),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 shell rm /sdcard/%s' %
                         (adb_path, uuid.uuid4()))),
         ])
    build_environment.run_subprocess = run_command_mock
    build_environment.take_screencap(test_dest, adb_device='123456')

  def test_get_device_dpi(self):
    build_environment = android.BuildEnvironment(
        android.BuildEnvironment.build_defaults())
    adb_path = build_environment._find_binary(android.BuildEnvironment.ADB)
    run_command_mock = common_test.RunCommandMockList(
        [common_test.RunCommandMock(
            self, args='%s devices -l' % adb_path,
            stdout=('List of devices attached\n'
                    '123456\tdevice\tusb:2-3.4\tproduct:razor\tmodel:Nexus_7\t'
                    'device:flo\n')),
         common_test.RunCommandMock(
             self, args=('%s -s 123456 shell getprop ro.sf.lcd_density' %
                         adb_path),
             stdout='240'),
         ])
    build_environment.run_subprocess = run_command_mock
    dpi = build_environment.get_device_dpi(adb_device='123456')
    self.assertEqual(dpi, 240)


if __name__ == '__main__':
  unittest.main()

