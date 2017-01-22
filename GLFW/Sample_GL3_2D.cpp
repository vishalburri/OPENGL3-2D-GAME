#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<bits/stdc++.h>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;
void draw1(int i);
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 *************
 *************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float position1 = 0;
float position2 = 0;
float position3 = 0;
float position4 = 0;
float position6[10000]={0};
float position7[10000]={0};
float xpos=0;
float ypos=0;
float zoom=1;
float ctrl=0;
float alt=0;

int press=-1;
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float cirlce_rotation = 70;
float cirlce_rotation1 = 70;
float rectangle_rotation1 = 0;
int flag=0;
float pos[10000]={0};
int score=0;
int vis[10000]={0};
int k=0;
float position5[10000]={0};
int q=-1;
vector<float>quex;
vector<float>quey;
vector<int>quei;
float laserx[10000],lasery[10000];
float laserx1[10000],lasery1[10000];
int score2;
int dig=-1;
int ex=0,exred=0,exgreen=0;
int j=-1;
int random2[10000];
float posx[10000]={0};
int score1;
float reltime,curtime;
int lb=0,rb=0,gg=0;
int flag2=0,flag3=0;
int flag4=0;
float xcollide[10000],ycollide[10000];
int lmouse=0,rmouse=0;
int leftmove=0,rightmove=0,movepan=0,moverifle=0,movebullet=0;
float speed=1;
double last_update=glfwGetTime();
float u_time1[10000]={0};
float u_time2[10000]={0};
double utime3=0;
int flagp=0;
void initvar(){
	
	quex.clear();
	quey.clear();
	quei.clear();

	position1 = 0;
	position2 = 0;
	position3 = 0;
	position4 = 0;
	for(int i=0;i<10000;i++){
		position6[i]=0;
		position7[i]=0;
			pos[i]=0;
		position5[i]=0;
		vis[i]=0;
		xcollide[i]=0;
		ycollide[i]=0;
		u_time1[i]=0;
		u_time2[i]=0;
	}
	xpos=0;
	ypos=0;
	zoom=1;

	press=-1;
	flag=0;
	score=0;
	k=0;
	q=-1;
	dig=-1;
	exred=0,exgreen=0;
	j=-1;
	flag2=0,flag3=0;
	flag4=0;
	speed=1;
	last_update=glfwGetTime();
	flagp=0;

}

void mousezoom(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset==-1) { 
		zoom /= 1.1; 
	}
	else if(yoffset==1){
		zoom *= 1.1; 
	}
	if (zoom<=1) {
		zoom = 1;
	}
	if (zoom>=2) {
		zoom=2;
	}
	if(xpos-8.0f/zoom<-8)
		xpos=-8+8.0f/zoom;
	else if(xpos+8.0f/zoom>8)
		xpos=8-8.0f/zoom;
	if(ypos-4.0f/zoom<-4)
		ypos=-4+4.0f/zoom;
	else if(ypos+4.0f/zoom>4)
		ypos=4-4.0f/zoom;
	Matrices.projection = glm::ortho((float)(-8.0f/zoom+xpos), (float)(8.0f/zoom+xpos), (float)(-4.0f/zoom+ypos), (float)(4.0f/zoom+ypos), 0.1f, 500.0f);
}
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void pan(){
	if(xpos-8.0f/zoom<-8)
		xpos=-8+8.0f/zoom;
	else if(xpos+8.0f/zoom>8)
		xpos=8-8.0f/zoom;
	if(ypos-4.0f/zoom<-4)
		ypos=-4+4.0f/zoom;
	else if(ypos+4.0f/zoom>4)
		ypos=4-4.0f/zoom;
	Matrices.projection = glm::ortho((float)(-8.0f/zoom+xpos), (float)(8.0f/zoom+xpos), (float)(-4.0f/zoom+ypos), (float)(4.0f/zoom+ypos), 0.1f, 500.0f);

}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_RIGHT_CONTROL:
				ctrl=0;
				break;
			case GLFW_KEY_RIGHT_ALT:
				alt=0;
				break;
			case GLFW_KEY_LEFT_CONTROL:
				ctrl=0;
				break;
			case GLFW_KEY_LEFT_ALT:
				alt=0;
				break;
			default:
				break;
		}
		if(key==GLFW_KEY_RIGHT || key==GLFW_KEY_LEFT){
			lb=0;
			rb=0;
		}
		if(key==GLFW_KEY_S || key==GLFW_KEY_F)
			gg=0;
		if(key==GLFW_KEY_SPACE)
			flag2=0;


	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;

			default:
				break;
		}
		double current_time = glfwGetTime();
		if(key == GLFW_KEY_SPACE && (current_time-last_update) > 0.5){
			last_update = current_time;
			press++;
			flag3=1;
			position6[press]=position4;
			position7[press]=position3;
			//flag4=0;
			xcollide[press]=-7.6;
			ycollide[press]=0.55;
		}
		if(key==GLFW_KEY_RIGHT_CONTROL || key==GLFW_KEY_LEFT_CONTROL)
			ctrl=1;
		if(key == GLFW_KEY_RIGHT_ALT || key == GLFW_KEY_LEFT_ALT)
			alt=1;	
		if(ctrl==1 && key == GLFW_KEY_LEFT  && current_time-utime3>0.05){
			position1-=0.2;
			lb=-1;
			utime3=glfwGetTime();
		}



		if(ctrl==1 && key == GLFW_KEY_RIGHT && current_time-utime3>0.05){
			position1+=0.2;
			lb=1;
			utime3=glfwGetTime();
		}
		if(alt==1 && key == GLFW_KEY_LEFT && current_time-utime3>0.05){
			position2-=0.2;
			rb=-1;
			utime3=glfwGetTime();
		}
		if(alt==1 && key == GLFW_KEY_RIGHT && current_time-utime3>0.05){
			rb=1;
			position2+=0.2;
			utime3=glfwGetTime();
		}
		if(key == GLFW_KEY_S && current_time-utime3>0.05){
			position3-=0.2;
			gg=-1;
			utime3=glfwGetTime();
		}
		if(key== GLFW_KEY_F && current_time-utime3>0.05){
			position3+=0.2;
			gg=1;
			utime3=glfwGetTime();
		}
		if(key==GLFW_KEY_A && current_time-utime3>0.05){
			position4-=10;
			utime3=glfwGetTime();
		}
		if(key==GLFW_KEY_D && current_time-utime3>0.05){
			position4+=10;
			utime3=glfwGetTime();
		}
		if(key==GLFW_KEY_UP)
			mousezoom(window,0,+1);
		if(key==GLFW_KEY_DOWN)
			mousezoom(window,0,-1);
		if(key==GLFW_KEY_RIGHT){
			xpos+=0.2;
			pan();
		}
		if(key==GLFW_KEY_LEFT){
			xpos-=0.2;
			pan();
		}
		if(key==GLFW_KEY_Z){
			ypos+=0.2;
			pan();
		}
		if(key==GLFW_KEY_X){
			ypos-=0.2;
			pan();
		}
		if(key==GLFW_KEY_M && current_time-utime3>0.05){
			speed*=1.1;
		}
		if(key==GLFW_KEY_N && current_time-utime3>0.05){
			speed/=1.1;
		}
		if(key==GLFW_KEY_ENTER && ex==1){
			initvar();
			ex=0;
		}

	}

}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				triangle_rot_dir *= -1;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if(GLFW_PRESS == action)
			lmouse = 1;
		else if(GLFW_RELEASE == action){
			lmouse = 0;
			leftmove=0;
			rightmove=0;
			moverifle=0;
		}
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if(GLFW_PRESS == action)
			rmouse = 1;
		else if(GLFW_RELEASE == action)
			rmouse = 0;
	}
	if(lmouse==1){
		double lx;
		double ly;
		glfwGetCursorPos(window, &lx, &ly);
		if(ex==1 && lx>700 && lx<890 && ly>405 && ly<474){
			//ex=0;
			initvar();
			ex=0;

		}
		else if(ex==0){

			if(lx<1050 && lx>1020 && ly>20 && ly<50){
				if(flagp==0)
				flagp=1;
				else
					flagp=0;
			}
			else if((lx < (-3.15+position1)*94+50+94*8) && ((-3.15+position1)*94-25+94*8) <lx && ly>600){
				leftmove=1;
				rightmove=0;
				moverifle=0;
			}
			else if(ly>600 && lx < (2.85+position2)*94+50+94*8 && lx>(2.85+position2)*94-25+94*8){
				leftmove=0;
				rightmove=1;
				moverifle=0;
			}
			else if(lx>0 && lx<200 && ly<600){
				moverifle=1;
				leftmove=0;
				rightmove=0;
			}
			else if(ly<600 && lx>200){
				leftmove=0;
				rightmove=0;
				moverifle=0;
				press++;
				flag3=1;
				position4=-1*atan((ly-330+position3*110)/(lx-30))*(180/M_PI);
				position6[press]=position4;
				position7[press]=position3;
				xcollide[press]=-7.6;
				ycollide[press]=0.55;
			}
		}

	}


}
void drag (GLFWwindow* window){
	double lx;
	double ly;
	glfwGetCursorPos(window, &lx, &ly);
	if(leftmove==1){
		position1=((lx-94*8)/94)+3.15;
	}
	if(rightmove==1){
		position2=((lx-94*8)/94)-2.85;
	}
	if(rmouse==1){
		xpos=((lx-94*8)/394);
		ypos=-1*((ly-100*4)/400);
		pan();
	}
	if(moverifle==1){
		position3=-1*((ly-100*6)/100)-2.65;
	}
	// printf("%lf %lf %f\n",lx,ly,position1);

}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-8.0f, 8.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle,*tri,*play, *circle,*pause1,*pause2, *circle1,*circle41,*restart,*pause, *rectangle1,*rectang,*laser[10000],*level[7],*segment[7],*scoredis[7], *circle2, *circle3, *rectangle2, *rectangle3, *rectangle4, *rectangle5, *rectangle6, *rectangle7, *circle4, *circle5, *line, *line1, *rect[10000],*rectan;

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		-9, -3,0, // vertex 0
		0,-3,0, // vertex 1
		9,-3,0, // vertex 2
	};
	static const GLfloat vertex_buffer_data1 [] = {
		-8, -0.5,0, // vertex 0
		-7.4,0.07,0, // vertex 1
		-8,0.5,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		0,0,1, // color 0
		0,0,1, // color 1
		0,0,1, // color 2
	};
	static const GLfloat vertex_buffer [] = {
		0, 0,0, // vertex 0
		-0.3,0.2,0, // vertex 1
		-0.3,-0.2,0, // vertex 2
	};
	static const GLfloat playvertex [] = {
		0, 0,0, // vertex 0
		-0.2,0.1,0, // vertex 1
		-0.2,-0.1,0, // vertex 2
	};

	static const GLfloat color_buffer [] = {
		1,1,1, // color 0
		1,1,1, // color 1
		1,1,1, // color 2
	};
	static const GLfloat playcolor [] = {
		1,1,1, // color 0
		1,1,1, // color 1
		1,1,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	line = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	line1 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data1, color_buffer_data, GL_LINE);
	tri = create3DObject(GL_TRIANGLES, 3, vertex_buffer, color_buffer, GL_FILL);
	play = create3DObject(GL_TRIANGLES, 3, playvertex, playcolor, GL_FILL);



}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat gundata [] = {
		-0.2,-0.2,0, // vertex 1
		0.3,-0.2,0, // vertex 2
		0.3, 0.3,0, // vertex 3

		0.3, 0.3,0, // vertex 3
		-0.2, 0.3,0, // vertex 4
		-0.2,-0.2,0  // vertex 1
	};
	static const GLfloat rectanglegundata [] = {
		-0.2,-0.1,0, // vertex 1
		0.8,-0.1,0, // vertex 2
		0.8, 0.1,0, // vertex 3

		0.8, 0.1,0, // vertex 3
		-0.2, 0.1,0, // vertex 4
		-0.2,-0.1,0  // vertex 1
	};
	static const GLfloat mirror [] = {
		-0.2,-0.1,0, // vertex 1
		0.9,-0.1,0, // vertex 2
		0.9, 0.0,0, // vertex 3

		0.9, 0.0,0, // vertex 3
		-0.2, 0.0,0, // vertex 4
		-0.2,-0.1,0  // vertex 1
	};
	static const GLfloat vertex_buffer_data [] = {
		-0.2,-0.2,0, // vertex 1
		0.5,-0.2,0, // vertex 2
		0.5, 0.5,0, // vertex 3

		0.5, 0.5,0, // vertex 3
		-0.2, 0.5,0, // vertex 4
		-0.2,-0.2,0  // vertex 1
	};
	GLfloat vertexlaser [] = {
		0,0,0, // vertex 1
		0.5,0,0, // vertex 2
		0.5, 0.1,0, // vertex 3

		0.5, 0.1,0, // vertex 3
		0, 0.1,0, // vertex 4
		0,0,0  // vertex 1
	};
	GLfloat vertexrestart [] = {
		0,0,0, // vertex 1
		2,0,0, // vertex 2
		2, 0.7,0, // vertex 3

		2, 0.7,0, // vertex 3
		0, 0.7,0, // vertex 4
		0,0,0  // vertex 1
	};
	GLfloat display [] = {
		0,0,0, // vertex 1
		0.2,0,0, // vertex 2
		0.2, 0.05,0, // vertex 3

		0.2, 0.05,0, // vertex 3
		0, 0.05,0, // vertex 4
		0,0,0 
	};
	GLfloat pausesymbol [] = {
		0,0,0, // vertex 1
		0.2,0,0, // vertex 2
		0.2, 0.05,0, // vertex 3

		0.2, 0.05,0, // vertex 3
		0, 0.05,0, // vertex 4
		0,0,0 
	};

	static const GLfloat color_buffer_data [] = {
		1,0.2,0.2, // color 1
		1,0.2,0.2, // color 2
		1,0.8,0.8, // color 3

		1,0.8,0.8, // color 3
		1,0.8,0.8, // color 4
		1,0.2,0.2  // color 1
	};
	static const GLfloat color_buffer_data1 [] = {
		0.2,1,0.2, // color 1
		0.2,1,0.2, // color 2
		0.8,1,0.8, // color 3

		0.8,1,0.8, // color 3
		0.8,1,0.8, // color 4
		0.2,1,0.2  // color 1
	};
	static const GLfloat color_buffer [] = {
		1,0.7,0.7, // color 1
		1,0.7,0.7, // color 2
		1,0.7,0.7, // color 3

		1,0.7,0.7, // color 3
		1,0.7,0.7, // color 4
		1,0.7,0.7  // color 1
	};
	static const GLfloat color_buffer1 [] = {
		0.7,1,0.7, // color 1
		0.7,1,0.7, // color 2
		0.7,1,0.7, // color 3

		0.7,1,0.7, // color 3
		0.7,1,0.7, // color 4
		0.7,1,0.7  // color 1
	};
	static const GLfloat colorgundata [] = {
		0.645098,0.270588,0.145098,
		0.645098,0.270588,0.145098,// color 1
		0.645098,0.270588,0.145098,// color 1
		0.645098,0.270588,0.145098,// color 1
		0.645098,0.270588,0.145098,// color 1
		0.645098,0.270588,0.145098// color 1
	};
	static const GLfloat colormirror [] = {
		0.5,0.5,0.5, // color 1
		0.5,0.5,0.5, // color 2
		0.5,0.5,0.5, // color 3
		0.5,0.5,0.5, // color 1
		0.5,0.5,0.5, // color 2
		0.5,0.5,0.5
			// color 1
	};
	GLfloat colorlaser [] = {
		0.5,0.5,0.5, // color 1
		0.5,0.5,0.5, // color 2
		0.5,0.5,0.5, // color 3
		0.5,0.5,0, // color 1
		0.5,0.5,0.5, // color 2
		0.5,0.5,0.5
			// color 1
	};
	GLfloat colordisplay [] = {
		0,0,1,
		0,0,1,
		0,0,1,
		0,0,1,
		0,0,1,
		0,0,1
			// color 1
	};
	GLfloat colorpause [] = {
		1,1,1,
		1,1,1,
		1,1,1,
		1,1,1,
		1,1,1,
		1,1,1
			// color 1
	};
	

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	rectan = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer, GL_FILL);

	rectangle1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data1, GL_FILL);
	rectang = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer1, GL_FILL);
	restart = create3DObject(GL_TRIANGLES, 6, vertexrestart, color_buffer_data1, GL_FILL);
	pause1 =  create3DObject(GL_TRIANGLES, 6,pausesymbol, colorpause, GL_FILL);
	pause2 =  create3DObject(GL_TRIANGLES, 6, pausesymbol, colorpause, GL_FILL);

	rectangle2 = create3DObject(GL_TRIANGLES, 6, gundata, colorgundata, GL_FILL);
	rectangle3 = create3DObject(GL_TRIANGLES, 6, rectanglegundata, colorgundata, GL_FILL);
	rectangle4 = create3DObject(GL_TRIANGLES, 6, mirror, colormirror, GL_FILL);
	rectangle5 = create3DObject(GL_TRIANGLES, 6, mirror, colormirror, GL_FILL);
	rectangle6 = create3DObject(GL_TRIANGLES, 6, mirror, colormirror, GL_FILL);
	rectangle7 = create3DObject(GL_TRIANGLES, 6, mirror, colormirror, GL_FILL);
	for(int i=0;i<10000;i++)
		laser[i] = create3DObject(GL_TRIANGLES, 6, vertexlaser, colorlaser, GL_FILL);
	for(int i=0;i<7;i++)
		segment[i]=create3DObject(GL_TRIANGLES, 6, display, colordisplay, GL_FILL);
	for(int i=0;i<7;i++)
		scoredis[i]=create3DObject(GL_TRIANGLES, 6, display, colordisplay, GL_FILL);
	for(int i=0;i<7;i++)
		level[i]=create3DObject(GL_TRIANGLES, 6, display, colordisplay, GL_FILL);





}
void createCircle()
{
	GLfloat vertex_buffer_data [360*9]={0};
	GLfloat vertex_buffer_data1 [360*9]={0};
	GLfloat vertex_buffer_data2 [360*9]={0};
	GLfloat vertex_buffer_data3 [360*9]={0};


	for(int i=0;i<360;i++)
	{
		vertex_buffer_data[9*i]=0;
		vertex_buffer_data[9*i+1]=0;
		vertex_buffer_data[9*i+2]=0;
		vertex_buffer_data[9*i+3]=0.35*cos(i*M_PI/180);
		vertex_buffer_data[9*i+4]=0.35*sin(i*M_PI/180);
		vertex_buffer_data[9*i+5]=0;
		vertex_buffer_data[9*i+6]=0.35*cos((i+1)*M_PI/180);
		vertex_buffer_data[9*i+7]=0.35*sin((i+1)*M_PI/180);
		vertex_buffer_data[9*i+8]=0;
	}
	for(int i=0;i<360;i++)
	{
		vertex_buffer_data1[9*i]=0;
		vertex_buffer_data1[9*i+1]=0;
		vertex_buffer_data1[9*i+2]=0;
		vertex_buffer_data1[9*i+3]=0.6*cos(i*M_PI/180);
		vertex_buffer_data1[9*i+4]=0.6*sin(i*M_PI/180);
		vertex_buffer_data1[9*i+5]=0;
		vertex_buffer_data1[9*i+6]=0.6*cos((i+1)*M_PI/180);
		vertex_buffer_data1[9*i+7]=0.6*sin((i+1)*M_PI/180);
		vertex_buffer_data1[9*i+8]=0;
	}
	for(int i=0;i<360;i++)
	{
		vertex_buffer_data2[9*i]=0;
		vertex_buffer_data2[9*i+1]=0;
		vertex_buffer_data2[9*i+2]=0;
		vertex_buffer_data2[9*i+3]=0.35*cos(i*M_PI/180);
		vertex_buffer_data2[9*i+4]=0.35*sin(i*M_PI/180);
		vertex_buffer_data2[9*i+5]=0;
		vertex_buffer_data2[9*i+6]=0.35*cos((i+1)*M_PI/180);
		vertex_buffer_data2[9*i+7]=0.35*sin((i+1)*M_PI/180);
		vertex_buffer_data2[9*i+8]=0;
	}
	for(int i=0;i<360;i++)
	{
		vertex_buffer_data3[9*i]=0;
		vertex_buffer_data3[9*i+1]=0;
		vertex_buffer_data3[9*i+2]=0;
		vertex_buffer_data3[9*i+3]=0.25*cos(i*M_PI/180);
		vertex_buffer_data3[9*i+4]=0.25*sin(i*M_PI/180);
		vertex_buffer_data3[9*i+5]=0;
		vertex_buffer_data3[9*i+6]=0.25*cos((i+1)*M_PI/180);
		vertex_buffer_data3[9*i+7]=0.25*sin((i+1)*M_PI/180);
		vertex_buffer_data3[9*i+8]=0;
	}
	GLfloat color_buffer_data [360*9];
	GLfloat color_buffer_data1 [360*9];
	GLfloat color_buffer_data2 [360*9];
	GLfloat color_buffer2 [360*9];

	GLfloat color_buffer_data3 [360*9];

	static GLfloat color_buffer_data4 [360*9];



	for (int i = 0; i<360*9 ; i+=3)
	{
		color_buffer_data[i]=1;
		color_buffer_data[i+1]=0.3;
		color_buffer_data[i+2]=0.3;
	}
	for (int i = 0; i<360*9 ; i+=3)
	{
		color_buffer_data1[i]=0.4;
		color_buffer_data1[i+1]=1;
		color_buffer_data1[i+2]=0.4;
	}
	for (int i = 0; i<360*9 ; i+=3)
	{
		color_buffer_data2[i]=0.645098;
		color_buffer_data2[i+1]=0.270588;
		color_buffer_data2[i+2]= 0.145098;
	}
	for (int i = 0; i<360*9 ; i+=3)
	{
		color_buffer2[i]=0.645098;
		color_buffer2[i+1]=0.470588;
		color_buffer2[i+2]= 0.345098;
	}
	for (int i = 0; i<360*9 ; i+=3)
	{
		color_buffer_data3[i]=0;
		color_buffer_data3[i+1]=0;
		color_buffer_data3[i+2]=1;
	}
	circle = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data,GL_FILL);
	circle1 = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data,GL_FILL);
	circle2 = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data1,GL_FILL);
	circle3 = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data1,GL_FILL);
	circle4 = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data1,color_buffer_data2,GL_LINE);
	circle41 = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data1,color_buffer2,GL_LINE);

	circle5 = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data2,color_buffer_data3,GL_LINE);
	pause = create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data3,color_buffer_data4,GL_FILL);



}


void createobjects()
{
	int i;
	for(i=0;i<10000;i++){
		GLfloat vertex_buffer_data [18]={0};

		int random_integer = -4 + rand() % 12;
		posx[i]=random_integer;
		if(random_integer>=-4 && random_integer<=0)
			random2[i] = rand()%2;
		else{
			random2[i] = rand()%3;
			if(random2[i]==1)
				random2[i]+=1;
		}
		vertex_buffer_data [0] = random_integer;
		vertex_buffer_data [1] = 3.4;
		vertex_buffer_data [2] = 0;
		vertex_buffer_data [3] = random_integer+0.2;
		vertex_buffer_data [4] = 3.4;
		vertex_buffer_data [5] = 0;
		vertex_buffer_data [6] = random_integer+0.2;
		vertex_buffer_data [7] = 3.7;
		vertex_buffer_data [8] = 0;
		vertex_buffer_data [9] = random_integer+0.2;
		vertex_buffer_data [10] = 3.7;
		vertex_buffer_data [11] = 0;
		vertex_buffer_data [12] = random_integer;
		vertex_buffer_data [13] = 3.7;
		vertex_buffer_data [14] = 0;
		vertex_buffer_data [15] = random_integer;
		vertex_buffer_data [16] = 3.4;
		vertex_buffer_data [17] = 0;

		//R
		GLfloat color_buffer_data1 [] = {
			1,0,0, // color 1
			1,0,0, // color 2
			1,0,0, // color 3

			1,0,0, // color 3
			1,0,0, // color 4
			1,0,0  // color 1
		};
		//G
		GLfloat color_buffer_data2 [] = {
			0,1,0, // color 1
			0,1,0, // color 2
			0,1,0, // color 3

			0,1,0, // color 3
			0,1,0, // color 4
			0,1,0  // color 1
		};
		//B
		GLfloat color_buffer_data3 [] = {
			0,0,0, // color 1
			0,0,0, // color 2
			0,0,0, // color 3

			0,0,0, // color 3
			0,0,0, // color 4
			0,0,0  // color 1
		};
		if(random2[i]==0)
			rect[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data3, GL_FILL);
		if(random2[i]==1)
			rect[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data1, GL_FILL);
		if(random2[i]==2)
			rect[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data2, GL_FILL);
	}
}
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	int sx,sy,mul;
	float vl;
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;

			// MVP = Projection * View * Model
	dig=-1;
	mul=score/100;
	vl=1;
	for(int i=0;i<mul;i++){
		vl*=1.2;
		speed=vl;

	}
	if(score<0)
		score=0;
	score1=score;
	score2=score1;

	while(score2!=0){
		score2/=10;
		dig++;
	}


	if(position1<-3)
		position1=-3;
	if(position1>2.9)
		position1=2.9;
	if(position2>4.2)
		position2=4.2;
	if(position2<-2)
		position2=-2;
	if(position3>2.9)
		position3=2.9;
	if(position3<-2.5)
		position3=-2.5;
	if(position4>70)
		position4=70;
	if(position4<-70)
		position4=-70;
	if(!ex)
	{
		sx=0;
		sy=0;
	}
	if(ex==1)
	{
		sx=5.5;
		sy=3.5;

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateRectangle111 = glm::translate (glm::vec3(0.6, -0.35, 0));        // glTranslatef
		glm::mat4 rotateRectangle111 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle111 * rotateRectangle111);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(tri);


		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateRectangle112 = glm::translate (glm::vec3(-0.6, -0.7, 0));        // glTranslatef
		glm::mat4 rotateRectangle112 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle112 * rotateRectangle112);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(restart);


	}
	if(ex==0){
if(flagp==1){
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateRectangle114 = glm::translate (glm::vec3(3.12, 3.7, 0));        // glTranslatef
		glm::mat4 rotateRectangle114 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle114 * rotateRectangle114);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(play);
}
if(flagp==0){
Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateRectangle113 = glm::translate (glm::vec3(2.95, 3.6, 0));        // glTranslatef
		glm::mat4 rotateRectangle113 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle113 * rotateRectangle113);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(pause1);

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateRectangle114 = glm::translate (glm::vec3(3.1, 3.6, 0));        // glTranslatef
		glm::mat4 rotateRectangle114 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle114 * rotateRectangle114);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(pause2);
}
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateRectangle112 = glm::translate (glm::vec3(3, 3.7, 0));        // glTranslatef
		glm::mat4 rotateRectangle112 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle112 * rotateRectangle112);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(pause);

		if(leftmove!=1){
			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle = glm::translate (glm::vec3(-3.15+position1, -3.3, 0));        // glTranslatef
			glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle * rotateRectangle);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rectangle);
		}
		if(leftmove==1){
			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle = glm::translate (glm::vec3(-3.15+position1, -3.3, 0));        // glTranslatef
			glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle * rotateRectangle);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rectan);
		}


		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateCircle = glm::translate (glm::vec3(-3+position1, -2.8, 0));        // glTranslatef
		glm::mat4 rotateCircle = glm::rotate((float)(cirlce_rotation*M_PI/180.0f), glm::vec3(-1,0,0)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateCircle * rotateCircle);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(circle);


		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateCircle1 = glm::translate (glm::vec3(-3+position1, -3.5, 0));        // glTranslatef
		glm::mat4 rotateCircle1 = glm::rotate((float)(cirlce_rotation1*M_PI/180.0f), glm::vec3(-1,0,0)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateCircle1 * rotateCircle1);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(circle1);
		if(rightmove!=1){
			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle1 = glm::translate (glm::vec3(2.85+position2, -3.3, 0));        // glTranslatef
			glm::mat4 rotateRectangle1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle1 * rotateRectangle1);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rectangle1);
		}
		if(rightmove==1){
			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle1 = glm::translate (glm::vec3(2.85+position2, -3.3, 0));        // glTranslatef
			glm::mat4 rotateRectangle1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle1 * rotateRectangle1);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rectang);
		}

		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateCircle2 = glm::translate (glm::vec3(3+position2, -2.8, 0));        // glTranslatef
		glm::mat4 rotateCircle2 = glm::rotate((float)(cirlce_rotation1*M_PI/180.0f), glm::vec3(-1,0,0)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateCircle2 * rotateCircle2);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(circle2);


		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateCircle3 = glm::translate (glm::vec3(3+position2, -3.5, 0));        // glTranslatef
		glm::mat4 rotateCircle3 = glm::rotate((float)(cirlce_rotation1*M_PI/180.0f), glm::vec3(-1,0,0)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateCircle3 * rotateCircle3);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(circle3);

		/*	Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle3 = glm::translate (glm::vec3(-7.8, 0.6+position3, 0));        // glTranslatef
			glm::mat4 rotateRectangle3 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle3 * rotateRectangle3);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rectangle2);
		 */
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateRectangle4 = glm::translate (glm::vec3(-7.6, 0.65+position3, 0));        // glTranslatef
		glm::mat4 rotateRectangle4 = glm::rotate((float)(position4*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle4 * rotateRectangle4);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(rectangle3);

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateRectangle5 = glm::translate (glm::vec3(0, 0.6, 0));        // glTranslatef
		glm::mat4 rotateRectangle5 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle5 * rotateRectangle5);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(line);

		/*Matrices.model = glm::mat4(1.0f);

		  glm::mat4 translateRectangle6 = glm::translate (glm::vec3(0, 0.6+position3, 0));        // glTranslatef
		  glm::mat4 rotateRectangle6 =glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		  Matrices.model *= (translateRectangle6 * rotateRectangle6);
		  MVP = VP * Matrices.model;
		  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		  draw3DObject(line1);*/

		if(flag==1){
			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle7 = glm::translate (glm::vec3(0, 0+pos[j], 0));        // glTranslatef
			glm::mat4 rotateRectangle7 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle7 * rotateRectangle7);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rect[j]);
		}
		for(int i=0;i<=j;i++){
			if(speed<1)
				speed=1;
			if(speed>3)
				speed=3;
			float c_time2=glfwGetTime();

			if(c_time2-u_time2[i]>0.005 && !flagp){
			pos[i]-=(0.02*speed);
			u_time2[i]=glfwGetTime();
		}
			//if(pos[i]==-5.5 && random2[i]==0)
			//printf("%f\n",pos[i]);
			//if(4+pos[i] == -2) {
			if( (abs(-3.15+position1-posx[i])<0.35) && pos[i]< -6.3 && random2[i]==1 && pos[i]>-7){
				if(vis[i]==0)
					score+=5;

				vis[i]=1;
			}
			//		}
			if(random2[i]==2 && abs(2.85+position2-posx[i])<0.35 && pos[i]<-6.3 && pos[i]> -7){
				if(vis[i]==0)
					score+=5;

				vis[i]=1;
			}
			if( (abs(-3.15+position1-posx[i])<0.35) && pos[i]< -6.3 && random2[i]==0 && pos[i]>-7){
				ex=1;
			}
			//		}
			if(random2[i]==0 && abs(2.85+position2-posx[i])<0.35 && pos[i]<-6.3 && pos[i]> -7){
				ex=1;
			}


			if(!vis[i]){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translateRectangle7 = glm::translate (glm::vec3(0, 0+pos[i], 0));        // glTranslatef
				glm::mat4 rotateRectangle7 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translateRectangle7 * rotateRectangle7);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				/*if(pos[i]<0 && pos[i]>-0.2)
				  q++;*/
				if (pos[i] <0){

					quex.push_back(posx[i]);
					quey.push_back(pos[i]+4);
					quei.push_back(i);

				}

				if(pos[i]<=-8){
					if(random2[i]==1){
						exred++;
						score-=3;
					}
					if(random2[i]==2){
						exgreen++;
						score-=3;
					}
					if(exred==5 || exgreen==5)
						ex=1;

					vis[i]=1;
					k++;
				}

				for(int z=0;z<=j;z++){
					for(int y=0;y<=press;y++){
						if((abs(3.5+pos[z]-lasery[y])<0.2) && abs(posx[z]-laserx[y])<0.2 ){
							position5[y]+=16;
							if(random2[z]==0 && vis[z]==0)
								score+=10;
							else if(vis[z]==0)
								score-=3;
							vis[z]=1;

						}}

				}
				if(pos[i]<=0)
					draw3DObject(rect[i]);

			}
		}


		if(flag3==1){
			for(int i=0;i<=press;i++){
				float c_time1=glfwGetTime();
				
				if(c_time1-u_time1[i] > 0.005 && !flagp){				
				position5[i]+=0.2;
				u_time1[i]=glfwGetTime();
			}

				laserx[i]=xcollide[i]+position5[i]*cos(position6[i]*M_PI/180.0f);
				lasery[i]=(ycollide[i]+position7[i])+position5[i]*sin(position6[i]*M_PI/180.0f);

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translateRectangle13 = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef

				glm::mat4 translateRectangle12 = glm::translate (glm::vec3(laserx[i], lasery[i], 0));        // glTranslatef
				glm::mat4 rotateRectangle12 = glm::rotate((float)(position6[i]*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translateRectangle12 * rotateRectangle12*translateRectangle13);
				MVP = VP * Matrices.model;	
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(laser[i]);
				/*for(auto it=quei.begin();it!=quei.end();it++){ 
				  vis[*it]=1;*/
				laserx1[i]=xcollide[i]+(position5[i]+0.4)*cos(position6[i]*M_PI/180.0f);
				lasery1[i]=(ycollide[i]+position7[i])+(position5[i]+0.4)*sin(position6[i]*M_PI/180.0f);
				float p1x=-3+0.2*cos(45*M_PI/180);
				float p1y=2-0.2*sin(45*M_PI/180);
				float p2x=-3-0.9*cos(45*M_PI/180);
				float p2y=2+0.9*sin(45*M_PI/180);
				float q1x=-1.1-0.2*cos(45*M_PI/180);
				float q1y=-1.4-0.2*sin(45*M_PI/180);
				float q2x=-1.1+0.9*cos(45*M_PI/180);
				float q2y=-1.4+0.9*sin(45*M_PI/180);
				float r1x=5-0.2*cos(45*M_PI/180);
				float r1y=-1.2-0.2*sin(45*M_PI/180);
				float r2x=5+0.9*cos(45*M_PI/180);
				float r2y=-1.2+0.9*sin(45*M_PI/180);
				float s1x=5.5+0.2*cos(45*M_PI/180);
				float s1y=2.5-0.2*sin(45*M_PI/180);
				float s2x=5.5-0.9*cos(45*M_PI/180);
				float s2y=2.5+0.9*sin(45*M_PI/180);
				//check for mirror 1 collision
				if((laserx[i]+lasery[i]+1.3)*(laserx1[i]+lasery1[i]+1.3)<0 && (p1y-lasery[i]-(tan(position6[i]*M_PI/180)*(p1x-laserx[i])))*(p2y-lasery[i]-(tan(position6[i]*M_PI/180)*(p2x-laserx[i])))<0){
					position5[i]=0;
					xcollide[i]=(laserx[i]+laserx1[i])/2;
					ycollide[i]=(lasery[i]+lasery1[i])/2;
					position7[i]=0;
					position6[i]=2*135-position6[i];
				}
				//check for mirror 2 collision

				if((laserx[i]-lasery[i]-0.1)*(laserx1[i]-lasery1[i]-0.1)<0 && (q1y-lasery[i]-(tan(position6[i]*M_PI/180)*(q1x-laserx[i])))*(q2y-lasery[i]-(tan(position6[i]*M_PI/180)*(q2x-laserx[i])))<0){
					position5[i]=0;
					xcollide[i]=(laserx[i]+laserx1[i])/2;
					ycollide[i]=(lasery[i]+lasery1[i])/2;
					position7[i]=0;
					position6[i]=2*45-position6[i];
				}
				//check for mirror 3 collision
				if((laserx[i]-lasery[i]-6)*(laserx1[i]-lasery1[i]-6)<0 && (r1y-lasery[i]-(tan(position6[i]*M_PI/180)*(r1x-laserx[i])))*(r2y-lasery[i]-(tan(position6[i]*M_PI/180)*(r2x-laserx[i])))<0){
					position5[i]=0;
					xcollide[i]=(laserx[i]+laserx1[i])/2;
					ycollide[i]=(lasery[i]+lasery1[i])/2;
					position7[i]=0;
					position6[i]=2*45-position6[i];
				}
				// check for mirror 4 collision
				if((laserx[i]+lasery[i]-7.8)*(laserx1[i]+lasery1[i]-7.8)<0 && (s1y-lasery[i]-(tan(position6[i]*M_PI/180)*(s1x-laserx[i])))*(s2y-lasery[i]-(tan(position6[i]*M_PI/180)*(s2x-laserx[i])))<0){
					position5[i]=0;
					xcollide[i]=(laserx[i]+laserx1[i])/2;
					ycollide[i]=(lasery[i]+lasery1[i])/2;
					position7[i]=0;
					position6[i]=2*135-position6[i];
				}
			}
			}



			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle8 = glm::translate (glm::vec3(5.5, 2.5, 0));        // glTranslatef
			glm::mat4 rotateRectangle8 = glm::rotate((float)(135*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle8 * rotateRectangle8);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rectangle4);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle9 = glm::translate (glm::vec3(5, -1.2, 0));        // glTranslatef
			glm::mat4 rotateRectangle9 = glm::rotate((float)(45*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle9 * rotateRectangle9);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rectangle5);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle10 = glm::translate (glm::vec3(-3, 2, 0));        // glTranslatef
			glm::mat4 rotateRectangle10 = glm::rotate((float)(135*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle10 * rotateRectangle10);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rectangle6);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateRectangle11 = glm::translate (glm::vec3(-1.1, -1.4, 0));        // glTranslatef
			glm::mat4 rotateRectangle11 = glm::rotate((float)(45*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateRectangle11 * rotateRectangle11);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(rectangle7);
			if(moverifle!=1){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatecircle4 = glm::translate (glm::vec3(-8, 0.65+position3, 0));        // glTranslatef
				glm::mat4 rotatecircle4 = glm::rotate((float)(position4*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatecircle4 * rotatecircle4);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(circle4);
			}
			if(moverifle==1){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatecircle4 = glm::translate (glm::vec3(-8, 0.65+position3, 0));        // glTranslatef
				glm::mat4 rotatecircle4 = glm::rotate((float)(position4*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatecircle4 * rotatecircle4);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(circle41);
				//7 segment display
			}
		}
		if(dig==-1)
			dig=0;

		for(int a=dig;a>=0;a--){
			int p=score1%10;
			if(p==0||p==4||p==5||p==6||p==8||p==9){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment1 = glm::translate (glm::vec3(6.5+a*0.5-sx, 3.4+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment1 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment1 * rotatesegment1);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(segment[0]);
			}
			if(p==0||p==2||p==6||p==8){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment2 = glm::translate (glm::vec3(6.5+a*0.5-sx, 3.21+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment2 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment2 * rotatesegment2);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(segment[1]);
			}
			if(p==0||p==2||p==3||p==5||p==6||p==8){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment3 = glm::translate (glm::vec3(6.49+a*0.5-sx, 3.2+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment3 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment3 * rotatesegment3);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(segment[2]);
			}
			if(p==0||p==1||p==3||p==4||p==5||p==6||p==7||p==8||p==9){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment4 = glm::translate (glm::vec3(6.73+a*0.5-sx, 3.21+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment4 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment4 * rotatesegment4);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(segment[3]);
			}
			if(p==0||p==1||p==2||p==3||p==4||p==7||p==8||p==9){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment5 = glm::translate (glm::vec3(6.73+a*0.5-sx, 3.4+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment5 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment5 * rotatesegment5);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(segment[4]);
			}
			if(p==0||p==2||p==3||p==5||p==6||p==7||p==8||p==9){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment6 = glm::translate (glm::vec3(6.5+a*0.5-sx, 3.55+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment6 = glm::rotate((float)(	0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment6 * rotatesegment6);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(segment[5]);
			}
			if(p==8 ||p==2||p==5||p==3||p==4||p==6||p==9){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment7 = glm::translate (glm::vec3(6.49+a*0.5-sx, 3.37+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment7 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment7 * rotatesegment7);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(segment[6]);
			}
			score1=score1/10;
		}

		for(int c=0;c<5;c++){

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translatesegment8 = glm::translate (glm::vec3(3.5+c*0.4+0.7-sx, 3.4+0.3-sy, 0));        // glTranslatef
			glm::mat4 rotatesegment8 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translatesegment8 * rotatesegment8);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(scoredis[0]);
			if(c!=0){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment9 = glm::translate (glm::vec3(3.5+c*0.4+0.7-sx, 3.21+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment9 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment9 * rotatesegment9);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(scoredis[1]);
			}
			if(c!=3){

				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment10 = glm::translate (glm::vec3(3.49+c*0.4+0.7-sx, 3.2+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment10 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment10 * rotatesegment10);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(scoredis[2]);
			}
			if(c==0 ||c==2||c==3){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment11 = glm::translate (glm::vec3(3.7+c*0.4+0.7-sx, 3.21+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment11 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment11 * rotatesegment11);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(scoredis[3]);
			}
			if(c==2||c==3){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment12 = glm::translate (glm::vec3(3.7+c*0.4+0.7-sx, 3.4+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment12 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment12 * rotatesegment12);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(scoredis[4]);
			}

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translatesegment13 = glm::translate (glm::vec3(3.5+c*0.4+0.7-sx, 3.55+0.3-sy, 0));        // glTranslatef
			glm::mat4 rotatesegment13 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translatesegment13 * rotatesegment13);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(scoredis[5]);

			if(c==0 || c==3||c==4){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translatesegment14 = glm::translate (glm::vec3(3.5+c*0.4+0.7-sx, 3.37+0.3-sy, 0));        // glTranslatef
				glm::mat4 rotatesegment14 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatesegment14 * rotatesegment14);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(scoredis[6]);
			}	
		}
		if(ex==0){
		int le=mul+1;
			
			for(int c=0;c<6;c++){
				if(c==5)
					c=6;

				if(c==0||c==1||c==3||c==4 ||c==2 ){
					int angle=90;
					float shift=0;
					if(c==2)
						angle=120;
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment15 = glm::translate (glm::vec3(-7.5+c*0.4, 3.4+0.3, 0));
					// glTranslatef
					glm::mat4 rotatesegment15 = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment15 * rotatesegment15);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[0]);
				}

				if(c==0||c==1||c==3||c==4||c==2){
					int angle=90;
					float shift=0;
					if(c==2){
						angle=120;
						shift=0.1;
					}
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment16 = glm::translate (glm::vec3(-7.5+c*0.4+shift, 3.21+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment16 = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment16 * rotatesegment16);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[1]);
				}
				if(c==0||c==1||c==3||c==4){
					Matrices.model = glm::mat4(1.0f);


					glm::mat4 translatesegment17 = glm::translate (glm::vec3(-7.51+c*0.4, 3.2+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment17 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment17 * rotatesegment17);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[2]);
				}
				if(c==2){
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment18 = glm::translate (glm::vec3(-7.34+c*0.4, 3.21+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment18 = glm::rotate((float)(70*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment18 * rotatesegment18);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[3]);
				}
				if(c==2){
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment19 = glm::translate (glm::vec3(-7.28+c*0.4, 3.4+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment19 = glm::rotate((float)(70*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment19 * rotatesegment19);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[4]);
				}
				if(c==1||c==3){
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment20 = glm::translate (glm::vec3(-7.51+c*0.4, 3.55+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment20 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment20 * rotatesegment20);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[5]);
				}
				if(c==1||c==3){
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment21 = glm::translate (glm::vec3(-7.51+c*0.4, 3.37+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment21 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment21 * rotatesegment21);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[6]);
				}



				//level no
				int p=le;
				if(c==6 &&(p==0||p==4||p==5||p==6||p==8||p==9)){
					int angle=90;

					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment15 = glm::translate (glm::vec3(-7.5+c*0.4, 3.4+0.3, 0));
					// glTranslatef
					glm::mat4 rotatesegment15 = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment15 * rotatesegment15);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[0]);
				}

				if(c==6 &&(p==0||p==2||p==6||p==8)){

					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment16 = glm::translate (glm::vec3(-7.5+c*0.4, 3.21+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment16 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment16 * rotatesegment16);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[1]);
				}
				if(c==6 && (p==2||p==3||p==5||p==6||p==8)){
					Matrices.model = glm::mat4(1.0f);


					glm::mat4 translatesegment17 = glm::translate (glm::vec3(-7.51+c*0.4, 3.2+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment17 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment17 * rotatesegment17);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[2]);
				}
				if(c==6 && (p==1||p==3||p==4||p==5||p==6||p==7||p==8||p==9)){
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment18 = glm::translate (glm::vec3(-7.28+c*0.4, 3.21+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment18 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment18 * rotatesegment18);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[3]);
				}
				if(c==6 && (p==1||p==2||p==3||p==4||p==7||p==8||p==9)){
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment19 = glm::translate (glm::vec3(-7.28+c*0.4, 3.4+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment19 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment19 * rotatesegment19);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[4]);
				}
				if(c==6 && (p==2||p==3||p==5||p==6||p==7||p==8||p==9)){
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment20 = glm::translate (glm::vec3(-7.51+c*0.4, 3.55+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment20 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment20 * rotatesegment20);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[5]);
				}
				if(c==6 && (p==8 ||p==2||p==5||p==3||p==4||p==6||p==9)){
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translatesegment21 = glm::translate (glm::vec3(-7.51+c*0.4, 3.37+0.3, 0));        // glTranslatef
					glm::mat4 rotatesegment21 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model *= (translatesegment21 * rotatesegment21);
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
					draw3DObject(level[6]);
				}

			}

		}		
		float increments = 1;

		//printf("%d\n",score);
		//camera_rotation_angle++; // Simulating camera rotation
		//  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
		//rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
	}

	/* Initialise glfw window, I/O callbacks and the renderer to use */
	/* Nothing to Edit here */
	GLFWwindow* initGLFW (int width, int height)
	{
		GLFWwindow* window; // window desciptor/handle

		glfwSetErrorCallback(error_callback);
		if (!glfwInit()) {
			//        exit(EXIT_FAILURE);
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

		if (!window) {
			glfwTerminate();
			//        exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(window);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		glfwSwapInterval( 1 );

		/* --- register callbacks with GLFW --- */

		/* Register function to handle window resizes */
		/* With Retina display on Mac OS X GLFW's FramebufferSize
		   is different from WindowSize */
		glfwSetFramebufferSizeCallback(window, reshapeWindow);
		glfwSetWindowSizeCallback(window, reshapeWindow);

		/* Register function to handle window close */
		glfwSetWindowCloseCallback(window, quit);

		/* Register function to handle keyboard input */
		glfwSetKeyCallback(window, keyboard);      // general keyboard input
		glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

		/* Register function to handle mouse click */
		glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
		glfwSetScrollCallback(window, mousezoom);

		return window;
	}

	/* Initialize the OpenGL rendering properties */
	/* Add all the models to be created here */
	void initGL (GLFWwindow* window, int width, int height)
	{
		/* Objects should be created before any other gl function and shaders */
		// Create the models
		createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
		createRectangle ();
		createCircle();
		createobjects();
		// Create and compile our GLSL program from the shaders
		programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
		// Get a handle for our "MVP" uniform
		Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


		reshapeWindow (window, width, height);

		// Background color of the scene
		glClearColor (  1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
		glClearDepth (1.0f);

		glEnable (GL_DEPTH_TEST);

		cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
		cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
		cout << "VERSION: " << glGetString(GL_VERSION) << endl;
		cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	}

	int main (int argc, char** argv)
	{
		int width = 1500;
		int height = 800;

		GLFWwindow* window = initGLFW(width, height);

		initGL (window, width, height);


		double last_update_time = glfwGetTime(), current_time,current,last_update=glfwGetTime();

		/* Draw in loop */
		while (!glfwWindowShouldClose(window)) {

			// OpenGL Draw commands

			draw();
			// Swap Frame Buffer in double buffering
			glfwSwapBuffers(window);

			// Poll for Keyboard and mouse events
			glfwPollEvents();
			//flag2=1;
			// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
			current_time = glfwGetTime(); // Time in seconds
			current = glfwGetTime(); // Time in seconds
			if ((current - last_update) > 0.5) { // atleast 0.5s elapsed since last frame
				last_update = current;
				flag2=1;
			}
			else
				flag2=0;

			if((current_time - last_update_time) >0.005)
			{
				if(lb==1)
					position1+=0.05;
				if(lb==-1)
					position1-=0.05;
				if(rb==1)
					position2+=0.05;
				if(rb==-1)
					position2-=0.05;
				if(gg==1)
					position3+=0.05;
				if(gg==-1)
					position3-=0.05;
				if(lmouse==1 || rmouse==1)
					drag(window);
			}
			if ((current_time - last_update_time) >= 1.6/speed && !flagp) { // atleast 0.5s elapsed since last frame
				// do something every 0.5 seconds ..
				last_update_time = current_time;
				flag=1;
				j++;
			}

			flag=0;
		}
		glfwTerminate();
		quex.clear();
		quey.clear();
		quei.clear();
		//    exit(EXIT_SUCCESS);
	}
