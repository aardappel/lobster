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

"""Simple Android build script that builds everything recursively.

This script will build all Android projects and libraries under the current
directory with the build settings specified on the command line, or the defaults
for those not specified.

Run 'build_all_android.py --help' for options.
"""

import argparse
import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir))
import buildutil.android
import buildutil.common

_SEARCH_PATH = 'search_path'
_APK_OUTPUT_DIR = 'apk_output_dir'
_LIB_OUTPUT_DIR = 'lib_output_dir'
_EXCLUDE_DIRS = 'exclude_dirs'
_APK_INSTALL = 'apk_install'
_APK_RUN = 'apk_run'
_ADB_DEVICES = 'adb_devices'
_CONTINUE_RUN_ON_FAILURE = 'continue_run_on_failure'
_GTEST = 'gtest'

_ADB_DEVICES_ERR = 'Use --adb_devices to specify one or more devices.'

class BuildAllEnvironment(buildutil.android.BuildEnvironment):

  """Class representing the build environment of multiple Android projects.

  Attributes:
    search_path: Path to start searching for projects to build.
  """

  def __init__(self, arguments):
    """Setup the environment to build projects under the working directory.

    Args:
      arguments: The argument object returned from ArgumentParser.parse_args().
    """

    super(BuildAllEnvironment, self).__init__(arguments)

    if type(arguments) is dict:
      args = arguments
    else:
      args = vars(arguments)

    self.search_path = args[_SEARCH_PATH]
    self.apk_install = args[_APK_INSTALL]
    self.apk_run = args[_APK_RUN]

  @staticmethod
  def build_defaults():
    """Helper function to set build defaults.

    Returns:
      A dict containing appropriate defaults for a build.
    """
    args = buildutil.android.BuildEnvironment.build_defaults()

    args[_SEARCH_PATH] = '.'
    args[_APK_OUTPUT_DIR] = 'apks'
    args[_LIB_OUTPUT_DIR] = 'libs'
    args[_EXCLUDE_DIRS] = []
    args[_APK_INSTALL] = False
    args[_APK_RUN] = False
    args[_ADB_DEVICES] = []
    args[_CONTINUE_RUN_ON_FAILURE] = False
    args[_GTEST] = False
    return args

  @staticmethod
  def add_arguments(parser):
    """Add module-specific command line arguments to an argparse parser.

    This will take an argument parser and add arguments appropriate for this
    module. It will also set appropriate default values.

    Args:
      parser: The argparse.ArgumentParser instance to use.
    """
    defaults = BuildAllEnvironment.build_defaults()

    buildutil.android.BuildEnvironment.add_arguments(parser)

    parser.add_argument('-R', '--' + _SEARCH_PATH,
                        help='Path to search for Android projects.',
                        dest=_SEARCH_PATH, default=defaults[_SEARCH_PATH])
    parser.add_argument('-O', '--' + _APK_OUTPUT_DIR,
                        help='Directory to place built apks.',
                        dest=_APK_OUTPUT_DIR,
                        default=defaults[_APK_OUTPUT_DIR])
    parser.add_argument('-L', '--' + _LIB_OUTPUT_DIR,
                        help='Directory to place built shared libraries.',
                        dest=_LIB_OUTPUT_DIR,
                        default=defaults[_LIB_OUTPUT_DIR])
    parser.add_argument('-E', '--' + _EXCLUDE_DIRS,
                        help='List of directory names (not paths) to exclude '
                        'from the Android project search.',
                        dest=_EXCLUDE_DIRS, default=defaults[_EXCLUDE_DIRS],
                        nargs='+')
    parser.add_argument('-i', '--' + _APK_INSTALL,
                        help=('Whether to install each apk on the selected '
                              'device(s).'),
                        dest=_APK_INSTALL, action='store_true',
                        default=defaults[_APK_INSTALL])
    parser.add_argument('-r', '--' + _APK_RUN,
                        help=('Whether to run each apk on the selected '
                              'device(s).'),
                        dest=_APK_RUN, action='store_true',
                        default=defaults[_APK_RUN])
    parser.add_argument('-d', '--' + _ADB_DEVICES,
                        help=('List of Android device serials to install to / '
                              'run on.  If @ is specified all '
                              'attached devices are selected.'),
                        dest=_ADB_DEVICES, default=defaults[_ADB_DEVICES],
                        nargs='+')
    parser.add_argument('--' + _CONTINUE_RUN_ON_FAILURE,
                        help=('When set, this script will continue to attempt '
                              'to run all built packages when a package fails '
                              'to launch.'),
                        dest=_CONTINUE_RUN_ON_FAILURE, action='store_true',
                        default=defaults[_CONTINUE_RUN_ON_FAILURE])
    parser.add_argument('--' + _GTEST,
                        help=('When set, the output of each executed package '
                              'is scraped for test failure messages.'),
                        dest=_GTEST, action='store_true',
                        default=defaults[_GTEST])


def main():
  """Entry point for the application, see the module help for more information.

  Returns:
    0 if successful, non-zero if an error occurs.
  """
  parser = argparse.ArgumentParser()
  BuildAllEnvironment.add_arguments(parser)
  args = parser.parse_args()

  env = BuildAllEnvironment(args)

  (rc, errmsg) = env.build_all(
      path=args.search_path, apk_output=args.apk_output_dir,
      lib_output=args.lib_output_dir, exclude_dirs=args.exclude_dirs)
  if (rc != 0):
    print >> sys.stderr, errmsg
    return rc

  adb_devices = args.adb_devices if args.adb_devices else [None]
  # If all devices are selected, get the list of serial numbers.
  if '@' in adb_devices:
    adb_devices = [d.serial for d in env.get_adb_devices()]

  if env.apk_install and not env.clean:
    for adb_device in adb_devices:
      try:
        (rc, errmsg) = env.install_all(
            path=args.search_path, adb_device=adb_device, exclude_dirs=[
                args.apk_output_dir, args.lib_output_dir] + args.exclude_dirs)
        if (rc != 0):
          print >> sys.stderr, errmsg
          return rc
      except buildutil.common.AdbError as e:
        print >> sys.stderr, str(e) + '\n' + _ADB_DEVICES_ERR
        return 1


  if env.apk_run and not env.clean:
    failed_targets = []
    for adb_device in adb_devices:
      try:
        (rc, errmsg, failures) = env.run_all(
            path=args.search_path, adb_device=adb_device, exclude_dirs=[
                 args.apk_output_dir, args.lib_output_dir] + args.exclude_dirs,
            continue_on_failure=args.continue_run_on_failure, gtest=args.gtest)
      except buildutil.common.AdbError as e:
        print >> sys.stderr, str(e) + '\n' + _ADB_DEVICES_ERR
        return 1

      failed_targets.extend(failures)
      if (rc != 0):
        print >> sys.stderr, errmsg
        if not args.continue_run_on_failure:
          return rc
    if failed_targets:
      print >> sys.stderr, 'The following targets failed:'
      print >> sys.stderr, os.linesep.join(failed_targets)
      return 1

  return 0

if __name__ == '__main__':
  sys.exit(main())
