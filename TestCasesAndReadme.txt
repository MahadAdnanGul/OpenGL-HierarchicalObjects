//Instructions
The input values are given in this order:
dt
number of inputs
Interpolation method (CatMullRom or Bezier (defaults to CR on anything else))
Angle Input Type (Quaternion vs Fixed) (Defaults to fixed on invalid)
posx posy posz (rotx roty rotz) (qx qy qz qw) (depending on which input was selected)

Copy paste one of the testcases into the keyframes file and run the program to see the results
Or you can setup the values in keyframes on your own too.

Project is currently setup for win32 debug mode. There may be dependancy issues if using any other mode.

Using the SOIL library for loading textures and using OPEN FBX for importing model data. All assets being used are in the skybox folder. Currently only using a simple cube fbx file as the torso and legs/legJoints and scaling it accordingly.


//Large Square
16
23
Bezier
Fixed
-12 0 -25	0 0 0

-12 0 -25	0 0 0

-6 0 -25	0 0 0
0 0 -25		0 0 0
6 0 -25		0 0 0	
12 0 -25	0 0 0	

12 0 -25	0 -90 0

12 0 -31	0 -90 0
12 0 -37	0 -90 0
12 0 -43	0 -90 0
12 0 -49	0 -90 0

12 0 -49	0 -180 0

6 0 -49		0 -180 0
0 0 -49		0 -180 0
-6 0 -49	0 -180 0
-12 0 -49	0 -180 0

-12 0 -49	0 -270 0

-12 0 -43	0 -270 0
-12 0 -37	0 -270 0
-12 0 -31	0 -270 0
-12 0 -25	0 -270 0

0 0 -25		0 -360 0

6 0 -25		0 -360 0



//ZIGZAG

16
10
Bezier
Fixed
-12 0 -25	0 0 0

-12 0 -25	0 -45 0

-6 0 -31	0 -45 0
-6 0 -31	0 -135 0
-12 0 -37	0 -135 0

-12 0 -37	0  45 0

-6 0 -31	0  45 0
-6 0 -31	0  135 0
-12 0 -25	0 135 0


-12 0 -25	0 135 0


//Diamond (Circle)
16
15
Bezier
Fixed

 0 0 -25	0  -45 0

 0 0 -25	0 -45 0

 6 0 -31	0  -45  0
 12 0 -37	0  -45  0
 18 0 -43	0  -135  0

 12 0 -49	0  -135 0
 6 0 -55	0  -135 0
 0 0 -61	0  -225 0

 -6 0 -55	0  -225 0
 -12 0 -49	0  -225 0
 -18 0 -43	0  -315 0

-12 0 -37	0  -315 0
-6 0 -31	0  -315 0
 0 0 -25	0  -405 0

 6 0 -31	0 -135 0


//Random

16
10
Bezier
Fixed
-20 0 0		0 0 0

-20 0 0		0 -45 0

 40 0 -90	0 -100 0
-20 0  -90	0 -180 0
-20 0 -0	0 -270 0

-12 0 -37	0  45 0

-6 0 -31	0  45 0
-6 0 -31	0  135 0
-12 0 -25	0 135 0


-12 0 -25	0 135 0


