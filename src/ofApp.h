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
    
    // initiate a videograbber object and objects to put image pixel data into
    ofVideoGrabber vidGrabber;
    ofPixels videoPixels;
    ofTexture videoTexture;
    
    int camWidth, camHeight, xSteps, ySteps, scanStyle, speed, seconds, minutes, hours, numOfSecs, numOfMins, numOfHours;
    string scanName;
    bool b_radial, b_drawCam, b_drawDots, b_smooth;
    float currTime;
    ofEasyCam cam;
    ofTrueTypeFont font;
    vector<ofTexture> minuteThumbs, hourThumbs;
    int hourWidth, minuteWidth, hourHeight, minuteHeight, thumbnailGutter ;
};
