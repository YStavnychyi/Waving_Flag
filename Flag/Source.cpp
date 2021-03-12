#define _USE_MATH_DEFINES
#define BITMAP_ID 0x4D42

#include <iostream>
#include <glut.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gl/GL.h>
#include <gl/GLU.h>
using namespace std;

//Zmienne globalne
HDC g_HDC;								//globalny kontekst urzdządzenia
bool fullScreen = false;				//true = tryb pełnoekrankowy
										//false = tryb okienkowy
bool keyPressed[256];					//tablica przyciśnięć klawisz

//Opis tekstury
BITMAPINFOHEADER bitmapInfoHeader;		//naglówek pliku
unsigned char* bitmapData;				//dane tekstury
unsigned int texture;					//obiekt tekstury
//Opis flagi
float flagPoints[36][20][3];			//flaga 36x20
float wrapValue;						//przenosi fali z końca na początek flagi

//Inicjacja punktów flagi za pomocą funkcji sin()
void InitializeFlag()
{
	int x;								//licznik w plaszczyżnie x
	int y;								//licznik w plaszczyżnie y
	float sinTemp;						

	//Przegląda wszystkie punkty flagi w pętli i wyznacza wartość funkcji sin dla
	//współrzędnej z. Wszpółrzędne x i y otrzymują wartość odpowiedniego licznika pętli
	for (x = 0; x < 36; x++)
	{
		for (y = 0; y < 20; y++)
		{
			flagPoints[x][y][0] = (float)x;
			flagPoints[x][y][1] = (float)y;
			sinTemp = (((float)x * 20.0f) / 360.0f) * 2.0f * M_PI;
			flagPoints[x][y][2] = (float)sin(sinTemp);
		}
	}
}

unsigned char *LoadBitmapFile(const char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE* filePtr;                      //wskażnik pliku
    BITMAPFILEHEADER bitmapFileHeader;  //naglówek pliku
    unsigned char* bitmapImage;         //bufor obrazu
    int imageIdx = 0;                   //licznik bajtów obrazu
    unsigned char tempRGB;              //zmienna zamiany składowych

    //otwiera plik w trybie read binary
    filePtr = fopen(filename, "rb");
    if (filePtr == NULL)
        return NULL;

    //wczytuje nagłówek pliku
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

    //sprawdza, czy rzeczywiście jest to plik BMP
    if (bitmapFileHeader.bfType != BITMAP_ID)
    {
        fclose(filePtr);
        return NULL;
    }

    //wczytuje nagłówek obrazu zapisanego w pliku
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr); 

    //ustawia wskażnik pliku na początku danych opisujących obraz
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    //przydziela pamięć na bufor obrazu
    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

    //sprawdza, czy pamięć została przydzielona
    if (!bitmapImage)
    {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    //wczytuje dane obrazu
    fread(bitmapImage, bitmapInfoHeader->biSizeImage, 1, filePtr);


    //sprawdza czy operacja powiodła się
    if (bitmapImage == NULL)
    {
        fclose(filePtr);
        return NULL;
    }

    //zamienia składowe R i B, aby uzyskać format RGB
    for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3)
    {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }

    //zamyka plik i zwraca wskażnik bufora zawierającego obraz
    fclose(filePtr);
    return bitmapImage;
}

//Zainicjowania grafiki OpenGl
void Initialize()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	//tlo w czarnym kolorze
	glEnable(GL_DEPTH_TEST);				//usuwanie ukrytych powierzchni
	glEnable(GL_CULL_FACE);					//brak obliczeń dla niewidocznych stron wielokątów
	glFrontFace(GL_CCW);					//niewidoczne strony posiadają porządek wierzcholków
											//przeciwny do kierunku ruchu wskazówek zegara
	glEnable(GL_TEXTURE_2D);				//wlącza tekstury 2D

    //ładuje obraz tekstury
    bitmapData = LoadBitmapFile("flag.bmp", &bitmapInfoHeader);

    glGenTextures(1, &texture);             //tworzy obiekt tekstury
    glBindTexture(GL_TEXTURE_2D, texture);  //aktywuje obiekt tekstury

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //tworzy obraz tekstury
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmapInfoHeader.biWidth, bitmapInfoHeader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmapData);
    InitializeFlag();
}

//Obraz flagi dla pojedynczej klatki animacjii
void DrawFlag()
{
    int x;                                  //licznik w kierunku osi x
    int y;                                  //licznik w kierunku osi y
    float texLeft;                          //lewa współrzędna tekstury
    float texBottom;                        //dolna współrzedna tekstury
    float texTop;                           //górna współrzedna tekstury
    float texRight;                         //prawa współrzędna tekstury

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture);  //wybiera obiekt tekstury
    glBegin(GL_QUADS);                      //rysuje czworokąty

    for (x = 0; x < 36; x++)
    {
        for (y = 0; y < 18; y++)
        {
            //wyznacza współrzędne tekstury dla bieżącego czworokąta
            texLeft = float(x) / 35.0f;
            texBottom = float(y) / 18.0f;
            texRight = float(x + 1) / 35.0f;
            texTop = float(y + 1) / 18.0f;

            //lewy dolny wierzcholek
            glTexCoord2f(texLeft, texBottom);
            glVertex3f(flagPoints[x][y][0], flagPoints[x][y][1], flagPoints[x][y][2]);

            //prawy dolny wierzcholek
            glTexCoord2f(texRight, texBottom);
            glVertex3f(flagPoints[x+1][y][0], flagPoints[x+1][y][1], flagPoints[x+1][y][2]);

            //prawy górny wierzcholek
            glTexCoord2f(texRight, texTop);
            glVertex3f(flagPoints[x+1][y+1][0], flagPoints[x+1][y+1][1], flagPoints[x+1][y+1][2]);

            //lewy górny wierzcholek
            glTexCoord2f(texLeft, texTop);
            glVertex3f(flagPoints[x][y+1][0], flagPoints[x][y+1][1], flagPoints[x][y+1][2]);
        }
    }
    glEnd();

    //Udaje ruch flagi
    for (y = 0; y < 19; y++)
    {
        wrapValue = flagPoints[35][y][2];
        for (x = 35; x < 0; x--)
        {
            flagPoints[x][y][2] = flagPoints[x - 1][y][2];
        }
        flagPoints[0][y][2] = wrapValue;
    }
    glPopMatrix();
}

//Przesunięcie układu współrzędnych
void Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(-15.0f, -10.0f, -50.0f);               //wykonuje przesunięcie
    DrawFlag();                                         //rysuje flagę
    glFlush();
    SwapBuffers(g_HDC);                                 //przełącza bufory
}

//funkcja określająca format pikseli
void SetupPixelFormat(HDC hDC)
{

}

