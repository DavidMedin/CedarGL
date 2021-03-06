 
#include "Shader.h"
#include <glm.hpp>
#include <ext.hpp>
//glew
#include <GL/glew.h>

#include <GLFW/glfw3.h>

//SPRIV-VM
#include <spvm/context.h>
#include <spvm/state.h>
#include <spvm/ext/GLSL450.h>

//shaderc
#include <shaderc/shaderc.hpp>

#define GETSPV(x) (*(*x).shad)
namespace Cedar {
	//used for debugging, one per shader, not program
	struct SpvShader {
		spvm_context_t spvContext;
		unsigned int spvSLength;
		spvm_source spvSource;
		//--
		spvm_program_t spvProgram;
		spvm_state_t spvState;
		//shaderType is 
		SpvShader();
		SpvShader(std::string content, unsigned int shaderType);
	};
	class Shader::ImplSpvShader {
	public:
		SpvShader* shad;
		ImplSpvShader() {};
		ImplSpvShader(SpvShader* in) :shad(in) {};
	};

	Shader::Shader() {
		shader_Program = 0;
	}

	std::list<Shader::ImplSpvShader*>* Shader::SPIRVVMFindUniform(std::string name)
	{
		std::list<Shader::ImplSpvShader*>* shads = new std::list<Shader::ImplSpvShader*>;
		bool found = false;
		spvm_result_t interfaceBlock;
		for (int i = 0; i < 2; i++) {
			interfaceBlock = spvm_state_get_result(GETSPV(spvShaders[i]).spvState, spvm_string(name.c_str()));
			if (interfaceBlock != NULL) {
				shads->push_back(spvShaders[i]);
				found = true;
			}
		}
		if (found == false) {
			printf("That interface block isn't in any of the initialized shaders!\n");
			delete(shads);
			return nullptr;
		}

		return shads;
	}

	std::string Shader::ReadShader(const char* path) {
		std::string tmp;
		std::ifstream shaderFile;
		shaderFile.open(path, std::ios::in);
		std::string input;
		while (!shaderFile.eof()) {
			getline(shaderFile, input);
			if (input.find("uniform") != std::string::npos) {

			}
			tmp.append(input);
			input.clear();
			tmp.append("\n");
		}
		//const char* C_String = (const char*)malloc(tmp.length());
		//if (C_String != nullptr) {
		//	memcpy((void*)C_String, (void*)tmp.c_str(), tmp.length());
		//	return C_String;
		//}
		return tmp;
	}
	unsigned int Shader::CompileShader(unsigned int type, const char* source) {
		GLuint id = glCreateShader(type);
		const char* src = source;
		GLCall(glShaderSource(id, 1, &src, nullptr));
		GLCall(glCompileShader(id));
		int result;
		GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
		if (!result) {
			int length;
			GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
			char* error = (char*)malloc(sizeof(char) * length);
			GLCall(glGetShaderInfoLog(id, length, &length, error));
			const char* message = nullptr;
			switch (type) {
			case GL_VERTEX_SHADER: {
				message = "vertex";
				break;
			}
			case GL_FRAGMENT_SHADER: {
				message = "fragment";
				break;
			}
			case GL_GEOMETRY_SHADER: {
				message = "geometry";
				break;
			}
			default: {
				message = "unknown shader";
			}
			}
			printf("failed to compile %s shader: ", message);
			std::cout << error << "\n";

			free(error);
			GLCall(glDeleteShader(id));
		}

		return id;
	}

	unsigned int Shader::CreateShaderProgram(const char* vertexShader, const char* fragmentShader, const char* geometryShader) {
		GLuint shader_program = glCreateProgram();
		GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertexShader);
		GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
		GLuint geometry_shader;
		if (geometryShader != nullptr) {
			geometry_shader = CompileShader(GL_GEOMETRY_SHADER, geometryShader);
		}
		GLCall(glAttachShader(shader_program, vertex_shader));
		GLCall(glAttachShader(shader_program, fragment_shader));
		if (geometryShader != nullptr) {
			GLCall(glAttachShader(shader_program, geometry_shader));
		}


		GLCall(glLinkProgram(shader_program));
		GLint rez = NULL;
		GLCall(glGetProgramiv(shader_program, GL_LINK_STATUS, &rez));
		if (rez == NULL || rez != GL_TRUE) {
			GLsizei logSize = 255;
			GLchar* log = (GLchar*)malloc(sizeof(GLchar) * logSize);
			GLsizei actualSize = 0;
			GLCall(glGetProgramInfoLog(shader_program, 255, &actualSize, log));
			std::cout << actualSize << "\n";
			std::cout << log << "\n";
			free(log);
		}

		GLCall(glValidateProgram(shader_program));
		rez = NULL;
		GLCall(glGetProgramiv(shader_program, GL_VALIDATE_STATUS, &rez));
		if (rez == NULL || rez != GL_TRUE) {
			GLsizei logSize = 255;
			GLchar* log = (GLchar*)malloc(sizeof(GLchar) * logSize);
			GLsizei actualSize = 0;
			GLCall(glGetProgramInfoLog(shader_program, 255, &actualSize, log));
			std::cout << actualSize << "\n";
			std::cout << log << "\n";
			free(log);
		}
		GLCall(glDeleteShader(vertex_shader));
		GLCall(glDeleteShader(fragment_shader));
		if (geometryShader != nullptr) {
			GLCall(glDeleteShader(geometry_shader));
		}
		return shader_program;
	}

	void Shader::_UniformEquals(int location, void* value, unsigned int type, unsigned int count, const char* uniformName)
	{
		UseShader();

		switch (type) {
		case GL_FLOAT_MAT4:
			GLCall(glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(*(glm::mat4*)value)));
			break;
		case GL_FLOAT_VEC3:
			GLCall(glUniform3fv(location, count, &(*(glm::vec3*)value)[0]));
			break;
		case GL_FLOAT_VEC2:
			GLCall(glUniform2fv(location, count, &(*(glm::vec3*)value)[0]));
			break;
		case GL_FLOAT:
			GLCall(glUniform1fv(location, count, ((float*)value)));
			break;
		case GL_INT:
			GLCall(glUniform1iv(location, count, ((int*)value)));
			break;
		default:
			printf("not a valid type\n"); //Type should be GL_FLOAT_MAT4,GL_FLOAT_VEC3 and the like
			return;
		}
		if (spvInitialized) {
			std::list<Shader::ImplSpvShader*>* shads = SPIRVVMFindUniform(uniformName);
			if (shads == nullptr) {
				printf("uniform name : %s isn't in that shader!\n", uniformName);
				return;
			}
			for (auto i : *shads) {
				spvm_result_t uniform = spvm_state_get_result((*(*i).shad).spvState, spvm_string(uniformName));
				if (type != GL_INT) {
					spvm_member_set_value_f(uniform->members, uniform->member_count, (float*)value);
				}
				else {
					spvm_member_set_value_i(uniform->members, uniform->member_count, (int*)value);
				}
			}
			delete shads;
		}
	}


	Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geomPath) {
		vertexContent = ReadShader(vertexPath);
		fragmentContent = ReadShader(fragmentPath);
		if (geomPath != nullptr) {
			vertexContent = ReadShader(geomPath);
			shader_Program = CreateShaderProgram(vertexContent.c_str(), fragmentContent.c_str(), geometryContent.c_str());
		}
		else
		{
			shader_Program = CreateShaderProgram(vertexContent.c_str(), fragmentContent.c_str(), NULL);
		}
		GLCall(glUseProgram(shader_Program));

	}

	Shader::Shader(const char* shaderPath) {
		//read and find identifiers
		// -> @vertex,@fragment,@geometry
		//add to flag var
		char flags = 0;

		//parse the file for identifiers
		std::string tmp;
		std::ifstream shaderFile;
		shaderFile.open(shaderPath, std::ios::in);
		std::string input;

		std::string vertexText;
		std::string fragmentText;
		std::string geometryText;

		while (!shaderFile.eof()) {
			getline(shaderFile, input);
			if (input.find("uniform") != std::string::npos) {

			}

			if (input.find("@vertex") != std::string::npos) {//should be after the vertex shader
				vertexText = tmp;
				tmp.clear();
				//might clear VertexText
				continue;
			}
			else if (input.find("@fragment") != std::string::npos) {
				fragmentText = tmp;
				tmp.clear();
				continue;
			}
			else if (input.find("@geometry") != std::string::npos) {
				geometryText = tmp;
				tmp.clear();
				continue;
			}

			tmp.append(input);
			input.clear();
			tmp.append("\n");
		}
		//compiling shaders
		shader_Program = glCreateProgram();
		GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertexText.c_str());
		GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragmentText.c_str());
		GLuint geometry_shader = NULL;
		//combine shaders
		GLCall(glAttachShader(shader_Program, vertex_shader));
		GLCall(glAttachShader(shader_Program, fragment_shader));
		if (!geometryText.empty()) {
			geometry_shader = CompileShader(GL_GEOMETRY_SHADER, geometryText.c_str());
			GLCall(glAttachShader(shader_Program, geometry_shader));
		}
		GLCall(glLinkProgram(shader_Program));
		GLCall(glValidateProgram(shader_Program));
		GLCall(glDeleteShader(vertex_shader));
		GLCall(glDeleteShader(fragment_shader));
		if (geometry_shader != NULL) {
			GLCall(glDeleteShader(geometry_shader));
		}

		GLCall(glUseProgram(shader_Program));
	}

	Shader::~Shader() {
		GLCall(glDeleteProgram(shader_Program));
	}

	void Shader::UseShader()
	{
		GLCall(glUseProgram(shader_Program));
	}

	unsigned int Shader::GetProgram()
	{
		return shader_Program;
	}

	void Shader::UniformEquals(const char* uniform_Name, unsigned int type, void* value, unsigned int count)
		//Type should be GL_FLOAT_MAT4,GL_FLOAT_VEC3 and the like
	{
		UseShader();
		int uni_Pos = glGetUniformLocation(shader_Program, uniform_Name);
		if (uni_Pos == -1) {
			printf("%s is not a variable in your shader!\n", uniform_Name);
			int rez = NULL;
			GLCall(glGetProgramiv(shader_Program, GL_ACTIVE_UNIFORMS, &rez));
			printf("	There are %d active uniforms in shader!\n", rez);
			GLsizei length;// use size of the buffer
			GLint size; //sizeof the variable
			GLenum type;
			GLchar buffer[64]; //= (char*)malloc(sizeof(char)*64)
			for (GLuint i = 0; i < (unsigned int)rez; i++) {
				GLCall(glGetActiveUniform(shader_Program, i, sizeof(buffer), &length, &size, &type, buffer));
				printf("	Uniform: #%d | Type: %u | Name : %s\n", i, type, buffer);
			}
			return;
		}
		//basicly the same function but whatever
		_UniformEquals(uni_Pos, value, type, count, uniform_Name);

	}
	// value must be an array of values, not an array of pointers
	void Shader::ArrayUniformEquals(const char* uniformName, unsigned int type, void* value, unsigned int count)
	{
		UseShader();
		//int* locs = (int*)malloc(sizeof(int) * count);
		char name[64];
		for (unsigned int i = 0; i < count; i++) {
			sprintf_s(name, "%s[%i]", uniformName, i);
			int loc = glGetUniformLocation(shader_Program, name);
			if (loc == -1) {
				std::cout << name << " is not a variable in your shader!\n";
			}
			else {
				_UniformEquals(loc, value, type, 1, uniformName);
			}
		}
	}

	void Shader::BindInterfaceBlock(std::string name, unsigned int interfaceType, unsigned int bindingPoint)
	{
		//bind shader to point
		UseShader();
		unsigned int index = glGetProgramResourceIndex(shader_Program, GL_SHADER_STORAGE_BLOCK, name.c_str());
		if (index == GL_INVALID_INDEX) {
			printf("%s is not a valid shader storage block in your shader!\n", name.c_str());
		}
		GLCall(glShaderStorageBlockBinding(shader_Program, index, bindingPoint));
		GLCall(glUseProgram(0));
	}




	bool Shader::GetSPIRVVMInitialized()
	{
		return spvInitialized;
	}

	void Shader::StartSPIRVVMDebug()
	{
		printf("Debugging Shader!==============================\n");
		bool exit = false;
		while (exit == false) {
			char inBuffer[40];
			//get data
			fgets(inBuffer, 40, stdin);
			std::string inString = std::string(inBuffer);
			//find first space, all chars before are command
			size_t commandEnd = inString.find(" ", 0);
			//look for commands
			size_t cmdGetUniform = inString.find("getuniform", 0, commandEnd);
			if (cmdGetUniform != std::string::npos) {

			}
		}
	}

	void Shader::InitializeSPIRVVMDebug() {
		spvShaders[0] = new Shader::ImplSpvShader(new SpvShader(vertexContent, GL_VERTEX_SHADER));
		spvShaders[1] = new Shader::ImplSpvShader(new SpvShader(fragmentContent, GL_FRAGMENT_SHADER));
		spvInitialized = true;
	}

	void Shader::SPIRVVMInterfaceWrite(std::string blockName, unsigned int type, unsigned int localIndex, void* data, unsigned int primitiveType, unsigned int sizeofData) {
		UseShader();
		//getting name of member
		char* nameBuffer = (char*)malloc(30);
		GLsizei actualSize;
		GLCall(glGetProgramResourceName(shader_Program, type, localIndex, 30, &actualSize, nameBuffer));

		//hand data over to SPIRV-VM
		//find the correct shaders to use
		std::list<Shader::ImplSpvShader*>* shads = SPIRVVMFindUniform(blockName);
		for (auto i : *shads) {
			spvm_result_t interfaceBlock = spvm_state_get_result(GETSPV(i).spvState, spvm_string(blockName.c_str()));
			spvm_member_t member = spvm_state_get_object_member(GETSPV(i).spvState, interfaceBlock, spvm_string(nameBuffer));

			if (primitiveType == GL_FLOAT)
				spvm_member_set_value_f(member->members, member->member_count, (float*)data);
			else if (primitiveType == GL_INT)
				spvm_member_set_value_i(member->members, member->member_count, (int*)data);
			else printf("invalid primitive type given to SPIRVVMInterfaceWrite!\n");
		}
		delete shads;
		free(nameBuffer);
	}

	void Shader::Reload() {
		//might need to remake the SpvShaders!
		unsigned int tmpShader = CreateShaderProgram(vertexContent.c_str(), fragmentContent.c_str(), NULL);
		shader_Program = tmpShader;
	}

	SpvShader::SpvShader()
	{
		spvContext = NULL;
		spvSLength = NULL;
		spvSource = NULL;
		spvProgram = NULL;
		spvState = NULL;
	}

	SpvShader::SpvShader(std::string content, unsigned int shaderType)
	{

		//set options
		shaderc::CompileOptions ops = shaderc::CompileOptions();
		ops.SetTargetEnvironment(shaderc_target_env_opengl, 4.5);

		//get spirv code from glsl code
		shaderc::Compiler comp = shaderc::Compiler();
		shaderc::SpvCompilationResult vertexResult;
		if (shaderType == GL_VERTEX_SHADER)
			vertexResult = comp.CompileGlslToSpv(content.c_str(), content.size(), shaderc_vertex_shader, "vertex Shader", ops);
		else if (shaderType == GL_FRAGMENT_SHADER)
			vertexResult = comp.CompileGlslToSpv(content.c_str(), content.size(), shaderc_fragment_shader, "fragment Shader", ops);
		else printf("spvShader() : No Shader of that type supported! (only vertex and fragment for now!)\n");


		//search errors
		std::cout << "Num of Errors: " << vertexResult.GetNumErrors() << "\n";
		for (int e = 0; e < vertexResult.GetNumErrors(); e++)
			std::cout << "    [" << e << "] " << vertexResult.GetErrorMessage() << "\n";
		std::cout << "Num of Warnings: " << vertexResult.GetNumWarnings() << "\n";

		//write
		spvSLength = vertexResult.end() - vertexResult.begin();
		auto iter = vertexResult.begin();
		spvm_source spvSource = (spvm_source)malloc(spvSLength * sizeof(spvm_word));
		if (spvSource == nullptr) {
			printf("Couldn't allocate memory for spvShader::spvSource!\n");
			return;
		}
		for (int i = 0; iter != vertexResult.end(); i++) {
			spvm_word u = *iter;
			spvSource[i] = u;
			iter++;
		}

		//initialize
		spvContext = spvm_context_initialize();
		spvProgram = spvm_program_create(spvContext, spvSource, spvSLength);
		spvState = spvm_state_create(spvProgram);
		spvm_ext_opcode_func* glsl_ext_data = spvm_build_glsl450_ext();
		spvm_result_t glsl_std_450 = spvm_state_get_result(spvState, spvm_string("GLSL.std.450"));
		if (glsl_std_450)
			glsl_std_450->extension = glsl_ext_data;
	}
}