#include "BitMap.h"
#include <fstream>

struct RTTEXINFO {
	int Height;
	int Width;
	int Format;
	int RealHeight;
	int RealWidth;
	bool useAlpha;
	bool isCompressed;
	short Flags;
	int MipMapCount;
};

struct RTTEX {
private:
	std::fstream stream;
	BitMap* bitMap = nullptr;

public:
	RTTEXINFO Info;

	template<typename T>
	void Read(T* data) {
		stream.read(reinterpret_cast<char*>(data), sizeof(T));
	}
	RTTEX(const char* File) {
		stream = std::fstream(File, std::ios_base::in | std::ios::binary);
		SetScanPosition(8, true);
		Read(&Info);

		bitMap = new BitMap(Info.Height, Info.Width);

		SetScanPosition(88);

		int* Bits = bitMap->GetBitData();

		for (int y = Info.Height - 1; y >= 0; y--) {
			for (int x = 0; x < Info.Width; x++)
			{
				if (Info.useAlpha) {
					RGB_A rgba;
					Read(&rgba);
					Bits[x + y * Info.Width] = (rgba.r << 16 | rgba.a << 8 | rgba.b | rgba.g << 24);
				}
				else {
					ColorRGB rgb;
					Read(&rgb);
					Bits[x + y * Info.Width] = (rgb.r << 16 | rgb.g << 8 | rgb.b | -16777216);
				}
			}
		}
	}

	~RTTEX() {
		delete bitMap;
	}
	void SetScanPosition(int Pos, bool isOrigin = false) {
		stream.seekg(Pos, isOrigin ? stream._Seekbeg : stream._Seekcur);
	}
	BitMap* GetMap() {
		return bitMap;
	}
};
