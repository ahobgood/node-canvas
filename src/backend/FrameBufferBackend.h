#ifndef __FB_BACKEND_H__
#define __FB_BACKEND_H__

#include <v8.h>
#include <string>

#include "ImageBackend.h"

using namespace std;

class FrameBufferBackend : public ImageBackend
{
  private:
    string m_fbPath;
    string m_lastErrorString;
    inline void SetErrStr(string str) { m_lastErrorString.assign(str); };

  public:
    FrameBufferBackend(int width, int height, string path);
    ~FrameBufferBackend();

    bool InitFB();
    inline const string ErrStr() { return m_lastErrorString; };

    static Nan::Persistent<v8::FunctionTemplate> constructor;
    static void Initialize(v8::Handle<v8::Object> target);
    static NAN_METHOD(New);
    static NAN_METHOD(Blit);
    static void BlitAsync(uv_work_t *req);
    static void BlitAsyncAfter(uv_work_t *req);
};

#endif
