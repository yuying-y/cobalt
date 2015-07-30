# Copyright 2015 Google Inc. All Rights Reserved.
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

{
  'variables': {
    'cobalt_code': 1,
  },
  'targets': [
    {
      'target_name': 'dom_parser',
      'type': 'static_library',
      'sources': [
        'html_decoder.cc',
        'html_decoder.h',
        'libxml_html_parser_wrapper.cc',
        'libxml_html_parser_wrapper.h',
        'libxml_parser_wrapper.cc',
        'libxml_parser_wrapper.h',
        'libxml_xml_parser_wrapper.cc',
        'libxml_xml_parser_wrapper.h',
        'parser.cc',
        'parser.h',
        'xml_decoder.cc',
        'xml_decoder.h',
      ],
      'dependencies': [
        '<(DEPTH)/cobalt/base/base.gyp:base',
        '<(DEPTH)/cobalt/dom/dom.gyp:dom',
        '<(DEPTH)/cobalt/loader/loader.gyp:loader',
        '<(DEPTH)/third_party/libxml/libxml.gyp:libxml',
      ],
    },

    {
      'target_name': 'dom_parser_test',
      'type': '<(gtest_target_type)',
      'sources': [
        'html_decoder_test.cc',
        'xml_decoder_test.cc',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:run_all_unittests',
        '<(DEPTH)/cobalt/dom/dom.gyp:dom_testing',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        'dom_parser',
      ],
      'actions': [
        {
          'action_name': 'dom_copy_test_data',
          'variables': {
            'input_files': [
              '<(DEPTH)/cobalt/dom_parser/testdata/',
            ],
            'output_dir': 'cobalt/dom_parser/testdata/',
          },
          'includes': [ '../build/copy_data.gypi' ],
        },
      ],
    },

    {
      'target_name': 'dom_parser_test_deploy',
      'type': 'none',
      'dependencies': [
        'dom_parser_test',
      ],
      'variables': {
        'executable_name': 'dom_parse_test',
      },
      'includes': [ '../build/deploy.gypi' ],
    },
  ],
}
