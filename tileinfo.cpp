

typedef bool TileData;

int const TILE_SIZE = 30;

struct MapRef {
    int x, y;
	MapRef(Point p): 
	    x(p.x / TILE_SIZE), 
		y(p.y / TILE_SIZE)
	{
	}
	MapRef(PrecisePoint p): 
	    x((int) p.x / TILE_SIZE), 
		y((int) p.y / TILE_SIZE)
	{
	}
	
};

template<typename T>
inline T StepTowards (T a, T const &b) 
{
    bool side = a < b;
    a += (side?1:-1);
	if ( side == (a<b) ) 
	    return a;
	else
	    return b; // if b falls between a and a +- 1, then return b
}



class Level
{
    TileData map[5][5];

	void vertical_test (int x, int y) {if (map[x][y]) throw 
	void horizontal_test (int, int);//these should end up being member functions anyway

	void AttemptVerticalSteps (MapRef loc, int goal_y)
	{
		while (loc.y != goal_y)
		{
			loc.y = StepTowards (loc.y, goal_y); //increase or decrease y by 1
			vertical_test(loc.x, loc.y); // check for collisions at the new y value
		}
	}

	void TestCollisions_Simple (Trajectory const &arc, MapRef start, MapRef end) //asks for a previous value of next_y_co to initialize with, and a desired row to finish on
	{
		while (start.x != end.x)
		{
			//This finds the next column, finds the relevant location of that column, and then turns that point back into a tile coordinate.
			MapRef step = arc.at(TILE_SIZE * StepTowards(step.x, end.x)); 
			AttemptVerticalSteps (start, step.y); // attempts to move up or down through these tiles
			
			if (map[step.x][step.y]) throw CollisionData (step.x * TILE_SIZE, 
			horizontal_test(step.x, step.y); // now that vertical motion for this column is handled, move into next column
			start = step; // pass calculated column back to be current column
		}
		AttemptVerticalSteps (start, end.y); //this tapes together the two _x segments, by moving from the edge of the column to the mid-goal itself
	}

public:
	
	void TestCollisions (Trajectory const &arc, double x, double end_x) {
		double mid_x = arc.getTurningArg();
		if (mid_x < x == mid_x < end_x) mid_x = x; // if mid_x isn't actually in the middle of x and end_x, then set it to either end point
		//if it is either end point then one of the last two lines will have no effect
		
		MapRef start  = arc.at(x),
			   middle = arc.at(mid_x),
			   end    = arc.at(end_x);
		
		TestCollisions_Simple(arc, start,  middle);
		TestCollisions_Simple(arc, middle, end   );
	}
};


