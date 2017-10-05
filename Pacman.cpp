#include <iostream>
#include <vector>
#include <conio.h>
#include <fstream>
#include <Windows.h>
#include <thread>
#include <queue>

using namespace std::chrono_literals;
auto game_speed = 0.1s;

typedef std::vector<char> Row;
typedef std::vector<Row> Matrix;

const bool use_colors = true;

//colors
void ShowConsoleCursor(bool);
WORD GetConsoleTextAttribute(HANDLE);
void set_color_red();
void set_color_yellow();
void set_color_blue();
void set_color_normal();

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
const int saved_colors = GetConsoleTextAttribute(hConsole);
//

// arrows
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77
// WSAD
#define UP_CAPS 87
#define DOWN_CAPS 83
#define LEFT_CAPS 65
#define RIGHT_CAPS 68
// wsad
#define UP 119
#define DOWN 115
#define LEFT 97
#define RIGHT 100

class Pacman
{

public:

	Pacman(int x_, int y_, char symbol_) : x(y_), y(x_), symbol(symbol_) {}

	std::pair<int, int> getCoords()
	{
		return std::make_pair(x, y);
	}

	int& getScore()
	{
		return score;
	}

	char& getDir()
	{
		return dir;
	}

	char& getSymbol()
	{
		return symbol;
	}

	std::queue<char>& q_dirs()
	{
		return q_dirs_;
	}

	void moveUp()
	{
		x--;
	}

	void moveDown()
	{
		x++;
	}

	void moveLeft()
	{
		y--;
	}

	void moveRight()
	{
		y++;
	}

private:

	int x, y;
	int score = 0;
	char dir;
	char symbol;
	std::queue<char> q_dirs_;

};

class Ghost
{

public:

	Ghost(int x_, int y_, char symbol_) : x(y_), y(x_), symbol(symbol_) {}

	std::pair<int, int> getCoords()
	{
		return std::make_pair(x, y);
	}

	char& getDir()
	{
		return dir;
	}

	char& getSymbol()
	{
		return symbol;
	}

	void moveUp()
	{
		x--;
	}

	void moveDown()
	{
		x++;
	}

	void moveLeft()
	{
		y--;
	}

	void moveRight()
	{
		y++;
	}

private:

	int x, y;
	char dir;
	char symbol;

};

class Map
{

private:
	int width_, height_;
	Matrix board_;
	Pacman& pac_;
	char coin_;
	char obstacle_;
	std::vector<Ghost> ghosts_;

public:

	Map(Pacman& p, int w, int h) : width_(w), height_(h), pac_(p)
	{
		board_ = Matrix(height_, Row(width_, ' '));
	}

	Map(Pacman& p, int w, int h, char obstacle, char coin) : width_(w), height_(h), pac_(p), obstacle_(obstacle), coin_(coin)
	{
		board_ = Matrix(height_, Row(width_, ' '));
	}

	Map(Pacman& p, int w, int h, char obstacle, char coin, int ghost_count) : width_(w), height_(h), pac_(p), obstacle_(obstacle), coin_(coin)
	{
		board_ = Matrix(height_, Row(width_, ' '));
		ghosts_.resize(ghost_count, Ghost(0, 0, 'A'));
	}

	Matrix& board()
	{
		return board_;
	}

	Pacman& pac()
	{
		return pac_;
	}

	int& width()
	{
		return width_;
	}

	int& height()
	{
		return height_;
	}

	char& coin()
	{
		return coin_;
	}

	char& obstacle()
	{
		return obstacle_;
	}

	std::vector<Ghost>& ghosts()
	{
		return ghosts_;
	}

	bool canMove(int i, int j)
	{
		if(i < 0 || j < 0 || i >= height() || j >= width())
			return false;
		return board()[i][j] != obstacle();
	}

	bool checkCoin(int i, int j)
	{
		return board()[i][j] == coin();
	}

	bool checkWin()
	{
		for (int i = 0; i < height(); ++i)
			if (std::find(board()[i].begin(), board()[i].end(), coin()) != board()[i].end())
				return false;
		return true;
	}

	bool checkDeath()
	{
		auto pac_coords = pac().getCoords();
		for (int k = 0; k < ghosts().size(); ++k)
		{
			auto ghost_coords = ghosts()[k].getCoords();
			if (pac_coords == ghost_coords)
				return true;
			else // the two passed through each other
			{
				if (pac_coords.first - ghost_coords.first == 1 && // pac lower than ghost
					pac().getDir() == 'u' && ghosts()[k].getDir() == 'd')
					return true;
				else if (pac_coords.first - ghost_coords.first == -1 && // pac higher than ghost
					pac().getDir() == 'd' && ghosts()[k].getDir() == 'u')
					return true;
				else if (pac_coords.second - ghost_coords.second == 1 && // pac is righer than ghost
					pac().getDir() == 'l' && ghosts()[k].getDir() == 'r')
					return true;
				else if (pac_coords.second - ghost_coords.second == -1 && // pac is lefter
					pac().getDir() == 'r' && ghosts()[k].getDir() == 'l')
					return true;
			}
		}
		return false;
	}

	bool GameOver()
	{
		if (checkWin())
		{
			std::cout << std::endl << "You won! :)" << std::endl;
			return true;
		}
		else if (checkDeath())
		{
			std::cout << std::endl << "You lose! :(" << std::endl;
			return true;
		}
		return false;
	}

	void eatCoin(const std::pair<int, int>& coords)
	{
		if (checkCoin(coords.first, coords.second))
			pac().getScore() += 100;
	}

	void pac_control()
	{
		auto coords = pac().getCoords();
		if (!pac().q_dirs().empty())
		{
			pac().getDir() = pac().q_dirs().front();
			pac().q_dirs().pop();
		}
		switch (pac().getDir())
		{
		case 'u':
			if (canMove(coords.first - 1, coords.second))
			{
				pac().moveUp();
				eatCoin(pac().getCoords());
			}
			break;
		case 'd':
			if (canMove(coords.first + 1, coords.second))
			{
				pac().moveDown();
				eatCoin(pac().getCoords());
			}
			break;
		case 'l':
			if (canMove(coords.first, coords.second - 1))
			{
				pac().moveLeft();
				eatCoin(pac().getCoords());
			}
			break;
		case 'r':
			if (canMove(coords.first, coords.second + 1))
			{
				pac().moveRight();
				eatCoin(pac().getCoords());
			}
			break;
		}
	}

	void ghost_control()
	{
		std::vector<std::vector<int>> mat(height(), std::vector<int>(width(), -1));
		for (int i = 0; i < height(); ++i)
			for (int j = 0; j < width(); ++j)
				if (board()[i][j] == '#')
					mat[i][j] = -2; // -2 is wall, -1 is free space
		auto start = pac().getCoords();
		for (int k = 0; k < ghosts().size(); ++k)
		{
			int step = 0;
			auto dest = ghosts()[k].getCoords();
			mat[start.first][start.second] = step;
			while (mat[dest.first][dest.second] == -1)
			{
				for (int i = 0; i < height(); ++i)
				{
					for (int j = 0; j < width(); ++j)
					{
						if (mat[i][j] == step)
						{
							if (i + 1 < height() && mat[i + 1][j] == -1)
								mat[i + 1][j] = step + 1;
							if (i > 0 && mat[i - 1][j] == -1)
								mat[i - 1][j] = step + 1;
							if (j + 1 < width() && mat[i][j + 1] == -1)
								mat[i][j + 1] = step + 1;
							if (j > 0 && mat[i][j - 1] == -1)
								mat[i][j - 1] = step + 1;
						}
					}
				}
				++step;
			}
			if (dest.first + 1 < height() && mat[dest.first + 1][dest.second] == step - 1)
				ghosts()[k].moveDown();
			else if (dest.first > 0 && mat[dest.first - 1][dest.second] == step - 1)
				ghosts()[k].moveUp();
			else if (dest.second + 1 < height() && mat[dest.first][dest.second + 1] == step - 1)
				ghosts()[k].moveRight();
			else if (dest.second > 0 && mat[dest.first][dest.second - 1] == step - 1)
				ghosts()[k].moveLeft();
		}
	}

	void move()
	{
		while (!GameOver())
		{
			pac_control();
			srand(time(NULL));
			ghost_control();
			Draw();
			std::this_thread::sleep_for(game_speed);
		}
	}

	void check_dir()
	{
		while (!GameOver())
		{
			if (_kbhit())
			{
				auto coords = pac().getCoords();
				switch (_getch())
				{
				case UP:
				case UP_ARROW:
				case UP_CAPS:
					if (canMove(coords.first - 1, coords.second))
						pac().q_dirs().push('u');
					else
					{
						switch (pac().getDir())
						{
						case 'l':
							if (canMove(coords.first - 1, coords.second - 1))
							{
								pac().q_dirs().push('l');
								pac().q_dirs().push('u');
							}
							else if (canMove(coords.first - 1, coords.second - 2))
							{
								pac().q_dirs().push('l');
								pac().q_dirs().push('l');
								pac().q_dirs().push('u');
							}
							break;
						case 'r':
							if (canMove(coords.first - 1, coords.second + 1))
							{
								pac().q_dirs().push('r');
								pac().q_dirs().push('u');
							}
							else if (canMove(coords.first - 1, coords.second + 2))
							{
								pac().q_dirs().push('r');
								pac().q_dirs().push('r');
								pac().q_dirs().push('u');
							}
							break;
						}
					}
					break;
				case DOWN:
				case DOWN_ARROW:
				case DOWN_CAPS:
					if (canMove(coords.first + 1, coords.second))
						pac().q_dirs().push('d');
					else
					{
						switch (pac().getDir())
						{
						case 'l':
							if (canMove(coords.first + 1, coords.second - 1))
							{
								pac().q_dirs().push('l');
								pac().q_dirs().push('d');
							}
							else if (canMove(coords.first + 1, coords.second - 2))
							{
								pac().q_dirs().push('l');
								pac().q_dirs().push('l');
								pac().q_dirs().push('d');
							}
							break;
						case 'r':
							if (canMove(coords.first + 1, coords.second + 1))
							{
								pac().q_dirs().push('r');
								pac().q_dirs().push('d');
							}
							else if (canMove(coords.first + 1, coords.second + 2))
							{
								pac().q_dirs().push('r');
								pac().q_dirs().push('r');
								pac().q_dirs().push('u');
							}
							break;
						}
					}
					break;
				case LEFT:
				case LEFT_ARROW:
				case LEFT_CAPS:
					if (canMove(coords.first, coords.second - 1))
						pac().q_dirs().push('l');
					else
					{
						switch (pac().getDir())
						{
						case 'u':
							if (canMove(coords.first - 1, coords.second - 1))
							{
								pac().q_dirs().push('u');
								pac().q_dirs().push('l');
							}
							else if (canMove(coords.first - 2, coords.second - 1))
							{
								pac().q_dirs().push('u');
								pac().q_dirs().push('u');
								pac().q_dirs().push('l');
							}
							break;
						case 'd':
							if (canMove(coords.first + 1, coords.second - 1))
							{
								pac().q_dirs().push('d');
								pac().q_dirs().push('l');
							}
							else if (canMove(coords.first + 2, coords.second - 1))
							{
								pac().q_dirs().push('d');
								pac().q_dirs().push('d');
								pac().q_dirs().push('l');
							}
							break;
						}
					}
					break;
				case RIGHT:
				case RIGHT_ARROW:
				case RIGHT_CAPS:
					if (canMove(coords.first, coords.second + 1))
						pac().q_dirs().push('r');
					else
					{
						switch (pac().getDir())
						{
						case 'u':
							if (canMove(coords.first - 1, coords.second + 1))
							{
								pac().q_dirs().push('u');
								pac().q_dirs().push('r');
							}
							else if (canMove(coords.first - 2, coords.second + 1))
							{
								pac().q_dirs().push('u');
								pac().q_dirs().push('u');
								pac().q_dirs().push('r');
							}
							break;
						case 'd':
							if (canMove(coords.first + 1, coords.second + 1))
							{
								pac().q_dirs().push('d');
								pac().q_dirs().push('r');
							}
							else if (canMove(coords.first + 2, coords.second + 1))
							{
								pac().q_dirs().push('d');
								pac().q_dirs().push('d');
								pac().q_dirs().push('r');
							}
							break;
						}
					}
					break;
				}
			}
		}
	}

	void Pat() // Eva's choice :)
	{
		std::ifstream fin("map.txt");
		for(int i = 0; i < height(); ++i)
			for (int j = 0; j < width(); ++j)
			{
				char c;
				fin >> c;
				if (c == obstacle())
					board()[i][j] = c;
				else
					board()[i][j] = coin();
			}
	}

	void updateMap()
	{
		auto coords = pac_.getCoords();
		for (int i = 0; i < height(); ++i)
			for (int j = 0; j < width(); ++j)
				if (board()[i][j] == pac().getSymbol())
					board()[i][j] = ' ';
		board()[coords.first][coords.second] = pac().getSymbol();
	}

	void Draw()
	{
		system("cls");
		updateMap();
		for (int i = 0; i < height(); ++i)
		{
			for (int j = 0; j < width(); ++j)
			{
				if (board()[i][j] == pac().getSymbol())
					set_color_blue();
				else if (board()[i][j] == coin())
					set_color_yellow();
				bool used_by_ghost = false;
				for (int k = 0; k < ghosts().size(); ++k)
				{
					auto coords = ghosts()[k].getCoords();
					if (coords.first == i && coords.second == j)
					{
						used_by_ghost = true;
						set_color_red();
						std::cout << ghosts()[k].getSymbol();
					}
				}
				if (!used_by_ghost)
					std::cout << board()[i][j];
				set_color_normal();
			}
			std::cout << std::endl;
		}
		std::cout << "X: " << pac().getCoords().second << " Y: " << pac().getCoords().first << std::endl;
		std::cout << "Score: " << pac().getScore();
		std::cout << std::endl << "Dir: " << pac().getDir() << " Front: " << (!pac().q_dirs().empty() ? pac().q_dirs().front() + "" : "Empty");
	}
};


int main()
{
	ShowConsoleCursor(false);
	Pacman pac(13, 21, 'P');
	Map map(pac, 27, 30, '#', 'o', 3);
	map.ghosts()[0] = Ghost(13, 14, 'A');
	map.ghosts()[1] = Ghost(10, 12, 'B');
	map.ghosts()[2] = Ghost(13, 10, 'C');
	//map.ghosts()[3] = Ghost(16, 12, 'D');
	map.Pat();
	std::thread* th_move = new std::thread(&Map::move, map);
	std::thread* th_check = new std::thread(&Map::check_dir, map);
	th_move->join();
	th_check->join();
}


//colors


void ShowConsoleCursor(bool show)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = show; // set the cursor visibility
	SetConsoleCursorInfo(out, &cursorInfo);
}

WORD GetConsoleTextAttribute(HANDLE hConsole)
{
	CONSOLE_SCREEN_BUFFER_INFO con_info;
	GetConsoleScreenBufferInfo(hConsole, &con_info);
	return con_info.wAttributes;
}

void set_color_red()
{
	if (use_colors)
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
}

void set_color_yellow()
{
	if (use_colors)
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

void set_color_blue()
{
	if (use_colors)
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

void set_color_normal()
{
	if (use_colors)
		SetConsoleTextAttribute(hConsole, saved_colors);
}