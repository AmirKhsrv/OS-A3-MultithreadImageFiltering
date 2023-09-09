/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////                                           /////////////////////
/////////////////////       Amirmohammad Khosravi Esfezar       ///////////////////// 
/////////////////////                                           /////////////////////
/////////////////////              S.N.: 810198386              /////////////////////
/////////////////////                                           /////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <bits/stdc++.h>
#include <sys/time.h>

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;

#pragma pack(1)
#pragma once

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef struct tagBITMAPFILEHEADER
{
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

int rows;
int cols;


typedef struct Pixel
{
  int* col = new int [cols];
} Pixel;

typedef struct Channel
{
  Pixel* row = new Pixel [rows];
} Channel;

typedef struct ImageRGB
{
  Channel red;
  Channel blue;
  Channel green;
} ImageRGB;



bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize)
{
  std::ifstream file(fileName);

  if (file)
  {
    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return 1;
  }
  else
  {
    cout << "File" << fileName << " doesn't exist!" << endl;
    return 0;
  }
}

ImageRGB getPixlesFromBMP24(ImageRGB& image, int end, int rows, int cols, char *fileReadBuffer)
{

  int count = 1;
  int extra = cols % 4;
  for (int i = 0; i < rows; i++)
  {
    count += extra;
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        switch (k)
        {
        case 0:
          image.red.row[i].col[j] = (u_int8_t)fileReadBuffer[end - count];
          break;
        case 1:
          image.green.row[i].col[j] =  (u_int8_t)fileReadBuffer[end - count];
          break;
        case 2:
          image.blue.row[i].col[j] =  (u_int8_t)fileReadBuffer[end - count];
          break;
        }
        count++;
      }
  }
  return image;
}

void writeOutBmp24(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize, ImageRGB& image)
{
  std::ofstream write(nameOfFileToCreate);
  if (!write)
  {
    cout << "Failed to write " << nameOfFileToCreate << endl;
    return;
  }
  int count = 1;
  int extra = cols % 4;
  for (int i = 0; i < rows; i++)
  {
    count += extra;
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        switch (k)
        {
        case 0:
          fileBuffer[bufferSize - count] = (char)image.red.row[i].col[j];
          break;
        case 1:
          fileBuffer[bufferSize - count] = (char)image.green.row[i].col[j];
          break;
        case 2:
          fileBuffer[bufferSize - count] = (char)image.blue.row[i].col[j];
          break;
        }
        count++;
      }
  }
  write.write(fileBuffer, bufferSize);
}

u_int8_t calculateNineCellMean(Channel& channel, int i, int j)
{
  return (channel.row[i - 1].col[j - 1] + channel.row[i - 1].col[j] + channel.row[i - 1].col[j + 1] + 
         channel.row[i].col[j - 1] + channel.row[i].col[j] + channel.row[i - 1].col[j + 1] + 
         channel.row[i + 1].col[j - 1] + channel.row[i + 1].col[j] + channel.row[i + 1].col[j + 1]) / 9;
}

ImageRGB filterSmoothing(ImageRGB& image)
{
  ImageRGB resultImage = image;
  for (int i = 1; i < rows - 1; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      resultImage.blue.row[i].col[j] = calculateNineCellMean(image.blue, i, j);
      resultImage.red.row[i].col[j] = calculateNineCellMean(image.red, i, j);
      resultImage.green.row[i].col[j] = calculateNineCellMean(image.green, i, j);
    }
  }
  return resultImage;
}

ImageRGB filterSepia(ImageRGB& image)
{
  ImageRGB resultImage;
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      int red = (image.red.row[i].col[j] * 0.393) + (image.green.row[i].col[j] * 0.769) + (image.blue.row[i].col[j] * 0.189);
      int green = (image.red.row[i].col[j] * 0.349) + (image.green.row[i].col[j] * 0.686) + (image.blue.row[i].col[j] * 0.168);
      int blue = (image.red.row[i].col[j] * 0.272) + (image.green.row[i].col[j] * 0.534) + (image.blue.row[i].col[j] * 0.131);
      red > 255 ? resultImage.red.row[i].col[j] = 255 : resultImage.red.row[i].col[j] = red; 
      green > 255 ? resultImage.green.row[i].col[j] = 255 : resultImage.green.row[i].col[j] = green;
      blue > 255 ? resultImage.blue.row[i].col[j] = 255 : resultImage.blue.row[i].col[j] = blue;
    }
  }
  return resultImage;
}

ImageRGB filterTotalMean(ImageRGB& image)
{
  ImageRGB resultImage;
  int totalSumRed = 0, totalSumGreen = 0, totalSumBlue = 0, totalPixels = 0;
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
    {
      totalSumRed += image.red.row[i].col[j];
      totalSumGreen += image.green.row[i].col[j];
      totalSumBlue += image.blue.row[i].col[j];
      totalPixels++;
    }
  int redMean = totalSumRed / totalPixels;
  int greenMean = totalSumGreen / totalPixels;
  int blueMean = totalSumBlue / totalPixels;
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      resultImage.red.row[i].col[j] = (image.red.row[i].col[j] * 0.4) + (redMean * 0.6); 
      resultImage.green.row[i].col[j] = (image.green.row[i].col[j] * 0.4) + (greenMean * 0.6);
      resultImage.blue.row[i].col[j] = (image.blue.row[i].col[j] * 0.4) + (blueMean * 0.6);
    }
  }
  return resultImage;
}

void addCross(ImageRGB& image)
{
  for (int i = 0; i < rows; i++)
  {
    for (int j = -1; j < 2; j++)
    {
      image.red.row[i].col[i + j] = 255;
      image.green.row[i].col[i + j] = 255;
      image.blue.row[i].col[i + j] = 255;
      image.red.row[rows - 1 - i].col[i + j] = 255;
      image.green.row[rows - 1 - i].col[i + j] = 255;
      image.blue.row[rows - 1 - i].col[i + j] = 255;
    }
  }
}

int main(int argc, char *argv[])
{
  cout << "------------------ SERIAL EXECUTION ------------------" << endl;
  struct timeval execStart, execEnd;
  gettimeofday(&execStart, NULL);
  
  struct timeval start, end;

  gettimeofday(&start, NULL);
  char *fileBuffer;
  int bufferSize;
  char *fileName = argv[1];
  if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
  {
    cout << "File read error" << endl;
    return 1;
  }
  gettimeofday(&end, NULL);
  double readTime = (end.tv_sec - start.tv_sec) * 1e6;
  readTime = (readTime + (end.tv_usec - start.tv_usec)) * 1e-3;
  
  ImageRGB image;

  gettimeofday(&start, NULL);
  image = getPixlesFromBMP24(image, bufferSize, rows, cols, fileBuffer);
  gettimeofday(&end, NULL);
  double savingTime = (end.tv_sec - start.tv_sec) * 1e6;
  savingTime = (savingTime + (end.tv_usec - start.tv_usec)) * 1e-3;
  
  gettimeofday(&start, NULL);
  image = filterSmoothing(image);
  gettimeofday(&end, NULL);
  double smoothingTime = (end.tv_sec - start.tv_sec) * 1e6;
  smoothingTime = (smoothingTime + (end.tv_usec - start.tv_usec)) * 1e-3;

  gettimeofday(&start, NULL);
  image = filterSepia(image);
  gettimeofday(&end, NULL);
  double sepiaTime = (end.tv_sec - start.tv_sec) * 1e6;
  sepiaTime = (sepiaTime + (end.tv_usec - start.tv_usec)) * 1e-3;

  gettimeofday(&start, NULL);
  image = filterTotalMean(image);
  gettimeofday(&end, NULL);
  double totalMeanTime = (end.tv_sec - start.tv_sec) * 1e6;
  totalMeanTime = (totalMeanTime + (end.tv_usec - start.tv_usec)) * 1e-3;

  gettimeofday(&start, NULL);
  addCross(image);
  gettimeofday(&end, NULL);
  double addCrossTime = (end.tv_sec - start.tv_sec) * 1e6;
  addCrossTime = (addCrossTime + (end.tv_usec - start.tv_usec)) * 1e-3;

  gettimeofday(&start, NULL);
  writeOutBmp24(fileBuffer, "output.bmp", bufferSize, image);
  gettimeofday(&end, NULL);
  double writeTime = (end.tv_sec - start.tv_sec) * 1e6;
  writeTime = (writeTime + (end.tv_usec - start.tv_usec)) * 1e-3;

  gettimeofday(&execEnd, NULL);
  double serialExecTime = (execEnd.tv_sec - execStart.tv_sec) * 1e6;
  serialExecTime = (serialExecTime + (execEnd.tv_usec - execStart.tv_usec)) * 1e-3;
  

  cout << std::fixed << (int)serialExecTime << std::setprecision(2) << " ms" << endl;

  cout << "Time taken for reading the file (fillAndAllocate): " << std::fixed << readTime << " ms" << endl;
  cout << "Time taken for saving data (getPixlesFromBMP24): " << std::fixed << savingTime  << " ms" << endl;
  cout << "Time taken for smoothing filter (filterSmoothing): " << std::fixed << smoothingTime << " ms" << endl;
  cout << "Time taken for sepia filter (filterSepia): " << std::fixed << sepiaTime << " ms" << endl;
  cout << "Time taken for total mean filter (filterTotalMean): " << std::fixed << totalMeanTime << " ms" << endl;
  cout << "Time taken for add cross (addCross): " << std::fixed << addCrossTime << " ms" << endl;
  cout << "Time taken for writing the file (writeOutBmp24): " << std::fixed << writeTime << " ms" << endl;
  cout << "Time taken for whole serial executoin (mili second): " << std::fixed << serialExecTime << " ms" << endl;
  cout << "------------------------------------------------------" << endl;
  return 0;
}

// another data structure : 
// int*** image = new int**[3];
// image[0] = new int*[rows];
// image[1] = new int*[rows];
// image[2] = new int*[rows];
// for (int i = 0; i < rows; i++)
// {
//   image[0][i] = new int[cols];
//   image[1][i] = new int[cols];
//   image[2][i] = new int[cols];
// }

/*
AMIRMOHAMMAD KHOSRAVI 810198386
░░░░░░░░░░░░░░░░░░░░░▄▀░░▌
░░░░░░░░░░░░░░░░░░░▄▀▐░░░▌
░░░░░░░░░░░░░░░░▄▀▀▒▐▒░░░▌
░░░░░▄▀▀▄░░░▄▄▀▀▒▒▒▒▌▒▒░░▌
░░░░▐▒░░░▀▄▀▒▒▒▒▒▒▒▒▒▒▒▒▒█
░░░░▌▒░░░░▒▀▄▒▒▒▒▒▒▒▒▒▒▒▒▒▀▄
░░░░▐▒░░░░░▒▒▒▒▒▒▒▒▒▌▒▐▒▒▒▒▒▀▄
░░░░▌▀▄░░▒▒▒▒▒▒▒▒▐▒▒▒▌▒▌▒▄▄▒▒▐
░░░▌▌▒▒▀▒▒▒▒▒▒▒▒▒▒▐▒▒▒▒▒█▄█▌▒▒▌
░▄▀▒▐▒▒▒▒▒▒▒▒▒▒▒▄▀█▌▒▒▒▒▒▀▀▒▒▐░░░▄
▀▒▒▒▒▌▒▒▒▒▒▒▒▄▒▐███▌▄▒▒▒▒▒▒▒▄▀▀▀▀
▒▒▒▒▒▐▒▒▒▒▒▄▀▒▒▒▀▀▀▒▒▒▒▄█▀░░▒▌▀▀▄▄
▒▒▒▒▒▒█▒▄▄▀▒▒▒▒▒▒▒▒▒▒▒░░▐▒▀▄▀▄░░░░▀
▒▒▒▒▒▒▒█▒▒▒▒▒▒▒▒▒▄▒▒▒▒▄▀▒▒▒▌░░▀▄
▒▒▒▒▒▒▒▒▀▄▒▒▒▒▒▒▒▒▀▀▀▀▒▒▒▄▀
*/