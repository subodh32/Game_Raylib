#include <raylib.h>

#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 1000

#define LEVEL_HEIGHT 12
#define LEVEL_WIDTH 20

#define BLOCK_SIZE 50

#define PLAYER_VELOCITY 500
#define JUMP_VELOCITY 700
#define GRAVITY 2000

#define MAX_BULLETS 50 //Maximum number of bullets at any time
#define BULLET_SPEED 700
#define BULLET_SIZE 15
#define MAX_BULLET_FRAMES 60*30 //No. of frames a bullet will last on screen

float delta; //Delta is the time since last frame was drawn

struct Box{
  int x,y;
  int width,height;
};

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

struct BlockBehavior
{
  bool visible;
  bool bullet_collidable;
  Color color;
  int display_height, display_width;
  int collison_height, collision_width;

  bool collidable_sides[4];
};

//Behavior of diffrent types of blocks
struct BlockBehavior block_behavior[] = {
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
      false,
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
  bool exits;
  int x,y;
  int vx,vy;
  int frames_since_spawned;
};

struct Level
{
  char name[10];
  int block_types[LEVEL_HEIGHT][LEVEL_WIDTH];
};

enum EnemyType{
  Follower //Walks towards player
};

struct EnemyTypeProperties
{
  struct Color color;
  int width,height;
}; 

struct EnemyTypeProperties enemy_properties[] = {
  {
    { 230, 41, 55, 255 }, //RED
    BLOCK_SIZE,BLOCK_SIZE
  }
};

struct Enemy
{
  int hp;
  enum EnemyType type;

  int x, y;
  int vx, vy;
  int ay;

  bool is_jumping;
  enum Direction direction;
};


void display_bullets(struct Bullet *bullet_list);
void update_bullets(struct Bullet *bullet_list);
void shoot_bullet(struct Player player,struct Bullet *bullet_list);
void bullet_level_collision(struct Bullet *bullet_list,struct Level *level);

void display_level(struct Level level);
void player_level_collision(struct Player *player, struct Level level);

void player_controller(struct Player *player, struct Bullet *bullet_list);

void enemy_draw_and_update(struct Enemy *enemy_list,struct Level *level,struct Player *player,struct Bullet *bullet_list);

void draw_enemy(struct Enemy enemy);
void enemy_update(struct Enemy *enemy);
void enemy_behavior(struct Enemy *enemy,struct Level *level,struct Player *player,struct Bullet *bullet_list);
void enemy_level_collision(struct Enemy *enemy,struct Level *level);

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

  struct Level current_level = level1;

  struct Player player = {
      screenWidth / 2,
      screenHeight / 2,
      0, 0,
      GRAVITY,
      false,
      RIGHT};

  struct Bullet bullet_list[MAX_BULLETS];

  //initialise all bullets as not existing with 0 velocity
  for(int i = 0; i < MAX_BULLETS; i++)
  {
    bullet_list[i].exits = false;
    bullet_list[i].vx = 0;
    bullet_list[i].vy = 0;
  }

  struct Enemy enemy1 = {
    100,
    Follower,
    10*BLOCK_SIZE,4*BLOCK_SIZE,
    0,0,
    GRAVITY,
    false,
    LEFT
  };

  //Camera to follow player
  cam.target = (Vector2){player.x, 0};
  cam.offset = (Vector2){screenWidth / 2, 0};
  cam.rotation = 0.0f;
  cam.zoom = 1.0f;

  while (!WindowShouldClose())
  {
    delta = GetFrameTime();

    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawFPS(20, 20);

    bullet_level_collision(bullet_list,&current_level);

    BeginMode2D(cam);

    //Draw Player
    DrawRectangle(player.x, player.y, 50, 50, PINK);
    draw_enemy(enemy1);

    //Draw other things
    display_bullets(bullet_list);
    display_level(current_level);

    // UPDATES
    update_bullets(bullet_list);
    player_controller(&player,bullet_list);

    //Collisions
    player_level_collision(&player, current_level);

    enemy_level_collision(&enemy1,&current_level);
    enemy_update(&enemy1);

    player.vy += delta * player.ay;
    player.x += player.vx * delta;
    player.y += player.vy * delta;

    cam.target = (Vector2){player.x, 0};

    EndMode2D();
    EndDrawing();
  }
  CloseWindow();
  return 0;
}

void draw_enemy(struct Enemy enemy)
{
  struct EnemyTypeProperties type;
  switch (enemy.type)
  {
  case Follower:
    type = enemy_properties[Follower];
    DrawRectangle(enemy.x,enemy.y,type.width,type.height,type.color);
    break;
  
  default:
    break;
  }
}

void enemy_level_collision(struct Enemy *enemy,struct Level *level)
{
    enum BlockType type_X, type_Y1, type_Y2;

  int collision_X, collision_Y;
  int level_X,level_Y;

  int midX,midY;
  midX = enemy->x + BLOCK_SIZE/2;
  midY = enemy->y + BLOCK_SIZE/2;

  if (enemy->vx > 0)
  {
    collision_X = (enemy->x + BLOCK_SIZE);
  }
  else if (enemy->vx < 0)
  {
    collision_X = (enemy->x);
  }

  level_X = collision_X/BLOCK_SIZE;

  if(enemy->vy >= 0)
  {
    collision_Y = enemy->y + BLOCK_SIZE;
  }
  else if(enemy->vy < 0)
  {
    collision_Y = enemy->y;
  }

  level_Y = collision_Y/BLOCK_SIZE;

  if (level_X < 0 || level_X > LEVEL_WIDTH)
  {
    enemy->vx = 0;
    return;
  }

  if (level_Y < 1 || level_Y > LEVEL_HEIGHT - 1)
  {
    enemy->vy = 0;
    return;
  }

  type_X = level->block_types[(enemy->y + BLOCK_SIZE/2) / BLOCK_SIZE][collision_X / BLOCK_SIZE];

  type_Y1 = level->block_types[collision_Y / BLOCK_SIZE][(enemy->x + 1) / BLOCK_SIZE];
  type_Y2 = level->block_types[collision_Y / BLOCK_SIZE][(enemy->x + BLOCK_SIZE - 1) / BLOCK_SIZE];

  if (enemy->vx > 0 && block_behavior[type_X].collidable_sides[LEFT_SIDE])
  {
    enemy->x = level_X*BLOCK_SIZE - BLOCK_SIZE;
    enemy->vx = 0;
  }

  if (enemy->vx < 0 && block_behavior[type_X].collidable_sides[RIGHT_SIDE])
  {
    enemy->x = level_X*BLOCK_SIZE + BLOCK_SIZE - 1;
    enemy->vx = 0;
  }

  if (enemy->vy > 0  &&
    (block_behavior[type_Y1].collidable_sides[TOP_SIDE] || 
    block_behavior[type_Y2].collidable_sides[TOP_SIDE]))
  {
    enemy->y = (level_Y * BLOCK_SIZE) - BLOCK_SIZE;

    enemy->ay = 0;
    enemy->vy = 0;
    enemy->is_jumping = false;
  }else{
    enemy->ay = GRAVITY;
  }

  if (enemy->vy < 0  &&
    (block_behavior[type_Y1].collidable_sides[BOTTOM_SIDE] || 
    block_behavior[type_Y2].collidable_sides[BOTTOM_SIDE]))
  {
    enemy->y = (level_Y * BLOCK_SIZE) + BLOCK_SIZE - 1;
    enemy->vy = 0;
  }
}

void enemy_update(struct Enemy *enemy)
{
  enemy->vy += delta*enemy->ay;

  enemy->x += delta*enemy->vx;
  enemy->y += delta*enemy->vy;
}

void bullet_level_collision(struct Bullet *bullet_list,struct Level *level)
{
  enum BlockType type;
  struct Bullet *bullet;

  for(int i = 0; i < MAX_BULLETS; i++)
  {
    bullet = &bullet_list[i];

    if(bullet->exits)
    {
      type = level->block_types[bullet->y / BLOCK_SIZE][ bullet->x / BLOCK_SIZE];

      if(block_behavior[type].bullet_collidable)
      {
        bullet->exits = false;
        bullet->vx = 0;
        bullet->vy = 0;
      }
    }
  }
}

void display_bullets(struct Bullet *bullet_list)
{
  int x,y;
  for(int i = 0; i < MAX_BULLETS; i++)
  {
    if(bullet_list[i].exits)
    {
      x = bullet_list[i].x;
      y = bullet_list[i].y;

      DrawCircle(x,y,BULLET_SIZE,BLACK);
    }
  }
}

void update_bullets(struct Bullet *bullet_list)
{
  struct Bullet *bullet;
  for(int i = 0; i < MAX_BULLETS; i++)
  {
    bullet = &bullet_list[i];

    if(bullet->exits)
    {      
      bullet->x += bullet->vx * delta;
      bullet->y += bullet->vy * delta;

      bullet->frames_since_spawned += 1;

      if(bullet->x > LEVEL_WIDTH*BLOCK_SIZE || bullet->x < 0
        || bullet->y > LEVEL_HEIGHT*BLOCK_SIZE || bullet->y < 0
        || bullet->frames_since_spawned > MAX_BULLET_FRAMES)
      {
        bullet->exits = false;
        bullet->vx = 0;
        bullet->vy = 0;
      }
    }
  }
}

void shoot_bullet(struct Player player,struct Bullet *bullet_list)
{
  int i = 0;
  while(i < MAX_BULLETS && bullet_list[i].exits)
  {
    i++;
  }

  if(i < MAX_BULLETS && bullet_list[i].exits == false)
  {
    bullet_list[i].exits = true;
    bullet_list[i].frames_since_spawned =  0;

    bullet_list[i].vx = player.direction * BULLET_SPEED;

    bullet_list[i].x = player.x + BLOCK_SIZE/2;
    bullet_list[i].y = player.y + BLOCK_SIZE/2;
  }
}

void display_level(struct Level level)
{
  enum BlockType type;

  for (int y = 0; y < LEVEL_HEIGHT; y++)
  {
    for (int x = 0; x < LEVEL_WIDTH; x++)
    {
      type = level.block_types[y][x];

      if (block_behavior[type].visible)
      {
        DrawRectangle(x * BLOCK_SIZE, y * BLOCK_SIZE, block_behavior[type].display_width, block_behavior[type].display_height, block_behavior[type].color);
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

  if (player->vx > 0 && block_behavior[type_X].collidable_sides[LEFT_SIDE])
  {
    player->x = level_X*BLOCK_SIZE - BLOCK_SIZE;
    player->vx = 0;
  }

  if (player->vx < 0 && block_behavior[type_X].collidable_sides[RIGHT_SIDE])
  {
    player->x = level_X*BLOCK_SIZE + BLOCK_SIZE - 1;
    player->vx = 0;
  }

  if (player->vy > 0  &&
    (block_behavior[type_Y1].collidable_sides[TOP_SIDE] || 
    block_behavior[type_Y2].collidable_sides[TOP_SIDE]))
  {
    player->y = (level_Y * BLOCK_SIZE) - BLOCK_SIZE;

    player->ay = 0;
    player->vy = 0;
    player->is_jumping = false;
  }else{
    player->ay = GRAVITY;
  }

  if (player->vy < 0  &&
    (block_behavior[type_Y1].collidable_sides[BOTTOM_SIDE] || 
    block_behavior[type_Y2].collidable_sides[BOTTOM_SIDE]))
  {
    player->y = (level_Y * BLOCK_SIZE) + BLOCK_SIZE - 1;
    //player->y = (level_Y * BLOCK_SIZE) - 1; for jumpable platforms
    player->vy = 0;
  }
}

void player_controller(struct Player *player, struct Bullet *bullet_list)
{
  if (IsKeyDown(KEY_A))
  {
    player->vx = -PLAYER_VELOCITY;
    player->direction = LEFT;
  }
  else if (IsKeyDown(KEY_D))
  {
    player->vx = PLAYER_VELOCITY;
    player->direction = RIGHT;
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

  if(IsKeyPressed(KEY_ENTER))
  {
    shoot_bullet(*player,bullet_list);
  }
}
