#include <memory>

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
}
