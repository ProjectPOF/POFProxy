#include "RTTEX.h"
#include "variant2.hpp"
#include "rtparam.hpp"
#include <urlmon.h>

#pragma comment(lib, "Urlmon.lib")

using namespace std::chrono;

struct Vector2 {
	float x;
	float y;
	Vector2(int x, int y) : x(x), y(y) {}
};

// Get Answer Directly if its white.
float GetAnswer(RTTEX& Image) {
	RGB_A WhiteColor(255, 255, 255, 255);
	uint8_t WhitePixelCount = 0;
	for (int Y = 0; Y < Image.Info.RealHeight; Y++) {
		for (int X = 0; X < Image.Info.RealWidth; X++) {
			if (Image.GetMap()->GetPixelRGBA(X, Y) == WhiteColor) {
				for (int i = 0; i < 60; i++) {
					if (X + i < 0 || X + i > Image.Info.Width) continue;
					if (Image.GetMap()->GetPixelRGBA(X + i, Y) == WhiteColor) {
						WhitePixelCount++;
					}
					if (WhitePixelCount > 50) {
						float m_x = X;
						return (m_x) * (1.f / Image.Info.Width);
					}
				}
			}
			WhitePixelCount = 0;
		}
	}
}

// Makes a RGBA brighter.
void MakeBrighter(RGB_A& toChange) {

	auto Bright = [&](RGB_A a) {
		a.r = trunc(a.r + (int)((-166.f * (float)a.r / 255.f) + 166.f));
		a.g = trunc(a.g + (int)((-166.f * (float)a.g / 255.f) + 166.f));
		a.b = trunc(a.b + (int)((-166.f * (float)a.b / 255.f) + 166.f));
		return a;
	};

	toChange = Bright(toChange);
}

// Makes a square texture 50x50 from puzzle piece.
BitMap* MakeSquare(RTTEX& FROM, Vector2 Size = Vector2(50, 50), bool makeBright = true)	 {
	BitMap* Map = new BitMap(Size.x, Size.y);
	int* Bits = Map->GetBitData();
	BitMap* FileMap = FROM.GetMap();

	int xStart = 24;
	int yStart = 0;

	for (int y = 0; y < FROM.Info.RealHeight; y++) {
		if (yStart != 0) break;
		for (int x = 0; x < FROM.Info.RealWidth; x++) {
			RGB_A col = FileMap->GetPixelRGBA(x, y);

			if (col.r == 255) {
				yStart = 20 + y;
				break;
			}
		}
	}

	for (int Y_POS = yStart; Y_POS < yStart + 50; Y_POS++) {
		for (int X_POS = xStart; X_POS < xStart + 50; X_POS++) {
			RGB_A rgba = FileMap->GetPixelRGBA(X_POS, Y_POS);
			if (makeBright)
				MakeBrighter(rgba);
			Bits[0] = *reinterpret_cast<int*>(&rgba);
			Bits++;
		}
	}

	return Map;
}

// A function that checks if the bright color is near brightness equation.
bool isNearEquation(RGB_A Original, RGB_A Color) {
	return std::abs(Original.r - Color.r) < 3 &&
		std::abs(Original.g - Color.g) < 3 &&
		std::abs(Original.b - Color.b) < 3;
}

// Function that matches square area to destination. Which will return slider value.
float AnswerByEquation(RTTEX& image, BitMap* Square) {
	if (!Square) return 0.f;

	BitMap* ImageMap = image.GetMap();
	bool onFinished = true;

	for (int Y = 0; Y < image.Info.RealHeight; Y++) {
		for (int X = 0; X < image.Info.RealWidth; X++) {
			if (isNearEquation(ImageMap->GetPixelRGBA(X, Y), Square->GetPixelRGBA(12, 12))) {
				int MatchCount = 0;
				for (int y = 12; y < Square->Height - 12; y++) {
					bool break_this = false;
					for (int x = 12; x < Square->Width - 12; x++) {
						if (!isNearEquation(ImageMap->GetPixelRGBA(X + x - 12, Y + y - 12), Square->GetPixelRGBA(x, y))) {
							onFinished = false;
							break_this = true;
							break;
						}
						MatchCount++;
					}
					if (break_this) break;
				}
				if (onFinished || MatchCount > 200) {
					printf("[CAPTCHA]: Solved Piece Location (%u,%u)\n", X, Y);
					return ((float)(X - 28) / 512);
				}
			}
		}
	}
	return 0.f;
}

std::string SolveCaptcha(variant_t& variant) {

	steady_clock::time_point start = high_resolution_clock::now();

	rtvar parse = rtvar::parse(variant.get_string());
	std::vector<std::string> Values = parse.find("add_puzzle_captcha")->m_values;
	std::string DownloadLink = "https://" + Values[2] + "/" + Values[0];
	std::string PuzzlePieceLink = "https://" + Values[2] + "/" + Values[1];
	std::string FilePath = "captcha" + Values[3] + ".rttex";
	std::string PuzzleFilePath = "piece" + Values[3] + ".rttex";

	char Dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, Dir);

	std::string ExactFilePath = (std::string)Dir + "\\" + FilePath;
	std::string ExactPuzzlePath = (std::string)Dir + "\\" + PuzzleFilePath;

	printf("[CAPTCHA]: Downloading From: %s\n\n", DownloadLink.c_str());

	if (SUCCEEDED(URLDownloadToFile(NULL, DownloadLink.c_str(), ExactFilePath.c_str(), 0, NULL))) {
		printf("[CAPTCHA]: Download succeeded.\n");
		RTTEX Image(FilePath.c_str());

		float Answer = GetAnswer(Image);

		if (Answer == 0.f) {
			printf("[CAPTCHA]: Downloading From: %s\n\n", PuzzlePieceLink.c_str());
			if(SUCCEEDED(URLDownloadToFile(NULL, PuzzlePieceLink.c_str(), ExactPuzzlePath.c_str(), 0, NULL))) {
			printf("[CAPTCHA]: Downloaded Puzzle Piece.\n");
			steady_clock::time_point m_start = high_resolution_clock::now();
			RTTEX PuzzlePiece(PuzzleFilePath.c_str());
			BitMap* Square = MakeSquare(PuzzlePiece, Vector2(50, 50), true);
			Answer = AnswerByEquation(Image, Square);
			steady_clock::time_point m_end = high_resolution_clock::now();
			if (Answer != 0.f) {
				printf("[CAPTCHA]: Matched Pattern (in %f miliseconds)!\n\n", duration<double, std::milli>(m_end - m_start));
				}
			}
		}

		std::remove(FilePath.c_str());
		std::remove(PuzzleFilePath.c_str());
		
		steady_clock::time_point end = high_resolution_clock::now();

		std::cout << "[CAPTCHA]: Downloaded & Solved in " << duration<double, std::milli>(end - start).count() << " miliseconds.\n";
		return "action|dialog_return\ndialog_name|puzzle_captcha_submit\ncaptcha_answer|" + std::to_string(Answer) + "|CaptchaID|" + Values[3];
	}
	printf("[CAPTCHA]: File couldn't download.\n\n");
	std::remove(FilePath.c_str());
	return "";
}
