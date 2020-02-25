/*
 Project Title: SlitScan Clock
 Description:
 ©Daniel Buzzo 2020
 dan@buzzo.com
 http://buzzo.com
 https://github.com/danbz
 */
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    //    camWidth =  640;  // try to grab at this size from the camera.
    //    camHeight = 480;
    camWidth =  1280;  // try to grab at this size from an apple HDwebcam camera.
    camHeight = 720;
    //    camWidth =  1280;  // try to grab at this size from a standard external 4x3 webcam camera.
    //    camHeight = 1024;
    
    float aspectRatio = camHeight / camWidth;
    
        
    sWidth = ofGetWidth();
    sHeight = ofGetHeight();
    numOfSecs = 60; // debug to allow running everything faster
    numOfMins = 60;
    numOfHours = 24;
    
    // calculate size of hour and minute thumbnails
    
    thumbnailGutter = 4;
    thumbsMargin = 6;
    hourThumbLineLength = 12;
    minuteThumbLineLength = 15;
    hourWidth = (sWidth - (hourThumbLineLength +1 * thumbnailGutter) - thumbsMargin * 2) /hourThumbLineLength;
    minuteWidth = (sWidth - (minuteThumbLineLength +1 * thumbnailGutter) - thumbsMargin * 2) /minuteThumbLineLength;
    hourHeight = hourWidth * aspectRatio;
    minuteHeight = minuteWidth * aspectRatio;
    xSteps = ySteps = 0;
    speed = 1;
    scanStyle = 5; // start as clock style
    scanName = "horizontal";
    b_radial = b_smooth = false;
    b_drawCam = false;
    // load font as shapes to blend over video layer
    font.load("AkzidGroBol.ttf", 100, true, true, true);
    // font.load("LiberationMono-Regular.ttf", 50);
    
    // ask the video grabber for a list of attached camera devices. & put it into a vector of devices
    vector<ofVideoDevice> devices = vidGrabber.listDevices();
    for(int i = 0; i < devices.size(); i++){ // loop through and print out the devices to the console log
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    vidGrabber.setDeviceID(1); // set the ID of the camera we will use
    vidGrabber.setDesiredFrameRate(30); // set how fast we will grab frames from the camera
    vidGrabber.initGrabber(camWidth, camHeight); // set the width and height of the camera
    videoPixels.allocate(camWidth,camHeight, OF_PIXELS_RGB); // set up our pixel object to be the same size as our camera object
    videoTexture.allocate(videoPixels);
    ofSetVerticalSync(true);
    
    ofSetBackgroundColor(0, 0, 0); // set the background colour to dark black
    ofEnableDepthTest();
    ofDisableSmoothing();
    ofEnableAlphaBlending();
    
    vidGrabber.update();
    
    // set start time
    seconds = ofGetSeconds();
    minutes = ofGetMinutes();
    hours = ofGetHours();
    //**********************************************
    // TO DO - set up with correct amount of thumbs for hrs mins & seconds
    //**********************************************

    for (int i = 0; i <minutes; i++){
        makeMinuteThumb();
    }
    
    for (int i = 0; i <hours; i++){
        makeHourThumb();
    }

}

//--------------------------------------------------------------
void ofApp::update(){
    
    vidGrabber.update();
    ofPixels pixels = vidGrabber.getPixels();
    ofColor color;
    
    if (ofGetSystemTimeMillis() > currTime + 1000){ // one second has elapsed
        currTime = ofGetSystemTimeMillis();
        seconds ++;
        if (seconds >= numOfSecs){ // grab a minute chunk from the camera
            // make a minute thumbnail and push into vector of minute thumbs
            //            ofPixels newPixels;
            //            newPixels = videoPixels; // load last minute's slitscan
            //            newPixels.resize(minuteWidth, minuteHeight);
            //            ofTexture newThumb;
            //            newThumb.allocate(newPixels);
            //            newThumb.loadData(newPixels);
            //            minuteThumbs.push_back(newThumb);
            makeMinuteThumb();
            seconds = 0;
            minutes ++;
        }
        
        if (minutes >= numOfMins){ // grab an hour chunk from the camera
            //            ofPixels newPixels;
            //            newPixels = pixels; // grab a full frame from camera
            //            // alternatively blend all minute frames together to make hour composite.
            //            newPixels.resize(hourWidth, hourHeight);
            //            ofTexture newThumb;
            //            newThumb.allocate(newPixels);
            //            newThumb.loadData(newPixels);
            //            hourThumbs.push_back(newThumb);
            makeHourThumb();
            seconds = minutes = 0 ;
            hours ++;
            minuteThumbs.clear(); // empty the vector of minute thumbnails
        }
        
        if (hours >= numOfHours){ // grab a day chunk from the camera
            hours = seconds = minutes = 0 ;
            hourThumbs.clear(); // empty the vector of hour thumbnails
        }
        xSteps =0; // step on to the next line. increase this number to make things faster
    }
    
    switch (scanStyle) {
        case 1: // scan horizontal
            for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line
                ofColor color = pixels.getColor( xSteps, y); // get the pixels on line ySteps
                videoPixels.setColor(xSteps, y, color);
            }
            videoTexture.loadData(videoPixels);
            
            if ( xSteps >= camWidth ) {
                xSteps = 0; // if we are on the bottom line of the image then start at the top again
            }
            xSteps += speed; // step on to the next line. increase this number to make things faster
            break;
            
        case 2: // scan vertical
            for (int x=0; x<camWidth; x++ ) { // loop through all the pixels on a line
                color = pixels.getColor(x, ySteps); // get the pixels on line ySteps
                videoPixels.setColor(x, ySteps, color);
            }
            videoTexture.loadData(videoPixels);
            
            if ( ySteps >= camHeight ) {
                ySteps = 0; // if we are on the bottom line of the image then start at the top again
            }
            ySteps += speed; // step on to the next line. increase this number to make things faster
            break;
            
        case 3: // scan horizontal from centre
            if (xSteps < camWidth/numOfSecs){
                for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line to draw new line at 0 in target image
                    color = pixels.getColor( camWidth/2, y); // get the pixels on line ySteps
                    videoPixels.setColor(1, y, color);
                }
                
                for (int x = camWidth; x>=0; x-= 1){
                    for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line
                        videoPixels.setColor(x, y, videoPixels.getColor( x-1, y )); // copy each pixel in the target to 1 pixel the right
                    }
                }
            }
            videoTexture.loadData(videoPixels);
            xSteps++;
            break;
            
        case 4: // scan vertical from centre
            for (int x=0; x<camWidth; x++ ) { // loop through all the pixels on a line to draw new column at 0 in target image
                color = pixels.getColor(x, camHeight/2); // get the pixels on line ySteps
                videoPixels.setColor(x, 1, color);
            }
            
            for (int y = camHeight; y>=0; y-= 1){
                for (int x=0; x<camWidth; x++ ) { // loop through all the pixels on a column
                    videoPixels.setColor(x, y, videoPixels.getColor( x, y-1 )); // copy each pixel in the target to 1 pixel below
                }
            }
            videoTexture.loadData(videoPixels);
            break;
            
        case 5: // slitscan clock
            
            if (xSteps < camWidth/numOfSecs){
                for (int y=0; y < camHeight; y++ ) { // loop through all the pixels on a line
                    color = pixels.getColor( xSteps + (camWidth/numOfSecs * seconds), y); // get the pixels on line ySteps
                    videoPixels.setColor( xSteps + (camWidth/numOfSecs * seconds), y, color);
                }
            }
            videoTexture.loadData(videoPixels);
            xSteps++;
            break;
            
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if (b_drawCam){ // draw camera debug to screen
        vidGrabber.draw(sWidth-camWidth/4 -10, sHeight-camHeight/4 -10, camWidth/4, camHeight/4); // draw our plain image
        ofDrawBitmapString(" scanning " + scanName + " , press 1,2 or 3: for scantype, r: radial, c: camview, a: antialiased" , 10, sHeight -10);
    }
    
    // generate text and draw clock time onscreen
    string clockHours, clockMins, clockSecs, time;
    if (hours < 10){
        clockHours = "0" +ofToString(hours);
    }  else {
        clockHours = ofToString(hours);
    }
    
    if (minutes < 10){
        clockMins = "0" +ofToString(minutes);
    }  else {
        clockMins = ofToString(minutes);
    }
    
    if (seconds < 10){
        clockSecs = "0" +ofToString(seconds);
    }  else {
        clockSecs = ofToString(seconds);
    }
    time = clockHours + ":" + clockMins + ":" + clockSecs;
    font.drawStringAsShapes( time  , sWidth -550, sHeight -90);
    
    if (minuteThumbs.size()>0){ // draw minute thumbs to screen
        for (int i =0; i < minuteThumbs.size(); i++){
            int x = thumbsMargin + ( (minuteWidth + thumbnailGutter) * (i % minuteThumbLineLength) );
            int y = thumbsMargin + ( ( hourHeight + thumbnailGutter) * 2) + ( ( i / minuteThumbLineLength) * (minuteHeight + thumbnailGutter) );
            minuteThumbs[i].draw(x ,y);
        }
    }
    
    if (hourThumbs.size()>0){ // draw hour thumbs to screen
        for (int i =0; i < hourThumbs.size(); i++){
            int x = thumbsMargin + ( (hourWidth + thumbnailGutter) * (i % hourThumbLineLength) );
            int y = thumbsMargin + ( ( i / hourThumbLineLength) * (hourHeight + thumbnailGutter) );
            hourThumbs[i].draw(x ,y);
        }
    }
    
    if (b_radial){ // draw radial ribbon
        for (int i =0; i<videoTexture.getWidth(); i+=speed){
            ofPushMatrix();
            ofTranslate(sWidth/2, sHeight/2); // centre in right portion of screen
            ofRotateZDeg( ofMap(i, 0, videoTexture.getWidth()/speed, 0, 360));
            videoTexture.drawSubsection(0, 0, speed +2, videoTexture.getHeight(), i, 0);
            ofPopMatrix();
        }
    } else { // straight slices
        videoTexture.draw( 0, 0, sWidth, sHeight); // draw the seconds slitscan video texture we have constructed
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch (key) {
        case 'f':
            ofToggleFullscreen();
            break;
            
        case '1':
            scanStyle = 1;
            scanName = "horizontal";
            break;
        case '2':
            scanStyle = 2;
            scanName = "vertical";
            break;
        case '3':
            scanStyle = 3;
            scanName = "horizontal ribbon";
            break;
            
        case '4':
            scanStyle = 4;
            scanName = "vertical ribbon";
            break;
            
        case '5':
            scanStyle = 5;
            scanName = "let's be a slitscan clock";
            currTime = ofGetSystemTimeMillis();
            break;
            
        case 'c':
            b_drawCam =!b_drawCam;
            break;
            
        case 'r':
            b_radial =!b_radial;
            break;
            
        case 'a':
            if (b_smooth){
                ofEnableSmoothing();
            } else {
                ofDisableSmoothing();
            }
            b_smooth =!b_smooth;
            break;
            
        default:
            break;
    }
}


void ofApp::makeMinuteThumb(){
    ofPixels newPixels;
    newPixels = videoPixels; // load last minute's slitscan
    newPixels.resize(minuteWidth, minuteHeight);
    ofTexture newThumb;
    newThumb.allocate(newPixels);
    newThumb.loadData(newPixels);
    minuteThumbs.push_back(newThumb);
}

void ofApp::makeHourThumb(){
    ofPixels newPixels;
    //newPixels = pixels; // grab a full frame from camera
    newPixels = videoPixels; // load last minute's slitscan
    // alternatively blend all minute frames together to make hour composite.
    newPixels.resize(hourWidth, hourHeight);
    ofTexture newThumb;
    newThumb.allocate(newPixels);
    newThumb.loadData(newPixels);
    hourThumbs.push_back(newThumb);
}

