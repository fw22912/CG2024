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
#include <fstream>
#include <sstream>
#include <iostream>
#include <ModelTriangle.h>
#include <string>
#include <TextureMap.h>
#define WIDTH 320
#define HEIGHT 240


//interpolation
std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues){
	std::vector<float> output{};
	float gap = (to - from)/(numberOfValues);
	output.reserve(numberOfValues);

	for(int i = 0; i <= numberOfValues; ++i){
		output.push_back(from + i * gap);
	}

	return output;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues){
	std::vector<glm::vec3> result;
	float x_gap = (to[0] - from[0])/(numberOfValues);
	float y_gap = (to[1] - from[1])/(numberOfValues);
	float z_gap = (to[2] - from[2])/(numberOfValues);

	result.reserve(numberOfValues);
	for(int i = 0; i<=numberOfValues; ++i){
		glm::vec3 row = {from[0] + x_gap * i, from[1] + y_gap * i, from[2] + z_gap*i};
		result.push_back(row);
	}
	return result;
}


std::vector<CanvasPoint> interpolateCanvasPoints(CanvasPoint from, CanvasPoint to, float numberOfValues) {
	std::vector<CanvasPoint> result;
	float stepX = (to.x - from.x) / (numberOfValues);
	float stepY = (to.y - from.y) / (numberOfValues);

	result.reserve(numberOfValues);
	for (int i = 0; i <= numberOfValues; ++i) {
		CanvasPoint canvasPoints;
		canvasPoints.x = from.x + stepX * i;
		canvasPoints.y = from.y + stepY * i;
		result.push_back(canvasPoints);
	}
	return result;
}


std::vector<TexturePoint> interpolateTexturePoints(TexturePoint from, TexturePoint to, int numberOfValues){
	std::vector<TexturePoint> output;
	float stepX = (to.x - from.x) / (numberOfValues);
	float stepY = (to.y - from.y) / (numberOfValues);

	output.reserve(numberOfValues);
	for (int i = 0; i <= numberOfValues; ++i) {
		TexturePoint texturePoint;
		texturePoint.x = from.x + stepX * i;
		texturePoint.y = from.y + stepY * i;
		output.push_back(texturePoint);
	}
	return output;
}


uint32_t getTextureColour(TexturePoint texturePoint, TextureMap &textureMap) {
	int texX = round(texturePoint.x * (textureMap.width - 1));
	int texY = round(texturePoint.y * (textureMap.height - 1));
	int index = texY * textureMap.width + texX;

	return textureMap.pixels[index];
}


// Drawing
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


void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window){
	auto range_x = abs(to.x - from.x);
	auto range_y = abs(to.y - from.y);
	int steps = std::max(range_x, range_y);

	std::vector<float> y_vals = interpolateSingleFloats(from.y, to.y, steps);
	std::vector<float> x_vals = interpolateSingleFloats(from.x, to.x, steps);

	for (size_t i = 0; i < steps; i++){
		uint32_t pixelColour = (255 << 24) + (uint32_t(colour.red) << 16) + (uint32_t(colour.green) << 8) + uint32_t(colour.blue);
		window.setPixelColour(x_vals[i], y_vals[i], pixelColour);
	}
}


void drawTexturedLine(CanvasPoint from, CanvasPoint to, TexturePoint fromTP, TexturePoint toTP, TextureMap &textureMap, DrawingWindow &window) {
	float x_diff = to.x -from.x;
	float y_diff = to.y -from.y;
	int numberOfSteps = fmax(abs(x_diff), abs(y_diff));

	std::vector<CanvasPoint> canvasPoints = interpolateCanvasPoints(from, to, numberOfSteps);
	std::vector<TexturePoint> texturePoints = interpolateTexturePoints(fromTP, toTP, numberOfSteps);

	for (float i = 0.0; i < canvasPoints.size(); ++i) {
		CanvasPoint canvasPoint = canvasPoints[i];
		TexturePoint texturePoint = texturePoints[i];
		uint32_t colour = getTextureColour(texturePoint, textureMap);
		window.setPixelColour(canvasPoint.x, canvasPoint.y, colour);
	}
}


//helper functions
CanvasTriangle reorderTriangle(CanvasTriangle triangle){
	if (triangle[0].y > triangle[1].y) std::swap(triangle[0], triangle[1]);
	if (triangle[1].y > triangle[2].y) std::swap(triangle[1], triangle[2]);
	if (triangle[0].y > triangle[1].y) std::swap(triangle[0], triangle[1]);

	return triangle;
}


std::unordered_map<std::string, Colour> readOBJ_material(std::string material_path){
	// std::string material_path = "/Users/hyoyeon/Desktop/UNI/Year 3/Computer Graphics/CG2024/Weekly Workbooks/04 Wireframes and Rasterising/models/cornell-box.mtl";
	std::unordered_map<std::string, Colour> colourmap;
	std::ifstream file(material_path);

	std::string line;
	std::string curr_material;
	while(std::getline(file, line)){
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;

		if(prefix == "newmtl"){
			iss >> curr_material;
			colourmap[curr_material] = Colour();
			colourmap[curr_material].name = curr_material;
		} else if(prefix == "Kd"){
			float r, g, b;
			iss >> r >> g >> b;
			colourmap[curr_material].red = static_cast<int>(r * 255.0f);
			colourmap[curr_material].green = static_cast<int>(g * 255.0f);
			colourmap[curr_material].blue = static_cast<int>(b * 255.0f);
		}
	}
	file.close();
	return colourmap;
}


std::pair<std::vector<glm::vec3>, std::vector<ModelTriangle>> read_OBJ(std::string file_name, float scale, std::string material_path){
	std::ifstream inputFile(file_name);
	std::string line;
	std::vector<ModelTriangle> triangles;
	std::vector<glm::vec3> vertices;
	std::unordered_map<std::string, Colour> colourmap = readOBJ_material(material_path);
	std::string curr_colour;

	while (std::getline(inputFile, line)) {
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;

		if (prefix == "v") {
			double x, y, z;
			iss >> x >> y >> z;
			vertices.push_back(glm::vec3{x * scale, y * scale, z * scale});
			// std::cout << "Vertex - x: " << x << " y: " << y << " z: " << z << "\n";
		} else if (prefix == "f") {
			int v1, v2, v3;
			char slash;

			iss >> v1 >> slash >> v2 >> slash >> v3;
			Colour colour = colourmap.find(curr_colour)->second;
			std::cout << "Colour: " << colourmap.find(curr_colour)->second;
 			ModelTriangle triangle(vertices[v1 - 1], vertices[v2 - 1], vertices[v3 - 1], colour);
			triangle.colour = colour;
			triangles.push_back(triangle);
			// std::cout << "Face ||  v1: " << v1 << " v2: " << v2 << " v3: " << v3 << " Colour: " << colour << "\n";
		} else if (prefix == "usemtl") {
			iss >> curr_colour;
			if (colourmap.find(curr_colour) == colourmap.end()) {
				std::cout << "No corresponding colour material: " << curr_colour << "\n";
			}
		}
	}
	inputFile.close();

	std::pair<std::vector<glm::vec3>, std::vector<ModelTriangle>> pair = {vertices, triangles};

	return pair;
}



// Triangle
void draw_stroked_triangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window){
	drawLine(triangle.v0(), triangle.v1(), colour, window);
	drawLine(triangle.v1(), triangle.v2(), colour, window);
	drawLine(triangle.v2(), triangle.v0(), colour, window);
}



void fill_top_triangle(CanvasPoint v0, CanvasPoint v1, CanvasPoint v2, Colour colour, DrawingWindow &window)
{
	float stepX1 = (v1.x - v0.x) / (v1.y - v0.y);
	float stepX2 = (v2.x - v0.x) / (v2.y - v0.y);

	for(float y = v0.y; y <= v1.y; ++y){
		float x1 = v0.x + (y - v0.y) * stepX1;
		float x2 = v0.x + (y - v0.y) * stepX2;

		for (float x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
			window.setPixelColour(x, y, (255 << 24) + (uint32_t(colour.red) << 16) + (uint32_t(colour.green) << 8) + uint32_t(colour.blue));
		}
	}
}


void fill_lower_triangle(CanvasPoint v0, CanvasPoint v1, CanvasPoint v2, Colour colour, DrawingWindow &window)
{
    float stepX1 = (v2.x - v0.x) / (v2.y - v0.y);
    float stepX2 = (v2.x - v1.x) / (v2.y - v1.y);

    for(float y = v0.y; y <= v2.y; ++y){
        float x1 = v0.x + (y - v0.y) * stepX1;
        float x2 = v1.x + (y - v1.y) * stepX2;

        for (float x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
            window.setPixelColour(x, y, (255 << 24) + (uint32_t(colour.red) << 16) + (uint32_t(colour.green) << 8) + uint32_t(colour.blue));
        }
    }
}



void draw_filled_triangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window){
	CanvasTriangle sorted_triangle = reorderTriangle(triangle);
	CanvasPoint v0 = sorted_triangle.v0();
	CanvasPoint v1 = sorted_triangle.v1();
	CanvasPoint v2 = sorted_triangle.v2();

	if(sorted_triangle.v1().y == sorted_triangle.v0().y){
		fill_lower_triangle(v0, v1, v2, colour, window);
	}
	else if(sorted_triangle.v2().y == sorted_triangle.v1().y)
	{
		fill_top_triangle(v0, v1, v2, colour, window);
	}
	else {
		float t = (v1.y - v0.y) / (v2.y - v0.y);
		float v3_x = v0.x + t * (v2.x - v0.x);
		CanvasPoint v3 = {v3_x, v1.y};

		fill_top_triangle(v0, v1, v3, colour, window);
		fill_lower_triangle(v1, v3, v2, colour, window);
	}
}




void fill_texture_top(CanvasPoint bottom, CanvasPoint top, CanvasPoint middle, TextureMap &textureMap, DrawingWindow &window) {
	float top_inc = (top.x - bottom.x) / (top.y - bottom.y);
	float mid_inc = (middle.x - bottom.x) / (middle.y - bottom.y);
	float curr_topX = bottom.x;
	float curr_midX = bottom.x;

	for (float y = bottom.y; y <= top.y; ++y) {
		CanvasPoint from = {curr_topX, y};
		CanvasPoint to = {curr_midX, y};

		float y_inc = (y - bottom.y) / (top.y - bottom.y);

		TexturePoint fromTP = {bottom.texturePoint.x + y_inc * (top.texturePoint.x - bottom.texturePoint.x),
							   bottom.texturePoint.y + y_inc * (top.texturePoint.y - bottom.texturePoint.y)};
		TexturePoint toTP = {bottom.texturePoint.x + y_inc * (middle.texturePoint.x - bottom.texturePoint.x),
							 bottom.texturePoint.y + y_inc * (middle.texturePoint.y - bottom.texturePoint.y)};
		drawTexturedLine(from, to, fromTP, toTP, textureMap, window);
		curr_topX += top_inc;
		curr_midX += mid_inc;
	}
}


void fill_texture_bottom(CanvasPoint top, CanvasPoint bottom, CanvasPoint middle, TextureMap &textureMap, DrawingWindow &window) {
    float top_inc = (top.x - bottom.x) / (top.y - bottom.y);
    float mid_inc = (top.x - middle.x) / (top.y - middle.y);
    float curr_topX = bottom.x;
    float curr_midX = middle.x;

    for (float y = bottom.y; y <= top.y; ++y) {
        CanvasPoint from = {curr_topX, y};
        CanvasPoint to = {curr_midX, y};

        float y_inc = (y - bottom.y) / (top.y - bottom.y);
        TexturePoint fromTP = {bottom.texturePoint.x + y_inc * (top.texturePoint.x - bottom.texturePoint.x),
                               bottom.texturePoint.y + y_inc * (top.texturePoint.y - bottom.texturePoint.y)};
        TexturePoint toTP = {middle.texturePoint.x + y_inc * (top.texturePoint.x - middle.texturePoint.x),
                             middle.texturePoint.y + y_inc * (top.texturePoint.y - middle.texturePoint.y)};

        drawTexturedLine(from, to, fromTP, toTP, textureMap, window);
        curr_topX += top_inc;
        curr_midX += mid_inc;
    }
}


void draw_textured_triangle(CanvasTriangle triangle, TextureMap &textureMap, DrawingWindow &window) {
	CanvasTriangle sorted = reorderTriangle(triangle);
	CanvasPoint v0 = sorted.v0();
	CanvasPoint v1 = sorted.v1();
	CanvasPoint v2 = sorted.v2();

	if (v1.y == v2.y){
		fill_texture_top(v0, v1, v2, textureMap, window);
	}
	else if (v0.y == v1.y) {
		fill_texture_bottom(v2, v0, v1, textureMap, window);
	}
	else {
		float splitRatio = (v1.y - v0.y) / (v2.y - v0.y);
		CanvasPoint inter = {v0.x + splitRatio * (v2.x - v0.x), v1.y};
		inter.texturePoint.x = v0.texturePoint.x + splitRatio * (v2.texturePoint.x - v0.texturePoint.x);
		inter.texturePoint.y = v0.texturePoint.y + splitRatio * (v2.texturePoint.y - v0.texturePoint.y);

		fill_texture_top(v0, v1, inter, textureMap, window);
		fill_texture_bottom(v2, v1, inter, textureMap, window);
	}
}


// CanvasPoint projectVertexOntoCanvasPoint(glm::vec3 cameraPosition, float focalLength, glm::vec3 vertexPosition, DrawingWindow &window){
// 	CanvasPoint projected_position;
// 	glm::vec3 relative_position = vertexPosition - cameraPosition;
//
// 	projected_position.x = focalLength * (relative_position.x / relative_position.z) + window.width * 0.5;
// 	projected_position.y = focalLength * (relative_position.y / relative_position.z) + window.height * 0.5;
//
// 	return projected_position;
// }

CanvasPoint projectVertexOntoCanvasPoint(glm::vec3 cameraPosition, float focalLength, glm::vec3 vertexPosition, DrawingWindow &window, float scalingFactor) {
	CanvasPoint projected_position;
	glm::vec3 relative_position = vertexPosition - cameraPosition;

	projected_position.x = focalLength * (relative_position.x / relative_position.z) * scalingFactor;
	projected_position.y = focalLength * (relative_position.y / relative_position.z) * scalingFactor;

	// std::cout << "BEFORE PROJECTED       X : " << projected_position.x << "  Y: " << projected_position.y << "\n";
	// projected_position.y = focalLength * (relative_position.y / relative_position.z) * scalingFactor;

	projected_position.x = window.width * 0.5 - projected_position.x;
	projected_position.y += window.height * 0.5;

	if(projected_position.y > 320)
	{
		std::cout<<"TOO LARGE Y VALUE:  " << projected_position.y << "\n";
		std::errc;
	}

	return projected_position;
}



void drawProjectedPoints(std::vector<CanvasPoint> points, Colour colour, DrawingWindow &window) {
	for (const auto& point : points) {
		// Draw each projected point on the canvas
		uint32_t pixelColour = (255 << 24) + (uint32_t(colour.red) << 16) + (uint32_t(colour.green) << 8) + uint32_t(colour.blue);

		// std::cout << "x: " << point.x << " y: " << point.y << "\n";
		window.setPixelColour(point.x, point.y, pixelColour);
	}
}


std::vector<CanvasPoint> projectFileOntoCanvasPoint(std::vector<glm::vec3> vertices, glm::vec3 cameraPosition, float focalLength, DrawingWindow &window){
	std::vector<CanvasPoint> projected_vertices;

	for(const auto& vertex : vertices)
	{
		CanvasPoint projected = projectVertexOntoCanvasPoint(cameraPosition, focalLength, vertex, window, 160);
		projected_vertices.push_back(projected);
		// std::cout << "Projected points: " << projected << "\n";
	}

	return projected_vertices;
}


void projectTriangleOntoCanvasPoint(std::vector<ModelTriangle>& triangles, glm::vec3 cameraPosition, float focalLength, DrawingWindow &window, float scalingFactor){
	for(const auto& triangle : triangles)
	{
		CanvasPoint v0 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[0], window, scalingFactor);
		CanvasPoint v1 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[1], window, scalingFactor);
		CanvasPoint v2 = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[2], window, scalingFactor);

		Colour colour = triangle.colour;
		CanvasTriangle new_triangle = CanvasTriangle(v0, v1, v2);

		draw_filled_triangle(new_triangle, colour, window);
		// draw_stroked_triangle(new_triangle, colour, window);
	}
}


CanvasTriangle randomTriangle(int width, int height){
	CanvasPoint v0(rand() % width, rand() % height);
	CanvasPoint v1(rand() % width, rand() % height);
	CanvasPoint v2(rand() % width, rand() % height);

	return CanvasTriangle{v0, v1, v2};
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
		else if(event.key.keysym.sym == SDLK_d){
			TextureMap map = TextureMap("/Users/hyoyeon/Desktop/UNI/Year 3/Computer Graphics/CG2024/Weekly Workbooks/03 Triangles and Textures/texture.ppm");
			CanvasPoint v0 = {160, 10};
			CanvasPoint v1 = {300, 230};
			CanvasPoint v2 = {10, 150};

			v0.texturePoint = TexturePoint{195.0f / map.width, 5.0f/map.height};
			v1.texturePoint = TexturePoint{395.0f / map.width, 380.0f/map.height};
			v2.texturePoint = TexturePoint{65.0f / map.width, 330.0f/map.height};

			CanvasTriangle texture_triangle = CanvasTriangle{v0, v1, v2};
			draw_textured_triangle(texture_triangle, map, window);
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
    Colour colour = Colour{255, 255, 255};

    std::string file_path = "/Users/hyoyeon/Desktop/UNI/Year 3/Computer Graphics/CG2024/Weekly Workbooks/04 Wireframes and Rasterising/models/cornell-box.obj";
    std::string material_path = "/Users/hyoyeon/Desktop/UNI/Year 3/Computer Graphics/CG2024/Weekly Workbooks/04 Wireframes and Rasterising/models/cornell-box.mtl";

    std::pair<std::vector<glm::vec3>, std::vector<ModelTriangle>> pair = read_OBJ(file_path, 0.35, material_path);
    std::vector<glm::vec3> vertices = pair.first;
    std::vector<ModelTriangle> triangles = pair.second;

    glm::vec3 cameraPoint = {0.0, 0.0, 4.0};
    float focalLength = 2.0;

    projectTriangleOntoCanvasPoint(triangles, cameraPoint, focalLength, window, 160);

    while (true) {
        // We MUST poll for events - otherwise the window will freeze!
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        // Render the frame
        window.renderFrame();
    }
}
