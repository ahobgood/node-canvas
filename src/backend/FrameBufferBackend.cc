#include "FrameBufferBackend.h"

using namespace v8;

FrameBufferBackend::FrameBufferBackend(int width, int height, string path)
	: ImageBackend(width, height)
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
	if (m_fbPath.empty()) {
		SetErrStr("FrameBuffer backend must use 'fb:[device]' (e.g. 'fb:/dev/fb0')");
		return false;
	}

	cerr << "FrameBufferBackend::Init(), width=" << width << ", height=" << height << ", path=" << m_fbPath << endl;

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

