/*
	2021-2022 FBLA COMPUTER GAME & SIMULATION PROGRAMMING PROJECT
	ALEXANDER HIGH SCHOOL, DOUGLAS COUNTY SCHOOL SYSTEM, GEORGIA

	Group members:
		David Gorzynski - Programmer - dagorzynski@gmail.com - 770 722 2508
		Emilio Lopez - Composer
		Brayden Bayle - Artist

	Writen in C++, using the OLC Pixel Game Engine. No copywriten code is present in this program.
		GitHub: https://github.com/OneLoneCoder/olcPixelGameEngine

	DESCRIPTION:
	This is a top-down city building arcade game that requires the user to quickly expand before time runs out.

	TODO: APOLOGY
	To anyone who needs to sort through this code, I am sincerely sorry. Hopefully the comments are enough to make sence of it. I reccomend collapsing all segments to begin with
*/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

#define PI 3.141592653589
#define ScreenCellWidth 9
#define ScreenCellHeight 7
#define ScreenCenter olc::vf2d(round(ScreenCellWidth / 2), round(ScreenCellHeight / 2))
#define CellSize 32
#define CellCenter olc::vf2d(CellSize / 2, CellSize / 2)
#define CharSize 8
#define TabSize 4
#define IconSize 16
#define CONSTANT true

namespace ASSETS
{
	/* = = = = INDIVIDIAL SPRITES = = = = */
	olc::Decal* logo;

	/* = = = = SPRITE SHEETS = = = = */
	struct spriteSheet
	{
		std::string path;
		int rows;
		int columns;
		olc::vi2d spriteSize;

		std::unique_ptr<olc::Sprite> sheet;
		std::vector<std::vector<std::unique_ptr<olc::Sprite>>> sprites;
		std::vector<std::vector<std::unique_ptr<olc::Decal>>> decals;

		spriteSheet(std::string _path, int _rows, int _columns, olc::vi2d _spriteSize) : path(_path), rows(_rows), columns(_columns), spriteSize(_spriteSize) {}
	};

	const std::string INFRA_ASSET_PATH = "Assets\\InfrastructureAssetsV7.png";
	const int INFRA_ASSET_ROWS = 6;
	const int INFRA_ASSETS_COLUMNS = 12;
	const olc::vi2d INFRA_ASSETS_SPRITE_SIZE = olc::vi2d(CellSize, CellSize);
    
	spriteSheet infraAssets(INFRA_ASSET_PATH, INFRA_ASSET_ROWS, INFRA_ASSETS_COLUMNS, INFRA_ASSETS_SPRITE_SIZE);
    
	const std::string INFRA_ICONS_PATH = "Assets\\InfrastructureIcons.png";
	const int INFRA_ICONS_ROWS = 6;
	const int INFRA_ICONS_COLUMNS = 1;
	const olc::vi2d INFRA_ICONS_SPRITE_SIZE = olc::vi2d(IconSize, IconSize);
    
	spriteSheet infraIcons(INFRA_ICONS_PATH, INFRA_ICONS_ROWS, INFRA_ICONS_COLUMNS, INFRA_ICONS_SPRITE_SIZE);
    
	const std::string ALPHA_CHARS_PATH = "Assets\\Arcade_Font_colorable[8x8]v3.png";
	const int ALPHA_CHARS_ROWS = 10;
	const int ALPHA_CHARS_COLUMNS = 10;
	const olc::vi2d ALPHA_CHARS_SPRITE_SIZE = olc::vi2d(CharSize, CharSize);
    
	spriteSheet alphaChars(ALPHA_CHARS_PATH, ALPHA_CHARS_ROWS, ALPHA_CHARS_COLUMNS, ALPHA_CHARS_SPRITE_SIZE);
    
	/* = = = = LOAD SPRITE SHEETS = = = = */
	std::unique_ptr<olc::Sprite> loadPartialSprite(olc::PixelGameEngine* game, olc::Sprite* sheet, olc::vi2d& pos, const olc::vi2d& size)
	{
		const olc::vi2d origin = olc::vi2d(0, 0);
		std::unique_ptr<olc::Sprite> out = std::unique_ptr<olc::Sprite>(new olc::Sprite(size.x, size.y));
		game->SetDrawTarget(out.get());
		game->DrawPartialSprite(origin, sheet, pos * size, size);
		return std::move(out);
	}

	std::unique_ptr<olc::Decal> loadDecalFromSprite(olc::Sprite* sprite)
	{
		std::unique_ptr<olc::Decal> out = std::unique_ptr<olc::Decal>(new olc::Decal(sprite));
		return std::move(out);
	}

	spriteSheet loadSpriteSheet(olc::PixelGameEngine* game, spriteSheet newSheet)
	{
		newSheet.sheet = std::make_unique<olc::Sprite>(newSheet.path);
		olc::Sprite* screen = game->GetDrawTarget();
		olc::vi2d CELL_SPRITE_POS;

		for (CELL_SPRITE_POS.y = 0; CELL_SPRITE_POS.y < newSheet.rows; CELL_SPRITE_POS.y++)
		{
			std::vector<std::unique_ptr<olc::Sprite>> asset;
			std::vector<std::unique_ptr<olc::Decal>> assetDecal;

			for (CELL_SPRITE_POS.x = 0; CELL_SPRITE_POS.x < newSheet.columns; CELL_SPRITE_POS.x++)
			{
				asset.push_back(loadPartialSprite(game, newSheet.sheet.get(), CELL_SPRITE_POS, newSheet.spriteSize));
				assetDecal.push_back(loadDecalFromSprite(asset[CELL_SPRITE_POS.x].get()));
			}

			newSheet.sprites.push_back(std::move(asset));
			newSheet.decals.push_back(std::move(assetDecal));
		}

		game->SetDrawTarget(screen);

		return std::move(newSheet);
	}

	/* = = = = LOAD VIDEO = = = = */

	std::string videoLocation = "Assets\\MenuVideoFrames\\";
	std::vector<olc::Decal*> menuVideoFrames;
    
	void loadVideo()
	{
		menuVideoFrames.reserve(138);

		for (int i = 0; i <= 137; i++)
		{
			menuVideoFrames.push_back((
			    new olc::Decal(
			        new olc::Sprite(videoLocation + "frame_" + (i != 0 ? std::string(3 - ((int)log10(i) + 1), '0') + std::to_string(i) : "00") + "_delay-0.1s.png"))
			    )
		    );
		}
	}

	/* = = = = MUSIC = = = = */
	std::string theme = "Assets\\Music\\MainTheme.wav";

	void loadSong()
	{
		olc::SOUND::InitialiseAudio(44100, 1, 8, 512);
		olc::SOUND::LoadAudioSample(theme);
		//olc::SOUND::PlaySample(1, true);
	}

	/* = = = = = SCORE MEMORY = = = = = */
	
	struct scoreEntry
	{
	    int score = -1;
	    int level = -1;
	    std::string name = "ERR";
	    
		scoreEntry(std::string line)
	    {
	        int pos1 = 0, pos2 = line.find(' ', pos1);

	        for(int i = 0; i < 3; i++)
	        {

	            switch(i)
	            {
	                case 0:
            	        score = std::stoi(line.substr(pos1, pos2));
						break;
	                case 1:
            	        level = std::stoi(line.substr(pos1 + 1, pos2));
						break;
	                case 2:
            	        name = line.substr(line.size() - 3);
						break;
	            }
	            
    	        pos1 = pos2;
    	        pos2 = line.find(' ', pos2);
	        }
	    }
	    
		scoreEntry(int _score, int _level, std::string _name) : score(_score), level(_level), name(_name) {}

	    std::string to_string()
	    {
	        return std::to_string(score) + (score < 1000 ? '\t' : ' ') + '\t' + std::to_string(level) + '\t' + '\t' + name + '\n';
	    }

		std::string to_save_string()
		{
			return std::to_string(score) + ' ' + std::to_string(level) + ' ' + name + '\n';
		}
	};
	
	std::string scorePath = "Assets\\Scores.txt";
	std::fstream scoreFile;
	std::vector<scoreEntry*> scores;
	
	void loadScores()
	{
		scoreFile.open(scorePath);
	    
		if(scoreFile.is_open())
	    {
			scores.clear();
	        std::string line;
	        
			while (getline(scoreFile, line))
			{
				scores.push_back(new scoreEntry(line));
			}
	    }
	    else
	        std::cout << "ERROR: could not load score path to load previous scores!" << std::endl;

		scoreFile.close();
	}
	
	void addScore(int score, int level, std::string name)
	{
		if (scores.size() > 0)
		{
			// Smallest score first
			if (score < scores[0]->score)
			{
				scores.insert(scores.begin(), new scoreEntry(score, level, name));
				return;
			}

			// Largest score last
			if (score > scores[scores.size() - 1]->score)
			{
				scores.push_back(new scoreEntry(score, level, name));
				return;
			}

			// Loop through
			for (int i = 0; i < scores.size() - 1; i++)
			{
				if (scores[i + 1]->score > score)
				{
					scores.insert(scores.begin() + i + 1, new scoreEntry(score, level, name));
					return;
				}
			}
		}
		else
			scores.push_back(new scoreEntry(score, level, name));
	}
	
	void saveScores()
	{
		scoreFile.open(scorePath, std::ios::out);
		

		if (scoreFile.is_open())
		{
			for (scoreEntry* s : scores)
				scoreFile << s->to_save_string();
		}
		else
			std::cout << "ERROR: could not load score path to save current scores!" << std::endl;

		scoreFile.close();
	}
}

namespace INDEXS
{
	struct aID {
		int ID;
		olc::vi2d pos;

		aID(int _ID, olc::vi2d _pos) : ID{ _ID }, pos{ _pos } {}
	};

	// Resource types and their matrix index
	namespace MATRIX {
		const int POPULATION	= 0;
		const int INCOME		= 1;
		const int CONSTRUCTION	= 2;
		const int ELECTRICITY	= 3;
		const int WASTE			= 4;
		const int TRAFFIC		= 5;
	};
	
	const std::vector<aID> SPRITES {
	  // TYPE, POS
		{ 0, { 0, 0 } },	// TOWER {+$, -C, -E, +W, +T}	  Holds people, makes money
		{ 1, { 0, 3 } },	// EPLANT {-$, -C, +E, +W, +T}	  Creates electricity
		{ 2, { 0, 6 } },	// WPLANT {-$, -C, -E, -W, +T}	  Handles human waste
		{ 3, { 0, 9 } },	// CPLANT {-$, +C, -E, +W, +T}	  Creates construction materials

		{ 4, { 1, 0 } },	// ROAD {-$, -C, =E, =W, -T}	  Accomidates traffic, must be next to a building
		{ 4, { 1, 3 } },	// ROAD {-$, -C, =E, =W, -T}	  Accomidates traffic, must be next to a building
		{ 4, { 1, 6 } },	// ROAD {-$, -C, =E, =W, -T}	  Accomidates traffic, must be next to a building
		{ 4, { 1, 9 } },	// ROAD {-$, -C, =E, =W, -T}	  Accomidates traffic, must be next to a building

		{ 5, { 2, 0 } },	// GRASS
		{ 5, { 2, 3 } },	// DIRT
		{ 5, { 2, 6 } },	// HILLS
		{ 5, { 2, 9 } },	// SAND

		{ 6, { 3, 0 } },	// FOREST
		{ 6, { 3, 3 } },	// ROCKS
		{ 6, { 3, 6 } },	// SWAMP
		{ 6, { 3, 9 } },	// BLANK1

		{ 7, { 4, 0 } },	// MOUNTAINS
		{ 7, { 4, 3 } },	// WATER
		{ 7, { 4, 6 } },	// CLOUDS
		{ 7, { 4, 9 } }		// BLANK2
	};

	namespace STRUCTURES {
		const int STRUCTURE_ID_END	= 4;
		// BUILDINGS
			const int TOWER			= 0;	// {+$, -C, -E, +W, +T}	  Holds people, makes money
			const int EPLANT		= 1;	// {-$, -C, +E, +W, +T}	  Creates electricity
			const int WPLANT		= 2;	// {-$, -C, -E, -W, +T}	  Handles human waste
			const int CPLANT		= 3;	// {-$, +C, -E, +W, +T}	  Creates construction materials
		const int ROAD = 4;
			const int STRAIGHT		= 4;	// {-$, -C, =E, =W, -T}	  Accomidates traffic, must be next to a building
			const int CORNER		= 5;
			const int T_JUNCTION	= 6;
			const int CROSS_ROAD	= 7;
	};
	
	namespace TERRAIN {
		const int BUILDABLE		= 5;		// Can build with no extra cost
			const int GRASS			= 8;
			const int DIRT			= 9;
			const int HILLS			= 10;
			const int SAND			= 11;
		const int REMOVEABLE	= 6;		// Requires money, gives materials
			const int FOREST		= 12;
			const int ROCKS			= 13;
			const int SWAMP			= 14;
			const int BLANK1		= 15;
		const int UNBUILDABLE	= 7;		// Entirely unbuildable
			const int MOUNTAINS		= 16;
			const int WATER			= 17;
			const int CLOUDS		= 18;	// Yet to be bought
			const int BLANK2		= 19;
	};

	namespace SCREENS
	{
		namespace MAIN
		{
			const int INDEX = 0;
    			const int VIDEO = 0;
    			const int ENTER = 1;
    			const int CONTROLS = 2;
    			const int LOGO = 3;
		};

		namespace MAP
		{
			const int INDEX = 1;
			namespace TOP_BAR {
				const int INDEX = 0;
				namespace ELEMENTS {
					const int POPULATION_ICON = 0;
					const int POPULATION_LABEL = 1;
					const int INCOME_ICON = 2;
					const int INCOME_LABEL = 3;
					const int CONSTRUCTION_ICON = 4;
					const int CONSTRUCTION_LABEL = 5;
					const int ELECTRICITY_ICON = 6;
					const int ELECTRICITY_LABEL = 7;
					const int WASTE_ICON = 8;
					const int WASTE_LABEL = 9;
					const int TIME_ICON = 10;
					const int TIME_LABEL = 11;
				};
			};
	
			namespace BOTTOM_BAR {
				const int INDEX = 1;
				namespace ELEMENTS {
					const int ICON = 0;
					const int BODY_1 = 1;
					const int DELINEATOR_1 = 2;
					const int BODY_2 = 3;
					const int DELINEATOR_2 = 4;
					const int BODY_3 = 5;
					const int DELINEATOR_3 = 6;
				};
		
				namespace MODES {
					const int BUILD = 0;
						const int BUILD_MAX = 5;
					const int UPGRADE = 1;
						const int UPGRADE_MAX = 3;
				};
	};
			
			namespace CELIBRATORY {
				const int INDEX = 2;
					const std::string FAIL = 
						"         TOO BAD         \n"
						"YOU DIDN'T QUITE MAKE IT!\n"
						"  BETTER LUCK NEXT TIME! ";
					const std::string WIN = 
						"     CONGRATULATIONS!    \n"
						"   YOU BEAT THIS LEVEL!  \n";
			};
		};

		namespace SCORE
		{
			const int INDEX = 2;
			    const int LOGO = 0;
    			const int HEADER = 1;
    			const int SCORES = 2;
    			const int INPUT = 3;
		};

	};

	namespace SONGS {
		const int THEME = 1;
	};

	namespace ICONS {
		const int INDEX = 5;
			namespace CURSORS {
				const int CURSOR = 0;
					const int DASHED = 0;
					const int SOLID = 1;
					const int INVALID = 2;
			};
			namespace BOTTOM_BAR_ICONS {
				const int BOTTOM_BAR_ICON = 3;
					const int QUANTITY = 3;
					const int QUALITY = 4;
					const int POLICIES = 5;
			};
			namespace DELINEATORS {
				const int DELINEATOR= 6;
					const int BLANK		=-1;
					const int DOTTED	= 0;
					const int DASHED	= 1;
					const int SOLID		= 2;
			}
			const int BLANK4 = 9;
	};
	
};

struct resourceMatrix
{
private:
	int iter = 0;
	float population,			// Score
		  money, construction,	// Currency & tradeable
		  electricity,			// Production & tradeable
		  waste, traffic;		// Deficits (capped & stored in buildings)
	std::string formatedString;	
	bool constant = false;

	// The number of resorces, used in the resorce matrix for looping and out-of-bounds checks
#define RESOURCE_TYPE_NUM	  6

public:
	resourceMatrix()
		: population{ 0.0f }, money{ 0.0f },
		construction{ 0.0f }, electricity{ 0.0f },
		waste{ 0.0f }, traffic{ 0.0f }
	{}

	resourceMatrix(const resourceMatrix& _copy)
		: population{ _copy[INDEXS::MATRIX::POPULATION]}, money{ _copy[INDEXS::MATRIX::INCOME] },
		construction{ _copy[INDEXS::MATRIX::CONSTRUCTION] }, electricity{ _copy[INDEXS::MATRIX::ELECTRICITY] },
		waste{ _copy[INDEXS::MATRIX::WASTE] }, traffic{ _copy[INDEXS::MATRIX::TRAFFIC] }
	{}

	resourceMatrix(float _population, float _income, float _construction, float _electricity, float _waste, float _traffic, bool _constant = false)
		: population{ _population }, money{ _income }, construction{ _construction }, electricity{ _electricity }, waste{ _waste }, traffic{ _traffic }
	{
		if (_constant)
			formatedString = formated_to_String();

		constant = _constant;
	}

	void clear() 
	{
		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			(*this)[iter] = 0.0f;
	}

	float operator [](const int _index) const
	{
		switch (_index)
		{
		case INDEXS::MATRIX::POPULATION:
			return population;
		case INDEXS::MATRIX::INCOME:
			return money;
		case INDEXS::MATRIX::CONSTRUCTION:
			return construction;
		case INDEXS::MATRIX::ELECTRICITY:
			return electricity;
		case INDEXS::MATRIX::WASTE:
			return waste;
		case INDEXS::MATRIX::TRAFFIC:
			return traffic;
		default:
			break;
		}
	}

	float& operator [](const int _index)
	{
		switch (_index)
		{
		case INDEXS::MATRIX::POPULATION:
			return population;
		case INDEXS::MATRIX::INCOME:
			return money;
		case INDEXS::MATRIX::CONSTRUCTION:
			return construction;
		case INDEXS::MATRIX::ELECTRICITY:
			return electricity;
		case INDEXS::MATRIX::WASTE:
			return waste;
		case INDEXS::MATRIX::TRAFFIC:
			return traffic;
		default:
			break;
		}
	}

	void operator += (const resourceMatrix& rhs)
	{
		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			(*this)[iter] += rhs[iter];
	}

	void operator -= (const resourceMatrix& rhs)
	{
		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			(*this)[iter] -= rhs[iter];
	}

	void operator *= (const resourceMatrix& rhs)
	{
		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			(*this)[iter] *= rhs[iter];
	}

	void operator *= (const float& rhs)
	{
		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			(*this)[iter] *= rhs;
	}

	resourceMatrix operator - ()
	{
		resourceMatrix out;

		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			out[iter] = -(*this)[iter];

		return out;
	}

	resourceMatrix operator + (const resourceMatrix& rhs)
	{
		resourceMatrix out;
		
		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			out[iter] = (*this)[iter] + rhs[iter];
		
		return out;
	}

	resourceMatrix operator - (const resourceMatrix& rhs)
	{
		resourceMatrix out;

		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			out[iter] = (*this)[iter] - rhs[iter];

		return out;
	}

	resourceMatrix operator / (const resourceMatrix& rhs)
	{
		resourceMatrix out;

		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			out[iter] = (*this)[iter] / rhs[iter];

		return out;
	}

	resourceMatrix operator * (const resourceMatrix& rhs)
	{
		resourceMatrix out;

		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			out[iter] = (*this)[iter] * rhs[iter];

		return out;
	}

	resourceMatrix operator * (const float rhs)
	{
		resourceMatrix out;

		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			out[iter] = (*this)[iter] * rhs;

		return out;
	}

	bool operator < (const resourceMatrix& rhs)
	{
		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			if ((*this)[iter] < rhs[iter])
				return true;

		return false;
	}

	bool operator < (const float rhs)
	{
		for (iter = RESOURCE_TYPE_NUM - 1; iter >= 0; iter--)
			if ((*this)[iter] < rhs)
				return true;

		return false;
	}

	std::string formated_to_String()
	{
		if (constant)
			return formatedString;

		std::string str = "";
		float val;

		const std::vector<std::string> ABRIVATIONS {
			"POP",
			"$$$",
			"CON",
			"POW",
			"SEW",
			"CAR"
		};

		for (iter = 0; iter < RESOURCE_TYPE_NUM; iter++)
		{
			val = (*this)[iter];

			str += ABRIVATIONS[iter];

			// Sign
			if (val > 0)
				str += '+';
			else if (val < 0)
				str += '-';
			else if (val == 0)
				str += ' ';

			if (val > 9)
				val = trunc(val);

			// Int
			if (trunc(val) == val)
			{
				str += std::to_string(std::abs((int)val));

				if (abs(val) <= 9)
					str += "   ";
				else
					str += "  ";

				if (iter % 2 == 1)
					str += '\n';
			}
			// Float (1s and 10ths)
			else
			{
				str += std::to_string(std::abs((int)val));
				str += '.';
				str += std::to_string(std::abs((int)(val * 10) % 10));

				str += ' ';

				if (iter % 2 == 1)
					str += '\n';
			}
		}

		return str;
	}

	const std::string to_String() const
	{
		return
			"  POP: " + std::to_string((*this)[0]) + '\n' +
			"  $$$: " + std::to_string((*this)[1]) + '\n' +
			"  CON: " + std::to_string((*this)[2]) + '\n' +
			"  POW: " + std::to_string((*this)[3]) + '\n' +
			"  SEW: " + std::to_string((*this)[4]) + '\n' +
			"  CAR: " + std::to_string((*this)[5]) + '\n' ;
	}

	friend std::ostream& operator << (std::ostream& _stream, const resourceMatrix& _out)
	{
		_stream << _out.to_String();
		return _stream;
	}
};

namespace MAPDATA
{

	/*
	Unbuildable
	 M - Mountain
	 W - Water
	Blocked (need to clear first)
	 T - Trees
	 R - Rocks
	Clear
	 G - Grass
	*/

	struct gameMap
	{
		std::string map;
		olc::vi2d mapSize;
		float time;
		int populationGoal;
		int maxPoints;
		resourceMatrix bonuses;
		resourceMatrix startingResources;
		resourceMatrix resourceExchangePrice;

		gameMap() = default;
		gameMap(std::string _map, olc::vi2d _mapSize,
			float _time, int _populationGoal, int _maxPoints,
			resourceMatrix _bonuses, resourceMatrix _startingResources, resourceMatrix _resourceExchangePrice)
			:
			map(_map), mapSize(_mapSize),
			time(_time), populationGoal(_populationGoal), maxPoints(_maxPoints),
			bonuses(_bonuses), startingResources(_startingResources), resourceExchangePrice(_resourceExchangePrice){}
	};

	const std::vector<gameMap> gameMaps = {
		gameMap ( // GRASS LANDS
		   {"MMMMMMMMMMMMMMMM"
			"MMTTGGTTGMMMTTMM"
			"MTTTTGGGGGGGGTMM"
			"MMTTGGGGGGGGGTMM"
			"MTTTTGGGGGGGGGTM"
			"MTTGGGGGGGGGGGGM"
			"MMTTGGGGGGGGGTMM"
			"MTTTTGGGGGGGGGTM"
			"MTTGGGGGGGGGGGGM"
			"MMTTGGGGGGGGGTMM"
			"MMTTGGGGGGGGGTMM"
			"MTTTTGGGGGGGGGTM"
			"MTTGGGGGGGGGGGGM"
			"MTTTTGGGGGGGGTMM"
			"MMTTTTMMTTTGTTMM"
		    "MMMMMMMMMMMMMMMM"},
			{ 16,16 },
			160.0f,
			250,
			2500,
			resourceMatrix{2 ,2  ,1 , 2,.8,0},
			resourceMatrix{0,400,300,100, 0,0},
			resourceMatrix{0,2,2,2,2,0}
		),
		gameMap( // FOREST
		   {"MMMTTTGGTTTM"
			"MMTTTGGGTTMM"
			"TTTTGGTTTGTM"
			"TTTTGGGTTTMM"
			"TTTTGGTTTGTT"
			"MTTTTGGTTTMM"
			"MMTTGGTGTGTM"
			"MTTTGGGTTTMT"
			"TTTTGGTGTGTT"
			"TTTTGGTTTGTM"
			"MMTTTGGGTTMM"
		    "MMMTTTGGTTTM"},
			{ 12,12 },
			160.0f,
			300,
			5000,
			resourceMatrix{2 ,2  ,1 , 2,.8,0},
			resourceMatrix{0,350,200,50, 0,0},
			resourceMatrix{0,2.5,2.5,2.5,2.5,0}
		),
		gameMap( // MOUNTAINS
		   {"MMMMMMMM"
			"MMTTTMMM"
			"MMTTGGMM"
			"MTTTTMMM"
			"TTMMMMMM"
			"TTTTMMMM"
			"MMTTTGMM"
			"MMMMMMMM"},
			{ 8,8 },
			160.0f,
			150,
			10000,
			resourceMatrix{2 ,2  ,1 , 2,.8,0},
			resourceMatrix{0,200,100,20, 40,0},
			resourceMatrix{0,4,4,4,4,0}
		),
	};
};

namespace CELLDATA 
{
	struct status 
	{
		float current, max;

		status() = default;
		status(float _current, float _max) : current{ _current }, max{ _max } {}
		status(const status& _copy) : current(_copy.current), max(_copy.max) {}

		status operator + (const status& rhs) 
		{
			return status(rhs.current + current, max);
		}

		bool operator += (const float& rhs)
		{
			if (current + rhs <= max)
			{
				if (current + rhs >= 0)
				{
					current += rhs;
					return true;
				}
				else
				{
					current = 0;
					return false;
				}
			}
			else
			{
				current = max;
				return false;
			}
		}

		bool operator -= (const float& rhs)
		{
			if (current - rhs <= max) 
			{
				if (current - rhs >= 0) 
				{
					current -= rhs;
					return true;
				}
				else
				{
					current = 0;
					return false;
				}
			}
			else
			{
				current = max;
				return false;
			}
		}

		bool operator = (const float& rhs)
		{
			if (rhs <= max)
			{
				if (rhs >= 0)
				{
					current = rhs;
					return true;
				}
				else
				{
					current = 0;
					return false;
				}
			}
			else
			{
				current = max;
				return false;
			}
		}

		// Normalized current value from 0 to 1
		float norm()
		{
			if (max)
				return current / max;
			else
				return 0;
		}

		// The inverted normal: 1 / (current / max)
		float inverse()
		{
			return 1 - norm();
		}

		// Will add to max and return any remainder
		float ceil(float _in)
		{
			if (_in > max - current) // Positive and max
			{
				current = max;
				return _in - current;
			}
			else if (_in + current < 0) // Negitive and min
			{
				current = 0;
				return _in + current;
			}

			// In range
			current += _in;
			return 0.0f;
		}

		const std::string to_string() const
		{
			if (max != 0)
				return std::to_string((int)round(current)) + '/' + std::to_string((int)round(max));
			else
				return "N/A";
		}
	};

	struct structureStatus
	{
		status quantity, quality, capacity, happyness, durrability, traffic, power;
		double deltaDurrability, deltaHappyness;

		structureStatus() = default;
		structureStatus(status _capacity, status _durrability, status _power, status _traffic, status _quantity, status _quality, status _happyness)
			: capacity{ _capacity }, durrability{ _durrability }, power{ _power }, traffic{ _traffic }, quantity{ _quantity }, quality{ _quality }, happyness{ _happyness } {}
		structureStatus(const structureStatus& _copy)
			: capacity(_copy.capacity), durrability(_copy.durrability), power(_copy.power), traffic(_copy.traffic), quantity(_copy.quantity), quality(_copy.quality), happyness(_copy.happyness) {}

		bool upgradeQuantity()
		{
			if (quantity += 1)
			{
				capacity.max += 15;
				quality.max -= 1;
				return true;
			}
			else
				return false;
		}

		bool upgradeQuality()
		{
			if (quality += 1)
			{
				quantity.max -= 1;
				return true;
			}
			else
				return false;
		}

		void updateHappiness(float elapsedTime)
		{
			happyness.ceil (
				(
					( (durrability.norm() + (power.current > 0) + traffic.inverse() ) / 3 ) *
					(1 + ( (quality.current / 5) - (quantity.current / 10) ) )
					- .5
				) * elapsedTime);
		}

		void updatePopulation(float elapsedTime)
		{
			capacity.ceil((happyness.norm() - 0.5) * elapsedTime);
		}

		const std::string to_string() const
		{
			return
				"\n Status:"
				"\n  Quantity:    " + quantity	 .to_string() +
				"\n  Quality:     " + quality	 .to_string() +
				"\n  Capacity:    " + capacity	 .to_string() +
				"\n  Happyness:   " + happyness	 .to_string() +
				"\n  Durrability: " + durrability.to_string() +
				"\n  Traffic:     " + traffic	 .to_string() +
				"\n  Power:       " + power		 .to_string();
		}

		const std::string formated_to_string()
		{
			const std::vector<std::string> STATUS_ABRIVATIONS {
				"L", // LVL
				"P", // CAP / production
				"H", // HAP
				"D", // DUR
				"C", // CAR
				"E", // POW / electricity
			};

			return
				STATUS_ABRIVATIONS[0] + ':' + (quantity + quality).to_string() + std::string(6 - (quantity + quality).to_string().size(), ' ') +
				STATUS_ABRIVATIONS[1] + ':' + capacity			  .to_string() + '\n' +
				STATUS_ABRIVATIONS[2] + ':' + happyness			  .to_string() + std::string(6 - happyness.to_string().size(), ' ') +
				STATUS_ABRIVATIONS[3] + ':' + durrability		  .to_string() + '\n' +
				STATUS_ABRIVATIONS[4] + ':' + traffic			  .to_string() + std::string(6 - traffic.to_string().size(), ' ') +
				STATUS_ABRIVATIONS[5] + ':' + power				  .to_string();
		}
	};

	const std::vector<olc::vi2d> ICON_SPRITE_POS = {
		{ INDEXS::ICONS::INDEX,	INDEXS::ICONS::BOTTOM_BAR_ICONS::QUANTITY	},
		{ INDEXS::ICONS::INDEX,	INDEXS::ICONS::BOTTOM_BAR_ICONS::QUALITY	},
		{ INDEXS::ICONS::INDEX,	INDEXS::ICONS::BOTTOM_BAR_ICONS::POLICIES	},
	};

	const std::vector<std::string> cellNames {
		"Tower",
		"Electric Plant",
		"Construction Plant",
		"Waste Plant",
		"Road",
		"Buildable",
		"Removable",
		"Unbuildable"
	};

	// The cost of each structure to initaly build, or the cost to remove a terrain feature
	const std::vector<resourceMatrix*> baseCost = {
		//					 P, $, C, E, W, T
		new resourceMatrix { 0,-5,-5, 0, 0, 0, CONSTANT}, // Tower
		new resourceMatrix {-5,-9,-9, 0, 0, 0, CONSTANT}, // Electricty Plant
		new resourceMatrix {-5,-9,-9, 0, 0, 0, CONSTANT}, // Waste Plant
		new resourceMatrix {-5,-9,-9, 0, 0, 0, CONSTANT}, // Construction Plant
		new resourceMatrix { 0,-5,-5, 0, 0, 0, CONSTANT}, // Road
		new resourceMatrix { 0, 0, 0, 0, 0, 0, CONSTANT}, // Grass
		new resourceMatrix { 0,-5,+5, 0,+5, 0, CONSTANT}, // Removable terrain
		new resourceMatrix { 0, 0, 0, 0, 0, 0, CONSTANT}  // Unbuildable terrain
	};

	// The cost of each strcture to upgrade after building
	const std::vector<resourceMatrix*> baseModifiers = {
		//					  P,  $,  C,  E,  W,  T
		new resourceMatrix {  0,+10, -2, -3, +4, +3, CONSTANT}, // Tower
		new resourceMatrix {  0, -3, -1, +7, +4,+.5, CONSTANT}, // Electricty Plant
		new resourceMatrix {  0, -3, -1, -2,-10, +1, CONSTANT}, // Waste Plant
		new resourceMatrix {  0, -3, +5, -2, +6, +2, CONSTANT}, // Construction Plant
		new resourceMatrix {  0,-.1,-.1,-.1, -1, -4, CONSTANT}, // Road
		new resourceMatrix {  0,  0,  0,  0,  0,  0, CONSTANT}, // Buildable terrain
		new resourceMatrix {  0,  0,  0,  0,  0,  0, CONSTANT}, // Removable terrain
		new resourceMatrix {  0,  0,  0,  0,  0,  0, CONSTANT}  // Unbuildable terrain
	};

	const std::vector<structureStatus> baseStatus = {
						     // Capacity        Durrability       power           traffic          quality            quantity        happyness
		structureStatus{{ 0.0f, 15.0f }, { 10.0f, 10.0f }, { 0.0f,  1.0f }, { 0.0f, 15.0f }, { 0.0f, 10.0f }, { 0.0f, 10.0f }, { 5.0f, 10.0f }}, // Tower
		structureStatus{{ 0.0f, 10.0f }, { 12.5f, 12.5f }, { 0.0f, 10.0f }, { 0.0f,  5.0f }, { 0.0f, 10.0f }, { 0.0f, 10.0f }, { 5.0f, 10.0f }}, // Electricity plant
		structureStatus{{ 0.0f, 10.0f }, { 10.0f, 10.0f }, { 0.0f,  2.5f }, { 0.0f, 10.0f }, { 0.0f, 10.0f }, { 0.0f, 10.0f }, { 5.0f, 10.0f }}, // Waste plant
		structureStatus{{ 0.0f, 10.0f }, { 15.0f, 15.0f }, { 0.0f,  2.5f }, { 0.0f, 10.0f }, { 0.0f, 10.0f }, { 0.0f, 10.0f }, { 5.0f, 10.0f }}, // Construction plant
		structureStatus{{ 0.0f,  0.0f }, { 20.0f, 20.0f }, { 0.0f,  0.5f }, { 0.0f, 25.0f }, { 0.0f,  0.0f }, { 0.0f,  0.0f }, { 0.0f, 0.0f }}  // Road
	};

	const float upgradeMultiplier = 1.5f;

	const resourceMatrix quantityUpgradeMultiplier = resourceMatrix {
		1.0f, upgradeMultiplier, upgradeMultiplier, upgradeMultiplier, upgradeMultiplier, upgradeMultiplier, CONSTANT
	};

	const resourceMatrix qualityUpgradeMultiplier = resourceMatrix{
		1.0f, upgradeMultiplier, upgradeMultiplier, upgradeMultiplier, upgradeMultiplier, upgradeMultiplier, CONSTANT
	};

	// The build discription of each structure 
	const std::vector<std::string> buildingBuildText = {
	// TOWER
		"A residential to\n"
		"wer. Holds new r\n"
		"esidents and gen\n"
		"erates profits  ",
	// Electricity plant
		"A power plant th\n"
		"at will provide \n"
		"your city with n\n"
		"eeded electricity",
	// Waste plant
		"Will remove wast\n"
		"e produced by hu\n"
		"man activity. Wh\n"
		"ere does it go?",
	// Construction plant
		"Produces the raw\n"
		"materials needed\n"
		"to expand and ma\n"
		"intain your city",
	// Road
		"Guides traffic f\n"
		"rom place to pla\n"
		"ce. Must be next\n"
		"to a building!",
	};

};

int clamp(int val, int min, int max, bool wrap = false)
{
	if (val < min)
		return wrap ? max : min;
	else if (val > max)
		return wrap ? min : max;
	else
		return val;
}

class Infrastructure : public olc::PixelGameEngine
{
public:
	Infrastructure()
	{
		sAppName = "FBLA: Infrastructure!";
	}

	/* ==== TIMER MANAGER ==== */

	struct counter
	{
		float current, max, inc;

		counter() = default;
		counter(float _max, float _incrament)
			: current(0.0f), max(_max), inc(_incrament) {}

		virtual bool incrament()
		{
			current += inc;
			if (current >= max)
			{
				current = 0;
				return true;
			}
			else
				return false;
		}
		
		virtual bool incrament(float _inc)
		{
			current += _inc;

			if ((max > 0 && current >= max) || (max < 0 && current <= max))
			{
				current = 0;
				return true;
			}
			else
				return false;
		}

		virtual const std::string to_string() const
		{
			return std::to_string(current) + "/" + std::to_string(max);
		}
	};

	struct timer : public counter
	{
	private:
		typedef counter super;

	public:
		bool autoReset;

		timer() = default;
		timer(float _maxTime, bool _autoReset = false)
			: super(_maxTime, 0.0f), autoReset(_autoReset) {}

		bool incrament(float elapsedTime) override
		{
			if (current < max)
			{
				current += elapsedTime;
				return false;
			}
			else
			{
				if (autoReset)
					reset();

				return true;
			}
		}

		void reset() 
		{
			current = 0.0f;
		}
	};

	/* ==== UI AND FONT ==== */
    
	struct panel
	{
		std::string name;
		olc::vi2d pos;
		olc::vi2d size;
		olc::Pixel color;
		bool enabled;
		bool drawRequest;

		std::vector<panel*> elements;

		panel() {}

		panel(std::string _name, olc::vi2d _pos, olc::vi2d _size, olc::Pixel _color, bool _enabled)
			: name(_name), pos(_pos), size(_size), color(_color), enabled(_enabled), drawRequest(_enabled) {}

		virtual void drawSelf(Infrastructure* game)
		{
			if (enabled)
			{
				game->FillRectDecal(pos, size, color);

				drawElements(game);
			}
		}

		void drawElements(Infrastructure* game)
		{
			for (panel* p : elements)
			{
				p->drawSelf(game);
			}
		}
	};

		struct image : panel
		{
		private:
			typedef panel super;

		public:
			olc::Decal* decal;
			ASSETS::spriteSheet* sheet;
			olc::vi2d spritePos;
			olc::vf2d scale;
			olc::Pixel imageColor;
			bool cullTransparent;

			image(std::string _name, olc::vi2d _pos, olc::vi2d _size, olc::Pixel _color, bool _enabled, olc::vf2d _scale, olc::Pixel _imageColor = olc::BLACK, bool _cullTransparent = false)
				: super(_name, _pos, _size, _color, _enabled), scale(_scale), cullTransparent(_cullTransparent), imageColor(_imageColor)
			{
				sheet = nullptr;
				spritePos = {-1,-1};
			}

			void setDecal(ASSETS::spriteSheet* _sheet, olc::vi2d _spritePos)
			{
				sheet = _sheet;
				spritePos = _spritePos;
			}

			void setDecal(olc::Decal* _decal)
			{
				decal = std::move(_decal);
			}

			void drawSelf(Infrastructure* game) override
			{
				if (enabled)
				{
					//game->FillRectDecal(pos, size, color);
					drawElements(game);

					if (cullTransparent)
						game->SetPixelMode(olc::Pixel::MASK);

					if(sheet && spritePos.x > -1 && spritePos.x < sheet->rows && spritePos.y > -1 && spritePos.y < sheet->columns)
						game->DrawDecal(pos, sheet->decals[spritePos.x][spritePos.y].get(), scale, imageColor);
					else if (decal)
					{
						game->DrawDecal(pos, decal, scale, imageColor);
					}
					else
						std::cout << "Error: invalid image corrdinates or invalid sprite sheet!" << std::endl;

					if (cullTransparent)
						game->SetPixelMode(olc::Pixel::NORMAL);
				}
			}
		};

		struct label : panel
		{
		private:
			typedef panel super;
			int iter, _iter, charSinceLineBreak;
			olc::vf2d drawPos;

		public:
			std::string content;
			int contentLength;
			olc::Pixel textColor;
			olc::vf2d textSize;
			bool cullTransparent;

			label(std::string _name, olc::vi2d _pos, olc::vi2d _size, olc::Pixel _color, bool _enabled, std::string _content, olc::Pixel _textColor = olc::BLACK, olc::vf2d _textSize = { 1.0f, 1.0f }, int _contentLength = -1, bool _cullTransparent = true)
				: super(_name, _pos, _size, _color, _enabled), content(_content), contentLength(_contentLength), textColor(_textColor), textSize(_textSize), cullTransparent(_cullTransparent) {}

			void drawChar(Infrastructure* game, char c, olc::vi2d pos, olc::vf2d size = { 1.0f, 1.0f }, olc::Pixel color = olc::BLACK)
			{
				const int offset = ' '; // ASCII character #32, the start of the game's font

				if (c - offset > 96 || c - offset < 0)
				{
					std::cout << "ERROR: could not draw character: " << c << " (Content: " << content << ")" << std::endl;
					return;
				}

				game->DrawDecal(pos, ASSETS::alphaChars.decals[(c - offset) / ASSETS::alphaChars.columns][(c - offset) % ASSETS::alphaChars.rows].get(), size, color);
			}

			void drawLabel(Infrastructure* game, std::string content)
			{
				drawPos = olc::vf2d(0.0f, 0.0f);

				for (iter = 0; iter < content.size(); iter++)
				{
					switch (content[iter])
					{
					case '\n':
						drawPos.y += textSize.y;
						drawPos.x = 0.0f;
						charSinceLineBreak = 0;
						break;
					case '\t':
						for (_iter = TabSize - (charSinceLineBreak % TabSize) - 1; _iter >= 0; _iter--)
						{
							drawChar(game, ' ', pos + drawPos * CharSize, textSize, textColor);
							drawPos.x += textSize.x;
							charSinceLineBreak++;
						}
						break;
					default:
						drawChar(game, content[iter], pos + drawPos * CharSize, textSize, textColor);
						drawPos.x += textSize.x;
						charSinceLineBreak++;
						break;
					}
				}
			}

			// Draws underlying pannel first, then iterates through the content characters
			void drawSelf(Infrastructure* game) override
			{
				if (enabled)
				{
					game->FillRectDecal(pos, size, color);
					drawElements(game);

					drawLabel(game, content);
				}
			}

			void setAsString(std::string str)
			{
				content = str;
				contentLength = str.size();
			}

			void setAsNumber(int num)
			{
				if (contentLength != -1)
				{
					if (num == 0)
					{
						content = std::string(contentLength, '0');
						return;
					}
					else if (contentLength - 1 - ((int)trunc(log10(num))) >= 0)
						content = std::string(contentLength - 1 - trunc(log10(num)), '0') + std::to_string(num);
				}
				else
					std::cout << "Error: Attempt to assign a non-numerical label a numerical value!" << std::endl;
			}
		};

        struct video : panel
        {
        private:
            typedef panel super;
            
        public:
			std::vector<olc::Decal*>* frames;
    	    timer frameTimer;
    	    double framerate;
    	    int currentFrame, maxFrame;
    	    
    	    olc::vf2d scale;
    	    olc::Pixel tint;
    	    bool cullTransparent;
    	    
    	    video() = default;
    	    video(std::string _name, olc::vi2d _pos, olc::vi2d _size, olc::Pixel _color, bool _enabled,
    	        double _framerate, std::vector<olc::Decal*>* _frames, bool _cullTransparent, olc::vf2d _scale = {1.0f, 1.0f}, olc::Pixel _tint = olc::WHITE)
    	         : super(_name, _pos, _size, _color, _enabled),
    	            frames(_frames), frameTimer(timer(_framerate / 60, true)), framerate(_framerate), currentFrame(1), maxFrame(_frames->size()),
    	            scale(_scale), tint(_tint), cullTransparent(_cullTransparent) {}
    	    
    	    bool update(float _elapsedTime)
    	    {
    	        if(enabled && frameTimer.incrament(_elapsedTime))
    	        {
	                currentFrame++;
	                if(currentFrame >= maxFrame)
	                {
	                    currentFrame %= maxFrame;
	                    return true;
	                }
    	        }
    	        return false;
    	    }
    	    
    	    void reset()
    	    {
    	        frameTimer.reset();
    	        currentFrame = 1;
    	    }
    	    
    	    void drawSelf(Infrastructure* game) override
    	    {
    	        if(enabled)
                {
					//game->FillRectDecal(pos, size, color);
					drawElements(game);

					if (frames && currentFrame < maxFrame)
					{
						if (cullTransparent)
							game->SetPixelMode(olc::Pixel::MASK);

						game->DrawDecal(pos, frames->at(currentFrame), scale, tint);

						if (cullTransparent)
							game->SetPixelMode(olc::Pixel::NORMAL);
					}
					else
						std::cout << "Error: invalid image corrdinates or invalid sprite sheet!" << std::endl;
                }
    	    }
        };
        
    struct screen
    {
		Infrastructure* game;
        std::vector<panel*> content;
        bool enabled;
        
        virtual void setEnable(bool in)
        {
            if(in != enabled)
            {
                for(panel* p : content)
                    p->enabled = in;
                enabled = in;
            }
        }
        
        virtual void init(){}
        
        virtual void update(Infrastructure* _game, float elapsedTime){}
        
        virtual void end(){}
    };
        
        struct menuScreen : public screen
        {
			const static int GAMEPLAY_VID = 0;
			const static int SCORE_BOARD = 1;

            video* gamePlayVideo;
            timer scoreScreenTimer;
			timer startTextBlink;
            int mode;
    	    
			menuScreen(Infrastructure* _game)
			{
				content.push_back(_game->UI[INDEXS::SCREENS::MAIN::INDEX]);
				content.push_back(_game->UI[INDEXS::SCREENS::SCORE::INDEX]);
				content[GAMEPLAY_VID]->elements[3]->enabled = true;
				content[SCORE_BOARD]->elements[0]->enabled = true;
				mode = GAMEPLAY_VID;
			}

            void init() override
            {
        		gamePlayVideo = (video*)(content[0]->elements[INDEXS::SCREENS::MAIN::VIDEO]);
        		gamePlayVideo->reset();
				transition(GAMEPLAY_VID);
				scoreScreenTimer = timer(5);
				startTextBlink = timer(1, true);
            }
            
            void update(Infrastructure* _game, float elapsedTime) override
            {
				game = _game;

				// Play game
				if (game->GetKey(olc::ENTER).bPressed)
					game->switchScreen(INDEXS::SCREENS::MAP::INDEX);

				// Force transition
				if (game->GetKey(olc::SHIFT).bHeld && game->GetKey(olc::C).bPressed)
					transition(mode == GAMEPLAY_VID ? SCORE_BOARD : GAMEPLAY_VID);

				// Update video, transition when video ends
				if (mode == GAMEPLAY_VID)
				{
					if (startTextBlink.incrament(elapsedTime))
					{
						label* enter = (label*)content[GAMEPLAY_VID]->elements[INDEXS::SCREENS::MAIN::ENTER];
						enter->textColor	= enter->textColor	== olc::WHITE ? olc::Pixel(0, 0, 0, 0) : olc::WHITE;
						enter->color		= enter->color		== olc::BLACK ? olc::Pixel(0, 0, 0, 0) : olc::BLACK;
					}

					if(gamePlayVideo->update(elapsedTime))
						transition(SCORE_BOARD);
				}

				// Update score board timer, transition when it ends
				else if(mode == SCORE_BOARD && scoreScreenTimer.incrament(elapsedTime))
				    transition(GAMEPLAY_VID);
                
        		// = = = = = DRAW UI = = = = = = //
				
				// Draw all UI
        		for (panel* p : content)
        			p->drawSelf(game);
            }
            
            void end() override
            {
				setEnable(false);
            }
            
            void transition(int _mode)
            {
                scoreScreenTimer.reset();
				startTextBlink.reset();
                gamePlayVideo->reset();
                mode = _mode;
				content[GAMEPLAY_VID]->enabled = _mode == GAMEPLAY_VID;
				content[SCORE_BOARD] ->enabled = _mode == SCORE_BOARD;
            }
            
        };
        
        struct gameScreen : public screen
        {
			/* ==== SPRITE STRUCT ==== */

			struct spriteManager
			{
				olc::vi2d assetPos;
				olc::vi2d assetPosOffset;
				olc::vi2d pos;
				olc::vi2d screenCellPos;
				float rotation;

				timer* frameTimer;
				counter frameCounter;
				bool animated;

				spriteManager() = default;
				spriteManager(olc::vi2d _assetPos, olc::vi2d _pos, float _rotation, float _frameDelay = 0.0f, int _maxFrame = 0, timer* _premadeTimer = nullptr)
					: assetPos(_assetPos), assetPosOffset({ 0,0 }), pos(_pos), rotation(_rotation), frameCounter(_maxFrame, 1.0f)
				{
					if (_frameDelay == 0.0f || _maxFrame == 0)
						animated = false;
					else
					{
						animated = true;

						if (_premadeTimer)
							frameTimer = _premadeTimer;
						else
							frameTimer = new timer(_frameDelay, true);
					}
				}

				void updateStatus(olc::vi2d _assetPosOffset = { -1,-1 }, float _frameDelay = 0.0f, int _maxFrame = 0, timer* _premadeTimer = nullptr)
				{
					if (_assetPosOffset > -1)
						assetPosOffset = _assetPosOffset;

					if (_frameDelay == 0.0f || _maxFrame == 0)
						animated = false;
					else
					{
						animated = true;
						frameCounter = counter(_maxFrame, 1.0f);

						if (_premadeTimer)
							frameTimer = _premadeTimer;
						else
							frameTimer = new timer(_frameDelay, true);
					}
				}

				void drawSelf(gameScreen* screen, float elapsedTime = 0.0f)
				{
					if (onScreen(screen))
					{
						if (animated && frameTimer->incrament(elapsedTime))
							frameCounter.incrament();

						screen->game->DrawRotatedDecal(
							screenCellPos * CellSize + CellCenter,
							ASSETS::infraAssets.decals
							[assetPos.x + assetPosOffset.x]
						[assetPos.y + assetPosOffset.y + (int)(frameCounter.current)].get(),
							rotation, CellCenter);
					}
				}

				bool onScreen(gameScreen* screen)
				{
					screenCellPos = pos - (screen->cameraPos - screen->cameraOffset);
					return screenCellPos.x >= 0 && screenCellPos.x < ScreenCellWidth&& screenCellPos.y >= 1 && screenCellPos.y < ScreenCellHeight - 1;
				}

				const std::string to_string() const
				{
					return
						"\n Manager"
						"\n  Asset:    " + assetPos.str() +
						"\n  Position: " + pos.str() +
						"\n  Cell Pos: " + screenCellPos.str() +
						"\n  Rotation: " + std::to_string(round(rotation)) +
						"\n  Animated: " + std::to_string(animated) +
						(animated ? "\n  Frame: " + frameCounter.to_string() : "");
				}
			};

			/* ==== CELL STRUCTS ==== */

			struct cell
			{
			public:
				int cellID;

				resourceMatrix* buildCost;
				std::string buildCostFormated;
				resourceMatrix* deltaResources;
				std::string deltaResorcesFormated;

				spriteManager sprite;

				cell() {}

				cell(INDEXS::aID _cellInfo, spriteManager _sprite) :
					cellID(_cellInfo.ID), sprite(_sprite),
					buildCost(CELLDATA::baseCost[_cellInfo.ID]),
					buildCostFormated(CELLDATA::baseCost[_cellInfo.ID]->formated_to_String()),
					deltaResources(CELLDATA::baseModifiers[_cellInfo.ID]),
					deltaResorcesFormated(CELLDATA::baseModifiers[_cellInfo.ID]->formated_to_String()) {}

				virtual void update(gameScreen* screen, float elapsedTime = 0.0f) {}

				virtual const std::string to_string() const
				{
					return CELLDATA::cellNames[cellID] + ":" + sprite.to_string() + '\n';
				}

				friend std::ostream& operator << (std::ostream& _stream, const cell& _out)
				{
					_stream << _out.to_string();
					return _stream;
				}
			};

				struct terrain : cell
				{
				private:
					// Gives access to the structure template constructor
					typedef cell super;

				public:
					bool owned;

					terrain() {}

					terrain(int _terrainID, olc::vi2d _pos, float _rotation) :
						super(INDEXS::SPRITES[_terrainID], spriteManager(INDEXS::SPRITES[_terrainID].pos, _pos, _rotation)) {}

					void update(gameScreen* screen, float elapsedTime = 0.0f) override
					{
						sprite.drawSelf(screen, elapsedTime);
					}
				};

				struct structure : cell
				{
				private:
					// Gives access to the structure template constructor
					typedef cell super;

				public:
					CELLDATA::structureStatus status;

					std::vector<cell*> surroundingRoads;

					float upgradeMultiplier;
					bool abandoned;

					structure() {}

					structure(int _structureID, olc::vi2d _pos, float _rotation = 2.0f * PI, float _frameDelay = 0.0f, int _maxFrames = 0, timer* _premadeTimer = nullptr)
						: super(INDEXS::SPRITES[_structureID], spriteManager(INDEXS::SPRITES[_structureID].pos, _pos, _rotation, _frameDelay, _maxFrames, _premadeTimer)),
						status(CELLDATA::baseStatus[_structureID]), upgradeMultiplier(CELLDATA::upgradeMultiplier), abandoned(false) {}

					void update(gameScreen* screen, float elapsedTime = 0.0f) override
					{
						float sewMulti = screen->resources[INDEXS::MATRIX::WASTE] >= 0.0f ? (log10(screen->resources[INDEXS::MATRIX::WASTE] + 1) / 4 + 0.5f) : 1.0f;

						switch (cellID)
						{
						case INDEXS::STRUCTURES::TOWER:
							// Update status
							status.updateHappiness(elapsedTime);
							status.updatePopulation(elapsedTime);
							
							// Update delta-resources
							deltaResources = new resourceMatrix(*CELLDATA::baseModifiers[cellID]);

							if(status.quantity.current > 0)
								*deltaResources *= upgradeMultiplier * status.quantity.current;
							*deltaResources *= status.capacity.norm();
							(*deltaResources)[INDEXS::MATRIX::INCOME] *= 1 / sewMulti;
							(*deltaResources)[INDEXS::MATRIX::INCOME] *= status.happyness.norm();
							
							// Calculate traffic
							screen->calculateTraffic(this, elapsedTime);
							break;

						case INDEXS::STRUCTURES::EPLANT:
							// Update status
							status.updateHappiness(elapsedTime);

							// Update delta-resources
							deltaResources = new resourceMatrix(*CELLDATA::baseModifiers[cellID]);
							if (status.quantity.current > 0) { (*deltaResources) *= upgradeMultiplier * status.quantity.current; }
							(*deltaResources)[INDEXS::MATRIX::ELECTRICITY] *= 1 / sewMulti;
							(*deltaResources)[INDEXS::MATRIX::ELECTRICITY] *= status.happyness.norm();

							screen->calculateTraffic(this, elapsedTime);
							break;

						case INDEXS::STRUCTURES::CPLANT:
							// Update status
							status.updateHappiness(elapsedTime);

							// Update delta-resources
							deltaResources = new resourceMatrix(*CELLDATA::baseModifiers[cellID]);
							if (status.quantity.current > 0) { (*deltaResources) *= upgradeMultiplier * status.quantity.current; }
							(*deltaResources)[INDEXS::MATRIX::CONSTRUCTION] *= 1 / sewMulti;
							(*deltaResources)[INDEXS::MATRIX::CONSTRUCTION] *= status.happyness.norm();

							screen->calculateTraffic(this, elapsedTime);
							break;

						case INDEXS::STRUCTURES::WPLANT:
							// Update status
							status.updateHappiness(elapsedTime);

							// Update delta-resources
							deltaResources = new resourceMatrix(*CELLDATA::baseModifiers[cellID]);
							if (status.quantity.current > 0) { (*deltaResources) *= upgradeMultiplier * status.quantity.current; }
							(*deltaResources)[INDEXS::MATRIX::WASTE] *= status.happyness.norm();

							screen->calculateTraffic(this, elapsedTime);
							break;

						case INDEXS::STRUCTURES::ROAD:
							// Update traffic
							screen->calculateTraffic(this, elapsedTime);

							if (sprite.animated == false && status.traffic.norm() >= 0.2f)
								sprite.updateStatus(olc::vi2d{ 0,1 }, 1.0f, 2);
							else if (sprite.animated == true && status.traffic.norm() < 0.2f)
								sprite.updateStatus(olc::vi2d{ 0,0 });
							break;

						default:
							std::cout << "UNREACHABLE: " << cellID << std::endl;
							break;
						}

						// Calculate traffic and happyness and stuff
						sprite.drawSelf(screen, elapsedTime);
					}

					const std::string to_string() const override
					{
						return CELLDATA::cellNames[cellID] + ":\n Surrounding Roads: " + std::to_string(surroundingRoads.size()) + sprite.to_string() + status.to_string() + '\n';
					}

					friend std::ostream& operator << (std::ostream& _stream, const structure& _out)
					{
						_stream << _out.to_string();
						return _stream;
					}
				};

            /* ==== MAP INFO ==== */
			int currentMap = 0;
			int endGameState = 0;
        	MAPDATA::gameMap mapData;
        	std::vector<cell*> map;
        	cell* selectedStructure;
        	int selectedPos;
        	
            /* ==== CAMERA INFO ==== */
        	olc::vf2d cameraPos; // centered
        	olc::vf2d cameraOffset; // Subtract to shift the draw pos to the top left of view
            
	        /* ==== RESOURCE INFO ==== */
        	resourceMatrix resources;
        	resourceMatrix deltaResources;
            
        	/* ==== BOTTOM BAR INFO ==== */
        	std::vector<int> selectedBottomBarOption { 0, 0, 0 };
        	int maxBottomBarOption, bottomBarMode;
	        
            /* ==== CURSOR INFO ==== */
        	double cursorFrameDelay = 1.0f;
        	timer cursorTimer = timer(cursorFrameDelay, true);
        	int cursorFrame = 0;
            
            /* ==== INPUT INfO ==== */
        	float holdFor = 0.75f;
        	timer timerA = timer(holdFor), timerD = timer(holdFor), timerW = timer(holdFor), timerS = timer(holdFor);
            
            /* ==== TIMING INFO ==== */
        	double resourceUpdateDelay = 1.0f;
        	timer resourceDelay = timer(resourceUpdateDelay, true);
        	timer* roadTimer;
        	int iter;
        	
		public:

			gameScreen(Infrastructure* _game)
			{
				content.push_back(_game->UI[INDEXS::SCREENS::MAP::INDEX]);
			}

    	    void init() override
    	    {
        		loadMap(currentMap);
        		cameraPos = mapData.mapSize / 2;
        		cameraOffset = { (float)round(ScreenCellWidth / 2), (float)round(ScreenCellHeight / 2) };
        		selectedPos = cameraPos.x + cameraPos.y * mapData.mapSize.x;
        		selectedStructure = map[selectedPos];
        
        		roadTimer = new timer(1.0f, true);
        
        		bottomBarMode = INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::BUILD;
        		maxBottomBarOption = INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::BUILD_MAX;
        		setBottomBarAlt(bottomBarMode, selectedBottomBarOption[bottomBarMode]);

				setEnable(true);
    	    }
    	    
            void update(Infrastructure* _game, float fElapsedTime) override
            {
				game = _game;

				// = = = = END GAME = = = = = //

				if(!endGameState)
					endGameState = checkEndGame();

				if (endGameState == 1)
				{
					if (game->GetKey(olc::ENTER).bPressed)
					{
						game->switchScreen(INDEXS::SCREENS::SCORE::INDEX);
					}
				}
				else if (endGameState == 2)
				{
					if (game->GetKey(olc::ENTER).bPressed)
					{
						{
							if (currentMap != 2)
							{
								content[0]->elements[INDEXS::SCREENS::MAP::CELIBRATORY::INDEX]->enabled = false;
								loadNextMap();
							}
							else
								game->switchScreen(INDEXS::SCREENS::SCORE::INDEX);
						}
					}
				}
				else
				{
					/* = = = = INPUT = = = = */

					if (game->GetKey(olc::C).bPressed && game->GetKey(olc::SHIFT).bHeld)
						resources[INDEXS::MATRIX::POPULATION] += mapData.populationGoal;


					// Move cursor
					if (game->GetKey(olc::Key::A).bPressed || game->GetKey(olc::LEFT).bPressed)
						moveCamera(-1.0f, 0);
					else if (game->GetKey(olc::Key::D).bPressed || game->GetKey(olc::RIGHT).bPressed)
						moveCamera(1.0f, 0);

					if (game->GetKey(olc::Key::A).bHeld || game->GetKey(olc::LEFT).bHeld)
					{
						if (timerW.incrament(fElapsedTime))
							moveCamera(-10.0f * fElapsedTime, 0);
						timerS.reset();
					}
					else if (game->GetKey(olc::Key::D).bHeld || game->GetKey(olc::RIGHT).bHeld)
					{
						if (timerS.incrament(fElapsedTime))
							moveCamera(10.0f * fElapsedTime, 0);
						timerW.reset();
					}
					else
					{
						cameraPos.x = trunc(cameraPos.x);
						timerW.reset();
						timerS.reset();
					}

					if (game->GetKey(olc::Key::W).bPressed || game->GetKey(olc::UP).bPressed)
						moveCamera(0, -1.0f);
					else if (game->GetKey(olc::Key::S).bPressed || game->GetKey(olc::DOWN).bPressed)
						moveCamera(0, 1.0f);

					if (game->GetKey(olc::Key::W).bHeld || game->GetKey(olc::UP).bHeld)
					{
						if (timerA.incrament(fElapsedTime))
							moveCamera(0, -10.0f * fElapsedTime);
						timerD.reset();
					}
					else if (game->GetKey(olc::Key::S).bHeld || game->GetKey(olc::DOWN).bHeld)
					{
						if (timerD.incrament(fElapsedTime))
							moveCamera(0, 10.0f * fElapsedTime);
						timerA.reset();
					}
					else
					{
						cameraPos.y = trunc(cameraPos.y);
						timerA.reset();
						timerD.reset();
					}

					// Bottom bar controls
					if (game->GetMouse(olc::Mouse::RIGHT).bReleased)
					{
						selectedBottomBarOption[bottomBarMode] = (selectedBottomBarOption[bottomBarMode] + 1) % maxBottomBarOption;
						setBottomBarAlt(bottomBarMode, selectedBottomBarOption[bottomBarMode], selectedStructure);
					}
					else if (game->GetMouse(olc::Mouse::LEFT).bReleased)
					{
						if (bottomBarMode == INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::BUILD)
							buildStructure(selectedBottomBarOption[bottomBarMode], selectedStructure);
						else if (bottomBarMode == INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::UPGRADE)
							upgradeStructure(selectedBottomBarOption[bottomBarMode], selectedStructure);
						else
							std::cout << "ERROR: Invalid bottom bar mode!" << std::endl;

						setBottomBarAlt(bottomBarMode, selectedBottomBarOption[bottomBarMode], selectedStructure);
					}

					// = = = = UPDATE TOP = = = = //

					if (resourceDelay.incrament(fElapsedTime))
					{
						float popTotal = 0.0f;

						for (cell* c : map)
						{
							if (c->cellID == INDEXS::STRUCTURES::TOWER)
							{
								popTotal += ((structure*)c)->status.capacity.current;
							}

							if (c->cellID <= INDEXS::STRUCTURES::STRUCTURE_ID_END)
								deltaResources += *c->deltaResources;
						}

						resources[INDEXS::MATRIX::POPULATION] = popTotal;

						if (!changeResources(deltaResources))
							std::cout << "End game or trade other resources" << std::endl;

						setBottomBarAlt(bottomBarMode, selectedBottomBarOption[bottomBarMode], selectedStructure);

						deltaResources.clear();
					}

					for (iter = 0; iter <= 4; iter++)
						((label*)(content[0]->elements[INDEXS::SCREENS::MAP::TOP_BAR::INDEX]->elements[iter * 2 + 1]))->setAsNumber(resources[iter]);

					mapData.time -= fElapsedTime;
					((label*)(content[0]->elements[INDEXS::SCREENS::MAP::TOP_BAR::INDEX]->elements[INDEXS::SCREENS::MAP::TOP_BAR::ELEMENTS::TIME_LABEL]))->setAsNumber(mapData.time);

				}

        		// = = = = = DRAW = = = = = = //
        
        		for (cell* c : map)
        		{
        			c->update(this, fElapsedTime);
        		}
                
				if (cursorTimer.incrament(fElapsedTime))
					cursorFrame = (cursorFrame + 1) % 2;

				game->DrawDecal(ScreenCenter * CellSize, ASSETS::infraAssets.decals[ASSETS::infraAssets.rows - 1][cursorFrame].get());

        		// = = = = = DRAW UI = = = = = = //
        
        		for (panel* p : content)
        		{
        			p->drawSelf(game);
        		}
            }
            
            void end() override
            {
				setEnable(false);
            }
            
        private:
                
            /* ==== MANAGE GAME ==== */
        	/*
        
        	Unbuildable
        	 M - Mountain
        	 W - Water
        	Blocked (need to clear first)
        	 T - Trees
        	 R - Rocks
        	Clear
        	 G - Grass
        
        */
            void loadMap(int mapNum)
            {
            	map.clear();
            	mapData = MAPDATA::gameMaps[mapNum];
				resources = mapData.startingResources;
				deltaResources = resourceMatrix();
            
            	for (int y = 0; y < mapData.mapSize.y; y++)
            	{
            		for (int x = 0; x < mapData.mapSize.x; x++)
            		{
            			cell* newCell;
            
            			switch (mapData.map[mapData.mapSize.y * y + x])
            			{
            			case 'M':
            				newCell = new terrain(INDEXS::TERRAIN::MOUNTAINS, { x, y }, (rand() % 2) * PI);
            				break;
            			case 'W':
            				newCell = new terrain(INDEXS::TERRAIN::WATER, { x, y }, (rand() % 4) / 2.0f * PI);
            				break;
            			case 'T':
            				newCell = new terrain(INDEXS::TERRAIN::FOREST, { x, y }, (rand() % 4) / 2.0f * PI);
            				break;
            			case 'R':
            				newCell = new terrain(INDEXS::TERRAIN::ROCKS, { x, y }, (rand() % 4) / 2.0f * PI);
            				break;
            			case 'G':
            				newCell = new terrain(INDEXS::TERRAIN::GRASS, { x, y }, (rand() % 4) / 2.0f * PI);
            				break;
            			default:
            				break;
            			}
            
            			map.push_back(newCell);
            		}
            	}
            }
            
			void loadNextMap()
			{
				((scoreScreen*)game->screens[INDEXS::SCREENS::SCORE::INDEX])->totalScore
					+= mapData.maxPoints * (resources[INDEXS::MATRIX::POPULATION] / mapData.populationGoal);

				currentMap++;
				loadMap(currentMap);

				cameraPos = mapData.mapSize / 2;
				cameraOffset = { (float)round(ScreenCellWidth / 2), (float)round(ScreenCellHeight / 2) };
				selectedPos = cameraPos.x + cameraPos.y * mapData.mapSize.x;
				selectedStructure = map[selectedPos];

				bottomBarMode = INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::BUILD;
				maxBottomBarOption = INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::BUILD_MAX;
				setBottomBarAlt(bottomBarMode, selectedBottomBarOption[bottomBarMode]);

				endGameState = 0;
			}

        	int checkEndGame()
        	{
        		const int NONE = 0, END = 1, WIN = 2;
				
				if (mapData.time <= 0.0f || resources[INDEXS::MATRIX::INCOME] < 0.0f)
				{
					content[0]->elements[INDEXS::SCREENS::MAP::CELIBRATORY::INDEX]->enabled = true;
					((label*)content[0]->elements[INDEXS::SCREENS::MAP::CELIBRATORY::INDEX]->elements[0])->content = INDEXS::SCREENS::MAP::CELIBRATORY::FAIL;
					return END;
				}
				else if (resources[INDEXS::MATRIX::POPULATION] >= mapData.populationGoal)
				{
					content[0]->elements[INDEXS::SCREENS::MAP::CELIBRATORY::INDEX]->enabled = true;
					((label*)content[0]->elements[INDEXS::SCREENS::MAP::CELIBRATORY::INDEX]->elements[0])->content = INDEXS::SCREENS::MAP::CELIBRATORY::WIN;
					return WIN;
				}
        
        		return NONE;
        	}
            
            /* ==== MANAGE UI ==== */

        	void setBottomBar(olc::vi2d iconPos, std::vector<int> delineators, std::vector<std::string> bodies)
        	{
        		std::vector<panel*>* elements = &content[0]->elements[INDEXS::SCREENS::MAP::BOTTOM_BAR::INDEX]->elements;

        		((image*)elements->at(INDEXS::SCREENS::MAP::BOTTOM_BAR::ELEMENTS::ICON))->setDecal(&ASSETS::infraAssets, iconPos);
        
        		if (delineators.size() == 3)
        			for (int i = 0; i < delineators.size(); i++)
        			{
        				if (delineators[i] != INDEXS::ICONS::DELINEATORS::BLANK && delineators[i] <= 3)
        				{
        					((image*)elements->at(INDEXS::SCREENS::MAP::BOTTOM_BAR::ELEMENTS::DELINEATOR_1 + i * 2))->setDecal( &ASSETS::infraAssets, { INDEXS::ICONS::INDEX, INDEXS::ICONS::DELINEATORS::DELINEATOR + delineators[i] - 1 });
        				}
        				else
        					((image*)elements->at(INDEXS::SCREENS::MAP::BOTTOM_BAR::ELEMENTS::DELINEATOR_1 + i * 2))->enabled = false;
        			}
        		else
        			std::cout << "SetBottomBar Error: diliniator ID vector size != 3!" << std::endl;
				
        		for (int i = 0; i < 3; i++)
        		{
        			if (bodies.size() > i && bodies[i] != "")
        				((label*)elements->at(INDEXS::SCREENS::MAP::BOTTOM_BAR::ELEMENTS::BODY_1 + i * 2))->content = bodies[i];
        			else
        				((label*)elements->at(INDEXS::SCREENS::MAP::BOTTOM_BAR::ELEMENTS::BODY_1 + i * 2))->enabled = false;
        		}
        	}
        
        	void setBottomBarAlt(int mode, int current, cell* selectedCell = nullptr)
        	{
				const static int INFO_MODE = 0;
				const static int UPGRADE_QUANT_MODE = 1;
				const static int UPGRADE_QUAL_MODE = 2;

        		switch (mode)
        		{
        		case INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::BUILD:
        			setBottomBar (
        				INDEXS::SPRITES[current].pos,
        				{ INDEXS::ICONS::DELINEATORS::SOLID, INDEXS::ICONS::DELINEATORS::SOLID, INDEXS::ICONS::DELINEATORS::SOLID },
        				{
        					"COST:\n" + CELLDATA::baseCost[current]->formated_to_String(),
        					"PRODUCTION:\n" + CELLDATA::baseModifiers[current]->formated_to_String(),
        					"DESCRIPTION:\n" + CELLDATA::buildingBuildText[current]
        				}
        			);
        			break;
        		case INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::UPGRADE:
        			if (selectedCell->cellID <= INDEXS::STRUCTURES::STRUCTURE_ID_END)
        			{
        				if (current == INFO_MODE || selectedCell->cellID == INDEXS::STRUCTURES::ROAD)
        				{
        					setBottomBar (
        						selectedCell->sprite.assetPos,
        						{ INDEXS::ICONS::DELINEATORS::SOLID, INDEXS::ICONS::DELINEATORS::SOLID, INDEXS::ICONS::DELINEATORS::SOLID },
        						{
        							"STATUS:\n"	 + ((structure*)selectedCell)->status.formated_to_string(),
        							"PRODUCTION:\n"  + selectedCell->deltaResources->formated_to_String(),
        							"DESCRIPTION:\n" + CELLDATA::buildingBuildText[selectedCell->cellID]
        						}
        					);
        				}
        				else if(mode == UPGRADE_QUANT_MODE || mode == UPGRADE_QUAL_MODE)
        				{
        					// Upgrade options (Roads are not upgradeable)
        					setBottomBar(
        						CELLDATA::ICON_SPRITE_POS[current - 1],
        						{ INDEXS::ICONS::DELINEATORS::SOLID, INDEXS::ICONS::DELINEATORS::SOLID, INDEXS::ICONS::DELINEATORS::SOLID },
        						{
        							(current == 1 ? "QUANTITY COST:\n" : "QUALITY COST:\n") + selectedCell->buildCost->formated_to_String(),
        							"OLD PRODUCTION:\n" + (*selectedCell->deltaResources * 1).formated_to_String(),
        							"NEW PRODUCTION:\n" + (*selectedCell->deltaResources * (current == 1 ? CELLDATA::quantityUpgradeMultiplier : CELLDATA::qualityUpgradeMultiplier)).formated_to_String()
        						}
        					);
        				}
        			}
        			else
        				std::cout << "ERROR: SetBottomBar(2) given cell is invalid!";
        			break;
        		default:
        			std::cout << "ERROR: SetBottomBar(2) mode is invalid!";
        			break;
        		}
        	}
            
        	/* ==== MANAGE RESOURCES ==== */
        	bool changeResources(resourceMatrix matrix, bool recurse = true)
        	{
        		resources += matrix;
        
        		if (resources < 0)
        		{
        			/*
        				money = sell overproduced resources, sell power, sell building supplies
        				supplies/power = buy with money when more is needed than availible
        				waste = remove (with money) from any building with excess trash
        			*/
        			if (resources[INDEXS::MATRIX::INCOME] < 0.0f)
        			{
						// First try to trade construction
						if (resources[INDEXS::MATRIX::CONSTRUCTION] > 0.0f)
						{
							float currentConstruction = resources[INDEXS::MATRIX::CONSTRUCTION];
							float currentDeficit = -resources[INDEXS::MATRIX::INCOME];
							float requiredConstruction = currentDeficit / (mapData.resourceExchangePrice[INDEXS::MATRIX::CONSTRUCTION] / 2);

							if (currentConstruction >= requiredConstruction)
							{
								resources[INDEXS::MATRIX::CONSTRUCTION] = clamp(currentConstruction - requiredConstruction, 0, currentConstruction + 1);
								resources[INDEXS::MATRIX::INCOME] = 0.0f;
								return true;
							}
							else if(currentConstruction > 0)
							{
								resources[INDEXS::MATRIX::INCOME] = clamp(-currentDeficit + currentConstruction * (mapData.resourceExchangePrice[INDEXS::MATRIX::CONSTRUCTION] / 2), -currentDeficit, 0);
								resources[INDEXS::MATRIX::CONSTRUCTION] = 0.0f;
							}
						}
						
						// Then trade electricity
						if(resources[INDEXS::MATRIX::ELECTRICITY] > 0.0f)
						{
							float currentElectricity = resources[INDEXS::MATRIX::ELECTRICITY];
							float currentDeficit = -resources[INDEXS::MATRIX::INCOME];
							float requiredElectricity = currentDeficit / (mapData.resourceExchangePrice[INDEXS::MATRIX::ELECTRICITY] / 2);

							if (currentElectricity >= requiredElectricity)
							{
								resources[INDEXS::MATRIX::ELECTRICITY] = clamp(currentElectricity - requiredElectricity, 0, currentElectricity + 1);
								resources[INDEXS::MATRIX::INCOME] = 0.0f;
								return true;
							}
							else if(currentElectricity > 0)
							{
								resources[INDEXS::MATRIX::INCOME] = clamp(-currentDeficit + currentElectricity * (mapData.resourceExchangePrice[INDEXS::MATRIX::ELECTRICITY] / 2), -currentDeficit, 0);
								resources[INDEXS::MATRIX::ELECTRICITY] = 0.0f;
							}
						}

						return false;
        			}
        			if (recurse && resources[INDEXS::MATRIX::CONSTRUCTION] < 0.0f)
        			{
        				if (!changeResources(
							{0,
							mapData.resourceExchangePrice[INDEXS::MATRIX::CONSTRUCTION] *
							resources[INDEXS::MATRIX::CONSTRUCTION],
							0, 0, 0, 0 }, false))
        					return false;
        				
        				resources[2] = 0.0f;
        			}
        			if (recurse && resources[INDEXS::MATRIX::ELECTRICITY] < 0.0f)
        			{
        				if (!changeResources(
							{ 0,
							mapData.resourceExchangePrice[INDEXS::MATRIX::ELECTRICITY] *
							resources[INDEXS::MATRIX::ELECTRICITY],
							0, 0, 0, 0 }, false))
        					return false;
        
        				resources[3] = 0.0f;
        			}
        			if (resources[INDEXS::MATRIX::WASTE] < 0.0f)
        				resources[INDEXS::MATRIX::WASTE] = 0;
        			if (resources[INDEXS::MATRIX::TRAFFIC] < 0.0f)
        				resources[INDEXS::MATRIX::TRAFFIC] = 0;
        		}
        		return true;
        	}
        
        	/* ==== MANAGE MAP ==== */
        	void buildStructure(int structureID, cell* selectedCell)
        	{
				if (selectedCell->cellID >= INDEXS::TERRAIN::UNBUILDABLE)
					return;

        		structure* newCell = nullptr;
				
        		if (changeResources(*CELLDATA::baseCost[selectedBottomBarOption[bottomBarMode]] + *((terrain*) selectedCell)->buildCost))
        		{
        			if(structureID != INDEXS::STRUCTURES::ROAD)
        				newCell = new structure(selectedBottomBarOption[bottomBarMode], selectedStructure->sprite.pos, 0.0f);
        			else
        				newCell = new structure(selectedBottomBarOption[bottomBarMode], selectedStructure->sprite.pos, 0.0f, 0, 0, roadTimer);
					
        			map[selectedPos] = std::move(newCell);
        
        			updateSelection(true, INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::UPGRADE, INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::UPGRADE_MAX);
        
        			updateSurroundingRoads(newCell);
        
        			if (structureID == INDEXS::STRUCTURES::ROAD)
        				allignRoads(selectedStructure);
        		}
        		else
        		{
        			return;
        		}
        	}
        
        	void upgradeStructure(int mode, cell* selectedCell)
        	{
        		if (mode == 1 && ((structure*)selectedCell)->status.upgradeQuantity()) // Quantity
        		{
        			changeResources(*selectedCell->buildCost);
        			*selectedCell->buildCost *= CELLDATA::upgradeMultiplier;
        			*selectedCell->deltaResources *= CELLDATA::quantityUpgradeMultiplier;
        		}
        		else if (mode == 2 && ((structure*)selectedCell)->status.upgradeQuality()) // Quality
        		{
        			changeResources(*selectedCell->buildCost);
        			*selectedCell->buildCost *= CELLDATA::upgradeMultiplier;
        			*selectedCell->deltaResources *= CELLDATA::qualityUpgradeMultiplier;
        		}
        	}
        
        	void allignRoads(cell* selectedRoad, bool deadEnd = false)
        	{
        		if (selectedRoad->cellID != INDEXS::STRUCTURES::ROAD)
        			return;

        		const int RGHT = 0;
        		const int LEFT = 2;
        		const int _UP_ = 1;
        		const int DOWN = 3;
        
        		const int STRGHT = 0;
        		const int CORNER = 1;
        		const int T_JUNC = 2;
        		const int X_JUNC = 3;
        
        		const std::vector< std::vector<int>> typeMap {
        			{-1  ,-1    }, {_UP_,STRGHT}, {_UP_,STRGHT}, {_UP_,STRGHT},
        			{RGHT,STRGHT}, {DOWN,CORNER}, {LEFT,CORNER}, {DOWN,T_JUNC},
        			{RGHT,STRGHT}, {RGHT,CORNER}, {_UP_,CORNER}, {_UP_,T_JUNC},
        			{RGHT,STRGHT}, {RGHT,T_JUNC}, {LEFT,T_JUNC}, {RGHT,X_JUNC}
        		};
        
        		olc::vi2d pos = selectedRoad->sprite.pos;
        		cell* up	= pos.y - 1 >= 0				 ? map[(pos.x + 0) + (mapData.mapSize.y * (pos.y - 1))] : nullptr;
        		cell* down	= pos.y + 1 < mapData.mapSize.y	 ? map[(pos.x + 0) + (mapData.mapSize.y * (pos.y + 1))] : nullptr;
        		cell* left	= pos.x - 1 >= 0				 ? map[(pos.x - 1) + (mapData.mapSize.y * (pos.y + 0))] : nullptr;
        		cell* right = pos.x + 1 < mapData.mapSize.x	 ? map[(pos.x + 1) + (mapData.mapSize.y * (pos.y + 0))] : nullptr;
        
        		int index = 0;
        
        		// 1st bit = right road
        		if (right && right->cellID == INDEXS::STRUCTURES::ROAD)
					index += 1 << 0;
        
        		// 2nd bit = left road
        		if (left && left->cellID == INDEXS::STRUCTURES::ROAD)
        			index += 1 << 1;
        
        		// 3rd bit = up road
        		if (up && up->cellID == INDEXS::STRUCTURES::ROAD)
        			index += 1 << 2;
        
        		// 4th bit = down road
        		if (down && down->cellID == INDEXS::STRUCTURES::ROAD)
        			index += 1 << 3;
        
        		if (index != 0)
        		{
        			selectedRoad->sprite.assetPos = INDEXS::SPRITES[INDEXS::STRUCTURES::ROAD + typeMap[index][1]].pos;
        			selectedRoad->sprite.rotation = typeMap[index][0] * PI / 2;
        
        			if (!deadEnd)
        			{
        				allignRoads(up, true);
        				allignRoads(down, true);
        				allignRoads(left, true);
        				allignRoads(right, true);
        			}
        		}
        	}
        
        	void updateSurroundingRoads(structure* selectedStructure)
        	{
        		olc::vi2d pos = selectedStructure->sprite.pos;
        		cell* next = nullptr;
        
        		if (selectedStructure->cellID <= INDEXS::STRUCTURES::STRUCTURE_ID_END)
        		{
        			for (int y = pos.y - 1; y <= pos.y + 1; y++)
        				for (int x = pos.x - 1; x <= pos.x + 1; x++)
        				{
        					if (y == pos.y && x == pos.x)
        						continue;
        
        					if (x + (mapData.mapSize.y * y) < map.size())
        						next = map[x + (mapData.mapSize.y * y)];
        
        					if (next && next->cellID == INDEXS::STRUCTURES::ROAD)
        						selectedStructure->surroundingRoads.push_back(next);
        				}
        		}
        
        		if (selectedStructure->cellID == INDEXS::STRUCTURES::ROAD)
        		{
        			for (int y = pos.y - 1; y <= pos.y + 1; y++)
        				for (int x = pos.x - 1; x <= pos.x + 1; x++)
        				{
        					if (y == pos.y && x == pos.x)
        						continue;
        
        					if (x + (mapData.mapSize.y * y) < map.size() && x + (mapData.mapSize.y * y) >= 0)
        						next = map[x + (mapData.mapSize.y * y)];
        
        					if (next && next->cellID <= INDEXS::STRUCTURES::STRUCTURE_ID_END)
        						((structure*)(map[x + (mapData.mapSize.y * y)]))->surroundingRoads.push_back(selectedStructure);
        				}
        		}
        	}
        
        	void calculateTraffic(cell* selectedCell, float elapsedTime)
        	{
        		olc::vi2d pos = selectedStructure->sprite.pos;
        		float traffic = 0.0f;
        		float diff = 0.0f;
        		structure* selectedStructure = (structure*)selectedCell;
        		int count = selectedStructure->surroundingRoads.size();
        
        		if (selectedStructure->cellID < INDEXS::STRUCTURES::STRUCTURE_ID_END)
        		{
        			traffic = 
        				(*(selectedStructure->deltaResources))[INDEXS::MATRIX::TRAFFIC] * elapsedTime +
        				selectedStructure->status.traffic.current * selectedStructure->status.traffic.norm() * elapsedTime;
        
        			selectedStructure->status.traffic.current -=
        				selectedStructure->status.traffic.current * selectedStructure->status.traffic.norm() * elapsedTime;
        
        			for (cell* s : selectedStructure->surroundingRoads)
        			{
        				diff = traffic / count;
        				traffic += ((structure*)s)->status.traffic.ceil(diff);
        				traffic -= diff;
        				count--;
        			}
        
        			if (traffic > 0)
        				selectedStructure->status.traffic.ceil(traffic);
        		}
        		else if (selectedStructure->cellID == INDEXS::STRUCTURES::ROAD)
        		{
        			if (selectedStructure->status.traffic.current > 0)
        			{
        				selectedStructure->status.traffic.current += (*(selectedStructure->deltaResources))[INDEXS::MATRIX::TRAFFIC] * selectedStructure->status.traffic.norm() * elapsedTime;
        
        				if (count > 0)
        				{
        					traffic =
        						selectedStructure->status.traffic.current * selectedStructure->status.traffic.norm() * elapsedTime;
        					selectedStructure->status.traffic.current -=
        						selectedStructure->status.traffic.current * selectedStructure->status.traffic.norm() * elapsedTime;
        
        					diff = traffic / count;
        
        					for (cell* s : selectedStructure->surroundingRoads)
        					{
        						traffic += ((structure*)s)->status.traffic.ceil(diff);
        						traffic -= diff;
        					}
        
        					if (traffic > 0)
        						selectedStructure->status.traffic.ceil(traffic);
        				}
        			}
        			else
        				selectedStructure->status.traffic.current = 0;
        
        		}
        
        	}
        
        	/* ==== MANAGE VIEW ==== */

        	void moveCamera(float x, float y)
        	{
        		bool inBounds = false;
        
        		if (cameraPos.x + x >= 0 && cameraPos.x + x <= mapData.mapSize.x - 1)
        		{
        			cameraPos.x += x;
        			inBounds = true;
        		}
        		else
        		{
        			cameraPos.x = round(cameraPos.x);
        			inBounds = false;
        		}
        
        		if(cameraPos.y + y >= 0 && cameraPos.y + y <= mapData.mapSize.y - 1)
        		{
        			cameraPos.y += y;
        			inBounds = true;
        		}
        		else
        		{
        			cameraPos.y = round(cameraPos.y);
        			inBounds = false;
        		}
        
        		if ( inBounds && ((int)(cameraPos.x - x) != (int)cameraPos.x || (int)(cameraPos.y - y) != (int)cameraPos.y) )
        			updateSelection(true);
        	}
        
        	void updateSelection(bool updateBottomBar, int setMode = -1, int setMax = -1)
        	{
        		selectedPos = cameraPos.x + cameraPos.y * mapData.mapSize.y;
        		selectedStructure = map[selectedPos];
        
        		if (setMode != -1 && setMax != -1) 
        		{
        			bottomBarMode = setMode;
        			maxBottomBarOption = setMax;
        
        			if (updateBottomBar)
        				setBottomBarAlt(bottomBarMode, selectedBottomBarOption[bottomBarMode], selectedStructure);
        
        			return;
        		}
        		else
        		{
        			if (selectedStructure->cellID <= INDEXS::STRUCTURES::STRUCTURE_ID_END)
        			{
        				bottomBarMode = INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::UPGRADE;
        				maxBottomBarOption = INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::UPGRADE_MAX;
        
        				if (updateBottomBar)
        					setBottomBarAlt(bottomBarMode, selectedBottomBarOption[bottomBarMode], selectedStructure);
        
        				return;
        			}
        			else if (bottomBarMode != INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::BUILD)
        			{
        				bottomBarMode = INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::BUILD;
        				maxBottomBarOption = INDEXS::SCREENS::MAP::BOTTOM_BAR::MODES::BUILD_MAX;
        				
        				if (updateBottomBar)
        					setBottomBarAlt(bottomBarMode, selectedBottomBarOption[bottomBarMode]);
        
        				return;
        			}
        		}
        
        		if(updateBottomBar)
        			setBottomBarAlt(bottomBarMode, selectedBottomBarOption[bottomBarMode], selectedStructure);
        	}
    
        };
        
        struct scoreScreen : public screen
        {
			int currentScore;
            char currentLetter;
            int currentSpace;
            std::string currentName;
			timer carretTimer = timer(1.0f, true);
			bool carret = false;
			int totalScore = 0;
            
			scoreScreen(Infrastructure* _game)
			{
				content.push_back(_game->UI[INDEXS::SCREENS::SCORE::INDEX]);
			}
            
            void init() override
			{
                currentLetter = 'A';
                currentSpace = 0;
                currentName = "AAA";
				getCurrentScores();
				setEnable(true);
				content[0]->elements[INDEXS::SCREENS::SCORE::INPUT]->enabled = true;
			}
            
            void update(Infrastructure* _game, float elapsedTime) override
            {
				game = _game;
                
        		// = = = = = GET NAME = = = = = = //

				if (game->GetKey(olc::W).bPressed || game->GetKey(olc::UP).bPressed)
				{
					currentLetter = clamp(currentLetter - 1, 'A', 'Z', true);
					currentName[currentSpace] = currentLetter;
					carretTimer.reset();
					carret = false;
				}
				else if (game->GetKey(olc::S).bPressed || game->GetKey(olc::DOWN).bPressed)
				{
					currentLetter = clamp(currentLetter + 1, 'A', 'Z', true);
					currentName[currentSpace] = currentLetter;
					carretTimer.reset();
					carret = false;
				}
				else if (game->GetKey(olc::A).bPressed || game->GetKey(olc::LEFT).bPressed)
				{
					currentSpace = clamp(currentSpace - 1, 0, 2);
					carretTimer.reset();
					carret = true;
				}
				else if (game->GetKey(olc::D).bPressed || game->GetKey(olc::RIGHT).bPressed)
				{
					currentSpace = clamp(currentSpace + 1, 0, 2);
					carretTimer.reset();
					carret = true;
				}
                else if(game->GetKey(olc::ENTER).bPressed)
                {
					ASSETS::addScore(totalScore, ((gameScreen*)game->screens[INDEXS::SCREENS::MAP::INDEX])->currentMap + 1, currentName);
					game->reset();
                }

				updateNameOnScreen(elapsedTime);
                
        		// = = = = = DRAW UI = = = = = = //
        
        		for (panel* p : content)
        		{
        			p->drawSelf(game);
        		}
            }
            
            void end() override
			{
				setEnable(false);
				content[0]->elements[INDEXS::SCREENS::SCORE::INPUT]->enabled = false;
			}

			void getCurrentScores()
			{
				std::string scores = "";
				
				for (int i = ASSETS::scores.size() - 1; i >= 0 && ASSETS::scores.size() - i < 20; i--)
				{
					scores += std::to_string(ASSETS::scores.size() - i) + '\t' + '\t' + ASSETS::scores[i]->to_string();
				}

				((label*)content[0]->elements[INDEXS::SCREENS::SCORE::SCORES])->setAsString(scores);

			}

			void updateNameOnScreen(float elapsedTime)
			{
				std::string temp = std::string(currentName);
				
				if (carretTimer.incrament(elapsedTime))
					carret = !carret;

				if(carret)
					temp[currentSpace] = '_';

				((label*)content[0]->elements[INDEXS::SCREENS::SCORE::INPUT]->elements[0])->setAsString(
					"SCORE: " + std::to_string(totalScore) +
					" ZONE: " + std::to_string(((gameScreen*)game->screens[INDEXS::SCREENS::MAP::INDEX])->currentMap) +
					" NAME: " + temp
				);
			}
        };
        
public:
	/* ==== LOAD GAME ==== */
	void loadUI()
	{
		// All main menu components
		panel* mainMenu = new panel("mainmenu", olc::vi2d{ 0,0 }, olc::vi2d{ ScreenCellWidth, ScreenCellHeight } *CellSize, olc::BLANK, false);
		{
			// Background video
			mainMenu->elements.push_back(
				new video(
					"mainMenuVideo", { 0,0 }, { ScreenCellWidth * CellSize, ScreenCellHeight * CellSize }, olc::BLACK, true,
					10.0f, & ASSETS::menuVideoFrames, true
				)
			);

			// Enter text
			mainMenu->elements.push_back(
				new label("pressEnterText", { ScreenCellWidth / 2 * CellSize - CharSize * 7 / 2, ScreenCellHeight / 2 * CellSize + CharSize * 3  }, { CharSize * 11, CharSize }, olc::BLACK, true,
					"PRESS ENTER", olc::WHITE, {1,1}, -1, false)
			);
			
			// Controls
			mainMenu->elements.push_back(
				new label("controlsText", { CharSize * 2, (ScreenCellHeight) * CellSize - CharSize * 3 - 1 }, { CharSize * 32 + 2, CharSize * 4 }, olc::BLACK, true,
					"CONTROLS:                                  \n"
					"WASD   = MOVE CURSOR | LMB  = BUILD/UPGRADE\n"
					"ARROWS = MOVE CURSOR | RMB  = CYCLE OPTIONS\n"
					"ENTER  = PLAY        | ESC  = CLOSE GAME   ",
					olc::WHITE, {0.75f, 0.75f}, -1, false
				)
			);

			// Logo image
			image* img = new image("logoimg", { -20 , -25 }, { 200, 150 }, olc::BLACK, false, { 0.31f, 0.31f }, olc::WHITE, true);
			img->setDecal(ASSETS::logo);
			mainMenu->elements.push_back(img);
		}
		UI.push_back(mainMenu);

		// All game screen UI
		panel* gameMenu = new panel("gamemenu", { 0,0 }, { ScreenCellWidth * CellSize, ScreenCellHeight * CellSize }, olc::BLANK, false);
		{
			// Shows resources
			panel* topbar = new panel("topbar", { 0,0 }, { ScreenCellWidth * CellSize, CellSize }, olc::BLACK, true);
			{
				const olc::vi2d topBarImageOffset(17, 3);
				const olc::vi2d topBarTextOffset(1, 22);
				const olc::vi2d topBarResoruceSeparation(48, 0);

				const std::vector<std::string> topNames{ "population", "money", "construction", "power", "waste", "time" };
				const int catDigits = 6;
				for (int i = 0; i < 6; i++)
				{
					image* img = new image(topNames[i] + "image", topBarImageOffset + topBarResoruceSeparation * i, { IconSize, IconSize }, olc::BLANK, true, { 1.0f, 1.0f }, olc::WHITE);
					img->setDecal(&ASSETS::infraIcons, { i,0 });
					topbar->elements.push_back(img);

					topbar->elements.push_back(new label(
						topNames[i] + "label", topBarTextOffset + topBarResoruceSeparation * i, { CharSize * catDigits, CharSize }, olc::BLANK, true,
						"000000", olc::WHITE, { 0.9f, 1.0f }, catDigits));
				}
			}
			gameMenu->elements.push_back(topbar);

			// SHows info, construction, and upgrades
			panel* bottombar = new panel("buildbar", { 0, (ScreenCellHeight - 1) * CellSize }, { ScreenCellWidth * CellSize, CellSize }, olc::BLACK, true);
			{
				bottombar->elements.push_back(new image(
					"iconimg", bottombar->pos + olc::vi2d{ 1, 2 }, { CellSize, CellSize }, olc::BLANK, true,
					{ (CellSize - 4.0f) / CellSize, (CellSize - 4.0f) / CellSize }, olc::WHITE)
				);

				std::vector<olc::vf2d> bodyTextSizes = { {0.625f, 1.0f} , {0.625f, 1.0f} , {0.75f, 0.75f} };

				for (int i = 0; i < 3; i++)
				{
					bottombar->elements.push_back(
						new label("bodylabel" + i, bottombar->pos + olc::vi2d(CellSize * (1.0f + 2.5f * i) + (2 - i), 0), { CellSize * 3, CellSize }, olc::BLANK, true,
							"", olc::WHITE, bodyTextSizes[i])
					);

					bottombar->elements.push_back(
						new image("delineatorimg" + i, bottombar->pos + olc::vi2d(CellSize * (1.0f + 2.5f * i) - (1 + i), 0), { CellSize, CellSize }, olc::BLANK, true,
							{ 1.0f, 1.0f }, olc::WHITE, true)
					);
				}
			}
			gameMenu->elements.push_back(bottombar);

			// Shows end game state
			panel* endGameMessage = new panel("celebration", { ScreenCellWidth * CellSize / 2, ScreenCellHeight * CellSize / 2 }, { ScreenCellWidth * CellSize / 4, ScreenCellHeight * CellSize / 4 }, olc::BLANK, false);
			{
				endGameMessage->elements.push_back(
					new label("messagelabel", endGameMessage->pos - olc::vi2d( CharSize * 25 / 2, CharSize * 3 / 2 ), olc::vi2d{ CharSize * 25, CharSize * 3 }, olc::BLACK, true,
						"", olc::WHITE, { 1.0f, 1.0f }, -1, false)
				);
			}
			gameMenu->elements.push_back(endGameMessage);
		}
		UI.push_back(gameMenu);
		
		panel* scoreBoard = new panel("scoreboard", olc::vi2d{ 0,0 }, olc::vi2d{ ScreenCellWidth, ScreenCellHeight } * CellSize, olc::BLACK, false);
		{
			// Logo image
			image* img = new image("logoimg", { 45 , -30 }, { 200, 150 }, olc::BLANK, false, { 0.185f, 0.185f }, olc::WHITE, true);
			img->setDecal(ASSETS::logo);
			scoreBoard->elements.push_back(img);
            
			// Catagorys
			scoreBoard->elements.push_back(
				new label("scorecatagorieslabel", { CharSize * 4, CharSize * 4 }, { CharSize * 32, CharSize }, olc::BLANK, true,
					"PLACE\tPOINTS\tSTAGE\tNAME", olc::WHITE)
			);
            
			// Loaded scores
			scoreBoard->elements.push_back(
				new label("scoreslabel", { CharSize * 4, CharSize * 5 }, { ScreenCellWidth * CellSize - CharSize * 2 , ScreenCellWidth * CellSize - CharSize * 4 }, olc::BLANK, true, "", olc::WHITE)
			);
            
			// Menu for entering in player score
			panel* ScoreBoardEntry = new panel("scoreboardentry", { 0, ScreenCellHeight * CellSize - CharSize * 2 }, { ScreenCellWidth * CellSize, CharSize * 2 }, olc::DARK_GREY, false);
			{
				ScoreBoardEntry->elements.push_back(
					new label("entrylabel", { CharSize * 1, ScreenCellHeight * CellSize - CharSize * 2 }, { ScreenCellWidth * CellSize - CharSize * 2 , CharSize * 2 }, olc::DARK_GREY, true, "SCORE: XXXX  STAGE: X  NAME: ___", olc::WHITE)
				);
			}
			scoreBoard->elements.push_back(ScoreBoardEntry);
		}
		UI.push_back(scoreBoard);
	}
	
	void loadScreens()
	{
		screens.push_back(new menuScreen(this));
		screens.push_back(new gameScreen(this));
		screens.push_back(new scoreScreen(this));
		((scoreScreen*)screens[INDEXS::SCREENS::SCORE::INDEX])->getCurrentScores();
		currentScreen = INDEXS::SCREENS::MAIN::INDEX;
		screens[currentScreen]->init();
	}

	void reset()
	{
		currentScreen = 0;
		UI.clear();
		screens.clear();
		loadUI();
		loadScreens();
	}

	/* ==== MANAGE UI ==== */

	void switchScreen(int screenID)
	{
		screens[currentScreen]->end();
		currentScreen = screenID;
		screens[currentScreen]->init();
	}
    
public:
	std::vector <panel*> UI; // Root of all UI components
	std::vector <screen*> screens; // Menu, Game, Score
	int currentScreen = 0; // Screen being displayed
    
public:
	bool OnUserCreate() override
	{
		std::cout << "Loading Map Assets...";

		// Load image assets
		ASSETS::infraAssets = ASSETS::loadSpriteSheet(this, std::move(ASSETS::infraAssets));
		
		std::cout << "\nDONE!\nLoading Font...";

		ASSETS::alphaChars  = ASSETS::loadSpriteSheet(this, std::move(ASSETS::alphaChars));

		std::cout << "\nDONE!\nLoading Icons...";

		ASSETS::infraIcons  = ASSETS::loadSpriteSheet(this, std::move(ASSETS::infraIcons));
		ASSETS::logo        = new olc::Decal(new olc::Sprite("Assets\\InfrastructureLogo-min.png"));

		std::cout << "\nDONE!\nLoading Video...";

		// Load other
		ASSETS::loadVideo();

		std::cout << "\nDONE!\nLoading Songs...";

		ASSETS::loadSong();

		std::cout << "\nDONE!\nLoading Scores...";

		ASSETS::loadScores();

		std::cout << "\nDONE!";

		// Set up UI
		loadUI();
		loadScreens();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		//Clear(olc::GREY);
		
		// = = = = = UNIVERSAL INPUT = = = = = = //

		// Leave game
		if (GetKey(olc::Key::ESCAPE).bPressed)
			return false;

		// Reset game
		if (GetKey(olc::Key::SHIFT).bHeld && GetKey(olc::Key::R).bPressed)
			reset();
        
		// Speed up game
		if (GetKey(olc::Key::SHIFT).bHeld && GetKey(olc::Key::V).bHeld)
			fElapsedTime *= 4;

		// = = = = = UPDATE SCREEN = = = = = = //

		screens[currentScreen]->update(this, fElapsedTime);

		return true;
	}

	bool OnUserDestroy() override
	{
		olc::SOUND::DestroyAudio();
		ASSETS::saveScores();
		return true;
	}
};

int main()
{
	Infrastructure demo;
	if (demo.Construct(ScreenCellWidth * CellSize, ScreenCellHeight * CellSize, 5, 5, false, true))
		demo.Start();
	return 0;
}