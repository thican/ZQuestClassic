#include "solidobject.h"
#include "base/zdefs.h"
#include "sprite.h"
#include "hero.h"

#ifdef IS_PLAYER
extern sprite_list guys;
extern HeroClass Hero;
#endif

using std::vector;

vector<solid_object*> solid_objects;

bool remove_object(solid_object* obj)
{
    bool ret = false;
    for(auto it = solid_objects.begin(); it != solid_objects.end();)
    {
        if(*it == obj)
        {
            it = solid_objects.erase(it);
            // return true; //don't keep iterating - optimization
            ret = true;
        }
        else
            ++it;
    }
    return ret;
}

void put_ffcwalkflags(BITMAP *dest, int32_t x, int32_t y)
{
	for(auto it = solid_objects.begin(); it != solid_objects.end(); ++it)
	{
		(*it)->putwalkflags(dest, x, y);
	}
}

static solid_object* curobject = NULL;

void setCurObject(solid_object* obj)
{
	curobject = obj;
}

solid_object* getCurObject()
{
	return curobject;
}

bool collide_object(solid_object const* obj)
{
	for(auto it = solid_objects.begin(); it != solid_objects.end(); ++it)
	{
		if (*it == obj || *it == curobject) continue;
		if ((*it)->collide(obj))
			return true;
	}
	return false;
}

bool collide_object(int32_t tx, int32_t ty, int32_t tw, int32_t th, solid_object const* ign)
{
	for(auto it = solid_objects.begin(); it != solid_objects.end(); ++it)
	{
		if (*it == ign || *it == curobject) continue;
		if ((*it)->collide(tx, ty, tw, th))
			return true;
	}
	return false;
}

solid_object::solid_object() : solid(false), in_solid_arr(false),
	hxsz(16), hysz(16), hxofs(0), hyofs(0),
	sxofs(0), syofs(0), sxsz_ofs(0), sysz_ofs(0),
	solidflags(0)
{}

solid_object::~solid_object()
{
	if(solid)
	{
	    remove_object(this);
	}
}

void solid_object::copy(solid_object const& other)
{
	x = other.x;
	y = other.y;
	old_x = other.old_x;
	old_y = other.old_y;
	vx = other.vx;
	vy = other.vy;
	hxsz = other.hxsz;
	hysz = other.hysz;
	hxofs = other.hxofs;
	hyofs = other.hyofs;
	sxofs = other.sxofs;
	syofs = other.syofs;
	sxsz_ofs = other.sxsz_ofs;
	sysz_ofs = other.sysz_ofs;
	solidflags = other.solidflags;
	setSolid(other.solid);
}

solid_object::solid_object(solid_object const& other) 
{
	copy(other);
}

solid_object& solid_object::operator=(solid_object const& other)
{
	copy(other);
	return *this;
}

bool solid_object::setSolid(bool set)
{
	solid = set;
	if(solid && !in_solid_arr)
	{
		solid_objects.push_back(this);
		in_solid_arr = true;
		return true;
	}
	else if(in_solid_arr && !solid)
	{
		remove_object(this);
		in_solid_arr = false;
		return true;
	}
	return false;
}
bool solid_object::getSolid() const
{
	return solid;
}
void solid_object::updateSolid()
{
	if(setSolid(solid))
		solid_update(false);
}
void solid_object::setTempNonsolid(bool set)
{
	ignore_solid_temp = set;
}
bool solid_object::getTempNonsolid() const
{
	return ignore_solid_temp;
}

bool solid_object::collide(solid_object const* o) const
{
	if(ignore_solid_temp) return false;
	return collide(o->x + o->hxofs + o->sxofs,
	               o->y + o->hyofs + o->syofs,
	               o->hxsz + o->sxsz_ofs,
	               o->hysz + o->sysz_ofs);
}
bool solid_object::collide(int32_t tx, int32_t ty, int32_t tw, int32_t th) const
{
	if(ignore_solid_temp) return false;
	int32_t rx = x+hxofs+sxofs, ry = y+hyofs+syofs;
	int32_t rw = hxsz+sxsz_ofs, rh = hysz+sysz_ofs;
	return tx+tw>rx && ty+th>ry &&
	       tx<rx+rw && ty<ry+rh;
}

void solid_object::putwalkflags(BITMAP *dest, int32_t tx, int32_t ty)
{
	if(ignore_solid_temp) return;
	tx += x.getFloor() + hxofs + sxofs;
	ty += y.getFloor() + hyofs + syofs;
	rectfill(dest, tx, ty, tx + hxsz-1 + sxsz_ofs,
	         ty + hysz-1 + sysz_ofs, makecol(255,85,85));
}

void solid_object::solid_update(bool push)
{
#ifdef IS_PLAYER
	if(push && solid)
	{
		if(x != old_x || y != old_y)
		{
			Hero.solid_push(this);
			guys.solid_push(this);
		}
	}
#endif
	old_x = x;
	old_y = y;
}
void solid_object::solid_push(solid_object* pusher)
{
	//Default behavior: Ignore
	//!TODO SOLIDPUSH Implement 'enemy::solid_push'
	//!TODO SOLIDPUSH finish 'HeroClass::solid_push' (good for 4-dir, needs diagonals)
}

void solid_object::solid_push_int(solid_object const* obj,zfix& dx, zfix& dy) const
{
	dx = dy = 0;
	zfix odx = obj->x - obj->old_x,
	     ody = obj->y - obj->old_y,
	     obj_x = obj->x + obj->hxofs + obj->sxofs,
	     obj_y = obj->y + obj->hyofs + obj->syofs,
	     obj_ox = obj->old_x + obj->hxofs + obj->sxsz_ofs,
	     obj_oy = obj->old_y + obj->hyofs + obj->sysz_ofs;
	int32_t obj_w = obj->hxsz + obj->sxsz_ofs,
	        obj_h = obj->hysz + obj->sysz_ofs;
	
	zfix rx = x+hxofs+sxofs, ry = y+hyofs+syofs,
		 rw = hxsz+sxsz_ofs, rh = hysz+sysz_ofs;
	
	if(odx && ody)
	{
		//first lets see if the object is even in range; if not we can skip all these checks
		if (collide((odx > 0?obj_ox:obj_x), (ody > 0 ? obj_oy : obj_y), abs(odx)+obj_w, abs(ody)+obj_h))
		{
			//we're grabbing the corners of the object. These coordinates assume a rotrect either going upright or downleft.
			zfix leftx = obj_x;
			zfix topy = obj_y;
			zfix todx = odx;
			zfix ox, oy, nx, ny;
			zfix rightx = leftx + obj_w - 1;
			zfix bottomy = topy + obj_h - 1;
			if (odx > 0)
			{
				ox = leftx;
				nx = rightx;
			}
			else
			{
				ox = rightx;
				nx = leftx;
			}
			if (ody > 0)
			{
				oy = topy;
				ny = bottomy;
			}
			else
			{
				oy = bottomy;
				ny = topy;
			}
			if (ody > 0)
			{
				todx = -odx;
				leftx = obj_ox;
				topy = obj_oy;
			}
			zfix centerx = leftx + (obj_w/2)-1;
			zfix centery = topy + (obj_h/2)-1;
			rightx = leftx + obj_w - 1;
			bottomy = topy + obj_h - 1;

			
			//however, depending on the direction, we want to use different corners of the object.
			//thankfully, we can tell the direction by getting the signs of the velocity. if they're both equal, it's upleft and downright instead.
			
			zfix lefty = (sign(odx) == sign(ody)) ? bottomy : topy;
			zfix righty = (sign(odx) == sign(ody)) ? topy : bottomy;
			zfix side = 0;
			bool collided = false;
			if (lineBoxCollision(leftx, lefty, leftx+todx, lefty+abs(ody), rx, ry, rw, rh))
			{
				--side;
				collided = true;
			}
			if (lineBoxCollision(rightx, righty, rightx+todx, righty+abs(ody), rx, ry, rw, rh))
			{
				++side;
				collided = true;
			}
			if(!collided)
			{
				if (insideRotRect(rx+((rw)/2)-1, ry+((rh)/2)-1, leftx, lefty, leftx+todx, lefty+abs(ody), centerx, centery, centerx+todx, centery+abs(ody)))
				{
					--side;
					collided = true;
				}
				if (insideRotRect(rx+((rw)/2)-1, ry+((rh)/2)-1, centerx, centery, centerx+todx, centery+abs(ody), rightx, righty, rightx+todx, righty+abs(ody)))
				{
					++side;
					collided = true;
				}
			}
			if (!collided && collide(obj_x, obj_y, obj_w, obj_h)) 
			{
				double val = comparePointLine(rx+rw/2, ry+rh/2, ox, oy, nx, ny);
				if (abs(val) > 6) side = sign(val);
				collided = true;
			}
			if (collided)
			{
				if (lineBoxCollision(ox, oy, nx, ny, rx+(rw/4), ry+(rh/4), rw/2, rh/2))
				{
					side = 0;
				}
				byte pdir = 0; //topleft
				byte pdir2 = 0; //the dir the thing will actually be pushed in
				if (odx >= 0 && ody < 0) pdir = 1; //topright
				if (odx < 0 && ody >= 0) pdir = 2; //bottomleft
				if (odx >= 0 && ody >= 0) pdir = 3; //bottomright
				
				switch(pdir)
				{
					case 0:
					{
						if (side < 0) pdir2 = left;
						else if (side > 0) pdir2 = up;
						else pdir2 = l_up;
						break;
					}
					case 1:
					{
						if (side < 0) pdir2 = up;
						else if (side > 0) pdir2 = right;
						else pdir2 = r_up;
						break;
					}
					case 2:
					{
						if (side < 0) pdir2 = left;
						else if (side > 0) pdir2 = down;
						else pdir2 = l_down;
						break;
					}
					case 3:
					{
						if (side < 0) pdir2 = down;
						else if (side > 0) pdir2 = right;
						else pdir2 = r_down;
						break;
					}
					default:
						break;
				}
				zfix orx = rx;
				zfix ory = ry;
				int32_t count = 0;
				while (side < 0 ? (insideRotRect(rx+((rw)/2)-1, ry+((rh)/2)-1, leftx, lefty, leftx+todx, lefty+abs(ody), centerx, centery, centerx+todx, centery+abs(ody))
						|| lineBoxCollision(leftx, lefty, leftx+todx, lefty+abs(ody), rx, ry, rw, rh)) 
						: (side > 0 ? (insideRotRect(rx+((rw)/2)-1, ry+((rh)/2)-1, centerx, centery, centerx+todx, centery+abs(ody), rightx, righty, rightx+todx, righty+abs(ody))
							|| lineBoxCollision(rightx, righty, rightx+todx, righty+abs(ody), rx, ry, rw, rh))
							: (lineBoxCollision(ox, oy, nx, ny, rx+(rw/4), ry+(rh/4), rw/2, rh/2) || obj->collide(rx, ry, rw, rh))))
				{
					bool check = true;
					if (obj->collide(rx, ry, rw, rh))
					{
						double val = comparePointLine(rx+rw/2, ry+rh/2, ox, oy, nx, ny);
						if (side == (abs(val) > 6 ? sign(val) : 0)) check = false;
					}
					if (check)
					{
						if (side < 0)
						{
							if (!(insideRotRect(rx+((rw)/2)-1, ry+((rh)/2)-1, leftx, lefty, leftx+todx, lefty+abs(ody), centerx, centery, centerx+todx, centery+abs(ody))
								|| lineBoxCollision(leftx, lefty, leftx+todx, lefty+abs(ody), rx, ry, rw, rh))) 
									break;
						}
						else if (side > 0)
						{
							if (!(insideRotRect(rx+((rw)/2)-1, ry+((rh)/2)-1, centerx, centery, centerx+todx, centery+abs(ody), rightx, righty, rightx+todx, righty+abs(ody))
								|| lineBoxCollision(rightx, righty, rightx+todx, righty+abs(ody), rx, ry, rw, rh)))
									break;
						}
						else if (!lineBoxCollision(ox, oy, nx, ny, rx+(rw/4), ry+(rh/4), rw/2, rh/2))
							break;
					}
					switch(pdir2)
					{
						case up:
						{
							--ry;
							break;
						}
						case down:
						{
							++ry;
							break;
						}
						case left:
						{
							--rx;
							break;
						}
						case right:
						{
							++rx;
							break;
						}
						case l_up:
						{
							--rx;
							--ry;
							break;
						}
						case r_up:
						{
							++rx;
							--ry;
							break;
						}
						case l_down:
						{
							--rx;
							++ry;
							break;
						}
						case r_down:
						{
							++rx;
							++ry;
							break;
						}
						default:
							break;
					}
				}
				dx = rx - orx;
				dy = ry - ory;
			}
			// if (collide(obj_x, obj_y, obj_w, obj_h))
			// {
				
				// if (abs(obj_x+sxofs - rx) > abs(obj_y + ((ry - obj_y) < 0? syofs:0) - ry))
				// {
					// if ((rx - obj_x) > 0) dx = obj_x + obj_w - rx;
					// else dx = obj_x - rw - rx;
				// }
				// else
				// {
					// if ((ry - obj_y) > 0) dy = obj_y + obj_h - ry;
					// else dy = obj_y - rh - ry;
				// }
			// }
		}
		return;
	}
	else if(odx)
	{
		if(odx > 0) //right
		{
			if(collide(obj_ox+obj_w, obj_oy, odx, obj_h)) //collided
			{
				dx = obj_x + obj_w - rx;
			}
		}
		else //left
		{
			if(collide(obj_x, obj_oy, -odx, obj_h)) //collided
			{
				dx = obj_x - rw - rx;
			}
		}
	}
	else if(ody)
	{
		if(ody > 0) //down
		{
			if(collide(obj_ox, obj_oy+obj_h, obj_w, ody)) //collided
			{
				dy = obj_y + obj_h - ry;
			}
		}
		else //up
		{
			if(collide(obj_ox, obj_y, obj_w, -ody)) //collided
			{
				dy = obj_y - rh - ry;
			}
		}
	}
}

