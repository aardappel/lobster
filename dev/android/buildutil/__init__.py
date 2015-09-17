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

"""Build utilities for use in automated build scripts and tools.

This module is a set of functions that can be used to help implement tools
to perform typical build tasks. The main focus is to enable turnkey building
both for users and also for continuous integration builds and tests.

Simple usage example:

  import buildutil.linux

  def main():
    env = buildutil.linux.BuildEnvironment(buildutil.linux.build_defaults())

    env.run_make()
    env.make_archive(['bin', 'lib', 'include'], 'output.zip')


Please see examples/buildutil/* for more comprehensive uses.
"""
