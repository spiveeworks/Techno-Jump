
#include <windows.h>
#include <math.h>


namespace Unit {
	typedef int Time;
	typedef double PreciseTime;
//	typedef int Distance;
//	typedef double PreciseDistance;
}
using namespace Unit;

Time const UNIT_SECOND = 60;
Time _time = 0;

struct Point {
    int x, y;
	Point (int x_c, int y_c): x{x_c}, y{y_c} {}
	Point() = default;
	Point(Point const &base) = default;
};

struct PrecisePoint {
    double x, y;
	operator Point() {return (Point){(int)x,(int)y};}
	PrecisePoint (double x_c, double y_c): x{x_c}, y{y_c} {}
	PrecisePoint (Point base): x{(double)base.x}, y{(double)base.y} {}
	PrecisePoint() = default;
	PrecisePoint(PrecisePoint const &base) = default;
};

class Trajectory {
public:
    virtual PrecisePoint at            (double) const = 0; 
	virtual double       operator()    (double) const = 0;
	virtual double       getTurningArg ()       const = 0;
};

#include "tileinfo.cpp"

class Projectile {
    PrecisePoint apex;
	//velocity means horizontal, gravity means vertical.
	double gravity, velocity; 
	// this class defines t = 0 to be at the apex, so that 'Initial' velocity is 0 just like displacement and time
	PreciseTime apex_time; 
	
	
	//Distances are measured in unscaled pixels, 
	//times are measured in ticks, where UNIT_SECOND is the number of ticks in a second (approx)
	//velocity and acceleration are (obviously) pixels per tick, and pixel per tick per tick respectively
	
public:
	Projectile (Point start, int height, double gravity_c, double velocity_c, Time time_now): //RestartFromHeight constructor
		apex (start), 
		gravity (gravity_c), velocity(velocity_c)
	{
	    apex.y -= height;
		apex_time = time_now - DropToTime (height);
	}
	Projectile (PrecisePoint apex_c, double gravity_c, double velocity_c, Time apex_time_c): //aggregator
		apex (apex_c), 
		gravity (gravity_c), 
		velocity(velocity_c),
		apex_time (apex_time_c) 
	{
	}
	Projectile() = default;
	
	
	PrecisePoint Displace (Time time_now)
	{
		PrecisePoint out (apex);
	    time_now -= apex_time; //relative time
		out.x += velocity * time_now; //no acceleration
		
		time_now *= time_now; //finding square of time
		out.y += gravity * time_now / 2; // s = 1/2 at^2 + ut, but u = 0
		
		return out;
	}
	
	PreciseTime DropToTime (double height)
	{
		return sqrt (2 * height / gravity);
	}
	
	void RestartFromHeight (Point start, double height, PreciseTime time_now)
	{
	    PreciseTime half_time = DropToTime(height);
		
	    apex = start;
		apex.y -= height;
		apex.x += velocity * half_time;
		apex_time = time_now + half_time;
	}
	
	void setVelocity(double velocity_c)
	{
	    velocity = velocity_c;
	}
	void setGravity(double gravity_c)
	{
	    gravity = gravity_c;
	}
};

class Player {
public:
    Point location;
	bool grounded = true;
	int ground_velocity;
	Projectile motion;

	Player(int x_c, int y_c, double gravity_c = 1.0, double velocity_c = 1.0): location{x_c, y_c}, motion(PrecisePoint(), gravity_c, velocity_c, 0) {}
	
	
	void Jump(int jump_height, int time_now)
	{
	    grounded = false;
		motion.setVelocity (ground_velocity);
		motion.RestartFromHeight (location, jump_height, time_now);
	}
	void UpdateMotion(int floor_pos, int time_now)
	{
	    if (grounded)
		{
		    location.x += ground_velocity;
		}
		else
		{
			location = motion.Displace(time_now);
			if (location.y >= floor_pos)
			{
				grounded = true;
				location.y = floor_pos;
			}
		}
	}
	
	void Accel (int impulse)
	{
	    ground_velocity += impulse;
	}
	
	void Resist (double factor)
	{
	    ground_velocity -= factor * ground_velocity;
	}
	
} player_body = Player {0, 400};


int const FLOOR_POS = 400;








struct Input {
    bool 	move_right, 
			move_left,
			lean_back,
			lean_forward,
			action_jump,
			action_interact;
}input;

int const ID_TIMER = 1;


int jump_height = 0, prev_jump_height = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {	
	
	
	
	    case WM_CREATE:
        {
			
            UINT ret;
			ret = SetTimer(hwnd, ID_TIMER, 1000 / UNIT_SECOND, NULL);
			if(ret == 0)
				MessageBox(hwnd, "Could not SetTimer()!", "Error", MB_OK | MB_ICONEXCLAMATION);
        }
		break;
	
	    case WM_PAINT:
        {
		
            RECT rcClient;
            PAINTSTRUCT ps;
            
            GetClientRect(hwnd, &rcClient);
			HDC paint_dc = BeginPaint(hwnd, &ps);
			
            FillRect(paint_dc, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
			//render(screenH.get(), rcClient, game.playercam);
            
			EndPaint(hwnd, &ps);
        }
        break;
        case WM_TIMER:
        {
		    ++ _time;
			if (player_body.grounded)
			{
			    if (input.move_left)
				    player_body.Accel(-1);
				else if (input.move_right)
				    player_body.Accel(+1);
				else
				    player_body.Resist(0.1);
				if (input.action_jump)
				    jump_height += 10;
				else if (jump_height > 0)
				{
				    player_body.Jump(jump_height, _time);
					prev_jump_height = jump_height;
					jump_height = 0;
				}
			}
			player_body.UpdateMotion(FLOOR_POS, _time);
			
			
			RECT rcClient;
			HDC update_dc = GetDC(hwnd);
			
			GetClientRect(hwnd, &rcClient);
            FillRect(update_dc, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
			rcClient.top = FLOOR_POS;
			FillRect(update_dc, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH));
			
			HGDIOBJ default_pen = SelectObject(update_dc, GetStockObject(DC_PEN));
			DWORD default_pen_col = SetDCPenColor(update_dc, 0x000000FF);
			
			MoveToEx(update_dc, rcClient.left, FLOOR_POS - jump_height, nullptr);
			if (jump_height != 0)
				LineTo (update_dc, rcClient.right, FLOOR_POS - jump_height);
			
			SetDCPenColor(update_dc, 0x0000FF00);
			MoveToEx(update_dc, rcClient.left, FLOOR_POS - prev_jump_height, nullptr);
			LineTo (update_dc, rcClient.right, FLOOR_POS - prev_jump_height);
			
			SetDCPenColor(update_dc, 0x00FF0000);
			POINT obj;
			obj.x = player_body.location.x; obj.y = player_body.location.y;
			Ellipse (update_dc, obj.x - 10, obj.y - 20, obj.x + 10, obj.y);
			
			SetDCPenColor(update_dc, default_pen_col);
			SelectObject(update_dc, default_pen);
			
			ReleaseDC(hwnd, update_dc);
        }
        break;
	
	
        case WM_KEYDOWN: 
		    switch (wParam)
			{
			case 0x4A: //J
			    input.move_left = true;
				break;
			case 0x4C: //L
			    input.move_right = true;
				break;
			case 0x49: //I
			    input.lean_back = true;
				break;
			case 0x4B: //J
			    input.lean_forward = true;
				break;
			case 0x5A: //Z
			    input.action_jump = true;
				break;
			case 0x58: //X
			    input.action_interact = true;
				break;
			}
		break;
		case WM_KEYUP: 
            switch (wParam)
			{
			case 0x4A: //J
			    input.move_left = false;
				break;
			case 0x4C: //L
			    input.move_right = false;
				break;
			case 0x49: //I
			    input.lean_back = false;
				break;
			case 0x4B: //J
			    input.lean_forward = false;
				break;
			case 0x5A: //Z
			    input.action_jump = false;
				break;
			case 0x58: //X
			    input.action_interact = false;
				break;
			}
		break;
	/*
	*/
	
	    case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
	    default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

HWND WindowRoot (HINSTANCE hInstance, WNDCLASSEX &wc, char const my_class_name[], char const what_am_i[]);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;
	
	hwnd = WindowRoot(hInstance, wc, "TJwindow", "Minimal techno-jump GUI");

	if (hwnd == NULL)
		return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}


//I pretty much got all of this from a tutorial. 
		//I just moved it into this function and dealt with return values >.>
HWND WindowRoot (HINSTANCE hInstance, WNDCLASSEX &wc, char const my_class_name[], char const what_am_i[]) {
//initialised constants and creates a window, passing its handle back
	HWND hwnd;
	
    wc.cbSize		 = sizeof(WNDCLASSEX);
    wc.style		 = 0;
    wc.lpfnWndProc	 = WndProc;
    wc.cbClsExtra	 = 0;
    wc.cbWndExtra	 = 0;
    wc.hInstance	 = hInstance;
    wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = my_class_name;
    wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return NULL;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        my_class_name,
        what_am_i,
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 616, 630,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return NULL;
    }
	return hwnd;
}















