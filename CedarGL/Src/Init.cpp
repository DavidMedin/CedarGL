#include "Init.h"
namespace Cedar {


	int graphicsFlag = 1;

	Shader* shaderList[2];


	//class ImplWindow {
	//public:
	//	GLFWwindow* win;
	//	ImplWindow(GLFWwindow* in) : win(in) {};
	//};

	static void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}



	void WindowSizeCallback( int width, int height)
	{
		std::list<Camera*>* camList = GetCameraList();
		/*for (list<Camera*>::iterator i = camList->begin(); i != camList->end(); i++) {
			i._Ptr->_Myval->RecalculateProjection();
		}*/
		for (Camera* i : *camList) {
			i->RecalculateProjection();
		}
	}
	void _WindowSizeCallback(GLFWwindow* win, int width, int height) {
		glfwSetWindowSize(win, width, height);
		WindowSizeCallback(width, height);
	}
	void FramebufferSizeCallback( int width, int height)
	{
		glViewport(0, 0, width, height);
	}
	//void  SetGraphicsFlag(int flag)
	//{
	//	graphicsFlag = graphicsFlag ^ flag;
	//}
	//bool  GetGraphicsFlag(int flag)
	//{
	//	return graphicsFlag & flag;
	//}

	void GraphicsEnable(unsigned int what) {
		switch (what) {
			//z_test
		case 1: {
			glEnable(GL_DEPTH_TEST);
			break;
		}
		default: {
			std::cout << what << "is not a valid enable\n";
		}
		}
	}

	void GraphicsDisable(unsigned int what)
	{
		switch (what) {
			//z_test
		case 1: {
			glDisable(GL_DEPTH_TEST);
			break;
		}
		default: {
			std::cout << what << "is not a valid enable\n";
		}
		}
	}


	//------------------------------
	int init(int width, int height, std::string name) {
		glfwSetErrorCallback(glfw_error_callback);

		if (!glfwInit()) {
			NewError("glfwInit didn't start correctly\n");
			return 0;
		}


		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
		if (!window) {
			NewError("window didn't open (glfw)\n");
			glfwTerminate();
			return 0;
		}
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);


		glewExperimental = GL_TRUE;
		glewInit();
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		//glEnable(GL_BLEND);

		//Imgui stuff
		int width_w, height_w;
		glfwGetFramebufferSize(window, &width_w, &height_w);
		glViewport(0, 0, width_w, height_w);

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init(NULL);




		//-------Input------------//

		glfwSetKeyCallback(glfwGetCurrentContext(), Key);
		glfwSetCursorPosCallback(glfwGetCurrentContext(), Mouse);
		glfwSetMouseButtonCallback(glfwGetCurrentContext(), MouseButton);
		glfwSetWindowSizeCallback(glfwGetCurrentContext(), _WindowSizeCallback);
		//glfwSetFramebufferSizeCallback(glfwGetCurrentContext(), FramebufferSizeCallback);

		shaderList[0] = new Shader("../../../DefaultShaders/LineVs.glsl", "../../../DefaultShaders/LineFs.glsl", nullptr);
		shaderList[1] = new Shader("../../../DefaultShaders/DotVs.glsl", "../../../DefaultShaders/DotFs.glsl", NULL);
		return 1;
	}

	void ClearWindow() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void PollEvents() {
		glfwPollEvents();
	}
	void DrawWindow() {
		glfwSwapBuffers(glfwGetCurrentContext());
	}
	int ShouldCloseWindow() {
		return glfwWindowShouldClose(glfwGetCurrentContext());
	}
	float GetDeltaTime() {
		static float prev_dt = (float)glfwGetTime();
		float dt = (float)((float)glfwGetTime() - prev_dt);
		prev_dt = (float)glfwGetTime();
		return dt;
	}


	void ImGuiNewFrame() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	void ImGuiRender() {
		//ImGui_ImplOpenGL3_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();
		//ImGuiFuncVar();
		//ImGui::Begin("Hello, World!");
		//ImGui::Text("Text!");
		//ImGui::End();

		ImGui::Render();
		//int display_w, display_h;
		//glfwGetFramebufferSize(glfwGetCurrentContext(), &display_w, &display_h);
		//glViewport(0, 0, display_w, display_h);
		//glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	Shader** GetShaders()
	{
		return shaderList;
	}
}