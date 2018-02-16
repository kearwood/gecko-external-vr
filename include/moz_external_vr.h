/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GFX_VR_EXTERNAL_API_H
#define GFX_VR_EXTERNAL_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef MOZILLA_INTERNAL_API
#include "mozilla/TypedEnumBits.h"
#include "mozilla/gfx/2D.h"
#endif

namespace mozilla {
namespace gfx {

// The maximum number of frames of latency that we would expect before we
// should give up applying pose prediction.
// If latency is greater than one second, then the experience is not likely
// to be corrected by pose prediction.  Setting this value too
// high may result in unnecessary memory allocation.
// As the current fastest refresh rate is 90hz, 100 is selected as a
// conservative value.
static const int kVRMaxLatencyFrames = 100;

// We assign VR presentations to groups with a bitmask.
// Currently, we will only display either content or chrome.
// Later, we will have more groups to support VR home spaces and
// multitasking environments.
// These values are not exposed to regular content and only affect
// chrome-only API's.  They may be changed at any time.
static const uint32_t kVRGroupNone = 0;
static const uint32_t kVRGroupContent = 1 << 0;
static const uint32_t kVRGroupChrome = 1 << 1;
static const uint32_t kVRGroupAll = 0xffffffff;

static const int kVRDisplayNameMaxLen = 256;

enum class VRDeviceType : uint16_t {
  Oculus,
  OpenVR,
  OSVR,
  GVR,
  Puppet,
  External,
  NumVRDeviceTypes
};

enum class VRDisplayCapabilityFlags : uint16_t {
  Cap_None = 0,
  /**
   * Cap_Position is set if the VRDisplay is capable of tracking its position.
   */
  Cap_Position = 1 << 1,
  /**
    * Cap_Orientation is set if the VRDisplay is capable of tracking its orientation.
    */
  Cap_Orientation = 1 << 2,
  /**
   * Cap_Present is set if the VRDisplay is capable of presenting content to an
   * HMD or similar device.  Can be used to indicate "magic window" devices that
   * are capable of 6DoF tracking but for which requestPresent is not meaningful.
   * If false then calls to requestPresent should always fail, and
   * getEyeParameters should return null.
   */
  Cap_Present = 1 << 3,
  /**
   * Cap_External is set if the VRDisplay is separate from the device's
   * primary display. If presenting VR content will obscure
   * other content on the device, this should be un-set. When
   * un-set, the application should not attempt to mirror VR content
   * or update non-VR UI because that content will not be visible.
   */
  Cap_External = 1 << 4,
  /**
   * Cap_AngularAcceleration is set if the VRDisplay is capable of tracking its
   * angular acceleration.
   */
  Cap_AngularAcceleration = 1 << 5,
  /**
   * Cap_LinearAcceleration is set if the VRDisplay is capable of tracking its
   * linear acceleration.
   */
  Cap_LinearAcceleration = 1 << 6,
  /**
   * Cap_StageParameters is set if the VRDisplay is capable of room scale VR
   * and can report the StageParameters to describe the space.
   */
  Cap_StageParameters = 1 << 7,
  /**
   * Cap_MountDetection is set if the VRDisplay is capable of sensing when the
   * user is wearing the device.
   */
  Cap_MountDetection = 1 << 8,
  /**
   * Cap_All used for validity checking during IPC serialization
   */
  Cap_All = (1 << 9) - 1
};

#ifdef MOZILLA_INTERNAL_API
MOZ_MAKE_ENUM_CLASS_BITWISE_OPERATORS(VRDisplayCapabilityFlags)
#endif // MOZILLA_INTERNAL_API

struct VRHMDSensorState {
  int64_t inputFrameID;
  double timestamp;
  VRDisplayCapabilityFlags flags;

  // These members will only change with inputFrameID:
  float orientation[4];
  float position[3];
  float leftViewMatrix[16];
  float rightViewMatrix[16];
  float angularVelocity[3];
  float angularAcceleration[3];
  float linearVelocity[3];
  float linearAcceleration[3];

#ifdef MOZILLA_INTERNAL_API

  void Clear() {
    memset(this, 0, sizeof(VRHMDSensorState));
  }

  bool operator==(const VRHMDSensorState& other) const {
    return inputFrameID == other.inputFrameID &&
           timestamp == other.timestamp;
  }

  bool operator!=(const VRHMDSensorState& other) const {
    return !(*this == other);
  }

  void CalcViewMatrices(const gfx::Matrix4x4* aHeadToEyeTransforms);

#endif // MOZILLA_INTERNAL_API
};

struct VRFieldOfView {
  double upDegrees;
  double rightDegrees;
  double downDegrees;
  double leftDegrees;

#ifdef MOZILLA_INTERNAL_API

  VRFieldOfView() = default;
  VRFieldOfView(double up, double right, double down, double left)
    : upDegrees(up), rightDegrees(right), downDegrees(down), leftDegrees(left)
  {}

  void SetFromTanRadians(double up, double right, double down, double left)
  {
    upDegrees = atan(up) * 180.0 / M_PI;
    rightDegrees = atan(right) * 180.0 / M_PI;
    downDegrees = atan(down) * 180.0 / M_PI;
    leftDegrees = atan(left) * 180.0 / M_PI;
  }

  bool operator==(const VRFieldOfView& other) const {
    return other.upDegrees == upDegrees &&
           other.downDegrees == downDegrees &&
           other.rightDegrees == rightDegrees &&
           other.leftDegrees == leftDegrees;
  }

  bool operator!=(const VRFieldOfView& other) const {
    return !(*this == other);
  }

  bool IsZero() const {
    return upDegrees == 0.0 ||
      rightDegrees == 0.0 ||
      downDegrees == 0.0 ||
      leftDegrees == 0.0;
  }

  Matrix4x4 ConstructProjectionMatrix(float zNear, float zFar, bool rightHanded) const;

#endif // MOZILLA_INTERNAL_API

};

#ifndef MOZILLA_INTERNAL_API

struct Point3D
{
  float x;
  float y;
  float z;
};

struct IntSize
{
  int32_t width;
  int32_t height;
};

struct Size
{
  float width;
  float height;
};

#endif // ifndef MOZILLA_INTERNAL_API

struct VRDisplayInfo
{
  enum Eye {
    Eye_Left,
    Eye_Right,
    NumEyes
  };

  uint32_t mDisplayID;
  VRDeviceType mType;
  char mDisplayName[kVRDisplayNameMaxLen];
  VRDisplayCapabilityFlags mCapabilityFlags;
  VRFieldOfView mEyeFOV[VRDisplayInfo::NumEyes];
  Point3D mEyeTranslation[VRDisplayInfo::NumEyes];
  IntSize mEyeResolution;
  bool mIsConnected;
  bool mIsMounted;
  uint32_t mPresentingGroups;
  uint32_t mGroupMask;
  Size mStageSize;
  // FINDME! TODO! HACK! This may be unsafe if not consistently packed or Matrix4x4 is not a trivial type:
#ifdef MOZILLA_INTERNAL_API
  Matrix4x4 mSittingToStandingTransform;
#else
  float mSittingToStandingTransform[16];
#endif
  uint64_t mFrameId;
  uint32_t mPresentingGeneration;
  VRHMDSensorState mLastSensorState[kVRMaxLatencyFrames];

#ifdef MOZILLA_INTERNAL_API

  VRDeviceType GetType() const { return mType; }
  uint32_t GetDisplayID() const { return mDisplayID; }
  const char* GetDisplayName() const { return mDisplayName; }
  VRDisplayCapabilityFlags GetCapabilities() const { return mCapabilityFlags; }

  const IntSize& SuggestedEyeResolution() const { return mEyeResolution; }
  const Point3D& GetEyeTranslation(uint32_t whichEye) const { return mEyeTranslation[whichEye]; }
  const VRFieldOfView& GetEyeFOV(uint32_t whichEye) const { return mEyeFOV[whichEye]; }
  bool GetIsConnected() const { return mIsConnected; }
  bool GetIsMounted() const { return mIsMounted; }
  uint32_t GetPresentingGroups() const { return mPresentingGroups; }
  uint32_t GetGroupMask() const { return mGroupMask; }
  const Size& GetStageSize() const { return mStageSize; }
  const Matrix4x4& GetSittingToStandingTransform() const { return mSittingToStandingTransform; }
  uint64_t GetFrameId() const { return mFrameId; }

  bool operator==(const VRDisplayInfo& other) const {
    for (size_t i = 0; i < kVRMaxLatencyFrames; i++) {
      if (mLastSensorState[i] != other.mLastSensorState[i]) {
        return false;
      }
    }
    return mType == other.mType &&
           mDisplayID == other.mDisplayID &&
           strncmp(mDisplayName, other.mDisplayName, kVRDisplayNameMaxLen) == 0 &&
           mCapabilityFlags == other.mCapabilityFlags &&
           mEyeResolution == other.mEyeResolution &&
           mIsConnected == other.mIsConnected &&
           mIsMounted == other.mIsMounted &&
           mPresentingGroups == other.mPresentingGroups &&
           mGroupMask == other.mGroupMask &&
           mEyeFOV[0] == other.mEyeFOV[0] &&
           mEyeFOV[1] == other.mEyeFOV[1] &&
           mEyeTranslation[0] == other.mEyeTranslation[0] &&
           mEyeTranslation[1] == other.mEyeTranslation[1] &&
           mStageSize == other.mStageSize &&
           mSittingToStandingTransform == other.mSittingToStandingTransform &&
           mFrameId == other.mFrameId &&
           mPresentingGeneration == other.mPresentingGeneration;
  }

  bool operator!=(const VRDisplayInfo& other) const {
    return !(*this == other);
  }

  const VRHMDSensorState& GetSensorState() const
  {
    return mLastSensorState[mFrameId % kVRMaxLatencyFrames];
  }

#endif // MOZILLA_INTERNAL_API
};

struct VRExternalShmem
{
  int64_t generationA;
  VRDisplayInfo displayInfo;
  int64_t generationB;
};

} // namespace gfx
} // namespace mozilla

#endif /* GFX_VR_EXTERNAL_API_H */
