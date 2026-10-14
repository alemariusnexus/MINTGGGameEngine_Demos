#include <MINTGGGameEngine.h>

LOG_USE_TAG("main")

enum GameObjectTag {
  TagRayTest = (1 << 0)
};


Vec2 rayStart(10, 10);
Vec2 rayEnd(150, 118);

bool moveRayStart = false;

Text debugText;

GameObject moveHandle;

// MCP2300X IO expander
GPIODeviceMCP2300X ioExpander(DEMO_RAYCASTING_I2C_PIN_SCL, DEMO_RAYCASTING_I2C_PIN_SDA);


void spawnRayTestCircle(float x, float y, int r)
{
  GameObject circle = GameObject::createCircle(x, y, r, Color::RED, false);
  circle.setTag(TagRayTest);
  game.spawnObject(circle);
}

void spawnRayTestRect(float x, float y, int w, int h)
{
  GameObject rect = GameObject::createRect(x, y, w, h, Color::RED, false);
  rect.setTag(TagRayTest);
  game.spawnObject(rect);
}

void gameSetup() {
  ioExpander.begin();

  // Define buttons
  game.input().defineButton("up", 0, ioExpander);
  game.input().defineButton("down", 1, ioExpander);
  game.input().defineButton("left", 2, ioExpander);
  game.input().defineButton("right", 3, ioExpander);
  game.input().defineButton("a", 4, ioExpander);
  game.input().defineButton("b", 5, ioExpander);
  game.input().defineButton("start", 6, ioExpander);
  game.input().defineButton("joy", 7, ioExpander);

  // Create debug text
  debugText = Text(0, 70);
  debugText.setFont(Font("5x8_ISO8859_1"));
  game.addText(debugText);

  //
  moveHandle = GameObject::createCircle(0, 0, 3, Color::GREEN, false);
  game.spawnObject(moveHandle);

  // Spawn test objects
  spawnRayTestCircle(120, 10, 15);
  spawnRayTestCircle(30, 20, 25);
  spawnRayTestRect(50, 10, 60, 40);

  // Draw results of the ray casts on the screen
  game.setDrawRayCasts(true);
}

void gameLoop(float dt) {
  const float rayStep = 1.0f;

  // Should the directional buttons move the start or end point of the ray?
  if (game.input().isButtonPressedThisFrame("a")) {
      moveRayStart = !moveRayStart;
  }

  // Move start/end point of the ray by the directional buttons
  Vec2& rayPoint = moveRayStart ? rayStart : rayEnd;
  if (game.input().isButtonPressed("left")) {
    rayPoint.setX(rayPoint.x()-rayStep);
  }
  if (game.input().isButtonPressed("right")) {
    rayPoint.setX(rayPoint.x()+rayStep);
  }
  if (game.input().isButtonPressed("up")) {
    rayPoint.setY(rayPoint.y()-rayStep);
  }
  if (game.input().isButtonPressed("down")) {
    rayPoint.setY(rayPoint.y()+rayStep);
  }

  moveHandle.setCenterPosition(moveRayStart ? rayStart : rayEnd);

  // Perform ray casting
  auto timeStart = TimerGetTickcountUs();
  RayCastResult res = game.castRay(rayStart, rayEnd, game.getGameObjectsWithTag(TagRayTest));
  auto timeEnd = TimerGetTickcountUs();

  // Show result
  string debugStr;
  debugStr += " FPS: "s + FloatToString(1.0f/dt, 1);
  debugStr += ", T: "s + to_string(timeEnd-timeStart) + "us\n"s;
  for (RayCastHit hit : res.getHits()) {
    debugStr += " "s;
    debugStr += hit.entering ? "enter"s : "exit "s;
    debugStr += " "s + FloatToString(hit.rayOffset, 1);
    debugStr += " ("s + FloatToString(hit.hitPoint.x(), 1) + ", "s + FloatToString(hit.hitPoint.y(), 1) + ")\n"s;
  }
  debugText.setText(debugStr);
}

void postDraw(float dt)
{
}

void onCollision(const GameObjectCollision& coll) {
}
