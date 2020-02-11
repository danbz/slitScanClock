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
    //**********************************************
    // TO DO - set up with correct system time on startup
    //**********************************************
    
    sWidth = ofGetWidth();
    sHeight = ofGetHeight();
    seconds = minutes = hours = 0; // set start time
    numOfSecs = 60; // debug to allow running everything faster
    numOfMins = 60;
    numOfHours = 24;
    
    // calculate size of hour and minute thumbnails
    thumbnailGutter = 2;
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
    b_radial = b_drawDots = b_smooth = false;
    b_drawCam = false;
    b_remote = false;
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
    vidGrabber.setDeviceID(0); // set the ID of the camera we will use
    vidGrabber.setDesiredFrameRate(30); // set how fast we will grab frames from the camera
    vidGrabber.initGrabber(camWidth, camHeight); // set the width and height of the camera
    videoPixels.allocate(camWidth,camHeight, OF_PIXELS_RGB); // set up our pixel object to be the same size as our camera object
    videoTexture.allocate(videoPixels);
    ofSetVerticalSync(true);
    
    ofSetBackgroundColor(0, 0, 0); // set the background colour to dark black
    ofEnableDepthTest();
    ofDisableSmoothing();
    ofEnableAlphaBlending();
}

//--------------------------------------------------------------
void ofApp::update(){
    
//    vidGrabber.update();
//    ofPixels pixels = vidGrabber.getPixels();
    
   // if (b_remote){
        remoteImage.clear();
        remoteImage.load("http://live1.brownrice.com/cam-images/westland.jpg");
        ofPixels pixels = remoteImage.getPixels();
   // }
    
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
                ofColor color = pixels.getColor(x, ySteps); // get the pixels on line ySteps
                videoPixels.setColor(x, ySteps, color);
            }
            videoTexture.loadData(videoPixels);
            
            if ( ySteps >= camHeight ) {
                ySteps = 0; // if we are on the bottom line of the image then start at the top again
            }
            ySteps += speed; // step on to the next line. increase this number to make things faster
            break;
            
        case 3: // scan horizontal from centre
            for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line to draw new line at 0 in target image
                ofColor color = pixels.getColor( camWidth/2, y); // get the pixels on line ySteps
                videoPixels.setColor(1, y, color);
            }
            
            for (int x = camWidth; x>=0; x-= 1){
                for (int y=0; y<camHeight; y++ ) { // loop through all the pixels on a line
                    videoPixels.setColor(x, y, videoPixels.getColor( x-1, y )); // copy each pixel in the target to 1 pixel the right
                }
            }
            videoTexture.loadData(videoPixels);
            break;
            
        case 4: // scan vertical from centre
            for (int x=0; x<camWidth; x++ ) { // loop through all the pixels on a line to draw new column at 0 in target image
                ofColor color = pixels.getColor(x, camHeight/2); // get the pixels on line ySteps
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
            if (ofGetSystemTimeMillis() > currTime + 1000){ // one second has elapsed
                currTime = ofGetSystemTimeMillis();
                seconds ++;
                if (seconds >= numOfSecs){ // grab a minute chunk from the camera
                    // make a minute thumbnail and push into vector of minute thumbs
                    ofPixels newPixels;
                    newPixels = videoPixels; // load last minute's slitscan
                    newPixels.resize(minuteWidth, minuteHeight);
                    ofTexture newThumb;
                    newThumb.allocate(newPixels);
                    newThumb.loadData(newPixels);
                    minuteThumbs.push_back(newThumb);
                    seconds = 0;
                    minutes ++;
                }
                
                if (minutes >= numOfMins){ // grab an hour chunk from the camera
                    ofPixels newPixels;
                    newPixels = pixels; // grab a full frame from camera
                    // alternatively blend all minute frames together to make hour composite.
                    newPixels.resize(hourWidth, hourHeight);
                    ofTexture newThumb;
                    newThumb.allocate(newPixels);
                    newThumb.loadData(newPixels);
                    hourThumbs.push_back(newThumb);
                    
                    seconds = 0;
                    minutes = 0 ;
                    hours ++;
                    minuteThumbs.clear(); // empty the vector of minute thumbnails
                }
                
                if (hours >= numOfHours){ // grab a day chunk from the camera
                    hours = 0;
                    seconds = 0;
                    minutes = 0 ;
                    hourThumbs.clear(); // empty the vector of hour thumbnails
                }
                // count seconds
                for (int j=0; j < camWidth/numOfSecs; j++){
                    for (int y=0; y < camHeight; y++ ) { // loop through all the pixels on a line
                        ofColor color = pixels.getColor( j + (camWidth/numOfSecs * seconds), y); // get the pixels on line ySteps
                        videoPixels.setColor(j + (camWidth/numOfSecs * seconds), y, color);
                    }
                    xSteps ++; // step on to the next line. increase this number to make things faster
                }
                videoTexture.loadData(videoPixels);
                // every second step a lineChunk, every 60 seconds step a chunk, every 60 minutes step a block
                
                if ( xSteps >= camWidth ) {
                    xSteps = 0; // if we are on the end line of the image then start at the top again
                }
            }
            break;
            
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if (b_drawCam){
        vidGrabber.draw(sWidth-camWidth/4 -10, sHeight-camHeight/4 -10, camWidth/4, camHeight/4); // draw our plain image
        ofDrawBitmapString(" scanning " + scanName + " , press 1,2 or 3: for scantype, r: radial, c: camview, a: antialiased" , 10, sHeight -10);
    }
    
    if (scanStyle == 5){
        // draw clock time onscreen
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
    }
    
    if (b_radial){ // radial ribbon
        for (int i =0; i<videoTexture.getWidth(); i+=speed){
            ofPushMatrix();
            ofTranslate(sWidth/2, sHeight/2); // centre in right portion of screen
            ofRotateZDeg( ofMap(i, 0, videoTexture.getWidth()/speed, 0, 360));
            videoTexture.drawSubsection(0, 0, speed +2, videoTexture.getHeight(), i, 0);
            ofPopMatrix();
        }
    } else if (b_drawDots) {
        cam.begin();
        // added from moire test to draw a circles depth shifted
        ofPixels pixelsRef;
        videoTexture.readToPixels(pixelsRef);
        float lightness;
        ofColor colorC;
        float maxCols = 1.0;
        ofPushMatrix();
        ofRotateXDeg( 180);
        // ofRotateYDeg(180);
        //ofTranslate(sWidth, sHeight);
        for (int i = 0; i < camWidth; i+= 10){
            for (int j = 0; j < camHeight; j+= 10){
                lightness = pixelsRef.getColor(i,j).getLightness();
                colorC = pixelsRef.getColor(i, j);
                //colorC = (ofMap(colorC.r, 0, 255, 0, maxCols)* 255/maxCols,ofMap(colorC.g, 0, 255, 0, maxCols)* 255/maxCols,ofMap(colorC.b, 0, 255, 0, maxCols)* 255/maxCols) ;
                ofSetColor(colorC);
                ofDrawCircle(i-camWidth/2, j-camHeight/2, ofMap(lightness, 0, 255, 100, -100),ofMap(lightness, 0, 255, 1, 5));
            }
        }
        ofPopMatrix();
        cam.end();
        
    } else {
        
        if (minuteThumbs.size()>0){
            // draw minute thumbs
            for (int i =0; i < minuteThumbs.size(); i++){
                int x = thumbsMargin + ( (minuteWidth + thumbnailGutter) * (i % minuteThumbLineLength) );
                int y = thumbsMargin + ( ( hourHeight + thumbnailGutter) * 2) + ( ( i / minuteThumbLineLength) * (minuteHeight + thumbnailGutter) );
                minuteThumbs[i].draw(x ,y);
                //  cout << "drawing minute thumb: " << i << "at pos: " << x << ":" << y << endl;
            }
        }
        if (hourThumbs.size()>0){
            // draw hour thumbs
            for (int i =0; i < hourThumbs.size(); i++){
                int x = thumbsMargin + ( (hourWidth + thumbnailGutter) * (i % hourThumbLineLength) );
                int y = thumbsMargin + ( ( i / hourThumbLineLength) * (hourHeight + thumbnailGutter) );
                hourThumbs[i].draw(x ,y);
                // cout << "drawing hour thumb: " << i << "at pos: " << x << ":" << y << endl;
            }
        }
        // draw the seconds slitscan video texture we have constructed
        videoTexture.draw( 0, 0, sWidth, sHeight);
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
            
        case 'r':
            b_radial =!b_radial;
            b_drawDots = false;
            break;
            
        case 'c':
            b_drawCam =!b_drawCam;
            break;
            
        case 'd':
            b_drawDots =!b_drawDots;
            b_radial = false;
            break;
            
        case 'a':
            if (b_smooth){
                ofEnableSmoothing();
            } else {
                ofDisableSmoothing();
            }
            b_smooth =!b_smooth;
            break;
        case 'b':
            
            b_remote =!b_remote;
            break;
            
        default:
            break;
    }
}
