#include "engine.h"

enum GameObjectTag {
  TagGround   = (1 << 0),
  TagWall     = (1 << 1),
  TagCoin     = (1 << 2)
};


// SPIELER
GameObject player;
GravitySimulator playerGravSim;
bool playerTouchesGround = false;

// PUNKTESTAND (MÜNZEN)
int score = 0;
Text scoreText;


void spawnPlatform(float x, float y, float w) {
  GameObject plaf = GameObject::createRect(x-w/2, y, w, 5, Color::BLACK);
  plaf.setTag(TagGround);
  game.spawnObject(plaf);
}

void spawnWall(float x, float y, float h) {
  GameObject wall = GameObject::createRect(x-2.5f, y-h, 5, h, Color::BLACK);
  wall.setTag(TagWall);
  game.spawnObject(wall);
}

void spawnCoin(float cx, float cy) {
  GameObject coin = GameObject::createCircle(cx-2, cy-2, 4, Color(0xA440));
  coin.setTag(TagCoin);
  game.spawnObject(coin);
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

  // Text für Punktestand initialisieren
  scoreText = Text(60, 2);
  scoreText.setColor(Color::BLACK);
  scoreText.setSize(2);
  game.addText(scoreText);

  // Spieler spawnen
  player = GameObject::createCircle(-50, 10, 6, ST77XX_BLUE);
  game.spawnObject(player);

  // Schwerkraftsimulation für Spieler festlegen
  playerGravSim.setGameObject(player);

  // Boden spawnen
  spawnPlatform(0, 110, 20000);

  // Diverse Plattformen spawnen
  spawnPlatform(30, 80, 20);
  spawnPlatform(80, 50, 30);
  spawnPlatform(130, 80, 20);

  // Wände spawnen
  spawnWall(80, 110, 30);

  // Münzen spawnen
  spawnCoin(30, 70);
  spawnCoin(80, 40);
  spawnCoin(130, 70);
  spawnCoin(70, 100);
  spawnCoin(90, 100);
}

void gameLoop(float dt) {
  // Spieler bewegen (Buttons links/rechts)
  const float moveSpeed = 1;
  if (game.input().isButtonPressed("left")) {
    player.setX(player.getX()-moveSpeed);
  }
  if (game.input().isButtonPressed("right")) {
    player.setX(player.getX()+moveSpeed);
  }

  // Sprung starten
  if (game.input().isButtonPressed("a")) {
    if (playerTouchesGround) { // Um double-jump zu vermeiden
      playerGravSim.setVelocity(-2.5f); // Sprunggeschwindigkeit, damit indirekt auch Sprunghöhe
    }
  }

  // Schwerkraft- und Sprungsimulation für den Spieler durchführen
  playerGravSim.simulate(dt);

  // Kamera soll sich mit Spieler mitbewegen
  game.setCameraOffset(player.getX() - 30, 0);

  // Punktestand-Text anpassen
  scoreText.setText(String("Coins: ") + String(score));

  // Annahme: Spieler berührt NICHT den Boden. Wird danach durch onCollision() ggf. widerlegt
  playerTouchesGround = false;
}



void onCollision(const GameObjectCollision& coll) {
  if (coll.isInvolved(player)) {
    GameObject other = coll.getOther(player);

    // Spieler kollidiert mit Boden/Plattform
    if (other.hasTag(TagGround)) {
      playerGravSim.reset(); // Nicht durch Boden fallen, Sprung anhalten
      if (player.isAbove(other)) {
        // Spieler kollidiert von oben mit Plattform -> auf Plattform setzen ("pop up from platform")
        player.setY(other.getY() - player.getHeight());
        playerTouchesGround = true;
      }
    }
    
    // Spieler kollidiert mit Wand
    if (other.hasTag(TagWall)) {
      // Spieler auf der jeweiligen Seite direkt an die Wand stellen ("pop out of wall")
      if (player.isLeftOf(other)) {
        player.setX(other.getX() - player.getWidth());
      } else if (player.isRightOf(other)) {
        player.setX(other.getX() + other.getWidth());
      }
    }
    
    // Spieler kollidiert mit Münze
    if (other.hasTag(TagCoin)) {
      game.despawnObject(other);
      score++;
    }
  }
}
