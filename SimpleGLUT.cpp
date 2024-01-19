//#include "stdafx.h"
#include <glew.h>
#include "Interpolation.h"
// standard


#include <fstream>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

// glut
#include <string>
#include <vector>


#include "Quaternion.h"
#include "SOIL/SOIL.h"
#include "OpenFBX/ofbx.h"

struct UserInput
{
	double x;
	double y;
	double z;

	double xR;
	double yR;
	double zR;

	double qX;
	double qY;
	double qZ;
	double qW;
};
struct Texture
{
	unsigned char* texture;
	int width;
	int height;
	int channels;
};

//================================
// global variables
//================================
// screen size
int i = 1;

int g_screenWidth  = 100;
int g_screenHeight = 100;

// frame index
int g_frameIndex = 0;

// angle for rotation
int g_angle = 0;

double g_eularX = 0;
double g_eularY = 0;
double g_eularZ = 0;

double g_positionX = 0;
double g_positionY = 0;
double g_positionZ = 0;

double g_rotationX = 0;
double g_rotationY = 0;
double g_rotationZ = 0;

double dt;
int framesPerKeyframe;


UserInput* g_userInput;
int inputSize;


GLfloat* g_rotation;

int const legAngleFactor = 45;
double legTraverseFactor = 0.5;


double legAngleA = 0;
int legAngleFactorA = legAngleFactor;
double legAngleB = 0;
int legAngleFactorB = -legAngleFactor;

double legTraverseA = 0;
double legTraverseFactorA = legTraverseFactor;

double legTraverseB  = 0;
double legTraverseFactorB = -legTraverseFactor;

double movementDelta = 0;

SplineType g_splineType;
AngleInput g_angleInput;


Texture skyboxTextures[6];
Texture ground;

ofbx::IScene* cubeFBX = nullptr;


float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

bool initOpenFBX(const char* filepath) {
	static char s_TimeString[256];
	FILE* fp = fopen(filepath, "rb");

	if (!fp) return false;

	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	auto* content = new ofbx::u8[file_size];
	fread(content, 1, file_size, fp);

	
	// Ignoring certain nodes will only stop them from being processed not tokenised (i.e. they will still be in the tree)
	ofbx::LoadFlags flags =
//		ofbx::LoadFlags::IGNORE_MODELS |
		ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
		ofbx::LoadFlags::IGNORE_CAMERAS |
		ofbx::LoadFlags::IGNORE_LIGHTS |
//		ofbx::LoadFlags::IGNORE_TEXTURES |
		ofbx::LoadFlags::IGNORE_SKIN |
		ofbx::LoadFlags::IGNORE_BONES |
		ofbx::LoadFlags::IGNORE_PIVOTS |
//		ofbx::LoadFlags::IGNORE_MATERIALS |
		ofbx::LoadFlags::IGNORE_POSES |
		ofbx::LoadFlags::IGNORE_VIDEOS |
		ofbx::LoadFlags::IGNORE_LIMBS |
//		ofbx::LoadFlags::IGNORE_MESHES |
		ofbx::LoadFlags::IGNORE_ANIMATIONS;

	cubeFBX = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u16)flags);


	if(!cubeFBX) {
		printf(ofbx::getError());
		//OutputDebugString(ofbx::getError());
	}
	delete[] content;
	fclose(fp);
	return true;
}

void drawCube(float size) {
	glPushMatrix();
	glScalef(size,size,size);
	//FBX Format uses 4 indices for every square rather than 6. So we work accordingly
	const ofbx::Vec3* vertices = cubeFBX->getMesh(0)->getGeometry()->getGeometryData().getPositions().values;
	const GLint* indices = cubeFBX->getMesh(0)->getGeometry()->getGeometryData().getPositions().indices;
	glEnableClientState(GL_VERTEX_ARRAY);
	// Bind the vertex array
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	// Loop through each face and render it using the corresponding indices
	for (int i = 0; i < 6; i++) {
		glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, &indices[i * 4]);
		glShadeModel(GL_SMOOTH);
	}

	glDisableClientState(GL_VERTEX_ARRAY);



	glPopMatrix();
}

void drawSkybox()
{
	//Not exactly a sky box but since our camera did not need to rotate a backdrop seemed enough for now.
	glPushMatrix();
	glTranslatef(0, 1, -300);
	glScalef(1000, 60, 1);
	
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(
	GL_TEXTURE_2D,  // Target
0,              // Level of detail (0 for base level)
GL_RGBA,        // Internal format (depends on your image format)
skyboxTextures[0].width,          // Width of the texture
skyboxTextures[0].height,         // Height of the texture
0,              // Border (always 0)
GL_RGBA,        // Format of the pixel data in your image (depends on your image format)
GL_UNSIGNED_BYTE, // Data type of the pixel data
skyboxTextures[0].texture          // Pointer to the image data
);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glBegin(GL_QUADS);
	// Front face vertices and texture coordinates
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f,  1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f,  1.0f,  1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

}

void applyGroundTexture()
{
	glPushMatrix();
	GLuint textureID;
	
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Set texture wrapping to repeat in both directions
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S    , GL_REPEAT );
	glTexImage2D(
	GL_TEXTURE_2D,  // Target
0,              // Level of detail (0 for base level)
GL_RGBA,        // Internal format (depends on your image format)
ground.width,          // Width of the texture
ground.height,         // Height of the texture
0,              // Border (always 0)
GL_RGBA,        // Format of the pixel data in your image (depends on your image format)
GL_UNSIGNED_BYTE, // Data type of the pixel data
ground.texture          // Pointer to the image data
);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	int numTilesX = 1; // Adjust as needed
	int numTilesY = 1; // Adjust as needed

	glBegin(GL_QUADS);

	// Define the vertices and texture coordinates
	for (int i = 0; i < numTilesX; i++) {
		for (int j = 0; j < numTilesY; j++) {
			float tileSizeX = 10.0f / numTilesX;
			float tileSizeY = 10.0f / numTilesY;

			float x0 = i * tileSizeX;
			float x1 = (i + 1) * tileSizeX;
			float y0 = j * tileSizeY;
			float y1 = (j + 1) * tileSizeY;

			// Vertex 1
			glTexCoord2f(x0, y0);
			glVertex3f(-100.0f, -10.0f, -100.0f);

			// Vertex 2
			glTexCoord2f(x1, y0);
			glVertex3f(100.0f, -10.0f, -100.0f);

			// Vertex 3
			glTexCoord2f(x1, y1);
			glVertex3f(100.0f, -10.0f, 100.0f);

			// Vertex 4
			glTexCoord2f(x0, y1);
			glVertex3f(-100.0f, -10.0f, 100.0f);
		}
	}

	glEnd();
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
	
}


void getUserInputFromFile(const std::string& filename) {
	std::ifstream inputFile(filename);

	if (!inputFile.is_open()) {
		//std::cerr << "Error: Failed to open the input file." << std::endl;
		return;
	}

	inputFile>>dt;
	inputFile>>inputSize;

	framesPerKeyframe = static_cast<int>(1000.0)/dt;

	g_userInput = new UserInput[inputSize];

	std::string splineType;
	inputFile>>splineType;
	if(splineType=="Bezier")
	{
		g_splineType = Bezier;
	}
	else
	{
		g_splineType = CatMullRom;
	}

	std::string angleType;
	inputFile>>angleType;
	if(angleType=="Quaternion")
	{
		g_angleInput = Quaternions;
	}
	else
	{
		g_angleInput = Fixed;
	}

	for (int i = 0; i < inputSize; i++) {
		inputFile >> g_userInput[i].x >> g_userInput[i].y >> g_userInput[i].z;
		if(g_angleInput==Fixed)
		{
			inputFile >> g_userInput[i].xR >> g_userInput[i].yR >> g_userInput[i].zR;
			// Convert rotation angles to radians
			g_userInput[i].xR = g_userInput[i].xR * M_PI / 180.0;
			g_userInput[i].yR = g_userInput[i].yR * M_PI / 180.0;
			g_userInput[i].zR = g_userInput[i].zR * M_PI / 180.0;
		}
		else
		{
			inputFile >> g_userInput[i].qX >> g_userInput[i].qY >> g_userInput[i].qZ >> g_userInput[i].qW;
		}
	}
	
	inputFile.close();
}


void init( void ) {
	// init something before main loop...
}

void CatMullRomHandling(double t, int currentKeyframeIndex, int nextKeyframeIndex)
{
	// Perform Catmull-Rom interpolation for position
	double interpolatedX = Interpolation::CatmullRomInterpolation(t,
	                                               g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].x,
	                                               g_userInput[currentKeyframeIndex].x,
	                                               g_userInput[nextKeyframeIndex].x,
	                                               g_userInput[(nextKeyframeIndex + 1) % inputSize].x);

	double interpolatedY = Interpolation::CatmullRomInterpolation(t,
	                                               g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].y,
	                                               g_userInput[currentKeyframeIndex].y,
	                                               g_userInput[nextKeyframeIndex].y,
	                                               g_userInput[(nextKeyframeIndex + 1) % inputSize].y);

	double interpolatedZ = Interpolation::CatmullRomInterpolation(t,
	                                               g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].z,
	                                               g_userInput[currentKeyframeIndex].z,
	                                               g_userInput[nextKeyframeIndex].z,
	                                               g_userInput[(nextKeyframeIndex + 1) % inputSize].z);

	// Perform Catmull-Rom interpolation for rotation (in radians)
	if(g_angleInput==Fixed)
	{
		double interpolatedXRotation = Interpolation::CatmullRomAngleInterpolation(t,
		                                                            g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].xR,
		                                                            g_userInput[currentKeyframeIndex].xR,
		                                                            g_userInput[nextKeyframeIndex].xR,
		                                                            g_userInput[(nextKeyframeIndex + 1) % inputSize].xR);

		double interpolatedYRotation = Interpolation::CatmullRomAngleInterpolation(t,
		                                                            g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].yR,
		                                                            g_userInput[currentKeyframeIndex].yR,
		                                                            g_userInput[nextKeyframeIndex].yR,
		                                                            g_userInput[(nextKeyframeIndex + 1) % inputSize].yR);

		double interpolatedZRotation = Interpolation::CatmullRomAngleInterpolation(t,
		                                                            g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].zR,
		                                                            g_userInput[currentKeyframeIndex].zR,
		                                                            g_userInput[nextKeyframeIndex].zR,
		                                                            g_userInput[(nextKeyframeIndex + 1) % inputSize].zR);

		g_rotationX = interpolatedXRotation;
		g_rotationY = interpolatedYRotation;
		g_rotationZ = interpolatedZRotation;

		Quaternion q = eulerToQuaternion(g_rotationX, g_rotationY, g_rotationZ);
		g_rotation = quaternionToMatrix(q);
	}
	else
	{
		double interpolatedXRotation = Interpolation::CatmullRomAngleInterpolation(t,
		                                                            g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].qX,
		                                                            g_userInput[currentKeyframeIndex].qX,
		                                                            g_userInput[nextKeyframeIndex].qX,
		                                                            g_userInput[(nextKeyframeIndex + 1) % inputSize].qX);

		double interpolatedYRotation = Interpolation::CatmullRomAngleInterpolation(t,
		                                                            g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].qY,
		                                                            g_userInput[currentKeyframeIndex].qY,
		                                                            g_userInput[nextKeyframeIndex].qY,
		                                                            g_userInput[(nextKeyframeIndex + 1) % inputSize].qY);

		double interpolatedZRotation = Interpolation::CatmullRomAngleInterpolation(t,
		                                                            g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].qZ,
		                                                            g_userInput[currentKeyframeIndex].qZ,
		                                                            g_userInput[nextKeyframeIndex].qZ,
		                                                            g_userInput[(nextKeyframeIndex + 1) % inputSize].qZ);

		double interpolatedWRotation = Interpolation::CatmullRomAngleInterpolation(t,
		                                                            g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].qW,
		                                                            g_userInput[currentKeyframeIndex].qW,
		                                                            g_userInput[nextKeyframeIndex].qW,
		                                                            g_userInput[(nextKeyframeIndex + 1) % inputSize].qW);

		Quaternion q;
		q.x = interpolatedXRotation;
		q.y = interpolatedYRotation;
		q.z = interpolatedZRotation;
		q.w = interpolatedWRotation;
		g_rotation = quaternionToMatrix(q);
	}
    
	movementDelta = abs(((abs(interpolatedX-g_positionX)+abs(interpolatedZ-g_positionZ))/2.0));
	// Update the current position and rotation
	g_positionX = interpolatedX;
	g_positionY = interpolatedY;
	g_positionZ = interpolatedZ;
}

void BezierHandling(double t, int currentKeyframeIndex, int nextKeyframeIndex)
{
	double interpolatedX = Interpolation::BSplinePositionInterpolation(t,
	                                                    g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].x,
	                                                    g_userInput[currentKeyframeIndex].x,
	                                                    g_userInput[nextKeyframeIndex].x,
	                                                    g_userInput[(nextKeyframeIndex + 1) % inputSize].x);

	double interpolatedY = Interpolation::BSplinePositionInterpolation(t,
	                                                    g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].y,
	                                                    g_userInput[currentKeyframeIndex].y,
	                                                    g_userInput[nextKeyframeIndex].y,
	                                                    g_userInput[(nextKeyframeIndex + 1) % inputSize].y);

	double interpolatedZ = Interpolation::BSplinePositionInterpolation(t,
	                                                    g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].z,
	                                                    g_userInput[currentKeyframeIndex].z,
	                                                    g_userInput[nextKeyframeIndex].z,
	                                                    g_userInput[(nextKeyframeIndex + 1) % inputSize].z);
	
	if(g_angleInput==Fixed)
	{
		double interpolatedXRotation = Interpolation::BSplineAngleInterpolation(t,
		                                                         g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].xR,
		                                                         g_userInput[currentKeyframeIndex].xR,
		                                                         g_userInput[nextKeyframeIndex].xR,
		                                                         g_userInput[(nextKeyframeIndex + 1) % inputSize].xR);

		double interpolatedYRotation = Interpolation::BSplineAngleInterpolation(t,
		                                                         g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].yR,
		                                                         g_userInput[currentKeyframeIndex].yR,
		                                                         g_userInput[nextKeyframeIndex].yR,
		                                                         g_userInput[(nextKeyframeIndex + 1) % inputSize].yR);

		double interpolatedZRotation = Interpolation::BSplineAngleInterpolation(t,
		                                                         g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].zR,
		                                                         g_userInput[currentKeyframeIndex].zR,
		                                                         g_userInput[nextKeyframeIndex].zR,
		                                                         g_userInput[(nextKeyframeIndex + 1) % inputSize].zR);

		g_rotationX = interpolatedXRotation;
		g_rotationY = interpolatedYRotation;
		g_rotationZ = interpolatedZRotation;
		

		Quaternion q = eulerToQuaternion(g_rotationX, g_rotationY, g_rotationZ);
		g_rotation = quaternionToMatrix(q);
	}
	else
	{
		double interpolatedXRotation = Interpolation::BSplineAngleInterpolation(t,
		                                                         g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].qX,
		                                                         g_userInput[currentKeyframeIndex].qX,
		                                                         g_userInput[nextKeyframeIndex].qX,
		                                                         g_userInput[(nextKeyframeIndex + 1) % inputSize].qX);

		double interpolatedYRotation = Interpolation::BSplineAngleInterpolation(t,
		                                                         g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].qY,
		                                                         g_userInput[currentKeyframeIndex].qY,
		                                                         g_userInput[nextKeyframeIndex].qY,
		                                                         g_userInput[(nextKeyframeIndex + 1) % inputSize].qY);

		double interpolatedZRotation = Interpolation::BSplineAngleInterpolation(t,
		                                                         g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].qZ,
		                                                         g_userInput[currentKeyframeIndex].qZ,
		                                                         g_userInput[nextKeyframeIndex].qZ,
		                                                         g_userInput[(nextKeyframeIndex + 1) % inputSize].qZ);

		double interpolatedWRotation = Interpolation::BSplineAngleInterpolation(t,
		                                                         g_userInput[(currentKeyframeIndex - 1 + inputSize) % inputSize].qW,
		                                                         g_userInput[currentKeyframeIndex].qW,
		                                                         g_userInput[nextKeyframeIndex].qW,
		                                                         g_userInput[(nextKeyframeIndex + 1) % inputSize].qW);

		Quaternion q;
		q.x = interpolatedXRotation;
		q.y = interpolatedYRotation;
		q.z = interpolatedZRotation;
		q.w = interpolatedWRotation;
		g_rotation = quaternionToMatrix(q);
	}
	
	movementDelta = abs(((abs(interpolatedX-g_positionX)+abs(interpolatedZ-g_positionZ))/2.0));
	g_positionX = interpolatedX;
	g_positionY = interpolatedY;
	g_positionZ = interpolatedZ;
}

void update( void ) {
	

    double t = static_cast<double>(g_frameIndex) / framesPerKeyframe; // Assuming 60 frames per second

    // Ensure t is in the range [0, 1]
    t = std::min(1.0, std::max(0.0, t));

    // Determine the current and next keyframes
    int currentKeyframeIndex = i;
    int nextKeyframeIndex = (i + 1) % inputSize; // Wrap around to the first keyframe if needed




	if(g_splineType==CatMullRom)
	{
		CatMullRomHandling(t, currentKeyframeIndex, nextKeyframeIndex);
	}
	else
	{
		BezierHandling(t, currentKeyframeIndex, nextKeyframeIndex);
	}

	if(currentKeyframeIndex == 1)
	{
		if(g_frameIndex==1)
		{
			legAngleA=0;
			legAngleFactorA=legAngleFactor;

			legAngleB=0;
			legAngleFactorB=-legAngleFactor;

			legTraverseA = 0;
			legTraverseB = 0;

			legTraverseFactorA = legTraverseFactor;
			legTraverseFactorB = -legTraverseFactor;
		}
	}
	
	legAngleA += movementDelta*legAngleFactorA;
	if (abs(legAngleA) >= legAngleFactor) {
		if(legAngleA>legAngleFactor)
		{
			legAngleA = legAngleFactor;
		}
		else
		{
			legAngleA = -legAngleFactor;
		}
		
		legAngleFactorA*=-1;
	}

	legTraverseA += movementDelta*legTraverseFactorA;
	if (abs(legTraverseA) >= legTraverseFactor) {
		if(legTraverseA>legTraverseFactor)
		{
			legTraverseA = legTraverseFactor;
		}
		else
		{
			legTraverseA = -legTraverseFactor;
		}
		legTraverseFactorA*=-1;
	}

	legAngleB += movementDelta*legAngleFactorB;
	if (abs(legAngleB) >= legAngleFactor) {
		if(legAngleB>legAngleFactor)
		{
			legAngleB = legAngleFactor;
		}
		else
		{
			legAngleB = -legAngleFactor;
		}
		legAngleFactorB*=-1;
	}

	legTraverseB += movementDelta*legTraverseFactorB;
	if (abs(legTraverseB) >= legTraverseFactor) {
		if(legTraverseB>legTraverseFactor)
		{
			legTraverseB = legTraverseFactor;
		}
		else
		{
			legTraverseB = -legTraverseFactor;
		}
		legTraverseFactorB*=-1;
	}

    // Increment the frame index and wrap around if needed
	// We are treating the first and last frame as a phantom frame so we dont actually use that for the trajectory
    g_frameIndex++;
    if (g_frameIndex >= framesPerKeyframe) {
        g_frameIndex = 0;
    	i = i+1;
    	if(i==inputSize-2)
    	{
    		i=1;
    	}
    }
}

//================================
// render
//================================
void render( void ) {

	// clear buffer
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glClearDepth (1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	// render state
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);

	// enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// light source attributes
	GLfloat LightAmbient[]	= { 0.4f, 0.4f, 0.4f, 1.0f };
	GLfloat LightDiffuse[]	= { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat LightSpecular[]	= { 0.4f, 0.4f, 0.4f, 1.0f };
	GLfloat LightPosition[] = { 5.0f, 5.0f, 5.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT , LightAmbient );
	glLightfv(GL_LIGHT0, GL_DIFFUSE , LightDiffuse );
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	// surface material attributes
	GLfloat material_Ka[]	= { 0.11f, 0.06f, 0.11f, 1.0f };
	GLfloat material_Kd[]	= { 0.43f, 0.47f, 0.54f, 1.0f };
	GLfloat material_Ks[]	= { 0.33f, 0.33f, 0.52f, 1.0f };
	GLfloat material_Ke[]	= { 0.1f , 0.0f , 0.1f , 1.0f };
	GLfloat material_Se		= 10;

	glMaterialfv(GL_FRONT, GL_AMBIENT	, material_Ka);
	glMaterialfv(GL_FRONT, GL_DIFFUSE	, material_Kd);
	glMaterialfv(GL_FRONT, GL_SPECULAR	, material_Ks);
	glMaterialfv(GL_FRONT, GL_EMISSION	, material_Ke);
	glMaterialf (GL_FRONT, GL_SHININESS	, material_Se);


	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	//Applying camera transforms
	glTranslatef(0,-5,-30);
	glRotated(15.0,1,0,0);

	//Render Torso
	glPushMatrix();
	glTranslatef (g_positionX, g_positionY, g_positionZ);
	glMultMatrixf(g_rotation);
	glScalef(4,4,4);
	drawCube(0.5);

	//Setting up parent child hierarchy
	glPushMatrix();

	//Thigh1
	glTranslatef (0, -0.75+std::max(legTraverseA,0.0), -0.4);
	glRotatef(legAngleA,0,0,1);
	glScalef(0.4,1,0.5);
	drawCube(0.5);

	//Calf1
	glTranslatef (0, -0.9, 0);
	glRotatef(std::min(0.0,-legAngleA),0,0,1);
	glScalef(1,1,1);
	drawCube(0.5);
	glPopMatrix();

	//Thigh2
	glPushMatrix();
	glTranslatef (0, -0.75+std::max(legTraverseB,0.0), 0.4);
	glRotatef(legAngleB,0,0,1);
	glScalef(0.4,1,0.5);
	drawCube(0.5);

	//Calf2
	glTranslatef (0, -0.9, 0);
	glRotatef(std::min(0.0,-legAngleB),0,0,1);
	glScalef(1,1,1);
	drawCube(0.5);
	glPopMatrix();

	glPopMatrix();
	// disable lighting
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	//Render Ground and skybox
	applyGroundTexture();
	drawSkybox();
	// swap back and front buffers
	glutSwapBuffers();
}
//================================
// keyboard input
//================================
void keyboard( unsigned char key, int x, int y ) {
}

//================================
// reshape : update viewport and projection matrix when the window is resized
//================================
void reshape( int w, int h ) {
	// screen size
	g_screenWidth  = w;
	g_screenHeight = h;	
	
	// viewport
	glViewport( 0, 0, (GLsizei)w, (GLsizei)h );

	// projection matrix
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)w/(GLfloat)h, 1.0, 2000.0);
}


//================================
// timer : triggered every 16ms ( about 60 frames per second )
//================================
void timer( int value ) {	
	// increase frame index
	g_frameIndex++;

	update();
	
	// render
	glutPostRedisplay();

	// reset timer
	// 16 ms per frame ( about 60 frames per second )
	glutTimerFunc( 16, timer, 0 );
}

void loadTextures()
{
	skyboxTextures[0].texture = SOIL_load_image(
	R"(Skybox\test1.png)",  // Replace with the path to your image file
	&skyboxTextures[0].width, &skyboxTextures[0].height, &skyboxTextures[0].channels,
	SOIL_LOAD_RGBA    // Specify the desired image format (e.g., SOIL_LOAD_RGBA)
);

	if (!skyboxTextures[0].texture) {
		printf("SOIL loading error: %s\n", SOIL_last_result());
		// Handle the error (e.g., exit or display a message)
	}


	ground.texture = SOIL_load_image(
		R"(Skybox\dirt.png)",  // Replace with the path to your image file
		&ground.width, &ground.height, &ground.channels,
		SOIL_LOAD_RGBA    // Specify the desired image format (e.g., SOIL_LOAD_RGBA)
	);

	if (!ground.texture) {
		printf("SOIL loading error: %s\n", SOIL_last_result());
		// Handle the error (e.g., exit or display a message)
	}
}


//================================
// main
//================================
int main( int argc, char** argv ) {
	//Import fbx data using OpenFBX (We're using a simple cube fbx for the torso and legs and scaling it accordingly).
	initOpenFBX("Skybox/Cube.fbx");

	//Load all textures using SOIL
	loadTextures();
	
	//Get Input from file
	getUserInputFromFile("keyframes.txt");
	
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize( 600, 600 ); 
	glutInitWindowPosition( 100, 100 );
	glutCreateWindow( argv[0] );

	if(glewInit()!=GLEW_OK)
	{
		printf("NotOK");
	}

	g_positionX = g_userInput[1].x;
	g_positionY = g_userInput[1].y;
	g_positionZ = g_userInput[1].z;

	
	// init
	init();
	
	// set callback functions
	glutDisplayFunc( render );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutTimerFunc( 16, timer, 0 );
	
	// main loop
	glutMainLoop();
	
	return 0;
}