#pragma once

// TODO Remove these single line files once headers are seperated into cpp files
struct CDesiredMovement
{
	enum Direction { UP, DOWN, LEFT, RIGHT };
	Direction direction;
};

class TPlayer;