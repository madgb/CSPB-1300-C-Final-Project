/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Woobin Huh

- All project requirements fully met? (YES or NO):
    Yes

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    No
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


//
// YOUR FUNCTION DEFINITIONS HERE
//

void process_1(string image_str, string output_image)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    vector<vector<Pixel>> process_1(width, vector<Pixel> (height));
    
    for(int row = 0; row < width; row++)
    {
        for(int col = 0; col < height; col++)
        {
            double distance = sqrt( pow((col - width/2), 2) + pow((row - height/2), 2));
            double scaling_factor = (height - distance)/height;
            int red = image[row][col].red * scaling_factor;
            int green = image[row][col].green * scaling_factor;
            int blue = image[row][col].blue * scaling_factor;
                
            Pixel pixel = {
                red, 
                green, 
                blue
            };
            process_1[row][col] = pixel;
        }
    }
    
    bool success = write_image(output_image, process_1);
}

void process_2(string image_str, string output_image, double scaling_factor)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    vector<vector<Pixel>> process_2(width, vector<Pixel> (height));

    for(int row = 0; row < width; row++)
    {
        for(int col = 0; col < height; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;
            double average_value = (red_value + green_value + blue_value) / 3.0;

            int red, green, blue;

            if (average_value >= 170)
            {
                red = int(255 - (255 - red_value) * scaling_factor);
                green = int(255 - (255 - green_value) * scaling_factor);
                blue = int(255 - (255 - blue_value) * scaling_factor);
            }
            else if (average_value < 90)
            {
                red = red_value * scaling_factor;
                green = green_value * scaling_factor;
                blue =  blue_value * scaling_factor;
            }
            else {
                red = red_value;
                green = green_value;
                blue =  blue_value;
            }
                
            Pixel pixel = {
                red, 
                green, 
                blue
            };
            process_2[row][col] = pixel;
        }
    }

    bool success = write_image(output_image, process_2);
}

void process_3(string image_str, string output_image)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    vector<vector<Pixel>> process_3(width, vector<Pixel> (height));

    for(int row = 0; row < width; row++)
    {
        for(int col = 0; col < height; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            int gray_value = (red_value + green_value + blue_value) / 3;

            int red = gray_value;
            int green = gray_value;
            int blue = gray_value;
                
            Pixel pixel = {
                red, 
                green, 
                blue
            };
            process_3[row][col] = pixel;
        }
    }

    bool success = write_image(output_image, process_3);
}

void process_4(string image_str, string output_image)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    vector<vector<Pixel>> process_4(height, vector<Pixel> (width));

    for(int row = 0; row < height; row++)
    {
        for(int col = 0; col < width; col++)
        {
            int red = image[col][row].red;
            int green = image[col][row].green;
            int blue = image[col][row].blue;
                
            Pixel pixel = {
                red, 
                green, 
                blue
            };
            process_4[row][(width - 1) - col] = pixel;
        }
    }

    bool success = write_image(output_image, process_4);
}

void process_5(string image_str, string output_image, int spins)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    // spin = 0, 1, 2, 3
    while (spins >= 4)
    {
        spins = spins - 4;
    }

    int new_width, new_height;

    // 90 or 270 degree spin
    if (spins % 2 != 0)
    {
        new_width = height;
        new_height = width;
    }
    // 0 or 180 degree spin
    else
    {
        new_width = width;
        new_height = height;
    };

    vector<vector<Pixel>> process_5(new_width, vector<Pixel> (new_height));


    for(int row = 0; row < new_width; row++)
    {
        for(int col = 0; col < new_height; col++)
        {
            int new_row, new_col, red, green, blue;
            if (spins % 2 != 0)
            {
                if (spins == 1)
                {
                    new_row = row;
                    new_col = (width - 1) - col;
                }
                else
                {
                    new_row = (height - 1) - row;
                    new_col = col;
                }
                red = image[col][row].red;
                green = image[col][row].green;
                blue = image[col][row].blue;
            }
            else
            {
                if (spins == 0)
                {
                    new_row = row;
                    new_col = col;
                }
                else
                {
                    new_row = (width - row) - 1;
                    new_col = (height - col) - 1;
                }
                red = image[row][col].red;
                green = image[row][col].green;
                blue = image[row][col].blue;
            }
                
            Pixel pixel = {
                red, 
                green,
                blue
            };

            process_5[new_row][new_col] = pixel;
        }
    }

    bool success = write_image(output_image, process_5);
}

void process_6(string image_str, string output_image, int x_scale, int y_scale)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    vector<vector<Pixel>> process_6(width * y_scale, vector<Pixel> (height * x_scale));

    for(int row = 0; row < width * y_scale; row++)
    {
        for(int col = 0; col < height * x_scale; col++)
        {
            int new_row = row/y_scale;
            int new_col = col/x_scale;
            int red = image[new_row][new_col].red;
            int green = image[new_row][new_col].green;
            int blue = image[new_row][new_col].blue;
                
            Pixel pixel = {
                red, 
                green, 
                blue
            };
            process_6[row][col] = pixel;
        }
    }

    bool success = write_image(output_image, process_6);
}

void process_7(string image_str, string output_image)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    vector<vector<Pixel>> process_7(width, vector<Pixel> (height));

    for(int row = 0; row < width; row++)
    {
        for(int col = 0; col < height; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            int gray_value = (red_value + green_value + blue_value) / 3;

            int red, green, blue;
            if (gray_value >= 255/2)
            {
                red = 255;
                green = 255;
                blue = 255;
            }
            else
            {
                red = 0;
                green = 0;
                blue = 0;
            }
                
            Pixel pixel = {
                red, 
                green, 
                blue
            };
            process_7[row][col] = pixel;
        }
    }

    bool success = write_image(output_image, process_7);
}

void process_8(string image_str, string output_image, double scaling_factor)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    vector<vector<Pixel>> process_8(width, vector<Pixel> (height));

    for(int row = 0; row < width; row++)
    {
        for(int col = 0; col < height; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            int red = 255 - ((255 - red_value) * scaling_factor);
            int green = 255 - ((255 - green_value) * scaling_factor);
            int blue = 255 - ((255 - blue_value) * scaling_factor);

                
            Pixel pixel = {
                red, 
                green, 
                blue
            };
            process_8[row][col] = pixel;
        }
    }

    bool success = write_image(output_image, process_8);
}

void process_9(string image_str, string output_image, double scaling_factor)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    vector<vector<Pixel>> process_9(width, vector<Pixel> (height));

    for(int row = 0; row < width; row++)
    {
        for(int col = 0; col < height; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            int red = red_value * scaling_factor;
            int green = green_value * scaling_factor;
            int blue = blue_value * scaling_factor;

                
            Pixel pixel = {
                red, 
                green, 
                blue
            };
            process_9[row][col] = pixel;
        }
    }

    bool success = write_image(output_image, process_9);
}

void process_10(string image_str, string output_image)
{
    vector<vector<Pixel>> image = read_image(image_str);
    int width = image.size();
    int height = image[0].size();

    vector<vector<Pixel>> process_10(width, vector<Pixel> (height));

    for(int row = 0; row < width; row++)
    {
        for(int col = 0; col < height; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;
            int max_color = red_value;

            int max_array[3] = {red_value, green_value, blue_value};
            for (int i = 0; i < 3; i++) {
                if (max_array[i] > max_color)
                {
                    max_color = max_array[i];
                }
            }

            int red, green, blue;

            if (red_value + green_value + blue_value >= 550)
            {
                red = 255;
                green = 255;
                blue = 255;
            }
            else if (red_value + green_value + blue_value <= 150)
            {
                red = 0;
                green = 0;
                blue = 0;
            }
            else if (max_color == red_value)
            {
                red = 255;
                green = 0;
                blue = 0;
            }
            else if (max_color == green_value)
            {
                red = 0;
                green = 255;
                blue = 0;
            }
            else
            {
                red = 0;
                green = 0;
                blue = 255;
            }

            Pixel pixel = {
                red, 
                green, 
                blue
            };
            process_10[row][col] = pixel;
        }
    }

    bool success = write_image(output_image, process_10);
}

void show_messages(string filename)
{
    cout << "IMAGE PROCESSING MENU" << endl;
    cout << "0) Change image (current: " << filename << ")" << endl;
    cout << "1) Vignette" << endl;
    cout << "2) Clarendon" << endl;
    cout << "3) Grayscale" << endl;
    cout << "4) Rotate 90 degrees" << endl;
    cout << "5) Rotate multiple 90 degrees" << endl;
    cout << "6) Enlarge" << endl;
    cout << "7) High contrast" << endl;
    cout << "8) Lighten" << endl;
    cout << "9) Darken" << endl;
    cout << "10) Black, white, red, green, blue" << endl;
    cout << "\n\n" << endl;
    cout << "Enter menu selection (Q to quit): " << endl;
}

int main()
{
    cout << "CSPB 1300 Image Processing Application" << endl;
    cout << "Enter input BMP filename: " << endl;
    string filename;
    cin >> filename;


    show_messages(filename);

    int selection;
    while (cin >> selection)
    {
        if (selection == 0)
        {
            cout << "Change image selected" << endl;
            cout << "Enter new input BMP filename: " << endl;
            cin >> filename;
            cout << "Successfully changed input image into: " << filename << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }
        if (selection == 1)
        {
            cout << "Vignette selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            process_1(filename, output_filename);

            cout << "Successfully applied vignette!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }

        if (selection == 2)
        {
            cout << "Clarendon selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            double scaling_factor;
            cout << "Enter scaling factor: (ex: 0.3)" << endl;
            cin >> scaling_factor;

            process_2(filename, output_filename, scaling_factor);

            cout << "Successfully applied clarendon!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }

        if (selection == 3)
        {
            cout << "Grayscale selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            process_3(filename, output_filename);

            cout << "Successfully applied grayscale!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }

        if (selection == 4)
        {
            cout << "Rotate 90 degrees selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            process_4(filename, output_filename);

            cout << "Successfully applied 90 degree rotation!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }

        if (selection == 5)
        {
            cout << "Rotate multiple 90 degrees selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            int spins;
            cout << "Enter number of 90 degree rotations: (ex: 3)" << endl;
            cin >> spins;

            process_5(filename, output_filename, spins);

            cout << "Successfully applied multiple 90 degree rotations!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }

        if (selection == 6)
        {
            cout << "Enlarge selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            int x_scale;
            cout << "Enter X scale: (ex: 2)" << endl;
            cin >> x_scale;

            int y_scale;
            cout << "Enter Y scale: (ex: 3)" << endl;
            cin >> y_scale;

            process_6(filename, output_filename, x_scale, y_scale);

            cout << "Successfully enlarged!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }

        if (selection == 7)
        {
            cout << "High contrast selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            process_7(filename, output_filename);

            cout << "Successfully applied high contrast!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }

        if (selection == 8)
        {
            cout << "Lighten selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            double scaling_factor;
            cout << "Enter scaling factor: (ex: 0.5)" << endl;
            cin >> scaling_factor;

            process_8(filename, output_filename, scaling_factor);

            cout << "Successfully lightened!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }

        if (selection == 9)
        {
            cout << "Darken selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            double scaling_factor;
            cout << "Enter scaling factor: (ex: 0.5)" << endl;
            cin >> scaling_factor;

            process_9(filename, output_filename, scaling_factor);

            cout << "Successfully darkened!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }

        if (selection == 10)
        {
            cout << "Black, white, red, green, blue selected" << endl;

            string output_filename;
            cout << "Enter output BMP filename: (ex: myimage.bmp)" << endl;
            cin >> output_filename;

            process_10(filename, output_filename);

            cout << "Successfully applied black, white, red, green, blue filter!" << endl;
            cout << "\n\n" << endl;
            show_messages(filename);
        }
    }

    cout << "Thank you for using my program!" << endl;
    cout << "Quitting..." << endl;

    return 0;
}