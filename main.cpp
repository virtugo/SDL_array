//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cmath>

// размеры массива для вывода на экран
#define SIZEX 32
#define SIZEY 10
#define BLOCKSIZE 25 // размер блока

int FillArray(int, int, int**); // заполняем массив
int DynamicToStatic(int (*)[SIZEY], int, int, int**);
int DrawScreen(int (*)[SIZEY]);
int DrawBlock(int, int, int);
int ClearMemory(int, int, int**); // очищаем память

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

bool init();
bool loadMedia();
void close();
SDL_Texture* loadTexture( std::string path );
SDL_Window* gWindow = NULL; //The window we'll be rendering to
SDL_Renderer* gRenderer = NULL; //The window renderer

// ====================================== MAIN ======================================

int main( int argc, char* args[] )
{
	// динамический массив
	int **dynWorld;
	int dynXM = 100;
	int dynYM = SIZEY;
	// статический массив
	int stWorld[SIZEX][SIZEY];
	// задаем сдвиг по X (можно поиграть с этим числом для наглядности)
	int sdvig = 0;
	int i;
	// инициализируем динамический массив
	dynWorld = (int **)malloc(sizeof(int *)*dynXM);
	for (i = 0; i<dynXM; i++)dynWorld[i] = (int *)malloc(sizeof(int)*dynYM);
    // ------------------------ ARRAY --------------------------------
    // заполняем массив
    FillArray(dynXM, dynYM, dynWorld);
    // передаем часть динамического массива в статический
    DynamicToStatic(stWorld, sdvig, dynYM, dynWorld);

    // ======================= SDL PART ==============================
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					}
				}

                // выводим изображение на экран
				DrawScreen(stWorld);

				//Update screen
				SDL_RenderPresent( gRenderer );
			}
		}
	}

	//Free resources and close SDL
	close();
	// ======================= </SDL PART> ==============================

    // освобождаем память
    ClearMemory(dynXM, dynYM, dynWorld);

	return 0;
}

// ====================================== OTHER ======================================
// заполняем массив
int FillArray(int XM, int YM, int **dynWorld)
{
	int i, j;
	int border=0;

	// весь заполняем белым цветом
	for (j = 0; j<YM; j++){
		for (i = 0; i<XM; i++){
			dynWorld[i][j] = 1; // белые
		}
	}

	// генерируем землю

	for(i = 0; i<XM; i++){
        border = rand() % 5; // 01234 верхний блок(в неперевёрнутой системе координат
        j=(YM-1)-border; // ищем Y
        dynWorld[i][j] = 2; // зеленый
	}

	return 0;
}

// преобразуем динамический массив в статический
int DynamicToStatic(int stWorld[][SIZEY], int sdvig, int YM, int **dynWorld)
{
	int i, j;

	for (j = 0; j<YM; j++){
		for (i = sdvig; i<SIZEX + sdvig; i++){
			stWorld[i - sdvig][j] = dynWorld[i][j];
		}
	}

	return 0;
}

// выводим изображение на экран
int DrawScreen(int stWorld[][SIZEY])
{
	int i, j;
    int x1,y1;

    //Очищаем экран
    //wiki.libsdl.org/SDL_SetRenderDrawColor
    SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0 ); // черный цвет
    SDL_RenderClear( gRenderer );

    // выводим в цикле
    for (j = 0; j<SIZEY; j++){
        y1=j*BLOCKSIZE;
        for (i = 0; i<SIZEX; i++){
            x1=i*BLOCKSIZE;
            DrawBlock(stWorld[i][j], x1,y1);
        }
    }

    return 0;
}

int DrawBlock(int blockType, int x1, int y1)
{
    SDL_Rect fillRect = { x1, y1, BLOCKSIZE, BLOCKSIZE }; //x,y,ширина, длина

    // белый
    if(blockType==1){
        SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 0 );
    // зеленый
    }else if(blockType==2){
        SDL_SetRenderDrawColor( gRenderer, 34, 177, 76, 0 );
    }
    // черный blockType==0
    else SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0 );

    SDL_RenderFillRect( gRenderer, &fillRect );

    return 0;
}

// чистим память
int ClearMemory(int XM, int YM, int **dynWorld)
{
	int i;

	for (i = 0; i<XM; i++)free(dynWorld[i]);
	free((void *)dynWorld);

	return 0;
}
// --------------------------- SDL -------------------------------
//Starts up SDL and creates window
bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "THE GAME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

//Loads media
bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Nothing to load
	return success;
}
//Frees media and shuts down SDL
void close()
{
	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}
//Loads individual image as texture
SDL_Texture* loadTexture( std::string path )
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	return newTexture;
}
