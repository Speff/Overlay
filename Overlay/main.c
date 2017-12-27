#define TRACE_ON

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Windows.h>
#include "window.h"


#ifndef TRACE_ON
#define TRACE 0
#else
#define TRACE 1
#endif

#define AXIS_X_MARGIN_LEFT      30
#define AXIS_X_MARGIN_RIGHT     10
#define AXIS_Y_MARGIN_TOP       10
#define AXIS_Y_MARGIN_BOTTOM    30

#define PIPE_BUFFER_SIZE	1024

static const struct
{
	float x, y;
	float r, g, b;
} vertices[3] =
{
	{ -0.6f, -0.4f, 1.f, 0.f, 0.f },
	{ 0.6f, -0.4f, 0.f, 1.f, 0.f },
	{ 0.f,  0.6f, 0.f, 0.f, 1.f }
};

#define N_HP_VALS 2000
#define N_CHAR_TITLE 1024
struct pipeData {
	unsigned int combo;
	float acc;
	unsigned int currentTime;
	unsigned int endTime;
	struct point hp[N_HP_VALS];
	char songName[N_CHAR_TITLE];
};

void drawIMgui(int width, int height, const char* displayText, float acc, unsigned int combo) {
	char *comboString = (char*)calloc(255, sizeof(char));
	char *accString = (char*)calloc(255, sizeof(char));

	snprintf(comboString, 255, "Current Combo: %ux", combo);
	snprintf(accString, 255, "Current Acc: %.2f%%", acc);

	// GUI ================================================================
	ImGui_ImplGlfwGL3_NewFrame();

	ImGuiWindowFlags blankFlag = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

	igPushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	struct ImVec4 colWindowBG = { 0.9f, 0.9f, 0.9f, 1.0f };
	struct ImVec4 colText = { 0.1f, 0.1f, 0.1f, 1.0f };

	igPushStyleColor(ImGuiCol_WindowBg, colWindowBG);
	igPushStyleColor(ImGuiCol_Text, colText);
	igBegin("Text Window", false, blankFlag);
	{
		const struct ImVec2 igOriginPoint = { 0, 0 };
		const struct ImVec2 igTextWindowSize = { (float)width, (float)height / 5 };

		igSetWindowPos(igOriginPoint, true);
		igSetWindowSize(igTextWindowSize, true);
		//igSetWindowSize({ (float)width, (float)height }, true);
		//igText("Testing, 1-2-3");
		//igText("Testing, Lorem ipsum dolor sit blah blah blah");
		igText(displayText);
		igText(accString);
		igText(comboString);
	}


	const struct ImVec2 igGraphWindowOriginPoint = { (float)0, (float)height / 5 };
	const struct ImVec2 igGraphWindowSize = { (float)width, (float)height * 4 / 5 };

	struct ImVec4 colAxis = { 0.4f, 0.4f, 0.4f, 1.0f };
	struct ImVec2 vertsAxis[] = {
		{ igGraphWindowOriginPoint.x + AXIS_X_MARGIN_LEFT, igGraphWindowOriginPoint.y + AXIS_Y_MARGIN_TOP },
		{ igGraphWindowOriginPoint.x + AXIS_X_MARGIN_LEFT, igGraphWindowOriginPoint.y + igGraphWindowSize.y - AXIS_Y_MARGIN_BOTTOM },
		{ igGraphWindowSize.x - AXIS_X_MARGIN_RIGHT, igGraphWindowOriginPoint.y + igGraphWindowSize.y - AXIS_Y_MARGIN_BOTTOM } };


	colWindowBG.x = 0.1f;
	colWindowBG.y = 0.1f;
	colWindowBG.z = 0.1f;
	igPushStyleColor(ImGuiCol_WindowBg, colWindowBG);
	igBegin("Graph Window", false, blankFlag);
	{

		igSetWindowPos(igGraphWindowOriginPoint, true);
		igSetWindowSize(igGraphWindowSize, true);

		struct ImDrawList* drawlist = igGetWindowDrawList();
		ImDrawList_AddLine(drawlist, vertsAxis[0], vertsAxis[1], igGetColorU32Vec(&colAxis), 2);
		ImDrawList_AddLine(drawlist, vertsAxis[1], vertsAxis[2], igGetColorU32Vec(&colAxis), 2);

	}
	igEnd();
}

int main() {
	GLFWwindow* window = NULL;
	int width = 0, height = 0;

	GLuint vertex_shader, fragment_shader, program;
	GLint attribute_coord2d, uniform_offset_x, uniform_scale_x, uniform_cutoff;
	GLuint vao; GLuint vbo;
	GLuint tex, fbo;
	struct ImVec4 clear_color = { 0.05f, 0.05f, 0.05f, 1.00f };

	OVERLAPPED connectOverlap;
	size_t nBytesTransferred;
	bool waitingForRequest = true;
	//char data[PIPE_BUFFER_SIZE];
	struct pipeData *dRec = (struct pipeData*)calloc(1, sizeof(struct pipeData));
	char* displayText = (char*)calloc(1, sizeof(char));

	// Timer
	double timeStarted = glfwGetTime();

	// List of points to plot
	struct point graph[N_HP_VALS];
	for (int i = 0; i < N_HP_VALS; i++) {
		float x = (float)((i - N_HP_VALS / 2.0) / N_HP_VALS/20.0f);
		graph[i].x = x;
		//graph[i].y = sinf(6.28f * x*(x + 5.5*x)*(x / 6 - 20.0) * (1.5 / 20.0f)) / x;
		graph[i].y = (GLfloat)(sinf((float)(6.28 * x / 5))/x - .5);
	}

	// Initialize GLFW
	initGLFW(&window, &width, &height);

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	program = glCreateProgram();
	compileShaders(vertex_shader, fragment_shader, program);

	// Get shader internal addresses
	attribute_coord2d = glGetAttribLocation(program, "coord2d");
	uniform_offset_x = glGetUniformLocation(program, "offset_x");
	uniform_scale_x = glGetUniformLocation(program, "scale_x");
	uniform_cutoff = glGetUniformLocation(program, "cutoff");

	// Gen and fill VBO to hold vertices
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, N_HP_VALS * sizeof(struct point), graph, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Generate VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Generate fbo and tex for MSAA
	glGenTextures(1, &tex);
	glGenFramebuffers(1, &fbo);

	// Pipe setup
	connectOverlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	connectOverlap.Offset = 0;
	connectOverlap.OffsetHigh = 0;
	fprintf(stdout, "Server started\n");

	HANDLE pipe = CreateNamedPipe(
		"\\\\.\\pipe\\osuPipe",
		PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
		PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		sizeof(struct pipeData),
		sizeof(struct pipeData),
		60*1000,
		NULL);

	if (pipe == INVALID_HANDLE_VALUE) {
		printf("Error: %u (line: %u)\n", GetLastError(), __LINE__);
	}

	if (!ConnectNamedPipe(pipe, &connectOverlap)) {
		printf("Waiting for pipe to establish connections\n");
		while (HasOverlappedIoCompleted(&connectOverlap)) printf("%u\n", (unsigned int)connectOverlap.Internal);
	}
	ReadFile(pipe, dRec, sizeof(struct pipeData), NULL, &connectOverlap);
	waitingForRequest = true;


	//if (TRACE) printf("line:%u: GL Error code: %u\n", __LINE__, glGetError());

	char* displayErrorText = (char*)calloc(1024, sizeof(char));

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		bool fSuccess = GetOverlappedResult(pipe, &connectOverlap, (LPDWORD)(&nBytesTransferred), FALSE); // See if 
		if (fSuccess) {
			free(displayText);
			displayText = (char*)calloc(strlen("Recieved text: ") + nBytesTransferred + 1, sizeof(char));
			strcat(displayText, "Recieved text: ");
			memcpy(displayText + strlen("Recieved text: "), dRec->songName, nBytesTransferred);

			memcpy(graph, dRec->hp, N_HP_VALS * sizeof(struct point));

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, N_HP_VALS * sizeof(struct point), graph);
			// glBufferSubData(GL_ARRAY_BUFFER, UPDATE_OFFSET_LOW, N_UPDATES * SIZEOF(POINT), graph);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			FlushFileBuffers(pipe);

			//printf("%s\nBytes xferred: %u\n", displayText, nBytesTransferred);


			ReadFile(pipe, dRec, sizeof(struct pipeData), NULL, &connectOverlap);
		}
		else {
			if (GetLastError() == ERROR_IO_INCOMPLETE){
			}
			else{
				printf("Pipe broken\n");
				FlushFileBuffers(pipe);
				DisconnectNamedPipe(pipe);

				if (!ConnectNamedPipe(pipe, &connectOverlap)) {
					//printf("Waiting for pipe to re-init after disconnect\n");
					while (HasOverlappedIoCompleted(&connectOverlap)) printf("%u\n", (unsigned int)(connectOverlap.Internal));
					printf("Pipe restored after disconnect\n");
				}

				if (!ReadFile(pipe, dRec, sizeof(struct pipeData), NULL, &connectOverlap)){
					if (GetLastError() != ERROR_PIPE_LISTENING) 
						printf("ReadFile Error: %u (line: %u)\n", GetLastError(), __LINE__);
				}
			}
			//else{
			//	printf("Error: %u (line: %u)\n", GetLastError(), __LINE__);
			//}
		}
		
		//printf("Error: %u\n", GetLastError());

		// Set up viewport
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// Lay out UI
		drawIMgui(width, height, displayText, dRec->acc, dRec->combo);

		// Render everything to the framebuffer
		// Bind and set up framebuffers for MSAA
		bindFBO(tex, fbo, width, height);

		// Set up viewport for rendering plot
		struct ImVec4 viewportBounds = {
			(float)AXIS_X_MARGIN_LEFT + 2,
			(float)AXIS_Y_MARGIN_BOTTOM + 2,
			(float)(width - AXIS_X_MARGIN_RIGHT - AXIS_X_MARGIN_LEFT),
			(float)((height * 4 / 5) - AXIS_Y_MARGIN_TOP - AXIS_Y_MARGIN_BOTTOM)
		};
		glViewport((GLint)viewportBounds.x, (GLint)viewportBounds.y, (GLint)viewportBounds.z, (GLint)viewportBounds.w);

		// Clear screen
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		// Render IMgui
		igRender();

		// Render OpenGL
		glUseProgram(program);

		// Set shader uniforms
		glUniform1f(uniform_offset_x, 0);
		glUniform1f(uniform_scale_x, 0.1f);
		//glUniform1f(uniform_cutoff, 2.0*((glfwGetTime() / 3) - (int)(glfwGetTime() / 3) - 0.5f));
		glUniform1f(uniform_cutoff, 1.0f);
		//printf("Time: %f\n", glfwGetTime() - (int)glfwGetTime());

		// Activate VBO for rendering
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// Define VBO attributes
		glEnableVertexAttribArray(attribute_coord2d);
		glVertexAttribPointer(
			attribute_coord2d,   // attribute
			2,                   // number of elements per vertex, here (x,y)
			GL_FLOAT,            // the type of each element
			GL_FALSE,            // take our values as-is
			0,                   // no space between values
			0                    // use the vertex buffer object
		);

		// Draw points
		glLineWidth(1.25);
		glDrawArrays(GL_LINE_STRIP, 0, 2000);

		// Unbind framebuffer
		renderFBO(tex, width, height);

		// Unbind program
		glUseProgram(0);
		// /GUI ===============================================================

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}