//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <cmath>

// размеры массива для вывода на экран
#define SIZEX 50
#define SIZEY 10
#define BLOCKSIZE 20 // размер блока
#define MOUNTHEIGHT 5 // высота гор

int FillArray(int, int, int**, int*, int*, int*); // заполняем массив
int PlusArray(int, int, int**, int, int*); // увеличиваем массив
int DynamicToStatic(int (*)[SIZEY], int, int, int**);
int DrawScreen(int (*)[SIZEY]);
int DrawBlock(int, int, int);
int ClearMemory(int, int, int**); // очищаем память

//Screen dimension constants
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 200;

bool init();
bool loadMedia();
void close();
SDL_Texture* loadTexture( std::string path );
SDL_Window* gWindow = NULL; //The window we'll be rendering to
SDL_Renderer* gRenderer = NULL; //The window renderer

// ====================================== MAIN ======================================

int main( int argc, char* args[] )
{
    srand(time(NULL)); // randomize
	// динамический массив
	int **dynWorld;
	int dynXM = SIZEX;
	int dynYM = SIZEY;
	int addArr= SIZEX; // величина, на которую увеличивается массив
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
                            dynXM=SIZEX;
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
                                // закрашиваем предыдущий блок
                                dynWorld[heroX][heroY]=behindHero;
                                heroY--;
                                //запоминаем цвет следующего блока
                                behindHero=dynWorld[heroX][heroY];
                                //перемещаем героя (закрашиваем блок)
                                dynWorld[heroX][heroY]=5;
                                DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                            }
                        }
                        // ВНИЗ
                        else if(e.key.keysym.sym==SDLK_s){
                            if(heroY<dynYM-1){ // граница
                                // закрашиваем предыдущий блок
                                dynWorld[heroX][heroY]=behindHero;
                                heroY++;
                                //запоминаем цвет следующего блока
                                behindHero=dynWorld[heroX][heroY];
                                //перемещаем героя (закрашиваем блок)
                                dynWorld[heroX][heroY]=5;
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
	int border1=3;
	int border2=border1;
	int raznitsa=0;
	int flag=0;
	int treeChance=0;

	// весь заполняем блоками неба
	for (j = 0; j<YM; j++){
		for (i = 0; i<XM; i++){
			dynWorld[i][j] = 3; // небо
		}
	}

	// генерируем землю

	for(i = 0; i<XM; i++){
        do{
            border2 = (rand() % 5) + 2; // 23456 верхний блок(в неперевёрнутой системе координат
            raznitsa = border2 - border1;
            if((raznitsa==1)||(raznitsa==-1)||(raznitsa==0)){
                y=((YM-1)-border2)+1; // ищем Y
                for(j=(YM-1);j>y;j--){
                    dynWorld[i][j] = 6; // земля
                }
                dynWorld[i][y] = 2; // трава наверху
                // координата героя
                if(i==SIZEX/2){
                    dynWorld[i][y-1] = 5; // цвет героя
                    *heroX=i;
                    *heroY=y-1;
                }
                // генератор деревьев
                if((i>0)&&(i<(XM-1))){
                    treeChance = rand() % 20;
                    if(treeChance==1){
                        //два коричневых и зеленые блоки
                        dynWorld[i][y-1] = 6;
                        dynWorld[i][y-2] = 6;
                        dynWorld[i][y-4] = 2;
                        dynWorld[i][y-3] = 2;
                        dynWorld[i-1][y-3] = 2;
                        dynWorld[i+1][y-3] = 2;
                    }
                }
                flag=1;
            }
        }while(flag!=1);
        border1=border2;
        flag=0;
	}
	//высота земли на краю карты
    *borderHeight=border2;

	return 0;
}

//дополняем массив
int PlusArray(int XM, int YM, int **dynWorld, int addArr, int *borderHeight)
{
	int i,j,y;
	int border1=*borderHeight;
	int border2=border1;
	int raznitsa=0;
	int flag=0;
	int treeChance=0;
    int biomType=0;
    int bankHeight=*borderHeight; // высота берега моря

	// весь заполняем блоками неба
	for (j = 0; j<YM; j++){
		for (i = XM-addArr; i<XM; i++){
			dynWorld[i][j] = 3; // небо
		}
	}
	// случайно выбираем тип биома
	biomType = rand() % 4;
	// biomType = 3; // отладка
	// 0 - обычный
	// 1 - снег
	// 2 - пустыня
	// 3 - море

	// генерируем землю

	for(i = XM-addArr; i<XM; i++){
        // если морской биом
        if(biomType==3){
            // дно
            // если начало биома, то на 1 ниже,
            // чем последний предыдущего
            // если уже не на дне
            // начало биома
            if(i<(bankHeight-1)+(XM-addArr)){
                if(border1>1)
                {
                    if(i!=XM-addArr)border2--;
                    //border2--;
                    y=YM-border2; // ищем Y (координата земди)
                    // заполняем верх водой
                    for(j=y-1;j>=YM-bankHeight;j--)
                        dynWorld[i][j] = 5;
                    // заполняем низ землей
                    for(j=YM-1;j>=y;j--)
                        dynWorld[i][j] = 6;
                }
                // если уже на дне, остаемся там
                else
                {
                    border2=border1=1;
                    y=YM-border2; // ищем Y
                    dynWorld[i][YM-1] = 6;
                    // рисуем воду
                    for(j=YM-2;j>=YM-bankHeight;j--)
                        dynWorld[i][j] = 5;
                }
            }
            // конец биома
            else if (i>XM-bankHeight){
                // повышаем дно
                border2++;
                y=YM-border2; // ищем Y
                // заполняем низ землей
                for(j=YM-1;j>=y;j--)
                    dynWorld[i][j] = 6;
                // заполняем верх водой
                for(j=y-1;j>=YM-bankHeight;j--)
                    dynWorld[i][j] = 5;
            }
            // если в середине биома, остаемся на дне
            else
            {
                border1=border2=1;
                dynWorld[i][YM-1] = 6;
                // рисуем воду
                for(j=YM-2;j>=YM-bankHeight;j--)
                    dynWorld[i][j] = 5;
            }
            border1=border2;
        }
        // если биомы: обычный, снег, пустыня
        else if(biomType==0 ||
                biomType==1 ||
                biomType==2)
        {
            do{
                border2 = (rand() % MOUNTHEIGHT)+2; // 23456 верхний блок(в неперевёрнутой системе координат
                raznitsa = border2 - border1;
                if((raznitsa==1)||(raznitsa==-1)||(raznitsa==0)){
                    y=YM-border2; // ищем Y
                    // заполняем низ землей
                    for(j=YM-1;j>y;j--){
                        if((biomType==0)||
                            (biomType==1))
                            dynWorld[i][j] = 6; // земля
                        else if(biomType==2)
                            dynWorld[i][j] = 7; // песок
                    }
                    if(biomType==0)dynWorld[i][y] = 2; // трава наверху
                    else if (biomType==1)dynWorld[i][y] = 1; // снег
                    else if (biomType==2)dynWorld[i][y] = 7; // песок
                    // генератор деревьев
                    if((i>0)&&(i<(XM-1))){
                        treeChance = rand() % 20;
                        if(treeChance==1){
                            if((biomType==0)||
                               (biomType==1)){
                                //два коричневых
                                dynWorld[i][y-1] = 6;
                                dynWorld[i][y-2] = 6;
                            }
                            // пустыня - кактусы
                            else if(biomType==2){
                                dynWorld[i][y-1] = 2;
                                dynWorld[i][y-2] = 2;
                            }
                            if(biomType==0){
                                //зеленые блоки
                                dynWorld[i][y-4] = 2;
                                dynWorld[i][y-3] = 2;
                                dynWorld[i-1][y-3] = 2;
                                dynWorld[i+1][y-3] = 2;
                            }
                            else if (biomType==1){
                                dynWorld[i][y-4] = 1;
                                dynWorld[i][y-3] = 1;
                                dynWorld[i-1][y-3] = 1;
                                dynWorld[i+1][y-3] = 1;
                            }
                        }
                    }
                    flag=1;
                }

            }while(flag!=1);
        }
        border1=border2;
        flag=0;
	}
    //высота земли на краю карты
    *borderHeight=border2;
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
        SDL_SetRenderDrawColor( gRenderer, 211, 239, 245, 0 );
    // сизый
    }else if(blockType==4){
        SDL_SetRenderDrawColor( gRenderer, 112, 146, 190, 0 );
    // синий
    }else if(blockType==5){
        SDL_SetRenderDrawColor( gRenderer, 63, 72, 204, 0 );
    // коричневый
    }else if(blockType==6){
        SDL_SetRenderDrawColor( gRenderer, 136, 0, 21, 0 );
    // желтый (пустыня)
    }else if(blockType==7){
        //SDL_SetRenderDrawColor( gRenderer, 239, 228, 176, 0 );
        SDL_SetRenderDrawColor( gRenderer, 255, 201, 14, 0 );

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
