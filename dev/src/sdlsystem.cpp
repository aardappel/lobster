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

#include "lobster/stdafx.h"

#include "lobster/vmdata.h"
#include "lobster/sdlincludes.h"
#include "lobster/sdlinterface.h"
#include "lobster/glinterface.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4244)
#endif
#include "stb/stb_image_write.h"
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

SDL_Window *_sdl_window = nullptr;
SDL_GLContext _sdl_context = nullptr;

SDL_Window *_sdl_debugger_window = nullptr;
SDL_GLContext _sdl_debugger_context = nullptr;


/*
// FIXME: document this, especially the ones containing spaces.

mouse1 mouse2 mouse3...
backspace tab clear return pause escape space delete
! " # $ & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ [ \ ] ^ _ `
a b c d e f g h i j k l m n o p q r s t u v w x y z
keypad 0 keypad 1 keypad 2 keypad 3 keypad 4 keypad 5 keypad 6 keypad 7 keypad 8 keypad 9
keypad . keypad / keypad * keypad - keypad +
enter equals up down right left insert home end page up page down
f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15
numlock caps lock scroll lock right shift left shift right ctrl left ctrl right alt left alt
right meta left meta left super right super alt gr compose help print screen sys req break

For controllers (all starting with controller_):

a b x y back guide start leftstick rightstick leftshoulder rightshoulder
dpup dpdown dpleft dpright misc1 paddle1 paddle2 paddle3 paddle4 touchpad

joy0 and up give raw joystick values with no mapping.
*/


struct KeyState {
    int64_t frames_down = 0;
    int64_t frames_up = 2;  // Unpressed at initialization: has been up for more than 1 frame.
    bool repeat = false;

    double lasttime[2] = { -1, -1 };
    int2 lastpos[2] = { int2(-1, -1), int2(-1, -1) };

    void Reset() {
        frames_down = 0;
        frames_up = 2;
        repeat = false;
    }

    void Set(bool on) {
        if (on) {
            frames_down = 1;
            if (frames_up > 1) frames_up = 0;  // Not in this frame, so turn off.
        } else {
            frames_up = 1;
            if (frames_down > 1) frames_down = 0;  // Not in this frame, so turn off.
        }
    }

    void FrameAdvance() {
        if (frames_up) {
            frames_up++;
            frames_down = 0;  // In case it was down+up last frame.
        }
        if (frames_down) {
            frames_down++;
            // This situation doesn't occur, a single frame up+down will get interpreted as down+up above.
            // Hopefully that will be much more rare than single frame down+up :)
            assert(!frames_up);
        }
        repeat = false;
    }

    pair<int64_t, int64_t> State() {
        return { frames_down, frames_up };
    }

    pair<int64_t, int64_t> Prev() {
        return { std::max(int64_t(0), frames_down - 1),
                 std::max(int64_t(0), frames_up - 1) };
    }
};

map<string, KeyState, less<>> keymap;

TextInput textinput;

int mousewheeldelta = 0;

int skipmousemotion = 3;

double frametime = 1.0f / 60.0f, lasttime = 0;
uint64_t timefreq = 0, timestart = 0;
int frames = 0;
vector<float> frametimelog;
double target_frametime = 0.001;
double last_sleep = 0.0;

int2 screensize = int2_0;
int2 inputscale = int2_1;

bool fullscreen = false;
bool cursor = true;
int cursorx = 0, cursory = 0;
bool landscape = true;
bool minimized = false;
bool noninteractivetestmode = false;

const int MAXAXES = 16;
float joyaxes[MAXAXES] = { 0 };
float controller_axes[SDL_GAMEPAD_AXIS_COUNT] = { 0 };

struct Finger {
    SDL_FingerID id;
    int2 mousepos;
    int2 mousedelta;
    bool used;

    Finger() : id(0), mousepos(-1), mousedelta(0), used(false) {};
};

const int MAXFINGERS = 10;
Finger fingers[MAXFINGERS];


void updatebutton(string &name, bool on, int posfinger, bool repeat) {
    auto &ks = keymap[name];
    if (!repeat) {
        ks.Set(on);
        ks.lasttime[on] = lasttime;
        ks.lastpos[on] = fingers[posfinger].mousepos;
    }
    ks.repeat = repeat;
}

void updatemousebutton(int button, int finger, bool on) {
    string name = "mouse";
    name += '0' + (char)button;
    if (finger) name += '0' + (char)finger;
    updatebutton(name, on, finger, false);
}

void clearfingers(bool delta) {
    for (auto &f : fingers) (delta ? f.mousedelta : f.mousepos) = int2(0);
}

int findfinger(SDL_FingerID id, bool remove) {
    for (auto &f : fingers) if (f.id == id && f.used) {
        if (remove) {
            // would be more correct to clear mouse position here, but that doesn't work with delayed touch..
            // would have to delay it too
            f.used = false;
        }
        return int(&f - fingers);
    }
    if (remove) return MAXFINGERS - 1; // FIXME: this is masking a bug...
    assert(!remove);
    for (auto &f : fingers) if (!f.used) {
        f.id = id;
        f.used = true;
        return int(&f - fingers);
    }
    assert(0);
    return 0;
}

const int2 &GetFinger(int i, bool delta) {
    auto &f = fingers[max(min(i, MAXFINGERS - 1), 0)];
    return delta ? f.mousedelta : f.mousepos;
}

float GetJoyAxis(int i) {
    return joyaxes[max(min(i, MAXAXES - 1), 0)];
}

float GetControllerAxis(int i) {
    return controller_axes[max(min(i, SDL_GAMEPAD_AXIS_COUNT - 1), 0)];
}

int updatedragpos(SDL_TouchFingerEvent &e, Uint32 et) {
    int num_touch_fingers;
    auto *touch_fingers = SDL_GetTouchFingers(e.touchID, &num_touch_fingers);
    //assert(num_touch_fingers && e.fingerID < num_touch_fingers);
    for (int i = 0; i < num_touch_fingers; i++) {
        auto *finger = touch_fingers[i];
        if (finger->id == e.fingerID) {
            // this is a bit clumsy as SDL has a list of fingers and so do we, but they work a bit differently
            int j = findfinger(e.fingerID, et == SDL_EVENT_FINGER_UP);
            auto &f = fingers[j];
            auto ep = float2(e.x, e.y);
            auto ed = float2(e.dx, e.dy);
            auto xy = ep * float2(screensize);

            // FIXME: converting back to int coords even though touch theoretically may have higher res
            f.mousepos = int2(xy * float2(inputscale));
            f.mousedelta += int2(ed * float2(screensize));
            return j;
        }
    }
    //assert(0);
    return 0;
}

string dropped_file;
string &GetDroppedFile() { return dropped_file; }

string SDLError(const char *msg) {
    string s = string_view(msg) + ": " + SDL_GetError();
    LOG_WARN(s);
    SDLShutdown();
    return s;
}

bool SDLHandleAppEvents(void * /*userdata*/, SDL_Event *event) {
    // NOTE: This function only called on mobile devices it appears.
    switch (event->type) {
        case SDL_EVENT_TERMINATING:
            /* Terminate the app.
             Shut everything down before returning from this function.
             */
            return false;
        case SDL_EVENT_LOW_MEMORY:
            /* You will get this when your app is paused and iOS wants more memory.
             Release as much memory as possible.
             */
            return false;
        case SDL_EVENT_WILL_ENTER_BACKGROUND:
            LOG_DEBUG("SDL_EVENT_WILL_ENTER_BACKGROUND");
            minimized = true;
            /* Prepare your app to go into the background.  Stop loops, etc.
             This gets called when the user hits the home button, or gets a call.
             */
            return false;
        case SDL_EVENT_DID_ENTER_BACKGROUND:
            LOG_DEBUG("SDL_EVENT_DID_ENTER_BACKGROUND");
            /* This will get called if the user accepted whatever sent your app to the background.
             If the user got a phone call and canceled it,
             you'll instead get an SDL_APP_DIDENTERFOREGROUND event and restart your loops.
             When you get this, you have 5 seconds to save all your state or the app will be terminated.
             Your app is NOT active at this point.
             */
            return false;
        case SDL_EVENT_WILL_ENTER_FOREGROUND:
            LOG_DEBUG("SDL_EVENT_WILL_ENTER_FOREGROUND");
            /* This call happens when your app is coming back to the foreground.
             Restore all your state here.
             */
            return false;
        case SDL_EVENT_DID_ENTER_FOREGROUND:
            LOG_DEBUG("SDL_EVENT_DID_ENTER_FOREGROUND");
            /* Restart your loops here.
             Your app is interactive and getting CPU again.
             */
            minimized = false;
            return false;
        default:
            /* No special processing, add it to the event queue */
            return true;
    }
}

const int2 &GetScreenSize() { return screensize; }

void ScreenSizeChanged() {
    int2 inputsize = int2_0;
    SDL_GetWindowSize(_sdl_window, &inputsize.x, &inputsize.y);
    SDL_GetWindowSizeInPixels(_sdl_window, &screensize.x, &screensize.y);
    inputscale = screensize / inputsize;
}

SDL_Gamepad *find_controller() {
    int count;
    auto *joystick_ids = SDL_GetJoysticks(&count);
    for (int i = 0; i < count; i++) {
        if (SDL_IsGamepad(joystick_ids[i])) {
            return SDL_OpenGamepad(joystick_ids[i]);
        }
    }
    return nullptr;
}
SDL_Gamepad *controller = nullptr;

#ifdef PLATFORM_ES3
int gl_major = 3, gl_minor = 0;
#else
int gl_major = 3, gl_minor = 2;
string glslversion = "150";
#endif
void SDLRequireGLVersion(int major, int minor) {
    #ifdef PLATFORM_WINNIX
        gl_major = major;
        gl_minor = minor;
        glslversion = cat(major, minor, "0");
    #endif
};

int2 DPIAwareScreenSize(int2 desired_screensize) {
    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    if (scale == 0.f) {
        LOG_ERROR("SDL_GetDisplayContentScale: ", SDL_GetError());
        scale = 1.f;
    }
    return int2(float2(desired_screensize) * scale);
}

string SDLInit(string_view_nt title, const int2 &desired_screensize, InitFlags flags, int samples) {
    MakeDPIAware();
    TextToSpeechInit();  // Needs to be before SDL_Init because COINITBASE_MULTITHREADED
    // SDL_SetMainReady();
    if (!SDL_Init(SDL_INIT_VIDEO /* | SDL_INIT_AUDIO*/)) {
        return SDLError("Unable to initialize SDL");
    }

    SDL_SetEventFilter(SDLHandleAppEvents, nullptr);

    LOG_INFO("SDL initialized...");

    SDL_SetLogPriorities(SDL_LOG_PRIORITY_WARN);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor);
    #ifdef PLATFORM_ES3
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    #else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        #if defined(__APPLE__) || defined(_WIN32)
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, samples > 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);
        #endif
    #endif

    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);      // set this if we're in 2D mode for speed on mobile?
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);    // because we redraw the screen each frame
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    #ifndef __EMSCRIPTEN__ // FIXME: https://github.com/emscripten-ports/SDL2/issues/86
        if (flags & INIT_LINEAR_COLOR) SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    #endif

    #ifdef _DEBUG
        // Hopefully get some more validation out of OpenGL.
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    #endif

    LOG_INFO("SDL about to figure out display mode...");

    // FIXME: for emscripten, this picks screen size, not browser window size, and doesn't resize.
    #ifdef PLATFORM_ES3
        landscape = desired_screensize.x >= desired_screensize.y;
        int display_mode_count;
        SDL_DisplayMode **modes =
            SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &display_mode_count);
        screensize = int2(320, 200);
        for (int i = 0; i < display_mode_count; i++) {
            SDL_DisplayMode *mode = modes[i];
            LOG_INFO("mode: ", mode->w, " ", mode->h);
            if (landscape ? mode->w > screensize.x : mode->h > screensize.y) {
                screensize = int2(mode->w, mode->h);
            }
        }
        LOG_INFO("chosen resolution: ", screensize.x, " ", screensize.y);
        LOG_INFO("SDL about to create window...");
        auto wflags = SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
        #ifdef __EMSCRIPTEN__
            wflags |= SDL_WINDOW_RESIZABLE;
        #endif
        _sdl_window = SDL_CreateWindow(title.c_str(), screensize.x, screensize.y, wflags);
        LOG_INFO(_sdl_window ? "SDL window passed..." : "SDL window FAILED...");
        if (landscape) SDL_SetHint("SDL_HINT_ORIENTATIONS", "LandscapeLeft LandscapeRight");
    #else
        screensize = DPIAwareScreenSize(desired_screensize);
        // STARTUP-TIME-COST: 0.16 sec.
        _sdl_window = SDL_CreateWindow(
            title.c_str(), screensize.x, screensize.y,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY |
                (flags & INIT_NO_RESIZABLE ? 0 : SDL_WINDOW_RESIZABLE) |
                (flags & INIT_BORDERLESS ? SDL_WINDOW_BORDERLESS : 0) |
                (flags & INIT_MAXIMIZED ? SDL_WINDOW_MAXIMIZED : 0));
        SDLSetFullscreen(flags);
    #endif
    ScreenSizeChanged();
    LOG_INFO("obtained resolution: ", screensize.x, " ", screensize.y);

    if (!_sdl_window)
        return SDLError("Unable to create window");

    LOG_INFO("SDL window opened...");


    _sdl_context = SDL_GL_CreateContext(_sdl_window);
    LOG_INFO(_sdl_context ? "SDL context passed..." : "SDL context FAILED...");
    if (!_sdl_context) return SDLError("Unable to create OpenGL context");

    LOG_INFO("SDL OpenGL context created...");

    #ifndef __IOS__
        if (flags & INIT_NO_VSYNC) {
            SDL_GL_SetSwapInterval(0);
        } else if (flags & INIT_FIXED_VSYNC) {
            SDL_GL_SetSwapInterval(1);
        } else {
            // By default, attempt adaptive vsync, which may fail.
            if (!SDL_GL_SetSwapInterval(-1)) {
                // Fall back on regular vsync.
                SDL_GL_SetSwapInterval(1);
            }
        }
    #endif

    auto gl_err = OpenGLInit(samples, flags & INIT_LINEAR_COLOR);

    // STARTUP-TIME-COST: 0.08 sec. (due to controller, not joystick)
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) && SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        SDL_JoystickEventsEnabled();
        SDL_UpdateJoysticks();
        int count;
        auto *joystick_ids = SDL_GetJoysticks(&count);
        for(int i = 0; i < count; i++) {
            SDL_Joystick *joy = SDL_OpenJoystick(joystick_ids[i]);
            if (joy) {
                LOG_INFO("Detected joystick: ", SDL_GetJoystickNameForID(joystick_ids[i]), " (",
                                    SDL_GetNumJoystickAxes(joy), " axes, ",
                                    SDL_GetNumJoystickButtons(joy), " buttons, ",
                                    SDL_GetNumJoystickBalls(joy), " balls, ",
                                    SDL_GetNumJoystickHats(joy), " hats)");
            };
        };
        controller = find_controller();
    }

    timestart = SDL_GetPerformanceCounter();
    timefreq = SDL_GetPerformanceFrequency();

    lasttime = -0.02f;    // ensure first frame doesn't get a crazy delta

    return gl_err;
}

void SDLSetFullscreen(InitFlags flags) {
    bool fs = false;
    if (flags & INIT_FULLSCREEN) {
        if (flags & INIT_NATIVE) {
            // If you switch to fullscreen you get some random display mode that is not the
            // native res? So we have to find the native res first.
            // It is weird that we have to do this, because SDL_CreateWindow called with
            // SDL_WINDOW_FULLSCREEN automatically uses desktop res and hz, but
            // SDL_SetWindowFullscreen doesn't??
            SDL_DisplayID display_in_use = 0;  // Only using first display
            int display_mode_count;
            SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(display_in_use, &display_mode_count);
            if (display_mode_count < 1) return;
            // Get destop mode, since we want to prefer the hz of it if possible.
            const SDL_DisplayMode *desktop_mode = SDL_GetDesktopDisplayMode(display_in_use);
            SDL_DisplayMode *bestdm = nullptr;
            int bestres = 0;
            for (int i = 0; i < display_mode_count; ++i) {
                SDL_DisplayMode *dm = modes[i];
                int res = dm->w * dm->h;
                // TODO: allow caller to specify hz? Fallback on other hz if none equal?
                if (res > bestres && dm->refresh_rate == desktop_mode->refresh_rate) {
                    bestres = res;
                    bestdm = dm;
                }
            }
            if (bestres) {
                // Set desired fullscreen res to highest res we found.
                SDL_SetWindowFullscreenMode(_sdl_window, bestdm);
                fs = true;
            } else {
                // Didn't find any mode (unlikely)? Fallback to desktop fullscreen.
                fs = true;
            }
        } else {
            fs = true;
        }
    } else {
        if (flags & INIT_MAXIMIZED) {
            SDL_MaximizeWindow(_sdl_window);
            fs = false;
        }
    }
    SDL_SetWindowFullscreen(_sdl_window, fs);
}

void SDLSetWindowSize(int2 size) {
    if (!_sdl_window) return;
    //size = DPIAwareScreenSize(size);
    SDL_SetWindowSize(_sdl_window, size.x, size.y);
    ScreenSizeChanged();
}

string SDLDebuggerWindow() {
    #ifdef PLATFORM_ES3
        return "Can\'t open debugger window on non-desktop platform";
    #endif
    if (!_sdl_debugger_context) {
        _sdl_debugger_window = SDL_CreateWindow(
            "Lobster Debugger", 600, 800,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
        if (!_sdl_debugger_window)
            return "Debugger SDL_CreateWindow fail";
        _sdl_debugger_context = SDL_GL_CreateContext(_sdl_debugger_window);
        if (!_sdl_debugger_context)
            return "Debugger SDL_GL_CreateContext fail";
        return OpenGLInit(1, true);
    } else {
        SDL_GL_MakeCurrent(_sdl_debugger_window, _sdl_debugger_context);
        return {};
    }
}

void SDLDebuggerOff() {
    SDL_GL_MakeCurrent(_sdl_window, _sdl_context);
    // Reset since we may have interrupted a user interaction.
    for (auto &it : keymap) it.second.Reset();
}

bool SDLDebuggerFrame() {
    #ifndef __EMSCRIPTEN__
        SDL_GL_SwapWindow(_sdl_debugger_window);
    #endif
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        extern pair<bool, bool> IMGUIEvent(SDL_Event *event);
        IMGUIEvent(&event);
        switch (event.type) {
            case SDL_EVENT_QUIT:
                return true;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                return true;
        }
    }
    return false;
}

double GetSeconds() { return (double)(SDL_GetPerformanceCounter() - timestart) / (double)timefreq; }

void SDLShutdown() {
    // FIXME: SDL gives ERROR: wglMakeCurrent(): The handle is invalid. upon SDL_GL_DeleteContext
    if (_sdl_context) {
        //SDL_GL_DeleteContext(_sdl_context);
        _sdl_context = nullptr;
    }
    if (_sdl_window) {
        SDL_DestroyWindow(_sdl_window);
        _sdl_window = nullptr;
    }
    if (_sdl_debugger_context) {
        //SDL_GL_DeleteContext(_sdl_debugger_context);
        _sdl_debugger_context = nullptr;
    }
    if (_sdl_debugger_window) {
        SDL_DestroyWindow(_sdl_debugger_window);
        _sdl_debugger_window = nullptr;
    }

    SDL_Quit();
}

vector<float> &SDLGetFrameTimeLog() { return frametimelog; }

float SDLGetRollingAverage(size_t n) {
    n = std::max((size_t)1, std::min(frametimelog.size(), n));
    float sum = 0.0f;
    for (size_t i = 0; i < n; i++) {
        sum += frametimelog[frametimelog.size() - n + i];
    }
    return sum / frametimelog.size();
}

void SetTargetFrameTime(double ft) { target_frametime = ft; }

bool SDLFrame() {
    if (minimized) {
        SDL_Delay(100);  // save CPU/battery
    } else {
        #ifndef __EMSCRIPTEN__
            SDL_GL_SwapWindow(_sdl_window);
            OpenGLPostSwapBuffers();
        #else
            emscripten_sleep(0);
        #endif
    }

    frametime = GetSeconds() - lasttime;
    lasttime += frametime;
    // Let's not run slower than this, very long pauses can cause animation & gameplay glitches.
    const double minfps = 5.0;
    frametime = min(1.0 / minfps, frametime);
    frames++;
    frametimelog.push_back((float)frametime);
    if (frametimelog.size() > 64) frametimelog.erase(frametimelog.begin());

    auto sleep_time = target_frametime - (frametime - last_sleep);
    if (sleep_time > 0.0) {
        SDL_DelayPrecise((uint64_t)sleep_time);
        last_sleep = sleep_time;
    }

    for (auto &it : keymap) it.second.FrameAdvance();

    mousewheeldelta = 0;
    clearfingers(true);
    dropped_file.clear();

    if (!cursor) clearfingers(false);

    bool closebutton = false;

    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        extern pair<bool, bool> IMGUIEvent(SDL_Event *event);
        auto nomousekeyb = IMGUIEvent(&event);
        switch(event.type) {
            case SDL_EVENT_QUIT:
                closebutton = true;
                break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                if (nomousekeyb.second) break;
                const char *kn = SDL_GetKeyName(event.key.key);
                if (!*kn) break;
                string name = kn;
                std::transform(name.begin(), name.end(), name.begin(),
                               [](char c) { return (char)::tolower(c); });
                updatebutton(name, event.key.down, 0, event.key.repeat);
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    // Built-in key-press functionality.
                    switch (event.key.key) {
                        case SDLK_PRINTSCREEN:
                            ScreenShot("screenshot-" + GetDateTime() + ".png");
                            break;
                    }
                }
                break;
            }

            // This #ifdef is needed, because on e.g. OS X we'd otherwise get SDL_EVENT_FINGER_DOWN
            // in addition to SDL_MOUSE_BUTTON_DOWN on laptop touch pads.
            #ifdef PLATFORM_TOUCH

            // FIXME: if we're in cursor==0 mode, only update delta, not position
            case SDL_EVENT_FINGER_DOWN: {
                if (nomousekeyb.first) break;
                int i = updatedragpos(event.tfinger, event.type);
                updatemousebutton(1, i, true);
                break;
            }
            case SDL_EVENT_FINGER_UP: {
                if (nomousekeyb.first) break;
                int i = findfinger(event.tfinger.fingerID, true);
                updatemousebutton(1, i, false);
                break;
            }

            case SDL_EVENT_FINGER_MOTION: {
                if (nomousekeyb.first) break;
                updatedragpos(event.tfinger, event.type);
                break;
            }

#else

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                if (nomousekeyb.first) break;
                updatemousebutton(event.button.button, 0, event.button.down);
                if (cursor) {
                    fingers[0].mousepos = int2(int(event.button.x), int(event.button.y)) * inputscale;  // TODO(SDL3): support floats here?
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION:
                if (nomousekeyb.first) break;
                fingers[0].mousedelta += int2(int(event.motion.xrel), int(event.motion.yrel));  // TODO(SDL3): support floats here?
                if (cursor) {
                    fingers[0].mousepos = int2(int(event.motion.x), int(event.motion.y)) * inputscale;  // TODO(SDL3): support floats here?
                } else {
                    //if (skipmousemotion) { skipmousemotion--; break; }
                    //if (event.motion.x == screensize.x / 2 && event.motion.y == screensize.y / 2) break;

                    //auto delta = int3(event.motion.xrel, event.motion.yrel);
                    //fingers[0].mousedelta += delta;

                    //auto delta = int3(event.motion.x, event.motion.y) - screensize / 2;
                    //fingers[0].mousepos -= delta;

                    //SDL_WarpMouseInWindow(_sdl_window, screensize.x / 2, screensize.y / 2);
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL: {
                if (nomousekeyb.first) break;
                if (event.wheel.which == SDL_TOUCH_MOUSEID) break;  // Emulated scrollwheel on touch devices?
                auto y = int(event.wheel.y);  // TODO(SDL3): support floats here?
                #ifdef __EMSCRIPTEN__
                    y = y > 0 ? 1 : -1;  // For some reason, it defaults to 10 / -10 ??
                #endif
                mousewheeldelta += event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -y : y;
                break;
            }

            #endif

            case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
                const int deadzone = 8192;
                if (event.jaxis.axis < MAXAXES) {
                    joyaxes[event.jaxis.axis] = abs(event.jaxis.value) > deadzone ? event.jaxis.value / (float)0x8000 : 0;
                };
                break;
            }

            case SDL_EVENT_JOYSTICK_HAT_MOTION:
                break;

            case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            case SDL_EVENT_JOYSTICK_BUTTON_UP: {
                string name = "joy" + to_string(event.jbutton.button);
                updatebutton(name, event.jbutton.down, 0, false);
                break;
            }

            case SDL_EVENT_GAMEPAD_ADDED:
                if (!controller) {
                    controller = SDL_OpenGamepad(event.gdevice.which);
                }
                break;
            case SDL_EVENT_GAMEPAD_REMOVED:
                if (controller &&
                    event.gdevice.which == SDL_GetJoystickID(SDL_GetGamepadJoystick(controller))) {
                    SDL_CloseGamepad(controller);
                    controller = find_controller();
                }
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                if (controller &&
                    event.gdevice.which == SDL_GetJoystickID(SDL_GetGamepadJoystick(controller))) {
                    string name = "controller_";
                    auto sdl_name = SDL_GetGamepadStringForButton((SDL_GamepadButton)event.gbutton.button);
                    name += sdl_name ? sdl_name : cat(event.gbutton.button);
                    updatebutton(name, event.gbutton.down, 0, false);
                }
                break;
            case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
                const int deadzone = 8192;
                controller_axes[event.gaxis.axis] =
                    abs(event.gaxis.value) > deadzone ? event.gaxis.value / (float)0x8000 : 0;
                break;
            }

            case SDL_EVENT_WINDOW_RESIZED: {
                ScreenSizeChanged();
                // reload and bind shaders/textures here
                break;
            }
            case SDL_EVENT_WINDOW_MINIMIZED:
            case SDL_EVENT_WINDOW_HIDDEN:
                minimized = true;
                break;
            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
            case SDL_EVENT_WINDOW_SHOWN:
                minimized = false;
                break;
            case SDL_EVENT_WINDOW_EXPOSED:
                // This event seems buggy, it fires right after SDL_EVENT_WINDOW_MINIMIZED when the user minimizes?
                break;
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                // never gets hit?
                /*
                for (int i = 1; i <= 5; i++)
                    updatemousebutton(i, false);
                */
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                closebutton = true;
                break;

            case SDL_EVENT_DROP_FILE:
                dropped_file = event.drop.data;
                #if 0
                    SDL_free(event.drop.data);  // TODO(SDL3): not possible/needed?
                #endif
                break;

            case SDL_EVENT_TEXT_INPUT:
                textinput.text += event.text.text;
                break;

            case SDL_EVENT_TEXT_EDITING:
                textinput.editing = event.edit.text;
                textinput.cursor = event.edit.start;
                textinput.len = event.edit.length;
                break;
        }
    }

    // simulate mouse up events, since SDL won't send any if the mouse leaves the window while down
    // doesn't work
    /*
    for (int i = 1; i <= 5; i++)
        if (!(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(i)))
            updatemousebutton(i, false);
    */

    /*
    if (SDL_GetMouseFocus() != _sdl_window) {
        int A = 1;
    }
    */

    TextToSpeechUpdate();

    return closebutton || (noninteractivetestmode && frames == 2 /* has rendered one full frame */);
}

void SDLWindowMinMax(int dir) {
    if (!_sdl_window) return;
    if (dir < 0) SDL_MinimizeWindow(_sdl_window);
    else if (dir > 0) SDL_MaximizeWindow(_sdl_window);
    else SDL_RestoreWindow(_sdl_window);
}

double SDLTime() { return lasttime; }
double SDLDeltaTime() { return frametime; }

pair<int64_t, int64_t> GetKS(string_view name) {
    auto &ks = keymap[string(name)];
    #ifdef PLATFORM_TOUCH
        // delayed results by one frame, that way they get 1 frame over finger hovering over target,
        // which makes gl_hit work correctly
        // FIXME: this causes more lag on mobile, instead, set a flag that this is the first frame we're touching,
        // and make that into a special case inside gl_hit
        return ks.Prev();
    #else
        return ks.State();
    #endif
}

bool KeyRepeat(string_view name) {
    auto &ks = keymap[string(name)];
    return ks.repeat;
}

double GetKeyTime(string_view name, int on) {
    auto &ks = keymap[string(name)];
    return ks.lasttime[on];
}

int2 GetKeyPos(string_view name, int on) {
    auto &ks = keymap[string(name)];
    return ks.lastpos[on];
}

void SDLTitle(string_view_nt title) {
    LOBSTER_FRAME_PROFILE_THIS_SCOPE;
    SDL_SetWindowTitle(_sdl_window, title.c_str());
}

int SDLWheelDelta() { return mousewheeldelta; }
bool SDLIsMinimized() { return minimized; }

bool SDLCursor(bool on) {
    if (on == cursor) return cursor;
    cursor = !cursor;
    if (cursor) {
        if (fullscreen) SDL_SetWindowMouseGrab(_sdl_window, false);
        SDL_ShowCursor();
        SDL_SetWindowRelativeMouseMode(_sdl_window, false);
        SDL_WarpMouseInWindow(_sdl_window, float(cursorx), float(cursory));
    } else {
        // TODO(SDL3): Use float cursor position?
        float fcursorx, fcursory;
        SDL_GetMouseState(&fcursorx, &fcursory);
        cursorx = (int)fcursorx;
        cursory = (int)fcursory;
        if (fullscreen) SDL_SetWindowMouseGrab(_sdl_window, true);
        SDL_HideCursor();
        SDL_SetWindowRelativeMouseMode(_sdl_window, true);
        clearfingers(false);
    }
    return !cursor;
}

bool SDLGrab(bool on) {
    SDL_SetWindowMouseGrab(_sdl_window, on);
    return SDL_GetWindowMouseGrab(_sdl_window) == true;
}

void SDLMessageBox(string_view_nt title, string_view_nt msg) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), msg.c_str(), _sdl_window);
}

bool SDLOpenURL(string_view url) {
    return SDL_OpenURL(url.data()) == 0;
}

int64_t SDLLoadFile(string_view_nt absfilename, string *dest, int64_t start, int64_t len) {
    LOG_DEBUG("SDLLoadFile: ", absfilename);
    auto f = SDL_IOFromFile(absfilename.c_str(), "rb");
    if (!f) return -1;
    auto filelen = SDL_SeekIO(f, 0, SDL_IO_SEEK_END);
    if (filelen < 0 || filelen == LLONG_MAX) {
        // If SDL_SeekIO fails it is supposed to return -1, but on Linux it returns LLONG_MAX instead.
        SDL_CloseIO(f);
        LOG_INFO("SDLLoadFile: ", absfilename, " failed to determine file size.");
        return -1;
    }
    if (!len) {  // Just the file length requested.
        SDL_CloseIO(f);
        return filelen;
    }
    if (len < 0) len = filelen;
    SDL_SeekIO(f, start, SDL_IO_SEEK_SET);
    dest->resize((size_t)len);
    auto rlen = SDL_ReadIO(f, &(*dest)[0], (size_t)len);
    SDL_CloseIO(f);
    if (len != (int64_t)rlen) {
        LOG_INFO("SDLLoadFile: ", absfilename, " file is not of requested length. requested: ", len, " found: ", rlen);
        return -1;
    }
    return  len;
}

bool ScreenShot(string_view_nt filename) {
    auto pixels = ReadPixels(int2(0), screensize);
    stbi_flip_vertically_on_write(0);
    auto ok = stbi_write_png(filename.c_str(), screensize.x, screensize.y, 3, pixels,
                             screensize.x * 3);
    delete[] pixels;
    return ok != 0;
}

void SDLTestMode() { noninteractivetestmode = true; }

int SDLScreenDPI(int screen) {
    // TODO(SDL3): "SDL_GetDisplayDPI() - not reliable across platforms,
    // approximately replaced by multiplying SDL_GetWindowDisplayScale() times
    // 160 on iPhone and Android, and 96 on other platforms."
    //
    // However, since we are given the screen here, we should use
    // SDL_GetDisplayContentScale(). This is not as good as using
    // SDL_GetWindowDisplayScale(), since that takes into account the window
    // scaling, which may be different than the display scale.
    float scale = SDL_GetDisplayContentScale(screen);
    if (scale == 0.f) {
        LOG_ERROR("SDL_GetDisplayContentScale: ", SDL_GetError());
        scale = 1.f;
    }
    #if SDL_PLATFORM_IOS || SDL_PLATFORM_ANDROID
        float scalar = 160.0;
    #else
        float scalar = 96.0;
    #endif
    return int(scale * scalar);
}

void SDLStartTextInput(int2 pos, int2 size) {
    SDL_StartTextInput(_sdl_window);
    SDL_Rect rect = { pos.x, pos.y, size.x, size.y };
    SDL_SetTextInputArea(_sdl_window, &rect, 0);  // TODO(SDL3): cursor parameter?
    textinput = TextInput();
}

TextInput &SDLTextInputState() {
    return textinput;
}

void SDLTextInputSet(string_view t) {
    textinput.text = t;
}

void SDLEndTextInput() {
    SDL_StopTextInput(_sdl_window);
}

const char *SDLControllerDataBase(const string& buf) {
    return SDL_AddGamepadMapping(buf.c_str()) >= 0 ? nullptr : SDL_GetError();
}

string SDLGetClipBoard() {
    auto s = SDL_GetClipboardText();
    string contents(s);
    SDL_free(s);
    return contents;
}

void SDLSetClipBoard(const char *contents) {
    SDL_SetClipboardText(contents);
}
