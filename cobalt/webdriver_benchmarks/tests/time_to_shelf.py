#!/usr/bin/python2
# Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
"""Calculate time to download YouTube TV app and initial layout of shelves."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import sys

# The parent directory is a module
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

# pylint: disable=C6204,C6203
import timer
import tv_testcase
import tv_testcase_util


class TimeToShelf(tv_testcase.TvTestCase):

  def setUp(self):
    # Override TvTestCase's setUp() so that blank startup can first be measured.
    pass

  def test_simple(self):
    """This test tries to measure the startup time for the YouTube TV page.

    Specifically, this test uses the Cobalt CVal Cobalt.Lifetime, which gets
    updated ~60Hz on a best effort basis and is in microseconds, to determine
    "timeToShelfBlankStartupTimeUs" and uses Timer to determine
    "timeToShelfTestTimeShelfDisplayMedianUs".
    """

    blank_startup_time_microseconds = self.get_cval('Cobalt.Lifetime')

    # Call TvTestCase's setUp() now that the blank startup time has been
    # measured.
    super(TimeToShelf, self).setUp()

    metrics_array = []
    for _ in range(10):
      with timer.Timer('TimeShelfDisplay') as t:
        self.load_tv()
      startup_time_microseconds = int(t.seconds_elapsed * 1000000)
      metrics_array.append(startup_time_microseconds)

    tv_testcase_util.record_test_result('timeToShelfBlankStartupTimeUs',
                                        blank_startup_time_microseconds)
    tv_testcase_util.record_test_result_median(
        'timeToShelfTestTimeShelfDisplayMedianUs', metrics_array)


if __name__ == '__main__':
  tv_testcase.main()
