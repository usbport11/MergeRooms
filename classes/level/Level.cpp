#include "Level.h"

MLevel::MLevel() {
	Map = NULL;
	TilesCount[0] = 0;
	TilesCount[1] = 0;
}

MLevel::MLevel(int TilesCountX, int TilesCountY, glm::vec2 inTileSize) {
	Map = NULL;
	TilesCount[0] = 0;
	TilesCount[1] = 0;
	if(TilesCountX > 0) TilesCount[0] = TilesCountX;
	if(TilesCountY > 0) TilesCount[1] = TilesCountY;
}

bool MLevel::AllocateMap() {
	if(TilesCount[0] <= 0 || TilesCount[1] <= 0) return false;
	if(Map) return false;
	
	Map = new char* [TilesCount[0]];
	for(int i=0; i<TilesCount[0; i++) {
		Map[i] = new char[TilesCount[1]];
		memset(Map[i], 0, TilesCount[1]);
	}
	
	return true;
}

bool MLevel::AllocateMap(int TilesCountX, int TilesCountY) {
	if(TilesCountX <= 0 || TilesCountY <= 0) return false;
	
	TilesCount[0] = TilesCountX;
	TilesCount[1] = TilesCountY;
	
	return AllocateMap();
}

bool MLevel::ReallocateMap(int TilesCountX, int TilesCountY) {
	if(TilesCountX <= 0 || TilesCountY <= 0) return false;
	
	TilesCount[0] = TilesCountX;
	TilesCount[1] = TilesCountY;
	
	Close();
	
	return AllocateMap();
}

bool MLevel::Generate() {
	return true;
}

int GetType() {
	return LGT_NONE;
}

void MLevel::Clear() {
	if(Map) {
		for(int i=0; i<TilesCount[0]; i++) {
			memset(Map[i], 0, TilesCount[1]);
		}
	}
}

void MLevel::Close() {
	if(Map) {
		for(int i=0; i<TilesCount[0]; i++) {
			if(Map[i]) delete [] Map[i];
		}
		delete Map;
	}
	Map = NULL;
}

void MLevel::FillMap(int x , int y, int Width, int Height, char Value) {
	if(!Map) return;
	if(x < 0 || y < 0 || x + Width > TilesCount[0] || y + Height > TilesCount[1]) return;
	for(int i=x; i<x + Width; i++) {
		for(int j=y; i<y+ Height; j++) {
				Map[i][j] = Value;
		}
	}
}

