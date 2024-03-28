#include <raylib.h>

#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 800

#define LEVEL_HEIGHT 12
#define LEVEL_WIDTH 20

#define BLOCK_SIZE 50

#define PLAYER_VELOCITY 500
#define JUMP_VELOCITY 1000
#define GRAVITY 50

float delta;

enum Direction
{
  LEFT = -1,
  RIGHT = 1
};

enum Sides
{
  TOP_SIDE,
  BOTTOM_SIDE,
  LEFT_SIDE,
  RIGHT_SIDE
};

struct Camera
{
  int x, y;
  float delta_time;
};

enum BlockType
{
  Air,
  Ground
};

struct Block
{
  bool visible;
  bool Collidable;
  Color color;
  int display_height, display_width;
  int collison_height, collision_width;

  bool collidable_sides[4];
};

struct Block block[] = {
    // Air
    {
      false,
      false,
      WHITE,
      0,0,
      0,0,
      {false, false, false, false}
    },

    // Ground
    {
      true,
      true,
      LIME,
      BLOCK_SIZE,BLOCK_SIZE,
      BLOCK_SIZE,BLOCK_SIZE,
      {true, true, true, true}
    },

    //Platform
    {
      true,
      true,
      BROWN,
      BLOCK_SIZE/2,BLOCK_SIZE/2,
      BLOCK_SIZE/2,BLOCK_SIZE,
      {true,false,true,true}
    }
};

struct Player
{
  int x, y;
  int vx, vy;
  int ay;
  bool is_jumping;
  enum Direction direction;
};

struct Bullet
{
  int x,y;
  int vx,vy;
  float time_since_spawned;
};

void update_bullets()
{
  
}

void shoot_bullet()
{
  
}

struct Level
{
  char name[10];
  int block_types[LEVEL_HEIGHT][LEVEL_WIDTH];
};

void display_level(struct Level level)
{
  enum BlockType type;

  for (int y = 0; y < LEVEL_HEIGHT; y++)
  {
    for (int x = 0; x < LEVEL_WIDTH; x++)
    {
      type = level.block_types[y][x];

      if (block[type].visible)
      {
        DrawRectangle(x * BLOCK_SIZE, y * BLOCK_SIZE, block[type].display_width, block[type].display_height, block[type].color);
      }
    }
  }
}

void player_level_collision(struct Player *player, struct Level level)
{
  enum BlockType type_X, type_Y1, type_Y2;

  int collision_X, collision_Y;
  int level_X,level_Y;

  int midX,midY;
  midX = player->x + BLOCK_SIZE/2;
  midY = player->y + BLOCK_SIZE/2;

  if (player->vx > 0)
  {
    collision_X = (player->x + BLOCK_SIZE);
  }
  else if (player->vx < 0)
  {
    collision_X = (player->x);
  }

  level_X = collision_X/BLOCK_SIZE;

  if(player->vy >= 0)
  {
    collision_Y = player->y + BLOCK_SIZE;
  }
  else if(player->vy < 0)
  {
    collision_Y = player->y;
  }

  level_Y = collision_Y/BLOCK_SIZE;

  if (level_X < 0 || level_X > LEVEL_WIDTH)
  {
    player->vx = 0;
    return;
  }

  if (level_Y < 1 || level_Y > LEVEL_HEIGHT - 1)
  {
    player->vy = 0;
    return;
  }

  type_X = level.block_types[(player->y + BLOCK_SIZE/2) / BLOCK_SIZE][collision_X / BLOCK_SIZE];

  type_Y1 = level.block_types[collision_Y / BLOCK_SIZE][(player->x + 1) / BLOCK_SIZE];
  type_Y2 = level.block_types[collision_Y / BLOCK_SIZE][(player->x + BLOCK_SIZE - 1) / BLOCK_SIZE];

  if (player->vx > 0 && block[type_X].collidable_sides[LEFT_SIDE])
  {
    player->x = level_X*BLOCK_SIZE - BLOCK_SIZE;
    player->vx = 0;
  }

  if (player->vx < 0 && block[type_X].collidable_sides[RIGHT_SIDE])
  {
    player->x = level_X*BLOCK_SIZE + BLOCK_SIZE - 1;
    player->vx = 0;
  }

  if (player->vy > 0  &&
    (block[type_Y1].collidable_sides[TOP_SIDE] || 
    block[type_Y2].collidable_sides[TOP_SIDE]))
  {
    player->y = (level_Y * BLOCK_SIZE) - BLOCK_SIZE;

    player->ay = 0;
    player->vy = 0;
    player->is_jumping = false;
  }else{
    player->ay = GRAVITY;
  }

  if (player->vy < 0  &&
    (block[type_Y1].collidable_sides[BOTTOM_SIDE] || 
    block[type_Y2].collidable_sides[BOTTOM_SIDE]))
  {
    player->y = (level_Y * BLOCK_SIZE) + BLOCK_SIZE - 1;
    //player->y = (level_Y * BLOCK_SIZE) - 1; for jumpable platforms
    player->vy = 0;
  }
}

void player_controller(struct Player *player)
{
  if (IsKeyDown(KEY_A))
  {
    player->vx = -PLAYER_VELOCITY;
  }
  else if (IsKeyDown(KEY_D))
  {
    player->vx = PLAYER_VELOCITY;
  }
  else
  {
    player->vx = 0;
  }

  if (IsKeyDown(KEY_SPACE))
  {
    if (player->is_jumping == false)
    {
      player->vy = -(JUMP_VELOCITY);
      player->is_jumping = true;
    }
  }
}

int main()
{
  const int screenWidth = SCREEN_WIDTH;
  const int screenHeight = SCREEN_HEIGHT;
  InitWindow(screenWidth, screenHeight, "Raylib basic window");
  SetTargetFPS(60);

  char key;

  Camera2D cam = {0};

  struct Level level1 = {
      "level1",
      {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}};

  struct Player player = {
      screenWidth / 2,
      screenHeight / 2,
      0, 0,
      GRAVITY,
      false,
      LEFT};

  cam.target = (Vector2){player.x, 0};
  cam.offset = (Vector2){screenWidth / 2, 0};
  cam.rotation = 0.0f;
  cam.zoom = 1.0f;

  while (!WindowShouldClose())
  {

    delta = GetFrameTime();

    // DrawRectangle(player.x, player.y, 50, 50, PINK);
    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawFPS(20, 20);

    BeginMode2D(cam);

    DrawRectangle(player.x, player.y, 50, 50, PINK);
    display_level(level1);

    // UPDATES
    player_controller(&player);

    player_level_collision(&player, level1);

    player.vy += player.ay;
    player.x += player.vx * delta;
    player.y += player.vy * delta;

    cam.target = (Vector2){player.x, 0};

    EndMode2D();
    EndDrawing();
  }
  CloseWindow();
  return 0;
}