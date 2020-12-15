/* OpenGL */
#include <glad/glad.h>
#include <glad/glad.c>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>

/*#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 720*/
#define WINDOW_WIDTH 3160
#define WINDOW_HEIGHT 1980
#define ZOOM_FACTOR 1.5f
#define ZOOM_FACTOR2 1.1f

void redraw(GLFWwindow* win);

int main()
{
	glfwInit(); glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
	GLFWwindow* win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Mandelbrot On The GPU", NULL, NULL);
	glfwMakeContextCurrent(win); gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	int windowSize[] = { WINDOW_WIDTH, WINDOW_HEIGHT };
#ifdef _DEBUG
	std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << " - This Should Be At Least 4.00\n";
#endif // DEBUG

	unsigned int shader = glCreateProgram(), vertex = glCreateShader(GL_VERTEX_SHADER), fragment = glCreateShader(GL_FRAGMENT_SHADER);
	{
		std::ifstream inFile; inFile.open("vert.glsl"); std::string vertexShaderSource; std::getline(inFile, vertexShaderSource, '\0'); inFile.close();
		const char * vshadersource = vertexShaderSource.c_str();
		glShaderSource(vertex, 1, &vshadersource, NULL); glCompileShader(vertex);
		//std::cout << vshadersource << '\n';
		GLint success = 0;
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		std::cout << "Vertex State: " << success << '\n';
		if (success == 0) exit(-1);
		inFile.open("frag.glsl"); std::string fragShaderSource; std::getline(inFile, fragShaderSource, '\0'); inFile.close();
		const char* fshadersource = fragShaderSource.c_str();
		glShaderSource(fragment, 1, &fshadersource, NULL); glCompileShader(fragment);
		//std::cout << fshadersource << '\n';
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		std::cout << "Fragment State: " << success << '\n';
		if (success == 0) exit(-1);
	}

	glAttachShader(shader, vertex); glAttachShader(shader, fragment); glLinkProgram(shader);
	glDeleteShader(vertex); glDeleteShader(fragment); glUseProgram(shader);

	/* Set Shader Values */
	glUniform2iv(glGetUniformLocation(shader, "windowsize"), 1, windowSize);
	glUniform1i(glGetUniformLocation(shader, "maxIterations"), 512);

	float plane[] =
	{
		-1.f, 1.f,  1.f,  1.f, -1.f, -1.f,
		 1.f, 1.f, -1.f, -1.f,  1.f, -1.f
	};

	unsigned int VAO, VBO; glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
	glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(plane), &plane, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); glEnableVertexAttribArray(0);

	bool isKeyDown = false;
	auto tp = std::chrono::high_resolution_clock::now();
	redraw(win);

	while (!glfwWindowShouldClose(win))
	{
		/* Update */
		if (glfwGetKey(win, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(win, true);
		else if (glfwGetKey(win, GLFW_KEY_F) && !isKeyDown)
		{
			int w, h; isKeyDown = true;
			if (!glfwGetWindowMonitor(win))
			{
				glfwSetWindowPos(win, 0, 0);
				// Remove glfwGetPrimaryMonitor() here to make psuedo fullscreen
				glfwSetWindowMonitor(win, glfwGetPrimaryMonitor(), 0, 0, glfwGetVideoMode(glfwGetPrimaryMonitor())->width,
					glfwGetVideoMode(glfwGetPrimaryMonitor())->height, glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);
			}
			else
			{
				glfwSetWindowMonitor(win, NULL, glfwGetVideoMode(glfwGetPrimaryMonitor())->width / 2 - WINDOW_WIDTH / 2,
					glfwGetVideoMode(glfwGetPrimaryMonitor())->height / 2 - WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT,
					glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);	
			}
			glfwGetWindowSize(win, &w, &h); glViewport(0, 0, w, h); windowSize[0] = w; windowSize[1] = h;
			glUniform2iv(glGetUniformLocation(shader, "windowsize"), 1, windowSize); redraw(win);
		}
		else if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) && !isKeyDown)
		{
			isKeyDown = true;
			double RE_BEGIN, IM_BEGIN, WIDTH, HEIGHT, x, y;
			glfwGetCursorPos(win, &x, &y);
			glGetUniformdv(shader, glGetUniformLocation(shader, "RE_BEGIN"), &RE_BEGIN);
			glGetUniformdv(shader, glGetUniformLocation(shader, "IM_BEGIN"), &IM_BEGIN);
			glGetUniformdv(shader, glGetUniformLocation(shader, "WIDTH"), &WIDTH);
			glGetUniformdv(shader, glGetUniformLocation(shader, "HEIGHT"), &HEIGHT);
			
			RE_BEGIN += x * WIDTH / static_cast<double>(windowSize[0]) - WIDTH / (2.0 * ZOOM_FACTOR);
			IM_BEGIN += (HEIGHT - (long double)y * HEIGHT / static_cast<double>(windowSize[1])) - HEIGHT / (2.0 * ZOOM_FACTOR);
			WIDTH /= ZOOM_FACTOR; HEIGHT /= ZOOM_FACTOR;
			
			glUniform1d(glGetUniformLocation(shader, "RE_BEGIN"), RE_BEGIN);
			glUniform1d(glGetUniformLocation(shader, "IM_BEGIN"), IM_BEGIN);
			glUniform1d(glGetUniformLocation(shader, "WIDTH"), WIDTH);
			glUniform1d(glGetUniformLocation(shader, "HEIGHT"), HEIGHT);

			system("cls");
			auto start = std::chrono::high_resolution_clock::now();
			redraw(win);
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << "Render Time: " << std::chrono::duration<double>(end - start).count() << '\n';
			glfwSetCursorPos(win, windowSize[0] / 2, windowSize[1] / 2);
		}
		else if (glfwGetKey(win, GLFW_KEY_R) && !isKeyDown)
		{
			isKeyDown = true;

			glUniform1d(glGetUniformLocation(shader, "RE_BEGIN"), -2.5);
			glUniform1d(glGetUniformLocation(shader, "IM_BEGIN"), -1.0);
			glUniform1d(glGetUniformLocation(shader, "WIDTH"), 3.0);
			glUniform1d(glGetUniformLocation(shader, "HEIGHT"), 2.0);
			
			// Higher Powers
			//glUniform1d(glGetUniformLocation(shader, "RE_BEGIN"), -2.25);
			//glUniform1d(glGetUniformLocation(shader, "IM_BEGIN"), -1.5);
			//glUniform1d(glGetUniformLocation(shader, "WIDTH"), 3.0*1.5);
			//glUniform1d(glGetUniformLocation(shader, "HEIGHT"), 2.0*1.5);

			redraw(win);
		}
		else if (!glfwGetKey(win, GLFW_KEY_F) && !glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) && !glfwGetKey(win, GLFW_KEY_R) && !glfwGetKey(win, GLFW_KEY_Q) && !glfwGetKey(win, GLFW_KEY_A) && isKeyDown)
			isKeyDown = false;
		else if (glfwGetKey(win, GLFW_KEY_Q))
		{
			isKeyDown = true;
			double RE_BEGIN, IM_BEGIN, WIDTH, HEIGHT, x, y;
			glfwGetCursorPos(win, &x, &y);
			glGetUniformdv(shader, glGetUniformLocation(shader, "RE_BEGIN"), &RE_BEGIN);
			glGetUniformdv(shader, glGetUniformLocation(shader, "IM_BEGIN"), &IM_BEGIN);
			glGetUniformdv(shader, glGetUniformLocation(shader, "WIDTH"), &WIDTH);
			glGetUniformdv(shader, glGetUniformLocation(shader, "HEIGHT"), &HEIGHT);

			RE_BEGIN += x * WIDTH / static_cast<double>(windowSize[0]) - WIDTH / (2.0 * ZOOM_FACTOR2);
			IM_BEGIN += (HEIGHT - (long double)y * HEIGHT / static_cast<double>(windowSize[1])) - HEIGHT / (2.0 * ZOOM_FACTOR2);
			WIDTH /= ZOOM_FACTOR2; HEIGHT /= ZOOM_FACTOR2;

			glUniform1d(glGetUniformLocation(shader, "RE_BEGIN"), RE_BEGIN);
			glUniform1d(glGetUniformLocation(shader, "IM_BEGIN"), IM_BEGIN);
			glUniform1d(glGetUniformLocation(shader, "WIDTH"), WIDTH);
			glUniform1d(glGetUniformLocation(shader, "HEIGHT"), HEIGHT);

			system("cls");
			auto start = std::chrono::high_resolution_clock::now();
			redraw(win);
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << "Render Time: " << std::chrono::duration<double>(end - start).count() << '\n';
			glfwSetCursorPos(win, windowSize[0] / 2, windowSize[1] / 2);
		}
		else if (glfwGetKey(win, GLFW_KEY_A))
		{
			isKeyDown = true;
			double RE_BEGIN, IM_BEGIN, WIDTH, HEIGHT, x, y;
			glfwGetCursorPos(win, &x, &y);
			glGetUniformdv(shader, glGetUniformLocation(shader, "RE_BEGIN"), &RE_BEGIN);
			glGetUniformdv(shader, glGetUniformLocation(shader, "IM_BEGIN"), &IM_BEGIN);
			glGetUniformdv(shader, glGetUniformLocation(shader, "WIDTH"), &WIDTH);
			glGetUniformdv(shader, glGetUniformLocation(shader, "HEIGHT"), &HEIGHT);

			RE_BEGIN -= x * WIDTH / static_cast<double>(windowSize[0]) - WIDTH / (2.0 * ZOOM_FACTOR2);
			IM_BEGIN -= (HEIGHT - (long double)y * HEIGHT / static_cast<double>(windowSize[1])) - HEIGHT / (2.0 * ZOOM_FACTOR2);
			WIDTH *= ZOOM_FACTOR2; HEIGHT *= ZOOM_FACTOR2;

			glUniform1d(glGetUniformLocation(shader, "RE_BEGIN"), RE_BEGIN);
			glUniform1d(glGetUniformLocation(shader, "IM_BEGIN"), IM_BEGIN);
			glUniform1d(glGetUniformLocation(shader, "WIDTH"), WIDTH);
			glUniform1d(glGetUniformLocation(shader, "HEIGHT"), HEIGHT);

			system("cls");
			auto start = std::chrono::high_resolution_clock::now();
			redraw(win);
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << "Render Time: " << std::chrono::duration<double>(end - start).count() << '\n';
			glfwSetCursorPos(win, windowSize[0] / 2, windowSize[1] / 2);
		}
		/* Render */
		//glClear(GL_COLOR_BUFFER_BIT);
		glfwPollEvents();
		std::this_thread::sleep_until(tp + std::chrono::milliseconds(17)); tp += std::chrono::milliseconds(17);
	}

	glfwDestroyWindow(win);
	glfwTerminate();
}

void redraw(GLFWwindow* win)
{
	glDrawArrays(GL_TRIANGLES, 0, 12); glfwSwapBuffers(win);
}