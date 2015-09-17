# Copyright 2014 Google Inc. All Rights Reserved.
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

"""@file buildutil/android.py Android-specific BuildEnvironment.
@namespace buildutil.android

Optional environment variables:

@li ANT_PATH = Path to ant binary. Required if ant is not in $PATH,
or not passed on command line.
@li ANDROID_SDK_HOME = Path to the Android SDK. Required if it is not passed
on the command line.
@li NDK_HOME = Path to the Android NDK. Required if it is not in passed on the
command line.
"""

import datetime
import errno
import os
import platform
import random
import re
import shlex
import shutil
import stat
import subprocess
import sys
import tempfile
import time
import uuid
import xml.etree.ElementTree
sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir))
import buildutil.common as common

_SDK_HOME_ENV_VAR = 'ANDROID_SDK_HOME'
_NDK_HOME_ENV_VAR = 'NDK_HOME'
_SDK_HOME = 'sdk_home'
_NDK_HOME = 'ndk_home'
_ANT_PATH_ENV_VAR = 'ANT_PATH'
_ANT_PATH = 'ant_path'
_ANT_FLAGS = 'ant_flags'
_ANT_TARGET = 'ant_target'
_APK_KEYSTORE = 'apk_keystore'
_APK_PASSFILE = 'apk_passfile'
_APK_KEYALIAS = 'apk_keyalias'
_APK_KEYPK8 = 'apk_keypk8'
_APK_KEYPEM = 'apk_keypem'
_SIGN_APK = 'sign_apk'
_MANIFEST_FILE = 'AndroidManifest.xml'
_NDK_MAKEFILE = 'Android.mk'
_ALWAYS_MAKE = 'always_make'
_ADB_LOGCAT_ARGS = 'adb_logcat_args'
_ADB_LOGCAT_MONITOR = 'adb_logcat_monitor'
_IGNORE_SDK_VERSION_MISSING = 'ignore_sdk_version_missing'

_MATCH_DEVICES = re.compile(r'^List of devices attached\s*')
_MATCH_PACKAGE = re.compile(r'^package:(.*)')
_GTEST_FAILED = re.compile(r'(\[\s*FAILEDs\*\]|.*FAILED TESTS)')

_ACTION_MAIN = 'android.intent.action.MAIN'
_CATEGORY_LAUNCHER = 'android.intent.category.LAUNCHER'
_NATIVE_ACTIVITY = 'android.app.NativeActivity'
_ANDROID_MANIFEST_SCHEMA = 'http://schemas.android.com/apk/res/android'


class XMLFile(object):
  """XML file base class factored for testability.

  Subclasses implement process(self, etree) to process the parsed XML.
  On error, they should raise common.ConfigurationError.

  Attributes:
    path: Path to XML file as set in initializer.
  """

  def __init__(self, path):
    """Constructs the XMLFile for a specified path.

    Args:
      path: The absolute path to the manifest file.

    Raises:
      common.ConfigurationError: Manifest file missing.
    """
    if path and not os.path.exists(path):
      raise common.ConfigurationError(path, os.strerror(errno.ENOENT))

    self.path = path

  def parse(self):
    """Parse the XML file and extract useful information.

    Raises:
      ConfigurationError: Elements were missing or incorrect in the file.
      IOError: Could not open XML file.
    """
    with open(self.path, 'r') as xmlfile:
      self._parse(xmlfile)

  def _parse(self, xmlfile):
    try:
      etree = xml.etree.ElementTree.parse(xmlfile)
      self.process(etree)
    except xml.etree.ElementTree.ParseError as pe:
      raise common.ConfigurationError(self.path, 'XML parse error: ' + str(pe))


class AndroidManifest(XMLFile):
  """Class that extracts build information from an AndroidManifest.xml.

  Attributes:
    min_sdk: Minimum SDK version from the uses-sdk element.
    target_sdk: Target SDK version from the uses-sdk element, or min_sdk if it
      is not set.
    package_name: Name of the package.
    activity_name: Name of the first activity in the manifest.
    activity_names: List of names of each of the activities.
    main_activity_name: Name of the main activity.
    lib_name: Name of the library loaded by android.app.NativeActivity.
    ignore_sdk_version_missing: If set, ignore issues when the manifest doesn't
      contain SDK versioning elements. This is intended to be used when the
      minSdkVersion is defined in gradle configuration.
  """

  class MissingActivityError(common.ConfigurationError):
    """Raised if an activity element isn't present in a manifest.

    Attribtues:
      manifest: Manifest instance which detected an error.
    """

    def __init__(self, path, error, manifest):
      """Initialize this MissingActivityError.

      Args:
        path: Path to file that generated the error.
        error: The specific error to report.
        manifest: Manifest instance which detected an error.
      """
      super(AndroidManifest.MissingActivityError, self).__init__(path, error)
      self.manifest = manifest

  def __init__(self, path, ignore_sdk_version_missing=False):
    """Constructs the AndroidManifest for a specified path.

    Args:
      path: The absolute path to the manifest file.
      ignore_sdk_version_missing: How to handle missing SDK version elements
        during manifest processing. If set to False, raise an exception. If
        set to True, processing will continue silently.

    Raises:
      ConfigurationError: Manifest file missing.
    """
    super(AndroidManifest, self).__init__(path)

    self.activity_name = ''
    self.activity_names = []
    self.lib_name = ''
    self.main_activity_name = ''
    self.min_sdk = 0
    self.package_name = ''
    self.target_sdk = 0
    self.ignore_sdk_version_missing = ignore_sdk_version_missing

  def _process_sdk_element(self, sdk_element):
    """Processes a "uses-sdk" XML element and stores min/target sdk attributes.

    Args:
      sdk_element: xml.etree.ElementTree that represents a "uses-sdk" clause
          in an AndroidManifest.

    Raises:
      common.ConfigurationError: sdk_element was None, or minSdkVersion missing
    """
    if sdk_element is None:
      raise common.ConfigurationError(self.path, 'uses-sdk element missing')

    min_sdk_version = AndroidManifest.__get_schema_attribute_value(
        sdk_element, 'minSdkVersion')

    if not min_sdk_version:
      raise common.ConfigurationError(self.path, 'minSdkVersion missing')

    target_sdk_version = AndroidManifest.__get_schema_attribute_value(
        sdk_element, 'targetSdkVersion')

    if not target_sdk_version:
      target_sdk_version = min_sdk_version
    self.min_sdk = int(min_sdk_version)
    self.target_sdk = int(target_sdk_version)

  def process(self, etree):
    """Process the parsed AndroidManifest to extract SDK version info.

    Args:
      etree: An xml.etree.ElementTree object of the parsed XML file.

    Raises:
      ConfigurationError: Required elements were missing or incorrect.
      MissingActivityError: If the main activity element isn't present. This
        instance will be completely populated when this exception is thrown.
    """
    root = etree.getroot()

    self.package_name = root.get('package')
    sdk_element = root.find('uses-sdk')

    # Attempt to process the element, but if ignore_sdk_version_missing is
    # False, ignore issues of sdk version elements not existing in the
    # manifest.
    try:
      self._process_sdk_element(sdk_element)
    except common.ConfigurationError as e:
      if not self.ignore_sdk_version_missing:
        raise e

    app_element = root.find('application')
    if app_element is None:
      raise common.ConfigurationError(self.path, 'application missing')

    if not self.package_name:
      raise common.ConfigurationError(self.path, 'package missing')

    activity_elements = app_element.findall('activity')
    for activity_element in activity_elements:
      activity_name = AndroidManifest.__get_schema_attribute_value(
          activity_element, 'name')
      self.activity_names.append(activity_name)

      if AndroidManifest._check_main_activity(activity_element):
        main_activity = activity_element
        self.main_activity_name = activity_name

    if self.main_activity_name == _NATIVE_ACTIVITY:
      for metadata_element in main_activity.findall('meta-data'):
        if (AndroidManifest.__get_schema_attribute_value(
            metadata_element, 'name') == 'android.app.lib_name'):
          self.lib_name = AndroidManifest.__get_schema_attribute_value(
              metadata_element, 'value')

      if not self.lib_name:
        raise common.ConfigurationError(
            self.path, 'meta-data android.app.lib_name missing')

    elif not self.main_activity_name:
      raise AndroidManifest.MissingActivityError(
          self.path, 'main activity missing', self)

    # For backwards compatability.
    if self.activity_names:
      self.activity_name = self.activity_names[0]

  @staticmethod
  def _check_main_activity(activity_xml_element):
    """Helper function to determine if an activity is considered main.

    An activity is considered main if it has an intent-filter, filters for
    the main action, and the category launcher.

    Args:
      activity_xml_element: xml.etree.ElementTree of an Android activity.

    Returns:
      True if the activity is considered main, False otherwise.
    """
    intent_filter_element = activity_xml_element.find('intent-filter')
    if intent_filter_element is None:
      return False

    action_elements = intent_filter_element.findall('action')
    action_names = (AndroidManifest.__get_schema_attribute_value(a, 'name')
                    for a in action_elements)
    if _ACTION_MAIN not in action_names:
      return False

    category_elements = intent_filter_element.findall('category')
    category_names = (AndroidManifest.__get_schema_attribute_value(c, 'name')
                      for c in category_elements)
    if _CATEGORY_LAUNCHER not in category_names:
      return False

    return True

  @staticmethod
  def __get_schema_attribute_value(xml_element, attribute):
    """Get attribute from xml_element using the Android manifest schema.

    Args:
      xml_element: xml.etree.ElementTree to query.
      attribute: Name of Android Manifest attribute to retrieve.

    Returns:
      XML attribute string from the specified element.
    """
    return xml_element.get('{%s}%s' % (_ANDROID_MANIFEST_SCHEMA, attribute))


class BuildXml(XMLFile):

  """Class that extracts build information from an ant build.xml.

  Attributes:
    project_name: The name of the project, used by ant to name output files.
  """

  def __init__(self, path):
    """Constructs the BuildXml for a specified path.

    Args:
      path: The absolute path to the build.xml file.

    Raises:
      ConfigurationError: build.xml file missing.
    """
    super(BuildXml, self).__init__(path)

    self.project_name = None

  def process(self, etree):
    """Process the parsed build.xml to extract project info.

    Args:
      etree: An xml.etree.ElementTree object of the parsed XML file.

    Raises:
      ConfigurationError: Required elements were missing or incorrect.
    """
    project_element = etree.getroot()

    if project_element.tag != 'project':
      raise common.ConfigurationError(self.path, 'project element missing')

    self.project_name = project_element.get('name')

    if not self.project_name:
      raise common.ConfigurationError(self.path, 'project name missing')


class AdbDevice(object):
  """Stores information about an Android device.

  Attributes:
    serial: Serial number of the device.
    type: Type of device.
    usb: USB location.
    product: Product codename.
    device: Device codename.
    model: Model name.
  """

  def __init__(self, adb_device_line=None):
    """Initialize this instance from a device line from "adb devices -l".

    Args:
      adb_device_line: Device line from "adb devices -l".
    """
    self.serial = ''
    self.type = ''
    self.usb = ''
    self.product = ''
    self.device = ''
    self.model = ''
    # 'adb device -l' returns a line per device according to this format:
    # SERIAL TYPE [KEY:VALUE ...]
    if adb_device_line:
      tokens = adb_device_line.split()
      if len(tokens) < 2:
        print >>sys.stderr, ('Warning: AdbDevice initialized with '
                             'adb_device_line "{}", expected "SERIAL TYPE '
                             '[KEY:VALUE ...]"'.format(adb_device_line))
      else:
        self.serial = tokens[0]
        self.type = tokens[1]
        for token in tokens[2:]:
          key_value = token.split(':')
          if len(key_value) != 2:
            continue
          setattr(self, key_value[0], key_value[1])

  def __str__(self):
    """Convert this instance into a string representation.

    Returns:
      A string in the form "key0:value0 key1:value1 ... keyN:valueN"
      where key is an attribute of this instance and value is the value of
      the attribute.
    """
    return ' '.join([':'.join([k, v]) for k, v in
                     sorted(self.__dict__.iteritems())])


class BuildEnvironment(common.BuildEnvironment):

  """Class representing an Android build environment.

  This class adds Android-specific functionality to the common
  BuildEnvironment.

  Attributes:
    ndk_home: Path to the Android NDK, if found.
    sdk_home: Path to the Android SDK, if found.
    ant_path: Path to the ant binary, if found.
    ant_flags: Flags to pass to the ant binary, if used.
    ant_target: Ant build target name.
    sign_apk: Enable signing of Android APKs.
    apk_keystore: Keystore file path to use when signing an APK.
    apk_keyalias: Alias of key to use when signing an APK.
    apk_passfile: Path to file containing a password to use when signing an
      APK.
    apk_keycertpair: (key, cert) tuple where Key is a .pk8 key file and
      cert is a .pem certificate file.
    adb_logcat_args: List of additional arguments passed to logcat when
      monitoring the application's output.
    adb_logcat_monitor: Whether to continue to monitor the application's
      output after it has launched or has been destroyed.
    always_make: Whether to build when the project is already up to date.
  """

  ADB = 'adb'
  ANDROID = 'android'
  ANT = 'ant'
  JARSIGNER = 'jarsigner'
  KEYTOOL = 'keytool'
  NDK_BUILD = 'ndk-build'
  ZIPALIGN = 'zipalign'

  def __init__(self, arguments):
    """Constructs the BuildEnvironment with basic information needed to build.

    The build properties as set by argument parsing are also available
    to be modified by code using this object after construction.

    It is required to call this function with a valid arguments object,
    obtained either by calling argparse.ArgumentParser.parse_args() after
    adding this modules arguments via BuildEnvironment.add_arguments(), or
    by passing in an object returned from BuildEnvironment.build_defaults().

    Args:
      arguments: The argument object returned from ArgumentParser.parse_args().
    """

    super(BuildEnvironment, self).__init__(arguments)

    if type(arguments) is dict:
      args = arguments
    else:
      args = vars(arguments)

    self.ndk_home = args[_NDK_HOME]
    self.sdk_home = args[_SDK_HOME]
    self.ant_path = args[_ANT_PATH]
    self.ant_flags = args[_ANT_FLAGS]
    self.ant_target = args[_ANT_TARGET]
    self.sign_apk = args[_SIGN_APK]
    self.apk_keystore = args[_APK_KEYSTORE]
    self.apk_keyalias = args[_APK_KEYALIAS]
    self.apk_passfile = args[_APK_PASSFILE]
    self.apk_keycertpair = None
    if args[_APK_KEYPK8] and args[_APK_KEYPEM]:
      self.apk_keycertpair = (args[_APK_KEYPK8], args[_APK_KEYPEM])
    self.always_make = args[_ALWAYS_MAKE]
    self.adb_logcat_args = args[_ADB_LOGCAT_ARGS]
    self.adb_logcat_monitor = args[_ADB_LOGCAT_MONITOR]
    self.ignore_sdk_version_missing = args[_IGNORE_SDK_VERSION_MISSING]

  @staticmethod
  def build_defaults():
    """Helper function to set build defaults.

    Returns:
      A dict containing appropriate defaults for a build.
    """
    args = common.BuildEnvironment.build_defaults()

    args[_SDK_HOME] = (os.getenv(_SDK_HOME_ENV_VAR) or
                       common.BuildEnvironment._find_path_from_binary(
                           BuildEnvironment.ANDROID, 2))
    args[_NDK_HOME] = (os.getenv(_NDK_HOME_ENV_VAR) or
                       common.BuildEnvironment._find_path_from_binary(
                           BuildEnvironment.NDK_BUILD, 1))
    args[_ANT_PATH] = (
        os.getenv(_ANT_PATH_ENV_VAR) or
        common._find_executable(BuildEnvironment.ANT))
    args[_ANT_FLAGS] = '-quiet'
    args[_ANT_TARGET] = 'release'
    args[_APK_KEYSTORE] = None
    args[_APK_KEYALIAS] = None
    args[_APK_PASSFILE] = None
    args[_APK_KEYPK8] = None
    args[_APK_KEYPEM] = None
    args[_SIGN_APK] = False
    args[_ALWAYS_MAKE] = False
    args[_ADB_LOGCAT_ARGS] = []
    args[_ADB_LOGCAT_MONITOR] = False
    # We expect an SDK version check by default.
    args[_IGNORE_SDK_VERSION_MISSING] = False

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

    parser.add_argument('-n', '--' + _NDK_HOME,
                        help='Path to Android NDK', dest=_NDK_HOME,
                        default=defaults[_NDK_HOME])
    parser.add_argument('-s', '--' + _SDK_HOME,
                        help='Path to Android SDK', dest=_SDK_HOME,
                        default=defaults[_SDK_HOME])
    parser.add_argument('-a', '--' + _ANT_PATH,
                        help='Path to ant binary', dest=_ANT_PATH,
                        default=defaults[_ANT_PATH])
    parser.add_argument('-A', '--' + _ANT_FLAGS,
                        help='Flags to use to override ant flags',
                        dest=_ANT_FLAGS, default=defaults[_ANT_FLAGS])
    parser.add_argument('-T', '--' + _ANT_TARGET,
                        help=('Target to use for ant build.  If the clean '
                              'option is specified, this is ignored.'),
                        dest=_ANT_TARGET, default=defaults[_ANT_TARGET])
    parser.add_argument('-k', '--' + _APK_KEYSTORE,
                        help='Path to keystore to use when signing an APK',
                        dest=_APK_KEYSTORE, default=defaults[_APK_KEYSTORE])
    parser.add_argument('-K', '--' + _APK_KEYALIAS,
                        help='Key alias to use when signing an APK',
                        dest=_APK_KEYALIAS, default=defaults[_APK_KEYALIAS])
    parser.add_argument('-P', '--' + _APK_PASSFILE,
                        help='Path to file containing keystore password',
                        dest=_APK_PASSFILE, default=defaults[_APK_PASSFILE])
    parser.add_argument('--' + _APK_KEYPK8,
                        help=('.pk8 key file used to sign an APK.  To use '
                              'this option %s must also specified a '
                              'certificate file.' % _APK_KEYPEM),
                        dest=_APK_KEYPK8, default=defaults[_APK_KEYPK8])
    parser.add_argument('--' + _APK_KEYPEM,
                        help=('.pem certificate file used to sign an APK.  '
                              'To use this option %s must also specified a '
                              'key file.' % _APK_KEYPK8),
                        dest=_APK_KEYPEM, default=defaults[_APK_KEYPEM])
    parser.add_argument('-S', '--' + _SIGN_APK,
                        help='Enable signing of Android APKs.',
                        dest=_SIGN_APK, action='store_true',
                        default=defaults[_SIGN_APK])
    parser.add_argument('-B', '--' + _ALWAYS_MAKE,
                        help='Always build all up to date targets.',
                        dest=_ALWAYS_MAKE, action='store_true')
    parser.add_argument('--no-' + _ALWAYS_MAKE,
                        help='Only build out of date targets.',
                        dest=_ALWAYS_MAKE, action='store_false')
    parser.add_argument('--' + _ADB_LOGCAT_ARGS,
                        help='Additonal arguments to pass to logcat',
                        dest=_ADB_LOGCAT_ARGS,
                        default=defaults[_ADB_LOGCAT_ARGS], nargs='+')
    parser.add_argument('--' + _ADB_LOGCAT_MONITOR,
                        help=('Whether to continue to monitor logcat output '
                              'after the application has been displayed or '
                              'destroyed.'),
                        dest=_ADB_LOGCAT_MONITOR, action='store_true',
                        default=defaults[_ADB_LOGCAT_MONITOR])
    parser.add_argument('--' + _IGNORE_SDK_VERSION_MISSING,
                        help=('Disable SDK version checks when reading the '
                              'application manifest. This is required in '
                              'cases where SDK versions are specified in '
                              'gradle project files instead of the manifest.'),
                        action='store_true',
                        default=defaults[_IGNORE_SDK_VERSION_MISSING])

    parser.set_defaults(
        **{_ALWAYS_MAKE: defaults[_ALWAYS_MAKE]})  # pylint: disable=star-args

  def _find_binary(self, binary, additional_paths=None):
    """Find a binary from the set of binaries managed by this class.

    This method enables the lookup of a binary path using the name of the
    binary to avoid replication of code which searches for binaries.

    This class allows the lookup of...
    * BuildEnvironment.ADB
    * BuildEnvironment.ANDROID
    * BuildEnvironment.ANT
    * BuildEnvironment.JARSIGNER
    * BuildEnvironment.KEYTOOL
    * BuildEnvironment.NDK_BUILD
    * BuildEnvironment.ZIPALIGN

    The _find_binary() method in derived classes may add more binaries.

    Args:
      binary: Name of the binary.
      additional_paths: Additional dictionary to search for binary paths.

    Returns:
      String containing the path of binary.

    Raises:
      ToolPathError: Binary is not at the specified path.
    """
    ndk_build_paths = []
    if self.ndk_home:
      ndk_build_paths = [os.path.join(self.ndk_home, '')]

    # zipalign is under the sdk/build-tools subdirectory in ADT 20140702
    # or newer.  In older ADT releases zipalign was located in sdk/tools.
    zip_align_paths = []
    if binary == BuildEnvironment.ZIPALIGN:
      zip_align_paths = [os.path.join(self.sdk_home, 'tools', '')]
      for root, dirs, _ in os.walk(os.path.join(self.sdk_home, 'build-tools')):
        zip_align_paths.extend([os.path.join(root, d, '') for d in dirs])
        break

    if not self.sdk_home and not ndk_build_paths:
      raise common.ToolPathError('Android SDK and NDK', '[unknown]')
    elif not ndk_build_paths:
      raise common.ToolPathError('Android NDK', '[unknown]')
    elif not self.sdk_home:
      raise common.ToolPathError('Android SDK', '[unknown]')

    search_dict = {
        BuildEnvironment.ADB: [os.path.join(
            self.sdk_home, 'platform-tools', '')],
        BuildEnvironment.ANDROID: [
            os.path.join(self.sdk_home, 'tools', '')],
        BuildEnvironment.ANT: [self.ant_path],
        BuildEnvironment.NDK_BUILD: ndk_build_paths,
        BuildEnvironment.JARSIGNER: [],
        BuildEnvironment.KEYTOOL: [],
        BuildEnvironment.ZIPALIGN: zip_align_paths}
    if additional_paths:
      search_dict.append(additional_paths)

    return common.BuildEnvironment._find_binary(self, binary, search_dict)

  def build_android_libraries(self, subprojects, output=None):
    """Build list of Android library projects.

    This function iteratively runs ndk-build over a list of paths relative
    to the current project directory.

    Args:
      subprojects: A list pf paths relative to the project directory to build.
      output: An optional directory relative to the project directory to
          receive the build output.

    Raises:
      SubCommandError: ndk-build invocation failed or returned an error.
      ToolPathError: Android NDK location not found in configured build
          environment or $PATH.
    """
    ndk_build = self._find_binary(BuildEnvironment.NDK_BUILD)

    for p in subprojects:
      # Disable parallel clean on OSX.
      cpu_count = self.cpu_count
      if self.clean and platform.mac_ver()[0]:
        cpu_count = 1

      args = [ndk_build, '-j' + str(cpu_count)]
      if self.always_make:
        args.append('-B')
      args += ['-C', self.get_project_directory(path=p)]
      if self.clean:
        args.append('clean')

      if self.verbose:
        args.append('V=1')

      if output:
        args.append(
            'NDK_OUT=%s' % self.get_project_directory(path=output))

      if self.make_flags:
        args += shlex.split(self.make_flags, posix=self._posix)

      self.run_subprocess(args)

  def _find_best_android_sdk(self, android, minsdk, target):
    """Finds the best installed Android SDK for a project.

    Based on the passed in min and target SDK levels, find the highest SDK
    level installed that is greater than the specified minsdk, up to the
    target sdk level. Return it as an API level string.

    Otherwise, if the minimum installed SDK is greater than the
    targetSdkVersion, return the maximum installed SDK version, or raise a
    ConfigurationError if no installed SDK meets the min SDK.

    Args:
      android: Path to android tool binary.
      minsdk: Integer minimum SDK level.
      target: Integer target SDK level.

    Returns:
      Highest installed Android SDK API level in the range, as a string.

    Raises:
      SubCommandError: NDK toolchain invocation failed or returned an error.
      ToolPathError: Android NDK or SDK location not found in configured build
          environment or $PATH, or ant not found.
      ConfigurationError: Required build configuration file missing or broken
          in an unrecoverable way.
    """
    acmd = [android, 'list', 'target', '--compact']
    (stdout, unused_stderr) = self.run_subprocess(acmd, capture=True)

    if self.verbose:
      print 'android list target returned: {%s}' % (stdout)
    # Find the highest installed SDK <= targetSdkVersion, if possible.
    #
    # 'android list target --compact' will output lines like:
    #
    # android-1
    # android-2
    #
    # for installed SDK targets, along with other info not starting with
    # android-.
    installed = 0
    for line in stdout.splitlines():
      l = line.strip()
      if l.startswith('android-'):
        nstr = l.split('-')[1]
        # Ignore preview SDK revisions (e.g "L").
        if not nstr.isdigit():
          continue
        n = int(nstr)
        if n > installed:
          if self.verbose:
            print 'sdk api level %d found' % (n)
          installed = n
        if installed == target:
          break

    if installed < minsdk:
      raise common.ConfigurationError(self.sdk_home,
                                      ('Project requires Android SDK %d, '
                                       'but only found up to %d' %
                                       (minsdk, installed)))

    apitarget = 'android-%d' % (installed)
    return apitarget

  def get_manifest_path(self, path='.'):
    """Get the path of the manifest file.

    Args:
      path: Optional relative path from project directory to project to build.

    Returns:
      Path of the manifest file.
    """
    return os.path.join(self.get_project_directory(path=path), _MANIFEST_FILE)

  def parse_manifest(self, path='.'):
    """Parse the project's manifest.

    Args:
      path: Optional relative path from project directory to project to build.

    Returns:
      AndroidManifest instance parsed from the project manifest.

    Raises:
      ConfigurationError: Required elements were missing or incorrect.
      MissingActivityError: If a main activity element isn't present.
    """
    manifest = AndroidManifest(self.get_manifest_path(path=path),
                               ignore_sdk_version_missing=
                               self.ignore_sdk_version_missing)
    manifest.parse()
    return manifest

  def create_update_build_xml(self, manifest, path='.'):
    """Create or update ant build.xml for an Android project.

    Args:
      manifest: Parsed AndroidManifest instance.
      path: Optional relative path from project directory to project to build.

    Returns:
      BuildXml instance which references the created / updated ant project.
    """
    android = self._find_binary(BuildEnvironment.ANDROID)

    project = self.get_project_directory(path=path)
    buildxml_path = os.path.join(project, 'build.xml')

    # Get the last component of the package name for the application name.
    app_name = manifest.package_name[manifest.package_name.rfind('.') + 1:]

    # If no build.xml exists, create one for the project in the directory
    # we are currently building.
    if (not os.path.exists(buildxml_path) or
        os.path.getmtime(buildxml_path) < os.path.getmtime(manifest.path)):
      apitarget = self._find_best_android_sdk(android, manifest.min_sdk,
                                              manifest.target_sdk)
      self.run_subprocess([android, 'update', 'project', '--path', project,
                           '--target', apitarget, '--name', app_name])

    buildxml = BuildXml(buildxml_path)
    buildxml.parse()
    return buildxml

  def get_apk_filenames(self, app_name, path='.'):
    """Get the set of output APK names for the project.

    Args:
      app_name: Basename of the APK parsed from build.xml.
      path: Relative path from project directory to project to build.

    Returns:
      (signed_apkpath, unsigned_apkpath) where signed_apkpath and
      unsigned_apkpath are paths to the signed and unsigned APKs respectively.
      Signing is optional so the signed APK may not be present when the
      project has been built successfully.
    """
    # ant outputs to $PWD/bin. The APK will have a name as constructed below.
    project_directory = self.get_project_directory(path=path)
    apk_directory = os.path.join(project_directory, 'bin')
    if self.ant_target == 'debug':
      unsigned_apkpath = os.path.join(apk_directory, '%s-%s.apk' % (
          app_name, self.ant_target))
      signed_apkpath = unsigned_apkpath
    else:
      unsigned_apkpath = os.path.join(apk_directory, '%s-%s-unsigned.apk' % (
          app_name, self.ant_target))
      signed_apkpath = os.path.join(apk_directory, '%s.apk' % app_name)
    return (signed_apkpath, unsigned_apkpath)

  def build_android_apk(self, path='.', output=None, manifest=None):
    """Build an Android APK.

    This function builds an APK by using ndk-build and ant, at an optionally
    specified relative path from the current project directory, and output to
    an optionally specified output directory, also relative to the current
    project directory. Flags are passed to ndk-build and ant as specified in
    the build environment. This function does not install the resulting APK.

    If no build.xml is found, one is generated via the 'android' command, if
    possible.

    Args:
      path: Optional relative path from project directory to project to build.
      output: Optional relative path from project directory to output
        directory.
      manifest: Parsed AndroidManifest instance.

    Raises:
      SubCommandError: NDK toolchain invocation failed or returned an error.
      ToolPathError: Android NDK or SDK location not found in configured build
          environment or $PATH, or ant not found.
      ConfigurationError: Required build configuration file missing or broken
          in an unrecoverable way.
      IOError: An error occurred writing or copying the APK.
    """
    ant = self._find_binary(BuildEnvironment.ANT)
    build_apk = True

    try:
      if not manifest:
        manifest = self.parse_manifest(path=path)
    except AndroidManifest.MissingActivityError as e:
      # If the activity is missing it's still possible to build the project.
      print >> sys.stderr, str(e)
      manifest = e.manifest
      build_apk = False

    # Create or update build.xml for ant.
    buildxml = self.create_update_build_xml(
        manifest if manifest else self.parse_manifest(path=path),
        path=path)

    acmd = [ant, 'clean' if self.clean else self.ant_target]
    if self.ant_flags:
      acmd += shlex.split(self.ant_flags, posix=self._posix)

    self.run_subprocess(acmd, cwd=path)

    if not build_apk:
      return

    signed_apkpath, unsigned_apkpath = self.get_apk_filenames(
        buildxml.project_name, path=path)
    source_apkpath = unsigned_apkpath

    if self.sign_apk and not self.clean:
      if self.ant_target != 'debug':
        source_apkpath = signed_apkpath
        self._sign_apk(unsigned_apkpath, signed_apkpath)
      else:
        print >>sys.stderr, 'Signing not required for debug target %s.' % (
            unsigned_apkpath)

    if output and not self.clean:
      out_abs = self.get_project_directory(path=output)
      if not os.path.exists(out_abs):
        os.makedirs(out_abs)
      if self.verbose:
        print 'Copying apk %s to: %s' % (source_apkpath, out_abs)
      shutil.copy2(source_apkpath, out_abs)

  @staticmethod
  def generate_password():
    """Generate a psuedo random password.

    Returns:
      8 character hexadecimal string.
    """
    return '%08x' % (random.random() * 16 ** 8)

  def _sign_apk(self, source, target):
    """This function signs an Android APK, optionally generating a key.

    This function signs an APK using a keystore and password as configured
    in the build configuration. If none are configured, it generates an
    ephemeral key good for 60 days.

    Args:
      source: Absolute path to source APK to sign.
      target: Target path to write signed APK to.

    Raises:
      SubCommandError: Jarsigner invocation failed or returned an error.
      ToolPathError: Jarsigner or keygen location not found in $PATH.
      ConfigurationError: User specified some but not all signing parameters.
      IOError: An error occurred copying the APK.
    """
    # Debug targets are automatically signed and aligned by ant.
    if self.ant_target is 'debug':
      return

    keystore = self.apk_keystore
    passfile = self.apk_passfile
    alias = self.apk_keyalias

    # If any of keystore, passwdfile, or alias are None we will create a
    # temporary keystore with a random password and alias and remove it after
    # signing. This facilitates testing release builds when the release
    # keystore is not available (such as in a continuous testing environment).
    ephemeral = False

    # Exit and don't sign if the source file is older than the target.
    if os.path.exists(target):
      if os.path.getmtime(source) < os.path.getmtime(target):
        return

    # If a key / cert pair is specified, generate a temporary key store to sign
    # the APK.
    temp_directory = ''
    if self.apk_keycertpair:
      key = tempfile.NamedTemporaryFile()
      self.run_subprocess(('openssl', 'pkcs8', '-inform', 'DER', '-nocrypt',
                           '-in', self.apk_keycertpair[0], '-out', key.name))

      p12 = tempfile.NamedTemporaryFile()
      password_file = tempfile.NamedTemporaryFile()
      password = BuildEnvironment.generate_password()
      passfile = password_file.name
      password_file.write(password)
      password_file.flush()

      alias = BuildEnvironment.generate_password()
      self.run_subprocess(('openssl', 'pkcs12', '-export', '-in',
                           self.apk_keycertpair[1], '-inkey', key.name,
                           '-out', p12.name, '-password', 'pass:' + password,
                           '-name', alias))
      key.close()

      temp_directory = tempfile.mkdtemp()
      keystore = os.path.join(temp_directory, 'temp.keystore')
      self.run_subprocess(('keytool', '-importkeystore', '-deststorepass',
                           password, '-destkeystore', keystore,
                           '-srckeystore', p12.name, '-srcstoretype', 'PKCS12',
                           '-srcstorepass', password))
      p12.close()

    try:
      if not keystore or not passfile or not alias:
        # If the user specifies any of these, they need to specify them all,
        # otherwise we may overwrite one of them.
        if keystore:
          raise common.ConfigurationError(keystore,
                                          ('Must specify all of keystore, '
                                           'password file, and alias'))
        if passfile:
          raise common.ConfigurationError(passfile,
                                          ('Must specify all of keystore, '
                                           'password file, and alias'))
        if alias:
          raise common.ConfigurationError(alias,
                                          ('Must specify all of keystore, '
                                           'password file, and alias'))
        ephemeral = True
        keystore = source + '.keystore'
        passfile = source + '.password'
        if self.verbose:
          print ('Creating ephemeral keystore file %s and password file %s' %
                 (keystore, passfile))

        password = BuildEnvironment.generate_password()
        with open(passfile, 'w') as pf:
          if self._posix:
            os.fchmod(pf.fileno(), stat.S_IRUSR | stat.S_IWUSR)
          pf.write(password)

        alias = os.path.basename(source).split('.')[0]

        # NOTE: The password is passed via the command line for compatibility
        # with JDK 6.  Move to use -storepass:file and -keypass:file when
        # JDK 7 is a requirement for Android development.
        acmd = [self._find_binary(BuildEnvironment.KEYTOOL), '-genkeypair',
                '-v', '-dname', 'cn=, ou=%s, o=fpl' % alias, '-storepass',
                password, '-keypass', password, '-keystore', keystore,
                '-alias', alias, '-keyalg', 'RSA', '-keysize', '2048',
                '-validity', '60']
        self.run_subprocess(acmd)

      tmpapk = target + '.tmp'

      if self.verbose:
        print 'Copying APK %s for signing as %s' % (source, tmpapk)

      shutil.copy2(source, tmpapk)

      with open(passfile, 'r') as pf:
        password = pf.read()

      # NOTE: The password is passed via stdin for compatibility with JDK 6
      # which - unlike the use of keytool above - ensures the password is
      # not visible when displaying the command lines of processes of *nix
      # operating systems like Linux and OSX.
      # Move to use -storepass:file and -keypass:file when JDK 7 is a
      # requirement for Android development.
      password_stdin = os.linesep.join(
          [password, password,  # Store password and confirmation.
           password, password])  # Key password and confirmation.
      acmd = [self._find_binary(BuildEnvironment.JARSIGNER),
              '-verbose', '-sigalg', 'SHA1withRSA', '-digestalg',
              'SHA1', '-keystore', keystore, tmpapk, alias]
      self.run_subprocess(acmd, stdin=password_stdin)

      # We want to align the APK for more efficient access on the device.
      # See:
      # http://developer.android.com/tools/help/zipalign.html
      acmd = [self._find_binary(BuildEnvironment.ZIPALIGN), '-f']
      if self.verbose:
        acmd.append('-v')
      acmd += ['4', tmpapk, target]  # alignment == 4
      self.run_subprocess(acmd)

    finally:
      if temp_directory and os.path.exists(temp_directory):
        shutil.rmtree(temp_directory)
      if ephemeral:
        if self.verbose:
          print 'Removing ephemeral keystore and password files'
        if os.path.exists(keystore):
          os.unlink(keystore)
        if os.path.exists(passfile):
          os.unlink(passfile)

  def find_projects(self, path='.', exclude_dirs=None):
    """Find all Android projects under the specified path.

    Args:
      path: Path to start the search in, defaults to '.'.
      exclude_dirs: List of directory names to exclude from project
        detection in addition to ['bin', 'obj', 'res'], which are always
        excluded.

    Returns:
      (apk_dirs, lib_dirs) where apk_dirs is the list of directories which
      contain Android projects that build an APK and lib_dirs is alist of
      Android project directories that only build native libraries.
    """
    project = self.get_project_directory(path=path)

    apk_dir_set = set()
    module_dir_set = set()

    # Exclude paths where buildutil or ndk-build may generate or copy files.
    exclude = (exclude_dirs if exclude_dirs else []) + ['bin', 'obj', 'res']

    if type(exclude_dirs) is list:
      exclude += exclude_dirs

    for root, dirs, files in os.walk(project, followlinks=True):
      for ex in exclude:
        if ex in dirs:
          dirs.remove(ex)
      if _MANIFEST_FILE in files:
        apk_dir_set.add(root)
      if _NDK_MAKEFILE in files:
        p = root
        # Handle the use or nonuse of the jni subdir.
        if os.path.basename(p) == 'jni':
          p = os.path.dirname(p)
        module_dir_set.add(p)

    return (list(apk_dir_set), list(module_dir_set))

  def build_all(self, path='.', apk_output='apks', lib_output='libs',
                exclude_dirs=None):
    """Locate and build all Android sub-projects as appropriate.

    This function will recursively scan a directory tree for Android library
    and application projects and build them with the current build defaults.
    This will not work for projects which only wish for subsets to be built
    or have complicated external manipulation of makefiles and manifests, but
    it should handle the majority of projects as a reasonable default build.

    Args:
      path: Optional path to start the search in, defaults to '.'.
      apk_output: Optional path to apk output directory, default is 'apks'.
      lib_output: Optional path to library output directory, default is 'libs'.
      exclude_dirs: Optional list of directory names to exclude from project
                    detection in addition to
                    [apk_output, lib_output, 'bin', 'obj', 'res'],
                    which are always excluded.
    Returns:
      (retval, errmsg) tuple of an integer return value suitable for returning
      to the invoking shell, and an error string (if any) or None (on success).
    """

    retval = 0
    errmsg = None

    apk_dirs, lib_dirs = self.find_projects(
        path=path, exclude_dirs=([apk_output, lib_output] + (
            exclude_dirs if exclude_dirs else [])))

    if self.verbose:
      print 'Found APK projects in: %s' % str(apk_dirs)
      print 'Found library projects in: %s' % str(lib_dirs)

    try:
      self.build_android_libraries(lib_dirs, output=lib_output)
      for apk in apk_dirs:
        self.build_android_apk(path=apk, output=apk_output)
      retval = 0

    except common.Error as e:
      errmsg = 'Caught buildutil error: %s' % e.error_message
      retval = e.error_code

    except IOError as e:
      errmsg = 'Caught IOError for file %s: %s' % (e.filename, e.strerror)
      retval = -1

    return (retval, errmsg)

  def get_adb_devices(self):
    """Get the set of attached devices.

    Returns:
      (device_list, command_output) where device_list is a list of AdbDevice
      instances, one for each attached device and command_output is the raw
      output of the ADB command.
    """
    out = self.run_subprocess(
        '%s devices -l' % self._find_binary(BuildEnvironment.ADB),
        capture=True, shell=True)[0]

    devices = []

    lines = out.splitlines()
    start_line = 0
    for i, line in enumerate(lines):
      if _MATCH_DEVICES.match(line):
        start_line = i + 1
        break

    if start_line:
      for device_line in lines[start_line:]:
        if device_line:
          devices.append(AdbDevice(device_line))
    return (devices, out)

  def check_adb_devices(self, adb_device=None):
    """Gets the only attached device, or the attached device matching a serial.

    When using adb to connect to a device, adb's behavior changes depending on
    how many devices are connected. If there is only one device connected, then
    no device needs to be specified (as the only device will be used). If more
    than one device is connected and no device is specified, adb will error out
    as it does not know which device to connect to.

    This method ensures that for either case enough valid information is
    specified, and returns an instance of AdbDevice representing the valid
    device.

    Args:
      adb_device: The serial to match a device on.

    Returns:
      The only AdbDevice connected to adb, or AdbDevice matching the serial.

    Raises:
      AdbError: More than one attached device and no serial specified, or
                device with matching serial specified cannot be found.
    """
    devices, out = self.get_adb_devices()
    number_of_devices = len(devices)
    if number_of_devices == 0:
      raise common.AdbError('No Android devices are connected to this host.')

    if adb_device:
      devices = [d for d in devices if d.serial == adb_device]
      if not devices:
        raise common.AdbError(
            '%s not found in the list of devices returned by "adb devices -l".'
            'The devices connected are: %s' % (adb_device, os.linesep + out))
    elif number_of_devices > 1:
      raise common.AdbError(
          'Multiple Android devices are connected to this host and none were '
          'specified. The devices connected are: %s' % (os.linesep + out))
    return devices[0]

  def get_adb_device_argument(self, adb_device=None):
    """Construct the argument for ADB to select the specified device.

    Args:
      adb_device: Serial of the device to use with ADB.

    Returns:
      A string which contains the second argument passed to ADB to select a
      target device.
    """
    return '-s ' + adb_device if adb_device else ''

  def list_installed_packages(self, adb_device=None):
    """Get the list of packages installed on an Android device.

    Args:
      adb_device: The serial of the device to query.

    Returns:
      List of package strings.

    Raises:
      AdbError: If it's not possible to query the device.
    """
    packages = []
    for line in self.run_subprocess(
        '%s %s shell pm list packages' % (
            self._find_binary(BuildEnvironment.ADB),
            self.get_adb_device_argument(adb_device)),
        shell=True, capture=True)[0].splitlines():
      m = _MATCH_PACKAGE.match(line)
      if m:
        packages.append(m.groups()[0])
    return packages

  def get_adb_device_name(self, device):
    """Get the string which describes an AdbDevice based upon the verbose mode.

    Args:
      device: AdbDevice instance.

    Returns:
      String which describes the device.
    """
    return str(device) if self.verbose else device.serial

  def install_android_apk(self, path='.', adb_device=None, force_install=True):
    """Install an android apk on the given device.

    This function will attempt to install an unsigned APK if a signed APK is
    not available which will *only* work on rooted devices.

    Args:
      path: Relative path from project directory to project to run.
      adb_device: The serial of the device to run the apk on. If None it will
        the only device connected will be used.
      force_install: Whether to install the package if it's older than the
        package on the target device.

    Raises:
      ConfigurationError: If no APKs are found.
      AdbError: If it's not possible to install the APK.
    """
    adb_path = self._find_binary(BuildEnvironment.ADB)
    device = self.check_adb_devices(adb_device=adb_device)
    adb_device_arg = self.get_adb_device_argument(adb_device=device.serial)
    try:
      manifest = self.parse_manifest(path=path)
    except AndroidManifest.MissingActivityError as e:
      print >>sys.stderr, str(e)
      return
    buildxml = self.create_update_build_xml(manifest, path=path)
    apks = [f for f in self.get_apk_filenames(buildxml.project_name,
                                              path=path) if os.path.exists(f)]
    if not apks:
      raise common.ConfigurationError(
          'Unable to find an APK for the project in %s' % (
              self.get_project_directory(path=path)))

    print 'Installing %s on %s' % (apks[0], self.get_adb_device_name(device))

    # If the project is installed and it's older than the current APK,
    # uninstall it.
    if manifest.package_name in self.list_installed_packages(
        adb_device=adb_device):

      if not force_install:
        # Get the modification time of the package on the device.
        get_package_modification_date_args = [adb_path]
        if adb_device_arg:
          get_package_modification_date_args.extend(adb_device_arg.split())
        get_package_modification_date_args.extend([
            'shell',
            r'f=( $(ls -l $( IFS=":"; p=( $(pm path %s) ); echo ${p[1]} )) ); '
            r'echo "${f[4]} ${f[5]}"' % manifest.package_name])
        out, _ = self.run_subprocess(get_package_modification_date_args,
                                     capture=True)
        if out:
          remote_modification_time = int(time.mktime(
              datetime.datetime.strptime(out.splitlines()[0],
                                         '%Y-%m-%d %H:%M').timetuple()))
          local_modification_time = os.stat(apks[0]).st_mtime
          if local_modification_time < remote_modification_time:
            print 'Not installing %s, already up to date (%d vs. %d)' % (
                manifest.package_name, local_modification_time,
                remote_modification_time)
            return

      self.run_subprocess('%s %s uninstall %s' % (
          adb_path, adb_device_arg, manifest.package_name), shell=True)
    # Install the APK.
    self.run_subprocess('%s %s install %s' % (
        adb_path, adb_device_arg, apks[0]), shell=True)

  def install_all(self, path='.', adb_device=None, exclude_dirs=None):
    """Locate and install all Android APKs.

    This function recursively scans a directory tree for Android application
    projects and installs them on the specified device.

    Args:
      path: Path to search the search in, defaults to '.'
      adb_device: The serial of the device to install the APK to. If None the
        only device connected will be used.
      exclude_dirs: List of directory names to exclude from project
        detection (see find_projects() for more information).

    Returns:
      (retval, errmsg) tuple of an integer return value suitable for
      returning to the invoking shell, and an error string (if any) or None
      (on success).
    """
    retval = 0
    errmsg = None

    apk_dirs, unused_lib_dirs = self.find_projects(path=path,
                                                   exclude_dirs=exclude_dirs)

    for apk_dir in apk_dirs:
      try:
        self.install_android_apk(path=apk_dir, adb_device=adb_device)
      except common.Error as e:
        errmsg = 'buildutil error: %s' % e.error_message
        retval = e.error_code
        break

    return (retval, errmsg)

  def stop_process(self, package_name, adb_device=None):
    """Attempts to stop a process running under the given package name.

    Args:
      package_name: Name of the package to stop.
      adb_device: Serial of the device to stop the app on. If none specified,
        the only device connected will be used.
    """
    adb_path = self._find_binary(BuildEnvironment.ADB)
    device = self.check_adb_devices(adb_device=adb_device)
    adb_device_arg = self.get_adb_device_argument(adb_device=device.serial)
    self.run_subprocess(('%s %s shell am force-stop %s' %
                         (adb_path, adb_device_arg, package_name)),
                        shell=True)

  def get_device_dpi(self, adb_device=None):
    """Returns the dpi of a device connected to adb.

    Args:
      adb_device: Serial of the device to get the dpi from. If none specified,
        the only device connected will be used.

    Returns:
      int of the device's pixel density.
    """
    adb_path = self._find_binary(BuildEnvironment.ADB)
    device = self.check_adb_devices(adb_device=adb_device)
    adb_device_arg = self.get_adb_device_argument(adb_device=device.serial)
    density, _ = self.run_subprocess(('%s %s shell getprop ro.sf.lcd_density' %
                                      (adb_path, adb_device_arg)
                                     ),
                                     shell=True,
                                     capture=True)
    return int(density)


  def take_screencap(self, destination, adb_device=None):
    """Takes a screencap on the device and saves it to destination.

    Saves the screencap to a random filename on the device's sdcard,
    transfers the screencap to destination path, then deletes the screencap
    on the device.

    Args:
      destination: The file path where the screencap will be saved.
      adb_device: Serial of the device to take the screencap with. If none
        specified, the only device connected will be used.
    """
    adb_path = self._find_binary(BuildEnvironment.ADB)
    device = self.check_adb_devices(adb_device=adb_device)
    adb_device_arg = self.get_adb_device_argument(adb_device=device.serial)
    temp_filename = uuid.uuid4()

    self.run_subprocess(('%s %s shell screencap -p /sdcard/%s' %
                         (adb_path, adb_device_arg, temp_filename)),
                        shell=True)

    self.run_subprocess(('%s %s pull /sdcard/%s %s' %
                         (adb_path, adb_device_arg, temp_filename, destination)
                        ),
                        shell=True)

    self.run_subprocess(('%s %s shell rm /sdcard/%s' %
                         (adb_path, adb_device_arg, temp_filename)),
                        shell=True)

  def run_android_apk(self, path='.', adb_device=None, wait=True,
                      end_match=None, echo_log=True, apk_missing_allowed=True):
    """Run an android apk on the given device.

    Args:
      path: Relative path from project directory to project to run.
      adb_device: The serial of the device to run the apk on. If None the only
        device connected will be used.
      wait: Optional argument to tell the function to wait until the process
        completes and dump the output.
      end_match: Optional compiled regex applied to logs. When a match is
        found, the process is considered finished (sets wait to False).
      echo_log: If set, prints the output from logcat.
      apk_missing_allowed: If set to False, raise an AdbError if the activity
        couldn't be started.

    Returns:
      String containing adb logcat output for the run.
      None if there is a manifest configuration problem.

    Raises:
      AdbError: If the activity wasn't started.
    """
    try:
      manifest = self.parse_manifest(path=path)
    except AndroidManifest.MissingActivityError as e:
      print >> sys.stderr, str(e)
      return
    main_activity_name = manifest.main_activity_name
    if '.' not in manifest.main_activity_name:
      main_activity_name = '.' + manifest.main_activity_name
    full_name = '%s/%s' % (manifest.package_name, main_activity_name)
    adb_path = self._find_binary(BuildEnvironment.ADB)
    device = self.check_adb_devices(adb_device=adb_device)
    adb_device_arg = self.get_adb_device_argument(adb_device=device.serial)

    print 'Launching %s on %s' % (full_name, self.get_adb_device_name(device))

    # Stop the app before reading logs.
    self.run_subprocess(('%s %s shell am force-stop %s' %
                         (adb_path, adb_device_arg, manifest.package_name)),
                        shell=True)

    # Clear logcat.
    self.run_subprocess(('%s %s logcat -c' % (adb_path, adb_device_arg)),
                        shell=True)

    out, _ = self.run_subprocess(('%s %s shell am start -n %s' %
                                  (adb_path, adb_device_arg, full_name)),
                                 shell=True,
                                 capture=True)

    if 'Error: Activity class {' + manifest.package_name in out:
      raise common.AdbError(out)

    end_match = end_match or re.compile(
        (r'.*(Displayed|Activity destroy timeout|'
         r'Force finishing activity).*%s.*' % manifest.package_name))

    # Build list of arguments for logcat.
    logcat_args = [adb_path]
    logcat_args.extend(adb_device_arg.split())
    logcat_args.append('logcat')
    if self.adb_logcat_args:
      logcat_args.extend([shlex.split(a)[0] for a in self.adb_logcat_args])

    # We only need to capture stdout so use subprocess directly rather than
    # run_subprocess() to simplify the process of reading the stdout pipe.
    process = subprocess.Popen(args=logcat_args,
                               bufsize=-1,
                               stdout=subprocess.PIPE)
    stdout = process.stdout
    logdata = []
    try:
      while wait or self.adb_logcat_monitor:
        line = stdout.readline()
        logdata.append(line)
        stripped_line = line.rstrip()
        if echo_log:
          print stripped_line
        if end_match.match(stripped_line):
          wait = False
    except KeyboardInterrupt:
      pass

    process.kill()

    return ''.join(logdata)

  def run_all(self, path='.', adb_device=None, exclude_dirs=None, wait=True,
              continue_on_failure=False, gtest=False):
    """Locate and run all Android APKs.

    This function recursively scans a directory tree for Android application
    projects and runs them on the specified device.

    Args:
      path: Path to search the search in, defaults to '.'
      adb_device: The device to run the APK on. If none it will use the
        only device connected.
      exclude_dirs: List of directory names to exclude from project
        detection (see find_projects() for more information).
      wait: Whether to wait for the application to start.
      continue_on_failure: Whether to continue trying to execute all
        projects if a failure occurs.
      gtest: Whether the target is a googletest and the output should be
        scraped for test failures.

    Returns:
      (retval, errmsg, failures) tuple of an integer return value suitable for
      returning to the invoking shell, an error string (if any) and a list
      of packages that failed to launch or None (on success).
    """
    retval = 0
    errmsg = None
    failures = []

    apk_dirs, unused_lib_dirs = self.find_projects(path=path,
                                                   exclude_dirs=exclude_dirs)

    for apk_dir in apk_dirs:
      try:
        logoutput = self.run_android_apk(path=apk_dir, adb_device=adb_device,
                                         wait=wait)
        if gtest:
          for line in logoutput.splitlines():
            if _GTEST_FAILED.match(line):
              raise common.Error(error_message=('Test %s failed.' % apk_dir))
      except common.Error as e:
        errmsg = 'buildutil error: %s' % e.error_message
        retval = e.error_code
        failures.append(apk_dir)
        if continue_on_failure:
          continue
        break

    return (retval, errmsg, failures)
