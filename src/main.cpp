#include <memory>
#include <chrono>
#include <thread>
#include <list>

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Platform/GLContext.h>

#include <GLFW/glfw3.h>

struct Player
{

	struct Position {
		double x, y, z;
	};

	struct Dimensions {
		double width, height, depth;
	};

	struct Orientation {
		double yaw, pitch, roll;
	};

	Position position;
	Dimensions dimensions;
	Orientation orientation;

};

const double BLOCK_SIZE = 10.0;

struct Block
{

	struct Position {
		double x, y, z;
	};

	Position position;

};

struct World
{
	Player player;
	std::list<Block> blocks;
};

GLFWwindow* glfw_window;



using namespace std::chrono_literals;
using namespace Magnum;

void create_window()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfw_window = glfwCreateWindow(1024, 800, "POC-BASIC-PLAYER", nullptr, nullptr);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    glfwMakeContextCurrent(glfw_window);

}

void render(const World& world)
{

    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

//    _camera->draw(_drawables);

    glfwSwapBuffers(glfw_window);
}

int main()
{
	Player you = {
		{ 0.0, 0.0, 0.0 },
		{ 1.0, 1.0, 1.0 },
		{ 1.0, 0.0, 0.0 }
	};
	Block block1 = {
		{ 10.0, 0.0, 0.0 }
	};
	Block block2 = {
		{ 0.0, 10.0, 0.0 }
	};

	World world = {
		you,
		{{ block1, block2 }}
	};

	if(!glfwInit()) std::exit(1);

	create_window();

	// Create Magnum context in an isolated scope
	Magnum::Platform::GLContext ctx{};

	while (true) {
		render(world);
		// 60 fps
	    std::this_thread::sleep_for(16ms);
	}

}
