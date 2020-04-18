#include "Source.h"
#define WIDTH 1500
#define HEIGHT 1100




int main(int argv, char* argc[]) {
	init(WIDTH, HEIGHT, "Default");

	glPointSize(10);

	Object* skull = new Object("../Models/Skull/BigJaw.fbx");
	glm::quat* firstRotate = new glm::quat(glm::angleAxis(0.0f, glm::vec3(1, 0, 0)));

	Object* plane = new Object("../Models/Plane/plane.dae",(Node*)skull);
	plane->Translate(vec3(0, -.1f, 0));



	Light* mainLight = new Light(vec3(1,1,1),.25);
	mainLight->translate = TranslateVec(&mainLight->translate, vec3(0, 1, .25));
	vec3* ambientResult = new vec3(mainLight->color * mainLight->intensity);
	vec2* lightRamp1 = new vec2(.33f,0.f);
	vec2* lightRamp2 = new vec2(0.878f,.586f);

	int renderSwitch = 0;

	Shader* meshShad = new Shader("../Shaders/MeshVs.glsl","../Shaders/MeshFs.glsl",NULL);


	Camera* cam = new Camera(vec3(0.0f,0.0f,1.0));
	cam->NewProjection(32, .1f, 100);
	cam->UpdateViewMatrix();

	while (!ShouldCloseWindow()) {
		float dt = GetDeltaTime();
		ClearWindow();
		ImGuiNewFrame();

		float imGuiDragSpeed = 0.0025f;

		ImGui::Begin("Variables");
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("shader")) {
			if (ImGui::TreeNode("LightRamp")) {
				ImGui::SliderFloat2("LightRamp1", (float*)lightRamp1,0,1);
				ImGui::SliderFloat2("LightRamp2", (float*)lightRamp2,0,1);
				ImGui::TreePop();
			}
			ImGui::InputInt("RenderSwitch", &renderSwitch);
			if (ImGui::Button("Reload Shader")) {
				meshShad->Reload();
			}
			ImGui::TreePop();
		}
		if (ImGui::Button("Rotate First Bone")) {
			//skull->mesh->skelly->boneMatrices[0] = glm::rotate(skull->mesh->skelly->boneMatrices[0], radians(5.0f), vec3(1, 0, 0));



			glm::quat* tmp = firstRotate;
			firstRotate = new glm::quat(glm::angleAxis(radians(10.0f), glm::vec3(1, 0, 0)) * *firstRotate);
			delete tmp;
			/*cout << glm::to_string(glm::mat4_cast(glm::normalize(*firstRotate))) << "\n";
			glm::mat4* sacrifice = skull->modelMatrix;
			skull->modelMatrix = new glm::mat4(*skull->modelMatrix*glm::mat4_cast(*firstRotate));
			delete sacrifice;*/
			skull->mesh->skelly->Rotate(0, firstRotate);
		}
		//if (ImGui::Button("Rotate Second Bone")) {
			//skull->mesh->skelly->boneMatrices[1] = translate(glm::rotate(translate(skull->mesh->skelly->boneMatrices[1],skull->mesh->skelly->boneOffsets[1]), radians(10.0f), vec3(1, 0, 0)), skull->mesh->skelly->boneOffsets[1] * vec3(-1, -1, -1));
		//}
		UpdateNodes();

		ImGui::End();

		meshShad->UniformEquals("ambientColor",GL_FLOAT_VEC3, ambientResult,1);
		meshShad->UniformEquals("lightPos",GL_FLOAT_VEC3, &mainLight->translate,1);
		meshShad->UniformEquals("lightColor",GL_FLOAT_VEC3, &mainLight->color,1);
		meshShad->UniformEquals("LightRamp1", GL_FLOAT_VEC2, lightRamp1,1);
		meshShad->UniformEquals("LightRamp2", GL_FLOAT_VEC2, lightRamp2,1);
		meshShad->UniformEquals("renderSwitch", GL_INT, &renderSwitch,1);
		//ImGui::ShowDemoWindow();

		if (GetKey(keys::A_KEY)) {
			cam->Translate(vec3(cos(-radians(cam->GetY()) + radians(90.0f)) * dt, 0.0f, -sin(-radians(cam->GetY()) + radians(90.0f)) * dt));
		}
		if (GetKey(keys::D_KEY)) {
			cam->Translate(-vec3(cos(-radians(cam->GetY()) + radians(90.0f)) * dt, 0.0f, -sin(-radians(cam->GetY()) + radians(90.0f)) * dt));
		}
		if (GetKey(keys::W_KEY)) {
			cam->Translate(vec3(cos(-radians(cam->GetY())) * dt, 0.0f, -sin(-radians(cam->GetY())) * dt));
		}
		if (GetKey(keys::S_KEY)) {
			cam->Translate(-vec3(cos(-radians(cam->GetY())) * dt, 0.0f, -sin(-radians(cam->GetY())) * dt));
		}
		if (GetKey(keys::CTRL_KEY)) {
			cam->Translate(vec3(0, -1, 0)*dt);
		}

		if (GetKey(keys::SPACE_KEY)) {
			cam->Translate(vec3(0, 1, 0) * dt);
		}
		float x, y;
		GetMousePos(&x, &y);
		if (GetKey(keys::Q_KEY)) {
			SetDisabledMouse(true);
			cam->Rotate(10*-dt * x, vec3(0, 1, 0));
			cam->Rotate(10*-dt * y, cam->GetRightAxis());
		}
		else {
			SetDisabledMouse(false);
		}

		skull->Draw(meshShad, cam);
		plane->Draw(meshShad, cam);
		//bone->Draw(cam);

		cam->UpdateViewMatrix();
		
		
		//Error Handleing
		string error = PollError();
		while (error != "Empty") {
			cout << error << "\n";
			error = PollError();
		}
	


		ImGuiRender();
		DrawWindow();
		PollEvents();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	return 0;
}