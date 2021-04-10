#include <cmath>
#include <SFML/Graphics.hpp>

//=============================================================================
// Point coordinates storage
//=============================================================================
struct Point
{
    uint x;
    uint y;
};
//_____________________________________________________________________________

bool operator== (const Point &p_1, const Point &p_2)
{
    return p_1.x == p_2.x && p_1.y == p_2.y;
}
//_____________________________________________________________________________

bool operator!= (const Point &p_1, const Point &p_2)
{
    return !operator==(p_1, p_2);
}
//=============================================================================



//=============================================================================
// Pixel color storage
//=============================================================================
struct Pixel
{
    Pixel();
    Pixel(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_);
    Pixel(const uint32_t color);

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    static const Pixel White;
    static const Pixel Black;
};
//_____________________________________________________________________________

Pixel::Pixel() :
    r(0),
    g(0),
    b(0),
    a(0)
{}
//_____________________________________________________________________________

Pixel::Pixel(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) :
    r(r_),
    g(g_),
    b(b_),
    a(a_)
{}
//_____________________________________________________________________________

Pixel::Pixel(const uint32_t color)
{
    const uint8_t *rgba = reinterpret_cast<const uint8_t *>(&color);
    r = rgba[0];
    g = rgba[1];
    b = rgba[2];
    a = rgba[3];
}
//_____________________________________________________________________________

bool operator== (const Pixel &p_1, const Pixel &p_2)
{
    return p_1.r == p_2.r && p_1.g == p_2.g && p_1.b == p_2.b && p_1.a == p_2.a;
}
//_____________________________________________________________________________

bool operator!= (const Pixel &p_1, const Pixel &p_2)
{
    return !operator==(p_1, p_2);
}
//_____________________________________________________________________________

const Pixel Pixel::White = Pixel(255, 255, 255, 255);
const Pixel Pixel::Black = Pixel(0, 0, 0, 255);
//=============================================================================



//=============================================================================
class Shape_recognizer
{
public:
    Shape_recognizer();
    ~Shape_recognizer();

    bool read_image_from_file(const char *file_name);
    const Pixel get_pixel(const uint x, const uint y);
    bool recognize_shape();
    bool points_between(const Point &p_1, const Point &p_2);
    float distance_between(const Point &p_1, const Point &p_2);

private:
    Pixel *data;
    uint width;
    uint height;
    bool empty;
};
//_____________________________________________________________________________

Shape_recognizer::Shape_recognizer() :
    data(nullptr),
    width(0),
    height(0),
    empty(true)
{}
//_____________________________________________________________________________

Shape_recognizer::~Shape_recognizer()
{
    if (!empty)
        delete[] data;
}
//_____________________________________________________________________________

bool Shape_recognizer::read_image_from_file(const char *file_name)
{
    sf::Image img;

    if (img.loadFromFile(file_name))
    {   
        width = img.getSize().x;
        height = img.getSize().y;
        data = new Pixel[width * height];
        empty = false;

        const uint32_t *src_data_32 = reinterpret_cast<const uint32_t *>(img.getPixelsPtr());

        for (uint i = 0; i < width * height; ++i)
            data[i] = src_data_32[i];

        return true;
    }

    // Loading error
    return false;
}
//_____________________________________________________________________________

const Pixel Shape_recognizer::get_pixel(const uint x, const uint y)
{
    return data[y * width + x];
}
//_____________________________________________________________________________

bool Shape_recognizer::recognize_shape()
{
    if (empty)
        return false;

    // Finding shape's border points
    Point max_x = {0,     0};      // Most right point (higher on vertical line)
    Point min_x = {width, 0};      // Most left point (lower on vertical line)
    Point max_y = {0,     0};      // Most down point (right on horizontal line)
    Point min_y = {0,     height}; // Most up point (left on horizontal line)

    // Example:               x
    //-|---------------------->
    // | min_y ________ max_x
    // |      |        |
    // |      |        |
    // |      |        |
    // |      |________|
    // | min_x          max_y
    // |
    // v y

    for (uint y = 0; y < height; ++y)
        for (uint x = 0; x < width; ++x)
        {
            if (get_pixel(x, y) == Pixel::Black)
            {
                if (x > max_x.x)
                    max_x = {x, y};

                if (x <= min_x.x)
                    min_x = {x, y};

                if (y >= max_y.y)
                    max_y = {x, y};

                if (y < min_y.y)
                    min_y = {x, y};
            }

        }

    // Recognizing shape by it's border points
    if (max_x == min_x ||
        max_x == max_y ||
        max_x == min_y ||
        min_x == max_y ||
        min_x == min_y ||
        max_y == min_y)
    {
        // If any two points are equal (Triangle)
        float side_len = 0;
        float a_angle = 0;
        float b_angle = 0;

        // Counting existing side length and it's angles
        if (max_x != min_y)
        {
            side_len = distance_between(max_x, min_y);
            float a_side_len = distance_between(max_x, min_x);
            float b_side_len = distance_between(min_y, min_x);
            uint a_scalar_prod = abs(min_x.x - max_x.x) * abs(min_y.x - max_x.x) + abs(min_x.y - max_x.y) * abs(min_y.y - max_x.y);
            uint b_scalar_prod = abs(min_x.x - min_y.x) * abs(max_x.x - min_y.x) + abs(min_x.y - min_y.y) * abs(max_x.y - min_y.y);
            a_angle = acos(a_scalar_prod / (side_len * a_side_len));
            b_angle = acos(b_scalar_prod / (side_len * b_side_len));
        }
        else
        {
            side_len = distance_between(max_x, max_y);
            side_len = distance_between(max_x, max_y);
            float a_side_len = distance_between(max_x, min_x);
            float b_side_len = distance_between(max_y, min_x);
            uint a_scalar_prod = abs(min_x.x - max_x.x) * abs(max_y.x - max_x.x) + abs(min_x.y - max_x.y) * abs(max_y.y - max_x.y);
            uint b_scalar_prod = abs(min_x.x - max_y.x) * abs(max_x.x - max_y.x) + abs(min_x.y - max_y.y) * abs(max_x.y - max_y.y);
            a_angle = acos(a_scalar_prod / (side_len * a_side_len));
            b_angle = acos(b_scalar_prod / (side_len * b_side_len));
        }

        printf("Triangle with side %.2f and angles %.2f, %.2f\n", side_len, a_angle, b_angle);
    }
    else if (!points_between(max_x, max_y))
    {
        // If there are no points between some two one-side points
        // (Shape with not straight side => Circle)
        printf("Circle with radius %.2f\n", distance_between(min_x, max_x) / 2 /* r = d/2 */);
        
    }
    else if (distance_between(max_x, min_x) == distance_between(max_y, min_y))
    {
        // If diagonals are equal (Shape with straight angles)
        const float x_side_len = distance_between(min_y, max_x);
        const float y_side_len = distance_between(max_y, max_x);

        if (x_side_len == y_side_len)
        {
            // Square
            printf ("Square with side %.2f\n", x_side_len);
        }
        else
        {
            // Rectangle
            printf ("Rectangle with sides %.2f x %.2f\n", x_side_len, y_side_len);
        }
    }
    else
    {
        // Unable to recognize
        return false;
    }

    // Shape successfully recognized
    return true;

}
//_____________________________________________________________________________

bool Shape_recognizer::points_between(const Point &p_1, const Point &p_2)
{
    return false;
}
//_____________________________________________________________________________

float Shape_recognizer::distance_between(const Point &p_1, const Point &p_2)
{
    return sqrt((p_2.x - p_1.x) * (p_2.x - p_1.x) + (p_2.y - p_1.y) * (p_2.y - p_1.y));
}
//=============================================================================




int main(int argc, char *argv[])
{
    // Checking arguments count
    if (argc < 2)
    {
        printf("No input file\n");
        return 0;
    }

    // Reading image from file
    Shape_recognizer sr;
    if (!sr.read_image_from_file(argv[1]))
    {
        printf("Unable to open \"%s\"\n", argv[1]);
        return 0;
    }

    // Recognizing shape
    if (!sr.recognize_shape())
    {
        printf("Recognition error\n");
        return 0;
    }

    return 0;
}
