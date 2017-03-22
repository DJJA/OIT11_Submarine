// EV3 defines
#define MOTOR_LEFT motorB
#define MOTOR_RIGHT motorC
#define SENSOR_GYRO S2
#define SENSOR_ULTRASONIC S4
// Sonar defines
#define DISPLAY_ORIGIN_X 89
#define DISPLAY_ORIGIN_Y 64
#define SONAR_RADIUS 50
#define SONAR_ROTATION_SPEED 4
// Other defines
#define ATTACK_SPEED 10
#define WHEEL_DIAMETER 56	// milimeter
// Margins
#define MARGIN_DISTANCE 10
#define MARGIN_ANGLE 0

#define MAX_DISTANCE 30


struct ObjectLocation{
	long degrees;
	float distance;
};

void searchPhase();
bool isInRange(short iObjects, float currentDistance);
void attackPhase();
short getClosestLiveTarget();
bool isTargetEliminated(short iTarget);
float calculateDrivenDistance();


// Sonar functions
void drawSonar(long currentAngle);
void sonarSetPixel(short x, short y);
void sonarDrawLine(short x1, short y1, short x2, short y2);
short calculateDisplayX(short x);
short calculateDisplayY(short y);
short sonarCalculateX(long degrees, float distance);
short sonarCalculateY(long degrees, float distance);


// Vars
struct ObjectLocation mTargets[10];
short mTargetsDetected = 0;
short mEliminatedTargets[10];
short mEliminatedTargetsCount = 0;

/*
If inaccurate, use the average of the gyro and both big motors
If still inaccurate, check after turning if the object is there, if not scan for it (cone that gets bigger)
*/

task main(){
	//waitForButtonPress();
	// Search phase
	searchPhase();
	// Attack phase
	attackPhase();
	// Play triumphant sound
}

void searchPhase(){
	// Play sound searching
	//playSound(soundBeepBeep);
	// Reset the gyro, otherwise it won't be 0
	waitForButtonPress();
	displayCenteredBigTextLine(1, "Resetting gyro...");
	displayCenteredBigTextLine(3, "4");
	sleep(1000);
	resetGyro(SENSOR_GYRO);								// Might want to wait after reseting for more accuracy ...
	displayCenteredBigTextLine(3, "3");
	sleep(1000);
	getGyroRate(SENSOR_GYRO);
	displayCenteredBigTextLine(3, "2");
	sleep(1000);
	getGyroDegrees(SENSOR_GYRO);
	displayCenteredBigTextLine(3, "1");
	sleep(1000);
	eraseDisplay();
	//sleep(2000);
	//getGyroDegrees(SENSOR_GYRO);
	//sleep(2000);
	//getGyroDegrees(SENSOR_GYRO);
	//sleep(2000);
	// Turn motors on
	/*setMotorSpeed(MOTOR_LEFT, SONAR_ROTATION_SPEED);
	setMotorSpeed(MOTOR_RIGHT, SONAR_ROTATION_SPEED*-1);*/
	setMotorSync(MOTOR_LEFT, MOTOR_RIGHT, 100, SONAR_ROTATION_SPEED);
	// Start reading gyrosensor
	bool onObject = false;
	//int count = 0;
	float distance = getUSDistance(SENSOR_ULTRASONIC);
	long angle = getGyroDegrees(SENSOR_GYRO);
	while(mTargetsDetected < 10 && angle < 360)
	{
		// Read ultrasoon sensor out

		//sleep(1000);
		displayCenteredBigTextLine(1, "nTargets: %d", mTargetsDetected);
		//displayCenteredBigTextLine(1, "%d", count++);
		if(mTargetsDetected > 0)
		{
			displayCenteredBigTextLine(7, "L.ANGLE: %d", mTargets[mTargetsDetected-1].degrees);
			displayCenteredBigTextLine(9, "L.DISTANCE: %f", mTargets[mTargetsDetected-1].distance);
		}
		displayCenteredBigTextLine(3, "C.ANGLE: %d", angle);
		displayCenteredBigTextLine(5, "C.DISTANCE: %f", distance);

		//displayCenteredBigTextLine(3, "%f", distance);
		if(distance <= MAX_DISTANCE)	// Object is in attacking range
		{
			if(!onObject)	// We were not already seeing an object
			{
				onObject = true;
				mTargets[mTargetsDetected].degrees = getGyroDegrees(SENSOR_GYRO);
				mTargets[mTargetsDetected].distance = distance;
				mTargetsDetected++;
			}
			else
			{
				/*if(!isInRange(mTargetsDetected-1, distance))	// This object is too far away to be the same object, so it's a new one
				{
					mTargets[mTargetsDetected].degrees = getGyroDegrees(SENSOR_GYRO);
					mTargets[mTargetsDetected].distance = distance;
					mTargetsDetected = mTargetsDetected + 1;
				}*/
			}
		}
		else
		{
			if(onObject)
				onObject = false;
		}
		//drawSonar(getGyroDegrees(SENSOR_GYRO));
		//sleep(.5);
		angle = getGyroDegrees(SENSOR_GYRO);
		distance = getUSDistance(SENSOR_ULTRASONIC);
	}

	// Turn motors off
	/*setMotorSpeed(MOTOR_LEFT, 0);
	setMotorSpeed(MOTOR_RIGHT, 0);*/
	setMotorSync(MOTOR_LEFT, MOTOR_RIGHT, 0, 0);
	//displayString(7,"Done: %d", mTargetsDetected);
	// Draw the sonar with the objects on the screen
	drawSonar(-1);
	//waitForButtonPress();
	sleep(5000);

}

bool isInRange(short iObjects, float currentDistance)
{
	short margin = 5;	// Margin in cm
	if((currentDistance + margin) < mTargets[iObjects] || (currentDistance - margin) > mTargets[iObjects])
		return true;
	return false;
}

void attackPhase(){
	// reset mgyro
	sleep(1000);
	resetGyro(SENSOR_GYRO);
	sleep(1000);
	// Get closest "living" target
	short iTarget = getClosestLiveTarget();
	while(iTarget != -1){	// As long as a target is found
		// Play sound forward
		//playSound(soundBlip);
		// Turn to the object
		//		Turn on the motors
		displayCenteredBigTextLine(1, "Target %d / %d", mEliminatedTargetsCount+1, mTargetsDetected);
		displayCenteredBigTextLine(3, "ANGLE: %d", mTargets[iTarget].degrees);
		displayCenteredBigTextLine(7, "DIS: %f", mTargets[iTarget].distance);

		int rotationSpeed;
		// Determine turn direction
		if(getGyroDegrees(SENSOR_GYRO) < mTargets[iTarget].degrees)
			rotationSpeed = SONAR_ROTATION_SPEED;
		else
			rotationSpeed = -SONAR_ROTATION_SPEED;

		setMotorSync(MOTOR_LEFT, MOTOR_RIGHT, 100, rotationSpeed);

		float distance;
		long angle;
		int margin_angle = MARGIN_ANGLE;
		int reverseDirectionCount = 0;
		bool onObject = false;
		while(!onObject){
			distance = getUSDistance(SENSOR_ULTRASONIC);
			angle = getGyroDegrees(SENSOR_GYRO);
			displayCenteredBigTextLine(5, "C.ANGLE: %d", angle);
			displayCenteredBigTextLine(9, "C.DISTANCE: %d", distance);
			if(distance >= (mTargets[iTarget].distance - MARGIN_DISTANCE) && distance <= (mTargets[iTarget].distance + MARGIN_DISTANCE) &&		// Check if distance is correct with margin
			angle >= (mTargets[iTarget].degrees - margin_angle) && angle <= (mTargets[iTarget].degrees + margin_angle))							// Check if angle is correct with margin
				onObject = true;
			else if((rotationSpeed > 0 && (angle - margin_angle) > mTargets[iTarget].degrees) || (rotationSpeed < 0 && (angle + margin_angle) < mTargets[iTarget].degrees))	// If it's turning clockwise and the c.angle - margin is bigger than the object angle -> object passed
			{																																																																								// or if it's turning counterclockwise and the c.angle + margin is smaller than the object angle -> object passed
				// Turn into the other direction, we've passed the object
				rotationSpeed *= -1;
				setMotorSync(MOTOR_LEFT, MOTOR_RIGHT, 100, rotationSpeed);
				margin_angle = MARGIN_ANGLE + reverseDirectionCount;
				reverseDirectionCount++;
			}
		}
		//		Turn motors off
		/*setMotorSpeed(MOTOR_LEFT, 0);
		setMotorSpeed(MOTOR_RIGHT, 0);*/
		setMotorSync(MOTOR_LEFT, MOTOR_RIGHT, 0, 0);
		// Drive the measured distance
		//		Reset motor encoder
		sleep(1000);
		resetMotorEncoder(MOTOR_LEFT);
		resetMotorEncoder(MOTOR_RIGHT);
		sleep(1000);
		setMotorSpeed(MOTOR_LEFT, ATTACK_SPEED);
		setMotorSpeed(MOTOR_RIGHT, ATTACK_SPEED);
		//setMotorSync(MOTOR_LEFT, MOTOR_RIGHT, 0, ATTACK_SPEED);
		while(calculateDrivenDistance() < distance) {}
		//while(getUSDistance(SENSOR_ULTRASONIC) > 5){}
		setMotorSync(MOTOR_LEFT, MOTOR_RIGHT, 0, 0);
		// *Asume we've knocked over the target*
		// Drive back to origin
		/*setMotorSpeed(MOTOR_LEFT, -1*ATTACK_SPEED);
		setMotorSpeed(MOTOR_RIGHT, -1*ATTACK_SPEED);*/
		sleep(1000);
		setMotorSync(MOTOR_LEFT, MOTOR_RIGHT, 0, -ATTACK_SPEED); // set to -1* if error
		while(calculateDrivenDistance() > 0) {}
		//while(((getMotorEncoder(MOTOR_LEFT) + getMotorEncoder(MOTOR_RIGHT))/2) > 0){}
		setMotorSync(MOTOR_LEFT, MOTOR_RIGHT, 0, 0);
		// Add target to the eliminated targets list
		mEliminatedTargets[mEliminatedTargetsCount] = iTarget;
		mEliminatedTargetsCount++;
		iTarget = getClosestLiveTarget();
	}
}

float calculateDrivenDistance(){
	// Get degrees turned
	float degrees_left = getMotorEncoder(MOTOR_LEFT);
	float degrees_right = getMotorEncoder(MOTOR_RIGHT);
	float avg_rotations = (((degrees_left + degrees_right) / 2) / 360);
	float periphery_wheel = PI * WHEEL_DIAMETER;
	return (avg_rotations * periphery_wheel) / 10;	// Convert to centimeters
}

short getClosestLiveTarget(){
	short iClosestTarget = -1;
	short i;
	for(i=0;i<mTargetsDetected;i++)
	{
		if(!isTargetEliminated(i)){
			if(iClosestTarget == -1 || mTargets[i].distance < mTargets[iClosestTarget].distance)	// Should get to the second condition if the first is true
				iClosestTarget = i;
		}
	}
	return iClosestTarget;
}

bool isTargetEliminated(short iTarget){
	short i;
	for(i=0;i<mEliminatedTargetsCount;i++)
		if(iTarget == mEliminatedTargets[i])
			return true;
	return false;
}


void drawSonar(long currentAngle){
	// Clear the display
	eraseDisplay();
	// Draw the grid
	/*// -Origin cross
	sonarSetPixel(0,1);
	sonarSetPixel(0,-1);
	sonarSetPixel(1,0);
	sonarSetPixel(-1,0);*/
	// Origin arrow
	sonarSetPixel(2,2);
	sonarSetPixel(1,0);
	sonarSetPixel(-1,0);
	sonarSetPixel(-1,-1);
	sonarSetPixel(1,0);
	sonarSetPixel(1,-1);
	// -Axes boundries
	short p = 2, z = 2;
	sonarDrawLine(0, SONAR_RADIUS+1, 0, SONAR_RADIUS+1+z);
	sonarDrawLine(-1*p, SONAR_RADIUS+1, p, SONAR_RADIUS+1);
	sonarDrawLine(SONAR_RADIUS+1, 0, SONAR_RADIUS+1+z, 0);
	sonarDrawLine(SONAR_RADIUS+1, -1*p, SONAR_RADIUS+1, p);

	sonarDrawLine(0, -1*(SONAR_RADIUS+1), 0, -1*(SONAR_RADIUS+1+z));
	sonarDrawLine(-1*p, -1*(SONAR_RADIUS+1), p, -1*(SONAR_RADIUS+1));
	sonarDrawLine(-1*(SONAR_RADIUS+1), 0, -1*(SONAR_RADIUS+1+z), 0);
	sonarDrawLine(-1*(SONAR_RADIUS+1), -1*p, -1*(SONAR_RADIUS+1), p);
	// Draw the scanline
	if(currentAngle != -1)
		sonarDrawLine(0, 0, sonarCalculateX(currentAngle,SONAR_RADIUS), sonarCalculateY(currentAngle,SONAR_RADIUS));
	// Draw the objects
	short i;
	for(i=0;i<sizeof(mTargets)/sizeof(ObjectLocation);i++)
		sonarSetPixel(sonarCalculateX(mTargets[i].degrees, mTargets[i].distance), sonarCalculateY(mTargets[i].degrees, mTargets[i].distance));
}

void sonarSetPixel(short x, short y){
	setPixel(calculateDisplayX(x), calculateDisplayY(y));
}

void sonarDrawLine(short x1, short y1, short x2, short y2){
	drawLine(calculateDisplayX(x1), calculateDisplayY(y1), calculateDisplayX(x2), calculateDisplayY(y2));
}

short calculateDisplayX(short x){
	return x + DISPLAY_ORIGIN_X;
}

short calculateDisplayY(short y){
	return y + DISPLAY_ORIGIN_Y;
}

short sonarCalculateX(long degrees, float distance){
	if(degrees > 270){
		degrees = degrees - 270;
		return round(-1*distance*cos(degrees));
	}else if(degrees > 180){
		degrees = degrees - 180;
		return round(-1*distance*sin(degrees));
	}else if(degrees > 90){
		degrees = degrees - 90;
		return round(distance*cos(degrees));
	}
	else
		return round(distance*sin(degrees));
}

short sonarCalculateY(long degrees, float distance){
	if(degrees > 270){
		degrees = degrees - 270;
		return round(distance*sin(degrees));
	}else if(degrees > 180){
		degrees = degrees - 180;
		return round(-1*distance*cos(degrees));
	}else if(degrees > 90){
		degrees = degrees - 90;
		return round(-1*distance*sin(degrees));
	}
	else
		return round(distance*cos(degrees));
}
