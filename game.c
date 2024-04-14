#include <raylib.h>
#include <stdio.h>

#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 1000

#define LEVEL_HEIGHT 12
#define LEVEL_WIDTH 25

#define BLOCK_SIZE 50

#define PLAYER_VELOCITY 500
#define JUMP_VELOCITY 700
#define GRAVITY 2000

#define MAX_BULLETS 50 //Maximum number of bullets at any time
#define BULLET_SPEED 700
#define BULLET_SIZE 15
#define MAX_BULLET_FRAMES 60*30 //No. of frames a bullet will last on screen

#define ENEMY_BULLET_COOLDOWN 0.5 //in seconds
#define ENEMY_VELOCITY 500
#define MAX_ENEMY 10

#define PLAYER_MAX_HP 10

#define MAX_LEVELS 4

float delta; //Delta is the time since last frame was drawn
int score;

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
      BLOCK_SIZE/2,BLOCK_SIZE,
      BLOCK_SIZE/2,BLOCK_SIZE,
      {true,false,true,true}
    }
};

struct Player
{
  int hp;
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
  bool fired_by_player;
};

enum EnemyType{
  Gunner, //Shoots player
  Runner //Runs towards player
};

struct EnemyTypeProperties
{
  struct Color color;
  int width,height;
  int max_hp;
}; 

struct EnemyTypeProperties enemy_properties[] = {
  {
    { 230, 41, 55, 255 }, //RED
    BLOCK_SIZE,BLOCK_SIZE,
    3
  },
  {
    { 41, 128, 230, 255 }, //BLUE
    BLOCK_SIZE,BLOCK_SIZE,
    1
  }
};

struct Enemy
{
  bool exits;
  int hp;
  enum EnemyType type;

  int x, y;
  int vx, vy;
  int ay;

  bool is_jumping;
  enum Direction direction;

  float time_since_event[2]; //in general time for any event ex: time since last bullet was shot,time since last jumped
};

struct Level
{
  int id;
  int player_start_x, player_start_y;
  int block_types[LEVEL_HEIGHT][LEVEL_WIDTH];
  int total_enemies;
  struct Enemy enemies[MAX_ENEMY];
};


//an array of struct Level
struct Level levels[] = {
  {
      0, //level id
      SCREEN_WIDTH/2,SCREEN_HEIGHT/2, //player start position

      {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 2, 2, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 1, 2, 2, 2, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
       {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
       {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
       {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}},

       2, // No. of enemies

       {
        {
        true,
        3,
        Gunner,
    
        10*BLOCK_SIZE,4*BLOCK_SIZE,
        0,0,
        GRAVITY,
    
        false,
        LEFT,
    
        {0,0}
        },

        {
        true,
        1,
        Runner,
    
        13*BLOCK_SIZE,4*BLOCK_SIZE,
        0,0,
        GRAVITY,
    
        false,
        RIGHT,
    
        {0,0}
        }
       }
  },

  {
      1, //level id
      SCREEN_WIDTH/2,SCREEN_HEIGHT/2, //player start position

      {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 2, 2, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
       {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
       {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}},

       2,

       //Enemies

       {
        {
        true,
        3,
        Gunner,
    
        6*BLOCK_SIZE,4*BLOCK_SIZE,
        0,0,
        GRAVITY,
    
        false,
        LEFT,
    
        {0,0}
        },
        {
        true,
        1,
        Runner,
    
        8*BLOCK_SIZE,4*BLOCK_SIZE,
        0,0,
        GRAVITY,
    
        false,
        RIGHT,
    
        {0,0}
        }
       }
  },

  {
      2,
      SCREEN_WIDTH/2,SCREEN_HEIGHT/2,

      {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 2, 2, 0, 0, 1, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 1, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
       {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
       {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}},

       3,
       
       {
        {
        true,
        3,
        Gunner,
    
        6*BLOCK_SIZE,4*BLOCK_SIZE,
        0,0,
        GRAVITY,
    
        false,
        LEFT,
    
        {0,0}
        },
        {
        true,
        1,
        Runner,
    
        8*BLOCK_SIZE,4*BLOCK_SIZE,
        0,0,
        GRAVITY,
    
        false,
        RIGHT,
    
        {0,0}
        },
        {
        true,
        1,
        Runner,
    
        10*BLOCK_SIZE,4*BLOCK_SIZE,
        0,0,
        GRAVITY,
    
        false,
        RIGHT,
    
        {0,0}
        },
        {
          true,
        1,
        Runner,
    
        14*BLOCK_SIZE,4*BLOCK_SIZE,
        0,0,
        GRAVITY,
    
        false,
        LEFT,
    
        {0,0}
        }
        
       }
  },

  {
      3,
      SCREEN_WIDTH/2,SCREEN_HEIGHT/2,

      {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 2, 2, 0, 0, 1, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 1, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
      {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
      {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}},

      3,


  {   {
      true,
      1,
      Runner,
      8*BLOCK_SIZE,4*BLOCK_SIZE,
      0,0,
      GRAVITY,
      false,
      RIGHT,
      {0,0}
      },
      {
      true,
      1,
      Runner,
      10*BLOCK_SIZE,4*BLOCK_SIZE,
      0,0,
      GRAVITY,
      false,
      RIGHT,
      {0,0}
      },
      {
      true,
      1,
      Runner,
      14*BLOCK_SIZE,4*BLOCK_SIZE,
      0,0,
      GRAVITY,
      false,
      RIGHT,
      {0,0}
      }
      }
  }

};

void display_bullets(struct Bullet *bullet_list);
void update_bullets(struct Bullet *bullet_list);
void shoot_bullet(struct Player player,struct Bullet *bullet_list);
void bullet_level_collision(struct Bullet *bullet_list,struct Level *level);

void display_level(struct Level level);
void player_level_collision(struct Player *player, struct Level level);

void player_controller(struct Player *player, struct Bullet *bullet_list);

void enemy_list_draw(struct Enemy *enemy_list);
void enemy_list_update(struct Enemy *enemy_list,struct Level *level,struct Player *player,struct Bullet *bullet_list);
void enemy_list_levelcollisions(struct Enemy *enemy_list,struct Level *level);

void draw_enemy(struct Enemy enemy);
void enemy_update(struct Enemy *enemy);
void enemy_behavior(struct Enemy *enemy,struct Level *level,struct Player *player,struct Bullet *bullet_list);
void enemy_level_collision(struct Enemy *enemy,struct Level *level);
void enemy_shoot_bullet(struct Enemy *enemy,struct Bullet *bullet_list);

void player_bullet_collision(struct Player *player,struct Bullet *bullet_list);
void enemy_bullet_collision(struct Enemy *enemy,struct Bullet *bullet_list);

void save_game(struct Player player,struct Enemy enemy_list[],struct Level *level);
void load_game(struct Player *player,struct Enemy *enemy_list,struct Level *level,struct Level *loaded_level,struct Bullet *bullet_list);

int main()
{
  const int screenWidth = SCREEN_WIDTH;
  const int screenHeight = SCREEN_HEIGHT;
  InitWindow(screenWidth, screenHeight, "Game");
  SetTargetFPS(60);

  bool game_over = false;
  bool game_won = false;

  char key;

  Camera2D cam = {0};

  struct Level level1 = levels[0];
  struct Level current_level = level1;
  struct Level loaded_level = current_level;

  struct Player player = {
      PLAYER_MAX_HP,
      loaded_level.player_start_x, loaded_level.player_start_y,
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

  score = 0;
  struct Enemy enemy_list[MAX_ENEMY];

  for(int i = 0; i < MAX_ENEMY; i++)
  {
    enemy_list[i].exits = false;
    enemy_list[i].vx = 0;
    enemy_list[i].vy = 0;
    enemy_list[i].ay = GRAVITY;
    enemy_list[i].is_jumping = false;
    enemy_list[i].direction = LEFT;
  }

  for(int i = 0; i < current_level.total_enemies; i++)
  {
    enemy_list[i] = current_level.enemies[i];
  }

  //Camera to follow player
  cam.target = (Vector2){player.x, 0};
  cam.offset = (Vector2){screenWidth / 2, 0};
  cam.rotation = 0.0f;
  cam.zoom = 1.0f;

  while (!WindowShouldClose())
  {
    if(game_over)
    {
      BeginDrawing();
      ClearBackground(RAYWHITE);
      DrawText("GAME OVER",SCREEN_WIDTH/2 - 40*3,SCREEN_HEIGHT/2 - 40,40,RED);
      EndDrawing();

      continue;
    }

    if(game_won)
    {
      BeginDrawing();
      ClearBackground(RAYWHITE);
      DrawText("YOU WON",SCREEN_WIDTH/2 - 40*3,SCREEN_HEIGHT/2 - 40,40,GREEN);
      DrawText(TextFormat("Score: %i",score),SCREEN_WIDTH/2 - 40*3,SCREEN_HEIGHT/2,40,BLUE);
      EndDrawing();

      continue;
    }

    //if player is on second last or last block of level change level to next level
    if(player.x + BLOCK_SIZE/2 > (LEVEL_WIDTH - 2)*BLOCK_SIZE)
    {
      if(current_level.id + 1 >= MAX_LEVELS)
      {
        game_won = true;
        continue;
      }
      else{
        current_level = levels[current_level.id + 1];
      }
    }

    //if current level is not the same as loaded level then reset player and enemies
    if(current_level.id != loaded_level.id)
    {
      player.x = current_level.player_start_x;
      player.y = current_level.player_start_y;

      for(int i = 0; i < MAX_ENEMY; i++)
      {
        enemy_list[i].exits = false;
        enemy_list[i].vx = 0;
        enemy_list[i].vy = 0;
        enemy_list[i].ay = GRAVITY;
        enemy_list[i].is_jumping = false;
        enemy_list[i].direction = LEFT;
      }

      for(int i = 0; i < current_level.total_enemies; i++)
      {
        enemy_list[i] = current_level.enemies[i];
      }

      loaded_level = current_level;
    }

    delta = GetFrameTime();

    //save and load game
    if(IsKeyPressed(KEY_K))
    {
      save_game(player,enemy_list,&current_level);
    }
    if(IsKeyPressed(KEY_L))
    {
      load_game(&player,enemy_list,&current_level,&loaded_level,bullet_list);
    }

    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawFPS(20, 20);
    DrawText(TextFormat("Health: %i",player.hp),SCREEN_WIDTH - 200,0,20,RED);
    DrawText(TextFormat("Score: %i",score),SCREEN_WIDTH - 500,20,20,BLUE);

    bullet_level_collision(bullet_list,&current_level);

    BeginMode2D(cam);

    //Draw Player
    DrawRectangle(player.x, player.y, 50, 50, PINK);
    DrawRectangle(player.x + player.direction * BLOCK_SIZE/2, player.y + BLOCK_SIZE/2, 40, 10, BLACK);
    enemy_list_draw(enemy_list);

    //Draw other things
    display_bullets(bullet_list);
    display_level(current_level);

    // UPDATES
    update_bullets(bullet_list);
    player_controller(&player,bullet_list);
    player_bullet_collision(&player,bullet_list);

    //Collisions
    player_level_collision(&player, current_level);
    enemy_list_update(enemy_list,&current_level,&player,bullet_list);
    enemy_list_levelcollisions(enemy_list,&current_level);

    player.vy += delta * player.ay;
    player.x += player.vx * delta;
    player.y += player.vy * delta;

    cam.target = (Vector2){player.x, 0};

    if(player.hp < 0)
    {
      game_over = true;
    }

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
  case Gunner:
    type = enemy_properties[Gunner];
    DrawRectangle(enemy.x,enemy.y,type.width,type.height,type.color);
    DrawRectangle(enemy.x + enemy.direction * type.width/2,enemy.y + type.height/2,40,10,BLACK);
    break;

  case Runner:
    type = enemy_properties[Runner];
    DrawRectangle(enemy.x,enemy.y,type.width,type.height,type.color);
    DrawRectangle(enemy.x + enemy.direction * type.width/2,enemy.y + type.height/2,40,10,GRAY);
  
  default:
    break;
  }
}

void save_game(struct Player player,struct Enemy enemy_list[],struct Level *level)
{
  FILE *save;
  char filename[] = "game.sav";

  save = fopen(filename,"w");

  if(save != NULL)
  {
    fprintf(save,"%d ",level->id);
    fprintf(save,"%d ",score);
    fprintf(save,"%d ",player.hp);
    fprintf(save,"%d ",player.x);
    fprintf(save,"%d ",player.y);
    fprintf(save,"\n");

    for(int i = 0; i < MAX_ENEMY; i++)
    {
        fprintf(save,"%d ",enemy_list[i].exits);
        fprintf(save,"%d ",enemy_list[i].hp);
        fprintf(save,"%d ",enemy_list[i].type);
        fprintf(save,"%d ",enemy_list[i].x);
        fprintf(save,"%d ",enemy_list[i].y);
        fprintf(save,"\n");
    }
  }
}

void load_game(struct Player *player,struct Enemy *enemy_list,struct Level *level,struct Level *loaded_level,struct Bullet *bullet_list)
{
  FILE *save;
  char filename[] = "game.sav";

  FILE *debug = fopen("debug.txt","w");
  int level_id;
  int x,y;
  bool exits;
  int hp,type;

  save = fopen(filename,"r");

  for(int i = 0; i < MAX_ENEMY; i++)
  {
    enemy_list[i].exits = false;
  }

  if(save != NULL)
  {
    fscanf(save,"%d",&level_id);
    fscanf(save,"%d",&score);
    fscanf(save,"%d %d %d",&hp,&x,&y);

    *level = levels[level_id];
    *loaded_level = levels[level_id];

    player->hp = hp;
    player->x = x;
    player->y = y;

    for(int i = 0; i < MAX_ENEMY; i++)
    {
      fscanf(save,"%d %d %d %d %d ",&exits,&hp,&type,&x,&y);

      fprintf(debug,"hp: %d",hp);
      enemy_list[i].exits = exits;
      enemy_list[i].type = type;
      enemy_list[i].hp = hp;
      enemy_list[i].x = x;
      enemy_list[i].y = y;
    }
  }

  for(int i = 0; i < MAX_BULLETS; i++)
  {
    bullet_list[i].exits = false;
    bullet_list[i].vx = 0;
    bullet_list[i].vy = 0;
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

void enemy_shoot_bullet(struct Enemy *enemy,struct Bullet *bullet_list)
{
  int i = 0;
  while(i < MAX_BULLETS && bullet_list[i].exits)
  {
    i++;
  }

  if(i < MAX_BULLETS && bullet_list[i].exits == false)
  {
    bullet_list[i].exits = true;
    bullet_list[i].fired_by_player = false;
    bullet_list[i].frames_since_spawned =  0;

    bullet_list[i].vx = enemy->direction * BULLET_SPEED;

    bullet_list[i].x = enemy->x + BLOCK_SIZE/2;
    bullet_list[i].y = enemy->y + BLOCK_SIZE/2;
  }
}

void enemy_behavior(struct Enemy *enemy,struct Level *level,struct Player *player,struct Bullet *bullet_list)
{
  //Line of sight of enemy
  int point_x,point_y; //point that scans the level for either player or obstacle in line of sight of enemy
  
  point_x = enemy->x + BLOCK_SIZE/2;
  point_y = enemy->y + BLOCK_SIZE/2;

  bool point_intersects_player = point_x > player->x && point_x < (player->x+BLOCK_SIZE)
                                  && point_y > player->y && point_y < (player->y + BLOCK_SIZE);

  bool point_intersects_level = ( level->block_types[point_y/BLOCK_SIZE][point_y/BLOCK_SIZE] != Air );

  while(point_intersects_player == false
        && point_intersects_level == false 
        && point_x > 0 && point_x < (LEVEL_WIDTH - 1)*BLOCK_SIZE
        && point_y > 0 && point_y < (LEVEL_WIDTH - 1)*BLOCK_SIZE)
  {
    bool point_intersects_player = point_x > player->x && point_x < (player->x+BLOCK_SIZE)
                                  && point_y > player->y && point_y < (player->y + BLOCK_SIZE);

    bool point_intersects_level = ( level->block_types[point_x/BLOCK_SIZE][point_y/BLOCK_SIZE] != Air );

    if (point_intersects_player)
    {

      if(enemy->type == Gunner)
      {
        if(enemy->time_since_event[0] > ENEMY_BULLET_COOLDOWN)
        {
          DrawCircle(enemy->x,enemy->y,100,BLUE);
          enemy_shoot_bullet(enemy,bullet_list);
          enemy->time_since_event[0] = 0;
        }

        float time = 0.06;
        float error = 0.02;

        for(float i = time; i < time*3; i += time)
        {
          if(enemy->time_since_event[0] > i - error && enemy->time_since_event[0] < i)
          {
            enemy_shoot_bullet(enemy,bullet_list);
            enemy->time_since_event[0] = i;
          }
        } 
      }

      if(enemy->type == Runner)
      {
        enemy->vx = enemy->direction * ENEMY_VELOCITY;
      }

    }

    point_x += enemy->direction * BLOCK_SIZE;
  }

  if(enemy->type == Runner)
  {
    //check if enemy intersects player
    if (enemy->x < player->x + BLOCK_SIZE &&
      enemy->x + enemy_properties[enemy->type].width > player->x &&
      enemy->y < player->y + BLOCK_SIZE &&
      enemy->y + enemy_properties[enemy->type].height > player->y)
    {
      player->hp -= 1;
      enemy->exits = false;
    }
    
  }
  
}

void enemy_list_draw(struct Enemy *enemy_list)
{
  for(int i = 0; i < MAX_ENEMY; i++)
  {
    if(enemy_list[i].exits)
    {
      draw_enemy(enemy_list[i]);
    }
  }
}

void enemy_list_update(struct Enemy *enemy_list,struct Level *level,struct Player *player,struct Bullet *bullet_list)
{
  for(int i = 0; i < MAX_ENEMY; i++)
  {
    if(enemy_list[i].exits)
    {
      enemy_behavior(&enemy_list[i],level,player,bullet_list);
      enemy_update(&enemy_list[i]);
      enemy_bullet_collision(&enemy_list[i],bullet_list);
      //enemy_level_collision(&enemy_list[i],level);
    }
  }
}

void enemy_list_levelcollisions(struct Enemy *enemy_list,struct Level *level)
{
  for(int i = 0; i < MAX_ENEMY; i++)
  {
    if(enemy_list[i].exits)
    {
      enemy_level_collision(&enemy_list[i],level);
    }
  }
}

void enemy_update(struct Enemy *enemy)
{
  enemy->vy += delta*enemy->ay;

  enemy->x += delta*enemy->vx;
  enemy->y += delta*enemy->vy;

  enemy->time_since_event[0] += delta;

  if(enemy->hp <= 0)
  {
    enemy->exits = false;
    score++;
  }
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

void enemy_bullet_collision(struct Enemy *enemy,struct Bullet *bullet_list)
{
  struct Bullet *bullet;
  for(int i = 0; i < MAX_BULLETS; i++)
  {
    bullet = &bullet_list[i];

    if(bullet->exits)
    {
      if(bullet->x > enemy->x && bullet->x < enemy->x + enemy_properties[enemy->type].width
      && bullet->y > enemy->y && bullet->y < enemy->y + enemy_properties[enemy->type].height
      && bullet->fired_by_player)
      {
        bullet->exits = false;
        enemy->hp -= 1;
      }
    }
  }
}

void player_bullet_collision(struct Player *player,struct Bullet *bullet_list)
{
  struct Bullet *bullet;
  for(int i = 0; i < MAX_BULLETS; i++)
  {
    bullet = &bullet_list[i];

    if(bullet->exits)
    {
      if(bullet->x > player->x && bullet->x < player->x + BLOCK_SIZE
      && bullet->y > player->y && bullet->y < player->y + BLOCK_SIZE
      && !bullet->fired_by_player)
      {
        bullet->exits = false;
        player->hp -= 1;
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
    bullet_list[i].fired_by_player = true;
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
