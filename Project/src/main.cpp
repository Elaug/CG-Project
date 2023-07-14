// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)
// Example:
struct UniformBlock {
	alignas(16) glm::mat4 mvpMat;
};

struct GlobalUniformBlock {
	alignas(16) glm::vec3 DlightDir;
	alignas(16) glm::vec3 DlightColor;
	alignas(16) glm::vec3 AmbLightColor;
	alignas(16) glm::vec3 eyePos;
};

// The vertices data structures
// Example
struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};





// MAIN ! 
class SimpleCube : public BaseProject {
	protected:

	// Current aspect ratio (used by the callback that resized the window
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLMesh, DSLGlobal;

	// Vertex formats
	VertexDescriptor VDMesh;

	// Pipelines [Shader couples]
	Pipeline PMesh;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	// Models
	Model<VertexMesh> MBody, MWheel;
	// Descriptor sets
	DescriptorSet DSBody, DSWheel1, DSWheel2, DSWheel3, DSGlobal;
	// Textures
	Texture TBody, TWheel;
	
	// C++ storage for uniform variables
	UniformBlock uboBody, uboWheel1, uboWheel2, uboWheel3;
	GlobalUniformBlock gubo;

	//Other application parameters

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Slot Machine";
    	windowResizable = GLFW_TRUE; //se voglio una fixed window o resizable
		initialBackgroundColor = {0.0f, 0.005f, 0.01f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 4;
		texturesInPool = 4;
		setsInPool = 4;
		
		Ar = (float)windowWidth / (float)windowHeight;
	}
	
	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		Ar = (float)w / (float)h;
	}
	
	
	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLMesh.init(this, {
					// this array contains the bindings:
					// first  element : the binding number
					// second element : the type of element (buffer or texture)
					//                  using the corresponding Vulkan constant
					// third  element : the pipeline stage where it will be used
					//                  using the corresponding Vulkan constant
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				});
		DSLGlobal.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			});


		// Vertex descriptors
		VDMesh.init(this, {
				  // this array contains the bindings
				  // first  element : the binding number
				  // second element : the stride of this binging
				  // third  element : whether this parameter change per vertex or per instance
				  //                  using the corresponding Vulkan constant
				  {0, sizeof(VertexMesh), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  // this array contains the location
				  // first  element : the binding number
				  // second element : the location number
				  // third  element : the offset of this element in the memory record
				  // fourth element : the data type of the element
				  //                  using the corresponding Vulkan constant
				  // fifth  elmenet : the size in byte of the element
				  // sixth  element : a constant defining the element usage
				  //                   POSITION - a vec3 with the position
				  //                   NORMAL   - a vec3 with the normal vector
				  //                   UV       - a vec2 with a UV coordinate
				  //                   COLOR    - a vec4 with a RGBA color
				  //                   TANGENT  - a vec4 with the tangent vector
				  //                   OTHER    - anything else
				  //
				  // ***************** DOUBLE CHECK ********************
				  //    That the Vertex data structure you use in the "offsetoff" and
				  //	in the "sizeof" in the previous array, refers to the correct one,
				  //	if you have more than one vertex format!
				  // ***************************************************
				  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, pos), // VK_FORMAT_R32G32B32_SFLOAT -> vec3
				         sizeof(glm::vec3), POSITION},
				  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, norm), // VK_FORMAT_R32G32_SFLOAT -> vec2
				         sizeof(glm::vec3), NORMAL},
				  {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMesh, UV), // VK_FORMAT_R32G32_SFLOAT -> vec2
						 sizeof(glm::vec2), UV}
				});

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		PMesh.init(this, &VDMesh, "shaders/ShaderVert.spv", "shaders/ShaderFrag.spv", {&DSLGlobal, &DSLMesh });

		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF
		MBody.init(this,   &VDMesh, "Models/SlotBody.obj", OBJ);
		MWheel.init(this, &VDMesh, "Models/SlotWheel.obj", OBJ);

		/*
		// Creates a mesh with direct enumeration of vertices and indices
		M2.vertices = {{{-3,-1,-3}, {0.0f,0.0f}}, {{-3,-1,3}, {0.0f,1.0f}},
					    {{3,-1,-3}, {1.0f,0.0f}}, {{3,-1,3}, {1.0f,1.0f}}};
		M2.indices = {0, 1, 2,    1, 3, 2};
		M2.initMesh(this, &VD);
		*/


		// Create the textures
		// The second parameter is the file name
		TBody.init(this,   "textures/SlotBody.png");
		TWheel.init(this, "textures/SlotWheel.png");
		// Init local variables
	}
	
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		PMesh.create();

		// Here you define the data set
		DSBody.init(this, &DSLMesh, {
		// the second parameter, is a pointer to the Uniform Set Layout of this set
		// the last parameter is an array, with one element per binding of the set.
		// first  elmenet : the binding number
		// second element : UNIFORM or TEXTURE (an enum) depending on the type
		// third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
		// fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
					{0, UNIFORM, sizeof(UniformBlock), nullptr},
					{1, TEXTURE, 0, &TBody}
				});
		DSWheel1.init(this, &DSLMesh, {
						{0, UNIFORM, sizeof(UniformBlock), nullptr},
						{1, TEXTURE, 0, &TWheel}
			});
		DSWheel2.init(this, &DSLMesh, {
						{0, UNIFORM, sizeof(UniformBlock), nullptr},
						{1, TEXTURE, 0, &TWheel}
			});
		DSWheel3.init(this, &DSLMesh, {
						{0, UNIFORM, sizeof(UniformBlock), nullptr},
						{1, TEXTURE, 0, &TWheel}
			});
		DSGlobal.init(this, &DSLGlobal, {
				{0, UNIFORM, sizeof(GlobalUniformBlock), nullptr}
			});
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PMesh.cleanup();

		// Cleanup datasets
		DSBody.cleanup();
		DSWheel1.cleanup();
		DSWheel2.cleanup();
		DSWheel3.cleanup();
		DSGlobal.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		TBody.cleanup();
		TWheel.cleanup();
		// Cleanup models
		MBody.cleanup();
		MWheel.cleanup();
		//M2.cleanup();
		
		// Cleanup descriptor set layouts
		DSLMesh.cleanup();
		DSLGlobal.cleanup();
		
		// Destroies the pipelines
		PMesh.destroy();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// binds the pipeline
		PMesh.bind(commandBuffer);
		// For a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter
		DSGlobal.bind(commandBuffer, PMesh, 0, currentImage);
		// binds the data set
		DSBody.bind(commandBuffer, PMesh, 1, currentImage);
		
		// For a Dataset object, this command binds the corresponing dataset
		// to the command buffer and pipeline passed in its first and second parameters.
		// The third parameter is the number of the set being bound
		// As described in the Vulkan tutorial, a different dataset is required for each image in the swap chain.
		// This is done automatically in file Starter.hpp, however the command here needs also the index
		// of the current image in the swap chain, passed in its last parameter
					
		// binds the model
		MBody.bind(commandBuffer);
		// For a Model object, this command binds the corresponing index and vertex buffer
		// to the command buffer passed in its parameter
		
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MBody.indices.size()), 1, 0, 0, 0);
		// the second parameter is the number of indexes to be drawn. For a Model object,
		// this can be retrieved with the .indices.size() method.

		MWheel.bind(commandBuffer);
		DSWheel1.bind(commandBuffer, PMesh, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MWheel.indices.size()), 1, 0, 0, 0);
		DSWheel2.bind(commandBuffer, PMesh, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MWheel.indices.size()), 1, 0, 0, 0);
		DSWheel3.bind(commandBuffer, PMesh, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MWheel.indices.size()), 1, 0, 0, 0);

		/*
		M2.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(M2.indices.size()), 1, 0, 0, 0);
		*/

	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		// Integration with the timers and the controllers
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);
		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

		
		// Parameters
		// Camera FOV-y, Near Plane and Far Plane
		const float FOVy = glm::radians(45.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 100.0f;
		
		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;
		glm::vec3 camTarget = glm::vec3(0,0,0);
		glm::vec3 camPos    = camTarget + glm::vec3(6,3,10);
		glm::mat4 View = glm::lookAt(camPos, camTarget, glm::vec3(0,1,0));


		glm::mat4 World = glm::mat4(1);		
		uboBody.mvpMat = Prj * View * World;
		DSBody.map(currentImage, &uboBody, sizeof(uboBody), 0);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block
		World = glm::translate(glm::mat4(1),glm::vec3(-0.15f,0.93f,-0.15f));
		uboWheel1.mvpMat = Prj * View * World;
		DSWheel1.map(currentImage, &uboWheel1, sizeof(uboWheel1), 0);

		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.93f, -0.15f));
		uboWheel2.mvpMat = Prj * View * World;
		DSWheel2.map(currentImage, &uboWheel2, sizeof(uboWheel2), 0);

		World = glm::translate(glm::mat4(1), glm::vec3(0.15f, 0.93f, -0.15f));
		uboWheel3.mvpMat = Prj * View * World;
		DSWheel3.map(currentImage, &uboWheel3, sizeof(uboWheel3), 0);


	}	
};


// This is the main: probably you do not need to touch this!
int main() {
    SimpleCube app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}