////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: ��â�� Chang-hyeon Park,
// Modified by Bong-Soo Sohn and Dong-Jun Kim
//
// Originally programmed for Virtual LEGO.
// Modified later to program for Virtual Billiard.
//
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <random>
#include <iostream>
#include "GameManager.h"

GameManager gameManager;

IDirect3DDevice9* Device = NULL;

// window size
const int Width = 1024;
const int Height = 768;


// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21 // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 1

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere
{
private:
    float center_x, center_y, center_z;

protected:
    //Morgan - Need this to not draw again ball already touched
    float m_radius;
    float m_velocity_x;
    float m_velocity_z;
    int   dead;

public:
    CSphere(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_radius = 0;
        m_velocity_x = 0;
        m_velocity_z = 0;
        m_pSphereMesh = NULL;
    }
    ~CSphere(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;

        dead = 0;
        m_mtrl.Ambient = color;
        m_mtrl.Diffuse = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power = 5.0f;

        if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
            return false;
        return true;
    }

    void destroy(void)
    {
        if (m_pSphereMesh != NULL)
        {
            m_pSphereMesh->Release();
            m_pSphereMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
        m_pSphereMesh->DrawSubset(0);
    }

    //Morgan - Detect the collision between balls -> we will use in CPrincipalSphere in the function hitBy
    bool hasIntersected(CSphere& ball)
    {
        D3DXVECTOR3 cord = this->getCenter();
        float tX = cord.x;
        float tZ = cord.z;

        D3DXVECTOR3 cordball = ball.getCenter();
        float ballx = cordball.x;
        float ballz = cordball.z;

        // we are using multiplications because it's faster than calling pow
        float distance = sqrt((tX - ballx) * (tX - ballx) + (tZ - ballz) * (tZ - ballz));
        return distance < (M_RADIUS + M_RADIUS);
    }

    void hitBy(CSphere& ball)
    {
        // Insert your code here.
        // Morgan - Normal ball will not moved or anything so nothing to do here
    }

    void ballUpdate(float timeDiff)
    {
        const float TIME_SCALE = 3.3;
        D3DXVECTOR3 cord = this->getCenter();
        double vx = abs(this->getVelocity_X());
        double vz = abs(this->getVelocity_Z());

        if (vx > 0.01 || vz > 0.01)
        {
            float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
            float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

            //correction of position of ball
            // Morgan - just uncomment it : Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
            if (tX >= (4.5 - M_RADIUS)) {
                tX = 4.5 - M_RADIUS;
                this->setVelocity_X(-vx);
            }
            else if (tX <= (-4.5 + M_RADIUS)) {
                tX = -4.5 + M_RADIUS;
                this->setVelocity_X(vx);
            }
            else if (tZ <= (-3 + M_RADIUS)) {
                tZ = -3 + M_RADIUS;
                this->setVelocity_Z(vz);
            }
            else if (tZ >= (3 - M_RADIUS)) {
                this->setVelocity_Z(-vz);
                tZ = 3 - M_RADIUS;
            }

            this->setCenter(tX, cord.y, tZ);
        }
        else
        {
            this->setPower(0, 0);
        }
        // this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
        double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
        if (rate < 0)
            rate = 0;
        this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
    }

    double getVelocity_X() { return this->m_velocity_x; }
    double getVelocity_Z() { return this->m_velocity_z; }
    //Morgan - Need to access velocity when the principal ball touch other ball or wall
    void setVelocity_X(double velocity) { this->m_velocity_x = velocity; }
    void setVelocity_Z(double velocity) { this->m_velocity_z = velocity; }
    //Morgan - New attributs to tell if the the ball has be touched to not draw it again 
    void setDead(int a) { dead = a; }
    int getDead(void) { return dead; }



    void setPower(double vx, double vz)
    {
        this->m_velocity_x = vx;
        this->m_velocity_z = vz;
    }

    void setCenter(float x, float y, float z)
    {
        D3DXMATRIX m;
        center_x = x;
        center_y = y;
        center_z = z;
        D3DXMatrixTranslation(&m, x, y, z);
        setLocalTransform(m);
    }

    float getRadius(void) const { return (float)(M_RADIUS); }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
    D3DXVECTOR3 getCenter(void) const
    {
        D3DXVECTOR3 org(center_x, center_y, center_z);
        return org;
    }

private:
    D3DXMATRIX m_mLocal;
    D3DMATERIAL9 m_mtrl;
    ID3DXMesh* m_pSphereMesh;

    //Morgan - Just to try some stuff
public:
    void setM_Mtrl(D3DXCOLOR color) {
        m_mtrl.Ambient = color;
        m_mtrl.Diffuse = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power = 5.0f;
    }

};

// -----------------------------------------------------------------------------
// CPrincipalSphere class definition
// Morgan - Need to have a principale ball that can "kill" the other when they intersect
// -----------------------------------------------------------------------------

class CPrinpaleSphere : public CSphere {

public:

    //intersected on csphere

    bool hitBy(CSphere& ball)
    {
        if (this->hasIntersected(ball) && ball.getDead() != 1) {
            ball.setDead(1);
            ball.destroy();
            this->setVelocity_X(-this->getVelocity_X());
            this->setVelocity_Z(-this->getVelocity_Z());
            gameManager.removeBalle();
            gameManager.addScore(1);
            return (true);
        }
        // Insert your code here.
        return (false);
    }

    bool hitByPad(CSphere& ball)
    {
        if (this->hasIntersected(ball)) {
            this->setVelocity_X(-this->getVelocity_X());
            this->setVelocity_Z(-this->getVelocity_Z());
            return (true);
        }
        return (false);
    }

    void ballUpdate(float timeDiff, CSphere& pad)
    {
        const float TIME_SCALE = 3.3;
        D3DXVECTOR3 cord = this->getCenter();
        double vx = abs(this->getVelocity_X());
        double vz = abs(this->getVelocity_Z());

        if (vx > 0.01 || vz > 0.01)
        {
            float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
            float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

            //correction of position of ball
            // Morgan - just uncomment it : Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
            if (tX >= (4.5 - M_RADIUS)) {
                tX = 4.5 - M_RADIUS;
                this->setVelocity_X(-vx);
            }
            else if (tX <= (-7)) {
                gameManager.removeLife(); //remove one life
                this->setOnPad(pad);
            }
            else if (tZ <= (-3 + M_RADIUS)) {
                tZ = -3 + M_RADIUS;
                this->setVelocity_Z(vz);
            }
            else if (tZ >= (3 - M_RADIUS)) {
                this->setVelocity_Z(-vz);
                tZ = 3 - M_RADIUS;
            }

            this->setCenter(tX, cord.y, tZ);
        }
        else
        {
            this->setPower(0, 0);
        }
        double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
        if (rate < 0)
            rate = 0;
        this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
    }

    void setIsOnPad(bool n) { isOnPad = n; }

    bool getIsOnPad() { return (isOnPad); }

    void setOnPad(CSphere& pad)
    {
        isOnPad = true;
        D3DXVECTOR3 padPos = pad.getCenter();
        this->setCenter(padPos.x + (float)M_RADIUS * 3, (float)M_RADIUS, padPos.z);
    }

    void Launch(CSphere& pad)
    {
        isOnPad = false;
        if (pad.getVelocity_Z() > 0) {
            this->setPower(2, 1.5);
        }
        else {
            this->setPower(2, -1.5);
        }
    }

private:
    bool isOnPad;
};

// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall
{

private:
    float m_x;
    float m_z;
    float m_width;
    float m_depth;
    float m_height;

public:
    CWall(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_width = 0;
        m_depth = 0;
        m_pBoundMesh = NULL;
    }
    ~CWall(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;

        m_mtrl.Ambient = color;
        m_mtrl.Diffuse = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power = 5.0f;

        m_width = iwidth;
        m_depth = idepth;

        if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
            return false;
        return true;
    }
    void destroy(void)
    {
        if (m_pBoundMesh != NULL)
        {
            m_pBoundMesh->Release();
            m_pBoundMesh = NULL;
        }
    }
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
        m_pBoundMesh->DrawSubset(0);
    }

    //Morgan - Intersection between wall and ball
    bool hasIntersected(CSphere& ball)
    {
        D3DXVECTOR3 cord = ball.getCenter();
        float tX = cord.x;
        float tZ = cord.z;

        if (tX >= (4.5 - M_RADIUS)) {
            ball.setVelocity_X(-ball.getVelocity_X());
            return true;
        }
        else if (tX <= (-4.5 + M_RADIUS)) {
            return true;
        }
        else if (tZ <= (-3 + M_RADIUS)) {
            return true;
        }
        else if (tZ >= (3 - M_RADIUS)) {
            ball.setVelocity_Z(-ball.getVelocity_Z());
            return true;
        }
        return false;
    }

    //Morgan - nothing to do because we don't want anything special
    void hitBy(CSphere& ball)
    {
        //nothing happend to the wall if it's hit ! it's just a wall
        // Insert your code here.
    }

    void setPosition(float x, float y, float z)
    {
        D3DXMATRIX m;
        this->m_x = x;
        this->m_z = z;

        D3DXMatrixTranslation(&m, x, y, z);
        setLocalTransform(m);
    }

    float getHeight(void) const { return M_HEIGHT; }

private:
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

    D3DXMATRIX m_mLocal;
    D3DMATERIAL9 m_mtrl;
    ID3DXMesh* m_pBoundMesh;

};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight
{
public:
    CLight(void)
    {
        static DWORD i = 0;
        m_index = i++;
        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));
        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bound._radius = 0.0f;
    }
    ~CLight(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
    {
        if (NULL == pDevice)
            return false;
        if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
            return false;

        m_bound._center = lit.Position;
        m_bound._radius = radius;

        m_lit.Type = lit.Type;
        m_lit.Diffuse = lit.Diffuse;
        m_lit.Specular = lit.Specular;
        m_lit.Ambient = lit.Ambient;
        m_lit.Position = lit.Position;
        m_lit.Direction = lit.Direction;
        m_lit.Range = lit.Range;
        m_lit.Falloff = lit.Falloff;
        m_lit.Attenuation0 = lit.Attenuation0;
        m_lit.Attenuation1 = lit.Attenuation1;
        m_lit.Attenuation2 = lit.Attenuation2;
        m_lit.Theta = lit.Theta;
        m_lit.Phi = lit.Phi;
        return true;
    }
    void destroy(void)
    {
        if (m_pMesh != NULL)
        {
            m_pMesh->Release();
            m_pMesh = NULL;
        }
    }
    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return false;

        D3DXVECTOR3 pos(m_bound._center);
        D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
        D3DXVec3TransformCoord(&pos, &pos, &mWorld);
        m_lit.Position = pos;

        pDevice->SetLight(m_index, &m_lit);
        pDevice->LightEnable(m_index, TRUE);
        return true;
    }

    void draw(IDirect3DDevice9* pDevice)
    {
        if (NULL == pDevice)
            return;
        D3DXMATRIX m;
        D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
        pDevice->SetTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);
        m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
    DWORD m_index;
    D3DXMATRIX m_mLocal;
    D3DLIGHT9 m_lit;
    ID3DXMesh* m_pMesh;
    d3d::BoundingSphere m_bound;
};

// -----------------------------------------------------------------------------
// BallGenerator class definition (Yoan)
// -----------------------------------------------------------------------------

class BallGenerator {
public:
    BallGenerator() {
        _maxBalls = 0;
        _isInit = false;
    }

    void InitBalls(int maxBalls, D3DXCOLOR ballsColor, IDirect3DDevice9* device) {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> distX(-3.0f, 4.0f);
        std::uniform_real_distribution<float> distZ(-2.5f, 2.50f);

        _maxBalls = maxBalls;

        for (int i = 0; i < maxBalls; i++) {
            CSphere newSphere;
            if (false == newSphere.create(device, ballsColor))
                return;
            newSphere.setCenter(distX(mt), (float)M_RADIUS, distZ(mt));
            newSphere.setPower(0, 0);
            _spheres.push_back(newSphere);
        }
        _isInit = true;
    }

    void Draw() {
        if (_isInit == false) return;
        for (int i = 0; i != _spheres.size(); i++) {
            _spheres[i].draw(Device, g_mWorld);
        }
    }

    void CheckCollision(CPrinpaleSphere& mainSphere) {
        int i = 0;
        while (i != _spheres.size()) {
            if (mainSphere.hitBy(_spheres[i])) {
                _spheres.erase(_spheres.begin() + i);
                i = 0;
            }
            else {
                i++;
            }
        }
    }
private:
    std::vector<CSphere> _spheres;
    int _maxBalls;
    bool _isInit;
};

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
LPD3DXFONT font = NULL;
CWall g_legoPlane;
CWall g_legowall[3];
BallGenerator balls;
CSphere pad;
CLight g_light;
CPrinpaleSphere g_principal_ball;

double g_camera_pos[3] = { 0.0, 5.0, -8.0 };

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void destroyAllLegoBlock(void)
{
}

/*
    Custom function to draw text (Yoan)
*/
bool CustomDrawFont(LPD3DXFONT font, unsigned int x, unsigned int y, int alpha, unsigned char r, unsigned char g, unsigned char b, LPCSTR message)
{	// Create a colour for the text
    D3DCOLOR fontColor = D3DCOLOR_ARGB(alpha, r, g, b);
    RECT rct; //Font
    rct.left = x;
    rct.right = 1680;
    rct.top = y;
    rct.bottom = rct.top + 200;
    font->DrawTextA(NULL, message, -1, &rct, 0, fontColor);
    return true;
}

// initialization
bool Setup()
{
    int i;

    D3DXCreateFont(Device, 30, 0, FW_BOLD, 0, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &font);
    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);

    // create plane and set the position
    if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN))
        return false;
    g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

    // create walls and set the position. note that there are four walls
    if (false == g_legowall[0].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::BLACK))
        return false;
    g_legowall[0].setPosition(0.0f, 0.12f, 3.06f);
    if (false == g_legowall[1].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::BLACK))
        return false;
    g_legowall[1].setPosition(0.0f, 0.12f, -3.06f);
    if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::BLACK))
        return false;
    g_legowall[2].setPosition(4.56f, 0.12f, 0.0f);

    // Yoan: Create spheres
    // Params: Number of spheres, Color, Device
    balls.InitBalls(20, d3d::YELLOW, Device);

    // Yoan: Create pad
    if (false == pad.create(Device, d3d::WHITE))
        return false;
    pad.setCenter(-4.5f, (float)M_RADIUS, 0);

    // Yoan: Create red (bouncy ball) !
    if (false == g_principal_ball.create(Device, d3d::RED)) return false;
    g_principal_ball.setCenter(-4.5f + (float)M_RADIUS * 3, (float)M_RADIUS, 0);
    g_principal_ball.setIsOnPad(true);

    // light setting
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type = D3DLIGHT_POINT;
    lit.Diffuse = d3d::WHITE;
    lit.Specular = d3d::WHITE * 0.9f;
    lit.Ambient = d3d::WHITE * 0.9f;
    lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
    lit.Range = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (false == g_light.create(Device, lit))
        return false;

    // Position and aim the camera.
    D3DXVECTOR3 pos(0.0f, 10.0f, -6.0f);
    D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
    D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
    Device->SetTransform(D3DTS_VIEW, &g_mView);

    // Set the projection matrix.
    D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
        (float)Width / (float)Height, 1.0f, 100.0f);
    Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

    // Set render states.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

    g_light.setLight(Device, g_mWorld);
    return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
    for (int i = 0; i < 3; i++)
    {
        g_legowall[i].destroy();
    }
    destroyAllLegoBlock();
    g_light.destroy();
}

// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
    int i = 0;
    int j = 0;

    if (Device)
    {
        Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
        Device->BeginScene();

        if (!gameManager.getIsOver()) {
            if (gameManager.getBalls() > 0)
            {
                // update the position of each ball. during update, check whether each ball hit by walls.
                if (g_principal_ball.getIsOnPad()) {
                    g_principal_ball.setOnPad(pad);
                }
                else {
                    g_principal_ball.ballUpdate(timeDelta, pad);
                }
                pad.ballUpdate(timeDelta);

                // Yoan - Check for collisions with main sphere
                balls.CheckCollision(g_principal_ball);
                g_principal_ball.hitByPad(pad);

                // draw plane, walls, and spheres
                g_legoPlane.draw(Device, g_mWorld);
                for (i = 0; i < 3; i++)
                {
                    g_legowall[i].draw(Device, g_mWorld);
                }
                // Draw the "random balls"
                balls.Draw();
                CustomDrawFont(font, (1024 / 2) - 100, 700, 255, 255, 255, 255, "Press SPACE to start");
                CustomDrawFont(font, (512 / 2) - 25, 20, 255, 255, 255, 255, gameManager.printScore()); //Display the score - should display 0 at the beginning but it doesn't work
                CustomDrawFont(font, (1536 / 2) - 25, 20, 255, 255, 255, 255, gameManager.printLife()); //Display the life - should display 3 at the beginning but it doesn't work
                pad.draw(Device, g_mWorld);
                g_principal_ball.draw(Device, g_mWorld); //Morgan - Think to draw again the principale ball
                g_light.draw(Device);
            }
            else {
                CustomDrawFont(font, (1024 / 2) - 50, (768 / 2) - 50, 255, 255, 255, 255, gameManager.victory());
            }
        }
        else
        {
            CustomDrawFont(font, (1024 / 2) - 50, (768/2) - 50, 255, 255, 255, 255, gameManager.gameOver());
        }

        Device->EndScene();
        Device->Present(0, 0, 0, 0);
        Device->SetTexture(0, NULL);
    }
    return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static bool wire = false;
    static bool isReset = true;
    static int old_x = 0;
    static int old_y = 0;
    static enum {
        WORLD_MOVE,
        LIGHT_MOVE,
        BLOCK_MOVE
    } move = WORLD_MOVE;

    switch (msg)
    {
    case WM_DESTROY:
    {
        ::PostQuitMessage(0);
        break;
    }
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            ::DestroyWindow(hwnd);
            break;
        case VK_RETURN:
            if (NULL != Device)
            {
                wire = !wire;
                Device->SetRenderState(D3DRS_FILLMODE,
                    (wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
            }
            break;
        case VK_LEFT:
            pad.setPower(0, 1.5f);
            break;
        case VK_RIGHT:
            pad.setPower(0, -1.5f);
            break;
        case VK_SPACE:

            //D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
            //D3DXVECTOR3 whitepos = g_sphere[3].getCenter();
            //double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
            //                                                                  pow(targetpos.z - whitepos.z, 2))); // �⺻ 1 ��и�
            //if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0)
            //{
            //  theta = -theta;
            //} //4 ��и�
            //if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0)
            //{
            //  theta = PI - theta;
            //} //2 ��и�
            //if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0)
            //{
            //  theta = PI + theta;
            //} // 3 ��и�
            //double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
            //g_sphere[3].setPower(distance * cos(theta), distance * sin(theta));

            //Morgan - We will to link the space button and our ball;
            /*D3DXVECTOR3 targetposBIS = pad.getCenter();
            D3DXVECTOR3	whiteposBIS = g_principal_ball.getCenter();
            double thetaBIS = acos(sqrt(pow(targetposBIS.x - whiteposBIS.x, 2)) / sqrt(pow(targetposBIS.x - whiteposBIS.x, 2) + pow(targetposBIS.z - whiteposBIS.z, 2)));		// ±âº» 1 »çºÐ¸é
            if (targetposBIS.z - whiteposBIS.z <= 0 && targetposBIS.x - whiteposBIS.x >= 0) { thetaBIS = -thetaBIS; }	//4 »çºÐ¸é
            if (targetposBIS.z - whiteposBIS.z >= 0 && targetposBIS.x - whiteposBIS.x <= 0) { thetaBIS = PI - thetaBIS; } //2 »çºÐ¸é
            if (targetposBIS.z - whiteposBIS.z <= 0 && targetposBIS.x - whiteposBIS.x <= 0) { thetaBIS = PI + thetaBIS; } // 3 »çºÐ¸é
            double distanceBIS = sqrt(pow(targetposBIS.x - whiteposBIS.x, 2) + pow(targetposBIS.z - whiteposBIS.z, 2));
            g_principal_ball.setPower(distanceBIS * cos(thetaBIS), distanceBIS * sin(thetaBIS));*/
            g_principal_ball.Launch(pad);

            break;
        }
        break;
    }

    case WM_MOUSEMOVE:
    {
        int new_x = LOWORD(lParam);
        int new_y = HIWORD(lParam);
        float dx;
        float dy;

        if (LOWORD(wParam) & MK_LBUTTON)
        {

            if (isReset)
            {
                isReset = false;
            }
            else
            {
                D3DXVECTOR3 vDist;
                D3DXVECTOR3 vTrans;
                D3DXMATRIX mTrans;
                D3DXMATRIX mX;
                D3DXMATRIX mY;

                switch (move)
                {
                case WORLD_MOVE:
                    dx = (old_x - new_x) * 0.01f;
                    dy = (old_y - new_y) * 0.01f;
                    D3DXMatrixRotationY(&mX, dx);
                    D3DXMatrixRotationX(&mY, dy);
                    g_mWorld = g_mWorld * mX * mY;

                    break;
                }
            }

            old_x = new_x;
            old_y = new_y;
        }
        else
        {
            isReset = true;

            if (LOWORD(wParam) & MK_RBUTTON)
            {
                dx = (old_x - new_x); // * 0.01f;
                dy = (old_y - new_y); // * 0.01f;

                D3DXVECTOR3 coord3d = pad.getCenter();
                pad.setCenter(coord3d.x + dx * (-0.007f), coord3d.y, coord3d.z + dy * 0.007f);
            }
            old_x = new_x;
            old_y = new_y;

            move = WORLD_MOVE;
        }
        break;
    }
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
    HINSTANCE prevInstance,
    PSTR cmdLine,
    int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));

    if (!d3d::InitD3D(hinstance,
        Width, Height, true, D3DDEVTYPE_HAL, &Device))
    {
        ::MessageBox(0, "InitD3D() - FAILED", 0, 0);
        return 0;
    }

    if (!Setup())
    {
        ::MessageBox(0, "Setup() - FAILED", 0, 0);
        return 0;
    }

    d3d::EnterMsgLoop(Display);

    Cleanup();

    Device->Release();

    return 0;
}