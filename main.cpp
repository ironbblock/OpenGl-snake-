#ifndef SHADER_H_
#define SHADER_H_
#define NAME(x) #x
#define ABS(x) (x > 0 ? x : -x)
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <random>
#include <ctime>
#include <windows.h>
#include <Thread>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#pragma comment(lib, "winmm.lib")
using namespace std;
class Shader {
public:
	unsigned int ID;
	unsigned int vertex, fragment;
	Shader(const char* vPath, const char* fPath) {
		string vCode, fCode;
		ifstream vFile, fFile;
		stringstream vSS, fSS;
		vFile.open(vPath);
		fFile.open(fPath);
		vSS << vFile.rdbuf();
		fSS << fFile.rdbuf();
		vFile.close();
		fFile.close();
		vCode = vSS.str();
		fCode = fSS.str();
		const char* vShaderCode = vCode.c_str();
		const char* fShaderCode = fCode.c_str();

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	void use() {
		glUseProgram(ID);
	}
	void log() {
		int  success;
		char infoLog[512];
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}
};
#endif


const int WIDTH = 1080, HEIGHT = 1080, N = 1521;
float o = 0.005f, e = 0.0001f, targetFPS = 120.0, frameTime = 1.0 / 120;
unsigned int VAO, VBO, texture, ch = 10;
int width, height, nrChannels;
unsigned char* texData = nullptr;
int g[5][3] = {//[dx][point]
	{-1, -1, -1},
	{0, 2, 4},
	{1, 3, 5},
	{0, 1, 3},
	{2, 4, 5}
};
//		0	1(3)
//		
//		2(4)   5
//

int tg[5][3] = {//[tailDx][point]
	{-1, -1, -1},
	{1, 3, 5},
	{0, 2, 4},
	{2, 4, 5},
	{0, 1, 3}
};
struct Vertex {
	float x, y, z, R, G, B;
}v[6 * N];
struct vec {
	float x, y;
};
struct fPos {
	float x, y;
	Vertex operator + (const vec& a)const {
		float X = a.x + x, Y = a.y + y;
		return { X, Y, 0, 1, 1, 1 };
	}
	Vertex operator - (const vec& a)const {
		float X = x - a.x, Y = y - a.y;
		return { X, Y, 0, 1, 1, 1 };
	}
	float Dis(fPos a) {
		float dx = abs(x - a.x), dy = abs(y - a.y);
		return sqrt(dx * dx + dy * dy);
	}
};
struct iPos {
	int x, y;
	bool operator == (const iPos& a)const {
		return x == a.x && y == a.y;
	}
};
struct posHash {
	iPos p;
	size_t operator () (const iPos& a)const {
		size_t h1 = hash<int>{}(a.x);
		size_t h2 = hash<int>{}(a.y);
		return h1 ^ h2;
	}
};
struct posEqual {
	bool operator ()(const iPos& p1, const iPos& p2)const {
		return p1 == p2;
	}
};
unordered_map<iPos, int, posHash, posEqual> mp;
void debug(int x) {
	cout << x << endl;
}
void itof(Vertex q[], int st, int x, int y, float R = 1.0f, float G = 1.0f, float B = 1.0f) {
	fPos j = { 0.05f * x - 1, 0.05f * y - 1 };
	vec ul = { -0.025f, +0.025f }, ur = { 0.025f, 0.025f };
	q[st] = j + ul;
	q[st + 1] = q[st + 3] = j + ur;
	q[st + 2] = q[st + 4] = j - ur;
	q[st + 5] = j - ul;
	for (int i = 0; i < 6; i++) {
		q[st + i].R = R;
		q[st + i].G = G;
		q[st + i].B = B;
	}
}
Vertex food[6];
iPos ifood[1];
Vertex edge[] = {
	{-0.975f, 0.975f, 0, 0.5f, 0.5f, 0.5f},
	{-0.975f, -0.975f, 0, 0.5f, 0.5f, 0.5f},
	{-0.975f, 0.975f, 0, 0.5f, 0.5f, 0.5f},
	{0.975f, 0.975f, 0, 0.5f, 0.5f, 0.5f},
	{0.975f, 0.975f, 0, 0.5f, 0.5f, 0.5f},
	{0.975f, -0.975f, 0, 0.5f, 0.5f, 0.5f},
	{-0.975f, -0.975f, 0, 0.5f, 0.5f, 0.5f},
	{0.975f, -0.975f, 0, 0.5f, 0.5f, 0.5f}

};
//39 * 39
class Snake {// x, y belong [1,39]
public:
	int dx = 1, len = 2;
	iPos A[N];
	Vertex head() {
		fPos j = { 0.05f * A[0].x - 1, 0.05f * A[0].y - 1 };
		vec up = { 0, +0.025f }, dn = { 0, -0.025f }, lf = { -0.025f, 0 }, rt = { +0.025f, 0 };
		if (dx == 1) return j + lf;
		if (dx == 2) return j + rt;
		if (dx == 3) return j + up;
		if (dx == 4) return j + dn;
	}
	void init() {
		A[0] = { 20, 20 };
		A[1] = { 21, 20 };
		mp[A[0]]++;
		mp[A[1]]++;
	}
	void move() {
		mp[A[len - 1]]--;
		for (int i = len - 1; i > 0; i--) A[i] = A[i - 1];
		if (dx == 1) A[0].x = A[0].x - 1;//left
		if (dx == 2) A[0].x = A[0].x + 1;//right
		if (dx == 3) A[0].y = A[0].y + 1;//up
		if (dx == 4) A[0].y = A[0].y - 1;//down
		//Cartesian coordinate system
		mp[A[0]]++;
	}
	void turn(int i) {
		dx = i;
		PlaySound(TEXT("Sounds\\t04.wav"), NULL, SND_FILENAME | SND_ASYNC);
	}
	bool ifDie() {
		if (abs(head().x) >= 0.974f || abs(head().y) >= 0.974f || hitSelf()) {
			PlaySound(TEXT("Sounds\\die.wav"), NULL, SND_FILENAME | SND_ASYNC);
			return 1;
		}
		return 0;
	}
	void longer() {
		A[len] = A[len - 1];
		move();
		mp[A[len - 1]] = 1;
		len++;
	}
	void convert() {
		for (int i = 0; i < len * 6; i += 6) {
			itof(v, i, A[i / 6].x, A[i / 6].y);
		}
	}
	bool hitSelf() {
		if (len == 1) return 0;
		float dist = 999.0;
		fPos a = { head().x, head().y };
		for (int i = 1; i < len; i++) {
			float x = 0.05f * A[i].x - 1, y = 0.05f * A[i].y - 1;
			dist = min(dist, a.Dis({ x, y }));
		}
		return dist <= 0.025f;
	}
	int tailDx() {
		if (A[len - 2].y - A[len - 1].y == 1) return 3;
		if (A[len - 2].y - A[len - 1].y == -1) return 4;
		if (A[len - 2].x - A[len - 1].x == 1) return 2;
		if (A[len - 2].x - A[len - 1].x == -1) return 1;
	}
};
void smoothDraw(Snake& s, Vertex q[], float o) {
	for (int i = 0; i < 3; i++) {
		q[g[s.dx][i]].x += o * (s.dx == 2);
		q[g[s.dx][i]].x -= o * (s.dx == 1);
		q[g[s.dx][i]].y += o * (s.dx == 3);
		q[g[s.dx][i]].y -= o * (s.dx == 4);
	}
	int f = s.len * 6 - 6;
	for (int i = 0; i < 3; i++) {
		q[f + tg[s.tailDx()][i]].x += o * (s.tailDx() == 2);
		q[f + tg[s.tailDx()][i]].x -= o * (s.tailDx() == 1);
		q[f + tg[s.tailDx()][i]].y += o * (s.tailDx() == 3);
		q[f + tg[s.tailDx()][i]].y -= o * (s.tailDx() == 4);
	}
}

void smoothLonger(Snake& s, Vertex q[], float o) {
	for (int i = 0; i < 3; i++) {
		q[g[s.dx][i]].x += o * (s.dx == 2);
		q[g[s.dx][i]].x -= o * (s.dx == 1);
		q[g[s.dx][i]].y += o * (s.dx == 3);
		q[g[s.dx][i]].y -= o * (s.dx == 4);
	}
}

int getDx(GLFWwindow* w) {
	int d = 0;
	if (!d) {
		if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) d = 3;
		else if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) d = 4;
		else if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) d = 1;
		else if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) d = 2;
	}
	return d;
}
void check(Snake& s, int dt, bool ifLonger) {

	if (!ifLonger) s.move();
	else s.longer();
	if ((abs(dt - s.dx) > 1 || dt + s.dx == 5) && dt) s.turn(dt);
	s.convert();
}
void excheck(Snake& s, int dt, bool ifLonger) {
	if ((abs(dt - s.dx) != 1 || dt + s.dx == 5) && dt) s.turn(dt);
	/*if (!ifLonger) s.move();
	else s.longer();*/

	s.convert();
}
bool ifEat(Snake& s) {
	return s.A[0].x == ifood[0].x && s.A[0].y == ifood[0].y;
}
int randInt() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(1, 39);
	return distrib(gen);
}
void generate() {
	std::mt19937 r(time(NULL));
	there: int x = randInt(), y = randInt(), fl = 1;
	if (mp[{x, y}]) goto there;
	itof(food, 0, x, y, 1.0f, 0);
	ifood[0] = { x, y };
	/*cout << x << " " << y << endl;*/
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
void verProc(int len, Vertex q[], const char* c) {
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * len * 6, q, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// parameter--> (pos value,VertexAttributeSize,DataForm,IfNormalizeData) 1,2,3
	//Stride(interval between consecutive vertex attribute groups) 4
	//Offset of the starting pos of pos data in buffer 5
	if (c == "TRIANGLES") glDrawArrays(GL_TRIANGLES, 0, len * 6);
	if (c == "LINE") glDrawArrays(GL_LINE_STRIP, 0, len * 2);
}

void speed(GLFWwindow* w) {
	bool j_pressed = (glfwGetKey(w, GLFW_KEY_J) == GLFW_PRESS);
	if (j_pressed) ch = 5, o = 0.01f;
	else ch = 15, o = 0.00333f;
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Just because you are too beautiful", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	Shader myShader("ShaderFile/v_Shader.vs", "ShaderFile/F_Shader.fs");
	myShader.use();
	myShader.log();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	/*glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);*/



	//texData = stbi_load("Textures/stone.png", &width, &height, &nrChannels, 0);
	//if (texData == nullptr) {
	//	std::cout << "Failed to load texture" << std::endl;
	//	return -1;
	//}
	//glGenTextures(1, &texture);
	//glBindTexture(GL_TEXTURE_2D, texture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texData);
	//// textureTarget, mipmapLevel, imageForm, width, height, 0, 
	//glGenerateMipmap(GL_TEXTURE_2D);
	////cout << nrChannels << endl;
	////cout << typeid(texData).name();
	//stbi_image_free(texData);
	Snake se;
	se.init();
	generate();
	//ch = 1, o = 0.05f;
	//cout << ch << o;
	int c = 0, dt = 0, fl = 0, close = 0, predt = 0;
	//double lastTime = GetCurrentTime();
	//thread T(speed, window);
	//vector<thread> threads;
	while (!glfwWindowShouldClose(window)) {
		/*double deltaTime = GetCurrentTime() - lastTime;
		if (deltaTime < frameTime) Sleep(frameTime - deltaTime);*/
		processInput(window);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//cout << se.hitSelf();
		if (abs(se.head().x) < 0.974f && abs(se.head().y) < 0.974f && !se.hitSelf()) {
			if (fl) smoothLonger(se, v, o);
			else smoothDraw(se, v, o);
		}

		c++;
		if (!dt) dt = getDx(window);
		//threads.emplace_back(speed, window);
		//debug(d);
		//cout << predt << " " << dt << endl;
		if (close) Sleep(20);
		//cout << ch << endl;
		if (c == ch) {
			if (close && predt == dt) {
				glfwSetWindowShouldClose(window, true);
				cout << "You a little fuck." << endl;
				exit(0);
			}

			if (!close) check(se, dt, fl);// ifLonger
			else excheck(se, dt, fl);
			predt = dt;
			if (!fl) fl = ifEat(se);
			else fl = 0;
			c = dt = 0;
			/*se.ifDie(window);*/
			close = se.ifDie();
			if (fl) generate();
			//else fl = 0;
			//se.longer();

		}
		verProc(4, edge, "LINE");

		verProc(1, food, "TRIANGLES");
		verProc(se.len, v, "TRIANGLES");
		//cout << se.A[0].x << " " << se.A[0].y << " " << close << endl;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteTextures(1, &texture);

	glfwTerminate();

	system("pause");
	return 0;
}
