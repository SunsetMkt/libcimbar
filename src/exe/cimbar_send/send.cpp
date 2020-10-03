/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "cimb_translator/Config.h"
#include "encoder/Encoder.h"
#include "fountain/FountainInit.h"
#include "gui/shaky_cam.h"
#include "gui/window.h"
#include "serialize/str.h"
#include "util/loop_iterator.h"

#include "cxxopts/cxxopts.hpp"

#include <functional>
#include <iostream>
#include <string>
#include <vector>
using std::string;
using std::vector;

int main(int argc, char** argv)
{
	cxxopts::Options options("cimbar video encoder", "Draw a bunch of weird static on the screen!");

	int compressionLevel = cimbar::Config::compression_level();
	unsigned ecc = cimbar::Config::ecc_bytes();
	unsigned defaultFps = 15;
	options.add_options()
	    ("i,in", "Source file", cxxopts::value<vector<string>>())
	    ("c,compression", "Compression level. 0 == no compression.", cxxopts::value<int>()->default_value(turbo::str::str(compressionLevel)))
	    ("e,ecc", "ECC level", cxxopts::value<unsigned>()->default_value(turbo::str::str(ecc)))
	    ("f,fps", "Target FPS", cxxopts::value<unsigned>()->default_value(turbo::str::str(defaultFps)))
	    ("s,shakycam", "Successive images are offset, like a shaky camera effect", cxxopts::value<bool>())
	    ("h,help", "Print usage")
	;
	options.show_positional_help();
	options.parse_positional({"in"});
	options.positional_help("<in...>");

	auto result = options.parse(argc, argv);
	if (result.count("help") or !result.count("in"))
	{
	  std::cout << options.help() << std::endl;
	  return 0;
	}

	vector<string> infiles = result["in"].as<vector<string>>();

	compressionLevel = result["compression"].as<int>();
	ecc = result["ecc"].as<unsigned>();
	unsigned fps = result["fps"].as<unsigned>();
	if (fps == 0)
		fps = defaultFps;
	unsigned delay = 1000 / fps;

	FountainInit::init();

	bool dark = true;
	bool is_shakycam = result.count("shakycam");

	cimbar::shaky_cam cam(cimbar::Config::image_size(), 1080, 1080, dark);
	// if we don't need the shakycam, we'll just turn it off
	// we could use a separate code path (just do a mat copyTo),
	// but this is fine.
	if (!is_shakycam)
		cam.toggle();

	cimbar::window w(cam.width(), cam.height(), "cimbar_send");
	if (!w.is_good())
	{
		std::cerr << "failed to create window :(" << std::endl;
		return 50;
	}

	bool running = true;
	bool start = true;

	auto draw = [&w, &cam, delay, &running, &start] (const cv::Mat& frame, unsigned) {
		if (!start and w.should_close())
			return running = false;
		start = false;

		cv::Mat& windowImg = cam.draw(frame);
		w.show(windowImg, delay);
		return true;
	};

	Encoder en(ecc);
	while (running)
		for (const string& f : infiles)
			en.encode_fountain(f, draw, compressionLevel);
	return 0;
}
