#pragma once

#include "components/simple_scene.h"

#define RED     glm::vec3(1, 0, 0)
#define GREEN   glm::vec3(0, 1, 0)
#define BLUE    glm::vec3(0, 0, 1)
#define NAVY_BLUE    glm::vec3(0, 0, 0.5)
#define AVATAR_BLUE    glm::vec3(0, 0.4, 1)
#define YELLOW  glm::vec3(1, 1, 0)
#define WHITE   glm::vec3(1, 1, 1)
#define GREY    glm::vec3(0.45, 0.45, 0.45)
#define GRASS_COLOR    glm::vec3(0.49, 0.78, 0.31)
#define BLACK   glm::vec3(0, 0, 0)
#define DARKER_SKIN_COLOR glm::vec3(0.8, 0.5, 0)
#define SKIN_COLOR glm::vec3(1, 0.7, 0)
#define BAG_COLOR glm::vec3(0.3, 0.1, 0)

#define PLAYERSTARTX 25
#define PLAYERSTARTY 25
#define PLAYER_LOGICSPACE_OFFSET 12.5
#define BULLET_SPEED 12
#define MAX_BULLET_DISTANCE 25
#define XP_GOAL 30

namespace m1
{
    class Tema1 : public gfxc::SimpleScene
    {
     public:
        struct ViewportSpace
        {
            ViewportSpace() : x(0), y(0), width(1), height(1) {}
            ViewportSpace(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };

        struct LogicSpace
        {
            LogicSpace() : x(0), y(0), width(1), height(1) {}
            LogicSpace(float x, float y, float width, float height)
                : x(x), y(y), width(width), height(height) {}
            float x;
            float y;
            float width;
            float height;
        };

     public:
        Tema1();
        ~Tema1();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void DrawScene(glm::mat3 visMatrix);
        void DrawDeadScene(glm::mat3 visMatrix);
        void DrawOkScene(glm::mat3 visMatrix);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;

        // Sets the logic space and view space
        // logicSpace: { x, y, width, height }
        // viewSpace: { x, y, width, height }
        glm::mat3 VisualizationTransf2D(const LogicSpace &logicSpace, const ViewportSpace &viewSpace);
        glm::mat3 VisualizationTransf2DUnif(const LogicSpace &logicSpace, const ViewportSpace &viewSpace);

        void SetViewportArea(const ViewportSpace &viewSpace, glm::vec3 colorColor = glm::vec3(0), bool clear = true);
        

     protected:
        float length, l;
        ViewportSpace viewSpace;
        LogicSpace logicSpace;
        glm::mat3 modelMatrix, visMatrix;

        float playerX = PLAYERSTARTX, playerY = PLAYERSTARTY;
        float lefthandX, lefthandY, righthandX, righthandY;

        float eye_angle = 0;
        float time_until_fire = 0;
        float bullet_length = 0.5f;
        float time_until_enemy_spawn = 2.5;
        float time_until_hpup_spawn = 20;
        float time_until_shotgun_spawn = 25;
        float shotgun_time = 0;

        float hp = 20;
        float xp = 0;

        bool dead = false;
    };
}   // namespace m1
