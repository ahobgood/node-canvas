#include "FrameBufferBackend.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

using namespace v8;

FrameBufferBackend::FrameBufferBackend(int width, int height, string path)
	: ImageBackend(width, height)
	, m_fbfd(-1)
	, m_screenBufSize(0)
	, m_screenBuf(NULL)
	{
	size_t path_pos = string::npos;
	path_pos = path.find(':');
	if (string::npos != path_pos) m_fbPath.assign(path.substr(path_pos + 1));
}

FrameBufferBackend::~FrameBufferBackend()
{
    if (surface) {
        destroySurface();
        Nan::AdjustExternalMemory(-approxBytesPerPixel() * width * height);
    }
}

bool FrameBufferBackend::InitFB() {
	// cerr << "FrameBufferBackend::InitFB(), width=" << width << ", height=" << height << ", path=" << m_fbPath << endl;

	if (m_fbPath.empty()) {
		SetErrStr("FrameBuffer backend must use 'fb:[device]' (e.g. 'fb:/dev/fb0')");
		return false;
	}

	int fd = open(m_fbPath.c_str(), O_RDWR);
	if (fd == -1) {
		SetErrStr(string("Error opening framebuffer device: ") + strerror(errno));
		return false;
	}


	struct fb_var_screeninfo vinfo;

	if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
		SetErrStr(string("Error retrieving VSCREENINFO data from framebuffer: ") + strerror(errno));
		close(fd);
		return false;
	}

	// back up the old vscreeninfo struct for later restoration
	memcpy(&m_origScreenVarInfo, &vinfo, sizeof(struct fb_var_screeninfo));

	vinfo.bits_per_pixel = 32;

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo)) {
		SetErrStr(string("Error sending VSCREENINFO data to framebuffer: ") + strerror(errno));
		close(fd);
		return false;
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &m_screenFixInfo)) {
		SetErrStr(string("Error retrieving FSCREENINFO data to framebuffer: ") + strerror(errno));
		close(fd);
		return false;
	}

	m_screenBufSize = m_screenFixInfo.smem_len;
	void *fb = (char*)mmap(0, m_screenBufSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if ((int)fb == -1) {
		SetErrStr(string("Error during framebuffer mmap(): ") + strerror(errno));
		close(fd);
		return false;
	}

	m_screenBuf = fb;
	m_fbfd = fd;

	// cerr << "FrameBufferBackend::InitFB(): done!" << endl;

	return true;
}

bool FrameBufferBackend::blit(const unsigned char *data) {
	if (NULL == m_screenBuf) {
		SetErrStr(string("Error performing blit: framebuffer memory unavailable"));
		return false;
	}

	memcpy(m_screenBuf, data, m_screenBufSize);

	return true;
}

Nan::Persistent<FunctionTemplate> FrameBufferBackend::constructor;

void FrameBufferBackend::Initialize(Handle<Object> target)
{
	Nan::HandleScope scope;

	Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(FrameBufferBackend::New);
	ImageBackend::constructor.Reset(ctor);
	ctor->InstanceTemplate()->SetInternalFieldCount(1);
	ctor->SetClassName(Nan::New<String>("FrameBufferBackend").ToLocalChecked());
	target->Set(Nan::New<String>("FrameBufferBackend").ToLocalChecked(), ctor->GetFunction());
}


NAN_METHOD(FrameBufferBackend::New)
{
	int width  = 0;
	int height = 0;
	string path;


	if (info[0]->IsNumber()) width  = info[0]->Uint32Value();
	if (info[1]->IsNumber()) height = info[1]->Uint32Value();
	if (info[2]->IsString()) path = *String::Utf8Value(info[2]);

	FrameBufferBackend* backend = new FrameBufferBackend(width, height, path);
	backend->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

