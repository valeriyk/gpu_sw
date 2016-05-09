#include "tgaimage.h"
//#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
//Model *model = NULL;
const int width  = 800;
const int height = 800;

/*
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            P.z = 0;
            for (int i=0; i<3; i++) P.z += pts[i][2]*bc_screen[i];
            if (zbuffer[int(P.x+P.y*width)]<P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}


void draw_line ();



*/

struct Point2D {
    int x, y;
};

struct Point3Df {
	float x, y, z;
};

struct Vect2D {
	Point2D a, b;
};

struct Vect3Df {
	Point3Df a, b;
};

struct Face {
	int v1idx, v2idx, v3idx;
};

Point2D world2screen(Point3Df in) {
	Point2D out;
	out.x = ((in.x + 1.0) * width / 2.0 + 0.5);
	out.y = ((in.y + 1.0) * height / 2.0 + 0.5);
	return out;
}

int orient2d(const Point2D& a, const Point2D& b, const Point2D& c)
{
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
}

int min3 (int a, int b, int c) {
	int min = a;
	if (  a > b) min = b;
	if (min > c) min = c;
	return min;
}

int max3 (int a, int b, int c) {
	int max = a;
	if (  a < b) max = b;
	if (max < c) max = c;
	return max;
}

/*float vect2d_scal_prod (Vect2D  Point2D x, Point2D y) {
}*/
/*
float vect3d_scal_prod (Vect3Df a, Vect3Df b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}
*/

void draw_triangle (const Point2D& v0, const Point2D& v1, const Point2D& v2, TGAImage &image, TGAColor color)
{
    // Compute triangle bounding box
    int minX = min3(v0.x, v1.x, v2.x);
    int minY = min3(v0.y, v1.y, v2.y);
    int maxX = max3(v0.x, v1.x, v2.x);
    int maxY = max3(v0.y, v1.y, v2.y);

    // Clip against screen bounds
    minX = max3(minX, 0, 0);
    minY = max3(minY, 0, 0);
    maxX = min3(maxX, width - 1, width);
    maxY = min3(maxY, height - 1, height);

    // Rasterize
    Point2D p;
    for (p.y = minY; p.y <= maxY; p.y++) {
        for (p.x = minX; p.x <= maxX; p.x++) {
            // Determine barycentric coordinates
            int w0 = orient2d(v1, v2, p);
            int w1 = orient2d(v2, v0, p);
            int w2 = orient2d(v0, v1, p);

            // If p is on or inside all edges, render pixel.
            if (w0 >= 0 && w1 >= 0 && w2 >= 0)
                //renderPixel(p, w0, w1, w2);
                image.set(p.x, p.y, color);
        }
    }
}

int main(int argc, char** argv) {
    
    const int NUM_OF_VERTICES = 1258;
    const int NUM_OF_FACES = 2492;
    const int ALPHA_SIZE = 16;
    
    Point3Df obj_vrt  [NUM_OF_VERTICES];
    int      vrt_idx = 0;
    
    Face     obj_face [NUM_OF_FACES];
    int      face_idx = 0;
    
    
    typedef enum {V_DATA, VT_DATA, VN_DATA, VP_DATA, F_DATA, COMMENT, EMPTY} obj_line_type;
    typedef enum {LINE_TYPE, VALUE1, VALUE2, VALUE3, VALUE4} obj_line_field;  
    typedef enum {VERTEX_IDX, TEXTURE_IDX, NORMAL_IDX} obj_face_elem;
      
    
    char alpha_num [ALPHA_SIZE];
    for (int i = 0; i < ALPHA_SIZE; i++) alpha_num[i] = '\0';
    int  alpha_idx = 0;
    
    obj_line_type     line_type  = EMPTY;
    obj_line_field    line_field = LINE_TYPE;
    obj_face_elem     face_elem  = VERTEX_IDX;
    
    // Parse Wavefront OBJ format
    FILE *fp = fopen ("obj/african_head.obj", "r");
    if (!fp) return 1;
    
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
    	char c = (char) ch;
		
		// for any line that is a comment we check only for end of line:
		if (COMMENT == line_type) {
			if (c == '\n') line_type = EMPTY;
		}
		else {
			// First detect line type
			if (LINE_TYPE == line_field) {
				if (V_DATA != line_type) {
					switch (c) {
						case '\n':
						case '\r':
						case '\t':
						case ' ':     line_type =   EMPTY; break;
						case 'g':
						case 's':
						case '#':     line_type = COMMENT; break;
						case 'f':     line_type =  F_DATA; break;
						case 'v':     line_type =  V_DATA; break;
						default:      return 2;
					}
					// Assuming that the line is not a comment,
					// we keep parsing the line type until we detect that:
					// - line is F_DATA
					// Detecting V_DATA is not sufficient to stop parsing
					// because it may turn into VT_DATA, VN_DATA or VP_DATA,
					// but we can know this only after looking at the
					// next character
					if (F_DATA == line_type) line_field = VALUE1; 
				}
				else {
					// here we finally can find out if our line type
					// was V_DATA, or it is VT_DATA/VN_DATA/VP_DATA
					switch (c) {
						case '\t':    line_type =  V_DATA; break;
						case ' ':     line_type =  V_DATA; break;
						case 't':     line_type = VT_DATA; break;
						case 'n':     line_type = VN_DATA; break;
						case 'p':     line_type = VP_DATA; break;
						default:      return 2;
					}
					// No need to further parse the line type, move
					// to parsing first value
					line_field = VALUE1;
				}
			}
			// After line type detected, read values VALUE1...VALUE4
			// accordingly
			else {
				if (((c >= '0') && (c <= '9')) || (c == '.') || (c == '-') || (c == 'e')) {
					alpha_num[alpha_idx++] = c;	
				}
				else if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '/') || (c == '\r')) {
					if (alpha_num[0] == '\0') {
						 // skip heading white spaces
						 // if unexpected delimeter is found then fail
						 if ((c != ' ') && (c != '\t')) return 2;
					}
					else {
						// convert string to number and save it
						if (V_DATA == line_type) {
							float af = (float) atof (alpha_num);
							if      (line_field == VALUE1) obj_vrt[vrt_idx].x = af;
							else if (line_field == VALUE2) obj_vrt[vrt_idx].y = af;
							else if (line_field == VALUE3) obj_vrt[vrt_idx].z = af;
						}
						else if ((F_DATA == line_type) && (VERTEX_IDX == face_elem)) {
							int ai = atoi (alpha_num);
							ai--; // decrement all indices because in OBJ they start at 1
							if      (line_field == VALUE1) obj_face[face_idx].v1idx = ai;
							else if (line_field == VALUE2) obj_face[face_idx].v2idx = ai;
							else if (line_field == VALUE3) obj_face[face_idx].v3idx = ai;
						}
						
						if (c == '/') {
							switch (face_elem) {
								case VERTEX_IDX:  face_elem = TEXTURE_IDX; break;
								case TEXTURE_IDX: face_elem =  NORMAL_IDX; break;
								case NORMAL_IDX:  return 2;
							}
						}
						else {
							switch (line_field) {
								case VALUE1:    line_field = VALUE2; break;
								case VALUE2:    line_field = VALUE3; break;
								case VALUE3:    line_field = VALUE4; break;
								case VALUE4:    break;
								case LINE_TYPE: return 2;
							}
							// reset face elem before moving to the next value
							face_elem = VERTEX_IDX;
						}
											
						// cleanup before moving on to the next value
						for (int i = 0; i < ALPHA_SIZE; i++) alpha_num[i] = '\0';
						alpha_idx = 0;
						
						if (c == '\n') {
							if      (V_DATA == line_type)  vrt_idx++;
							else if (F_DATA == line_type) face_idx++;
							line_type  = EMPTY;
							line_field = LINE_TYPE;	
						}
					}
				}
				else return 2;
			}
		}
    }
    
    fclose (fp);
    
    TGAImage image(width, height, TGAImage::RGB);
    
    Point3Df light_dir = {0.0,0.0,-0.5};
    Point3Df tri_normal;
    
    for (int i = 0; i < NUM_OF_FACES; i++) {
        Face f = obj_face[i];
        Point3Df w0 = obj_vrt[f.v1idx];
        Point3Df w1 = obj_vrt[f.v2idx];
        Point3Df w2 = obj_vrt[f.v3idx];
        Point2D  s0 = world2screen(w0);
        Point2D  s1 = world2screen(w1);
        Point2D  s2 = world2screen(w2);
        
        //vect3d_scal_prod(tri_normal, light_dir);
        // move two sides of the triangle to (0,0,0) each
        Point3Df f0;
        f0.x = w2.x - w0.x;
        f0.y = w2.y - w0.y;
        f0.z = w2.z - w0.z;
        Point3Df f1;
        f1.x = w1.x - w0.x;
        f1.y = w1.y - w0.y;
        f1.z = w1.z - w0.z;
        
        // cross product of two sides
        tri_normal.x = f0.y*f1.z - f0.z*f1.y;
        tri_normal.y = f0.z*f1.x - f0.x*f1.z; 
        tri_normal.z = f0.x*f1.y - f0.y*f1.x;
        
        // normalize the cross product
        float tri_normal_length = (float) sqrt(tri_normal.x*tri_normal.x + tri_normal.y*tri_normal.y + tri_normal.z*tri_normal.z);
        tri_normal.x = tri_normal.x/tri_normal_length;
        tri_normal.y = tri_normal.y/tri_normal_length;
        tri_normal.z = tri_normal.z/tri_normal_length;
        
        // scalar product
        float intensity = tri_normal.x*light_dir.x + tri_normal.y*light_dir.y + tri_normal.z*light_dir.z;
        
        //draw_triangle (p0, p1, p2, image, TGAColor(i%255, i%255, i%255, 255));
        if (intensity > 0) draw_triangle (s0, s1, s2, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
    }


    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    //delete model;
    return 0;
}
