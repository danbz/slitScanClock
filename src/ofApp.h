/*
 Project Title: SlitScan Clock
 Description:
 ©Daniel Buzzo 2020
 dan@buzzo.com
 http://buzzo.com
 https://github.com/danbz
 */

#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{
    
public:
    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key); 
    
    void makeMinuteThumb();
    void makeHourThumb();
    void calculateTime();
    
    // initiate a videograbber object and objects to put image pixel data into
    ofVideoGrabber vidGrabber;
    ofPixels videoPixels, pixels;
    ofTexture videoTexture;
    
    int xSteps, ySteps, scanStyle, speed, seconds, minutes, hours, numOfSecs, numOfMins, numOfHours;
    int hourWidth, minuteWidth, hourHeight, minuteHeight, thumbnailGutter, thumbsMargin, hourThumbLineLength, minuteThumbLineLength;
    int sWidth, sHeight;
    bool b_radial, b_drawCam, b_smooth;
    float currTime, camWidth, camHeight;
    string scanName, time;

    ofTrueTypeFont font;
    vector<ofTexture> minuteThumbs, hourThumbs;
};
