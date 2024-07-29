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



int calculateDefaultNumFrames(const unsigned int fps_numerator, const unsigned int fps_denominator);



class VideoInputSource {
private:
	videoInput mVideoInput;
	int mDeviceID;
	int mWidth, mHeight;
	unsigned char* mBuffer;
	bool mFrameSkip;

public:
	VideoInputSource(const int device_id, const char* connection_type, const int width, const int height, const bool frame_skip);
	~VideoInputSource();

	const unsigned char* GetFrame();
	int GetWidth();
	int GetHeight();
};
