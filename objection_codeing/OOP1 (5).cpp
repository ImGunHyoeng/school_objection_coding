#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdlib> // include rand()
#include <Windows.h>
#include <conio.h>

class Canvas;
class GameObject {
	char*	shape;
	int		pos;
	bool	visible;

public:
	GameObject(const char* shape, int pos, bool visible)
		: shape(new char[strlen(shape) + 1]), pos(pos), visible(visible)
	{
		if (this->shape != nullptr)
			strcpy(this->shape, shape);
	}

	int		getPos() const { return pos; } // getter
	void	setPos(int pos) { this->pos = pos; } // setter

	const char* getShape() const { this->shape; } // getter
	void	setShape(const char* shape) { strcpy(this->shape, shape); } // setter
	int		getShapeSize() const { return (int)strlen(this->shape); }

	bool	isVisible() const { return visible; } // gettter
	void	setVisible(bool visible = true) { this->visible = visible; } // setter


	virtual void print() const { printf("GameObject %p\n", this); }

	virtual void draw(Canvas& canvas);

	void move(int speed) { pos += speed; }

	virtual void update(GameObject* objs[], int max_objs) {}

	virtual ~GameObject() {
		delete[] this->shape;
		this->shape = nullptr;
		this->pos = 0;
		this->visible = visible;
	}
};

// canvas
class Canvas {
	char* frameBuffer;
	int		size;

public:
	Canvas(int size) : size(size), frameBuffer(new char[(size_t)size + 1])
	{
		clear();
	}

	void clear()
	{
		for (int i = 0; i < size; i++)
			frameBuffer[i] = ' ';
		frameBuffer[size] = '\0';
	}

	void draw(const char* shape, int pos, bool visible)
	{
		if (visible == false) return;

		for (int i = 0; i < strlen(shape); i++)
		{
			if (pos + i < 0) continue;
			if (pos + i > size - 1) continue;

			frameBuffer[pos + i] = shape[i];
		}
	}

	void render() const
	{
		printf("%s\r", frameBuffer);
	}

	~Canvas()
	{
		delete[] frameBuffer;
		frameBuffer = nullptr;
		size = 0;
	}
};


void GameObject::draw(Canvas& canvas) { canvas.draw(shape, pos, visible); }


// player
class Player : public GameObject {

public:

	Player(const char* shape, int pos, bool visible) : GameObject(shape, pos, visible)
	{}

	void print() const override { printf("Player %p\n", this); }

	~Player()
	{}
};

class BlinkablePlayer : public Player {
	int n_frames;

public:
	BlinkablePlayer(const char* shape, int pos, bool visible) : Player(shape, pos, visible), n_frames(0)
	{
	}

	void blink(int n_frames)
	{
		this->n_frames = n_frames;
	}

	void draw(Canvas& canvas) override
	{
		if (n_frames > 0) {
			n_frames--;
			if (n_frames % 2 == 0) return;			
			GameObject::draw(canvas);
			return;
		}
		GameObject::draw(canvas);
	}


	~BlinkablePlayer() {}
};


// enemy
class Enemy : public GameObject {

	int hp;

public:

	Enemy(const char* shape, int pos, bool visible) : GameObject(shape, pos, visible), hp(1)
	{}

	void print() const override { printf("Enemy %p\n", this); }

	void onHit()
	{
		if (--hp <= 0) {
			setVisible(false);
		}
	}

	void update(GameObject* objs[], int max_objs) override {
		int prob = rand() % 100;
		if (prob < 80) return;
		if (prob < 90) move(-1);
		else move(1);
	}

	bool isColliding(int pos) const
	{
		int myPos = getPos();
		return pos >= myPos && pos < myPos + getShapeSize();
	}

	~Enemy() {}
};


// bullet
class Bullet : public GameObject {
	int			direction;

public:
	Bullet(const char* shape, int pos, bool visible, int direction)
		: GameObject(shape, pos, visible), direction(direction)
	{}

	void fire(const GameObject* player, int direction)
	{
		if (player == nullptr) return;

		int player_pos = player->getPos();
		
		setVisible();
		setPos(player_pos + player->getShapeSize());

		this->direction = direction;
		setShape(">");
		if (direction == 1)
		{
			setPos(player_pos);
			setShape("<");
		}
	}

	void move()
	{
		if (direction == 0) setPos(getPos() + 1);
		else setPos(getPos() - 1);
	}

	void print() const override { printf("Bullet %p\n", this); }

	void update(GameObject* objs[], int max_objs) override
	{
		if (isVisible() == false) return;		

		for (int i = 0; i < max_objs; i++)
		{
			if (objs[i] == nullptr) continue;

			Enemy* enemy = dynamic_cast<Enemy*>(objs[i]);
			if (enemy == nullptr) continue;
			if (enemy->isVisible() == false) continue;

			if (enemy->isColliding(getPos()) == true) {
				enemy->onHit();
				setVisible(false);
				return;
			}
		}

		move();
	}

	~Bullet()
	{
		direction = 0;
	}
};

int main()
{
	const int canvas_size = 80;
	Canvas* canvas = new Canvas(canvas_size);
	const int max_objs = 50;
	GameObject* objs[max_objs];

	for (int i = 0; i < max_objs; i++)
		objs[i] = nullptr;

	for (int i = 0; i < 5; i++)
	{
		switch (rand() % 2)
		{
		case 0:
			objs[i] = new BlinkablePlayer("P", rand() % canvas_size, true);
			break;
		case 1:
			objs[i] = new Enemy("E", rand() % canvas_size, true);
			break;
		}
	}

	bool exit_flag = false;
	while (exit_flag == false)
	{
		canvas->clear();

		if (_kbhit())
		{
			Bullet* found = nullptr;
			BlinkablePlayer* player = nullptr;
			int randPos;

			int ch = _getch();

			switch (tolower(ch))
			{
			case 'a':
				
			case 'd':
				
			case ' ':
				// find a player closest to a random position.
				randPos = rand() % 75;
				for (int i = 0; i < max_objs; i++)
				{
					if (objs[i] == nullptr) continue;

					BlinkablePlayer* obj = dynamic_cast<BlinkablePlayer*>(objs[i]);
					if (obj == nullptr) continue;

					if (obj->isVisible() == false) continue;

					if (player == nullptr) {
						player = obj;
					}
					else {
						if (abs(obj->getPos() - randPos) < abs(player->getPos() - randPos))
						{
							player = obj;
						}
					}
				}
				if (player == nullptr)
					break;

				printf("\n random [%d] player [%d]\n", randPos, player->getPos());
				// find an empty slot to allocate a new bullet or unused bullet
				for (int i = 0; i < max_objs; i++)
				{
					if (objs[i] == nullptr) {
						Bullet* bullet = new Bullet(">", 0, false, 0);
						bullet->fire(player, rand() % 2);
						player->blink(30);
						objs[i] = bullet;
						break;
					}
					Bullet* bullet = dynamic_cast<Bullet*>(objs[i]);
					if (bullet == nullptr) continue;
					if (bullet->isVisible() == true) continue;
					bullet->fire(player, rand() % 2);
					player->blink(30);
					break;
				}

				break;
			case 'q':
				exit_flag = true;
				break;
			}
		}

		for (int i = 0; i < max_objs; i++)
		{
			if (objs[i] == nullptr) continue;

			objs[i]->update(objs, max_objs);
		}

		for (int i = 0; i < max_objs; i++)
		{
			if (objs[i] == nullptr) continue;

			objs[i]->draw(*canvas);
		}

		
		canvas->render();
		Sleep(100);
	}

	for (int i = 0; i < max_objs; i++)
	{
		if (objs[i] == nullptr) continue;
		delete objs[i];
		objs[i] = nullptr;
	}
	
	return 0;
}