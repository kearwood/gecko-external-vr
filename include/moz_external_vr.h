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
#endif // MOZILLA_INTERNAL_API

namespace mozilla {
#ifdef MOZILLA_INTERNAL_API
namespace dom {
  enum class GamepadHand : uint8_t;
}
#endif //  MOZILLA_INTERNAL_API
namespace gfx {

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
static const int kVRControllerNameMaxLen = 256;
static const int kVRControllerMaxCount = 16;
static const int kVRControllerMaxTriggers = 16;
static const int kVRControllerMaxAxis = 16;

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

enum class ControllerHand : uint8_t {
  _empty,
  Left,
  Right,
  EndGuard_
};

#endif // ifndef MOZILLA_INTERNAL_API

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

struct VRDisplayState
{
  enum Eye {
    Eye_Left,
    Eye_Right,
    NumEyes
  };

  char mDisplayName[kVRDisplayNameMaxLen];
  VRDisplayCapabilityFlags mCapabilityFlags;
  VRFieldOfView mEyeFOV[VRDisplayState::NumEyes];
  Point3D mEyeTranslation[VRDisplayState::NumEyes];
  IntSize mEyeResolution;
  bool mIsConnected;
  bool mIsMounted;
  Size mStageSize;
  // FINDME! TODO! HACK! This may be unsafe if not consistently packed or Matrix4x4 is not a trivial type:
#ifdef MOZILLA_INTERNAL_API
  Matrix4x4 mSittingToStandingTransform;
#else
  float mSittingToStandingTransform[16];
#endif
};

struct VRControllerState
{
  char mControllerName[kVRControllerNameMaxLen];
#ifdef MOZILLA_INTERNAL_API
  dom::GamepadHand mHand;
#else
  ControllerHand mHand;
#endif
  uint32_t mNumButtons;
  uint32_t mNumAxes;
  uint32_t mNumTriggers;
  uint32_t mNumHaptics;
  // The current button pressed bit of button mask.
  uint64_t mButtonPressed;
  // The current button touched bit of button mask.
  uint64_t mButtonTouched;
  float mTriggerValue[kVRControllerMaxTriggers];
  float mAxisValue[kVRControllerMaxAxis];
};

struct VRSystemState
{
  VRDisplayState displayState;
  VRHMDSensorState sensorState;
  VRControllerState controllerState[kVRControllerMaxCount];
};

struct VRExternalShmem
{
  int64_t generationA;
  VRSystemState state;
  int64_t generationB;
};

} // namespace gfx
} // namespace mozilla

#endif /* GFX_VR_EXTERNAL_API_H */
