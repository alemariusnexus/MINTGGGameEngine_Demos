#include "engine.h"

enum GameObjectTag {
  TagRayTest = (1 << 0)
};


Vec2 rayStart(10, 10); // Anfangspunkt
Vec2 rayEnd(150, 118); // Endpunkt

bool moveRayStart = false;

Text debugText;


void spawnRayTestCircle(float x, float y, int r)
{
  GameObject circle = GameObject::createCircle(x, y, r, ST77XX_RED, false);
  circle.setTag(TagRayTest);
  game.spawnObject(circle);
}

void spawnRayTestRect(float x, float y, int w, int h)
{
  GameObject rect = GameObject::createRect(x, y, w, h, ST77XX_RED, false);
  rect.setTag(TagRayTest);
  game.spawnObject(rect);
}

void gameSetup() {
  // Buttons definieren
  game.input().defineButtonMCP23009("up", 0);
  game.input().defineButtonMCP23009("down", 1);
  game.input().defineButtonMCP23009("left", 2);
  game.input().defineButtonMCP23009("right", 3);
  game.input().defineButtonMCP23009("a", 4);
  game.input().defineButtonMCP23009("b", 5);
  game.input().defineButtonMCP23009("start", 6);
  game.input().defineButtonMCP23009("joy", 7);

  // Debugtext erstellen
  debugText = Text(0, 70);
  game.addText(debugText);

  // Testobjekte spawnen
  spawnRayTestCircle(120, 10, 15);
  spawnRayTestCircle(30, 20, 25);
  spawnRayTestRect(50, 10, 60, 40);

  // Ergebnis der RayCasts auf den Bildschirm zeichnen
  game.setDrawRayCasts(true);
}

void gameLoop(float dt) {
  const float rayStep = 1.0f;

  // Soll der Start- oder Endpunkt des Rays mit Richtungstasten verschoben werden?
  if (game.input().isButtonPressed("a")) {
    moveRayStart = true;
  } else if (game.input().isButtonPressed("b")) {
    moveRayStart = false;
  }

  // Start-/Endpunkt des Rays mit den Richtungstasten verschieben
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

  // Raycasting durchführen
  unsigned long timeStart = micros();
  RayCastResult res = game.castRay(rayStart, rayEnd, game.getGameObjectsWithTag(TagRayTest));
  unsigned long timeEnd = micros();

  // Ergebnis anzeigen
  String debugStr;
  debugStr += String(" FPS: ") + String(1.0f/dt, 1);
  debugStr += String(", T: ") + String(timeEnd-timeStart) + String("us\n");
  for (RayCastHit hit : res.getHits()) {
    debugStr += " ";
    debugStr += String(hit.entering ? "enter" : "exit ");
    debugStr += String(" ") + String(hit.rayOffset, 1);
    debugStr += String(" (") + String(hit.hitPoint.x(), 1) + String(", ") + String(hit.hitPoint.y(), 1) + String(")\n");
  }
  debugText.setText(debugStr);
}



void onCollision(const GameObjectCollision& coll) {
}
