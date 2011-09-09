#include "testApp.h"

enum dataStingTypes{

	MINZ_RW,
	MINY_RW,
	COM_RW,
	MINZ_PW,
	MINY_PW,
	COM_PW,

};


//--------------------------------------------------------------
void testApp::setup() {

	isCloud			= true;
	isRawCloud	    = false;
	isFloor			= true;

	filterFactor = 0.1f;
	smoothFactor = 0.1f;
	yRot = 140;
	xRot = 0;
	zTrans = - 5000;
	yTrans = 0;
	viewScale = 2;
	pointProp = 0.25;
	eyeProp = 0.9375;
	isScreen = true;
	sternProp = 0.8;

	testBox.set(800,800,400);

	selectedUser = 0;
	numUsers = 0;
	currentUserId =0;

	#if defined(USE_FILE)
	fileName = "morePointing.oni";
	setupRecording(fileName);
	#else
	setupRecording();
	#endif


    userColors[0] = myCol(255,0,0);
    userColors[1] = myCol(0,255,0);
    userColors[2] = myCol(0,0,255);
    userColors[3] = myCol(255,255,0);
    userColors[4] = myCol(255,110,199);
    userColors[5] = myCol(86,26,139);
    userColors[6] = myCol(100,100,100);
    userColors[7] = myCol(100,100,100);
    userColors[8] = myCol(100,100,100);
    userColors[9] = myCol(100,100,100);

	screenZ = -3000;
	screenDims.set(0, 2000, 4000, 3000);
	spherePos.set(0,1500,0);
	sphereRad = 750;
	floorPlane.ptPoint.X = 0;
	floorPlane.ptPoint.Y = 0;
	floorPlane.ptPoint.Z = 0;
	floorPlane.vNormal.X = 0;
	floorPlane.vNormal.Y = 0;
	floorPlane.vNormal.Z = 0;

	setupGUI();







}

void testApp::setupRecording(string _filename) {

#if defined (TARGET_OSX) || defined(TARGET_LINUX)// only working on Mac/Linux at the moment (but on Linux you need to run as sudo...)
	if(fileName == ""){
	hardware.setup();				// libusb direct control of motor, LED and accelerometers
	hardware.setLedOption(LED_OFF); // turn off the led just for yacks (or for live installation/performances ;-)
	}
#endif


	if(fileName != ""){
	Context.setupUsingRecording(ofToDataPath(_filename));
	}else{
	Context.setup();
	}

	// all nodes created by code -> NOT using the xml config file at all
	//Context.setupUsingXMLFile();
	sceneGen.Create(Context.getXnContext());

	depthGen.setup(&Context);
	imageGen.setup(&Context);

	userGen.calibEnabled = false;
	userGen.setup(&Context);
	userGen.setSmoothing(filterFactor);				// built in openni skeleton smoothing...
	userGen.setUseMaskPixels(true);
	userGen.setUseCloudPoints(true);
	userGen.setMaxNumberOfUsers(10);					// use this to set dynamic max number of users (NB: that a hard upper limit is defined by MAX_NUMBER_USERS in ofxUserGenerator)
	userGen.addUserListener(this);


	//Context.toggleRegisterViewport();  //this fucks up realWorld proj
	Context.toggleMirror();

	//hardware.setTiltAngle(0);

}

void testApp::setupGUI(){

	//gui.loadFont("MONACO.TTF", 8);
	gui.setup("settings", 0, 0, ofGetWidth(), ofGetHeight());
	gui.addPanel("Kinect Settings", 4, false);
	gui.addPanel("RW Calibration", 4, false);
	gui.addPanel("Pointing Calibration", 4, false);
	gui.addPanel("Projection Screen", 4, false);

	gui.setWhichPanel(0);
	gui.setWhichColumn(0);

	gui.addSlider("tilt", "TILT", 0, -35, 35, false);

	vector <guiVariablePointer> t_vars;

	t_vars.push_back( guiVariablePointer("numUsers", &numUsers, GUI_VAR_INT) );
	t_vars.push_back( guiVariablePointer("currentUserId", &currentUserId, GUI_VAR_INT) );
	t_vars.push_back( guiVariablePointer("u_point", &minZ_rw_str, GUI_VAR_STRING ));
	t_vars.push_back( guiVariablePointer("u_height", &minY_rw_str, GUI_VAR_STRING ));
	t_vars.push_back( guiVariablePointer("rot_CoM_rw", &CoM_rw_str, GUI_VAR_STRING ));

	gui.addVariableLister("User Data", t_vars);

	vector<string> t_vec;
	t_vec.push_back("none");

	userSelector = gui.addTextDropDown("SelectUser", "SELECT_USER", 0, t_vec);

	gui.setWhichPanel(1);
	gui.setWhichColumn(0);

	gui.addSlider("YRot", "Y_ROT", yRot, 0,180,false);
	gui.addSlider("YTrans", "Y_TRANS", yTrans, -10000,10000,false);
	gui.addSlider("ZTrans", "Z_TRANS", zTrans, -10000,10000,false);
	gui.addSlider("viewScale", "VIEW_SCALE", viewScale, 1,4,false);

	vector <guiVariablePointer> f_vars;

	f_vars.push_back( guiVariablePointer("floorPoint.y", &floorPlane.ptPoint.Y, GUI_VAR_FLOAT) );
	f_vars.push_back( guiVariablePointer("correctionAngle", &correctAngle, GUI_VAR_FLOAT) );
	f_vars.push_back( guiVariablePointer("correctAxis.x", &correctAxis.x, GUI_VAR_FLOAT) );
	f_vars.push_back( guiVariablePointer("correctAxis.y", &correctAxis.y, GUI_VAR_FLOAT) );
	f_vars.push_back( guiVariablePointer("correctAxis.z", &correctAxis.z, GUI_VAR_FLOAT) );

	gui.addVariableLister("Floor Data", f_vars);

	gui.addToggle("find floor", "FIND_FLOOR", isFloor);

	gui.setWhichPanel(2);

	gui.addSlider("pointTestProp", "POINT_PROP", pointProp, 0.01, 1.0, false);
	gui.addSlider("eyeProp", "EYE_PROP", eyeProp, 0.01, 1.0, false);
	gui.addSlider("sternProp", "STERN_PROP", sternProp, 0.01, 1.0, false);

	gui.setWhichPanel(3);

	gui.addLabel("Screen Properties");

    scTog = gui.addToggle("ScreenOn", "SC_ON", true);
	gui.addSlider("ScreenX", "SC_X", screenDims.x , -10000, 10000, true);
	gui.addSlider("ScreenY", "SC_Y", screenDims.y , 0, 10000, true);
	gui.addSlider("ScreenW", "SC_W", screenDims.width, 1, 10000, true);
	gui.addSlider("ScreenH", "SC_H", screenDims.height, 1, 10000, true);
	gui.addSlider("ScreenZ", "SC_Z", screenZ , -10000, 2000, true);

    gui.addLabel("Sphere Properties");

    spTog = gui.addToggle("SphereOn", "SP_ON", false);
    gui.addSlider("SphereX", "SP_X", spherePos.x , -10000, 10000, false);
	gui.addSlider("SphereY", "SP_Y", spherePos.y , 0, 10000, false);
	gui.addSlider("SphereZ", "SP_Z", spherePos.z , -10000, 5000, false);
    gui.addSlider("SphereRad", "SP_RAD", sphereRad , 100, 3500, false);

	gui.addLabel("Adjust View");
	gui.addSlider("YRot", "Y_ROT", yRot, 0,360,false);
	gui.addSlider("XRot", "X_ROT", xRot, 0,360,false);

	gui.setupEvents();
	gui.enableEvents();

	ofAddListener(gui.guiEvent, this, &testApp::eventsIn);
}

void testApp::eventsIn(guiCallbackData & data){


	if(data.getXmlName() == "TILT"){
		hardware.setTiltAngle(data.getFloat(0));
	}else if(data.getXmlName() == "Y_ROT"){
		yRot = data.getFloat(0);
    }else if(data.getXmlName() == "X_ROT"){
		xRot = data.getFloat(0);
	}else if(data.getXmlName() == "Z_TRANS"){
		zTrans = data.getFloat(0);
	}else if(data.getXmlName() == "Y_TRANS"){
		yTrans = data.getFloat(0);
	}else if(data.getXmlName() == "POINT_PROP"){
		pointProp = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setPointProp(pointProp);
	}else if(data.getXmlName() == "EYE_PROP"){
		eyeProp = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setEyeProp(eyeProp);
    }else if(data.getXmlName() == "STERN_PROP"){
		sternProp = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setSternProp(sternProp);
	}else if(data.getXmlName() == "VIEW_SCALE"){
		viewScale = data.getFloat(0);
	}else if(data.getXmlName() == "FIND_FLOOR"){
		isFloor = data.getInt(0);
	}else if(data.getXmlName() == "SELECT_USER"){

        selectedUser = data.getInt(0) - 1;

		if(selectedUser >= 0){currentUserId = dsUsers[selectedUser]->id;}else{
		currentUserId = 0;
		}

    }else if(data.getXmlName() == "SC_ON"){
		isScreen = data.getInt(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->isScreen = isScreen;
		if(isScreen){spTog->setValue(0,0);}else{spTog->setValue(1,0);}
    }else if(data.getXmlName() == "SP_ON"){
		isScreen = !data.getInt(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->isScreen = isScreen;
		if(!isScreen){scTog->setValue(0,0);}else{scTog->setValue(1,0); }
	}else if(data.getXmlName() == "SC_Z"){
		screenZ = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setScreen(screenZ, screenDims);
	}else if(data.getXmlName() == "SC_X"){
		screenDims.x = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setScreen(screenZ, screenDims);
	}else if(data.getXmlName() == "SC_Y"){
		screenDims.y = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setScreen(screenZ, screenDims);
	}else if(data.getXmlName() == "SC_W"){
		screenDims.width = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setScreen(screenZ, screenDims);
	}else if(data.getXmlName() == "SC_H"){
		screenDims.height = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setScreen(screenZ, screenDims);
	}else if(data.getXmlName() == "SP_Z"){
		spherePos.z = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setSphere(spherePos, sphereRad);
	}else if(data.getXmlName() == "SP_X"){
		spherePos.x = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setSphere(spherePos, sphereRad);
	}else if(data.getXmlName() == "SP_Y"){
		spherePos.y = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setSphere(spherePos, sphereRad);
	}else if(data.getXmlName() == "SP_RAD"){
		sphereRad = data.getFloat(0);
		for(int i = 0; i < dsUsers.size(); i++)dsUsers[i]->setSphere(spherePos, sphereRad);
	}


}



//--------------------------------------------------------------
void testApp::update(){

	ofBackground(100, 100, 100);

#ifdef TARGET_OSX  // only working on Mac at the moment
	hardware.update();
#endif

	gui.update();

	// update all nodes
	Context.update();
	depthGen.update();
	imageGen.update();

	// update tracking nodes
	userGen.update();
	allUserMasks.setFromPixels(userGen.getUserPixels(), userGen.getWidth(), userGen.getHeight(), OF_IMAGE_GRAYSCALE);

	//update the users

	for(int  i = 0; i < dsUsers.size(); i++)dsUsers[i]->update();

	if(dsUsers.size() > 0 && currentUserId > 0){

		CoM_rw_str = dsUsers[selectedUser]->getDataStr(COM_RW);
		minZ_rw_str = dsUsers[selectedUser]->getDataStr(MINZ_RW);
		minY_rw_str = dsUsers[selectedUser]->getDataStr(MINY_RW);

	}

	//try to get the floor pane

	if(isFloor){

		XnStatus st = sceneGen.GetFloor(floorPlane);

		ofVec3f vN(floorPlane.vNormal.X, floorPlane.vNormal.Y, floorPlane.vNormal.Z);
		vN.normalize();
		ofVec3f yRef(0,1,0);
		correctAngle = yRef.angle(vN);
		correctAxis = vN.getCrossed(yRef);
		kinectPos.set(0,0,0);
		kinectPos.rotate(correctAngle, ofVec3f(floorPlane.ptPoint.X, floorPlane.ptPoint.Y, floorPlane.ptPoint.Z), correctAxis);



		for(int  i = 0; i < dsUsers.size(); i++){
			dsUsers[i]->setFloorPlane(ofVec3f(floorPlane.ptPoint.X, floorPlane.ptPoint.Y, floorPlane.ptPoint.Z),
								  correctAxis, correctAngle);
		}
	}




}

//--------------------------------------------------------------
void testApp::draw(){

	ofSetColor(255,255,255);
	gui.draw();


	if(gui.getSelectedPanel() == 0){

		glPushMatrix();
		glTranslatef(400, 70, 0);
		glScalef(0.7, 0.7, 1);

		imageGen.draw(0, 0, 640, 480); // first draw the image
        depthGen.draw(0,480, 640, 480);

		if(dsUsers.size() > 0 && currentUserId > 0){

			ofEnableAlphaBlending();
			ofSetColor(userColors[selectedUser].red,userColors[selectedUser].blue,userColors[selectedUser].green,50);
			dsUsers[selectedUser]->drawMask(ofRectangle(0,0,640,480)); //then mask over
			ofDisableAlphaBlending();


		}else{

			//draw masks and features for all users

            ofEnableAlphaBlending();
            for(int i=0; i < numUsers; i++){
			ofSetColor(userColors[dsUsers[i]->id].red,userColors[dsUsers[i]->id].blue,userColors[dsUsers[i]->id].green,50);
			dsUsers[i]->drawMask(ofRectangle(0,0,640,480)); //then mask over
            }
			ofDisableAlphaBlending();

		}

		glPopMatrix();


	}else if(gui.getSelectedPanel() == 1){

		draw3Dscene();

	}else if(gui.getSelectedPanel() == 2){

		draw3Dscene();

	}else if(gui.getSelectedPanel() == 3){

		draw3Dscene(true);
	}

}


void testApp::draw3Dscene(bool drawScreen){

	glPushMatrix();
	glTranslatef(ofGetWidth()/2,ofGetHeight()/2, 0);
	glScalef(0.1, 0.1, 0.1);
	glTranslatef(0,yTrans,zTrans);
	glRotatef(yRot,0,1,0);
	glRotatef(xRot,1,0,0);
	ofSetColor(255);
	ofNoFill();

	glPushMatrix();
	ofBox(0, 0, 0, 10000);
	glPopMatrix();

	glTranslatef(0, 5000, 5000);

	ofBox(kinectPos.x, -(kinectPos.y - floorPlane.ptPoint.Y) * viewScale, -kinectPos.z, 150); //the origin aka(kinect)

	//drawfloor Plane and normal

	if(isRawCloud){

		ofLine(floorPlane.ptPoint.X *viewScale, 0, -5000 * viewScale,
			   floorPlane.ptPoint.X *viewScale, 0, 0
			   );

		ofLine(-5000, 0, -floorPlane.ptPoint.Z * viewScale,
			   5000, 0, -floorPlane.ptPoint.Z * viewScale
			   );

		ofLine(floorPlane.ptPoint.X * viewScale - (floorPlane.vNormal.X*1000),
			   floorPlane.vNormal.Y * 1000,
			   -(floorPlane.ptPoint.Z * viewScale - (floorPlane.vNormal.Z*1000)),

			   floorPlane.ptPoint.X * viewScale + (floorPlane.vNormal.X*1000),
			   -(floorPlane.vNormal.Y * 1000),
			   -(floorPlane.ptPoint.Z * viewScale + (floorPlane.vNormal.Z*1000))
			   );

	}else{

		ofSetColor(100);

		for(int i = 0; i < 9; i++){

			int x = 4000 - (1000 * i);
			int z = 9000 - (1000 * i);
			ofLine(x , 0, 0, x, 0, -10000);
			ofLine(-5000 , 0, -z, 5000, 0, -z);
		}

	}

    if(isScreen){

		glPushMatrix();
		ofSetColor(255);
		glTranslatef(0, 0, -screenZ * viewScale);

		ofSetRectMode(OF_RECTMODE_CENTER);
		ofRect(screenDims.x * viewScale, -(screenDims.y -floorPlane.ptPoint.Y) * viewScale,
												screenDims.width * viewScale,
												screenDims.height * viewScale);
		ofSetRectMode(OF_RECTMODE_CORNER);

		for(int i =0; i < dsUsers.size(); i++){
			dsUsers[i]->drawIntersect(viewScale);
		}

		glPopMatrix();

	}else{


        glPushMatrix();
        glScalef(viewScale,viewScale,viewScale);
        glTranslatef(spherePos.x, -(spherePos.y - floorPlane.ptPoint.Y), -spherePos.z);

        ofNoFill();
        ofSetColor(100,100,100);
        ofSetLineWidth(1);
        ofSphere(0, 0, 0, sphereRad);

        glPopMatrix();

        for(int i =0; i < dsUsers.size(); i++){
			dsUsers[i]->drawSphereIntersect(viewScale);
		}


	}
	if(dsUsers.size() > 0 && currentUserId > 0){


		if(isCloud)dsUsers[selectedUser]->drawPointCloud(viewScale, true, userColors[selectedUser]);
		if(isRawCloud)dsUsers[selectedUser]->drawPointCloud(viewScale, false);
		dsUsers[selectedUser]->drawRWFeatures(viewScale, true);

	}else{

		for(int i = 0; i < dsUsers.size(); i++){
		    if(isCloud)dsUsers[i]->drawPointCloud(viewScale, true, userColors[dsUsers[i]->id]);
		    dsUsers[i]->drawRWFeatures(viewScale, true);
		}

	}

	glPopMatrix();
}


void testApp::onNewUser(int id)
{
	cout << "new dsUser added \n";
    bool nAdded = false;


    if(!nAdded){  //okay so now add a new one

	dsUser * newUser = new dsUser(id, &userGen, &depthGen);

		newUser->setFloorPlane(ofVec3f(floorPlane.ptPoint.X, floorPlane.ptPoint.Y, floorPlane.ptPoint.Z),
								 correctAxis, correctAngle);

	newUser->setEyeProp(eyeProp);
	newUser->setPointProp(pointProp);
	newUser->setSternProp(sternProp);
	newUser->setScreen(screenZ, screenDims);
	newUser->setSphere(spherePos, sphereRad);
	newUser->isScreen = isScreen;

	dsUsers.push_back(newUser);
	numUsers += 1;
	userSelector->vecDropList.push_back(ofToString(id, 0));
    }
}

void testApp::onLostUser(int id)
{
	cout << "dsUser removed \n";
	numUsers -= 1;


	for(int i = 0; i < userSelector->vecDropList.size(); i++){ //erase the menu reference

		if(ofToInt(userSelector->vecDropList[i]) == id){

			userSelector->vecDropList.erase(userSelector->vecDropList.begin() + i);

		}
    }

    for(int i = 0; i < dsUsers.size(); i ++){
		if(dsUsers[i]->id == id){

            delete dsUsers[i];
			dsUsers.erase(dsUsers.begin() + i);

		}
	}
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){

	switch (key) {


	}

}


//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){


}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	gui.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	gui.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	gui.mouseReleased();

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}
