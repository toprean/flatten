#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdio>

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

//--------------------------------------------------

#define M_COPY_STRING_PROPERLY(p_dst, p_src, int_dst_max_strlen_plus_one) \
strncpy(p_dst, p_src, int_dst_max_strlen_plus_one); \
p_dst[int_dst_max_strlen_plus_one - 1] = '\0';

//--------------------------------------------------

#define CONST_STRING__FALLBACK_TIMESTAMP                        "yyyyy-mm-dd hh:mm:ss.ddd,ddd,ddd"
#define CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX        64
#define CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART1  32
#define CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART2  16
#define CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG                 4096

//--------------------------------------------------

static void obtain_timestamp_prefix(char * arg_timestamp)
{
	int rc = 0;
	struct timespec struct_timespec_temp;
	struct tm struct_tm_temp;
	//--------------------------------------------------
	// Reference:
	//
	//--    struct timespec {
	//--        time_t   tv_sec;        // seconds
	//--        long     tv_nsec;       // nanoseconds
	//--    };
	//--------------------------------------------------
	char sbuf_prefix_part1             [ CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART1 ] = "\0";
	char sbuf_prefix_part2_no_commas   [ CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART2 ] = "\0";
	char sbuf_prefix_part2_with_commas [ CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART2 ] = "\0";
	//--------------------------------------------------
	memset(&struct_timespec_temp, 0, sizeof(struct_timespec_temp));
	memset(&struct_tm_temp,       0, sizeof(struct_tm_temp));
	//--memset(sbuf_prefix_part1,             0, ((size_t) CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART1) );
	//--memset(sbuf_prefix_part2_no_commas,   0, ((size_t) CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART2) );
	//--memset(sbuf_prefix_part2_with_commas, 0, ((size_t) CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART2) );
	//--------------------------------------------------
	rc = clock_gettime(CLOCK_REALTIME, &struct_timespec_temp);
	if ( rc != 0 )
	{
		//--------------------------------------------------
		// Something went wrong.
		//--------------------------------------------------
		M_COPY_STRING_PROPERLY(
			arg_timestamp,
			CONST_STRING__FALLBACK_TIMESTAMP,
			CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX);
	}
	else
	{
		//--------------------------------------------------
		// The call to clock_gettime() was successful.
		// Convert "time_t" to "struct tm" (using the local time zone)
		//--------------------------------------------------
		localtime_r(
			&struct_timespec_temp.tv_sec,
			&struct_tm_temp);
		//--------------------------------------------------
		// Convert "struct tm" to "yyyyy-mm-dd hh:mm:ss."
		//
		// Reference:
		// http://man7.org/linux/man-pages/man3/strftime.3.html
		//
		// %Y     The year as a decimal number including the century.
		// %m     The month as a decimal number (range 01 to 12).
		// %d     The day of the month as a decimal number (range 01 to 31).
		//
		// %H     The hour as a decimal number using a 24-hour clock (range 00 to 23).
		// %M     The minute as a decimal number (range 00 to 59).
		// %S     The second as a decimal number (range 00 to 60).  (The range is up to 60 to allow for occasional leap seconds.)
		//--------------------------------------------------
		strftime(
			sbuf_prefix_part1,
			(size_t) CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART1,
			"%Y-%m-%d %H:%M:%S.",
			&struct_tm_temp);
		//--------------------------------------------------
		// Convert "tv_nsec" to "ddd,ddd,ddd"
		//--------------------------------------------------
		snprintf(
			sbuf_prefix_part2_no_commas,
			(size_t) CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX_PART2,
			"%09ld",
			struct_timespec_temp.tv_nsec);
		//--------------------------------------------------
		// Displaying milliseconds.
		//--------------------------------------------------
		sbuf_prefix_part2_with_commas [  0 ] = sbuf_prefix_part2_no_commas [ 0 ];
		sbuf_prefix_part2_with_commas [  1 ] = sbuf_prefix_part2_no_commas [ 1 ];
		sbuf_prefix_part2_with_commas [  2 ] = sbuf_prefix_part2_no_commas [ 2 ];
		sbuf_prefix_part2_with_commas [  3 ] = '\0';
		//--------------------------------------------------
		// Use the code below if you want to display microseconds or nanoseconds.
		//--------------------------------------------------
		//--sbuf_prefix_part2_with_commas [  3 ] = ',';
		//--sbuf_prefix_part2_with_commas [  4 ] = sbuf_prefix_part2_no_commas [ 3 ];
		//--sbuf_prefix_part2_with_commas [  5 ] = sbuf_prefix_part2_no_commas [ 4 ];
		//--sbuf_prefix_part2_with_commas [  6 ] = sbuf_prefix_part2_no_commas [ 5 ];
		//--sbuf_prefix_part2_with_commas [  7 ] = ',';
		//--sbuf_prefix_part2_with_commas [  8 ] = sbuf_prefix_part2_no_commas [ 6 ];
		//--sbuf_prefix_part2_with_commas [  9 ] = sbuf_prefix_part2_no_commas [ 7 ];
		//--sbuf_prefix_part2_with_commas [ 10 ] = sbuf_prefix_part2_no_commas [ 8 ];
		//--sbuf_prefix_part2_with_commas [ 11 ] = '\0';
		//--------------------------------------------------
		snprintf(
			arg_timestamp,
			(size_t) CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX,
			"%s%s",
			sbuf_prefix_part1,
			sbuf_prefix_part2_with_commas);
	} // end if
} // end function

//--------------------------------------------------

static void logmsg(const char * arg_fmt, ...)
{
	char sbuf_timestamp_prefix [ CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX ];
	char sbuf_log_msg          [ CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG          ];
	int int_num_chars = 0;
	va_list ap;
	//--memset(sbuf_timestamp_prefix, 0, ((size_t) CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX) );
	//--memset(sbuf_log_msg,          0, ((size_t) CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG)          );
	obtain_timestamp_prefix(sbuf_timestamp_prefix);
	va_start(ap, arg_fmt);
	int_num_chars = vsnprintf(sbuf_log_msg, (size_t) CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG, arg_fmt, ap);
	va_end(ap);
	//--------------------------------------------------
	// If truncation occurred, hint at the truncation with "..." at the end of the local string buffer.
	//--------------------------------------------------
	if ( int_num_chars >= ((int) CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG) ) {
		sbuf_log_msg [ CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG - 4 ] = '.';
		sbuf_log_msg [ CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG - 3 ] = '.';
		sbuf_log_msg [ CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG - 2 ] = '.';
		//--sbuf_log_msg [ CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG - 1 ] = '\0';
	} // end if
	//--------------------------------------------------
	printf(
		"{%.*s} %.*s\n",
		(int) CONST_INT__MAX_STRLEN_PLUS_ONE__TIMESTAMP_PREFIX - 1,
		sbuf_timestamp_prefix,
		(int) CONST_INT__MAX_STRLEN_PLUS_ONE__LOG_MSG - 1,
		sbuf_log_msg);
} // end function

//--------------------------------------------------

class Settings
{
public:
	Settings() : goodInput(false) {}

	//--------------------------------------------------

	enum InputType
	{
		INVALID,
		VIDEO_FILE,
		IMAGE_LIST
	};

	//--------------------------------------------------

	void write(cv::FileStorage& fs) const
	{
		fs << "{"
				  << "input" << input

				  << "original_image_width" << originalSize.width
				  << "original_image_height" << originalSize.height

				  << "intermediate_image_width" << intermedSize.width
				  << "intermediate_image_height" << intermedSize.height

				  << "final_image_width" << finalSize.width
				  << "final_image_height" << finalSize.height

				  << "use_fisheye_model" << useFisheye

				  << "camera_matrix" << cameraMatrix

				  << "distortion_coefficients" <<  distortionCoefficients
		   << "}";
	}

	//--------------------------------------------------

	void read(const cv::FileNode& node)
	{
		node["input"] >> input;

		node["original_image_width"] >> originalSize.width;
		node["original_image_height"] >> originalSize.height;

		node["intermediate_image_width"] >> intermedSize.width;
		node["intermediate_image_height"] >> intermedSize.height;

		node["final_image_width"] >> finalSize.width;
		node["final_image_height"] >> finalSize.height;

		node["use_fisheye_model"] >> useFisheye;

		node["camera_matrix"] >> cameraMatrix;

		node["distortion_coefficients"] >> distortionCoefficients;

		validate();
	}

	//--------------------------------------------------

	void validate()
	{
		goodInput = true;

		if ( originalSize.width <= 0 || originalSize.height <= 0 )
		{
			std::cerr << "Invalid original image size: " << originalSize.width << " x " << originalSize.height << std::endl;
			goodInput = false;
		}

		if ( intermedSize.width <= 0 || intermedSize.height <= 0 )
		{
			std::cerr << "Invalid intermediate image size: " << intermedSize.width << " x " << intermedSize.height << std::endl;
			goodInput = false;
		}

		if ( intermedSize.width < originalSize.width )
		{
			std::cerr << "Invalid: intermediate width (" << intermedSize.width
				<< ")_must be >= original width (" << originalSize.width << std::endl;
			goodInput = false;
		}

		if ( intermedSize.height < originalSize.height )
		{
			std::cerr << "Invalid: intermediate height (" << intermedSize.height
				<< ")_must be >= original height (" << originalSize.height << std::endl;
			goodInput = false;
		}

		if ( finalSize.width > intermedSize.width )
		{
			std::cerr << "Invalid: final width (" << finalSize.width
				<< ")_must be >= intermediate width (" << intermedSize.width << std::endl;
			goodInput = false;
		}

		if ( finalSize.height > intermedSize.height )
		{
			std::cerr << "Invalid: final height (" << finalSize.height
				<< ")_must be >= intermediate height (" << intermedSize.height << std::endl;
			goodInput = false;
		}

		if ( input.empty() )
		{
			inputType = INVALID;
		}
		else
		{
			if ( isListOfImages(input) && readStringList(input, imageList) )
			{
				inputType = IMAGE_LIST;
			}
			else
			{
				inputType = VIDEO_FILE;
			}
			if ( inputType == VIDEO_FILE )
			{
				videoCapture.open(input);
			}
			if ( inputType != IMAGE_LIST && !videoCapture.isOpened() )
			{
				inputType = INVALID;
			}
		}
		if ( inputType == INVALID )
		{
			std::cerr << " Input does not exist: " << input;
			goodInput = false;
		}
		frameNum = 0;
	}

	//--------------------------------------------------

	cv::Mat nextImage()
	{
		cv::Mat result;
		if ( videoCapture.isOpened() )
		{
			videoCapture >> result;
			++frameNum;
		}
		else if ( frameNum < imageList.size() )
		{
			result = cv::imread(imageList[frameNum++], cv::IMREAD_COLOR);
		}
		return result;
	}

	//--------------------------------------------------

	static bool readStringList(const std::string& filename, std::vector<std::string>& l)
	{
		l.clear();
		cv::FileStorage fs(filename, cv::FileStorage::READ);
		if ( !fs.isOpened() )
		{
			return false;
		}
		cv::FileNode n = fs.getFirstTopLevelNode();
		if ( n.type() != cv::FileNode::SEQ )
		{
			return false;
		}
		cv::FileNodeIterator it = n.begin();
		cv::FileNodeIterator it_end = n.end();
		for ( ; it != it_end; ++it )
		{
			l.push_back((std::string)*it);
		}
		return true;
	}

	//--------------------------------------------------

	static bool isListOfImages(const std::string& filename)
	{
		std::string s(filename);
		// Look for file extension
		if ( s.find(".xml") == std::string::npos && s.find(".yaml") == std::string::npos && s.find(".yml") == std::string::npos )
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	//--------------------------------------------------

public:

	std::string input;
	std::vector<std::string> imageList;
	size_t frameNum;

	cv::Size originalSize;
	cv::Size intermedSize;
	cv::Size finalSize;

	bool useFisheye;

	cv::VideoCapture videoCapture;
	InputType inputType;
	bool goodInput;

	cv::Mat cameraMatrix;
	cv::Mat distortionCoefficients;

};

//--------------------------------------------------

int main (int argc, char** argv)
{
	logmsg("main() begins.");
	const cv::String keys
		= "{help h usage ? |           | print this message            }"
		  "{@settings      |default.xml| input setting file            }";

	cv::CommandLineParser parser(argc, argv, keys);

	parser.about("This is a distortion flattening program.\n"
				 "Usage: flatten [configuration_file] -- default ./default.xml]\n"
				 "The configuration file can be XML, YML or YAML.");

	if ( !parser.check() )
	{
		parser.printErrors();
		return 0;
	}

	if ( parser.has("help") )
	{
		parser.printMessage();
		return 0;
	}

	//! [file_read]
	Settings s;
	const std::string inputSettingsFile = parser.get<std::string>(0);
	cv::FileStorage fs(inputSettingsFile, cv::FileStorage::READ); // Read the settings
	if ( !fs.isOpened() )
	{
		std::cerr << "Fatal error: Could not open the configuration file: \"" << inputSettingsFile << "\"" << std::endl;
		parser.printMessage();
		return -1;
	}
	s.read(fs["Settings"]);
	fs.release();                                         // close Settings file
	//! [file_read]

	//FileStorage fout("settings.yml", FileStorage::WRITE); // write config as YAML
	//fout << "Settings" << s;

	if ( !s.goodInput )
	{
		std::cerr << "Fatal error: Invalid input detected. Application stopping." << std::endl;
		return -1;
	}

	// rectangle that defines the region of interest
	cv::Rect myROI(
		(s.intermedSize.width - s.finalSize.width) >> 1,
		(s.intermedSize.height - s.finalSize.height) >> 1,
		s.finalSize.width,
		s.finalSize.height);

	cv::Mat view; // original image
	cv::Mat rview; // rectified image (bigger than the original image)
	cv::Mat cview; // crop inside the rectified image
	cv::Mat map1;
	cv::Mat map2;

	if ( s.useFisheye )
	{
		cv::Mat newCamMat;
		cv::fisheye::estimateNewCameraMatrixForUndistortRectify(
			s.cameraMatrix, s.distortionCoefficients, s.originalSize,
			cv::Matx33d::eye(), newCamMat, 1, s.intermedSize);
		cv::fisheye::initUndistortRectifyMap(
			s.cameraMatrix, s.distortionCoefficients, cv::Matx33d::eye(),
			newCamMat, s.intermedSize, CV_16SC2, map1, map2);
	}
	else
	{
		cv::initUndistortRectifyMap(
			s.cameraMatrix, s.distortionCoefficients, cv::Mat(),
			cv::getOptimalNewCameraMatrix(
				s.cameraMatrix, s.distortionCoefficients,
				s.originalSize, 1, s.intermedSize, 0), 
			s.intermedSize, CV_16SC2, map1, map2);
	}

	if ( s.inputType == Settings::IMAGE_LIST )
	{
		for(;;)
		{
			size_t i = s.frameNum;
			view = s.nextImage();
			if ( view.empty() )
			{
				break;
			}
			logmsg("main() s.imageList[%zu] (out of %zu) = '%s'", i, s.imageList.size(), s.imageList[i].c_str());

			cv::remap(view, rview, map1, map2, cv::INTER_CUBIC);

			// Crop the bigger rectified image down to the rectangle defined by the region of interest.
			// Note that this does not copy the data.
			cview = rview(myROI);

			// Save the view to a file.
			const std::string& original_filename = s.imageList[i];
			std::size_t idx = original_filename.find_last_of(".");
			std::string temp_prefix = original_filename.substr(0, idx);
			std::string temp_suffix = original_filename.substr(idx + 1);
			std::string outfilename = temp_prefix + "-b." + temp_suffix;
			//logmsg("main() outfilename = '%s'", outfilename.c_str());
			bool result = false;
			try
			{
				result = cv::imwrite(outfilename, cview);
			}
			catch (const cv::Exception& ex)
			{
				fprintf(stderr, "cv::imwrite() encountered an exception: %s\n", ex.what());
			}
			if ( !result )
			{
				logmsg("main() Could not save view to '%s'.", outfilename.c_str());
				// Give the user a chance to see the error message and decide what to do in response to the error.
				// At this point, the user has a choice: press a key to continue or press Ctrl-C to quit.
				cv::waitKey(0);
			}

		} // end for
	}
	else if ( s.inputType == Settings::VIDEO_FILE )
	{
		logmsg("main() input video file = '%s'", s.input.c_str());

		std::size_t idx = s.input.find_last_of('.');
		std::string outputVideoFilename = s.input.substr(0, idx) + "-b.avi";

		logmsg("main() output video file = '%s'", outputVideoFilename.c_str());

		cv::VideoWriter videoWriter;

		int fourcc = static_cast<int>(s.videoCapture.get(cv::CAP_PROP_FOURCC));

		cv::Size deducedOriginalSize = cv::Size(
			(int) s.videoCapture.get(cv::CAP_PROP_FRAME_WIDTH), 
			(int) s.videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT));

		assert( deducedOriginalSize.width == s.originalSize.width && deducedOriginalSize.height == s.originalSize.height );

		videoWriter.open(outputVideoFilename, fourcc, s.videoCapture.get(cv::CAP_PROP_FPS), deducedOriginalSize, true);
		if ( !videoWriter.isOpened() )
		{
			std::cerr << "Fatal error: Could not open the output video for writing: " << outputVideoFilename << std::endl;
			return -1;
		}

		for(;;)
		{
			size_t i = s.frameNum;
			view = s.nextImage();
			if ( view.empty() )
			{
				break;
			}
			logmsg("main() frame %zu", i);

			cv::remap(view, rview, map1, map2, cv::INTER_CUBIC);

			// Crop the bigger rectified image down to the rectangle defined by the region of interest.
			// Note that this does not copy the data.
			cview = rview(myROI);

			try
			{
				videoWriter.write(cview);
			}
			catch (const cv::Exception& ex)
			{
				fprintf(stderr, "cv::VideoWriter::write() encountered an exception: %s\n", ex.what());
				logmsg("main() ends abnormally.");
				return -1;
			}

		} // end for
	}

	logmsg("main() ends normally.");
	return 0;
}

//--------------------------------------------------
// end of this file
//--------------------------------------------------
