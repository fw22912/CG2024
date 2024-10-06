#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/detail/type_vec.hpp>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <CanvasTriangle.h>
#include <TextureMap.h>
#define WIDTH 320
#define HEIGHT 240



std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues)
{
	std::vector<float> output{};
	float gap = (to - from)/(numberOfValues-1);
	output.reserve(numberOfValues);

	for(int i = 0; i < numberOfValues; i++)
	{
		output.push_back(from + i * gap);
	}

	return output;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues){
	std::vector<glm::vec3> result;
	float x_gap = (to[0] - from[0])/(numberOfValues-1);
	float y_gap = (to[1] - from[1])/(numberOfValues-1);
	float z_gap = (to[2] - from[2])/(numberOfValues-1);

	result.reserve(numberOfValues);
	for(int i = 0; i<numberOfValues; i++)
	{
		glm::vec3 row = {from[0] + x_gap * i, from[1] + y_gap * i, from[2] + z_gap*i};
		result.push_back(row);
	}
	return result;
}



void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window)
{
	auto range_x = abs(to.x - from.x);
	auto range_y = abs(to.y - from.y);
	int steps = std::max(range_x, range_y);
	// std::cout << "before from x, y: " << from.x << "," << from.y << std::endl;
	// std::cout << "before to x, y: " << to.x << "," << to.y << std::endl;
	std::vector<float> y_vals = interpolateSingleFloats(from.y, to.y, steps);
	std::vector<float> x_vals = interpolateSingleFloats(from.x, to.x, steps);
	// std::cout << "after from x, y: " << from.x << "," << from.y << std::endl;
	// std::cout << "after to x, y: " << to.x << "," << to.y << std::endl;

	for (size_t i = 0; i < steps; i++){
		uint32_t pixelColour = (255 << 24) + (uint32_t(colour.red) << 16) + (uint32_t(colour.green) << 8) + uint32_t(colour.blue);
		window.setPixelColour(x_vals[i], y_vals[i], pixelColour);
	}
}


//helper functions

CanvasTriangle reorderTriangle(CanvasTriangle triangle)
{
	if (triangle[0].y > triangle[1].y) std::swap(triangle[0], triangle[1]);
	if (triangle[1].y > triangle[2].y) std::swap(triangle[1], triangle[2]);
	if (triangle[0].y > triangle[1].y) std::swap(triangle[0], triangle[1]);

	return triangle;
}

std::pair<int, int> fill_top_triangle(int y, CanvasPoint top, CanvasPoint mid, CanvasPoint bot) {
	float fst_gap, scd_gap;
	int lft, rgt;

	fst_gap = (y - top.y) / float(mid.y - top.y);
	scd_gap = (y - top.y) / float(bot.y - top.y);
	lft = top.x + fst_gap * (mid.x - top.x);
	rgt = top.x + scd_gap * (bot.x - top.x);
    return std::make_pair(lft, rgt);
}


std::pair<int, int> fill_lower_triangle(int y, CanvasPoint top, CanvasPoint mid, CanvasPoint bot){
	float fst_gap, scd_gap;
	int lft, rgt;

	fst_gap = (y - mid.y) / float(bot.y - mid.y);
	scd_gap = (y - top.y) / float(bot.y - top.y);
	lft = mid.x + fst_gap * (bot.x - mid.x);
	rgt = top.x + scd_gap * (bot.x - top.x);

	return std::make_pair(lft, rgt);
}


//Drawing Triangles
void draw_stroked_triangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window){
	drawLine(triangle.v0(), triangle.v1(), colour, window);
	drawLine(triangle.v1(), triangle.v2(), colour, window);
	drawLine(triangle.v2(), triangle.v0(), colour, window);
}


void draw_filled_triangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window){
	triangle = reorderTriangle(triangle);
	std::pair<int, int> coordinates;

	for (int y = triangle[0].y; y <= triangle[2].y; y++) {
		if (y < triangle[1].y) {
			coordinates = fill_top_triangle(y, triangle.v0(), triangle.v1(), triangle.v2());
		} else {
			coordinates = fill_lower_triangle(y, triangle.v0(), triangle.v1(), triangle.v2());
		}

		for (int x = std::min(coordinates.first,coordinates.second); x <= std::max(coordinates.first, coordinates.second); x++){
			uint32_t fillcolour = (255 << 24) + (uint32_t(colour.red) << 16) + (uint32_t(colour.green) << 8) + uint32_t(colour.blue);
			window.setPixelColour(x, y, fillcolour);
		}
	}
}


void drawTextureTriangle(TexturePoint point, DrawingWindow &window){

	TextureMap texture = TextureMap("/Users/hyoyeon/Desktop/UNI/Year 3/Computer Graphics/CG2024/Weekly Workbooks/01 Introduction and Orientation/extras/RedNoise/src/texture.ppm");

}

void draw(DrawingWindow &window) {
	window.clearPixels();
	glm::vec3 topLeft(255, 0, 0);        // red
	glm::vec3 topRight(0, 0, 255);       // blue
	glm::vec3 bottomRight(0, 255, 0);    // green
	glm::vec3 bottomLeft(255, 255, 0);   // yellow

	for (size_t y = 0; y < window.height; y++) {
		std::vector<glm::vec3> left = interpolateThreeElementValues(topLeft, bottomLeft, HEIGHT);
		std::vector<glm::vec3> right = interpolateThreeElementValues(topRight, bottomRight, HEIGHT);
		for (size_t x = 0; x < window.width; x++) {
			std::vector<glm::vec3> row = interpolateThreeElementValues(left[y], right[y], WIDTH);
			glm::vec3 result = row[x];
			uint32_t colour = (255 << 24) + (uint32_t(result[0]) << 16) + (uint32_t(result[1]) << 8) + uint32_t(result[2]);
			window.setPixelColour(x, y, colour);
		}
	}
}

CanvasTriangle randomTriangle(int width, int height){
	CanvasPoint v0(rand() % width, rand() % height);
	CanvasPoint v1(rand() % width, rand() % height);
	CanvasPoint v2(rand() % width, rand() % height);

	return CanvasTriangle(v0, v1, v2);
}


void handleEvent(SDL_Event event, DrawingWindow &window) {
	CanvasTriangle triangle = randomTriangle(window.width, window.height);
	Colour colour = Colour(rand()% 255, rand()% 255, rand()% 255);
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if(event.key.keysym.sym == SDLK_u){
			draw_stroked_triangle(triangle, colour, window);
		}
		else if(event.key.keysym.sym == SDLK_f){
			draw_stroked_triangle(triangle, {255, 255, 255}, window);
			draw_filled_triangle(triangle, colour, window);

		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}




int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	std::vector<glm::vec3> result = interpolateThreeElementValues({1.0, 4.0, 9.2}, {4.0, 1.0, 9.8}, 4);

	Colour colour = Colour{0, 255, 255};

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// draw_filled_triangle(triangle, colour, window);
		// draw_stroked_triangle(triangle, colour, window);
		// float point = 320 / 3;
		// drawLine({0, 0}, {160, 120}, colour, window);
		// drawLine({319, 0}, {160, 120}, colour, window);
		// drawLine({160, 0}, {160, 239}, colour, window);
		// drawLine({point, 120}, {point * 2, 120}, colour, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
