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



const int kStepsPerSymbol = 2;         // Mỗi symbol = 2 đơn vị di chuyển
const float cycle = 360.0f;
const int numSlice = 54;    // có 54 ô

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
        
        Texture2D texture;
        WHEEL_STATE state;
        int realW, realH;
        float startSpinTime;  
        float angle;
        float velocity;
       


        const float deltaTime = 0.02f; //(FPS ~ 50 -> deltaTime = 1/FPS)
        const float accel = 70.0f;        // tốc độ tăng tốc (độ/giây²)
        const float maxSpeed = 166.73f;   // tốc độ tối đa (độ/giây) ~ 1 vòng/s
        const float friction = 0.995f;   // ma sát khi giảm tốc (giảm chậm)

        

        int phase ; // 0: idle, 1: tăng tốc, 2: giữ, 3: giảm tốc, 4: chạm đến cột ->dừng , 5: quay ngược lại về mid của slice

        int goContinueCnt = 0 ;




        //control KẾT QUẢ DỪNG. Để di chuyển thêm 1 ô -> tăng biến này lên 2 đơn vị.
        int goToResult;
        

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

            centerX = realW/2;
            centerY = realH/2;
            velocity = 0.0f;
            angle = 0.0f;

        }

        ~Wheel()
        {
            UnloadTexture(texture);
        }

        bool IsCollidingWithBar()
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
        bool IsStopPosition()
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
                       
                    }
                    
                    break;
                
                case 2: // giữ tốc độ cao
                    
                    if(goToResult -- <= 0)
                    {
                        phase = 3;
                    }
                    break;

                case 3: // giảm tốc
                    velocity *= friction;
                    if (velocity < 25) {
                        phase = 4; // dừng
                    }
                    break;


                case 4: // Chạm cột mốc
                {
                    const float kMinSpeed = 1.5f;

                    // 1. Giảm tốc độ về mức tối thiểu
                    if (velocity > kMinSpeed)
                        velocity *= friction;
                    else
                        velocity = kMinSpeed;

                    // 2. Lần đầu chạm cột mốc → cho chạy tiếp 1 đoạn goContinueCnt frame
                    bool firstTouch = (goContinueCnt == 0);
                    bool atMinSpeed = (velocity == kMinSpeed);

                    if (IsCollidingWithBar() && firstTouch && atMinSpeed)
                    {
                        goContinueCnt = 100;
                    }

                    // 3. Nếu đang trong giai đoạn “chạy tiếp”
                    if (goContinueCnt > 0)
                    {
                        goContinueCnt--;

                        // 3.1. Ở giai đoạn cuối thì giảm tốc mạnh hơn
                        if (goContinueCnt <= 30)
                            velocity *= friction;

                        // 3.2. Chuẩn bị chuyển sang phase 5
                        if (goContinueCnt == 1)
                        {
                            velocity = -1.0f;
                            phase = 5;
                        }
                    }

                    break;
                }

                case 5: // vi tri dung

                    if(IsStopPosition())
                    {
                        phase = 0;
                        velocity = 0.0f;
                        state = STOP;
                        
                    }
                 break;
            }

            angle += velocity * deltaTime;

            if (angle >= cycle) angle -= cycle;
            if (angle < 0) angle += cycle;
        }
        void Draw()
        {
            DrawTexturePro(texture, (Rectangle){0,0,(float)w, (float)h},
            (Rectangle){(float)(offsetX + centerX),(float)(offsetY + centerY),(float)realW, (float)realH},
            (Vector2){(float)(centerX ) ,(float)(centerY)}, angle,WHITE
            );
        }
        void StartSpin(int resultPosition)
        {
            state = SPINNING;
            startSpinTime = GetTime();
            phase = 1;
            velocity = 0.0f;



            // --- Result control ---
            // Mỗi symbol trong kết quả tương ứng với 2 bước.
            goToResult =resultPosition * kStepsPerSymbol;

        }
        bool IsWheelSpin()
        {
            if(state == SPINNING) return 1;
            else return 0;
        }

};
#define  ARROW_FRAME 1532
#define ARROW_LOOP_FRAME 12
class Arrow
{
public:
    Texture2D arrowBase[ARROW_FRAME];
    Texture2D arrowLoop[12];
    Texture2D arrowAnim[ARROW_FRAME + 54*2]; //-> this texture will be displace
    int totalFrame;
    Arrow()
    {   
        for(int i = 0; i < ARROW_FRAME; i++)
        {
            char buf[256];
            sprintf(buf,"./Anim/Arrow/Arrow_%05d.png",i);
            arrowBase[i] = LoadTexture(buf);

        }
        for(int i = 0; i < ARROW_LOOP_FRAME; i++)
        {
            char buf[256];
            sprintf(buf,"./Anim/ArrowLoop/ArrowLoop_%02d.png",i);
            arrowLoop[i] = LoadTexture(buf);
        }

    }
    ~Arrow()
    {   
        for(int i = 0; i < ARROW_FRAME; i++)
            UnloadTexture(arrowBase[i]);
        for(int i = 0; i < ARROW_LOOP_FRAME; i++)
        {
            UnloadTexture(arrowLoop[i]);
        }
    }

    void initArrowAnimation(int result)
    {   int i ;
        int j ;
        int index = 0;
        for(j = 0; j < 119; j++)
        {
            arrowAnim[index++] = arrowBase[j];
        }
        for(i = 0; i < result * kStepsPerSymbol; i++)
        {
            arrowAnim[index++] = arrowLoop[i % ARROW_LOOP_FRAME];
        }
    
        for(j = 119; j < ARROW_FRAME; j++)
        {
            arrowAnim[index++] = arrowBase[j];
        }
        totalFrame = index;
    }

};




#define PARTICLE_FRAMES 60
Texture2D bgrParticle[PARTICLE_FRAMES];





#define LOGO_FRAME 60
Texture2D logoBrand[LOGO_FRAME];

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

    for(int i = 0; i < LOGO_FRAME; i++)
    {
        char buf[256];
        sprintf(buf,"./Anim/Logo/LogoGame_%05d.png",i);
        logoBrand[i] = LoadTexture(buf);
    }


    int curentFrameParticle = 0;
    int currentFrameArrow = 0;
    int currentFrameLogo = 0;


    Wheel wheel = Wheel();
    Arrow arrow = Arrow();
    SetTargetFPS(60);

    int result = 0;// result nằm trong khoảng 0->53. Do có tất cả 54 ô.
    Texture2D logo = LoadTexture("./Graphic/logo_winstar.png");
    
    arrow.initArrowAnimation(result);
    while (!WindowShouldClose()) {
        if(IsKeyPressed(KEY_SPACE) && wheel.state == STOP)
        {   
            wheel.StartSpin(result);
           
        }
        if(curentFrameParticle++ / 2 >= PARTICLE_FRAMES - 1) curentFrameParticle = 0;
        if(currentFrameLogo++ / 2 >= LOGO_FRAME - 1) currentFrameLogo = 0;
        
        if(wheel.IsWheelSpin())
        {   
            currentFrameArrow++;

        }else 
        {   
            currentFrameArrow = 0;
        }
        if(currentFrameArrow >= arrow.totalFrame )
        { 
            currentFrameArrow = arrow.totalFrame - 1;
        }


         

        wheel.UpdateWheel();
        float fps = GetFPS();
        BeginDrawing();
        // BeginMode2D(camera);
         
        ClearBackground(GRAY);
        DrawTexture(wheel_bgr,0,0,WHITE);
        //bgrParticleAnim.DoAnimation();
        DrawTexture(bgrParticle[curentFrameParticle/2],0,0,WHITE);
        wheel.Draw();

        DrawTexture(arrow.arrowAnim[currentFrameArrow], 412 , 214, WHITE);

        DrawTexture(logoBrand[currentFrameLogo/2], 216, -62, WHITE);
        DrawTexture(logo,354,761,WHITE);
        
        DrawText(TextFormat("FPS = %0.2f", fps),10,10,40,WHITE);
        if(wheel.phase == 2) std::cout << currentFrameArrow << std::endl;
        
         EndDrawing();
    }
    UnloadTexture(wheel_bgr);

    CloseWindow();
    return 0;
}
