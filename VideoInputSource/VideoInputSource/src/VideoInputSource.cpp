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



#define _CRT_NONSTDC_NO_DEPRECATE



#include "videoInput/videoInput.h"

#include <malloc.h>
#include <xmmintrin.h>

#include "VideoInputSource.h"



int calculateDefaultNumFrames(const unsigned int fps_numerator, const unsigned int fps_denominator) {
	return (int)(60 * 60 * 24 * (__int64)fps_numerator / (__int64)fps_denominator);
}



VideoInputSource::VideoInputSource(const int device_id, const char* connection_type, const int width, const int height, const bool frame_skip)
	: mDeviceID(device_id), mFrameSkip(frame_skip) {
	
	int conenction;
	if (stricmp(connection_type, "Composite") == 0) {
		conenction = VI_COMPOSITE;
	}
	else if (stricmp(connection_type, "S_Video") == 0) {
		conenction = VI_S_VIDEO;
	}
	else if (stricmp(connection_type, "Tuner") == 0) {
		conenction = VI_TUNER;
	}
	else if (stricmp(connection_type, "USB") == 0) {
		conenction = VI_USB;
	}
	else {
		throw "VideoInputSource: connection type is invalid";
	}

	if (!mVideoInput.setupDevice(mDeviceID, width, height, conenction)) {
		throw "VideoInputSource: cannot init device";
	}

	mWidth = mVideoInput.getWidth(mDeviceID);
	mHeight = mVideoInput.getHeight(mDeviceID);
	int mSize = mVideoInput.getSize(mDeviceID);
	if (!(mWidth == width && mHeight == height)) {
		mVideoInput.stopDevice(mDeviceID);
		throw "VideoInputSource: cannot init device with assigned width and height";
	}

	mBuffer = (unsigned char*)_aligned_malloc(sizeof(unsigned char) * mSize, sizeof(__m128));
	if (mBuffer == NULL) {
		mVideoInput.stopDevice(mDeviceID);
		throw "VideoInputSource: cannot allocate frame buffer";
	}
	memset(mBuffer, 0, sizeof(unsigned char) * mSize);
}

VideoInputSource::~VideoInputSource() {
	mVideoInput.stopDevice(mDeviceID);
	_aligned_free(mBuffer);
}

const unsigned char* VideoInputSource::GetFrame() {
	bool hasNewFrame = false;
	while (true) {
		hasNewFrame = mVideoInput.isFrameNew(mDeviceID);
		if (hasNewFrame) {
			bool success = mVideoInput.getPixels(mDeviceID, mBuffer, false, false);
			if (!success) {
				throw "VideoInputSource: cannot get frame";
			}
		}
		if (mFrameSkip || hasNewFrame) {
			break;
		}
		Sleep(0);
	}

	return mBuffer;
}

int VideoInputSource::GetWidth() {
	return mWidth;
}

int VideoInputSource::GetHeight() {
	return mHeight;
}
