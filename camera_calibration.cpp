#include <stdarg.h>
#include <stdio.h>
#include <time.h>

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

	enum Pattern
	{
		NOT_EXISTING,
		CHESSBOARD,
		CIRCLES_GRID,
		ASYMMETRIC_CIRCLES_GRID
	};

	//--------------------------------------------------

	enum InputType
	{
		INVALID,
		CAMERA,
		VIDEO_FILE,
		IMAGE_LIST
	};

	//--------------------------------------------------

	void write(cv::FileStorage& fs) const
	{
		fs << "{"
				  << "BoardSize_Width"  << boardSize.width
				  << "BoardSize_Height" << boardSize.height
				  << "Square_Size"      << squareSize
				  << "Calibrate_Pattern" << patternToUse
				  << "Calibrate_NrOfFrameToUse" << nrFrames
				  << "Calibrate_FixAspectRatio" << aspectRatio
				  << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
				  << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

				  << "Write_DetectedFeaturePoints" << writePoints
				  << "Write_extrinsicParameters"   << writeExtrinsics
				  << "Write_gridPoints" << writeGrid
				  << "Write_outputFileName"  << outputFileName

				  << "Show_UndistortedImage" << showUndistorted

				  << "Input_FlipAroundHorizontalAxis" << flipVertical
				  << "Input_Delay" << delay
				  << "Input" << input
		   << "}";
	}

	//--------------------------------------------------

	void read(const cv::FileNode& node)
	{
		node["BoardSize_Width" ] >> boardSize.width;
		node["BoardSize_Height"] >> boardSize.height;
		node["Calibrate_Pattern"] >> patternToUse;
		node["Square_Size"]  >> squareSize;
		node["Calibrate_NrOfFrameToUse"] >> nrFrames;
		node["Calibrate_FixAspectRatio"] >> aspectRatio;
		node["Write_DetectedFeaturePoints"] >> writePoints;
		node["Write_extrinsicParameters"] >> writeExtrinsics;
		node["Write_gridPoints"] >> writeGrid;
		node["Write_outputFileName"] >> outputFileName;
		node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
		node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
		node["Calibrate_UseFisheyeModel"] >> useFisheye;
		node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
		node["Show_UndistortedImage"] >> showUndistorted;
		node["Input"] >> input;
		node["Input_Delay"] >> delay;
		node["Fix_K1"] >> fixK1;
		node["Fix_K2"] >> fixK2;
		node["Fix_K3"] >> fixK3;
		node["Fix_K4"] >> fixK4;
		node["Fix_K5"] >> fixK5;

		validate();
	}

	//--------------------------------------------------

	void validate()
	{
		goodInput = true;
		if ( boardSize.width <= 0 || boardSize.height <= 0 )
		{
			std::cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << std::endl;
			goodInput = false;
		}
		if ( squareSize <= 10e-6 )
		{
			std::cerr << "Invalid square size " << squareSize << std::endl;
			goodInput = false;
		}
		if ( nrFrames <= 0 )
		{
			std::cerr << "Invalid number of frames " << nrFrames << std::endl;
			goodInput = false;
		}

		if ( input.empty() )
		{
			inputType = INVALID;
		}
		else
		{
			if ( input[0] >= '0' && input[0] <= '9' )
			{
				std::stringstream ss(input);
				ss >> cameraID;
				inputType = CAMERA;
			}
			else
			{
				if ( isListOfImages(input) && readStringList(input, imageList) )
				{
					inputType = IMAGE_LIST;
					nrFrames = (nrFrames < (int)imageList.size()) ? nrFrames : (int)imageList.size();
				}
				else
				{
					inputType = VIDEO_FILE;
				}
			}
			if ( inputType == CAMERA )
			{
				inputCapture.open(cameraID);
			}
			if ( inputType == VIDEO_FILE )
			{
				inputCapture.open(input);
			}
			if ( inputType != IMAGE_LIST && !inputCapture.isOpened() )
			{
				inputType = INVALID;
			}
		}
		if ( inputType == INVALID )
		{
			std::cerr << " Input does not exist: " << input;
			goodInput = false;
		}

		flag = 0;
		if ( calibFixPrincipalPoint ) flag |= cv::CALIB_FIX_PRINCIPAL_POINT;
		if ( calibZeroTangentDist )   flag |= cv::CALIB_ZERO_TANGENT_DIST;
		if ( aspectRatio )            flag |= cv::CALIB_FIX_ASPECT_RATIO;
		if ( fixK1 )                  flag |= cv::CALIB_FIX_K1;
		if ( fixK2 )                  flag |= cv::CALIB_FIX_K2;
		if ( fixK3 )                  flag |= cv::CALIB_FIX_K3;
		if ( fixK4 )                  flag |= cv::CALIB_FIX_K4;
		if ( fixK5 )                  flag |= cv::CALIB_FIX_K5;

		if ( useFisheye )
		{
			// the fisheye model has its own enum, so overwrite the flags
			flag = cv::fisheye::CALIB_FIX_SKEW | cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC;
			if ( fixK1 )                  flag |= cv::fisheye::CALIB_FIX_K1;
			if ( fixK2 )                  flag |= cv::fisheye::CALIB_FIX_K2;
			if ( fixK3 )                  flag |= cv::fisheye::CALIB_FIX_K3;
			if ( fixK4 )                  flag |= cv::fisheye::CALIB_FIX_K4;
			if ( calibFixPrincipalPoint ) flag |= cv::fisheye::CALIB_FIX_PRINCIPAL_POINT;
		}

		calibrationPattern = NOT_EXISTING;
		if ( !patternToUse.compare("CHESSBOARD") ) calibrationPattern = CHESSBOARD;
		if ( !patternToUse.compare("CIRCLES_GRID") ) calibrationPattern = CIRCLES_GRID;
		if ( !patternToUse.compare("ASYMMETRIC_CIRCLES_GRID") ) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;
		if ( calibrationPattern == NOT_EXISTING )
		{
			std::cerr << " Camera calibration mode does not exist: " << patternToUse << std::endl;
			goodInput = false;
		}
		atImageList = 0;

	}

	//--------------------------------------------------

	cv::Mat nextImage()
	{
		cv::Mat result;
		if ( inputCapture.isOpened() )
		{
			cv::Mat view0;
			inputCapture >> view0;
			view0.copyTo(result);
		}
		else if ( atImageList < imageList.size() )
		{
			result = cv::imread(imageList[atImageList++], cv::IMREAD_COLOR);
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
	cv::Size boardSize;          // The size of the board -> Number of items by width and height
	Pattern calibrationPattern;  // One of the Chessboard, circles, or asymmetric circle pattern
	float squareSize;            // The size of a square in your defined unit (point, millimeter,etc).
	int nrFrames;                // The number of frames to use from the input for calibration
	float aspectRatio;           // The aspect ratio
	int delay;                   // In case of a video input
	bool writePoints;            // Write detected feature points
	bool writeExtrinsics;        // Write extrinsic parameters
	bool writeGrid;              // Write refined 3D target grid points
	bool calibZeroTangentDist;   // Assume zero tangential distortion
	bool calibFixPrincipalPoint; // Fix the principal point at the center
	bool flipVertical;           // Flip the captured images around the horizontal axis
	std::string outputFileName;  // The name of the file where to write
	bool showUndistorted;        // Show undistorted images after calibration
	std::string input;           // The input ->
	bool useFisheye;             // use fisheye camera model for calibration
	bool fixK1;                  // fix K1 distortion coefficient
	bool fixK2;                  // fix K2 distortion coefficient
	bool fixK3;                  // fix K3 distortion coefficient
	bool fixK4;                  // fix K4 distortion coefficient
	bool fixK5;                  // fix K5 distortion coefficient

	int cameraID;
	std::vector<std::string> imageList;
	size_t atImageList;
	cv::VideoCapture inputCapture;
	InputType inputType;
	bool goodInput;
	int flag;

private:
	std::string patternToUse;

};

//--------------------------------------------------

static inline void read(const cv::FileNode& node, Settings& x, const Settings& default_value = Settings())
{
	if ( node.empty() )
	{
		x = default_value;
	}
	else
	{
		x.read(node);
	}
}

//--------------------------------------------------

enum
{
	DETECTION = 0,
	CAPTURING = 1,
	CALIBRATED = 2
};

//--------------------------------------------------

bool runCalibrationAndSave(
	Settings& s,
	cv::Size imageSize,
	cv::Mat& cameraMatrix,
	cv::Mat& distCoeffs,
	std::vector<std::vector<cv::Point2f> > imagePoints,
	float grid_width,
	bool release_object
);

//--------------------------------------------------

int main(int argc, char* argv[])
{
	logmsg("main() begins.");
	const cv::String keys
		= "{help h usage ? |           | print this message            }"
		  "{@settings      |default.xml| input setting file            }"
		  "{d              |           | actual distance between top-left and top-right corners of "
		  "the calibration grid }"
		  "{winSize        | 11        | Half of search window for cornerSubPix }";

	cv::CommandLineParser parser(argc, argv, keys);

	parser.about("This is a camera calibration sample.\n"
				 "Usage: camera_calibration [configuration_file -- default ./default.xml]\n"
				 "Near the sample file you'll find the configuration file, which has detailed help of "
				 "how to edit it. It may be any OpenCV supported file format XML/YAML.");

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
		std::cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << std::endl;
		parser.printMessage();
		return -1;
	}
	fs["Settings"] >> s;
	fs.release();                                         // close Settings file
	//! [file_read]

	//FileStorage fout("settings.yml", FileStorage::WRITE); // write config as YAML
	//fout << "Settings" << s;

	if ( !s.goodInput )
	{
		std::cerr << "Invalid input detected. Application stopping." << std::endl;
		logmsg("main() ends.");
		return -1;
	}

	int winSize = parser.get<int>("winSize");
	logmsg("main() winSize = %d.", winSize);

	float grid_width = s.squareSize * (s.boardSize.width - 1);
	bool release_object = false;
	if ( parser.has("d") )
	{
		grid_width = parser.get<float>("d");
		release_object = true;
	}

	std::vector<std::vector<cv::Point2f> > imagePoints;
	cv::Mat cameraMatrix;
	cv::Mat distCoeffs;
	cv::Size imageSize;
	int mode = ( s.inputType == Settings::IMAGE_LIST ) ? CAPTURING : DETECTION;
	clock_t prevTimestamp = 0;
	const char ESC_KEY = 27;

	//! [get_input]
	logmsg("main() Getting input.");
	for(;;)
	{
		cv::Mat view;
		bool blinkOutput = false;

		size_t i = s.atImageList;
		view = s.nextImage();

		//-----  If no more image, or got enough, then stop calibration and show result -------------
		if ( mode == CAPTURING && imagePoints.size() >= (size_t)s.nrFrames )
		{
			logmsg("main() Calling runCalibrationAndSave().");
			if ( runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints, grid_width, release_object) )
			{
				logmsg("main() mode = CALIBRATED.");
				mode = CALIBRATED;
			}
			else
			{
				logmsg("main () mode = DETECTION.");
				mode = DETECTION;
			}
		}
		if ( view.empty() )          // If there are no more images stop the loop
		{
			// if calibration threshold was not reached yet, calibrate now
			if ( mode != CALIBRATED && !imagePoints.empty() )
			{
				logmsg("main() Calling runCalibrationAndSave().");
				runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints, grid_width, release_object);
			}
			break;
		}
		//! [get_input]

		imageSize = view.size();  // Format input image.
		if ( s.flipVertical )
		{
			cv::flip(view, view, 0);
		}

		//! [find_pattern]
		std::vector<cv::Point2f> pointBuf;

		bool found = false;

		int chessBoardFlags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE;

		if ( !s.useFisheye )
		{
			// fast check erroneously fails with high distortions like fisheye
			chessBoardFlags |= cv::CALIB_CB_FAST_CHECK;
		}

		switch ( s.calibrationPattern ) // Find feature points on the input format
		{
		case Settings::CHESSBOARD:
			{
				found = cv::findChessboardCorners(view, s.boardSize, pointBuf, chessBoardFlags);
				break;
			}
		case Settings::CIRCLES_GRID:
			{
				found = cv::findCirclesGrid(view, s.boardSize, pointBuf);
				break;
			}
		case Settings::ASYMMETRIC_CIRCLES_GRID:
			{
				found = cv::findCirclesGrid(view, s.boardSize, pointBuf, cv::CALIB_CB_ASYMMETRIC_GRID);
				break;
			}
		default:
			{
				logmsg("main() Unknown value for calibrationPattern.");
				found = false;
				break;
			}
		}
		//! [find_pattern]
		if ( found )
		{
			logmsg("main() s.imageList[%zu] (out of %d) = '%s' <-- success: usable frame", i, s.nrFrames, s.imageList[i].c_str());
		}
		else
		{
			logmsg("main() s.imageList[%zu] (out of %d) = '%s' <-- failure: unusable frame", i, s.nrFrames, s.imageList[i].c_str());
		}
		//! [pattern_found]
		if ( found )
		{
			// improve the found corners' coordinate accuracy for chessboard
			if ( s.calibrationPattern == Settings::CHESSBOARD )
			{
				cv::Mat viewGray;
				cv::cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);
				cv::cornerSubPix(viewGray, pointBuf,
					cv::Size(winSize, winSize),
					cv::Size(-1,-1),
					cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.0001)
				);
			}

			if ( mode == CAPTURING &&  // For camera only take new samples after delay time
				( !s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay * 1e-3 * CLOCKS_PER_SEC ) )
			{
				imagePoints.push_back(pointBuf);
				prevTimestamp = clock();
				blinkOutput = s.inputCapture.isOpened();
			}

			// Draw the corners.
			cv::drawChessboardCorners(view, s.boardSize, cv::Mat(pointBuf), found);

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
				result = cv::imwrite(outfilename, view);
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
		}
		//! [pattern_found]
		//----------------------------- Output Text ------------------------------------------------
		//! [output_text]
		if ( blinkOutput )
		{
			bitwise_not(view, view);
		}
		//! [output_text]
		//------------------------- Video capture  output  undistorted ------------------------------
		//! [output_undistorted]
		if ( mode == CALIBRATED && s.showUndistorted )
		{
			cv::Mat temp = view.clone();
			if ( s.useFisheye )
			{
				cv::fisheye::undistortImage(temp, view, cameraMatrix, distCoeffs);
			}
			else
			{
				cv::undistort(temp, view, cameraMatrix, distCoeffs);
			}
		}
		//! [output_undistorted]
		//------------------------------ Show image and check for input commands -------------------
		//! [await_input]
		cv::imshow("Image View", view);
		char key = (char)cv::waitKey(s.inputCapture.isOpened() ? 50 : s.delay);

		if ( key == ESC_KEY )
		{
			break;
		}

		if ( key == 'u' && mode == CALIBRATED )
		{
			s.showUndistorted = !s.showUndistorted;
		}

		if ( s.inputCapture.isOpened() && key == 'g' )
		{
			mode = CAPTURING;
			imagePoints.clear();
		}
		//! [await_input]
	}

	// -----------------------Show the undistorted image for the image list ------------------------
	//! [show_results]

	if ( s.inputType == Settings::IMAGE_LIST && s.showUndistorted )
	{
		cv::Mat view;
		cv::Mat rview;
		cv::Mat map1;
		cv::Mat map2;

		if ( s.useFisheye )
		{
			cv::Mat newCamMat;
			cv::fisheye::estimateNewCameraMatrixForUndistortRectify(
				cameraMatrix, distCoeffs, imageSize, cv::Matx33d::eye(), newCamMat, 1);
			cv::fisheye::initUndistortRectifyMap(
				cameraMatrix, distCoeffs, cv::Matx33d::eye(), newCamMat, imageSize, CV_16SC2, map1, map2);
		}
		else
		{
			cv::initUndistortRectifyMap(
				cameraMatrix, distCoeffs, cv::Mat(),
				cv::getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0), 
				imageSize, CV_16SC2, map1, map2);
		}

		for ( size_t i = 0; i < s.imageList.size(); i++ )
		{
			view = cv::imread(s.imageList[i], cv::IMREAD_COLOR);
			if ( view.empty() )
			{
				continue;
			}
			cv::remap(view, rview, map1, map2, cv::INTER_CUBIC);
			cv::imshow("Image View", rview);
			cv::waitKey(50);

			// Save the view to a file.
			const std::string& original_filename = s.imageList[i];
			std::size_t idx = original_filename.find_last_of(".");
			std::string temp_prefix = original_filename.substr(0, idx);
			std::string temp_suffix = original_filename.substr(idx + 1);
			std::string outfilename = temp_prefix + "-c." + temp_suffix;
			logmsg("main() outfilename = '%s'", outfilename.c_str());
			bool result = false;
			try
			{
				//logmsg("main() Calling cv::imwrite()");
				result = cv::imwrite(outfilename, rview);
			}
			catch (const cv::Exception& ex)
			{
				fprintf(stderr, "cv::imwrite() encountered an exception: %s\n", ex.what());
			}
			if ( !result )
			{
				logmsg("main() Could not save view to '%s'.", outfilename.c_str());
				cv::waitKey(0);
			}
		} // end for
	} // end if

	//! [show_results]

	logmsg("main() ends normally.");
	return 0;
}

//--------------------------------------------------

//! [compute_errors]
static double 
computeReprojectionErrors( 
	const std::vector<std::vector<cv::Point3f> >& objectPoints,
	const std::vector<std::vector<cv::Point2f> >& imagePoints,
	const std::vector<cv::Mat>& rvecs,
	const std::vector<cv::Mat>& tvecs,
	const cv::Mat& cameraMatrix,
	const cv::Mat& distCoeffs,
	std::vector<float>& perViewErrors,
	bool fisheye)
{
	std::vector<cv::Point2f> imagePoints2;
	size_t totalPoints = 0;
	double totalErr = 0;
	double err = 0;
	perViewErrors.resize(objectPoints.size());

	for ( size_t i = 0; i < objectPoints.size(); ++i )
	{
		if ( fisheye )
		{
			cv::fisheye::projectPoints(objectPoints[i], imagePoints2, rvecs[i], tvecs[i], cameraMatrix, distCoeffs);
		}
		else
		{
			cv::projectPoints(objectPoints[i], rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
		}
		err = cv::norm(imagePoints[i], imagePoints2, cv::NORM_L2);

		size_t n = objectPoints[i].size();
		perViewErrors[i] = (float) std::sqrt(err*err/n);
		totalErr        += err*err;
		totalPoints     += n;
	}

	return std::sqrt(totalErr/totalPoints);
}
//! [compute_errors]

//--------------------------------------------------

//! [board_corners]
static void 
calcBoardCornerPositions(
	cv::Size boardSize,
	float squareSize,
	std::vector<cv::Point3f>& corners,
	Settings::Pattern patternType /*= Settings::CHESSBOARD*/)
{
	corners.clear();

	switch(patternType)
	{
	case Settings::CHESSBOARD:
	case Settings::CIRCLES_GRID:
		{
			for ( int i = 0; i < boardSize.height; ++i )
			{
				for ( int j = 0; j < boardSize.width; ++j )
				{
					corners.push_back(cv::Point3f(j*squareSize, i*squareSize, 0));
				}
			}
			break;
		}
	case Settings::ASYMMETRIC_CIRCLES_GRID:
		{
			for ( int i = 0; i < boardSize.height; i++ )
			{
				for ( int j = 0; j < boardSize.width; j++ )
				{
					corners.push_back(cv::Point3f((2*j + i % 2)*squareSize, i*squareSize, 0));
				}
			}
			break;
		}
	default:
		{
			break;
		}
	}
}

//--------------------------------------------------

//! [board_corners]
static bool 
runCalibration(
	Settings& s,
	cv::Size& imageSize,
	cv::Mat& cameraMatrix,
	cv::Mat& distCoeffs,
	std::vector<std::vector<cv::Point2f> > imagePoints,
	std::vector<cv::Mat>& rvecs,
	std::vector<cv::Mat>& tvecs,
	std::vector<float>& reprojErrs,
	double& totalAvgErr,
	std::vector<cv::Point3f>& newObjPoints,
	float grid_width,
	bool release_object)
{
	//! [fixed_aspect]
	cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	if ( s.flag & cv::CALIB_FIX_ASPECT_RATIO )
	{
		cameraMatrix.at<double>(0,0) = s.aspectRatio;
	}
	//! [fixed_aspect]
	if ( s.useFisheye )
	{
		distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
	}
	else
	{
		distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
	}

	std::vector<std::vector<cv::Point3f> > objectPoints(1);
	calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0], s.calibrationPattern);
	objectPoints[0][s.boardSize.width - 1].x = objectPoints[0][0].x + grid_width;
	newObjPoints = objectPoints[0];

	objectPoints.resize(imagePoints.size(),objectPoints[0]);

	//Find intrinsic and extrinsic camera parameters
	double rms;

	if ( s.useFisheye )
	{
		cv::Mat _rvecs;
		cv::Mat _tvecs;

		rms = cv::fisheye::calibrate(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, _rvecs, _tvecs, s.flag);

		rvecs.reserve(_rvecs.rows);
		tvecs.reserve(_tvecs.rows);

		for ( int i = 0; i < int(objectPoints.size()); i++)
		{
			rvecs.push_back(_rvecs.row(i));
			tvecs.push_back(_tvecs.row(i));
		} // end for
	}
	else
	{
		int iFixedPoint = -1;
		if ( release_object )
		{
			iFixedPoint = s.boardSize.width - 1;
		}
		rms = calibrateCameraRO(
			objectPoints, imagePoints, imageSize, iFixedPoint,
			cameraMatrix, distCoeffs, rvecs, tvecs, newObjPoints,
			s.flag | cv::CALIB_USE_LU
		);
	}

	if ( release_object )
	{
		std::cout << "New board corners: " << std::endl;
		std::cout << newObjPoints[0] << std::endl;
		std::cout << newObjPoints[s.boardSize.width - 1] << std::endl;
		std::cout << newObjPoints[s.boardSize.width * (s.boardSize.height - 1)] << std::endl;
		std::cout << newObjPoints.back() << std::endl;
	}

	std::cout << "Re-projection error reported by calibrateCamera: "<< rms << std::endl;

	bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

	objectPoints.clear();
	objectPoints.resize(imagePoints.size(), newObjPoints);

	totalAvgErr = computeReprojectionErrors(
		objectPoints, imagePoints, rvecs, tvecs, cameraMatrix,
		distCoeffs, reprojErrs, s.useFisheye
	);

	return ok;
}

//--------------------------------------------------

// Print camera parameters to the output file
static void
saveCameraParams(
	Settings& s,
	cv::Size& imageSize,
	cv::Mat& cameraMatrix,
	cv::Mat& distCoeffs,
	const std::vector<cv::Mat>& rvecs,
	const std::vector<cv::Mat>& tvecs,
	const std::vector<float>& reprojErrs,
	const std::vector<std::vector<cv::Point2f> >& imagePoints,
	double totalAvgErr,
	const std::vector<cv::Point3f>& newObjPoints)
{
	logmsg("saveCameraParams() s.outputFileName = '%s'", s.outputFileName.c_str());
	cv::FileStorage fs( s.outputFileName, cv::FileStorage::WRITE );

	time_t tm;
	time( &tm );
	struct tm *t2 = localtime( &tm );
	char buf[1024];
	strftime( buf, sizeof(buf), "%c", t2 );

	fs << "calibration_time" << buf;

	if ( !rvecs.empty() || !reprojErrs.empty() )
	{
		fs << "nr_of_frames" << (int)std::max(rvecs.size(), reprojErrs.size());
	}
	fs << "image_width" << imageSize.width;
	fs << "image_height" << imageSize.height;
	fs << "board_width" << s.boardSize.width;
	fs << "board_height" << s.boardSize.height;
	fs << "square_size" << s.squareSize;

	if ( s.flag & cv::CALIB_FIX_ASPECT_RATIO )
	{
		fs << "fix_aspect_ratio" << s.aspectRatio;
	}

	if ( s.flag )
	{
		std::stringstream flagsStringStream;
		if ( s.useFisheye )
		{
			flagsStringStream << "flags:"
				<< (s.flag & cv::fisheye::CALIB_FIX_SKEW ? " +fix_skew" : "")
				<< (s.flag & cv::fisheye::CALIB_FIX_K1 ? " +fix_k1" : "")
				<< (s.flag & cv::fisheye::CALIB_FIX_K2 ? " +fix_k2" : "")
				<< (s.flag & cv::fisheye::CALIB_FIX_K3 ? " +fix_k3" : "")
				<< (s.flag & cv::fisheye::CALIB_FIX_K4 ? " +fix_k4" : "")
				<< (s.flag & cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC ? " +recompute_extrinsic" : "");
		}
		else
		{
			flagsStringStream << "flags:"
				<< (s.flag & cv::CALIB_USE_INTRINSIC_GUESS ? " +use_intrinsic_guess" : "")
				<< (s.flag & cv::CALIB_FIX_ASPECT_RATIO ? " +fix_aspectRatio" : "")
				<< (s.flag & cv::CALIB_FIX_PRINCIPAL_POINT ? " +fix_principal_point" : "")
				<< (s.flag & cv::CALIB_ZERO_TANGENT_DIST ? " +zero_tangent_dist" : "")
				<< (s.flag & cv::CALIB_FIX_K1 ? " +fix_k1" : "")
				<< (s.flag & cv::CALIB_FIX_K2 ? " +fix_k2" : "")
				<< (s.flag & cv::CALIB_FIX_K3 ? " +fix_k3" : "")
				<< (s.flag & cv::CALIB_FIX_K4 ? " +fix_k4" : "")
				<< (s.flag & cv::CALIB_FIX_K5 ? " +fix_k5" : "");
		}
		fs.writeComment(flagsStringStream.str());
	}

	fs << "flags" << s.flag;

	fs << "fisheye_model" << s.useFisheye;

	fs << "camera_matrix" << cameraMatrix;
	fs << "distortion_coefficients" << distCoeffs;

	fs << "avg_reprojection_error" << totalAvgErr;
	if ( s.writeExtrinsics && !reprojErrs.empty() )
	{
		fs << "per_view_reprojection_errors" << cv::Mat(reprojErrs);
	}

	if ( s.writeExtrinsics && !rvecs.empty() && !tvecs.empty() )
	{
		CV_Assert(rvecs[0].type() == tvecs[0].type());
		cv::Mat bigmat((int)rvecs.size(), 6, CV_MAKETYPE(rvecs[0].type(), 1));
		bool needReshapeR = rvecs[0].depth() != 1 ? true : false;
		bool needReshapeT = tvecs[0].depth() != 1 ? true : false;

		for ( size_t i = 0; i < rvecs.size(); i++ )
		{
			cv::Mat r = bigmat(cv::Range(int(i), int(i+1)), cv::Range(0,3));
			cv::Mat t = bigmat(cv::Range(int(i), int(i+1)), cv::Range(3,6));

			if ( needReshapeR )
			{
				rvecs[i].reshape(1, 1).copyTo(r);
			}
			else
			{
				//*.t() is MatExpr (not Mat) so we can use assignment operator
				CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
				r = rvecs[i].t();
			}

			if ( needReshapeT )
			{
				tvecs[i].reshape(1, 1).copyTo(t);
			}
			else
			{
				CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
				t = tvecs[i].t();
			}
		} //  end for
		fs.writeComment("a set of 6-tuples (rotation vector + translation vector) for each view");
		fs << "extrinsic_parameters" << bigmat;
	} // end if

	if ( s.writePoints && !imagePoints.empty() )
	{
		cv::Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
		for ( size_t i = 0; i < imagePoints.size(); i++ )
		{
			cv::Mat r = imagePtMat.row(int(i)).reshape(2, imagePtMat.cols);
			cv::Mat imgpti(imagePoints[i]);
			imgpti.copyTo(r);
		}
		fs << "image_points" << imagePtMat;
	}

	if ( s.writeGrid && !newObjPoints.empty() )
	{
		fs << "grid_points" << newObjPoints;
	}
}

//--------------------------------------------------

//! [run_and_save]
bool
runCalibrationAndSave(
	Settings& s,
	cv::Size imageSize,
	cv::Mat& cameraMatrix,
	cv::Mat& distCoeffs,
	std::vector<std::vector<cv::Point2f> > imagePoints,
	float grid_width,
	bool release_object)
{
	std::vector<cv::Mat> rvecs;
	std::vector<cv::Mat> tvecs;
	std::vector<float> reprojErrs;
	double totalAvgErr = 0;
	std::vector<cv::Point3f> newObjPoints;

	bool ok = runCalibration(s, imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs, reprojErrs,
							 totalAvgErr, newObjPoints, grid_width, release_object);
	std::cout << (ok ? "Calibration succeeded" : "Calibration failed")
		 << ". avg re projection error = " << totalAvgErr << std::endl;

	if ( ok )
	{
		saveCameraParams(s, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, reprojErrs, imagePoints,
						 totalAvgErr, newObjPoints);
	}
	return ok;
}
//! [run_and_save]

//--------------------------------------------------
// end of this file
//--------------------------------------------------
