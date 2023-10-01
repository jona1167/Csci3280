/*

CSCI 3280, Introduction to Multimedia Systems
Spring 2023

Assignment 01 Skeleton

photomosaic.cpp

*/
//#include <time.h>
#include "stdio.h"
#include <iostream>
#include <vector>
#include <string>
#include "malloc.h"
#include "memory.h"
#include "math.h"
#include "bmp.h"		//	Simple .bmp library
#include "list_files.h" //	Simple list file library
#include <algorithm> //add
#define DBL_EPISION 1e-5//smallest difference
#define SAFE_FREE(p)  { if(p){ free(p);  (p)=NULL;} }
using namespace std;




// compute the sum of gray-scale value with in an area in bitmap
static double compute_gray_scale(Bitmap& bmp, int x1, int y1, int x2, int y2)
{
    double gray_scale = 0.0;
    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
            unsigned char r, g, b;
            bmp.getColor(x, y, r, g, b); // get gray-scale of this point
            gray_scale += 0.299 * r + 0.587 * g + 0.114 * b; //Y¡¦ = 0.299 * R + 0.587 * G + 0.114 * B
        }
    }

	return gray_scale;
}


class Error {//extra
public:
    Error(const string& msg = "")
        : msg_ { msg }
    {
    }
    const char* what() const
    {
        return msg_.c_str();
    }

private:
    string msg_;
};

// a rectangle shape (topleft to downright)
struct Shape {
    int h;
    int w;
};
ostream& operator<<(ostream& os, const Shape& shape)
{
    os << "(" << shape.w << ", " << shape.h << ")";
    return os;
}

// x,y
struct Coordinate
{
	int x;
	int y;
};
ostream& operator<<(ostream& os, const Coordinate& coord)
{
    os << "(" << coord.x << ", " << coord.y << ")";
    return os;
}


struct Surrounding//neighbors
{
  
	int x;
	int y;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	double w;  
};


class Image {
public:
    Image(const char* path, const Shape& shape)
        : path_ { path }
        , bmp_ { new Bitmap(path) }
        , gray_scale_ { -1.0 }
    {

		shape_.h = bmp_->getHeight();
		shape_.w = bmp_->getWidth();
        if (shape_.h != shape.h || shape_.w != shape.w) {//resize
            resize(shape);
        }
    }

	~Image() {
		delete bmp_;
	}

    // resize to shape
    void resize(const Shape& shape)
    {
        if (shape_.h == shape.h && shape_.w == shape.w) {//checking
            return;
        }

        // interpolation
        Bitmap* new_bmp = bilinear_interpolate(shape);

        // replace internal bmp
		delete bmp_;
		bmp_ = new_bmp;

        // set new shape
        shape_ = shape;
    }

    // get the gray_scale for the whole image
	double get_gray_scale() {
        if (gray_scale_ < -0.1) {
			gray_scale_ = compute_gray_scale(*bmp_, 0, 0, shape_.w, shape_.h);
        }

		return gray_scale_;
	}

    // get the gray scale of a range in the image
	double get_gray_scale_of_range(int x1, int y1, int x2, int y2)
	{
		return compute_gray_scale(*bmp_, x1, x2, y1, y2);
	}

    // return the internal bitmap of the image
	Bitmap& get_bitmap()
	{
		return *bmp_;
	}

    // return the size shape of the image
	Shape get_shape() const
	{
		return shape_;
	}

    // return the path of the image
	const string& get_path() const{
		return path_;
	}

private:
    // Generate a new bitmap of shape by bilinear interplolation
	Bitmap* bilinear_interpolate(const Shape& shape)
	{
        Bitmap* new_bmp = new Bitmap(shape.w, shape.h);     
        for (int x = 0; x < shape.w; x++) {
            for (int y = 0; y < shape.h; y++) {
                vector<Surrounding> surroundings(4);
                get_surroundings(shape, x, y, surroundings);
				unsigned char r, g, b;
				compute_color(surroundings, r, g, b);
				new_bmp->setColor(x, y, r, g, b);
            }
        }

		return new_bmp;
	}

    // get 4 surrounding of an unknown pixel
    void get_surroundings(const Shape& dst, int x, int y, vector<Surrounding>& surs)
    {
        // map (x, y) to original image
		double mapped_x = (double) x * shape_.w / dst.w;
		double mapped_y = (double) y * shape_.h / dst.h;

        // compute the coordinaies of its 4 surroundings
        int left = (int)mapped_x;
        int top = (int)mapped_y;
        int right = (left + 1 < shape_.w ? (left + 1) : (left - 1));
        int bottom = (top + 1 < shape_.h ? (top + 1) : (top - 1));

        surs[0].x = left;
        surs[0].y = top;
        surs[1].x = right;
        surs[1].y = top;
        surs[2].x = left;
        surs[2].y = bottom;
        surs[3].x = right;
        surs[3].y = bottom;

        // compute their weight
        for (int i = 0; i < 4; i++) {
            bmp_->getColor(surs[i].x, surs[i].y, surs[i].r, surs[i].g, surs[i].b);
            surs[i].w = (1 - fabs(surs[i].x - mapped_x)) * (1 - fabs(surs[i].y - mapped_y));
        }
    }

    // compute color by weighted average
    void compute_color(const vector<Surrounding>& surroundings,
        unsigned char& r, unsigned char& g, unsigned char& b)
    {
        double dr = 0.0;
        double dg = 0.0;
        double db = 0.0;
        for (auto const& sur : surroundings) {
            dr += sur.w * sur.r;
            dg += sur.w * sur.g;
            db += sur.w * sur.b;
        }

        r = (unsigned char)dr;
        g = (unsigned char)dg;
        b = (unsigned char)db;
    }

private:
	string path_;
    Bitmap* bmp_;
    Shape shape_;
	double gray_scale_;
};

class Tiles {
public:
    Tiles(const char* path, const Shape& shape)
        : shape_(shape)
    {

        load(path);
       
        sort_by_brightness();
    }

    ~Tiles()
    {
        for (auto tile : tiles_) {
            delete tile;
        }
    }

    // **return image of the nearest gray scale using binary search
	Image* find_nearest(double gray_scale)
	{
		int l = 0;
		int h = tiles_.size() - 1;
		int m;
		while (l < h) {
			m = (l + h) / 2;
			double m_gray_scale = tiles_[m]->get_gray_scale();
			double diff = m_gray_scale - gray_scale;
			if (fabs(diff) <= DBL_EPISION) {
				break;
			} else if (diff > DBL_EPISION) {
				h = m - 1;
			} else if (diff < -DBL_EPISION) {
				l = m + 1;
			}
		}

		return tiles_[m];
	}


	Shape get_shape() const
	{
		return shape_;
	}

private:
 
    void load(const char* tile_dir)
    {
        vector<string> tile_paths;
        list_files(tile_dir, ".bmp", tile_paths, false);

        if (tile_paths.empty()) {
            throw Error("no tiles image found");
        }

        for (auto const& path : tile_paths) {
            tiles_.push_back(new Image(path.c_str(), shape_));
        }
    }

    // **sort the tiles by brightness (gray_scale)
    void sort_by_brightness()
    {
        sort(tiles_.begin(), tiles_.end(), [](Image* a, Image* b) {
            return a->get_gray_scale() < b->get_gray_scale() - DBL_EPISION;
        });
    }

private:
    vector<Image*> tiles_;
    const Shape shape_;
};


// parse shape arguments
static void parse_shape(char* arg_cstr, Shape& output, Shape& cell)
{
    
    const string comma = ",";
    string arg(arg_cstr);
    vector<string> size_strs(4);
    size_t start = 0;
    for (int i = 0; i < 4; i++) {
        size_t end = arg.find(comma, start);
        size_strs[i] = arg.substr(start, end - start);
        start = end + 1;
    }

    
    output.w = atoi(size_strs[0].c_str());
    output.h = atoi(size_strs[1].c_str());
    cell.w = atoi(size_strs[2].c_str());
    cell.h = atoi(size_strs[3].c_str());

    if (output.h % cell.h || output.w % cell.w) {
        throw Error("Bad size in arguments");
    }
}

class Mosaic {
public:
    Mosaic(Image& source, Tiles& tiles)
        : source_(source)
        , tiles_(tiles)
    {
    }

    // compose a mosaic image with tiles and save it
    void compose(const char* output_name_)
    {
        
        Bitmap target(source_.get_shape().w, source_.get_shape().h);

        
        int tile_w = tiles_.get_shape().w;
        int tile_h = tiles_.get_shape().h;
        int output_w = source_.get_shape().w;
        int output_h = source_.get_shape().h;

        // find tiles for cells one by one
        int x1, y1, x2, y2;
        for (int r = 0; r < output_h / tile_h; r++) {
            y1 = r * tile_h;
            y2 = (r + 1) * tile_h;
            for (int c = 0; c < output_w / tile_w; c++) {
				x1 = c * tile_w;
				x2 = (c + 1) * tile_w;

                
				find_and_fill_tile(target, x1, y1, x2, y2);
			}
        }

		target.save(output_name_);
    }

private:
    
    void find_and_fill_tile(Bitmap& target, int x1, int y1, int x2, int y2)
    {
        
		double gray_scale = compute_gray_scale(source_.get_bitmap(), x1, y1, x2, y2);
		Image* tile = tiles_.find_nearest(gray_scale);
		for (int y = y1; y < y2; y++) {
			for (int x = x1; x < x2; x++) {
				unsigned char r, g, b;
				tile->get_bitmap().getColor(x - x1, y - y1, r, g, b);
				target.setColor(x, y, r, g, b);
			}
		}
}

private:
    Image& source_;
    Tiles& tiles_;
};


int main(int argc, char** argv)
{
    //clock_t Start_time = clock();
    if (argc != 5) {//error input
        cerr << "Usage:" << endl << "photomasic <input.bmp> <photo_dir> <output_w,output_h,cell_w,cell_h> <output.bmp>" << endl;
        return 0;
    }

    try {
        // Parse output and cell shape specified in argv[3]
        Shape output_shape, cell_shape;
        parse_shape(argv[3], output_shape, cell_shape);
        // cout << "output = " << output_shape<< ", cell = " << cell_shape<< endl;check

        // Read source bitmap from argv[1]
        Image source(argv[1], output_shape);
        //source.save("resized.bmp");

        // List .bmp files in argv[2] and do preprocessing
        Tiles tiles(argv[2], cell_shape);

        // Compose the output image as save to argv[4]
        Mosaic mosaic(source, tiles);
        mosaic.compose(argv[4]);
        
       // cout << "Time taken: " << (double)(clock() - Start_time) / CLOCKS_PER_SEC << endl;
        return 0;
    }
    catch (const Error & err) {//extra for checking
        cerr << err.what() << endl;
        return 0;
    }

    
    //return 0;
}