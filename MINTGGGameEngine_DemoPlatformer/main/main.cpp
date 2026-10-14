#include <MINTGGGameEngine.h>

LOG_USE_TAG("main")

enum GameObjectTag {
  TagGround   = (1 << 0),
  TagWall     = (1 << 1),
  TagCoin     = (1 << 2)
};


// PLAYER
GameObject player;
GravitySimulator playerGravSim;
bool playerTouchesGround = false;

// SCORE (COINS)
int score = 0;
Text scoreText;

// MCP2300X IO EXPANDER
GPIODeviceMCP2300X ioExpander(DEMO_PLATFORMER_I2C_PIN_SCL, DEMO_PLATFORMER_I2C_PIN_SDA);


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
  GameObject coin = GameObject::createCircle(cx-2, cy-2, 4, Color::fromRGB565(0xA440));
  coin.setTag(TagCoin);
  game.spawnObject(coin);
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

  // Initialize score text
  scoreText = Text(2, 2);
  scoreText.setColor(Color::BLACK);
  scoreText.setScale(2);
  game.addText(scoreText);

  // Spawn player
  player = GameObject::createCircle(-50, 10, 6, Color::BLUE, true, false);
  player.setCollider(Collider::createRect(0, 0, 12, 12));
  game.spawnObject(player);

  // Apply gravity simulation to player
  playerGravSim.setGameObject(player);

  // Spawn ground
  spawnPlatform(0, 110, 20000);

  // Spawn a bunch of platforms
  spawnPlatform(30, 80, 20);
  spawnPlatform(80, 50, 30);
  spawnPlatform(130, 80, 20);

  // Spawn walls
  spawnWall(80, 110, 30);

  // Spawn coins
  spawnCoin(30, 70);
  spawnCoin(80, 40);
  spawnCoin(130, 70);
  spawnCoin(70, 100);
  spawnCoin(90, 100);
}

void gameLoop(float dt) {
  // Toggle drawing colliders for debugging
  if (game.input().isButtonPressedThisFrame("start")) {
      game.setDrawColliders(!game.isDrawColliders());
  }

  // Move player (buttons left/right)
  const float moveSpeed = 1;
  if (game.input().isButtonPressed("left")) {
    player.setX(player.getX()-moveSpeed);
  }
  if (game.input().isButtonPressed("right")) {
    player.setX(player.getX()+moveSpeed);
  }

  // Start jump
  if (game.input().isButtonPressed("a")) {
    if (playerTouchesGround) { // To prevent double jumping
      playerGravSim.setVelocity(-2.5f); // Jump speed, which indirectly determines jump height
    }
  }

  // Perform gravity and jump simulation for player
  playerGravSim.simulate(dt);

  // Camera should move with the player
  game.setCameraOffset(player.getX() - 30, 0);

  // Adjust score text
  scoreText.setText("Coins: "s + to_string(score));

  // Assumption: Player does NOT touch ground. Might be disproven later by onCollision().
  playerTouchesGround = false;
}

void postDraw(float dt)
{
}

void onCollision(const GameObjectCollision& coll) {
  if (coll.isInvolved(player)) {
    GameObject other = coll.getOther(player);

    // Player collides with ground/platforms
    if (other.hasTag(TagGround)) {
      playerGravSim.reset(); // Don't fall through ground -> stop jump
      if (player.isAbove(other)) {
        // Player collides with platform from above -> pop them up from the platform
        player.setY(other.getY() - player.getHeight());
        playerTouchesGround = true;
      } else if (player.isBelow(other)) {
        // Player collides with platform from below -> pop them down from the platform
        player.setY(other.getY() + other.getHeight());
      }
    }

    // Player collides with wall
    if (other.hasTag(TagWall)) {
      // Pop player out of wall
      if (player.isAbove(other)) {
        playerGravSim.reset(); // Don't fall through ground -> stop jump
        player.setY(other.getY() - player.getHeight());
      } else if (player.isLeftOf(other)) {
        player.setX(other.getX() - player.getWidth());
      } else if (player.isRightOf(other)) {
        player.setX(other.getX() + other.getWidth());
      }
    }
    
    // Player collides with coin
    if (other.hasTag(TagCoin)) {
      game.despawnObject(other);
      score++;
    }
  }
}
