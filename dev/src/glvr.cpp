// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
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

#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "glinterface.h"
#include "glincludes.h"

#include "sdlinterface.h"
#include "sdlincludes.h"

#ifdef PLATFORM_VR

#include "openvr.h"

vr::IVRSystem *vrsys = nullptr;
vr::IVRRenderModels *vrmodels = nullptr;
vr::TrackedDevicePose_t trackeddeviceposes[vr::k_unMaxTrackedDeviceCount];

string GetTrackedDeviceString(vr::TrackedDeviceIndex_t device, vr::TrackedDeviceProperty prop)
{
    assert(vrsys);
    uint32_t buflen = vrsys->GetStringTrackedDeviceProperty(device, prop, nullptr, 0, nullptr);
    if(buflen == 0) return "";

    char *buf = new char[buflen];
    buflen = vrsys->GetStringTrackedDeviceProperty(device, prop, buf, buflen, nullptr);
    std::string s = buf;
    delete [] buf;
    return s;
}

#endif  // PLATFORM_VR

int2 rtsize = int2_0;
uint mstex[2] = { 0, 0 };
uint retex[2] = { 0, 0 };

void VRShutDown()
{
    #ifdef PLATFORM_VR
    
    vrsys = NULL;
    vr::VR_Shutdown();

    for (int i = 0; i < 2; i++)
    {
        DeleteTexture(mstex[i]);
        DeleteTexture(retex[i]);
    }

    #endif  // PLATFORM_VR
}

bool VRInit()
{
    #ifdef PLATFORM_VR

    if (vrsys) return true;

    if (!vr::VR_IsHmdPresent()) return false;

    vr::EVRInitError err = vr::VRInitError_None;
    vrsys = vr::VR_Init(&err, vr::VRApplication_Scene);

    if (err != vr::VRInitError_None)
    {
        vrsys = nullptr;
        Output(OUTPUT_ERROR, "VR system init failed: %s", vr::VR_GetVRInitErrorAsEnglishDescription(err));
        return false;
    }

    vrsys->GetRecommendedRenderTargetSize((uint *)&rtsize.x_mut(), (uint *)&rtsize.y_mut());

    auto devicename = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
    auto displayname = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
    Output(OUTPUT_INFO, "VR running on device: \"%s\", display: \"%s\", rt size: (%d, %d)",
        devicename.c_str(), displayname.c_str(), rtsize.x(), rtsize.y());

    vrmodels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &err);
    if(!vrmodels)
    {
        VRShutDown();
        Output(OUTPUT_ERROR, "VR get render models failed: %s", vr::VR_GetVRInitErrorAsEnglishDescription(err));
        return false;
    }

    if (!vr::VRCompositor())
    {
        VRShutDown();
        Output(OUTPUT_ERROR, "VR compositor failed to initialize");
        return false;
    }

    // Get focus?
    vr::VRCompositor()->WaitGetPoses(trackeddeviceposes, vr::k_unMaxTrackedDeviceCount, NULL, 0);

    return true;

    #else

    return false;

    #endif  // PLATFORM_VR
}

float4x4 FromOpenVR(const vr::HmdMatrix44_t &mat) { return float4x4(&mat.m[0][0]).transpose(); }

float4x4 FromOpenVR(const vr::HmdMatrix34_t &mat)
{
    return float4x4(float4(&mat.m[0][0]),
                    float4(&mat.m[1][0]),
                    float4(&mat.m[2][0]),
                    float4(0, 0, 0, 1)).transpose();  // FIXME: simplify
}

void VREye(int eye, float znear, float zfar)
{
    #ifdef PLATFORM_VR

    if (!vrsys) return;

    glEnable(GL_MULTISAMPLE);

    auto retf = TF_CLAMP | TF_NOMIPMAP;
    auto mstf = retf | TF_MULTISAMPLE;
    if (!mstex[eye]) mstex[eye] = CreateBlankTexture(rtsize, float4_0, mstf);
    if (!retex[eye]) retex[eye] = CreateBlankTexture(rtsize, float4_0, retf);

    SwitchToFrameBuffer(mstex[eye], rtsize, true, mstf, retex[eye]);

    auto proj = FromOpenVR(vrsys->GetProjectionMatrix((vr::EVREye)eye, znear, zfar, vr::API_OpenGL));
    Set3DMode(80, 1, znear, zfar);
    view2clip = proj;  // Override the projection set by Set3DMode

    auto eye2head = FromOpenVR(vrsys->GetEyeToHeadTransform((vr::EVREye)eye));
    auto vrview = eye2head;
    if (trackeddeviceposes[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
    {
        auto hmdpose = FromOpenVR(trackeddeviceposes[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);
        vrview = hmdpose * vrview;
    }
    AppendTransform(invert(vrview), vrview);

    #endif  // PLATFORM_VR
}

void VRFinish()
{
    #ifdef PLATFORM_VR

    if (!vrsys) return;

    SwitchToFrameBuffer(0, GetScreenSize(), false, 0, 0);

    glDisable(GL_MULTISAMPLE);

    for (int i = 0; i < 2; i++)
    {
        vr::Texture_t vrtex = { (void *)retex[i], vr::API_OpenGL, vr::ColorSpace_Gamma };
        auto err = vr::VRCompositor()->Submit((vr::EVREye)i, &vrtex);
        (void)err;
        assert(!err);
    }
    
    vr::VRCompositor()->PostPresentHandoff();

    glFlush();

    auto err = vr::VRCompositor()->WaitGetPoses(trackeddeviceposes, vr::k_unMaxTrackedDeviceCount, NULL, 0);
    (void)err;
    assert(!err);

    #endif  // PLATFORM_VR
}

using namespace lobster;

void AddVR()
{
    STARTDECL(vr_init) ()
    {
        return Value(VRInit());
    }
    ENDDECL0(vr_init, "", "", "I",
        "initializes VR mode. returns true if a hmd was found and initialized");

    STARTDECL(vr_starteye) (Value &isright, Value &znear, Value &zfar)
    {
        VREye(isright.True(), znear.fval(), zfar.fval());
        return Value();
    }
    ENDDECL3(vr_starteye, "isright,znear,zfar", "IFF", "",
        "starts rendering for an eye. call for each eye, followed by drawing the world as normal."
        " replaces gl_perspective");

    STARTDECL(vr_finish) ()
    {
        VRFinish();
        return Value();
    }
    ENDDECL0(vr_finish, "", "", "",
        "finishes vr rendering by compositing (and distorting) both eye renders to the screen");

    STARTDECL(vr_geteyetex) (Value &isright)
    {
        return Value((int)retex[isright.True()]);
    }
    ENDDECL1(vr_geteyetex, "isright", "I", "I",
        "returns the texture for an eye. call after vr_finish. can be used to render the non-VR display");

}

