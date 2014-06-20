//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cmath>

// размеры массива для вывода на экран
#define SIZEX 32
#define SIZEY 10
#define BLOCKSIZE 25 // размер блока

int FillArray(int, int, int**, int*, int*, int*); // заполняем массив
int PlusArray(int, int, int**, int, int*); // увеличиваем массив
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
	int dynXM = 32;
	int dynYM = SIZEY;
	int addArr=10; // величина, на которую увеличивается массив(334234)
	// координаты героя
	int heroX=0;
	int heroY=0;
	int behindHero=3; // блок за героем - небо
	int borderHeight=0; // // высота земли на краю карты
	// статический массив
	int stWorld[SIZEX][SIZEY];
	// задаем сдвиг по X (можно поиграть с этим числом для наглядности)
	int mapMove = 0;
	int i;
	// инициализируем динамический массив
	dynWorld = (int **)malloc(sizeof(int *)*dynXM);
	for (i = 0; i<dynXM; i++)dynWorld[i] = (int *)malloc(sizeof(int)*dynYM);
    // ------------------------ ARRAY --------------------------------
    // заполняем массив
    FillArray(dynXM, dynYM, dynWorld, &heroX, &heroY, &borderHeight);
    // передаем часть динамического массива в статический
    DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);

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
					//User presses a key
					//falconpl.org/project_docs/sdl/class_SDLK.html
                    else if( e.type == SDL_KEYDOWN ) {
                        // esc, q - выход
                        if(e.key.keysym.sym==SDLK_ESCAPE ||
                           e.key.keysym.sym==SDLK_q){
                            quit = true;
                        }
                        // генератор карты
                        else if(e.key.keysym.sym==SDLK_r){
                            FillArray(dynXM, dynYM, dynWorld, &heroX, &heroY, &borderHeight);
                            behindHero=3; // небо за героем
                            mapMove=0;
                            DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                        }
                        // увеличиваем массив справа
                        else if(e.key.keysym.sym==SDLK_g){
                            dynXM+=addArr;
                            dynWorld = (int **)realloc((void *)dynWorld, (dynXM+addArr)*sizeof(int *));
                            for (i = (dynXM-addArr); i<dynXM; i++)dynWorld[i] = (int *)malloc(sizeof(int)*dynYM);
                            PlusArray(dynXM, dynYM, dynWorld, addArr, &borderHeight);
                            //DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                        }
                        // карта вправо
                        else if(e.key.keysym.sym==SDLK_RIGHT){
                            if(mapMove<(dynXM-SIZEX)){
                                mapMove++;
                                DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                            }
                        }
                        // карта влево
                        else if(e.key.keysym.sym==SDLK_LEFT){
                            if(mapMove>0){
                                mapMove--;
                                DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                            }
                        }
                        // ВПРАВО
                        else if(e.key.keysym.sym==SDLK_d){
                            if(heroX<(dynXM-1)){ // граница
                                // закрашиваем предыдущий блок
                                dynWorld[heroX][heroY]=behindHero;
                                heroX++;
                                //запоминаем цвет следующего блока
                                behindHero=dynWorld[heroX][heroY];
                                //перемещаем героя (закрашиваем блок)
                                dynWorld[heroX][heroY]=5;
                                //двигаем карту, если можно
                                if(mapMove<(dynXM-SIZEX)&&
                                    (heroX>SIZEX/2))mapMove++;
                                DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                            }
                        }
                        // ВЛЕВО
                        else if(e.key.keysym.sym==SDLK_a){
                            if(heroX>0){ // граница
                                // закрашиваем предыдущий блок
                                dynWorld[heroX][heroY]=behindHero;
                                heroX--;
                                //запоминаем цвет следующего блока
                                behindHero=dynWorld[heroX][heroY];
                                //перемещаем героя (закрашиваем блок)
                                dynWorld[heroX][heroY]=5;
                                //двигаем карту, если можно
                                if(mapMove>0&&
                                   heroX<(dynXM-SIZEX/2))mapMove--;
                                DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                            }
                        }
                        // ВВЕРХ
                        else if(e.key.keysym.sym==SDLK_w){
                            if(heroY>0){ // граница
                                heroY--;
                                //запоминаем цвет следующего блока
                                behindHero=dynWorld[heroX][heroY];
                                //перемещаем героя (закрашиваем блок)
                                dynWorld[heroX][heroY]=5;
                                // закрашиваем предыдущий блок
                                dynWorld[heroX][heroY+1]=behindHero;
                                DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                            }
                        }
                        // ВНИЗ
                        else if(e.key.keysym.sym==SDLK_s){
                            if(heroY<dynYM-1){ // граница
                                heroY++;
                                //запоминаем цвет следующего блока
                                behindHero=dynWorld[heroX][heroY];
                                //перемещаем героя (закрашиваем блок)
                                dynWorld[heroX][heroY]=5;
                                // закрашиваем предыдущий блок
                                dynWorld[heroX][heroY-1]=behindHero;
                                DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                            }
                        }
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
int FillArray(int XM, int YM, int **dynWorld, int *heroX, int *heroY, int *borderHeight)
{
	int i,j,y;
	int border1=0;
	int border2=0;
	int raznitsa=0;
	int flag=0;

	// весь заполняем блоками неба
	for (j = 0; j<YM; j++){
		for (i = 0; i<XM; i++){
			dynWorld[i][j] = 3; // небо
		}
	}

	// генерируем землю

	for(i = 0; i<XM; i++){
        do{
            border2 = rand() % 5; // 01234 верхний блок(в неперевёрнутой системе координат
            raznitsa = border2 - border1;
            if((raznitsa==1)||(raznitsa==-1)||(raznitsa==0)){
                y=(YM-1)-border2; // ищем Y
                for(j=(YM-1);j>=y;j--){
                    dynWorld[i][j] = 4; // сизый
                }
                // координата героя
                if(i==SIZEX/2){
                    dynWorld[i][y-1] = 5; // цвет героя
                    *heroX=i;
                    *heroY=y-1;
                //высота земли на краю карты
                }else if(i==XM-1){
                    *borderHeight=border2;
                }
                flag=1;
            }
        }while(flag!=1);
        border1=border2;
        flag=0;
	}

	return 0;
}

//дополняем массив
int PlusArray(int XM, int YM, int **dynWorld, int addArr, int *borderHeight)
{
	int i,j,y;
	int border1=*borderHeight;
	int border2=0;
	int raznitsa=0;
	int flag=0;

	// весь заполняем блоками неба
	for (j = 0; j<YM; j++){
		for (i = XM-addArr; i<XM; i++){
			dynWorld[i][j] = 3; // небо
		}
	}

	// генерируем землю

	for(i = XM-addArr; i<XM; i++){
        do{
            border2 = rand() % 5; // 01234 верхний блок(в неперевёрнутой системе координат
            raznitsa = border2 - border1;
            if((raznitsa==1)||(raznitsa==-1)||(raznitsa==0)){
                y=(YM-1)-border2; // ищем Y
                for(j=(YM-1);j>=y;j--){
                    dynWorld[i][j] = 4; // сизый
                }
                // высота земли на краю карты
                if(i==XM-1){
                    *borderHeight=border2;
                }
                flag=1;
            }
        }while(flag!=1);
        border1=border2;
        flag=0;
	}

	return 0;
}

// преобразуем динамический массив в статический
int DynamicToStatic(int stWorld[][SIZEY], int mapMove, int YM, int **dynWorld)
{
	int i, j;

	for (j = 0; j<YM; j++){
		for (i = mapMove; i<SIZEX + mapMove; i++){
			stWorld[i - mapMove][j] = dynWorld[i][j];
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
    // голубой
    }else if(blockType==3){
        SDL_SetRenderDrawColor( gRenderer, 153, 217, 234, 0 );
    // сизый
    }else if(blockType==4){
        SDL_SetRenderDrawColor( gRenderer, 112, 146, 190, 0 );
    // синий
    }else if(blockType==5){
        SDL_SetRenderDrawColor( gRenderer, 63, 72, 204, 0 );
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
