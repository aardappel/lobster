// simple interface for SDL (that doesn't depend on its headers)

extern string SDLInit(const char *title, int2 &screensize);
extern bool SDLFrame(int2 &screensize);
extern void SDLShutdown();
extern void SDLTitle(const char *title);
extern bool SDLIsMinimized();

extern const int2 &GetFinger(int i, bool delta);
extern bool GetKS(const char *name, bool delta);

extern float SDLTime();
extern float SDLDeltaTime();

extern int SDLWheelDelta();

extern bool SDLCursor(bool on);
extern bool SDLGrab(bool on);

extern bool SDLPlaySound(const char *filename, bool sfxr);
extern void SDLSoundClose();
