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



#include <vapoursynth/VapourSynth.h>
#include <vapoursynth/VSHelper.h>

#include "VideoInputSource.h"



class VSVideoInputSourceData {
public:
	VideoInputSource* videoInputSource = nullptr;
	VSVideoInfo vi = {};
	const VSVideoInfo* videoInfo = nullptr;

	VSVideoInputSourceData(const int device_id, const char* connection_type, const int width, const int height, const unsigned int fps_numerator, const unsigned int fps_denominator, const int num_frames, const bool frame_skip, VSCore* core, const VSAPI* vsapi) {
		videoInputSource = new VideoInputSource(device_id, connection_type, width, height, frame_skip);

		// set video info & format
		//const VSFormat* videoFormat = vsapi->registerFormat(cmRGB, stInteger, 8, 0, 0, core);
		const VSFormat* videoFormat = vsapi->getFormatPreset(pfRGB24, core);

		vi.format = videoFormat;
		vi.width = videoInputSource->GetWidth();
		vi.height = videoInputSource->GetHeight();
		vi.fpsNum = fps_numerator;
		vi.fpsDen = fps_denominator;
		vi.numFrames = num_frames;

		videoInfo = &vi;
		//videoInfo = new VSVideoInfo();
		//*videoInfo = vi;
	}

	~VSVideoInputSourceData() {
		delete videoInputSource;
		//delete videoInfo;
	}
};



static void VS_CC VSVideoInputSourceInit(VSMap* in, VSMap* out, void** instanceData, VSNode* node, VSCore* core, const VSAPI* vsapi) {
	VSVideoInputSourceData* videoInputSourceData = (VSVideoInputSourceData*)*instanceData;
	vsapi->setVideoInfo(videoInputSourceData->videoInfo, 1, node);
}



static const VSFrameRef* VS_CC VSVideoInputSourceGetFrame(int n, int activationReason, void** instanceData, void** frameData, VSFrameContext* frameCtx, VSCore* core, const VSAPI* vsapi) {
	VSVideoInputSourceData* videoInputSourceData = (VSVideoInputSourceData*)*instanceData;
	if (activationReason == arInitial) {
		const VSFormat* videoFormat = videoInputSourceData->videoInfo->format;

		VSFrameRef* dst = vsapi->newVideoFrame(videoFormat, videoInputSourceData->videoInfo->width, videoInputSourceData->videoInfo->height, nullptr, core);

		const unsigned char* videoBuffer = videoInputSourceData->videoInputSource->GetFrame();

		for (int plane = 0; plane < videoFormat->numPlanes; ++plane) {
			int dst_stride = vsapi->getStride(dst, plane);
			uint8_t* dstp = vsapi->getWritePtr(dst, plane);

			int height = videoInputSourceData->videoInfo->height;
			int width = videoInputSourceData->videoInfo->width;

			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					dstp[x] = videoBuffer[((height - y - 1) * width + x) * 3 + (2 - plane)];
				}

				dstp += dst_stride;
			}
		}

		return dst;
	}

	return nullptr;
}



static void VS_CC VSVideoInputSourceFree(void* instanceData, VSCore* core, const VSAPI* vsapi) {
	VSVideoInputSourceData* videoInputSourceData = (VSVideoInputSourceData*)instanceData;
	delete videoInputSourceData;
}



static void VS_CC VSVideoInputSourceCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi) {
	int err;

	int device_id = vsapi->propGetInt(in, "device_id", 0, NULL);
	const char* connection_type = vsapi->propGetData(in, "connection_type", 0, NULL);
	int width = vsapi->propGetInt(in, "width", 0, NULL);
	int height = vsapi->propGetInt(in, "height", 0, NULL);

	int fps_numerator = vsapi->propGetInt(in, "fps_numerator", 0, &err);
	if (err) {
		fps_numerator = 30;
	}
	int fps_denominator = vsapi->propGetInt(in, "fps_denominator", 0, &err);
	if (err) {
		fps_denominator = 1;
	}
	int num_frames = vsapi->propGetInt(in, "num_frames", 0, &err);
	if (err) {
		num_frames = calculateDefaultNumFrames(fps_numerator, fps_denominator);
	}
	bool frame_skip = !! vsapi->propGetInt(in, "frame_skip", 0, &err);
	if (err) {
		frame_skip = true;
	}

	VSVideoInputSourceData* videoInputSourceData;
	try {
		videoInputSourceData = new VSVideoInputSourceData(device_id, connection_type, width, height, fps_numerator, fps_denominator, num_frames, frame_skip, core, vsapi);
	}
	catch (const char* e) {
		vsapi->setError(out, e);
		return;
	}

	vsapi->createFilter(in, out, "VideoInputSource", VSVideoInputSourceInit, VSVideoInputSourceGetFrame, VSVideoInputSourceFree, fmUnordered, 0, videoInputSourceData, core);
}



VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin* plugin) {
	configFunc("org.fieliapm.VideoInputSource", "video_input_source", "VideoInputSource filter for VapourSynth prior to R55 & VapourSynth Classic", VAPOURSYNTH_API_VERSION, 1, plugin);
	registerFunc("VideoInputSource",
		"device_id:int;"
		"connection_type:data;"
		"width:int;"
		"height:int;"
		"fps_numerator:int:opt;"
		"fps_denominator:int:opt;"
		"num_frames:int:opt;"
		"frame_skip:int:opt;"
	, VSVideoInputSourceCreate, nullptr, plugin);
}
