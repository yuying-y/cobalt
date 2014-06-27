/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BASE_COMPILER_H_
#define BASE_COMPILER_H_

// Macro for hinting that an expression is likely to be true.
#if !defined(LIKELY)
#if defined(__GNUC__)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#else
#error unsupported compiler
// Uncomment below if you want a quick workaround.
// #define LIKELY(x) (x)
#endif  // defined(__GNUC__)
#endif  // !defined(LIKELY)

// Macro for hinting that an expression is likely to be false.
#if !defined(UNLIKELY)
#if defined(__GNUC__)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#error unsupported compiler
// Uncomment below if you want a quick workaround.
// #define UNLIKELY(x) (x)
#endif  // defined(__GNUC__)
#endif  // !defined(UNLIKELY)

#endif  // BASE_COMPILER_H_
