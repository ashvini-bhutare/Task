
# Assignment 0 Description (5% of total grade)

**Due: January 29th**

----------------------------------

Introductory OpenGL assignment: The purpose of this assignment is to show your familiarity with OpenGL via performing some simple OpenGL tasks that will be used throughout the course.

## Minimum Requirements
* Draw a cube with a different color for each face. (15 points)
* Enable user initiated transformations via keyboard/mouse clicks. (30 points)
  * Rotation 
  * Scaling 
  * Translation
* Generate a checkerboard texture. (25 points)
* Apply texture to some/all of the faces of the mesh. (15 points)
* Enable switch between orthographic and perspective projections. (15 points)

## Sample Output
* Please find a sample output on this link: https://www.youtube.com/watch?v=5d6JBqpGnrE&feature=youtu.be


##############################################################

See basically you are rendering using OpenGL. You need something where you draw. 
In here it is "WINDOW" where you draw.

1. how screen is drawn?
==> here window is drawn using GLFW library functions. The rendering is done using OpenGL APIs/functions.

2. how to draw object in the scene?
==> to draw objects in OpenGL first you need to create the data on CPU and then send it to GPU shader
steps:

a. first you write shader and compile those shaders. these shaders will get compiled on GPU.
b. link shader program object
c. then you store the data in vertex buffer objects and send it to GPU.
d. by using shader program object you will draw the objects on screen.

in code : vertex array objects and vertex buffer objects.

3. how do you make an object rotates?
==> to make objects rotation. OpenGL uses the maths here. here we use Open GLM library to perform these operation
It has an roation funtion which creates roatio matrix.

4. how do you make an object translates?
==> similar like above glm has translate function which creates translation matrix.

5. how do you make an object scales?
==> similar like above glm has scale function which creates scaling matrix.

by using above matrix we create transformationMatrix which make objects perform rotaion, translate, scale.


6. how to apply a texture on an object?
==> to apply text

7. how to make camera in perspective projection?
==> perspective projection means how does your eyes look basicaally. it has an pyaramid view. to set projection in OpenGL. you need projection matrix.
this matrix is crearted by using glm::perspective() function. then we will set it to shader via unifrom;

8. how to make camera in orthographic projections?
==> ortho means how dooes it look in 2D view. it had an cuboid view.
to create orthographic projection matrix. here we have used glm::ortho() function. which will create orthographic matrix for use.
Then we will send these to shader via uniform.

##############################################################
#instuction to build program

the program is build up using the visual studio 2019 IDE [you could you any even 2022 latest].
You couls check video for instalon of visual studio on your machine. If you don't have install.

1. Select 

Solution Configuration : Debug
Solution Platform : 64 

to build program.

2. Got to Build -> Build Assignment It will build program. You will find Exe in Debug fodler of project directory
#instruction to run program 

1. Go to below github link and download the project.
https://github.com/ashvini-bhutare/Task/tree/main/01_assignment_0-SarahMutahar-master

2. open project in Visual studio solution
3. Run program.

##############################################################
follwoing keys are used:

O : for orthographic projection
P : for perspective projection(which is default one)

T - translate
S - scale
R - rotate

as per section of the above transaformation values [T/S/R keys]
you can use X, Y ,Z keys to translate, scale and rotate among all the axis

Escape: program will terminate.

##############################################################

NOTE : TO UNDERSTAND THE DEATILS IN DEPTH YOU CAN CHECK OPENGL PIPELINE.
