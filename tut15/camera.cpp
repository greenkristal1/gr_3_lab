/*

	Copyright 2010 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <GL/freeglut.h>

#include "camera.h"
//#include "math_3d.h"
#define M_PI 3.14
const static float STEP_SCALE = 0.1f;//размер шага
const static int MARGIN = 10;//граница экрана, когда камера начинает двигаться сама

Camera::Camera(int WindowWidth, int WindowHeight)
{
    m_windowWidth  = WindowWidth;//высота окна
    m_windowHeight = WindowHeight;//ширина окна
    m_pos          = Vector3f(0.0f, 0.0f, 0.0f);//позиция камеры
    m_target       = Vector3f(0.0f, 0.0f, 1.0f);//направление камеры
    m_target.Normalize();
    m_up           = Vector3f(0.0f, 1.0f, 0.0f);//верхний вектор

    Init();
}


Camera::Camera(int WindowWidth, int WindowHeight, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
{
    m_windowWidth  = WindowWidth;//высота окна
    m_windowHeight = WindowHeight;//ширина окна
    m_pos = Pos;//позиция камеры

    m_target = Target;//направление камеры
    m_target.Normalize();

    m_up = Up;//верхний вектор
    m_up.Normalize();

    Init();
}


void Camera::Init()//устанавливает внутренние параметры камеры
{
    Vector3f HTarget(m_target.x, 0.0, m_target.z);//вычисление горионтального угла
    HTarget.Normalize();
    
    if (HTarget.z >= 0.0f)//проверка какой кватернион соответствует вектору для конечного подсчета значения координаты Z
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = 360.0f - ToDegree(asin(HTarget.z));//подсчет горизонтального угла
        }
        else
        {
            m_AngleH = 180.0f + ToDegree(asin(HTarget.z));//подсчет горизонтального угла
        }
    }
    else
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = ToDegree(asin(-HTarget.z));//подсчет горизонтального угла
        }
        else
        {
            m_AngleH = 90.0f + ToDegree(asin(-HTarget.z));//подсчет горизонтального угла
        }
    }
    
    m_AngleV = -ToDegree(asin(m_target.y));//подсчет вертикального угла

    m_OnUpperEdge = false;
    m_OnLowerEdge = false;
    m_OnLeftEdge  = false;
    m_OnRightEdge = false;//проверка не касается ли курсор границ экрана. В начале курсор в центре экрана

    m_mousePos.x  = m_windowWidth / 2;//вычисление центра экрана по x
    m_mousePos.y  = m_windowHeight / 2;//вычисление центра экрана по y

    glutWarpPointer(m_mousePos.x, m_mousePos.y);//перемещает курсор в центр экрана в начале запуска программы
}


bool Camera::OnKeyboard(int Key)//доставляет событие клавиатуры в класс
{
    bool Ret = false;

    switch (Key) {

    case GLUT_KEY_UP://движение вперед
        {
            m_pos += (m_target * STEP_SCALE);
            Ret = true;
        }
        break;

    case GLUT_KEY_DOWN://движение назад
        {
            m_pos -= (m_target * STEP_SCALE);
            Ret = true;
        }
        break;

    case GLUT_KEY_LEFT://движение влево
        {
            Vector3f Left = m_target.Cross(m_up);
            Left.Normalize();
            Left *= STEP_SCALE;
            m_pos += Left;
            Ret = true;
        }
        break;

    case GLUT_KEY_RIGHT://движение вправо
        {
            Vector3f Right = m_up.Cross(m_target);
            Right.Normalize();
            Right *= STEP_SCALE;
            m_pos += Right;
            Ret = true;
        }
        break;
    }

    return Ret;
}


void Camera::OnMouse(int x, int y)//сообщает камере, как изменилось положение мыши
{
    const int DeltaX = x - m_mousePos.x;
    const int DeltaY = y - m_mousePos.y;//подсчет разницы между новыми координатами и предыдущими по осям и X и Y.

    m_mousePos.x = x;
    m_mousePos.y = y;//обновление координат

    m_AngleH += (float)DeltaX / 20.0f;//записываем новые значения для следующих вызовов функции. 
    //Мы обновляем текущие горизонтальные и вертикальные углы на эту разность в значениях. По сути настраиваем sensetivity
    m_AngleV += (float)DeltaY / 20.0f;

    if (DeltaX == 0) { // обновляем значения 'm_OnEdge' согласно положению курсора
        if (x <= MARGIN) {
        //    m_AngleH -= 1.0f;
            m_OnLeftEdge = true;
        }
        else if (x >= (m_windowWidth - MARGIN)) {
        //    m_AngleH += 1.0f;
            m_OnRightEdge = true;
        }
    }
    else {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (DeltaY == 0) {
        if (y <= MARGIN) {
            m_OnUpperEdge = true;
        }
        else if (y >= (m_windowHeight - MARGIN)) {
            m_OnLowerEdge = true;
        }
    }
    else {
        m_OnUpperEdge = false;
        m_OnLowerEdge = false;
    }

    Update();
}


void Camera::OnRender()//Эта функция вызывается из главного цикла рендера.
//Она нам необходима для случаев, когда мышь не движется, но находится около одной из границ экрана
{
    bool ShouldUpdate = false;

    if (m_OnLeftEdge) {
        m_AngleH -= 0.1f;
        ShouldUpdate = true;
    }
    else if (m_OnRightEdge) {
        m_AngleH += 0.1f;
        ShouldUpdate = true;
    }

    if (m_OnUpperEdge) {
        if (m_AngleV > -90.0f) {
            m_AngleV -= 0.1f;
            ShouldUpdate = true;
        }
    }
    else if (m_OnLowerEdge) {
        if (m_AngleV < 90.0f) {
           m_AngleV += 0.1f;
           ShouldUpdate = true;
        }
    }

    if (ShouldUpdate) {
        Update();
    }
}

void Camera::Update()//перерасчет векторов направления и вектора вверх, основанных на новых горизонтальном и вертикальном углах.
{
    const Vector3f Vaxis(0.0f, 1.0f, 0.0f);

    // Rotate the view vector by the horizontal angle around the vertical axis
    Vector3f View(1.0f, 0.0f, 0.0f);
    View.Rotate(m_AngleH, Vaxis);
    View.Normalize();

    // Rotate the view vector by the vertical angle around the horizontal axis
    Vector3f Haxis = Vaxis.Cross(View);
    Haxis.Normalize();
    View.Rotate(m_AngleV, Haxis);
       
    m_target = View;
    m_target.Normalize();

    m_up = m_target.Cross(Haxis);
    m_up.Normalize();
}
