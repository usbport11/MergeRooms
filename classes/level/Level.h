#ifndef levelH
#define levelH

#define LGT_NONE 0
#define LGT_DUNGEON 1
#define LGT_CAVE 2

class MLevel {
protected:
	int Type;
	int TilesCount[2];
	char** Map;
	
	void FillMap(int x , int y, int width, int height);
	bool AllocateMap();
	bool AllocateMap(int TilesCountX, int TilesCountY);
	bool ReallocateMap(int TilesCountX, int TilesCountY);
public:
	MLevel();
	MLevel(int TilesCountX, int TilesCountY);
	virtual bool Generate();
	virtual int GetType();
	void Clear();
	void Close();
};

#endif
