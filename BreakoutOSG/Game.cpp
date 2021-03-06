#include "Game.h"
#include "ResourceManager.h"
#include "SpriteRender.h"
#include "GameObject.h"
#include "BallObject.h"
#include "ParticleGenerator.h"
#include "PostProcessor.h"

SpriteRender* Renderer;
GameObject* Player;
BallObject* Ball;
ParticleGenerator* Particles;
PostProcessor* Effects;
GLfloat ShakeTime = 0.0f;

Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.0f, -1.0f),
		glm::vec2(-1.0f, 0.0f)
	};

	GLfloat max = 0.0f;
	GLuint best_match = -1;
	for (GLuint i = 0; i < 4; i++)
	{
		GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}

GLboolean CheckCollision(GameObject& one, GameObject& two)
{
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
		two.Position.x + two.Size.x >= one.Position.x;
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
		two.Position.y + two.Size.y >= one.Position.y;

	return collisionX && collisionY;
}

Collision CheckCollision(BallObject& one, GameObject& two)
{
	glm::vec2 center(one.Position + one.Radius);
	glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	glm::vec2 closest = aabb_center + clamped;

	difference = closest - center;
	if (glm::length(difference) < one.Radius)
	{
		return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
	}
	else
	{
		return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
	}
}

Game::Game(GLuint width, GLuint height):
	State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{   
	GameObject one, two;
}

Game::~Game()
{
	delete Renderer;
	delete Player;
	delete Ball;
	delete Particles;
	delete Effects;
}

void Game::Init()
{
	ResourceManager::LoadShader("../shaders/sprite.vs", "../shaders/sprite.frag", nullptr, "sprite");
	ResourceManager::LoadShader("../shaders/particle.vs", "../shaders/particle.frag", nullptr, "particle");
	ResourceManager::LoadShader("../shaders/post_processing.vs", "../shaders/post_processing.frag", nullptr, "postprocessing");

	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projection);

	ResourceManager::LoadTexture("../resources/textures/background.jpg", GL_FALSE, "background");
	ResourceManager::LoadTexture("../resources/textures/awesomeface.png", GL_TRUE, "face");
	ResourceManager::LoadTexture("../resources/textures/block.png", GL_FALSE, "block");
	ResourceManager::LoadTexture("../resources/textures/block_solid.png", GL_FALSE, "block_solid");
	ResourceManager::LoadTexture("../resources/textures/paddle.png", true, "paddle");
	ResourceManager::LoadTexture("../resources/textures/particle.png", GL_TRUE, "particle");

	Shader spriteShader = ResourceManager::GetShader("sprite");
	Renderer = new SpriteRender(spriteShader);
	Shader particleShader = ResourceManager::GetShader("particle");
	Texture2D particleTexture = ResourceManager::GetTexture("particle");
	Particles = new ParticleGenerator(particleShader, particleTexture, 500);
	Shader postprocessingShader = ResourceManager::GetShader("postprocessing");
	Effects = new PostProcessor(postprocessingShader, this->Width, this->Height);

	GameLevel one;
	one.Load("../resources/levels/one.lvl", this->Width, this->Height*0.5);
	GameLevel two;
	two.Load("../resources/levels/two.lvl", this->Width, this->Height*0.5);
	GameLevel three;
	three.Load("../resources/levels/three.lvl", this->Width, this->Height*0.5);
	GameLevel four;
	four.Load("../resources/levels/four.lvl", this->Width, this->Height*0.5);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;

	glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Texture2D paddleTexture = ResourceManager::GetTexture("paddle");
	Player = new GameObject(playerPos, PLAYER_SIZE, paddleTexture);

	Texture2D faceTexture = ResourceManager::GetTexture("face");
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, faceTexture);
}

void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0)
			{
				Player->Position.x -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
			}
		}

		if (this->Keys[GLFW_KEY_SPACE])
		{
			Ball->Stuck = false;
		}
	}
}

void Game::Update(GLfloat dt)
{
	Ball->Move(dt, this->Width);
	this->DoCollisions();
	Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2));

	if (ShakeTime > 0.0f)
	{
		ShakeTime -= dt;
		if (ShakeTime <= 0.0f)
		{
			Effects->Shake = GL_FALSE;
		}
	}

	if (Ball->Position.y >= this->Height)
	{
		this->ResetLevel();
		this->ResetPlayer();
	}
}

void Game::DoCollisions()
{
	for (GameObject& box : this->Levels[this->Level].Bricks)
	{
		if (!box.Destroyed)
		{
			Collision collision = CheckCollision(*Ball, box);
			if (std::get<0>(collision))
			{
				if (!box.IsSolid)
				{
					box.Destroyed = GL_TRUE;
				}
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);
				if (dir == LEFT || dir == RIGHT)
				{
					Ball->Velocity.x = -Ball->Velocity.x;
					GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
					if (dir == LEFT)
					{
						Ball->Position.x += penetration;
					}
					else
					{
						Ball->Position.x -= penetration;
					}
				}
				else
				{
					Ball->Velocity.y = -Ball->Velocity.y;
					GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
					if (dir == UP)
					{
						Ball->Position.y -= penetration;
						Ball->Position.y += penetration;
					}
				}
			}
		}
	}

	Collision result = CheckCollision(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(result))
	{
		GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
		GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		GLfloat percentage = distance / (Player->Size.x / 2);

		GLfloat strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
		Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
	}
}

void Game::ResetLevel()
{
	if (this->Level == 0)
	{
		this->Levels[0].Load("../resources/levels/one.lvl", this->Width, this->Height*0.5f);
	}
	else if (this->Level == 1)
	{
		this->Levels[1].Load("../resources/levels/two.lvl", this->Width, this->Height * 0.5);
	}
	else if (this->Level == 2)
	{
		this->Levels[2].Load("../resources/levels/three.lvl", this->Width, this->Height * 0.5);
	}
	else if (this->Level == 3)
	{
		this->Levels[3].Load("../resources/levels/four.lvl", this->Width, this->Height * 0.5f);
	}
}

void Game::ResetPlayer()
{
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)), INITIAL_BALL_VELOCITY);
}

void Game::Render()
{   
	
	if (this->State == GAME_ACTIVE)
	{  
		Effects->BeginRender();
		Texture2D backgroundTexture = ResourceManager::GetTexture("background");
		Renderer->DrawSprite(backgroundTexture, glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);
		this->Levels[this->Level].Draw(*Renderer);
		Player->Draw(*Renderer);
		Particles->Draw();
		Ball->Draw(*Renderer);
		Effects->EndRender();
		Effects->Render(glfwGetTime());
	}
}

