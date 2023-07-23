///////////////////////////////////////////////////////////////////////////////
//
//      TargaImage.cpp                          Author:     Stephen Chenney
//                                              Modified:   Eric McDaniel
//                                              Date:       Fall 2004
//
//      Implementation of TargaImage methods.  You must implement the image
//  modification functions.
//
///////////////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TargaImage.h"
#include "libtarga.h"
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

// constants
const int           RED             = 0;                // red channel
const int           GREEN           = 1;                // green channel
const int           BLUE            = 2;                // blue channel
const unsigned char BACKGROUND[3]   = { 0, 0, 0 };      // background color


// Computes n choose s, efficiently
double Binomial(int n, int s)
{
    double        res;

    res = 1;
    for (int i = 1 ; i <= s ; i++)
        res = (n - i + 1) * res / i ;

    return res;
}// Binomial


///////////////////////////////////////////////////////////////////////////////
//
//      Constructor.  Initialize member variables.
//
///////////////////////////////////////////////////////////////////////////////
TargaImage::TargaImage() : width(0), height(0), data(NULL)
{}// TargaImage

///////////////////////////////////////////////////////////////////////////////
//
//      Constructor.  Initialize member variables.
//
///////////////////////////////////////////////////////////////////////////////
TargaImage::TargaImage(int w, int h) : width(w), height(h)
{
   data = new unsigned char[width * height * 4];
   ClearToBlack();
}// TargaImage



///////////////////////////////////////////////////////////////////////////////
//
//      Constructor.  Initialize member variables to values given.
//
///////////////////////////////////////////////////////////////////////////////
TargaImage::TargaImage(int w, int h, unsigned char *d)
{
    int i;

    width = w;
    height = h;
    data = new unsigned char[width * height * 4];

    for (i = 0; i < width * height * 4; i++)
	    data[i] = d[i];
}// TargaImage

///////////////////////////////////////////////////////////////////////////////
//
//      Copy Constructor.  Initialize member to that of input
//
///////////////////////////////////////////////////////////////////////////////
TargaImage::TargaImage(const TargaImage& image) 
{
   width = image.width;
   height = image.height;
   data = NULL; 
   if (image.data != NULL) {
      data = new unsigned char[width * height * 4];
      memcpy(data, image.data, sizeof(unsigned char) * width * height * 4);
   }
}


///////////////////////////////////////////////////////////////////////////////
//
//      Destructor.  Free image memory.
//
///////////////////////////////////////////////////////////////////////////////
TargaImage::~TargaImage()
{
    if (data)
        delete[] data;
}// ~TargaImage


///////////////////////////////////////////////////////////////////////////////
//
//      Converts an image to RGB form, and returns the rgb pixel data - 24 
//  bits per pixel. The returned space should be deleted when no longer 
//  required.
//
///////////////////////////////////////////////////////////////////////////////
unsigned char* TargaImage::To_RGB(void)
{
    unsigned char   *rgb = new unsigned char[width * height * 3];
    int		    i, j;

    if (! data)
	    return NULL;

    // Divide out the alpha
    for (i = 0 ; i < height ; i++)
    {
	    int in_offset = i * width * 4;
	    int out_offset = i * width * 3;

	    for (j = 0 ; j < width ; j++)
        {
	        RGBA_To_RGB(data + (in_offset + j*4), rgb + (out_offset + j*3));
	    }
    }

    return rgb;
}// TargaImage


///////////////////////////////////////////////////////////////////////////////
//
//      Save the image to a targa file. Returns 1 on success, 0 on failure.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Save_Image(const char *filename)
{
    TargaImage	*out_image = Reverse_Rows();

    if (! out_image)
	    return false;

    if (!tga_write_raw(filename, width, height, out_image->data, TGA_TRUECOLOR_32))
    {
	    cout << "TGA Save Error: %s\n", tga_error_string(tga_get_last_error());
	    return false;
    }

    delete out_image;

    return true;
}// Save_Image


///////////////////////////////////////////////////////////////////////////////
//
//      Load a targa image from a file.  Return a new TargaImage object which 
//  must be deleted by caller.  Return NULL on failure.
//
///////////////////////////////////////////////////////////////////////////////
TargaImage* TargaImage::Load_Image(char *filename)
{
    unsigned char   *temp_data;
    TargaImage	    *temp_image;
    TargaImage	    *result;
    int		        width, height;

    if (!filename)
    {
        cout << "No filename given." << endl;
        return NULL;
    }// if

    temp_data = (unsigned char*)tga_load(filename, &width, &height, TGA_TRUECOLOR_32);
    if (!temp_data)
    {
        cout << "TGA Error: %s\n", tga_error_string(tga_get_last_error());
	    width = height = 0;
	    return NULL;
    }
    temp_image = new TargaImage(width, height, temp_data);
    free(temp_data);

    result = temp_image->Reverse_Rows();

    delete temp_image;

    return result;
}// Load_Image


///////////////////////////////////////////////////////////////////////////////
//
//      Convert image to grayscale.  Red, green, and blue channels should all 
//  contain grayscale value.  Alpha channel shoould be left unchanged.  Return
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::To_Grayscale()
{   
    for (int i = 0; i < (width * height * 4); i = i + 4)//looping by height
    {
            data[i] = data[i] * 0.299 + data[i + 1] * 0.587 + data[i + 2] * 0.114;//grayscale formula
            data[i + 1] = data[i];//passing the value to green
            data[i + 2] = data[i];//passing the value to 
    }
	return true;
}// To_Grayscale


///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using uniform quantization.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Quant_Uniform()
{
    unsigned char bits_per_channel = 8;//total bits per channel
    unsigned char red_bits = 3; unsigned char green_bits = 3; unsigned char blue_bits = 2;//bits allocated per color channel
    for (int i = 0; i < height; i++)//looping by height
    {
        int offset = i * width * 4;//offset to point to the height in data
        for (int j = 0; j < width; j++)//looping by width
        {
            int current_pixel = offset + j * 4;//point to start of current pixel in data
            data[current_pixel] = data[current_pixel] >> (bits_per_channel-red_bits) << (bits_per_channel - red_bits);
            data[current_pixel + 1] = data[current_pixel + 1] >> (bits_per_channel - green_bits) << (bits_per_channel - green_bits);
            data[current_pixel + 2] = data[current_pixel + 2] >> (bits_per_channel - blue_bits) << (bits_per_channel - blue_bits);
        }
    }
    return true;
}// Quant_Uniform


///////////////////////////////////////////////////////////////////////////////
//
//      Convert the image to an 8 bit image using populosity quantization.  
//  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
//Map information obtained from: https://www.geeksforgeeks.org/map-associative-containers-the-c-standard-template-library-stl/
//                               https://www.educative.io/answers/how-to-sort-a-map-by-value-in-cpp
bool order(const pair<std::vector<unsigned char>, int>& x,//comparison function necessary for color histogram
           const pair<std::vector<unsigned char>, int>& y)
{
    return (x.second > y.second);
}
bool TargaImage::Quant_Populosity()
{
    unsigned char bits_per_channel = 8;//total bits per channel
    unsigned char red_bits = 5; unsigned char green_bits = 5; unsigned char blue_bits = 5;//bits allocated per color channel
    for (int i = 0; i < height; i++)//uniform quantization to reduce the amount of colors before populosity
    {
        int offset = i * width * 4;
        for (int j = 0; j < width; j++)
        {
            int current_pixel = offset + j * 4;//point to start of current pixel in data
            data[current_pixel] = data[current_pixel] >> (bits_per_channel - red_bits) << (bits_per_channel - red_bits);
            data[current_pixel + 1] = data[current_pixel + 1] >> (bits_per_channel - green_bits) << (bits_per_channel - green_bits);
            data[current_pixel + 2] = data[current_pixel + 2] >> (bits_per_channel - blue_bits) << (bits_per_channel - blue_bits);
        }
    }
    std::vector<std::vector<unsigned char>> data_vector;//vector to store the rgb vector of each pixel
    for (int i = 0; i < (height * width * 4); i = i + 4)//the rgb of each pixel is stored in a vector which is then stored in another vector
    {
        std::vector<unsigned char> rgb;//vector of rgb values to be stored inside the data_vector
        for (int j = 0; j < 3; j++)
        {
            rgb.push_back(data[i + j]);
        }
        data_vector.push_back(rgb);
        rgb.clear();
    }
    std::map<std::vector<unsigned char>, int> color_map;
    for (int i = 0; i < data_vector.size(); i++)//create the color map by counting the color frequency
    {
        if (color_map.find(data_vector[i]) == color_map.end())
            color_map.insert({ data_vector[i] , 1 });
        else
            color_map[data_vector[i]]++;
    }
    data_vector.clear();
    std::vector<pair<std::vector<unsigned char>, int>> unordered_histogram;//sorting a map is complex so copy to a vector pair instead
    map<std::vector<unsigned char>, int>::iterator I;//map iterator
    for (I = color_map.begin(); I != color_map.end(); I++)//create the color map by counting the color frequency
    {
        unordered_histogram.push_back(make_pair(I->first, I->second));
    }
    sort(unordered_histogram.begin(), unordered_histogram.end(), order);//sorting the vector of the map from most to less frequent
    std::vector<std::vector<unsigned char>> color_histogram;
    for (int i = 0; i < 256; i++)//create the color map by counting the color frequency
    {
        color_histogram.push_back(unordered_histogram[i].first);
    }
    unordered_histogram.clear();
    for (int i = 0; i < (height * width * 4); i = i + 4)//map each pixel to its closest color in the histogram
    {
        float distance = 0, min_distance = 500;
        unsigned char chosen_color[3] = {};//array of rgb values
        for (int j = 0; j < 256; j++)//euclidean distance to each color in the histogram
        {
            distance = sqrt(pow(abs(data[i] - color_histogram[j][0]),2) + pow(abs(data[i + 1] - color_histogram[j][1]), 2) + pow(abs(data[i + 2] - color_histogram[j][2]), 2));
            if (distance < min_distance)
            {
                min_distance = distance;
                chosen_color[0] = color_histogram[j][0];
                chosen_color[1] = color_histogram[j][1];
                chosen_color[2] = color_histogram[j][2];
            }
        }
        data[i] = chosen_color[0];
        data[i + 1] = chosen_color[1];
        data[i + 2] = chosen_color[2];
    }
    return true;
}// Quant_Populosity


///////////////////////////////////////////////////////////////////////////////
//
//      Dither the image using a threshold of 1/2.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Dither_Threshold()
{   
    float* temp = new float[4 * height * width];//create a temporary dynamic array in memory to store the intensity
    To_Grayscale();
    for (int i = 0; i < (width * height * 4); i = i + 4)//loop through the image
    {
        temp[i] = data[i] / (float)256;//intensity range set to [0.0, 1.0]
        if (temp[i] > 0.5)//if threshold is passed set to white, otherwise black
        {
            data[i] = 255;
            data[i + 1] = 255;
            data[i + 2] = 255;
        }
        else
        {
            data[i] = 0;
            data[i + 1] = 0;
            data[i + 2] = 0;
        }
    }
    delete[] temp;//erase the temporary array
    return true;
}// Dither_Threshold


///////////////////////////////////////////////////////////////////////////////
//
//      Dither image using random dithering.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
//Random number reference: https://en.cppreference.com/w/cpp/numeric/random
bool TargaImage::Dither_Random()
{
    float* temp = new float[4 * height * width];//create a temporary dynamic array in memory to store the intensity
    To_Grayscale();
    for (int i = 0; i < (width * height * 4); i = i + 4)//adding random value to intensity
    {
        int random = (rand() % 103) -51;//[0, 102] -> [-51, 51] == [-0.2, 0.2]
        if ((data[i] + random) < 0)
        {
            data[i] = 0;
            data[i + 1] = 0;
            data[i + 2] = 0;
        }
        else if ((data[i] + random) > 255)
        {
            data[i] = 255;
            data[i + 1] = 255;
            data[i + 2] = 255;
        }
        else
        {
            data[i] = data[i] + random;
            data[i + 1] = data[i + 1] + random;
            data[i + 2] = data[i + 2] + random;
        }
    }
    for (int i = 0; i < (width * height * 4); i = i + 4)//loop through the image
    {
        temp[i] = data[i] / (float)256;//intensity range set to [0.0, 1.0]
        if (temp[i] > 0.5)//if threshold is passed set to white, otherwise black
        {
            data[i] = 255;
            data[i + 1] = 255;
            data[i + 2] = 255;
        }
        else
        {
            data[i] = 0;
            data[i + 1] = 0;
            data[i + 2] = 0;
        }
    }
    delete[] temp;//erase the temporary array
    return true;
}// Dither_Random


///////////////////////////////////////////////////////////////////////////////
//
//      Perform Floyd-Steinberg dithering on the image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
float find_closest_palette_color(float old_pixel, float* palette, int palette_size)
{
    float closest_color = palette[0];
    float closest_distance = fabs(old_pixel - palette[0]);

    for (int i = 0; i < palette_size; i++) {
        float distance = fabs(old_pixel - palette[i]);
        if (distance < closest_distance) {
            closest_distance = distance;
            closest_color = palette[i];
        }
    }
    return closest_color;
}
bool TargaImage::Dither_FS()
{
    To_Grayscale();
    float dither_palette[2] = { 0.0, 1.0 };
    float old_pixel, new_pixel;//temp variables to store pixel intensity
    float quant_error;//quantitation error
    float* I = new float[height * width];//create a temporary dynamic array in memory to store the intensity;
    for (int i = 0; i < (width * height); i++)
    {
        I[i] = data[i * 4] / (float)256;//intensity range set to [0.0, 1.0];
    }
    for (int i = 0; i < height; i++)//looping to do error correction
    {
        for (int j = 0; j < width; j++)
        {
            int pixel = (i * width) + j;//offset to point to the height in data
            old_pixel = I[pixel];
            new_pixel = find_closest_palette_color(old_pixel, dither_palette , 2);
            I[pixel] = new_pixel;
            quant_error = old_pixel - new_pixel;
            if ((j + 1) < width)
            {
                I[pixel + 1] = I[pixel + 1] + (0.4375 * quant_error);
            }
            if ((i + 1) < height && (j - 1) >= 0)
            {
                I[pixel + width - 1] = I[pixel + width - 1] + (0.1875 * quant_error);
            }
            if ((i + 1) < height)
            {
                I[pixel + width] = I[pixel + width] + (0.3125 * quant_error);
            }
            if ((i + 1) < height && (j + 1) < width)
            {
                I[pixel + width + 1] = I[pixel + width + 1] + (0.0625 * quant_error);
            }
        }
    }
    for (int i = 0; i < (width * height); i++)
    {
        if (I[i] == float(1))//if threshold is passed set to white, otherwise black
        {
            data[i * 4] = 255;
            data[i * 4 + 1] = 255;
            data[i * 4 + 2] = 255;
        }
        else
        {
            data[i * 4] = 0;
            data[i * 4 + 1] = 0;
            data[i * 4 + 2] = 0;
        }
    }
    delete[] I;
    return true;

}// Dither_FS


///////////////////////////////////////////////////////////////////////////////
//
//      Dither the image while conserving the average brightness.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Dither_Bright()
{
    float intensity_sum = 0;//sum of intensity in each pixel
    float average_bright;//average brightness
    int black_ratio;//constant ratio of black and white that should be maintained
    float threshold;//chosen threshold to maintain brightness
    int j;//temp variable
    std::vector<int> intensity_order;//create a temporary dynamic array in memory to store the intensity in increasing order
    std::vector<float> intensity;//create a temporary dynamic array in memory to store the intensity
    To_Grayscale();//grayscale
    for (int i = 0; i < (width * height * 4); i = i + 4)//loop through the image
    {
        intensity_order.push_back(data[i]);//intensity is stored for sorting
    }
    int size = sizeof(intensity_order) / sizeof(intensity_order[0]);
    sort(intensity_order.begin(), intensity_order.end());
    for (int i = 0,j=0; i < (width * height * 4); i = i + 4,j++)//loop through the image
    {
        intensity.push_back(data[i] / (float)256);//range from 0 to 1 for intensity
        intensity_sum += intensity[j];//intensity used in sum
    }
    average_bright = intensity_sum/(width*height);//average brightness is calculated
    black_ratio = (1 - average_bright)*100; //percent of black pixels based on 100 pixels
    black_ratio = (black_ratio * (width * height) / 100);//pixel in the ascending order intensity representative enough to preserve brightness
    threshold = intensity_order[black_ratio] / (float)256;
    for (int i = 0, j = 0; i < (width * height * 4); i = i + 4, j++)//loop through the image
    {
        if (intensity[j] > threshold)//if threshold is passed set to white, otherwise black
        {
            data[i] = 255;
            data[i + 1] = 255;
            data[i + 2] = 255;
        }
        else
        {
            data[i] = 0;
            data[i + 1] = 0;
            data[i + 2] = 0;
        }
    }
    intensity.clear();
    intensity_order.clear();
    return true;
}// Dither_Bright


///////////////////////////////////////////////////////////////////////////////
//
//      Perform clustered differing of the image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Dither_Cluster()
{
    float cluster[4][4] = { {0.7059, 0.3529, 0.5882, 0.2353},
                            {0.0588, 0.9412, 0.8235, 0.4118},
                            { 0.4706, 0.7647, 0.8824, 0.1176},
                            {0.1765, 0.5294,  0.2941, 0.6471} };//Array of cluster thresholds
    float* temp = new float[4 * height * width];//create a temporary dynamic array in memory to store the intensity
    To_Grayscale();
    for (int i = 0; i < (width * height * 4); i = i + 4)//loop through the image
    {
        temp[i] = data[i] / (float)256;//intensity range set to [0.0, 1.0]
        temp[i + 1] = data[i + 1] / (float)256;
        temp[i + 2] = data[i + 2] / (float)256;
    }
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int pixel = (i * width * 4) + (j * 4);
            if (temp[pixel] > cluster[j%4][i%4])//if threshold is passed set to white, otherwise black
            {
                data[pixel] = 255;
                data[pixel + 1] = 255;
                data[pixel + 2] = 255;
            }
            else
            {
                data[pixel] = 0;
                data[pixel + 1] = 0;
                data[pixel + 2] = 0;
            }
        }
    }
    delete[] temp;//erase the temporary array
    return true;
}// Dither_Cluster


///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using Floyd-Steinberg dithering over
//  a uniform quantization - the same quantization as in Quant_Uniform.
//  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Dither_Color()
{
    float old_red, old_green, old_blue, new_red, new_green, new_blue;//temp variables to store pixel intensity
    float quant_error_red, quant_error_green, quant_error_blue;//quantitation error
    float red_palette[8] = {0.0, 0.140625, 0.28515625, 0.42578125, 0.5703125, 0.7109375, 0.85546875, 1.0};//color palettes for rgb is declared ranges from 0 to 1 and 0 to 255
    unsigned char red_table[8] = { 0, 36, 73, 109, 146, 182, 219, 255 };
    float green_palette[8] = { 0.0, 0.140625, 0.28515625, 0.42578125, 0.5703125, 0.7109375, 0.85546875, 1.0 };
    unsigned char green_table[8] = { 0, 36, 73, 109, 146, 182, 219, 255 };
    float blue_palette[4] = { 0, 0.33203125, 0.6640625, 1 };
    unsigned char blue_table[4] = { 0, 85, 170, 255 };
    unsigned char* rgb = new unsigned char[3 * height * width];
    float* I = new float[3 * height * width];//create a temporary dynamic array in memory to store the intensity of the rgb;
    int iterator;//rgb iterator
    rgb = To_RGB();
    for (int i = 0; i < (width * height * 3); i++)//adding random value to intensity
    {
        I[i] = rgb[i] / (float)256;//intensity range set to [0.0, 1.0];
    }
    for (int i = 0; i < height; i++) 
    {
        for (int j = 0; j < width * 3; j += 3) 
        {
            int red = (i * width * 3) + j;
            int green = red + 1;
            int blue = green + 1;
            old_red = I[red];
            old_green = I[green];
            old_blue = I[blue];
            new_red = find_closest_palette_color(old_red,red_palette,8);//mapping to the closest intensity for rgb
            new_green = find_closest_palette_color(old_green,green_palette,8);
            new_blue = find_closest_palette_color(old_blue,blue_palette,4);
            I[red] = new_red;//the new intensity is stored in the intensity array
            I[green] = new_green;
            I[blue] = new_blue;
            quant_error_red = old_red - new_red;//the distance is taken as error
            quant_error_green = old_green - new_green;
            quant_error_blue = old_blue - new_blue;
            if ((red + 3) <= (height * width * 3))
                I[red + 3] = I[red + 3] + (0.4375 * quant_error_red);
            if ((green + 3) <= (height * width * 3))
                I[green + 3] = I[green + 3] + (0.4375 * quant_error_green);
            if ((blue + 3) <= (height * width * 3))
                I[blue + 3] = I[blue + 3] + (0.4375 * quant_error_blue);

            if ((red + (width * 3) - 3) <= (height * width * 3))
                I[red + (width * 3) - 3] = I[red + (width * 3) - 3] + (0.1875 * quant_error_red);
            if ((green + (width * 3) - 3) <= (height * width * 3))
                I[green + (width * 3) - 3] = I[(green + (width * 3) - 3)] + (0.1875 * quant_error_green);
            if ((blue + (width * 3) - 3) <= (height * width * 3))
                I[blue +(width * 3) - 3] = I[(blue + (width * 3) - 3)] + (0.1875 * quant_error_blue);

            if ((red + (width * 3)) <= (height * width * 3))
                I[red + (width * 3)] = I[red + (width * 3)] + (0.3125 * quant_error_red);
            if ((green + (width * 3)) <= (height * width * 3))
                I[green + (width * 3)] = I[green + (width * 3)] + (0.3125 * quant_error_green);
            if ((blue + (width * 3)) <= (height * width * 3))
                I[blue + (width * 3)] = I[blue + (width * 3)] + (0.3125 * quant_error_blue);
      
            if ((red + (width * 3) + 3) <= (height * width * 3))
                I[red + (width * 3) + 3] = I[red + (width * 3) + 3] + (0.0625 * quant_error_red);
            if ((green + (width * 3) + 3) <= (height * width * 3))
                I[green + (width * 3) + 3] = I[green + (width * 3) + 3] + (0.0625 * quant_error_green);
            if ((blue + (width * 3) + 3) <= (height * width * 3))
                I[blue + (width * 3) + 3] = I[blue + (width * 3) + 3] + (0.0625 * quant_error_blue);
        }
    }
    for (int i = 0, iterator = 0; i < (width * height * 4); i = i + 4, iterator = iterator + 3)//the intensity in I is checked to replace data 
    {                                                                                          //with the corresponding value in 0 to 255
        float red_difference = 1,green_difference = 1,blue_difference = 1;
        unsigned char red_value, green_value, blue_value;
        for (int j = 0; j < 8; j++)
        {
            float difference = fabs(I[iterator] - red_palette[j]);
            if (difference < red_difference)
            {
                red_difference = difference;
                red_value = red_table[j];
            }
        }
        for (int j = 0; j < 8; j++)
        {
            float difference = fabs(I[iterator + 1] - green_palette[j]);
            if (difference < green_difference)
            {
                green_difference = difference;
                green_value = green_table[j];
            }
        }
        for (int j = 0; j < 4; j++)
        {
            float difference = fabs(I[iterator + 2] - blue_palette[j]);
            if (difference < blue_difference)
            {
                blue_difference = difference;
                blue_value = blue_table[j];
            }
        }
        data[i] = red_value;
        data[i + 1] = green_value;
        data[i + 2] = blue_value;
    }
    delete[] I;
    delete[] rgb;
    return true;
}// Dither_Color


///////////////////////////////////////////////////////////////////////////////
//
//      Composite the current image over the given image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Comp_Over(TargaImage* pImage)
{
    if (width != pImage->width || height != pImage->height)
    {
        cout <<  "Comp_Over: Images not the same size\n";
        return false;
    }

    ClearToBlack();
    return false;
}// Comp_Over


///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image "in" the given image.  See lecture notes for 
//  details.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Comp_In(TargaImage* pImage)
{
    if (width != pImage->width || height != pImage->height)
    {
        cout << "Comp_In: Images not the same size\n";
        return false;
    }

    ClearToBlack();
    return false;
}// Comp_In


///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image "out" the given image.  See lecture notes for 
//  details.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Comp_Out(TargaImage* pImage)
{
    if (width != pImage->width || height != pImage->height)
    {
        cout << "Comp_Out: Images not the same size\n";
        return false;
    }

    ClearToBlack();
    return false;
}// Comp_Out


///////////////////////////////////////////////////////////////////////////////
//
//      Composite current image "atop" given image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Comp_Atop(TargaImage* pImage)
{
    if (width != pImage->width || height != pImage->height)
    {
        cout << "Comp_Atop: Images not the same size\n";
        return false;
    }

    ClearToBlack();
    return false;
}// Comp_Atop


///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image with given image using exclusive or (XOR).  Return
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Comp_Xor(TargaImage* pImage)
{
    if (width != pImage->width || height != pImage->height)
    {
        cout << "Comp_Xor: Images not the same size\n";
        return false;
    }

    ClearToBlack();
    return false;
}// Comp_Xor


///////////////////////////////////////////////////////////////////////////////
//
//      Calculate the difference bewteen this imag and the given one.  Image 
//  dimensions must be equal.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Difference(TargaImage* pImage)
{
    if (!pImage)
        return false;

    if (width != pImage->width || height != pImage->height)
    {
        cout << "Difference: Images not the same size\n";
        return false;
    }// if

    for (int i = 0; i < width * height * 4; i += 4)
    {
        unsigned char        rgb1[3];
        unsigned char        rgb2[3];

        RGBA_To_RGB(data + i, rgb1);
        RGBA_To_RGB(pImage->data + i, rgb2);

        data[i] = abs(rgb1[0] - rgb2[0]);
        data[i + 1] = abs(rgb1[1] - rgb2[1]);
        data[i + 2] = abs(rgb1[2] - rgb2[2]);
        data[i + 3] = 255;
    }

    return true;
}// Difference


///////////////////////////////////////////////////////////////////////////////
//
//      Perform 5x5 box filter on this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Filter(float filter[5][5],float filter_div, unsigned char* output_data)
{
    unsigned char* original_data = new unsigned char[width * height * 4];
    for (int i = 0; i < (width * height * 4); i++)
    {
        original_data[i] = output_data[i];
    }
    for (int i = 0; i < (width * height * 4); i = i + 4)
    {
        float avg_red = 0, avg_green = 0, avg_blue = 0;
        
        for (int h = 0; h < 5; h++)
        {
            for (int w = 0; w < 5; w++)
            {
                int box_pos = i + ((h - 2) * width * 4) + ((w - 2) * 4);
                int row = box_pos / (width * 4);
                int col = box_pos % (width * 4) / 4;
                if (row >= 2 && row < height - 2 && col >= 2 && col < width - 2)
                {
                                avg_red += original_data[box_pos] * filter[h][w];
                                avg_green += original_data[box_pos + 1] * filter[h][w];
                                avg_blue += original_data[box_pos + 2] * filter[h][w];
                }
            }
        }
        int new_red, new_green, new_blue;
        if ((int)avg_red > 0)
            new_red = int(float(avg_red / filter_div) + 0.5);
        else
            new_red = 0;
        if ((int)avg_green > 0)
            new_green = int(float(avg_green / filter_div) + 0.5);
        else
            new_green = 0;
        if ((int)avg_blue > 0)
            new_blue = int(float(avg_blue / filter_div) + 0.5);
        else
            new_blue = 0;
        output_data[i] = new_red;
        output_data[i + 1] = new_green;
        output_data[i + 2] = new_blue;
    }
    delete[] original_data;
    return true;
}
bool TargaImage::Filter_Box()
{
    float filter_div = 25.0;
    float filter[5][5] = { {1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1} };
    Filter(filter, filter_div, data);
    return true;
}// Filter_Box


///////////////////////////////////////////////////////////////////////////////
//
//      Perform 5x5 Bartlett filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Filter_Bartlett()
{
    float filter_div = 81.0;
    float filter[5][5] = { {1, 2, 3, 2, 1},
                           {2, 4, 6, 4, 2},
                           {3, 6, 9, 6, 3},
                           {2, 4, 6, 4, 2},
                           {1, 2, 3, 2, 1} };
    Filter(filter, filter_div, data);
    return true;
}// Filter_Bartlett


///////////////////////////////////////////////////////////////////////////////
//
//      Perform 5x5 Gaussian filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Filter_Gaussian()
{
    float filter_div = 256.0;
    float filter[5][5] = { {1, 4, 6, 4, 1},
                           {4, 16, 24, 16, 4},
                           {6, 24, 36, 24, 6},
                           {4, 16, 24, 16, 4},
                           {1, 4, 6, 4, 1} };
    Filter(filter, filter_div, data);
    return true;
}// Filter_Gaussian

///////////////////////////////////////////////////////////////////////////////
//
//      Perform NxN Gaussian filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////

bool TargaImage::Filter_Gaussian_N( unsigned int N )
{
    float** filter = new float* [N];
    for (int i = 0; i < N; i++) {
        filter[i] = new float[N];
    }
    // Compute binomial coefficients
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == 0 || i == N - 1 || j == 0 || j == N - 1) {
                filter[i][j] = 1;
            }
            else if (i == 1 || i == N - 2 || j == 1 || j == N - 2) {
                filter[i][j] = N - 1;
            }
            else {
                filter[i][j] = (N - 1) * (N - 1);
            }
        }
    }
    // Normalize filter coefficients
    float filter_div = 0.0f;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            filter_div += filter[i][j];
        }
    }

    // Apply filter
    unsigned char* original_data = new unsigned char[width * height * 4];
    for (int i = 0; i < (width * height * 4); i++) {
        original_data[i] = data[i];
    }

    for (int i = 0; i < (width * height * 4); i = i + 4) {
        float avg_red = 0, avg_green = 0, avg_blue = 0;

        for (int h = 0; h < N; h++) {
            for (int w = 0; w < N; w++) {
                int box_pos = i + ((h - (N / 2)) * width * 4) + ((w - (N / 2)) * 4);
                int row = box_pos / (width * 4);
                int col = box_pos % (width * 4) / 4;
                if (row >= (N / 2) && row < height - (N / 2) && col >= (N / 2) && col < width - (N / 2)) {
                    avg_red += original_data[box_pos] * filter[h][w];
                    avg_green += original_data[box_pos + 1] * filter[h][w];
                    avg_blue += original_data[box_pos + 2] * filter[h][w];
                }
            }
        }
        int new_red, new_green, new_blue;
        if ((int)avg_red > 0)
            new_red = int(float(avg_red / filter_div) + 0.5);
        else
            new_red = 0;
        if ((int)avg_green > 0)
            new_green = int(float(avg_green / filter_div) + 0.5);
        else
            new_green = 0;
        if ((int)avg_blue > 0)
            new_blue = int(float(avg_blue / filter_div) + 0.5);
        else
            new_blue = 0;
        data[i] = new_red;
        data[i + 1] = new_green;
        data[i + 2] = new_blue;
    }

    delete[] original_data;
    // Delete dynamically allocated memory for the filter
    for (int i = 0; i < N; i++) {
        delete[] filter[i];
    }
    delete[] filter;
    return true;
}// Filter_Gaussian_N


///////////////////////////////////////////////////////////////////////////////
//
//      Perform 5x5 edge detect (high pass) filter on this image.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Filter_Edge()
{
    float filter_div = 256.0;
    float filter[5][5] = { {-1, -4, -6, -4, -1},
                           {-4, -16, -24, -16, -4},
                           {-6, -24, 220, -24, -6},
                           {-4, -16, -24, -16, -4},
                           {-1, -4, -6, -4, -1} };
    Filter(filter, filter_div, data);
    return true;
}// Filter_Edge



///////////////////////////////////////////////////////////////////////////////
//
//      Perform a 5x5 enhancement filter to this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Filter_Enhance()
{
    unsigned char* output_data = new unsigned char[width * height * 4];
    float filter_div = 256.0;
    float filter[5][5] = { {-1, -4, -6, -4, -1},
                           {-4, -16, -24, -16, -4},
                           {-6, -24, 220, -24, -6},
                           {-4, -16, -24, -16, -4},
                           {-1, -4, -6, -4, -1} };
    Filter(filter, filter_div, output_data);
    for (int i = 0; i < (width * height * 4); i++)
    {
        data[i] += output_data[i];
    }
    delete[] output_data;
    return true;
}// Filter_Enhance


///////////////////////////////////////////////////////////////////////////////
//
//      Run simplified version of Hertzmann's painterly image filter.
//      You probably will want to use the Draw_Stroke funciton and the
//      Stroke class to help.
// Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::NPR_Paint()
{
    ClearToBlack();
    return false;
}



///////////////////////////////////////////////////////////////////////////////
//
//      Halve the dimensions of this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Reconstruct(float filter[3][3], float filter_div, unsigned char* output_data)
{
    unsigned char* original_data = new unsigned char[width * height * 4];
    for (int i = 0; i < (width * height * 4); i++)//copy output_data
    {
        original_data[i] = output_data[i];
    }
    for (int i = 0; i < height / 2; i++)//looping by height
    {
        int offset = i * width * 4 * 2;//offset to point to the height in data
        for (int j = 0; j < width / 2; j++)//looping by width
        {
            int p = offset + j * 4 * 2;
            float avg_red = 0, avg_green = 0, avg_blue = 0;

            for (int h = 0; h < 3; h++)
            {
                for (int w = 0; w < 3; w++)
                {
                    int box_pos = p + ((h - 1) * width * 4) + ((w - 1) * 4);
                    int row = i * 2 + (h - 1);
                    int col = j * 2 + (w - 1);
                    if (row >= 0 && row < height && col >= 0 && col < width)
                    {
                        avg_red += original_data[box_pos] * filter[h][w];
                        avg_green += original_data[box_pos + 1] * filter[h][w];
                        avg_blue += original_data[box_pos + 2] * filter[h][w];
                    }
                }
            }
            int new_red, new_green, new_blue;
            if ((int)avg_red > 0)
                new_red = int(float(avg_red / filter_div) + 0.5);
            else
                new_red = 0;
            if ((int)avg_green > 0)
                new_green = int(float(avg_green / filter_div) + 0.5);
            else
                new_green = 0;
            if ((int)avg_blue > 0)
                new_blue = int(float(avg_blue / filter_div) + 0.5);
            else
                new_blue = 0;
            int q = i * (width / 2) * 4 + j * 4;
            output_data[q] = new_red;
            output_data[q + 1] = new_green;
            output_data[q + 2] = new_blue;
            output_data[q + 3] = 255; // set alpha to 255
        }
    }
    delete[] original_data;
    return true;
}

bool TargaImage::Half_Size()
{
    float filter_div = 16.0;
    float filter[3][3] = { {1, 2, 1},
                           {2, 4, 2},
                           {1, 2, 1} };
    Reconstruct(filter, filter_div, data);
    width /= 2;
    height /= 2;
    return true;
}// Half_Size


///////////////////////////////////////////////////////////////////////////////
//
//      Double the dimensions of this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Double_Size()
{
    ClearToBlack();
    return false;
}// Double_Size


///////////////////////////////////////////////////////////////////////////////
//
//      Scale the image dimensions by the given factor.  The given factor is 
//  assumed to be greater than one.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Resize(float scale)
{
    ClearToBlack();
    return false;
}// Resize


//////////////////////////////////////////////////////////////////////////////
//
//      Rotate the image clockwise by the given angle.  Do not resize the 
//  image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
bool TargaImage::Rotate(float angleDegrees)
{
    ClearToBlack();
    return false;
}// Rotate


//////////////////////////////////////////////////////////////////////////////
//
//      Given a single RGBA pixel return, via the second argument, the RGB
//      equivalent composited with a black background.
//
///////////////////////////////////////////////////////////////////////////////
void TargaImage::RGBA_To_RGB(unsigned char *rgba, unsigned char *rgb)
{
    const unsigned char	BACKGROUND[3] = { 0, 0, 0 };

    unsigned char  alpha = rgba[3];

    if (alpha == 0)
    {
        rgb[0] = BACKGROUND[0];
        rgb[1] = BACKGROUND[1];
        rgb[2] = BACKGROUND[2];
    }
    else
    {
	    float	alpha_scale = (float)255 / (float)alpha;
	    int	val;
	    int	i;

	    for (i = 0 ; i < 3 ; i++)
	    {
	        val = (int)floor(rgba[i] * alpha_scale);
	        if (val < 0)
		    rgb[i] = 0;
	        else if (val > 255)
		    rgb[i] = 255;
	        else
		    rgb[i] = val;
	    }
    }
}// RGA_To_RGB


///////////////////////////////////////////////////////////////////////////////
//
//      Copy this into a new image, reversing the rows as it goes. A pointer
//  to the new image is returned.
//
///////////////////////////////////////////////////////////////////////////////
TargaImage* TargaImage::Reverse_Rows(void)
{
    unsigned char   *dest = new unsigned char[width * height * 4];
    TargaImage	    *result;
    int 	        i, j;

    if (! data)
    	return NULL;

    for (i = 0 ; i < height ; i++)
    {
	    int in_offset = (height - i - 1) * width * 4;
	    int out_offset = i * width * 4;

	    for (j = 0 ; j < width ; j++)
        {
	        dest[out_offset + j * 4] = data[in_offset + j * 4];
	        dest[out_offset + j * 4 + 1] = data[in_offset + j * 4 + 1];
	        dest[out_offset + j * 4 + 2] = data[in_offset + j * 4 + 2];
	        dest[out_offset + j * 4 + 3] = data[in_offset + j * 4 + 3];
        }
    }

    result = new TargaImage(width, height, dest);
    delete[] dest;
    return result;
}// Reverse_Rows


///////////////////////////////////////////////////////////////////////////////
//
//      Clear the image to all black.
//
///////////////////////////////////////////////////////////////////////////////
void TargaImage::ClearToBlack()
{
    memset(data, 0, width * height * 4);
}// ClearToBlack


///////////////////////////////////////////////////////////////////////////////
//
//      Helper function for the painterly filter; paint a stroke at
// the given location
//
///////////////////////////////////////////////////////////////////////////////
void TargaImage::Paint_Stroke(const Stroke& s) {
   int radius_squared = (int)s.radius * (int)s.radius;
   for (int x_off = -((int)s.radius); x_off <= (int)s.radius; x_off++) {
      for (int y_off = -((int)s.radius); y_off <= (int)s.radius; y_off++) {
         int x_loc = (int)s.x + x_off;
         int y_loc = (int)s.y + y_off;
         // are we inside the circle, and inside the image?
         if ((x_loc >= 0 && x_loc < width && y_loc >= 0 && y_loc < height)) {
            int dist_squared = x_off * x_off + y_off * y_off;
            if (dist_squared <= radius_squared) {
               data[(y_loc * width + x_loc) * 4 + 0] = s.r;
               data[(y_loc * width + x_loc) * 4 + 1] = s.g;
               data[(y_loc * width + x_loc) * 4 + 2] = s.b;
               data[(y_loc * width + x_loc) * 4 + 3] = s.a;
            } else if (dist_squared == radius_squared + 1) {
               data[(y_loc * width + x_loc) * 4 + 0] = 
                  (data[(y_loc * width + x_loc) * 4 + 0] + s.r) / 2;
               data[(y_loc * width + x_loc) * 4 + 1] = 
                  (data[(y_loc * width + x_loc) * 4 + 1] + s.g) / 2;
               data[(y_loc * width + x_loc) * 4 + 2] = 
                  (data[(y_loc * width + x_loc) * 4 + 2] + s.b) / 2;
               data[(y_loc * width + x_loc) * 4 + 3] = 
                  (data[(y_loc * width + x_loc) * 4 + 3] + s.a) / 2;
            }
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
//
//      Build a Stroke
//
///////////////////////////////////////////////////////////////////////////////
Stroke::Stroke() {}

///////////////////////////////////////////////////////////////////////////////
//
//      Build a Stroke
//
///////////////////////////////////////////////////////////////////////////////
Stroke::Stroke(unsigned int iradius, unsigned int ix, unsigned int iy,
               unsigned char ir, unsigned char ig, unsigned char ib, unsigned char ia) :
   radius(iradius),x(ix),y(iy),r(ir),g(ig),b(ib),a(ia)
{
}

