/*******************************************************************************************
 *
 *   raylib [core] example - 2D Camera platformer
 *
 *   Example originally created with raylib 2.5, last time updated with
 *raylib 3.0
 *
 *   Example contributed by arvyy (@arvyy) and reviewed by Ramon Santamaria
 *(@raysan5)
 *
 *   Example licensed under an unmodified zlib/libpng license, which is an
 *OSI-certified, BSD-like license that allows static linking with closed source
 *software
 *
 *   Copyright (c) 2019-2024 arvyy (@arvyy)
 *
 ********************************************************************************************/

#include "raylib-cpp/include/raylib-cpp.hpp"
#include "raylib.h"
#include "raymath.h"

#define G 400
#define PLAYER_JUMP_SPD 350.0f
#define PLAYER_SPD 200.0f

typedef struct Character {
  raylib::Vector2 position;
  raylib::Vector2 lastPosition;
  float speed;
  bool canJump;
} Character;

typedef struct EnvItem {
  Rectangle rect;
  int blocking;
  Color color;
} EnvItem;

class DetectArea {
public:
  EnvItem base;
  int active;
  int channel;
};

//----------------------------------------------------------------------------------
// Module functions declaration
//----------------------------------------------------------------------------------
void UpdateCharacter(Character *player, EnvItem *envItems,
                     DetectArea *detectionItems, int envItemsLength,
                     int detectionItemsLength, float delta);
void UpdateCameraCenter(Camera2D *camera, Character *player, EnvItem *envItems,
                        int envItemsLength, float delta, int width, int height);
void UpdateCameraCenterInsideMap(Camera2D *camera, Character *player,
                                 EnvItem *envItems, int envItemsLength,
                                 float delta, int width, int height);
void UpdateCameraCenterSmoothFollow(Camera2D *camera, Character *player,
                                    EnvItem *envItems, int envItemsLength,
                                    float delta, int width, int height);
void UpdateCameraEvenOutOnLanding(Camera2D *camera, Character *player,
                                  EnvItem *envItems, int envItemsLength,
                                  float delta, int width, int height);
void UpdateCameraCharacterBoundsPush(Camera2D *camera, Character *player,
                                     EnvItem *envItems, int envItemsLength,
                                     float delta, int width, int height);
bool InBounds(Vector2 *pos, Rectangle *rect, float delta);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera");

  Character player = {0};
  player.position = (Vector2){400, 280};
  player.lastPosition = player.position;
  player.speed = 0;
  player.canJump = false;

  //   Set up the enviroment
  //--------------------------------------------------------------------------------------
  EnvItem envItems[] = {{{0, 0, 1000, 400}, 0, YELLOW}, // background
                        {{0, 400, 1000, 20}, 1, BLACK},
                        {{300, 200, 400, 10}, 1, BLUE},
                        {{250, 300, 100, 10}, 1, ORANGE},
                        {{650, 300, 100, 10}, 1, GRAY}};

  //  Set up the detection items
  //  --------------------------------------------------------------------------------------

  DetectArea detectionItems[1] = {};

  DetectArea myArea;

  myArea.base = {{100, 100, 100, 100}, 0, Color{127, 106, 79, 128}};
  myArea.active = 1;
  myArea.channel = 1;

  detectionItems[0] = myArea;

  int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);
  int detectionItemsLength = sizeof(detectionItems) / sizeof(detectionItems[0]);

  // Set up the camera
  //--------------------------------------------------------------------------------------
  Camera2D camera = {0};
  camera.target = player.position;
  camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  // Store pointers to the multiple update camera functions
  void (*cameraUpdaters[])(Camera2D *, Character *, EnvItem *, int, float, int,
                           int) = {
      UpdateCameraCenter, UpdateCameraCenterInsideMap,
      UpdateCameraCenterSmoothFollow, UpdateCameraEvenOutOnLanding,
      UpdateCameraCharacterBoundsPush};

  int cameraOption = 1;
  int cameraUpdatersLength = sizeof(cameraUpdaters) / sizeof(cameraUpdaters[0]);

  const char *cameraDescriptions[] = {
      "Follow player center", "Follow player center, but clamp to map edges",
      "Follow player center; smoothed",
      "Follow player center horizontally; update player center vertically "
      "after landing",
      "Character push camera on getting too close to screen edge"};

  SetTargetFPS(60);
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) {
    // Update
    //----------------------------------------------------------------------------------
    float deltaTime = GetFrameTime();

    UpdateCharacter(&player, envItems, detectionItems, envItemsLength,
                    detectionItemsLength, deltaTime);

    // camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

    if (camera.zoom > 3.0f)
      camera.zoom = 3.0f;
    else if (camera.zoom < 0.25f)
      camera.zoom = 0.25f;

    if (IsKeyPressed(KEY_R)) {
      camera.zoom = 1.0f;
      player.position = (Vector2){400, 280};
    }

    if (IsKeyPressed(KEY_C))
      cameraOption = (cameraOption + 1) % cameraUpdatersLength;

    // Call update camera function by its pointer
    cameraUpdaters[cameraOption](&camera, &player, envItems, envItemsLength,
                                 deltaTime, screenWidth, screenHeight);
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(LIGHTGRAY);

    BeginMode2D(camera);
    printf("Drawing\n");

    // draw the enviromental areas
    printf("Drawing env iteareasms(%d)\n", envItemsLength);
    for (int i = 0; i < envItemsLength; i++)
      DrawRectangleRec(envItems[i].rect, envItems[i].color);

    // draw the detection areas
    printf("Drawing detection areas(%d)\n", detectionItemsLength);
    for (int i = 0; i < detectionItemsLength; i++) {
      printf("Drawing detection area\n");
      if (detectionItems[i].active == 1) {
        DrawRectangleRec(detectionItems[i].base.rect,
                         detectionItems[i].base.color);
      }
    }

    const int playerSize = 40;

    Rectangle playerRect = {player.position.x - playerSize / 2,
                            player.position.y - playerSize / 2, playerSize,
                            playerSize};
    DrawRectangleRec(playerRect, RED);

    DrawCircle(player.position.x, player.position.y, 5, GOLD);

    EndMode2D();

    // DrawText("Controls:", 20, 20, 10, BLACK);
    // DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
    // DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
    // DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10,
    // DARKGRAY);
    DrawText("- C to change camera mode", 40, 100, 10, DARKGRAY);
    DrawText("Current camera mode:", 20, 120, 10, BLACK);
    DrawText(cameraDescriptions[cameraOption], 40, 140, 10, DARKGRAY);

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

bool InBounds(Vector2 *pos, Rectangle *rect, float delta) {
  return (
      pos->x + delta >= rect->x && pos->x - delta <= rect->x + rect->width &&
      pos->y + delta >= rect->y && pos->y - delta <= rect->y + rect->height);
}

//----------------------------------------------------------------------------------
void UpdateCharacter(Character *player, EnvItem *envItems,
                     DetectArea *detectionItems, int envItemsLength,
                     int detectionItemsLength, float delta) {
  if (IsKeyDown(KEY_LEFT))
    player->position.x -= PLAYER_SPD * delta;
  if (IsKeyDown(KEY_RIGHT))
    player->position.x += PLAYER_SPD * delta;
  if (IsKeyDown(KEY_DOWN))
    player->position.y += PLAYER_SPD * delta;
  if (IsKeyDown(KEY_UP))
    player->position.y -= PLAYER_SPD * delta;

  bool hitObstacle = false;
  Vector2 *player_pos = &(player->position);
  Vector2 *lastPosition = &(player->lastPosition);
  for (int i = 0; i < envItemsLength; i++) {
    EnvItem *ei = envItems + i;

    Vector4 rect = {ei->rect.x, ei->rect.y, ei->rect.width, ei->rect.height};

    if (ei->blocking && InBounds(player_pos, &ei->rect, delta)

    ) {

      hitObstacle = true;
      player->speed = 0.0f;
      // new position is invalid, so we reset it to the last position
      player_pos->y = lastPosition->y;
      player_pos->x = lastPosition->x;

      break;
    }
  }

  for (int i = 0; i < detectionItemsLength; i++) {
    DetectArea *di = detectionItems + i;

    if (di->active && InBounds(player_pos, &di->base.rect, delta)) {
      if (di->channel == 1) {
        printf("Player is in the detection area\n");
      }
    }
  }

  if (!hitObstacle) {
    lastPosition->x = player_pos->x;
    lastPosition->y = player_pos->y;
    player->speed += G * delta;
  }
}

void UpdateCameraCenter(Camera2D *camera, Character *player, EnvItem *envItems,
                        int envItemsLength, float delta, int width,
                        int height) {
  camera->offset = (Vector2){width / 2.0f, height / 2.0f};
  camera->target = player->position;
}

void UpdateCameraCenterInsideMap(Camera2D *camera, Character *player,
                                 EnvItem *envItems, int envItemsLength,
                                 float delta, int width, int height) {
  camera->target = player->position;
  camera->offset = (Vector2){width / 2.0f, height / 2.0f};
  float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

  for (int i = 0; i < envItemsLength; i++) {
    EnvItem *ei = envItems + i;
    minX = fminf(ei->rect.x, minX);
    maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
    minY = fminf(ei->rect.y, minY);
    maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
  }

  Vector2 max = GetWorldToScreen2D((Vector2){maxX, maxY}, *camera);
  Vector2 min = GetWorldToScreen2D((Vector2){minX, minY}, *camera);

  if (max.x < width)
    camera->offset.x = width - (max.x - width / 2);
  if (max.y < height)
    camera->offset.y = height - (max.y - height / 2);
  if (min.x > 0)
    camera->offset.x = width / 2 - min.x;
  if (min.y > 0)
    camera->offset.y = height / 2 - min.y;
}

void UpdateCameraCenterSmoothFollow(Camera2D *camera, Character *player,
                                    EnvItem *envItems, int envItemsLength,
                                    float delta, int width, int height) {
  static float minSpeed = 30;
  static float minEffectLength = 10;
  static float fractionSpeed = 0.8f;

  camera->offset = (Vector2){width / 2.0f, height / 2.0f};
  Vector2 diff = Vector2Subtract(player->position, camera->target);
  float length = Vector2Length(diff);

  if (length > minEffectLength) {
    float speed = fmaxf(fractionSpeed * length, minSpeed);
    camera->target =
        Vector2Add(camera->target, Vector2Scale(diff, speed * delta / length));
  }
}

void UpdateCameraEvenOutOnLanding(Camera2D *camera, Character *player,
                                  EnvItem *envItems, int envItemsLength,
                                  float delta, int width, int height) {
  static float evenOutSpeed = 700;
  static int eveningOut = false;
  static float evenOutTarget;

  camera->offset = (Vector2){width / 2.0f, height / 2.0f};
  camera->target.x = player->position.x;

  if (eveningOut) {
    if (evenOutTarget > camera->target.y) {
      camera->target.y += evenOutSpeed * delta;

      if (camera->target.y > evenOutTarget) {
        camera->target.y = evenOutTarget;
        eveningOut = 0;
      }
    } else {
      camera->target.y -= evenOutSpeed * delta;

      if (camera->target.y < evenOutTarget) {
        camera->target.y = evenOutTarget;
        eveningOut = 0;
      }
    }
  } else {
    if (player->canJump && (player->speed == 0) &&
        (player->position.y != camera->target.y)) {
      eveningOut = 1;
      evenOutTarget = player->position.y;
    }
  }
}

void UpdateCameraCharacterBoundsPush(Camera2D *camera, Character *player,
                                     EnvItem *envItems, int envItemsLength,
                                     float delta, int width, int height) {
  static Vector2 bbox = {0.2f, 0.2f};

  Vector2 bboxWorldMin = GetScreenToWorld2D(
      (Vector2){(1 - bbox.x) * 0.5f * width, (1 - bbox.y) * 0.5f * height},
      *camera);
  Vector2 bboxWorldMax = GetScreenToWorld2D(
      (Vector2){(1 + bbox.x) * 0.5f * width, (1 + bbox.y) * 0.5f * height},
      *camera);
  camera->offset =
      (Vector2){(1 - bbox.x) * 0.5f * width, (1 - bbox.y) * 0.5f * height};

  if (player->position.x < bboxWorldMin.x)
    camera->target.x = player->position.x;
  if (player->position.y < bboxWorldMin.y)
    camera->target.y = player->position.y;
  if (player->position.x > bboxWorldMax.x)
    camera->target.x = bboxWorldMin.x + (player->position.x - bboxWorldMax.x);
  if (player->position.y > bboxWorldMax.y)
    camera->target.y = bboxWorldMin.y + (player->position.y - bboxWorldMax.y);
}