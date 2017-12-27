#include <window.h>

void initGLFW(GLFWwindow** window, int* width, int* height) {
	// Initialize GLFW
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) {
		fprintf(stdout, "[GFLW] failed to init!\n");
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Open GLFW window
	*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "osu! Monitor", NULL, NULL);
	if (!*window) {
		fprintf(stdout, "GLFW window intialization failed");
	}
	glfwMakeContextCurrent(*window);
	glfwGetWindowSize(*window, width, height);

	// Set GLFW callbacks
	glfwSetKeyCallback(*window, keyCallback);
	glfwSetWindowCloseCallback(*window, closeCallback);

	// Load OpenGL extensions (using GLAD)
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	//glfwSwapInterval(1);

	// Set up OpenGL
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);

	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(*window, true);
}

void compileShaders(GLuint vertex_shader, GLuint fragment_shader, GLuint program) {
	char *vertex_shader_text, *fragment_shader_text;
	size_t sourceSize = 0;

	FILE *fp = fopen("vert.GLSL", "r");
	if (fp == NULL) printf("vert.GLSL not found\n");
	fseek(fp, 0L, SEEK_END);
	sourceSize = ftell(fp);
	rewind(fp);
	vertex_shader_text = (char*)calloc(sourceSize + 1, sizeof(char));
	fread(vertex_shader_text, sizeof(char), sourceSize, (FILE*)fp);
	//printf("Vert Shader size: %u\n", sourceSize);

	// Generate vertex shader
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);

	GLint success = 0;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	GLint logSize = 0;
	glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logSize);
	char *vert_log = (char*)malloc(sizeof(char)*logSize);
	glGetShaderInfoLog(vertex_shader, logSize + 1, NULL, vert_log);
	vert_log[logSize - 1] = 0;
	//printf("Vertex shader log (length: %u):\n%s\n", logSize, vert_log);

	fp = fopen("frag.GLSL", "r");
	if (fp == NULL) printf("frag.GLSL not found\n");
	fseek(fp, 0L, SEEK_END);
	sourceSize = ftell(fp);
	rewind(fp);
	fragment_shader_text = (char*)calloc(sourceSize + 1, sizeof(char));
	fread(fragment_shader_text, sizeof(char), sourceSize, (FILE*)fp);
	//printf("Vert Shader size: %u\n", sourceSize);

	// Generate fragment shader
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);

	success = 0;
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	logSize = 0;
	glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logSize);
	char *frag_log = (char*)malloc(sizeof(char)*logSize);
	glGetShaderInfoLog(fragment_shader, logSize + 1, NULL, frag_log);
	frag_log[logSize - 1] = 0;
	//printf("Frag shader log (length: %u):\n%s\n", logSize, frag_log);

	// Compile program
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	//printf("Vert Shader:\n%s\n\nFrag Shader:\n%s\n\n", vertex_shader_text, fragment_shader_text);
	//free(vertex_shader_text);

	success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	logSize = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
	char *prog_log = (char*)malloc(sizeof(char)*logSize);
	glGetProgramInfoLog(program, logSize + 1, NULL, prog_log);
	prog_log[logSize - 1] = 0;
	//printf("Program log (length: %u):\n%s\n", logSize, prog_log);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}
void closeCallback(GLFWwindow* window) {
	// Do stuff on close
}

void errorCallback(int e, const char *d) {
	printf("Error %d: %s\n", e, d);
}

void bindFBO(GLuint tex, GLuint fbo, int width, int height) {
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA8, width, height, false);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
}

void renderFBO(GLuint fbo, int width, int height) {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);   // Make sure no FBO is set as the draw framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo); // Make sure your multisampled FBO is the read framebuffer
	glDrawBuffer(GL_BACK);                       // Set the back buffer as the draw buffer
												 // Blit FBO to screen
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
}