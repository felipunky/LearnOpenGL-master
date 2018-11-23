//#pragma comment( linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup" )

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Shader.h"
#include "wtypes.h"
#include <time.h>
#include <iostream>
#include <string>
#include <vector>

// FPS, iFrame counter.
int initialTime = time( NULL ), finalTime, frameCount, frames, FPS;
const char* title;
// Mouse.
static double xPre, yPre;
double xPos, yPos, xDif, yDif, vX, vY;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Our image size.
unsigned int SRC_WIDTH = 1920;
unsigned int SRC_HEIGHT = 1080;

int WIDTH, HEIGHT;

bool full = false;
std::string fullPrompt;

// We need this to be able to resize our window.
void framebuffer_size_callback( GLFWwindow* window, int width, int height );
// We need this to be able to close our window when pressing a key.
void processInput( GLFWwindow* window );
// We need this to be able to call our load image function from below main.
unsigned int loadTexture( const char *path );

// Our mouse button click flag.
float pressed = 0.0, right_pressed = 0.0;

// Quality of texture fetches.
int RGBA32 = 34836;
int RGBA16 = 34842;

int RGBA = 0;

std::string quality;

// Our mouse.
static void cursorPositionCallback( GLFWwindow *window, double xPos, double yPos );
// Our mouse button press.
static void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods );


int main() 
{

	// Get input from user.
	std::cout << "Do you want the simulation to run on fullscreen? yes or no, type: y for yes or n for no: ";
	std::cin >> fullPrompt;

	if( fullPrompt == "y" )
	{

		full = true;

	}
	
	else
	{
	
		full = false;
	
	}

	if( full == false )
	{

		std::cout << "Specify the width size of the window: ";
		std::cin >> SRC_WIDTH;

		std::cout << "Specify the height size of the window: ";
		std::cin >> SRC_HEIGHT;

	}

	std::cout << "High or low quality? high or low: type h for high or l for low: ";
	std::cin >> quality;

	if( quality == "h"  )
	{
	
		RGBA = RGBA32;
	
	}

	else
	{
	
		RGBA = RGBA16;
	
	}

	std::cout << "Specify the path to the texture you want to use to initialize the simulation: " << std::endl;

	std::string texturePathString;

	std::cin >> texturePathString;

	const char* texturePath = &texturePathString[0];

	// We initialize glfw and specify which versions of OpenGL to target.
	const char* glsl_version = "#version 150";
	glfwInit();
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	
	// Our window object.
	GLFWmonitor* monitor = NULL;

	if( full == true )
		monitor = glfwGetPrimaryMonitor();

	GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "NavierStokeish", monitor, NULL);

	if ( window == NULL )
	{

		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;

	}

	glfwMakeContextCurrent( window );
	glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

	// Initialize the mouse.
	glfwSetCursorPosCallback( window, cursorPositionCallback );
	// Initialize the mouse button.
	glfwSetMouseButtonCallback( window, mouseButtonCallback );

	// Load glad, we need this library to specify system specific functions.
	if( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) )
	{

		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;

	}

	// Setup window
	char path[1024];

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Setup Style
	ImGui::StyleColorsDark();

	// We build and compile our shader program.
	Shader Image( "vertex.glsl", "Image.glsl" );
	Shader BufferA( "vertex.glsl", "BufferA.glsl" );
	Shader BufferB( "vertex.glsl", "BufferB.glsl" );
	Shader BufferC( "vertex.glsl", "BufferC.glsl" );
	Shader BufferD( "BufferDVertex.glsl", "BufferD.glsl" );
	
	// This is for our screen quad.
	// We define the points in space that we want to render.
	float vertices[] =
	{
		
		// Positions.            // TextureCoordinates.
		-1.0f,  -1.0f, 0.0f,     //-1.0f,  1.0f,
		-1.0f,   1.0f, 0.0f,     //1.0f,  1.0f,
		 1.0f,   1.0f, 0.0f,     //1.0f, -1.0f,
		 1.0f,  -1.0f, 0.0f,    //-1.0f, -1.0f 

	};

	// We define our Element Buffer Object indices so that if we have vertices that overlap,
	// we dont have to render twice, 50% overhead.
	unsigned int indices[] = 
	{
	
		0, 1, 3,
		1, 2, 3

	};

	// We create a buffer ID so that we can later assign it to our buffers.
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays( 1, &VAO );
	glGenBuffers( 1, &VBO );
	glGenBuffers( 1, &EBO );

	// Bind Vertex Array Object.
	glBindVertexArray( VAO );

	// Copy our vertices array in a buffer for OpenGL to use.
	// We assign our buffer ID to our new buffer and we overload it with our triangles array.
	glBindBuffer( GL_ARRAY_BUFFER, VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

	// Copy our indices in an array for OpenGL to use.
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

	// Set our vertex attributes pointers.
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), ( void* ) 0 );
	glEnableVertexAttribArray( 0 );

	// Unbind the VBO.
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	// Unbind the VAO.
	glBindVertexArray( 0 );

	const int siz = 600;

	std::vector<glm::vec3> points;
	std::vector<int> ind;
	int counter = 0;

	float dis = 0.003;

	for ( int i = -siz; i < siz; ++i )
	{

		for ( int j = -siz; j < siz; ++j )
		{

			float x = i * dis;
			float y = j * dis;
			float z = 0;
			points.push_back( glm::vec3( x, y, z ) );

			ind.push_back( counter );

			counter++;

		}

	}

	// We create a buffer ID so that we can later assign it to our buffers.
	unsigned int VBOO, VAOO, EBOO;
	glGenVertexArrays( 1, &VAOO );
	glGenBuffers( 1, &VBOO );
	glGenBuffers( 1, &EBOO );

	// Bind Vertex Array Object.
	glBindVertexArray( VAOO );

	// Copy our vertices array in a buffer for OpenGL to use.
	// We assign our buffer ID to our new buffer and we overload it with our triangles array.
	glBindBuffer( GL_ARRAY_BUFFER, VBOO );
	//glBufferData( GL_ARRAY_BUFFER, points.size() * sizeof( Points ), passPoints, GL_STATIC_DRAW );
	glBufferData( GL_ARRAY_BUFFER, points.size() * sizeof( glm::vec3 ), &points.front(), GL_STATIC_DRAW );

	// Copy our indices in an array for OpenGL to use.
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBOO );
	//glBufferData( GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof( GLint ), passIndex, GL_STATIC_DRAW );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof( int ), &ind.front(), GL_STATIC_DRAW );

	// Set our vertex attributes pointers.
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), ( void* ) 0 );
	glEnableVertexAttribArray( 0 );
	/*
	// Set our texture coordinates attributes.
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* )( 6 * sizeof( float ) ) );
	glEnableVertexAttribArray( 2 );
	*/

	// Unbind the VBO.
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	// Unbind the VAO.
	glBindVertexArray( 0 );

	// Load them textures.
	//unsigned int tex = loadTexture( "Cassini-Projection-2.jpg" );
	
	BufferA.use();
	BufferA.setInt( "iChannel0", 0 );
	BufferA.setInt( "iChannel1", 1 );

	BufferB.use();
	BufferB.setInt( "iChannel0", 1 );
	BufferB.setInt( "iChannel1", 0 );
	BufferB.setInt( "iChannel2", 2 );

	BufferC.use();
	BufferC.setInt( "iChannel0", 0 );
	BufferC.setInt( "iChannel1", 1 );

	BufferD.use();
	BufferD.setInt( "iChannel0", 0 );
	BufferD.setInt( "iChannel1", 1 );
	BufferD.setInt( "iChannel2", 2 );

	Image.use();
	Image.setInt( "iChannel0", 0 );
	Image.setInt( "iChannel1", 1 );
	Image.setInt( "iChannel2", 2 );
	Image.setInt( "iChannel3", 3 );
	Image.setInt( "iChannel4", 4 );

	// BufferA Ping Pong FrameBuffers
	// Framebuffer configuration.
	unsigned int frameBuffer;
	glGenFramebuffers( 1, &frameBuffer );
	glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );

	glfwGetFramebufferSize( window, &WIDTH, &HEIGHT );

	// Create a colour attachment texture.
	unsigned int textureColourBuffer;
	glGenTextures( 1, &textureColourBuffer );
	glBindTexture( GL_TEXTURE_2D, textureColourBuffer );
	glTexImage2D( GL_TEXTURE_2D, 0, RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBuffer, 0 );
	
	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// BufferA frameBuffer configuration.
	// Framebuffer configuration.
	unsigned int frameBufferOne;
	glGenFramebuffers( 1, &frameBufferOne );
	glBindFramebuffer( GL_FRAMEBUFFER, frameBufferOne );

	// Create a colour attachment texture.
	unsigned int textureColourBufferOne;
	glGenTextures( 1, &textureColourBufferOne );
	glBindTexture( GL_TEXTURE_2D, textureColourBufferOne);
	glTexImage2D( GL_TEXTURE_2D, 0, RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferOne, 0 );

	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// BufferB frameBuffer configurations.
	// Framebuffer configuration.
	unsigned int frameBufferTwo;
	glGenFramebuffers( 1, &frameBufferTwo );
	glBindFramebuffer( GL_FRAMEBUFFER, frameBufferTwo );

	// Create a colour attachment texture.
	unsigned int textureColourBufferTwo;
	glGenTextures( 1, &textureColourBufferTwo );
	glBindTexture( GL_TEXTURE_2D, textureColourBufferTwo );
	glTexImage2D( GL_TEXTURE_2D, 0, RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferTwo, 0 );

	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Framebuffer configuration.
	unsigned int frameBufferThree;
	glGenFramebuffers( 1, &frameBufferThree );
	glBindFramebuffer( GL_FRAMEBUFFER, frameBufferThree );

	// Create a colour attachment texture.
	unsigned int textureColourBufferThree;
	glGenTextures( 1, &textureColourBufferThree );
	glBindTexture( GL_TEXTURE_2D, textureColourBufferThree );
	glTexImage2D( GL_TEXTURE_2D, 0, RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferThree, 0 );

	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// BufferC frameBuffer configurations.
	// Framebuffer configuration.
	unsigned int frameBufferFour;
	glGenFramebuffers(1, &frameBufferFour);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferFour);

	// Create a colour attachment texture.
	unsigned int textureColourBufferFour;
	glGenTextures( 1, &textureColourBufferFour );
	glBindTexture( GL_TEXTURE_2D, textureColourBufferFour );
	glTexImage2D( GL_TEXTURE_2D, 0, RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferFour, 0 );

	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// Framebuffer configuration.
	unsigned int frameBufferFive;
	glGenFramebuffers( 1, &frameBufferFive );
	glBindFramebuffer( GL_FRAMEBUFFER, frameBufferFive );

	// Create a colour attachment texture.
	unsigned int textureColourBufferFive;
	glGenTextures( 1, &textureColourBufferFive );
	glBindTexture( GL_TEXTURE_2D, textureColourBufferFive );
	glTexImage2D( GL_TEXTURE_2D, 0, RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferFive, 0 );

	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// BufferD frameBuffer configurations.
	// Framebuffer configuration.
	unsigned int frameBufferSix;
	glGenFramebuffers( 1, &frameBufferSix );
	glBindFramebuffer( GL_FRAMEBUFFER, frameBufferSix );

	// Create a colour attachment texture.
	unsigned int textureColourBufferSix;
	glGenTextures( 1, &textureColourBufferSix );
	glBindTexture( GL_TEXTURE_2D, textureColourBufferSix );
	glTexImage2D( GL_TEXTURE_2D, 0, RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferSix, 0 );

	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// Framebuffer configuration.
	unsigned int frameBufferSeven;
	glGenFramebuffers( 1, &frameBufferSeven );
	glBindFramebuffer( GL_FRAMEBUFFER, frameBufferSeven );

	// Create a colour attachment texture.
	unsigned int textureColourBufferSeven;
	glGenTextures( 1, &textureColourBufferSeven );
	glBindTexture( GL_TEXTURE_2D, textureColourBufferSeven );
	glTexImage2D( GL_TEXTURE_2D, 0, RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBufferSeven, 0 );

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Add an image.
	unsigned int tex = loadTexture( texturePath );

	// We want to know if the frame we are rendering is even or odd.
	bool even = true;

	// Setup input for GUI.
	ImVec4 leftMouseColour = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );
	ImVec4 rightMouseColour = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );
	std::string randomOrNot = "Random Colours!";
	std::string negativeOrNot = "Negative Colours!";

	// Render Loop.
	while( !glfwWindowShouldClose( window ) )
	{

		static float diffusionRate = 0.25f;
		static float sizeOfPainter = 0.05f;
		static float damping = 1.0f;
		static int randomColours = 1;
		static int negativeColours = 1;

		glfwGetFramebufferSize( window, &WIDTH, &HEIGHT );

		//glfwGetCursorPos( window, &xPos, &yPos );

		// Input.
		processInput( window );

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		ImGui::Begin( "Graphical User Interface" );   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text( "Pick your weapons!" );    
		ImGui::SliderFloat( "Diffusion Rate", &diffusionRate, 0.0f, 1.0f );
		ImGui::SliderFloat( "Size of Mouse Painter", &sizeOfPainter, 0.0f, 1.0f );  // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::SliderFloat( "Damping factor: 1 for no damping, less for more", &damping, 0.0f, 1.0f );
		if( ImGui::Button( "Left-Click Random Colours" ) )

			if( randomColours == 0 )
			{

				randomColours = 1;
				randomOrNot = "Random Colours!";

			}

			else
			{
			
				randomColours = 0;
				randomOrNot = "Not Random Colours";

			}

		ImGui::Text( randomOrNot.c_str() );
		ImGui::ColorEdit3( "Left-Click Colour", ( float* ) &leftMouseColour );
		if( ImGui::Button( "Right-Click Negative Colours" ) )

			if( negativeColours == 0 )
			{

				negativeColours = 1;
				negativeOrNot = "Negative Colours!";

			}

			else
			{
			
				negativeColours = 0;
				negativeOrNot = "Not Negative Colours";

			}

		ImGui::Text(negativeOrNot.c_str());
		ImGui::ColorEdit3( "Right-Click Colour", ( float* ) &rightMouseColour ); // Edit 3 floats representing a color
		ImGui::End();

		// Render.

		// Bind to frameBuffer and draw scene as normally would to colour texture.
		glBindFramebuffer( GL_FRAMEBUFFER, even ? frameBuffer : frameBufferOne );

		glClearColor( 0.2f, 0.3f, 0.1f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );

		BufferA.use();
		float currentFrame = glfwGetTime();
		//Set the iTimeDelta uniform.
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		BufferA.setFloat( "iTimeDelta", deltaTime );
		// Set the iTime uniform.
		float timeValue = currentFrame;
		BufferA.setFloat( "iTime", timeValue );
		// Set the iResolution uniform.
		BufferA.setVec2( "iResolution", WIDTH, HEIGHT );
		// Input the size of the Mouse Painter.
		BufferA.setFloat( "siz", sizeOfPainter );


		// Input iMouse.
		glfwGetCursorPos( window, &xPos, &yPos );
		yPos = HEIGHT - yPos;

		xDif = xPos - xPre;
		yDif = yPos - yPre;

		const float dx = 0.5;
		const float dt = dx * dx * 0.5;

		if( xDif != 0 && pressed > 0.5 )
		{
		
			if( deltaTime == 0 ) deltaTime = 1;
		
			vX = xDif / deltaTime;

		}

		if( yDif != 0 && pressed > 0.5 )
		{

			if( deltaTime == 0 ) deltaTime = 1;

			vY = yDif / deltaTime;

		}

		// BufferA
		BufferA.setVec3( "iMouse", xPos, yPos, pressed );

		glBindVertexArray( VAO );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferOne : textureColourBuffer );
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferThree : textureColourBufferTwo );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
		glBindVertexArray( 0 );

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		// BufferB
		glBindFramebuffer( GL_FRAMEBUFFER, even ? frameBufferTwo : frameBufferThree );

		glClearColor( 0.2f, 0.3f, 0.1f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );

		BufferB.use();
		BufferB.setInt( "iFrame", frames );
		//Set the iTimeDelta uniform.
		BufferB.setFloat( "iTimeDelta", deltaTime );
		// Set the iTime uniform.
		BufferB.setFloat( "iTime", timeValue );
		// Set the iResolution uniform.
		BufferB.setVec2( "iResolution", WIDTH, HEIGHT );
		// Input iMouse.
		BufferB.setVec4( "iMouse", xPos, yPos, pressed, right_pressed );
		// Input mouse iVel.
		BufferB.setVec2( "iVel", vX, vY );
		// Input colour from GUI.
		BufferB.setVec4( "iColour", leftMouseColour.x, leftMouseColour.y, leftMouseColour.z, leftMouseColour.w );
		BufferB.setVec4( "iColourOne", rightMouseColour.x, rightMouseColour.y, rightMouseColour.z, rightMouseColour.w );
		// Input the size of the Mouse Painter.
		BufferB.setFloat( "siz", sizeOfPainter );
		// Input the damping factor.
		BufferB.setFloat( "iDamping", damping );
		// Input the colour flag.
		BufferB.setInt( "iColourFlag", randomColours );
		// Input the negative flag.
		BufferB.setInt( "iNegativeFlag", negativeColours );
		// Input the diffusion rate float.
		BufferB.setFloat( "iDiffusion", diffusionRate );

		glBindVertexArray( VAO );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferOne : textureColourBuffer );
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferThree : textureColourBufferTwo );
		glActiveTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D, tex );
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray( 0 );

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );

		// BufferC
		glBindFramebuffer( GL_FRAMEBUFFER, even ? frameBufferFour : frameBufferFive );

		glClearColor( 0.2f, 0.3f, 0.1f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );

		BufferC.use();
		//Set the iTimeDelta uniform.
		BufferC.setFloat( "iTimeDelta", deltaTime );
		// Set the iTime uniform.
		BufferC.setFloat( "iTime", timeValue );
		// Set the iResolution uniform.
		BufferC.setVec2( "iResolution", WIDTH, HEIGHT );
		// Input iMouse.
		BufferC.setVec3( "iMouse", xPos, yPos, pressed );

		glBindVertexArray( VAO );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferFive : textureColourBufferFour );
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferThree : textureColourBufferTwo );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
		glBindVertexArray( 0 );

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );

		// BufferD
		glBindFramebuffer( GL_FRAMEBUFFER, even ? textureColourBufferSix : textureColourBufferSeven );
		glClearColor( 0.2f, 0.3f, 0.1f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );

		BufferD.use();
		//Set the iTimeDelta uniform.
		BufferD.setFloat( "iTimeDelta", deltaTime );
		// Set the iTime uniform.
		BufferD.setFloat( "iTime", timeValue );
		// Set the iResolution uniform.
		BufferD.setVec2( "iResolution", WIDTH, HEIGHT );
		// Input iMouse.
		BufferD.setVec3( "iMouse", xPos, yPos, pressed );

		glBindVertexArray( VAOO );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferOne : textureColourBuffer );
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferSeven : textureColourBufferSix );
		glActiveTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferThree : textureColourBufferTwo );
		glDrawElements( GL_POINTS, ind.size(), GL_UNSIGNED_INT, 0 );
		glEnable( GL_PROGRAM_POINT_SIZE );
		glBindVertexArray( 0 );

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );

		// Our last stage for our colour Navier-Stokes and mixing it with the Wave Equation.
		Image.use();
		// Set the iResolution uniform.
		Image.setVec2( "iResolution", WIDTH, HEIGHT );
		// Set the iTime uniform.
		Image.setFloat( "iTime", timeValue );
		glBindVertexArray( VAO );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferOne : textureColourBuffer );
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferThree : textureColourBufferTwo );
		glActiveTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferFive : textureColourBufferFour );
		glActiveTexture( GL_TEXTURE3 );
		glBindTexture( GL_TEXTURE_2D, even ? textureColourBufferSeven : textureColourBufferSix );
		glActiveTexture( GL_TEXTURE4 );
		glBindTexture( GL_TEXTURE_2D, tex );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
		
		glBindVertexArray( 0 );

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers( window );
		glfwPollEvents();

		xPre = xPos;
		yPre = yPos;

		even = !even;

		frameCount++;
		frames++;
		finalTime = time( NULL );
		if( finalTime - initialTime > 0 )
		{
		
			FPS = frameCount / ( finalTime - initialTime );
			std::stringstream title;
			title << "Navier-Stokes Simulation, FPS : " << FPS;
			frameCount = 0;
			initialTime = finalTime;

			glfwSetWindowTitle( window, title.str().c_str() );

		}

	}

	// De-allocate all resources once they've outlived their purpose.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glDeleteVertexArrays( 1, &VAO );
	glDeleteVertexArrays( 1, &VAOO );
	glDeleteBuffers( 1, &VBO );
	glDeleteBuffers( 1, &VBOO );
	glDeleteBuffers( 1, &EBO );
	glDeleteBuffers( 1, &EBOO );

	glfwTerminate();
	return 0;
}

void processInput( GLFWwindow *window )
{

	if ( glfwGetKey( window, GLFW_KEY_ESCAPE) == GLFW_PRESS )
		glfwSetWindowShouldClose( window, true );

}

void framebuffer_size_callback( GLFWwindow* window, int width, int height )
{

	glViewport( 0, 0, width, height );

}

static void cursorPositionCallback( GLFWwindow *window, double xPos, double yPos )
{

	xPos = 2.0 * xPos / WIDTH - 1;
	yPos = -2.0 * yPos / HEIGHT + 1;

}

static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) 
{

	if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS )
	{
	
		if( pressed == 0.0 )
		{
		
			pressed = 1.0;
		
		}
	
	}

	if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE )
	{

		if ( pressed == 1.0 )
		{

			pressed = 0.0;

		}

	}

	if( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS )
	{

		if (right_pressed == 0.0)
		{

			right_pressed = 1.0;

		}
	}

	if( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE )
	{ 

		if( right_pressed = 1.0 )
		{
		
			right_pressed = 0.0;
		
		}

	}
	//std::cout << std::boolalpha << pressed << std::endl;

}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}