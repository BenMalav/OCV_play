#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>

using namespace std;

struct Stopwatch
{
    Stopwatch(chrono::nanoseconds& res) : 
        result{res},
        clock_start{chrono::high_resolution_clock::now()} {}

    ~Stopwatch() 
    {
        result = chrono::high_resolution_clock::now() - clock_start;
    }

private:
    chrono::nanoseconds& result;
    const chrono::time_point<chrono::high_resolution_clock> clock_start;
};


cv::Mat getFrameDiff(cv::Mat prevFrame, cv::Mat curFrame, cv::Mat nextFrame)
{
    cv::Mat diffFrames1, diffFrames2, output;
    
    // Compute absolute difference between current frame and the next frame
    absdiff(nextFrame, curFrame, diffFrames1);

    // Compute absolute difference between current frame and the previous frame
    absdiff(curFrame, prevFrame, diffFrames2);

    // Bitwise "AND" operation between the above two diff images
    bitwise_and(diffFrames1, diffFrames2, output);

    return output;
}


cv::Mat getFrame(cv::VideoCapture cap, float scalingFactor)
{
    //float scalingFactor = 0.5;
    cv::Mat frame, output;

    // Capture the current frame
    cap >> frame;
    
    // Resize the frame
    resize(frame, frame, cv::Size(), scalingFactor, scalingFactor, cv::INTER_AREA);
    
    // Convert to grayscale
    cvtColor(frame, output, cv::COLOR_BGR2GRAY);
    
    return output;
}


int getFps(cv::VideoCapture& vc)
{
    double fps = vc.get(cv::CAP_PROP_FPS);

    int num_frames = 120;
    time_t start, end;

    cv::Mat frame;

    chrono::nanoseconds elapsed_ns;
    {
        Stopwatch watch(elapsed_ns);

        for (int i = 0; i < num_frames; i++)
        {
            vc >> frame;
        }
    }

    auto seconds = chrono::duration_cast<chrono::seconds>(elapsed_ns).count();

    fps  = num_frames / seconds;

    std::cout << "Estimated frames per second : " << fps << std::endl;

    return fps; 
}

int main() {
    cv::Mat frame, prevFrame, curFrame, nextFrame;
    cv::VideoCapture cap;

    bool isCamera = false;

    if (isCamera)
    {
        if (!cap.open("rtsp://admin:<PW>@192.168.1.114:554//h264Preview_01_main"))
        {
            std::cout << "Unable to open video capture\n";
            return -1;
        }
    }
    else
    {
        if (!cap.open("video.mp4")) 
        {
            std::cout << "Unable to open video capture\n";
            return -1;
        }
    }

    getFps(cap);

    // Scaling factor to resize the input frames from the webcam
    float scalingFactor = 0.75;
        
    prevFrame = getFrame(cap, scalingFactor);
    curFrame = getFrame(cap, scalingFactor);
    nextFrame = getFrame(cap, scalingFactor);

    while(true) 
    {
        cv::Mat frameDiff = getFrameDiff(prevFrame, curFrame, nextFrame);

        // Update the variables and grab the next frame
        prevFrame = curFrame;
        curFrame = nextFrame;
        nextFrame = getFrame(cap, scalingFactor);

            
        cv::imshow("frame", frameDiff);

        if (cv::waitKey(100) == 27) 
        {
            break; // stop capturing by pressing ESC
        }
    }

    return 0;
}
