// Шаблон программы для задачи от 5.10.2020-6.10.2020
//
/*
 Задача:
 Построить график функции
 z = sinc(sqrt(x*x + y*y))
 Шаг 1. Придумать формулы для перевода координат из трёхмерной системы
 (x, y, z) в координаты точки на картинке (sx, sy)
 с учётом масштабных коэффициентов по всем трём осям
 Шаг 2. Построить график функции, перебирая одну координату с большим шагом,
 а вторую - с маленьким шагом.
 Шаг 2.5. Построить график ещё раз, поменяв местами шаг по осям.
 (на данном этапе у нас уже получится некий график, но....)
 Шаг 3. Для сокрытия невидимых линий графика реализовать алгоритм
 "плавающего горизонта"
 1. График всегда рисуется от ближних объектов к дальним
 2. Если очередная точка находится ниже линии горизонта,
 то её не рисуем
 3. Если очередная точка находится выше линии горизонта, то:
 * точку рисуем;
 * поднимаем в этой колонке изображения горизонт до уровня
 новой точки
 4. Перед вторым проходом рисования графика (в перпендикулярных
 плоскостях) горизонт опускаем.
 Область построения: -30 <= x <= 30 и -30 <= y <= 30 (например)
 */
#include <fstream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <iostream>

// Это у нас структура заголовка TGA-файла
#pragma pack(push, 1)
struct TGA_Header {
	uint8_t id_len;			// Длина идентификатора
	uint8_t pal_type;		// Тип палитры
	uint8_t img_type;		// Тип изображения
	uint8_t pal_desc[5];	// Описание палитры
	uint16_t x_pos;			// Положение по оси X
	uint16_t y_pos;			// Положение по оси Y
	uint16_t width;			// Ширина
	uint16_t height;		// Высота
	uint8_t depth;			// Глубина цвета
	uint8_t img_desc;		// Описатель изображения
};
#pragma pack(pop)

constexpr uint16_t IMG_WIDTH = 1920;
constexpr uint16_t IMG_HEIGHT = 1080;
constexpr uint16_t x_center = IMG_WIDTH / 2;
constexpr uint16_t y_center = IMG_HEIGHT / 2;
constexpr uint16_t count_dots_X_Y = 30;
constexpr uint16_t count_dots_Z = 2;
constexpr uint16_t coeff_dots_X_Y = y_center / count_dots_X_Y;
constexpr uint16_t coeff_dots_Z = y_center / count_dots_Z;
constexpr uint32_t COL_BACKGROUND = 0xff003f3f;
constexpr uint32_t COL_DOT = 0xfdc00;
constexpr double Pi = acos(-1.);
constexpr double angle120 = 120 * Pi / 180;
constexpr double angle240 = 240 * Pi / 180;

double sinc(double x) {
	if (x == 0)
		return 1.;
	return sin(x) / x;
}

double my_evil_function(double x, double y) {
	return sinc(hypot(x, y));
}

std::vector<int> get_Dot(double x, double y, double z) {
	double screen_Xx = x_center + coeff_dots_X_Y * (0 * cos(angle120) - x * sin(angle120));
	double screen_Xy = y_center - coeff_dots_X_Y * (0 * sin(angle120) + x * cos(angle120));
	double screen_Yx = x_center + coeff_dots_X_Y * (0 * cos(angle240) - y * sin(angle240));
	double screen_Yy = y_center - coeff_dots_X_Y * (0 * sin(angle240) + y * cos(angle240));
	double screen_Zx = x_center;
	double screen_Zy = y_center - z * coeff_dots_Z;
	int x_result = (int) (x_center + (screen_Xx - x_center) + (screen_Yx - x_center) + (screen_Zx - x_center));
	int y_result = (int) (y_center + (screen_Xy - y_center) + (screen_Yy - y_center) + (screen_Zy - y_center));
	return {x_result,y_result};
}

int main() {
	std::vector<uint32_t> picture(IMG_WIDTH * IMG_HEIGHT);
	for (auto &&p : picture) {
		p = COL_BACKGROUND;
	}

	std::vector<uint32_t> horizon(IMG_WIDTH);
	for (auto &&h : horizon) {
		h = IMG_HEIGHT;
	}
	for (double step_X = count_dots_X_Y; step_X > -count_dots_X_Y; step_X -= 0.6) {
		for (double step_Y = count_dots_X_Y; step_Y > -count_dots_X_Y; step_Y -= 0.03) {
			double z = my_evil_function(step_X, step_Y);
			std::vector<int> dot = get_Dot(step_X, step_Y, z);
			for (double y1 = dot[1] - 2; y1 < dot[1] + 2; ++y1) {
				for (double x1 = dot[0] - 2; x1 < dot[0] + 2; ++x1) {
					if (y1 < horizon[x1]) {
						picture[y1 * IMG_WIDTH + x1] = COL_DOT;
						horizon[x1] = y1;
					}
				}
			}
		}
	}
	for (auto &&h : horizon) {
		h = IMG_HEIGHT;
	}
	for (double stepX = count_dots_X_Y; stepX > -count_dots_X_Y; stepX -= 0.03) {
		for (double stepY = count_dots_X_Y; stepY > -count_dots_X_Y; stepY -= 0.6) {
			double z = my_evil_function(stepX, stepY);
			std::vector<int> dot = get_Dot(stepX, stepY, z);
			for (double y1 = dot[1] - 2; y1 < dot[1] + 2; ++y1) {
				for (double x1 = dot[0] - 2; x1 < dot[0] + 2; ++x1) {
					if (y1 <= horizon[x1]) {
						picture[y1 * IMG_WIDTH + x1] = COL_DOT;
						horizon[x1] = y1;
					}
				}
			}
		}
	}


	TGA_Header hdr { };
	hdr.width = IMG_WIDTH;
	hdr.height = IMG_HEIGHT;
	hdr.depth = 32;
	hdr.img_type = 2;
	hdr.img_desc = 0x28;


	std::ofstream tga_file { "output.tga", std::ios::binary };

	tga_file.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));
	tga_file.write(reinterpret_cast<char*>(&picture[0]),
			picture.size() * sizeof(uint32_t));

	tga_file.close();

	return 0;
}
