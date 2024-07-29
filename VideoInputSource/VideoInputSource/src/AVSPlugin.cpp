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



#include <windows.h>
#include "avisynth/avisynth.h"

#include "VideoInputSource.h"



class AVSVideoInputSource : public IClip {
private:
	VideoInputSource* videoInputSource;
	VideoInfo vi;

public:
	AVSVideoInputSource(const int device_id, const char* connection_type, const int width, const int height, const unsigned int fps_numerator, const unsigned int fps_denominator, const int num_frames, const bool frame_skip, IScriptEnvironment* env) {
		try {
			videoInputSource = new VideoInputSource(device_id, connection_type, width, height, frame_skip);
		} catch (const char* e) {
			env->ThrowError(e);
		}

		memset(&vi, 0, sizeof(vi));
		vi.width = videoInputSource->GetWidth();
		vi.height = videoInputSource->GetHeight();
		vi.fps_numerator = fps_numerator;
		vi.fps_denominator = fps_denominator;
		vi.num_frames = num_frames;
		vi.pixel_type = VideoInfo::CS_BGR24;
	}

	__stdcall ~AVSVideoInputSource() {
		delete videoInputSource;
	}

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) {
		PVideoFrame dst = env->NewVideoFrame(vi);
		BYTE* dst_p = dst->GetWritePtr();
		const int dst_pitch = dst->GetPitch();
		const int dst_row_size = dst->GetRowSize();
		const int dst_height = dst->GetHeight();

		int videoWidth = videoInputSource->GetWidth();
		int videoHeight = videoInputSource->GetHeight();
		int videoRowSize = sizeof(unsigned char) * 3 * videoWidth;
		if (!(dst_row_size == videoRowSize && dst_height == videoHeight)) {
			env->ThrowError("VideoInputSource: frame format is not match");
		}

		const unsigned char* videoBuffer = videoInputSource->GetFrame();

		env->BitBlt(dst_p, dst_pitch, videoBuffer, videoRowSize, videoRowSize, videoHeight);

		return dst;
	}

	const VideoInfo& __stdcall GetVideoInfo() {
		return vi;
	}

	bool __stdcall GetParity(int n) { return false; }
	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {}
	void __stdcall SetCacheHints(int cachehints, int frame_range) {}
};



AVSValue __cdecl Create_AVSVideoInputSource(AVSValue args, void* user_data, IScriptEnvironment* env) {
	int fps_numerator = args[4].AsInt(30);
	int fps_denominator = args[5].AsInt(1);
	int num_frames = calculateDefaultNumFrames(fps_numerator, fps_denominator);
	return new AVSVideoInputSource(args[0].AsInt(), args[1].AsString(), args[2].AsInt(), args[3].AsInt(), fps_numerator, fps_denominator, args[6].AsInt(num_frames), args[7].AsBool(true), env);
}



extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment * env) {
	//const char* ARG_FORMAT = "[device_id]i[connection_type]s[width]i[height]i[fps_numerator]i[fps_denominator]i[num_frames]i[frame_skip]b";
	const char* ARG_FORMAT = "isii[fps_numerator]i[fps_denominator]i[num_frames]i[frame_skip]b";
	env->AddFunction("VideoInputSource", ARG_FORMAT, Create_AVSVideoInputSource, 0);
	return "`VideoInputSource' VideoInputSource plugin";
}
