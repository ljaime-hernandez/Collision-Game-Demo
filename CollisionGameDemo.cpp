
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

// extension used to change the panorama, zoom in and out of the screen, draw the objects and tiles lines, change its colors, and so on
#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"


class CollisionDemo : public olc::PixelGameEngine
{
public:
	CollisionDemo()
	{
		sAppName = "Collision Demo";
	}

private:
	olc::TileTransformedView tv;

	struct WorldObject
	{
		olc::vf2d position;
		olc::vf2d velocity;
		// the radius of our world object (a circle) will be half the size of a tile, so it will be 1.0f in width and height when drawing it full
		float radius = 0.5f;
	};

	WorldObject object;

	// world represented as a string with ASCII characters, allowing us to make big changes in very short time if needed
	std::string worldMap =
		"################################"
		"#..............................#"
		"#...#.....#...#.#.#####........#"
		"#...#.....#...#.#.#............#"
		"#...#.....#...#.#.#####........#"
		"#...#.....#...#.#.....#........#"
		"#...#####.#####.#.#####........#"
		"#..............................#"
		"#..#...#.#.####.#..#.#####.#...#"
		"#..##.##.#.#....#..#.#.....#...#"
		"#..#.#.#.#.####.#..#.#####.#...#"
		"#..#...#.#.#..#.#..#.#.....#...#"
		"#..#...#.#.####.####.#####.###.#"
		"#..............................#"
		"#..............................#"
		"#..####.####.#..#..#.###..###..#"
		"#..#....#..#.#..#..#.#..#.#....#"
		"#..#....#..#.#..#..#.#..#.##...#"
		"#..####.####.##.##.#.###. ###..#"
		"#..............................#"
		"#..###..####..#...#.####.......#"
		"#..#..#.#.....##.##.#..#.......#"
		"#..#..#.###...#.#.#.#..#.......#"
		"#..###..####..#...#.####.......#"
		"#..............................#"
		"#............###...............#"
		"#..........#.#..#..............#"
		"#............#...#.............#"
		"#..........#.#..#..............#"
		"#............###...............#"
		"#..............................#"
		"################################";

	// this 2-dimentions vector is capable of storing integers, this variable is declaring the size of our world, in this case it will be 32 x 32 tiles
	olc::vi2d worldSize = { 32, 32 };

	bool followObject = false;

public:
	// function called on the creation of the class in the main function
	bool OnUserCreate() override
	{
		// after the game instance is called in the main function, this function will create the parameters used for the display to be "defined", it will use
		// the parameters used on the construct to define the width and height of the screen, along with the size of the "tiles", which in this case will be 
		// 32 x 32 screen pixels
		tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, { 32, 32 });
		// this default position will locate our moving object (the circle in this example) in the third "tile" column of the third row, the position can 
		// change at any time during the game execution
		object.position = { 3.0f, 3.0f };
		return true;
	}

	// function called every frame of the demonstration
	bool OnUserUpdate(float elapsedTime) override
	{
		// we are setting the velocity to 0 in both coordinates for it to stop by default in every iteration, we are also assigning movement to different 
		// keys when held for the object to move when pressed
		object.velocity = { 0.0f, 0.0f };
		if (GetKey(olc::Key::W).bHeld) object.velocity += { 0.0f, -1.0f };
		if (GetKey(olc::Key::S).bHeld) object.velocity += { 0.0f, +1.0f };
		if (GetKey(olc::Key::A).bHeld) object.velocity += { -1.0f, 0.0f };
		if (GetKey(olc::Key::D).bHeld) object.velocity += { +1.0f, 0.0f };

		// function used to normalize the speed of the object, the velocity will change if the SHIFT key is held, if it is not then it will move at 5 
		// tiles per second, if it is pressed then it will be 10 tiles per second
		if (object.velocity.mag2() > 0)
			object.velocity = object.velocity.norm() * (GetKey(olc::Key::SHIFT).bHeld ? 10.0f : 5.0f);

		// condition used to change the camera position, for us to be able to follow the object and its movements, changed below with the pan and zoom conditions
		if (GetKey(olc::Key::SPACE).bReleased) followObject = !followObject;


		// this vector is used to update the position vector of the object, but we will use this one as an intermediate to calculate the posible position 
		// of it, we will base it using the position plus the velocity, modulated by the elapsed time
		olc::vf2d potentialPosition = object.position + object.velocity * elapsedTime;

		// we use the following variables to recognize the region of the world on which the object might or might not go into contact with a tile by using 
		// the current velocity as a point of reference, then we use the potential position previously declared and finally we expand the area in a squared 
		// position with both the topLeft and BottomRight vectors using the targetCell as our reference
		olc::vi2d currentCell = object.position.floor();
		olc::vi2d targetCell = potentialPosition;
		olc::vi2d areaTopLeftTile = (currentCell.min(targetCell) - olc::vi2d(1, 1)).max({ 0,0 });
		olc::vi2d areaBottomRightTile = (currentCell.max(targetCell) + olc::vi2d(1, 1)).min(worldSize);

		olc::vf2d vRayToNearest;

		// the iteration is working similarly to the iteration we use to draw the tiles in the world, this one is created to check for posible collisions 
		// with the tiles
		olc::vi2d cell;
		for (cell.y = areaTopLeftTile.y; cell.y <= areaBottomRightTile.y; cell.y++)
		{
			for (cell.x = areaTopLeftTile.x; cell.x <= areaBottomRightTile.x; cell.x++)
			{
				if (worldMap[cell.y * worldSize.x + cell.x] == '#')
				{
					// another clamping example, this one is made to calculate the position of the object towards the nearest tile in the screen from x and 
					// y vector positions
					olc::vf2d vNearestPoint;
					vNearestPoint.x = std::max(float(cell.x), std::min(potentialPosition.x, float(cell.x + 1)));
					vNearestPoint.y = std::max(float(cell.y), std::min(potentialPosition.y, float(cell.y + 1)));

					// the nearest point vector then is calculated using the previous nearest point vector minus the potential position to measure if theres
					// any sort of overlaping within the objects positions
					olc::vf2d vRayToNearest = vNearestPoint - potentialPosition;
					float fOverlap = object.radius - vRayToNearest.mag();

					// in some calculations the overlap may result in data which cannot be used as a number, so this condition will just check for it and 
					// fix the number
					if (std::isnan(fOverlap)) fOverlap = 0;

					// If overlap is positive, then a collision has occurred, so we displace backwards by the overlap amount. The potential position is 
					// then tested against other tiles in the area therefore resolving the collision
					if (fOverlap > 0)
					{
						potentialPosition = potentialPosition - vRayToNearest.norm() * fOverlap;
					}
				}
			}
		}

		// Set the objects new position to the allowed potential position based on the previous iteration fixing the objects position in case of a collision
		object.position = potentialPosition;


		// Clear World
		Clear(olc::DARK_RED);

		if (followObject)
		{
			// if SPACE is pressed, then we will use the SetWorldOffset function to follow the object and its movement with the camera by using the objects 
			// position and dividing the screens width and height into two, for the object to be located in the middle of it
			tv.SetWorldOffset(object.position - tv.ScaleToWorld(olc::vf2d(ScreenWidth() / 2.0f, ScreenHeight() / 2.0f)));
			DrawString({ 10,10 }, "Following Object");
		}

		// these functions can be using any time during the game execution to change the view, the paremeters for the GetMouse() function are 0 for right 
		// click, 2 for the mouse wheel click and 1 for the right click, during the click, if held, we will be able to drag the display to any direction 
		// we want to see the rest of the game world, if released the dragging will stop
		if (GetMouse(0).bPressed) tv.StartPan(GetMousePos());
		if (GetMouse(0).bHeld) tv.UpdatePan(GetMousePos());
		if (GetMouse(0).bReleased) tv.EndPan(GetMousePos());
		// the ZoomAtScreenPos will allow us to zoom in the game world view with the Q key and zoom out with the E key
		if (GetKey(olc::Key::Q).bPressed) tv.ZoomAtScreenPos(1.1f, GetMousePos());
		if (GetKey(olc::Key::E).bPressed) tv.ZoomAtScreenPos(0.9f, GetMousePos());
		
		// the topLeftTile and the botomRightTile will have the coordinates to allow us to see the tiles in the console depending on the objects position, 
		// the pan and the zoom position on the screen with the help of the transformed view library, the min and max functions on this vectors are another
		// example of clamping
		olc::vi2d topLeftTile = tv.GetTopLeftTile().max({ 0,0 });
		olc::vi2d bottomRightTile = tv.GetBottomRightTile().min(worldSize);
		olc::vi2d tile;

		// the following iteration will check on the world map and, if compared to the transformed view location along with all the characteristics described
		// before, it will draw the tiles with rect and diagonal lines (in black for this example)
		for (tile.y = topLeftTile.y; tile.y < bottomRightTile.y; tile.y++)
			for (tile.x = topLeftTile.x; tile.x < bottomRightTile.x; tile.x++)
			{
				if (worldMap[tile.y * worldSize.x + tile.x] == '#')
				{
					tv.DrawRect(tile, { 1.0f, 1.0f }, olc::BLACK);
				}
			}

		// the FillRectDecal function is used in this case to draw or color the region on the map being checked by our previous logic, the pixel portion of it 
		// is using red for the coloring and the 32 represents the amount of transparency
		tv.FillRectDecal(areaTopLeftTile, areaBottomRightTile - areaTopLeftTile + olc::vi2d(1, 1), olc::Pixel(255, 0, 0, 32));

		// the DrawCircle function will use the object location in the game to draw the shape based on the radius designated on the object declaration. in 
		// this case we assigned the circle a radius of 0.5f, this is because the game will consider 1.0f sized object to be the size of a "tile", 1.0f wide 
		// with 1.0f in height, finally using its third parameter to choose the circles color
		tv.DrawCircle(object.position, object.radius, olc::BLACK);

		// this function will draw a line in the middle of the circle if it detects movement in it, it will also help us to watch the direction on which 
		// the circle is moving, the example is in yellow 
		if (object.velocity.mag2() > 0)
		{
			tv.DrawLine(object.position, object.position + object.velocity.norm() * object.radius, olc::YELLOW);
		}

		return true;
	}
};

int main()
{
	CollisionDemo demo;
	
	// the instance of the game is constructed with a 640/480 pixels width and height, each pixel in game ill have a size of 2 x 2 screen pixel
	if (demo.Construct(640, 480, 2, 2))
		demo.Start();
	return 0;
}

/*
	License (OLC-3)
	~~~~~~~~~~~~~~~
	Copyright 2018 - 2021 OneLoneCoder.com
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:
	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.
	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	David Barr, aka javidx9, ©OneLoneCoder 2019, 2020, 2021
*/