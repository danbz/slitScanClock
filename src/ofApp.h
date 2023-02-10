/*
 Project Title: SlitScanVideoCam
 Description:
 ©Daniel Buzzo 2022
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
    ofPixels videoPixels, pixels;
    ofTexture videoTexture;
    
    int xSteps, ySteps, scanStyle, speed, numFrames, maxFrames;
    int sWidth, sHeight;
    bool b_radial, b_drawCam, b_smooth;
    float currTime, camWidth, camHeight;
    string scanName, time;

    ofTrueTypeFont font;
    vector< ofPixels > frames;
   // vector< ofFbo > frames;

    ofImage mask;
    ofFbo maskFBO, layeredFBO;
};
