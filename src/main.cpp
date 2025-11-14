/*******************************************************************************************
*
*   raylib [core] example - Refactored Circle movement
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <bits/stdc++.h>
//----------------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------------
const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 1920;
int CurrentColor = 1;
const float THICK_LINE = 10.0f;
const float cycle = 360.0f;
const int numSlice = 54;

//----------------------------------------------------------------------------------
/* ANIMATION CLASS*/
//----------------------------------------------------------------------------------
typedef enum ANIM_STATE
{

    INACTIVE,
    PLAY_ONCE,
    LOOP4EVER,
};
class ANIMATION
{
    public:
        ANIM_STATE state;
        char nameFile[256];
        int currentFrame;
        int totalFrames;
        int x,y,w,h;
        Texture2D *texture;
        int delayTime;
    

        ANIMATION(ANIM_STATE state, char* nameFile,int currentFrames, int totalFrames, int x, int y, int w, int h, Texture2D *texture, int delayTime)
        {
            this->state = state;
            strcpy(this->nameFile , nameFile);
            this->currentFrame = currentFrames;
            this->totalFrames = totalFrames;
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
            this->texture = texture;
            this->delayTime = delayTime;
        }
        ~ANIMATION()
        {
            for(int i = 0; i < totalFrames; i++)
            {
                UnloadTexture(texture[i]);
            }
        }

        void LoadAnimation()
        {   

            for(int i = 0; i < totalFrames; i++)
            {   char buf[256];
                sprintf(buf, nameFile, i);
                LoadTexture(buf);
                std::cout << buf << std::endl;
            }
        }

        void SetState(ANIM_STATE newState)
        {
            state = newState;
        }

        void DoAnimation()
        {
            
            if(state == PLAY_ONCE)
            {
                if(currentFrame < totalFrames)
                {
                    currentFrame++;
                }
                else
                {
                    state = INACTIVE;
                    currentFrame = 0;
                }
            }
            else if(state == LOOP4EVER)
            {
                currentFrame++;
                if(currentFrame >= totalFrames)
                {
                    currentFrame = 0;
                }
            }
            if(state == INACTIVE) return;
            DrawTexture(texture[currentFrame], x, y, WHITE);

        }


};
//----------------------------------------------------------------------------------
// Circle Class
//----------------------------------------------------------------------------------
typedef enum WHEEL_STATE
{   
    STOP,
    SPINNING,
};
#define FPS 50
class Wheel
{
    public:
        int w,h;
        int centerX,centerY;
        float rotate;
        int offsetX, offsetY;
        float deltaTime = 0.02f;
        Texture2D texture;
        WHEEL_STATE state;
        int realW, realH;
        float toltalTime;
        float toltalSpin;
        float startSpinTime;
        float oldPos;   
        float angle = 0.0f;
        float velocity = 0.0f;
       
        float accel = 70.0f;        // tốc độ tăng tốc (độ/giây²)
        float maxSpeed = 166.75f;   // tốc độ tối đa (độ/giây) ~ 1 vòng/s
        float friction = 0.995f;   // ma sát khi giảm tốc (giảm chậm)
        float friction1=  0.997f;
        float decelThreshold = 5.0f; // khi tốc độ nhỏ hơn giá trị này thì dừng

        float holdTime = 1.0f;     // giữ tốc độ cao trong 1 giây
        float elapsedHold = 0.0f;
        int phase = 0; // 0: idle, 1: tăng tốc, 2: giữ, 3: giảm tốc
        int tmp = 0;
        int turn_to_result = 0;
        int count_meet_bar = 7;
        int count_back;


        int lech = 12;
        

        Wheel()
        {
            texture = LoadTexture("./Graphic/wheel.png");
            w = texture.width;
            h = texture.height;
            state = STOP;
            rotate = 0.0f;
            offsetX = 12;
            offsetY = 431; 
            realW = 1057;
            realH =  1058;
            toltalTime = 10.0f;
            toltalSpin = 360.0f * 2 + 90*2;
            centerX = realW/2;
            centerY = realH/2;
            oldPos = rotate;


            

        }

        ~Wheel()
        {
            UnloadTexture(texture);
        }
        void Update()
        {   if(state != SPINNING) return;
            if(state == SPINNING)
            {   
                float currentTime = GetTime();
                float t = (currentTime - startSpinTime) / toltalTime;
                if(t >= 1.0f)
                {
                    state = STOP;
                    return;
                }

                rotate = oldPos + easeInEaseOutCustom(t, 0.2f) * toltalSpin;
            }
        }
        bool IsCollisionBar()
        {
            // Tính góc của mỗi lát
            float anglePerSlice = cycle / numSlice;
            float halfSlice = -anglePerSlice / 2.0f ;

            // Chuẩn hóa góc hiện tại về [0, cycle)
            float currentAngle = fmodf(angle, cycle);
            float nextAngle = fmodf(angle + velocity * deltaTime , cycle);

            // Xác định lát hiện tại và lát tiếp theo
            int currentSlice = (int)((currentAngle - halfSlice + cycle) / anglePerSlice) % numSlice;
            int nextSlice = (int)((nextAngle - halfSlice + cycle) / anglePerSlice) % numSlice;

            // Nếu lát thay đổi → wheel vừa qua ranh giới (bar)
            return (nextSlice != currentSlice);
        }
        bool IsStopRightPlace()
        {
            // Tính góc của mỗi lát
            float anglePerSlice = cycle / numSlice;
            float halfSlice = anglePerSlice / 2.0f;

            // Chuẩn hóa góc hiện tại về [0, cycle)
            float currentAngle = fmodf(angle, cycle);
            float nextAngle = fmodf(angle + velocity * deltaTime, cycle);

            // Xác định lát hiện tại và lát tiếp theo
            int currentSlice = (int)((currentAngle + cycle) / anglePerSlice) % numSlice;
            int nextSlice = (int)((nextAngle + cycle) / anglePerSlice) % numSlice;

            // Nếu lát thay đổi → wheel vừa qua ranh giới (bar)
            return (nextSlice != currentSlice);
        }
        void UpdateWheel()
        {
            switch (phase)
            {
                case 0:
                    break;
                 
                case 1: // tăng tốc
                    velocity += accel * deltaTime;
                    if (velocity >= maxSpeed) {
                        velocity = maxSpeed;
                        phase = 2;
                        elapsedHold = 0.0f;
                    }
                    break;

                case 2: // giữ tốc độ cao
                    if(lech -- <= 0)
                    {
                        phase = 3;
                    }
                    break;

                case 3: // giảm tốc
                    velocity *= friction;
                    if (velocity < 25) {
                        phase = 5; // dừng
                    }
                    break;


                case 5: // Chạm cột mốc
                {
                    // Giảm tốc độ nếu đang di chuyển nhanh
                    if (velocity > 1.5f)
                        velocity *= friction;
                    else
                        velocity = 1.5f;

                    // Nếu vừa mới chạm vào cột mốc lần đầu
                    if (IsCollisionBar() && count_meet_bar == 0 && velocity == 1.5f)
                        count_meet_bar = 100; 

                   
                    if (count_meet_bar > 0)
                    {
                        
                        count_meet_bar--;
                        if(count_meet_bar <= 30) velocity *= friction;


                        std::cout << "when meet bar"<<std::endl;

                       
                        if (count_meet_bar == 1)
                        {
                            velocity = -1.0f;
                            phase = 6;
                        }
                    }

                    break;
                }

                case 6: // vi tri dung
                    //velocity *= friction1;
                    if(IsStopRightPlace())
                    {
                        phase = 0;
                        velocity = 0.0f;
                        state = STOP;
                        std::cout << "time spin = "<< GetTime() - startSpinTime << std::endl;
                    }
                 break;
            }

            angle += velocity * deltaTime;
            if(phase !=0)
                std::cout << "phase = " << phase << " angle = "<< angle << "v = "<< velocity <<  std::endl;
            if (angle >= 360.0f) angle -= 360.0f;
            if (angle < 0) angle += 360.0f;
        }
        void Draw()
        {
            DrawTexturePro(texture, (Rectangle){0,0,(float)w, (float)h},
            (Rectangle){(float)(offsetX + centerX),(float)(offsetY + centerY),(float)realW, (float)realH},
            (Vector2){(float)(centerX ) ,(float)(centerY)}, angle,WHITE
            );
        }
        void StartSpin()
        {   
            std::cout << "start" << std::endl;
            state = SPINNING;
            startSpinTime = GetTime();
            oldPos = rotate;
            phase = 1;
            velocity = 0;
            tmp = 0;
            count_meet_bar = 0;

        }
        bool IsWheelSpin()
        {
            if(state == SPINNING) return 1;
            else return 0;
        }
        float easeInEaseOutCustom(float t, float accelPortion)
        {
            if(accelPortion <= 0.0f)
            {
                accelPortion = 0.01f;
            }
            if(accelPortion >= 1.0f)
                accelPortion = 0.99f;
            
            if (t < accelPortion)
            {
                // Giai đoạn tăng tốc: scale t về [0, 1]
                float nt = t / accelPortion;
                return nt * nt * nt * (accelPortion); // cubic in
            }
            else
            {
                // Giai đoạn giảm tốc: scale t về [0, 1]
                float nt = (t - accelPortion) / (1.0f - accelPortion);
                float f = (nt - 1.0f);
                return accelPortion + (1.0f - accelPortion) * (1.0f + f * f * f); // cubic out
            }
        }
};
#define PARTICLE_FRAMES 60
Texture2D bgrParticle[PARTICLE_FRAMES];
#define  ARROW_FRAME 1673
int arrow_loop = 1;
Texture2D arrow[ARROW_FRAME];

#define LOGO_FRAME 60

Texture2D logoBrand[LOGO_FRAME];

ANIMATION bgrParticleAnim = ANIMATION(LOOP4EVER, "./Anim/Particle/Background Particle_%05d.png", 0, PARTICLE_FRAMES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bgrParticle,1);
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Refactored Circle Example");

    Texture2D wheel_bgr = LoadTexture ("./Graphic/wheel_bgr.png");
    for (int i = 0; i < PARTICLE_FRAMES; i++)
    {
        char buf[256];
        sprintf(buf,"./Anim/Particle/Background Particle_%05d.png",i );
        bgrParticle[i] = LoadTexture(buf);
    }
    for(int i = 0; i < ARROW_FRAME; i++)
    {
        char buf[256];
        sprintf(buf,"./Anim/Arrow/Arrow_%05d.png",i);
        arrow[i] = LoadTexture(buf);

    }
    for(int i = 0; i < LOGO_FRAME; i++)
    {
        char buf[256];
        sprintf(buf,"./Anim/Logo/LogoGame_%05d.png",i);
        logoBrand[i] = LoadTexture(buf);
    }
    //bgrParticleAnim.LoadAnimation();
    // Camera2D camera = { 0 };
    // bool zooming = false;
    // float zoomSpeed = 0.3f;     // tốc độ zoom
    // float maxZoom = 3.0f;       // zoom tối đa
    // camera.target = (Vector2){540,960};
    // camera.offset = (Vector2){540,960};
    // camera.zoom = 1.0f;
    int wheel_spin_frame = 0;
    int curentFrameParticle = 0;
    int currentFrameArrow = 0;
    int currentFrameLogo = 0;
    Rectangle rect = {324, 209 + 30, 420, 409}; // vùng muốn zoom vào
    Vector2 rectCenter = {rect.x + rect.width / 2, rect.y + rect.height / 2};
    Wheel wheel = Wheel();
    SetTargetFPS(60);


    Texture2D logo = LoadTexture("./Graphic/logo_winstar.png");
    // vẽ toàn bộ cảnh 1080x1920 ở đây
    float dt = 0.016f;
    // Main game loop
    Color colorText = WHITE;
    // zooming = false;
    int framewheelin1loop;
    while (!WindowShouldClose()) {
        if(IsKeyPressed(KEY_SPACE) && wheel.state == STOP)
        {   
            wheel.StartSpin();

        }
        if(curentFrameParticle++ / 2 >= 59) curentFrameParticle = 0;
        if(currentFrameLogo++ / 2 >= 59) currentFrameLogo = 0;
        
        if(wheel.IsWheelSpin() && arrow_loop )
        {   
            
            currentFrameArrow++;

            
            if(currentFrameArrow >= ARROW_FRAME)
            { currentFrameArrow = 0;
                arrow_loop--;
            }
        }else 
        {   
            currentFrameArrow = 0;

        }
         
        if(wheel.IsWheelSpin())
        {    
            if(wheel.phase == 2)
            {
                colorText = RED;
            }else colorText = WHITE;
           
            wheel_spin_frame++;
           
        }
        // if (zooming)
        // {   
        //     std::cout << zooming << "  " << camera.zoom  << " " << dt <<  std::endl;
        //     // Di chuyển camera target dần dần đến tâm của rect
        //     camera.target.x += (rectCenter.x - camera.target.x) * dt * 2.0f;
        //     if(camera.zoom >= 2.0)
        //         camera.target.y += (rectCenter.y - camera.target.y) * dt * 2.0f;
        //     // camera.offset.x += (rect.width/2 - camera.offset.x) *dt;
           
               
        //     // Tăng dần zoom
        //     camera.zoom += zoomSpeed * dt;
        //     if (camera.zoom >= maxZoom || camera.offset.y <= rect.height/2)
        //     {
        //         // camera.zoom = maxZoom;
        //         zooming = false; // dừng lại khi đạt zoom mong muốn
        //     }
        // }
        wheel.UpdateWheel();
        float fps = GetFPS();
        BeginDrawing();
        // BeginMode2D(camera);
         
        ClearBackground(GRAY);
        DrawTexture(wheel_bgr,0,0,WHITE);
        //bgrParticleAnim.DoAnimation();
        DrawTexture(bgrParticle[curentFrameParticle/2],0,0,WHITE);
        wheel.Draw();

        DrawTexture(arrow[currentFrameArrow], 406, 213, WHITE);

        DrawTexture(logoBrand[currentFrameLogo/2], 216, -62, WHITE);
        DrawTexture(logo,354,761,WHITE);
        
        DrawText(TextFormat("FPS = %0.2f", fps),10,10,40,WHITE);
        DrawText(TextFormat("frame wheel =  %d", wheel_spin_frame),10,70,40,colorText);
        // DrawText(TextFormat("zoom = %0.2f| targe {%f,%f}", camera.zoom, camera.target.x, camera.target.y),10,60,40,WHITE);
        
        
         EndDrawing();
        // EndMode2D();
    }
    UnloadTexture(wheel_bgr);

    CloseWindow();
    return 0;
}
