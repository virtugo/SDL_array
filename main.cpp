//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <cmath>

// размеры массива для вывода на экран
#define SIZEX 40
#define SIZEY 23
#define BLOCKSIZE 20 // размер блока
#define MOUNTHEIGHT 15 // высота гор
#define HEROCOLOR 8 // цвет героя
// #define TREE_LEAF_COLOR 2 // цвет листвы
// текст для второй строки меню
#define MODEWALK "walk mode"
#define MODEDIG "dig mode"
#define MODEBUILD "build mode"

int FillArray(int, int, int**, int*, int*, int*); // заполняем массив
int PlusArray(int, int, int**, int, int*); // увеличиваем массив
int DynamicToStatic(int (*)[SIZEY], int, int, int**);
int DrawScreen(int (*)[SIZEY]);
int DrawBlock(int, int, int);
int ClearMemory(int, int, int**); // очищаем память

//перемещение героя
int heroGoRight(int*,int,int,int,int**,int(*)[SIZEY],int*,int*);
int heroGoLeft(int*,int,int,int,int**,int(*)[SIZEY],int*,int*);

// PUBLIC VARIABLES
char text1[10]="0"; // верхняя строка меню
int intInventory = 0;
int gameMode = 0; // режим игры: ходить, копать, строить
#define MODEMAX 1 // максимальный индекс режима игры (для удобства здесь)

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );

		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );

		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

bool init();
bool loadMedia();
void close();
SDL_Texture* loadTexture( std::string path );
SDL_Window* gWindow = NULL; //The window we'll be rendering to
SDL_Renderer* gRenderer = NULL; //The window renderer
TTF_Font *gFont = NULL; //Globally used font
LTexture gTextTexture1; // TEXT1
LTexture gTextTexture2; // TEXT2

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
	init();
    //Load media
    loadMedia();

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
                    intInventory=0;
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
                    heroGoRight(&heroX,heroY,dynXM,dynYM,dynWorld,stWorld,&behindHero,&mapMove);
                }
                // ВЛЕВО
                else if(e.key.keysym.sym==SDLK_a){
                    heroGoLeft(&heroX,heroY,dynXM,dynYM,dynWorld,stWorld,&behindHero,&mapMove);
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
                        dynWorld[heroX][heroY]=HEROCOLOR;
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
                        dynWorld[heroX][heroY]=HEROCOLOR;
                        DynamicToStatic(stWorld, mapMove, dynYM, dynWorld);
                    }
                }
                else if(e.key.keysym.sym==SDLK_t){
                    intInventory++;
                    /*
                    // меняем текстуру
                    SDL_Color textColor = { 0, 0, 0 };
                    gTextTexture1.loadFromRenderedText( text1, textColor );
                    */
                }
                else if(e.key.keysym.sym==SDLK_1){
                    if(gameMode<MODEMAX)gameMode++;
                    else gameMode=0;
                }
            }
        }
        // выводим изображение на экран
        DrawScreen(stWorld);
        //Update screen
        SDL_RenderPresent( gRenderer );
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
            border2 = (rand() % MOUNTHEIGHT)+2; // 23456 верхний блок(в неперевёрнутой системе координат
            raznitsa = border2 - border1;
            if((raznitsa==1)||(raznitsa==-1)||(raznitsa==0)){
                y=((YM-1)-border2)+1; // ищем Y
                for(j=(YM-1);j>y;j--){
                    dynWorld[i][j] = 6; // земля
                }
                dynWorld[i][y] = 9; // трава наверху
                // координата героя
                if(i==SIZEX/2){
                    dynWorld[i][y-1] = HEROCOLOR; // цвет героя
                    *heroX=i;
                    *heroY=y-1;
                }
                // генератор деревьев
                if((i>0)&&(i<(XM-1))){
                    treeChance = rand() % 20;
                    if(treeChance==1){
                        //два коричневых и зеленые блоки
                        dynWorld[i][y-1] = 10;
                        dynWorld[i][y-2] = 10;
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
                    if(biomType==0)dynWorld[i][y] = 9; // трава наверху
                    else if (biomType==1)dynWorld[i][y] = 1; // снег
                    else if (biomType==2)dynWorld[i][y] = 7; // песок
                    // генератор деревьев
                    if((i>0)&&(i<(XM-1))){
                        treeChance = rand() % 20;
                        if(treeChance==1){
                            if((biomType==0)||
                               (biomType==1)){
                                //два коричневых
                                dynWorld[i][y-1] = 10;
                                dynWorld[i][y-2] = 10;
                            }
                            // пустыня - кактусы
                            else if(biomType==2){
                                dynWorld[i][y-1] = 2;
                                dynWorld[i][y-2] = 2;
                            }
                            // обычный биом - листья
                            if(biomType==0){
                                //зеленые блоки
                                dynWorld[i][y-4] = 2;
                                dynWorld[i][y-3] = 2;
                                dynWorld[i-1][y-3] = 2;
                                dynWorld[i+1][y-3] = 2;
                            }
                            // снежный биом - снег на дереве
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

    // выводим текст
    //gTextTexture1.render( ( SCREEN_WIDTH - gTextTexture1.getWidth() ) / 2, ( SCREEN_HEIGHT - gTextTexture1.getHeight() ) / 2 );
    itoa(intInventory, text1, 10);
    SDL_Color textColor = { 0, 0, 0 };
    gTextTexture1.loadFromRenderedText( text1, textColor );
    gTextTexture1.render(0,0);
    // вторая строка
    if(gameMode==0)gTextTexture2.loadFromRenderedText( MODEWALK, textColor );
    else if(gameMode==1)gTextTexture2.loadFromRenderedText( MODEDIG, textColor );
    gTextTexture2.render(0,25);

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
    // коричневый (темный)
    }else if(blockType==6){
        SDL_SetRenderDrawColor( gRenderer, 136, 0, 21, 0 );
    // желтый (пустыня)
    }else if(blockType==7){
        //SDL_SetRenderDrawColor( gRenderer, 239, 228, 176, 0 );
        SDL_SetRenderDrawColor( gRenderer, 255, 201, 14, 0 );
    // лиловый (фиолетовый)
    }else if(blockType==8){
        SDL_SetRenderDrawColor( gRenderer, 163, 73, 164, 0 );
    // светло-зеленый (трава)
    }else if(blockType==9){
        SDL_SetRenderDrawColor( gRenderer, 40, 215, 92, 0 );
    // светло-коричневый (ствол дерева)
    }else if(blockType==10){
        SDL_SetRenderDrawColor( gRenderer, 185, 122, 87, 0 );
    // черный blockType==0
    }
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

				//Initialize SDL_ttf
				if( TTF_Init() == -1 )
				{
					printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
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
	//Open the font
	gFont = TTF_OpenFont( "arial.ttf", 28 );

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
	TTF_Quit();
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

// ========= FONT ==========
LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

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
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface == NULL )
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}
	else
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}

	//Return success
	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}
void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

// ПЕРЕМЕЩЕНИЕ ГЕРОЯ - ВПРАВО
int heroGoRight(int *heroX,int heroY, int dynXM, int dynYM,int **dynWorld, int stWorld[][SIZEY], int *behindHero,int *mapMove){
    if(*heroX<(dynXM-1)){ // граница
        // закрашиваем предыдущий блок
        dynWorld[*heroX][heroY]=*behindHero;
        *heroX=*heroX+1;
        //запоминаем цвет следующего блока
        *behindHero=dynWorld[*heroX][heroY];
        //перемещаем героя (закрашиваем блок)
        dynWorld[*heroX][heroY]=HEROCOLOR;
        //двигаем карту, если можно
        if(*mapMove<(dynXM-SIZEX)&&
            (*heroX>SIZEX/2))*mapMove=*mapMove+1;
        DynamicToStatic(stWorld, *mapMove, dynYM, dynWorld);
    }
    return 0;
}

// ПЕРЕМЕЩЕНИЕ ГЕРОЯ - ВЛЕВО
int heroGoLeft(int *heroX,int heroY, int dynXM, int dynYM,int **dynWorld, int stWorld[][SIZEY], int *behindHero,int *mapMove){
    if(*heroX>0){ // граница
        // закрашиваем предыдущий блок
        dynWorld[*heroX][heroY]=*behindHero;
        *heroX=*heroX-1;
        //запоминаем цвет следующего блока
        *behindHero=dynWorld[*heroX][heroY];
        //перемещаем героя (закрашиваем блок)
        dynWorld[*heroX][heroY]=HEROCOLOR;
        //двигаем карту, если можно
        if(*mapMove>0&&
           *heroX<(dynXM-SIZEX/2))*mapMove=*mapMove-1;
        DynamicToStatic(stWorld, *mapMove, dynYM, dynWorld);
    }

    return 0;
}
