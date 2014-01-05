/*

VideoInputSource - A AviSynth plug-in to capture video from webcam or video capture device
Copyright (C) 2014-present Himawari Tachibana <fieliapm@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/



#include "videoInput/videoInput.h"
#include <avisynth.h>
//#include <stdlib.h>



class VideoInputSource : public IClip {
private:
	videoInput mVideoInput;
	int mDeviceID;
	int mWidth, mHeight;
	int mSize;
	unsigned char* mBuffer;
	bool mFrameSkip;
	VideoInfo vi;

public:
	VideoInputSource(const int device_id, const char* connection_type, const int width, const int height, const unsigned int fps_numerator, const unsigned int fps_denominator, const int num_frames, const bool frame_skip, IScriptEnvironment* env)
		: mDeviceID(device_id), mFrameSkip(frame_skip) {
		int conenction;
		if(stricmp(connection_type, "Composite")==0) {
			conenction = VI_COMPOSITE;
		} else if (stricmp(connection_type, "S_Video")==0) {
			conenction = VI_S_VIDEO;
		} else if (stricmp(connection_type, "Tuner")==0) {
			conenction = VI_TUNER;
		} else if (stricmp(connection_type, "USB")==0) {
			conenction = VI_USB;
		} else {
			env->ThrowError("VideoInputSource: connection type is invalid");
		}
		mVideoInput.setupDevice(mDeviceID, width, height, conenction);
		mWidth = mVideoInput.getWidth(mDeviceID);
		mHeight = mVideoInput.getHeight(mDeviceID);
		mSize = mVideoInput.getSize(mDeviceID);

		if(!(mWidth == width && mHeight == height)) {
			env->ThrowError("VideoInputSource: cannot init videoInput with assigned width and height");
		}

		mBuffer = new unsigned char[mSize];
		memset(mBuffer, 0, sizeof(unsigned char)*mSize);

		memset(&vi, 0, sizeof(vi));
		vi.width = mWidth;
		vi.height = mHeight;
		vi.fps_numerator = fps_numerator;
		vi.fps_denominator = fps_denominator;
		vi.num_frames = num_frames;
		vi.pixel_type = VideoInfo::CS_BGR24;
	}

	__stdcall ~VideoInputSource() {
		delete[] mBuffer;
		mVideoInput.stopDevice(mDeviceID);
	}

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) {
		PVideoFrame dst = env->NewVideoFrame(vi);
		unsigned char* dst_p = dst->GetWritePtr();
		const int dst_pitch = dst->GetPitch();
		const int dst_row_size = dst->GetRowSize();
		const int dst_height = dst->GetHeight();

		const int mRowSize = sizeof(unsigned char)*3*mWidth;

		if(!(dst_row_size == mRowSize && dst_height == mHeight)) {
			env->ThrowError("VideoInputSource: frame format is not match");
		}

		if((!mFrameSkip) || mVideoInput.isFrameNew(mDeviceID)) {
			mVideoInput.getPixels(mDeviceID, mBuffer, false, false);
		}

		unsigned char* mBufferOffset = mBuffer;
		for(int h=0;h<mHeight;++h,dst_p+=dst_pitch,mBufferOffset+=mRowSize) {
			memcpy(dst_p, mBufferOffset, mRowSize);
		}

		return dst;
	}

	const VideoInfo& __stdcall GetVideoInfo() {
		return vi;
	}

	bool __stdcall GetParity(int n) { return false; }
	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {}
	void __stdcall SetCacheHints(int cachehints,int frame_range) {}
};



AVSValue __cdecl Create_VideoInputSource(AVSValue args, void* user_data, IScriptEnvironment* env) {
	int fps_numerator = args[4].AsInt(30);
	int fps_denominator = args[5].AsInt(1);
	int num_frames = (int)(60*60*24*(__int64)fps_numerator/(__int64)fps_denominator);
	return new VideoInputSource(args[0].AsInt(), args[1].AsString(), args[2].AsInt(), args[3].AsInt(), fps_numerator, fps_denominator, num_frames, args[7].AsBool(true), env);
}



extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
	//const char* ARG_FORMAT = "[device_id]i[connection_type]s[width]i[height]i[fps_numerator]i[fps_denominator]i[num_frames]i[frame_skip]b";
	const char* ARG_FORMAT = "isii[fps_numerator]i[fps_denominator]i[num_frames]i[frame_skip]b";
	env->AddFunction("VideoInputSource", ARG_FORMAT, Create_VideoInputSource, 0);
	return "`VideoInputSource' VideoInputSource plugin";
}
