/*
 Project Title: SlitScan Clock
 Description: using oF video grabber to sample camera input then accessing pixels in each frame from the camera to create a
 slitscan video.
 each minute we grab the screen to create thumbnail images for the past hour and eacch hour we grab a thumbnail to show images for the last 24 hrs
 
 
 ©Daniel Buzzo 2020, 21, 22
 dan@buzzo.com
 http://buzzo.com
 https://github.com/danbz
 */
#include "ofApp.h"

/** ****************
 To do
 
 save each minute grab as image file
 save each frame as 1fps movie
 have alternative clock start time for 'local-time' 24 hr clock &
 as elapsed time journey clock
 
 ** set actual clock time
 ** onto buses and in corridor
 ******************/

//--------------------------------------------------------------
void ofApp::setup(){
    
    // set the width and height of the camera grabber to suit your particular camera
    // common sizes are listed below
    // camWidth =  640;  // try to grab at this size from the camera for Raspberry Pi
    //  camHeight = 480;
    camWidth =  1280;  // try to grab at this size from an apple HDwebcam camera.
    camHeight = 720;
    //    camWidth =  1280;  // try to grab at this size from a standard external 4x3 webcam camera.
    //    camHeight = 1024;
    //    camWidth= 1334; // stereo labs zed camera
    //    camWidth= 1280;
    
    float aspectRatio = camHeight / camWidth;
    cout << aspectRatio << " aspect ratio" << endl;
    
    sWidth = ofGetWidth();
    sHeight = ofGetHeight();
    numOfSecs = 59; // debug to allow running everything faster
    numOfMins = 59;
    numOfHours = 23;
    
    // calculate size of hour and minute thumbnails
    thumbnailGutter = 1;
    thumbsMargin = 10;
    hourThumbLineLength = 8;
    minuteThumbLineLength = 15;
    
    hourWidth = (sWidth - (hourThumbLineLength) * thumbnailGutter - thumbsMargin * 2) / hourThumbLineLength;
    minuteWidth = (sWidth - (minuteThumbLineLength) * thumbnailGutter - (thumbsMargin + hourWidth) * 2) / minuteThumbLineLength;
    hourHeight = hourWidth * aspectRatio;
    minuteHeight = minuteWidth * aspectRatio;
    xSteps = ySteps = 0;
    speed = 1;
    scanStyle = 5; // start as clock style
    scanName = "horizontal";
    b_radial = b_smooth = false;
    b_drawCam = false;
    b_thumbs = true;
    
    // load a custom truetype font as outline shapes to blend over video layer
    font.load("LiberationMono-Regular.ttf", 100, true, true, true);
    
    // ask the video grabber for a list of attached camera devices. & put it into a vector of devices
    vector<ofVideoDevice> devices = vidGrabber.listDevices();
    for(int i = 0; i < devices.size(); i++){ // loop through and print out the devices to the console log
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    vidGrabber.setDeviceID(0); // set the ID of the camera we will use
    vidGrabber.setDesiredFrameRate(30); // set how fast we will grab frames from the camera
    vidGrabber.initGrabber(camWidth, camHeight); // set the width and height of the camera
    videoPixels.allocate(camWidth,camHeight, OF_PIXELS_RGB); // set up our pixel object to be the same size as our camera object
    videoTexture.allocate(videoPixels);
    // ofSetVerticalSync(true);
    ofSetBackgroundColor(0, 0, 0); // set the background colour to dark black
    ofDisableSmoothing();
    vidGrabber.update();
    
    // set start time from system time
    seconds = ofGetSeconds();
    minutes = ofGetMinutes();
    hours = ofGetHours();
    calculateTime();
    // set up with correct amount of thumbs for hrs mins & seconds
    
    videoPixels = vidGrabber.getPixels(); // preload current video frame into thumbarrays to get over cold start empty thumbnail arrays
    update();
    for (int i = 0; i < minutes; i++){
        makeMinuteThumb();
    }
    
    for (int i = 0; i < hours; i++){
        makeHourThumb();
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    // update the video grabber object
    ofColor color;
    
    vidGrabber.update();    //check to see if there is a new frame from the camera to process, and if there is - process it.
    if (vidGrabber.isFrameNew()){
        pixels = vidGrabber.getPixels();
        pixels.mirror(false, true);
    }
    if (ofGetSeconds() > seconds){ // one second has elapsed
        seconds = ofGetSeconds();
        
        if (ofGetMinutes() >  minutes){ // grab a minute chunk from the camera
            makeMinuteThumb();
            minutes = ofGetMinutes();
        } else {
            if (ofGetMinutes() == 0){
                minutes = 0;
            }
        }
        
        if (ofGetHours() > hours){ // grab an hour chunk from the camera
            makeHourThumb();
            seconds = minutes = 0 ;
            hours = ofGetHours();
            minuteThumbs.clear(); // empty the vector of minute thumbnails
        } else {
            if (ofGetHours() == 0){
                hours = 0;
            }
        }
        
        if (numOfHours == 0){ // grab a day chunk from the camera
            hours = seconds = minutes = 0 ;
            hourThumbs.clear(); // empty the vector of hour thumbnails
        }
        
        calculateTime();
        
        if (scanStyle!=1) {
            xSteps = 0;
        }
        // step on to the next line. increase this number to make things faster
    } else {
        if (ofGetSeconds() == 0){
            seconds = 0;
            calculateTime();
        }
    }
        
    switch (scanStyle) {
        case 1: // scan horizontal
            for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line
                color = pixels.getColor( xSteps, y); // get the pixels on line ySteps
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
    
    if (b_thumbs) {
        /// new thumb code ///
        int x, y;
        int margin = 5 +  hourWidth/2;
        int top = margin -10;
        int bottom = ofGetHeight() - margin ;
        int right = ofGetWidth() - margin;
        int left = margin;
        int numThumbsH = 6;
        int numThumbsV = 6;
        int thumbSpaceH = ofGetWidth() - margin * 2;
        int thumbSpaceV = ofGetHeight() - margin * 2;
        int thumbHSpacingH = thumbSpaceH/numThumbsH;
        int thumbHSpacingV = thumbSpaceV/numThumbsV;
        
        // for (int i = 0; i < 24; i ++){
        for (int i = 0; i < hourThumbs.size(); i ++){
            
            if (i <4){ //set up hours on top right going right
                x = i * thumbHSpacingH + ofGetWidth()/2;
                y = top;
                
            } else if (i < 9){   //set up hours on right - going down
                x = right;
                y = ((i-3) * thumbHSpacingV) + margin;
                
            } else if ( i < 16){ //set up hours on bottom  - going left
                x = ofGetWidth() -  (i-9) * thumbHSpacingH - margin;
                y = bottom;
                
            } else if (i < 21){ //set up hours on left - going up
                x = left;
                y = ofGetHeight() - (i-15) * thumbHSpacingV - margin ;
                
            } else if (i <24){ //set up hours on top left - going right
                x= (i-21) * thumbHSpacingH + margin;
                y = top;
            }
            hourThumbs[i].draw(x ,y);
            ofPushStyle();
            ofSetColor(100);
            // ofDrawRectangle(x - 10, y - 10 , 20, 20);
            ofSetColor(255);
            ofDrawBitmapString(ofToString(i + 1), x ,  y);
            ofPopStyle();
        }
        // minute thumbs
        int minMargin = margin + hourWidth/2;
        int minTop = minMargin;
        int minBottom = ofGetHeight() - minMargin;
        int minRight = ofGetWidth() - minMargin;
        int minLeft = minMargin;
        int minNumThumbsH = 15;
        int minNumThumbsV = 15;
        int minThumbSpaceH = ofGetWidth() - minMargin * 2;
        int minThumbSpaceV = ofGetHeight() - minMargin * 2;
        int minThumbHSpacingH = minThumbSpaceH/minNumThumbsH;
        int minThumbHSpacingV = minThumbSpaceV/minNumThumbsV;
        // for (int i = 0; i < 60; i ++){
        for (int i = 0; i < minuteThumbs.size(); i ++){
            if (i <7){ //set up hours on top right going right
                x = i * minThumbHSpacingH + ofGetWidth()/2 + minThumbHSpacingH/2;
                y = minTop;
                
            } else if (i < 22){   //set up hours on right - going down
                x = minRight;
                y = ((i-7) * minThumbHSpacingV) + minMargin;
                
            } else if ( i < 37){ //set up hours on bottom  - going left
                x = ofGetWidth() -  (i-22) * minThumbHSpacingH - minMargin;
                y = minBottom;
                
            } else if (i < 52){ //set up hours on left - going up
                x = minLeft;
                y = ofGetHeight() - (i-37) * minThumbHSpacingV - minMargin ;
                
            } else if (i <60){ //set up hours on top left - going right
                x = (i-52) * minThumbHSpacingH + minMargin;
                y = minTop;
            }
            minuteThumbs[i].draw(x ,y);
            ofPushStyle();
            ofSetColor(150);
            //  ofDrawRectangle(x - 10, y - 10 , 20, 20);
            ofSetColor(255, 255, 0);
            ofDrawBitmapString(ofToString(i + 1), x ,  y);
            ofPopStyle();
        }
    }
    
    // display the time using our truetype font
    font.drawStringAsShapes( time, sWidth -650, sHeight -90);
    
    if (b_drawCam){ // draw camera debug to screen
        vidGrabber.draw(sWidth-camWidth/4 -10, sHeight-camHeight/4 -10, camWidth/4, camHeight/4); // draw our plain image
        ofDrawBitmapString(" scanning " + scanName + " , 1-scan horizontal 2-scan vertical 3-ribbon horizontal 4-ribbon vertical 5-slitscan clock, r-radial, c-camview, a-antialiased, FPS:" + ofToString(ofGetFrameRate()) , 10, sHeight -10);
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
            
        case 't':
            b_thumbs =!b_thumbs;
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

//--------------------------------------------------------------
void ofApp::makeMinuteThumb(){
    ofPixels newPixels;
    ofTexture newThumb;
    newPixels = videoPixels; // load last minute's slitscan
    newPixels.resize(minuteWidth, minuteHeight);
    newThumb.allocate(newPixels);
    newThumb.loadData(newPixels);
    newThumb.setAnchorPercent(0.5, 0.5); // make thumbnails draw from centre anchor point
    minuteThumbs.push_back(newThumb);
}

//--------------------------------------------------------------
void ofApp::makeHourThumb(){
    ofPixels newPixels;
    ofTexture newThumb;
    //newPixels = pixels; // grab a full frame from camera
    newPixels = videoPixels; // load last minute's slitscan
    // alternatively blend all minute frames together to make hour composite.
    newPixels.resize(hourWidth, hourHeight);
    newThumb.allocate(newPixels);
    newThumb.loadData(newPixels);
    newThumb.setAnchorPercent(0.5, 0.5); // make thumbnails draw from centre anchor point
    hourThumbs.push_back(newThumb);
}

//--------------------------------------------------------------
void ofApp::calculateTime(){
    // generate text and draw clock time onscreen
    string clockHours, clockMins, clockSecs;
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
}
