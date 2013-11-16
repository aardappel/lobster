// simple interface for SDL (that doesn't depend on its headers)

extern string SDLInit(const char *title, int2 &screensize);
extern bool SDLFrame(int2 &screensize);
extern void SDLShutdown();
extern void SDLTitle(const char *title);
extern bool SDLIsMinimized();


struct UpDown
{
    bool isdown;
    bool wentdown;
    bool wentup;

    UpDown() : isdown(false), wentdown(false), wentup(false) {}
};


extern const int2 &GetFinger(int i, bool delta);
extern UpDown GetKS(const char *name);
extern float GetKeyTime(const char *name, int on);
extern int2 GetKeyPos(const char *name, int on);

extern float SDLTime();
extern float SDLDeltaTime();

extern int SDLWheelDelta();

extern bool SDLCursor(bool on);
extern bool SDLGrab(bool on);

extern bool SDLPlaySound(const char *filename, bool sfxr);
extern void SDLSoundClose();
