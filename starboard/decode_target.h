// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Module Overview: Starboard Decode Target module
//
// A target for decoding image and video data into. This corresponds roughly to
// an EGLImage, but that extension may not be fully supported on all GL
// platforms. SbDecodeTarget supports multi-plane targets. We need a mechanism
// for SbBlitter as well, and this is able to more-or-less unify the two.
//
// An SbDecodeTarget can be passed into any function which decodes video or
// image data. This allows the application to allocate fast graphics memory, and
// have decoding done directly into this memory, avoiding unnecessary memory
// copies, and also avoiding pushing data between CPU and GPU memory
// unnecessarily.
//
// #### SbDecodeTargetFormat
//
// SbDecodeTargets support several different formats that can be used to decode
// into and render from. Some formats may be easier to decode into, and others
// may be easier to render. Some may take less memory. Each decoder needs to
// support the SbDecodeTargetFormat passed into it, or the decode will produce
// an error. Each decoder provides a way to check if a given
// SbDecodeTargetFormat is supported by that decoder.
//
// #### SbDecodeTargetProvider
//
// Some components may need to acquire SbDecodeTargets compatible with a certain
// rendering context, which may need to be created on a particular thread. The
// SbDecodeTargetProvider is a way for the primary rendering context to provide
// an interface that can create SbDecodeTargets on demand by other system.
//
// The primary usage is likely to be the the SbPlayer implementation on some
// platforms.
//
// #### SbDecodeTarget Example
//
// Let's say there's an image decoder for .foo files:
//
//     bool SbImageDecodeFooSupportsFormat(SbDecodeTargetFormat format);
//     SbDecodeTarget SbImageDecodeFoo(void* data, int data_size,
//                                     SbDecodeTargetFormat format);
//
// First, the client should enumerate which SbDecodeTargetFormats are supported
// by that decoder.
//
//     SbDecodeTargetFormat kPreferredFormats[] = {
//         kSbDecodeTargetFormat3PlaneYUVI420,
//         kSbDecodeTargetFormat1PlaneRGBA,
//         kSbDecodeTargetFormat1PlaneBGRA,
//     };
//
//     SbDecodeTargetFormat format = kSbDecodeTargetFormatInvalid;
//     for (int i = 0; i < SB_ARRAY_SIZE_INT(kPreferredFormats); ++i) {
//       if (SbImageDecodeFooSupportsFormat(kPreferredFormats[i])) {
//         format = kPreferredFormats[i];
//         break;
//       }
//     }
//
// Now that the client has a format, it can create a decode target that it
// will use to decode the .foo file into. Let's assume format is
// kSbDecodeTargetFormat1PlaneRGBA, that we are on an EGL/GLES2 platform.
// Also, we won't do any error checking, to keep things even simpler.
//
//     SbDecodeTarget target = SbImageDecodeFoo(encoded_foo_data,
//                                              encoded_foo_data_size, format);
//
//     // If the decode works, you can get the texture out and render it.
//     GLuint texture =
//         SbDecodeTargetGetPlane(target, kSbDecodeTargetPlaneRGBA);
//

#ifndef STARBOARD_DECODE_TARGET_H_
#define STARBOARD_DECODE_TARGET_H_

#include "starboard/configuration.h"
#include "starboard/export.h"
#include "starboard/types.h"

#if SB_VERSION(3)

#if SB_HAS(BLITTER)
#include "starboard/blitter.h"
#elif SB_HAS(GLES2)  // SB_HAS(BLITTER)
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif  // SB_HAS(BLITTER)

#ifdef __cplusplus
extern "C" {
#endif

// --- Types -----------------------------------------------------------------

// Private structure representing a target for image data decoding.
typedef struct SbDecodeTargetPrivate SbDecodeTargetPrivate;

// A handle to a target for image data decoding.
typedef SbDecodeTargetPrivate* SbDecodeTarget;

// The list of all possible decoder target formats. An SbDecodeTarget consists
// of one or more planes of data, each plane corresponding with a surface. For
// some formats, different planes will be different sizes for the same
// dimensions.
//
// NOTE: For enumeration entries with an alpha component, the alpha will always
// be premultiplied unless otherwise explicitly specified.
typedef enum SbDecodeTargetFormat {
  // A decoder target format consisting of a single RGBA plane, in that channel
  // order.
  kSbDecodeTargetFormat1PlaneRGBA,

  // A decoder target format consisting of a single BGRA plane, in that channel
  // order.
  kSbDecodeTargetFormat1PlaneBGRA,

  // A decoder target format consisting of Y and interleaved UV planes, in that
  // plane and channel order.
  kSbDecodeTargetFormat2PlaneYUVNV12,

  // A decoder target format consisting of Y, U, and V planes, in that order.
  kSbDecodeTargetFormat3PlaneYUVI420,

  // An invalid decode target format.
  kSbDecodeTargetFormatInvalid,
} SbDecodeTargetFormat;

// All the planes supported by SbDecodeTarget.
typedef enum SbDecodeTargetPlane {
  // The RGBA plane for the RGBA format.
  kSbDecodeTargetPlaneRGBA = 0,

  // The BGRA plane for the BGRA format.
  kSbDecodeTargetPlaneBGRA = 0,

  // The Y plane for multi-plane YUV formats.
  kSbDecodeTargetPlaneY = 0,

  // The UV plane for 2-plane YUV formats.
  kSbDecodeTargetPlaneUV = 1,

  // The U plane for 3-plane YUV formats.
  kSbDecodeTargetPlaneU = 1,

  // The V plane for 3-plane YUV formats.
  kSbDecodeTargetPlaneV = 2,
} SbDecodeTargetPlane;

// A function that can produce an SbDecodeTarget of the given |format|, |width|,
// and |height|.
typedef SbDecodeTarget (*SbDecodeTargetAcquireFunction)(
    void* context,
    SbDecodeTargetFormat format,
    int width,
    int height);

// A function that can reclaim an SbDecodeTarget allocated with an
// SbDecodeTargetAcquireFunction.
typedef void (*SbDecodeTargetReleaseFunction)(void* context,
                                              SbDecodeTarget decode_target);

// An object that can acquire and release SbDecodeTarget instances.
typedef struct SbDecodeTargetProvider {
  // The function to acquire a new SbDecodeTarget from the provider.
  SbDecodeTargetAcquireFunction acquire;

  // The function to release an acquired SbDecodeTarget back to the provider.
  SbDecodeTargetReleaseFunction release;

  // |context| will be passed into every call to |acquire| and |release|.
  void* context;
} SbDecodeTargetProvider;

// --- Constants -------------------------------------------------------------

// Well-defined value for an invalid decode target handle.
#define kSbDecodeTargetInvalid ((SbDecodeTarget)NULL)

// --- Functions -------------------------------------------------------------

// Returns whether the given file handle is valid.
static SB_C_INLINE bool SbDecodeTargetIsValid(SbDecodeTarget handle) {
  return handle != kSbDecodeTargetInvalid;
}

// Returns whether a given format is valid.
static SB_C_INLINE bool SbDecodeTargetIsFormatValid(
    SbDecodeTargetFormat format) {
  return format != kSbDecodeTargetFormatInvalid;
}

#if SB_HAS(BLITTER)
// Creates a new SbBlitter-compatible SbDecodeTarget from one or more |planes|
// created from |display|.
//
// |format|: The format of the decode target being created.
// |planes|: An array of SbBlitterSurface handles to be bundled into an
// SbDecodeTarget. Must not be NULL. Is expected to have the same number of
// entries as the number of planes for |format|, in the order and size expected
// by that format.
SB_EXPORT SbDecodeTarget SbDecodeTargetCreate(SbDecodeTargetFormat format,
                                              SbBlitterSurface* planes);

// Gets the surface that represents the given plane.
SB_EXPORT SbBlitterSurface SbDecodeTargetGetPlane(SbDecodeTarget decode_target,
                                                  SbDecodeTargetPlane plane);
#elif SB_HAS(GLES2)  // SB_HAS(BLITTER)
// Creates a new EGL/GLES2-compatible SbDecodeTarget from one or more |planes|
// owned by |context|, created from |display|. Must be called from a thread
// where |context| is current.
//
// |display|: The EGLDisplay being targeted.
// |context|: The EGLContext used for this operation, or EGL_NO_CONTEXT if a
// context is not required.
// |format|: The format of the decode target being created.
// |planes|: An array of GLES Texture handles to be bundled into an
// SbDecodeTarget. Must not be NULL. Is expected to have the same number of
// entries as the number of planes for |format|, in the order and size expected
// by that format.
SB_EXPORT SbDecodeTarget SbDecodeTargetCreate(EGLDisplay display,
                                              EGLContext context,
                                              SbDecodeTargetFormat format,
                                              GLuint* planes);

// Gets the texture that represents the given plane.
SB_EXPORT GLuint SbDecodeTargetGetPlane(SbDecodeTarget decode_target,
                                        SbDecodeTargetPlane plane);

#else  // SB_HAS(BLITTER)

// Stub function for when graphics aren't enabled.  Always creates
// kSbDecodeTargetInvalid.
static SB_C_INLINE SbDecodeTarget
SbDecodeTargetCreate(SbDecodeTargetFormat format) {
  SB_UNREFERENCED_PARAMETER(format);
  return kSbDecodeTargetInvalid;
}

// Stub function for when graphics aren't enabled.  There is no concept of a
// plane, and |NULL| is always returned.
static SB_C_INLINE void* SbDecodeTargetGetPlane(SbDecodeTarget decode_target,
                                                SbDecodeTargetPlane plane) {
  SB_UNREFERENCED_PARAMETER(decode_target);
  SB_UNREFERENCED_PARAMETER(plane);
  return NULL;
}

#endif  // SB_HAS(BLITTER)

// Destroys the given SbDecodeTarget. Note that calling this function does NOT
// destroy the associated surfaces with it, in order to accommodate the case
// in which it is desirable for the lifetime of the surface to outlive the
// SbDecodeTarget that contains it. If the surfaces should be destroyed along
// with the SbDecodeTarget, then they should be extracted with
// |SbDecodeTargetGetPlane| and destroyed manually by the client.
SB_EXPORT void SbDecodeTargetDestroy(SbDecodeTarget decode_target);

// Gets the format that |decode_target| was created with.
SB_EXPORT SbDecodeTargetFormat
SbDecodeTargetGetFormat(SbDecodeTarget decode_target);

// Gets whether |decode_target| is opaque or not.  The underlying source of
// this value is expected to be properly maintained by the Starboard
// implementation.  So, for example, if an opaque only image type were decoded
// into an SbDecodeTarget, then the implementation would configure things in
// such a way that this function would return true.  By opaque, it is meant
// that all alpha values are guaranteed to be 255, if |decode_target| is of a
// format that has alpha values.  If |decode_target| is of a format that does
// not have alpha values, then this function should return |true|.
SB_EXPORT bool SbDecodeTargetIsOpaque(SbDecodeTarget decode_target);

// Inline convenience function to acquire an SbDecodeTarget of type |format|,
// |width|, and |height| from |provider|.
static SB_C_INLINE SbDecodeTarget
SbDecodeTargetAcquireFromProvider(SbDecodeTargetProvider* provider,
                                  SbDecodeTargetFormat format,
                                  int width,
                                  int height) {
  return provider->acquire(provider->context, format, width, height);
}

// Inline convenience function to release |decode_target| back to |provider|.
static SB_C_INLINE void SbDecodeTargetReleaseToProvider(
    SbDecodeTargetProvider* provider,
    SbDecodeTarget decode_target) {
  provider->release(provider->context, decode_target);
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SB_VERSION(3)

#endif  // STARBOARD_DECODE_TARGET_H_
