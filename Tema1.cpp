#include "lab_m1/Tema1/Tema1.h"

#include <stdlib.h>
#include <vector>
#include <iostream>

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/object2D.h"
#include "lab_m1/Tema1/Wall.h"
#include "lab_m1/Tema1/Projectile.h"
#include "lab_m1/Tema1/Obstacle.h"
#include "lab_m1/Tema1/Enemy.h"
#include "lab_m1/Tema1/HpUp.h"
#include "lab_m1/Tema1/ShotgunPowerUp.h"

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

list<Projectile> projectiles;
list<Enemy> enemies;
list<HpUp> hpUps;
list<ShotgunPowerUp> shotgunPowerUps;

vector<Obstacle> obstacles = {
    * (new Obstacle(5, 15, 35, 40)),
    * (new Obstacle(10, 15, 30, 35)),
    * (new Obstacle(25, 28, 40, 43)),
    * (new Obstacle(35, 37, 35, 37)),
    * (new Obstacle(45, 50, 30, 45)),
    * (new Obstacle(35, 40, 25, 30)),
    * (new Obstacle(10, 15, 10, 20)),
    * (new Obstacle(20, 25, 5, 10)),
    * (new Obstacle(25, 40, 15, 20)),
    * (new Obstacle(35, 40, 10, 15))
};


Tema1::Tema1()
{
}


Tema1::~Tema1()
{
}


void Tema1::Init()
{
    auto camera = GetSceneCamera();
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    logicSpace.x = playerX - PLAYER_LOGICSPACE_OFFSET;       // logic x
    logicSpace.y = playerY - PLAYER_LOGICSPACE_OFFSET;       // logic y
    logicSpace.width = 25;   // logic width
    logicSpace.height = 25;  // logic height

    glm::vec3 corner = glm::vec3(0.001, 0.001, 0);
    length = 0.99f;

    Mesh* square = Object2D::CreateSquare("square", glm::vec3(0), 1, GREY, true);
    AddMeshToList(square);

    Mesh* empty_square = Object2D::CreateSquare("empty_square", glm::vec3(0), 1, RED, false);
    AddMeshToList(empty_square);

    Mesh* circle = Object2D::CreateCircle("circle", GRASS_COLOR);
    AddMeshToList(circle);

    // mesh for projectile
    Mesh* centered_square = Object2D::CreateSquare("centered_square", glm::vec3(-bullet_length / 2, -bullet_length / 2, 0), bullet_length, BLACK, true);
    AddMeshToList(centered_square);
}


// 2D visualization matrix
glm::mat3 Tema1::VisualizationTransf2D(const LogicSpace & logicSpace, const ViewportSpace & viewSpace)
{
    float sx, sy, tx, ty;
    sx = viewSpace.width / logicSpace.width;
    sy = viewSpace.height / logicSpace.height;
    tx = viewSpace.x - sx * logicSpace.x;
    ty = viewSpace.y - sy * logicSpace.y;

    return glm::transpose(glm::mat3(
        sx, 0.0f, tx,
        0.0f, sy, ty,
        0.0f, 0.0f, 1.0f));
}


// Uniform 2D visualization matrix (same scale factor on x and y axes)
glm::mat3 Tema1::VisualizationTransf2DUnif(const LogicSpace & logicSpace, const ViewportSpace & viewSpace)
{
    float sx, sy, tx, ty, smin;
    sx = viewSpace.width / logicSpace.width;
    sy = viewSpace.height / logicSpace.height;
    if (sx < sy)
        smin = sx;
    else
        smin = sy;
    tx = viewSpace.x - smin * logicSpace.x + (viewSpace.width - smin * logicSpace.width) / 2;
    ty = viewSpace.y - smin * logicSpace.y + (viewSpace.height - smin * logicSpace.height) / 2;

    return glm::transpose(glm::mat3(
        smin, 0.0f, tx,
        0.0f, smin, ty,
        0.0f, 0.0f, 1.0f));
}


void Tema1::SetViewportArea(const ViewportSpace & viewSpace, glm::vec3 colorColor, bool clear)
{
    glViewport(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(colorColor.r, colorColor.g, colorColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    GetSceneCamera()->SetOrthographic((float)viewSpace.x, (float)(viewSpace.x + viewSpace.width), (float)viewSpace.y, (float)(viewSpace.y + viewSpace.height), 0.1f, 400);
    GetSceneCamera()->Update();
}


void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Tema1::Update(float deltaTimeSeconds)
{
    glm::ivec2 resolution = window->GetResolution();

    // Sets the screen area where to draw - the left half of the window
    viewSpace = ViewportSpace(0, 0, resolution.x, resolution.y);
    SetViewportArea(viewSpace, NAVY_BLUE, true);

    // Compute the 2D visualization matrix
    visMatrix = glm::mat3(1);
    visMatrix *= VisualizationTransf2DUnif(logicSpace, viewSpace);

    if (hp > 0 && xp < XP_GOAL) {

        // HP bar
        modelMatrix = visMatrix * transform2D::Translate(logicSpace.x + logicSpace.width + 3, logicSpace.y + logicSpace.height - 2);
        modelMatrix *= transform2D::Scale(hp / 5, 1);
        RenderMesh2D(meshes["square"], modelMatrix, RED);
        modelMatrix = visMatrix * transform2D::Translate(logicSpace.x + logicSpace.width + 3, logicSpace.y + logicSpace.height - 2);
        modelMatrix *= transform2D::Scale(4, 1);
        RenderMesh2D(meshes["empty_square"], modelMatrix, RED);

        // XP bar
        modelMatrix = visMatrix * transform2D::Translate(logicSpace.x + logicSpace.width + 3, logicSpace.y + logicSpace.height - 4);
        modelMatrix *= transform2D::Scale(xp / 5, 1);
        RenderMesh2D(meshes["square"], modelMatrix, YELLOW);
        modelMatrix = visMatrix * transform2D::Translate(logicSpace.x + logicSpace.width + 3, logicSpace.y + logicSpace.height - 4);
        modelMatrix *= transform2D::Scale(6, 1);
        RenderMesh2D(meshes["empty_square"], modelMatrix, YELLOW);


        // player right eye
        modelMatrix = visMatrix;
        modelMatrix *= transform2D::Translate(playerX, playerY);
        modelMatrix *= transform2D::Rotate(eye_angle + M_PI / 4);
        modelMatrix *= transform2D::Translate(1, 0);
        modelMatrix *= transform2D::Scale(0.3, 0.3);
        RenderMesh2D(meshes["circle"], modelMatrix, YELLOW);


        // player left eye
        modelMatrix = visMatrix;
        modelMatrix *= transform2D::Translate(playerX, playerY);
        modelMatrix *= transform2D::Rotate(eye_angle - M_PI / 4);
        modelMatrix *= transform2D::Translate(1, 0);
        modelMatrix *= transform2D::Scale(0.3, 0.3);
        RenderMesh2D(meshes["circle"], modelMatrix, YELLOW);


        // player body
        modelMatrix = visMatrix * transform2D::Translate(playerX, playerY);
        RenderMesh2D(meshes["circle"], shaders["VertexColor"], modelMatrix);


        // shotgun time remaining
        if (shotgun_time > 0) {
            shotgun_time -= deltaTimeSeconds;
        }

        // shotgun power-up spawning
        time_until_shotgun_spawn -= deltaTimeSeconds;
        if (time_until_shotgun_spawn <= 0) {

            float sgX, sgY;
            do {
                sgX = rand() % 49 + 1;
                sgY = rand() % 49 + 1;
            } while (sqrt((sgX - playerX) * (sgX - playerX) + (sgY - playerY) * (sgY - playerY)) < 7);

            shotgunPowerUps.push_back(ShotgunPowerUp(sgX, sgY));
            time_until_shotgun_spawn = 25;
        }

        //shotgunpowerups
        std::list<ShotgunPowerUp>::iterator sgIt = shotgunPowerUps.begin();
        while (sgIt != shotgunPowerUps.end()) {

            modelMatrix = visMatrix * transform2D::Translate(sgIt->x, sgIt->y);
            modelMatrix *= transform2D::Scale(0.5, 0.5);
            RenderMesh2D(meshes["circle"], modelMatrix, DARKER_SKIN_COLOR);


            // player - shotgun power-up collision
            if (sqrt((sgIt->y - playerY) * (sgIt->y - playerY) + (sgIt->x - playerX) * (sgIt->x - playerX)) < 1.5) {
                shotgunPowerUps.erase(sgIt++);
                shotgun_time = 10;
            }
            else {
                sgIt++;
            }
        }

        // hp-ups spawning
        time_until_hpup_spawn -= deltaTimeSeconds;
        if (time_until_hpup_spawn <= 0) {

            float hpX, hpY;
            do {
                hpX = rand() % 49 + 1;
                hpY = rand() % 49 + 1;
            } while (sqrt((hpX - playerX) * (hpX - playerX) + (hpY - playerY) * (hpY - playerY)) < 7);

            hpUps.push_back(HpUp(hpX, hpY));
            time_until_hpup_spawn = 20;
        }

        // HpUps
        std::list<HpUp>::iterator hpIt = hpUps.begin();
        while (hpIt != hpUps.end()) {

            modelMatrix = visMatrix * transform2D::Translate(hpIt->x, hpIt->y);
            modelMatrix *= transform2D::Scale(0.3, 1);
            RenderMesh2D(meshes["centered_square"], shaders["VertexColor"], modelMatrix);

            modelMatrix = visMatrix * transform2D::Translate(hpIt->x, hpIt->y);
            modelMatrix *= transform2D::Scale(1, 0.3);
            RenderMesh2D(meshes["centered_square"], shaders["VertexColor"], modelMatrix);

            modelMatrix = visMatrix * transform2D::Translate(hpIt->x, hpIt->y);
            modelMatrix *= transform2D::Scale(0.5, 0.5);
            RenderMesh2D(meshes["circle"], modelMatrix, GREEN);

            
            // player - hpup collision
            if (sqrt((hpIt->y - playerY) * (hpIt->y - playerY) + (hpIt->x - playerX) * (hpIt->x - playerX)) < 1.5) {
                hpUps.erase(hpIt++);
                hp = min(hp + 3, (float )20);

            } else {
                hpIt++;
            }
        }

        // fire rate
        if (time_until_fire > 0) {
            time_until_fire -= deltaTimeSeconds;
        }

        // projectiles
        std::list<Projectile>::iterator it = projectiles.begin();
        while (it != projectiles.end())
        {

            it->x += BULLET_SPEED * deltaTimeSeconds * cos(it->angle);
            it->y += BULLET_SPEED * deltaTimeSeconds * sin(it->angle);
            it->distance += BULLET_SPEED * deltaTimeSeconds;

            modelMatrix = visMatrix * transform2D::Translate(it->x, it->y);
            modelMatrix *= transform2D::Rotate(it->angle);
            RenderMesh2D(meshes["centered_square"], shaders["VertexColor"], modelMatrix);

            bool ok = true;

            // projectile - obstacle collision
            for (int i = 0; i < obstacles.size(); i++) {
                if (it->x > obstacles[i].x1 - bullet_length / 2 && it->x < obstacles[i].x2 + bullet_length / 2
                    && it->y > obstacles[i].y1 - bullet_length / 2 && it->y < obstacles[i].y2 + bullet_length / 2) {
                    ok = false;
                }
            }

            // "maximum distance reached" case
            if (it->distance >= MAX_BULLET_DISTANCE || it->x >= 50 - bullet_length / 2 || it->x <= bullet_length / 2
                || it->y >= 50 - bullet_length / 2 || it->y <= bullet_length / 2) {
                ok = false;
            }

            // projectile - enemy collision
            std::list<Enemy>::iterator enemy_it = enemies.begin();
            while (enemy_it != enemies.end()) {
                if (sqrt((enemy_it->y - it->y) * (enemy_it->y - it->y) + (enemy_it->x - it->x) * (enemy_it->x - it->x)) < 1.25) {
                    enemies.erase(enemy_it);
                    xp++;
                    ok = false;
                    break;
                }
                enemy_it++;
            }

            if (ok == false) {
                projectiles.erase(it++);
            }
            else {
                it++;
            }
        }

        //enemies spawning
        time_until_enemy_spawn -= deltaTimeSeconds;
        if (time_until_enemy_spawn <= 0) {
            float speed = rand() % 8 + 5;
            float startX, startY;

            do {
                startX = rand() % 49 + 1;
                startY = rand() % 49 + 1;
            } while (sqrt((startX - playerX) * (startX - playerX) + (startY - playerY) * (startY - playerY)) < 7);

            enemies.push_back(Enemy(startX, startY, speed));
            time_until_enemy_spawn = 2.5;
        }

        // enemies 
        std::list<Enemy>::iterator iter = enemies.begin();
        while (iter != enemies.end())
        {
            float enemy_player_angle = (-1) * atan2((float)(iter->y - playerY), (float)(iter->x - playerX));

            iter->x += iter->speed * deltaTimeSeconds * (-1) * cos(enemy_player_angle);
            iter->y += iter->speed * deltaTimeSeconds * sin(enemy_player_angle);

            modelMatrix = visMatrix * transform2D::Translate(iter->x, iter->y);
            RenderMesh2D(meshes["circle"], modelMatrix, RED);

            modelMatrix = visMatrix;
            modelMatrix *= transform2D::Translate(iter->x, iter->y);
            modelMatrix *= transform2D::Rotate(-enemy_player_angle);
            modelMatrix *= transform2D::Translate(-1, 0);
            modelMatrix *= transform2D::Scale(0.3, 0.3);
            RenderMesh2D(meshes["circle"], modelMatrix, BLACK);

            modelMatrix = visMatrix;
            modelMatrix *= transform2D::Translate(iter->x, iter->y);
            modelMatrix *= transform2D::Rotate(-enemy_player_angle + M_PI / 4);
            modelMatrix *= transform2D::Translate(-1, 0);
            modelMatrix *= transform2D::Scale(0.3, 0.3);
            RenderMesh2D(meshes["circle"], modelMatrix, BLACK);

            modelMatrix = visMatrix;
            modelMatrix *= transform2D::Translate(iter->x, iter->y);
            modelMatrix *= transform2D::Rotate(-enemy_player_angle - M_PI / 4);
            modelMatrix *= transform2D::Translate(-1, 0);
            modelMatrix *= transform2D::Scale(0.3, 0.3);
            RenderMesh2D(meshes["circle"], modelMatrix, BLACK);

            // player - enemy collision
            if (sqrt((iter->y - playerY) * (iter->y - playerY) + (iter->x - playerX) * (iter->x - playerX)) < 2) {
                enemies.erase(iter++);
                hp--;
            }
            else {
                iter++;
            }
        }

        DrawScene(visMatrix);

    } else  if (hp <= 0) {
        
        DrawDeadScene(visMatrix);
    } else if (xp >= 20) {
        DrawOkScene(visMatrix);
    }

}


void Tema1::FrameEnd()
{
}


void Tema1::DrawScene(glm::mat3 visMatrix)
{
    //obstacles
    for (int i = 0; i < obstacles.size(); i++) {
        modelMatrix = visMatrix * transform2D::Translate(obstacles[i].x1, obstacles[i].y1);
        modelMatrix *= transform2D::Scale(obstacles[i].x2 - obstacles[i].x1, obstacles[i].y2 - obstacles[i].y1);
        RenderMesh2D(meshes["square"], modelMatrix, AVATAR_BLUE);
    }

    // board
    modelMatrix = visMatrix * transform2D::Scale(50, 50);
    RenderMesh2D(meshes["square"], shaders["VertexColor"], modelMatrix);
}

void Tema1::DrawDeadScene(glm::mat3 visMatrix)
{
    // D
    modelMatrix = visMatrix * transform2D::Translate(playerX - 18, playerY - 5);
    modelMatrix *= transform2D::Scale(2, 10);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX - 16, playerY - 5);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX - 16, playerY + 3);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX - 12, playerY - 3);
    modelMatrix *= transform2D::Scale(2, 6);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    // E
    modelMatrix = visMatrix * transform2D::Translate(playerX - 8, playerY - 5);
    modelMatrix *= transform2D::Scale(2, 10);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX - 6, playerY - 5);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX - 6, playerY - 1);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX - 6, playerY + 3);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    // A

    modelMatrix = visMatrix * transform2D::Translate(playerX, playerY - 5);
    modelMatrix *= transform2D::Scale(2, 10);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX + 6, playerY - 5);
    modelMatrix *= transform2D::Scale(2, 10);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX + 2, playerY + 3);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX + 2, playerY - 1);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    // D

    modelMatrix = visMatrix * transform2D::Translate(playerX + 10, playerY - 5);
    modelMatrix *= transform2D::Scale(2, 10);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX + 12, playerY - 5);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX + 12, playerY + 3);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX + 16, playerY - 3);
    modelMatrix *= transform2D::Scale(2, 6);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    // red board
    modelMatrix = visMatrix * transform2D::Translate(playerX - 50, playerY - 50);
    modelMatrix *= transform2D::Scale(100, 100);
    RenderMesh2D(meshes["square"], modelMatrix, RED);
}

void Tema1::DrawOkScene(glm::mat3 visMatrix) {

    // O
    modelMatrix = visMatrix * transform2D::Translate(playerX - 8, playerY - 3);
    modelMatrix *= transform2D::Scale(2, 6);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX - 6, playerY - 5);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX - 6, playerY + 3);
    modelMatrix *= transform2D::Scale(4, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX - 2, playerY - 3);
    modelMatrix *= transform2D::Scale(2, 6);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    // K
    modelMatrix = visMatrix * transform2D::Translate(playerX + 2, playerY - 5);
    modelMatrix *= transform2D::Scale(2, 10);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX + 4, playerY - 1);
    modelMatrix *= transform2D::Scale(2, 2);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX + 6, playerY + 1);
    modelMatrix *= transform2D::Scale(2, 4);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);

    modelMatrix = visMatrix * transform2D::Translate(playerX + 6, playerY - 5);
    modelMatrix *= transform2D::Scale(2, 4);
    RenderMesh2D(meshes["square"], modelMatrix, BLACK);


    // green board
    modelMatrix = visMatrix * transform2D::Translate(playerX - 50, playerY - 50);
    modelMatrix *= transform2D::Scale(100, 100);
    RenderMesh2D(meshes["square"], modelMatrix, GREEN);
}



/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    // TODO(student): Move the logic window with W, A, S, D (up, left, down, right)

    if (window->KeyHold(GLFW_KEY_A) && hp > 0) {
        l = deltaTime * 10;

        bool ok = true;

        for (int i = 0; i < obstacles.size(); i++) {
            if (!(playerY + 1 <= obstacles[i].y1) && !(playerY - 1 >= obstacles[i].y2)) {
                if ((playerX - 1 >= obstacles[i].x2) && (playerX - 1 - l < obstacles[i].x2)) {
                    ok = false;
                    playerX = obstacles[i].x2 + 1;
                    logicSpace.x = obstacles[i].x2 + 1 - PLAYER_LOGICSPACE_OFFSET;
                    break;
                }
            }
        }

        if (ok == true) {
            if (playerX - 1 - l >= 0) {
                playerX -= l;
                logicSpace.x -= l;
            }
            else {
                playerX = 1;
                logicSpace.x = 1 - PLAYER_LOGICSPACE_OFFSET;
            }
        }
    }

    if (window->KeyHold(GLFW_KEY_D) && hp > 0) {
        l = deltaTime * 10;

        
        bool ok = true;

        for (int i = 0; i < obstacles.size(); i++) {
            if (!(playerY + 1 <= obstacles[i].y1) && !(playerY - 1 >= obstacles[i].y2)) {
                if ((playerX + 1 <= obstacles[i].x1) && (playerX + 1 + l > obstacles[i].x1)) {
                    ok = false;
                    playerX = obstacles[i].x1 - 1;
                    logicSpace.x = obstacles[i].x1 - 1 - PLAYER_LOGICSPACE_OFFSET;
                    break;
                }
            }
        }

        if (ok == true) {
            if (playerX + 1 + l <= 50) {
                playerX += l;
                logicSpace.x += l;
            }
            else {
                playerX = 49;
                logicSpace.x = 49 - PLAYER_LOGICSPACE_OFFSET;
            }
        }
    }

    if (window->KeyHold(GLFW_KEY_W) && hp > 0) {
        l = deltaTime * 10;
        
        bool ok = true;

        for (int i = 0; i < obstacles.size(); i++) {

            if (!(playerX + 1 <= obstacles[i].x1) && !(playerX - 1 >= obstacles[i].x2)) {
                if ((playerY + 1 <= obstacles[i].y1) && (playerY + 1 + l > obstacles[i].y1)) {
                    ok = false;
                    playerY = obstacles[i].y1 - 1;
                    logicSpace.y = obstacles[i].y1 - 1 - PLAYER_LOGICSPACE_OFFSET;
                    break;
                }
            }
        }


        if (ok == true) {
            if (playerY + 1 + l <= 50) {
                playerY += l;
                logicSpace.y += l;
            }
            else {
                playerY = 49;
                logicSpace.y = 49 - PLAYER_LOGICSPACE_OFFSET;
            }
        }
    }

    if (window->KeyHold(GLFW_KEY_S) && hp > 0) {
        l = deltaTime * 10;

        bool ok = true;

        for (int i = 0; i < obstacles.size(); i++) {
            if (!(playerX + 1 <= obstacles[i].x1) && !(playerX - 1 >= obstacles[i].x2)) {
                if ((playerY - 1 >= obstacles[i].y2) && (playerY - 1 - l < obstacles[i].y2)) {
                    ok = false;
                    playerY = obstacles[i].y2 + 1;
                    logicSpace.y = obstacles[i].y2 + 1 - PLAYER_LOGICSPACE_OFFSET;
                    break;
                }
            }
        }

        if (ok == true) {
            if (playerY - 1 - l >= 0) {
                playerY -= l;
                logicSpace.y -= l;
            }
            else {
                playerY = 1;
                logicSpace.y = 1 - PLAYER_LOGICSPACE_OFFSET;
            }
        }
        
    }

    if (window->KeyHold(GLFW_KEY_R)) {
        hp = 20;
        xp = 0;
        playerX = PLAYERSTARTX;
        playerY = PLAYERSTARTY;
        logicSpace.x = playerX - PLAYER_LOGICSPACE_OFFSET;
        logicSpace.y = playerY - PLAYER_LOGICSPACE_OFFSET;
        enemies.clear();
        projectiles.clear();
        hpUps.clear();
        shotgunPowerUps.clear();
        time_until_fire = 0;
        time_until_enemy_spawn = 2.5;
        time_until_hpup_spawn = 20;
        time_until_shotgun_spawn = 25;
        shotgun_time = 0;
    }
}


void Tema1::OnKeyPress(int key, int mods)
{
}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    glm::ivec2 resolution = window->GetResolution();

    eye_angle = (-1) * atan2((float)(mouseY - resolution.y / 2), (float)(mouseX - resolution.x / 2));
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event

    if (button == GLFW_MOUSE_BUTTON_2) {
        if (time_until_fire <= 0) {
            projectiles.push_back(Projectile(playerX + cos(eye_angle), playerY + sin(eye_angle), eye_angle));

            if (shotgun_time > 0) {
                projectiles.push_back(Projectile(playerX + cos(eye_angle + M_PI / 4), playerY + sin(eye_angle + M_PI / 4), eye_angle + M_PI / 4));
                projectiles.push_back(Projectile(playerX + cos(eye_angle - M_PI / 4), playerY + sin(eye_angle - M_PI / 4), eye_angle - M_PI / 4));
            }

            time_until_fire = 2;
        }
    }
    

}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}
