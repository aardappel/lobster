
// platform independent file access:

extern string StripFilePart(const char *filepath);
extern string StripDirPart(const char *filepath);
extern bool SetupDefaultDirs(const char *exefilepath, const char *auxfilepath, bool forcecommandline);   // call this at init to determine default folders to load stuff from
extern uchar *LoadFile(const char *relfilename, size_t *len = NULL);
extern FILE *OpenForWriting(const char *relfilename, bool binary);
extern string SanitizePath(const char *path);

// logging:

#define MINLOGLEVEL 0  // -1 = location trace, 0 = information, 1 = important
extern void DebugLog(int lev, const char *msg, ...);
extern void ProgramOutput(const char *msg);

extern void MsgBox(const char *err);

// time:

extern void InitTime();
extern double SecondsSinceStart();

