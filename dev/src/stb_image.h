extern "C" unsigned char *stbi_load(char const *filename,  int *x, int *y, int *comp, int req_comp);
extern "C" unsigned char *stbi_load_from_memory(uchar const *buffer, int len, int *x, int *y, int *comp, int req_comp);
extern "C" void           stbi_image_free(void *retval_from_stbi_load);
