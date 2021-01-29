#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_timer.h>

#define WINDOW_W 800
#define WINDOW_H 600

typedef struct window{
	int w;
	int h;
}window;

window win = {WINDOW_W, WINDOW_H};

SDL_Color color = {255, 255, 255, 255};

#define WIN_POINTS 5
#define FONT_SIZE 12
#define PLATFORM_W 10
#define PLATFORM_H 120
#define BALL_SIZE 20
#define PLAYER_SPEED 6
#define BALL_SPEED 6

char buffer[255];

SDL_Rect opponent = {
				 WINDOW_W - 50 - PLATFORM_W,
				 (WINDOW_H - PLATFORM_H) / 2,
				 PLATFORM_W, 
				 PLATFORM_H};

SDL_Rect player = {
				50,
				(WINDOW_H - PLATFORM_H) / 2,
				PLATFORM_W,
				PLATFORM_H};

SDL_Rect ball = {
				WINDOW_W - BALL_SIZE,
				WINDOW_H - BALL_SIZE,
				BALL_SIZE,
				BALL_SIZE};

SDL_Rect score_rect = {
				0,
				0,
				500, 40};

int are_colliding(SDL_Rect a, SDL_Rect b){
	if (b.x <= a.x + a.w)
		if (a.x <= b.x + b.w)
			if (b.y <= a.y + a.h)
				if (a.y <= b.y + b.h)
					return 1;
	return 0;
}

void player_movement(int* direction){
	player.y += PLAYER_SPEED * *direction * win.h / WINDOW_H;
	if (player.y > win.h - player.h) player.y = win.h - player.h;
	if (player.y < 0) player.y = 0;
}

void opponent_movement(int* ball_direction_x, int* ball_direction_y){
	opponent.y += (ball.y - opponent.y) * (win.h / WINDOW_H) / (BALL_SPEED);
	if (opponent.y + opponent.h > win.h) opponent.y = win.h - opponent.h;
}

void ball_movement(int* ball_direction_x, int* ball_direction_y, int* direction, int* multi, float* angle,
				   int *score, int *opponent_score, int *highscore){
	if (are_colliding(ball, player)){
		*ball_direction_x *= -1;
		ball.x = player.x + player.w;
		*multi = 1;
		*angle = (float)(player.y + player.h / 2 - ball.y + ball.h / 2) / 100;
	}else if (ball.x < 0){
		ball.x = win.w/2;
		ball.y = win.h/2;
		*ball_direction_x *= -1;
		*ball_direction_y *= -1;
		*multi = 2;
		*angle = (float)(rand() % 100) / 100;
		++*opponent_score;
	}
	
	if (are_colliding(ball, opponent)){
		*ball_direction_x *= -1;
		ball.x = opponent.x - ball.w;
		*multi = 1;
		*angle = (float)(opponent.y + opponent.h / 2 - ball.y + ball.h / 2) / 100;
	}else if (ball.x > win.w){
		ball.x = win.w/2;
		ball.y = win.h/2;
		*ball_direction_x *= -1;
		*ball_direction_y *= -1; 
		*multi = 2;
		*angle = (float)(rand() % 100) / 100;
		++*score;
	}
	if(ball.y < 0 || ball.y + ball.h > win.h){
		*ball_direction_y *= -1;
	}
	ball.x += *ball_direction_x * BALL_SPEED / *multi * (win.w / WINDOW_W);
	ball.y += *ball_direction_y * BALL_SPEED / *multi * (win.h / WINDOW_H * *angle);
	SDL_Delay(8);
}

void function_draw(SDL_Renderer* renderer, SDL_Surface* s, TTF_Font* font, int score, int opponent_score, int highscore){
	SDL_GetRendererOutputSize(renderer, &(win.w), &(win.h));
	SDL_SetRenderDrawColor(renderer,   0,   0,   0,   0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &player);
   	
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	opponent.x = win.w - opponent.w - 50;
	SDL_RenderFillRect(renderer, &opponent);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &ball);
	
	memset(buffer, 0, 255);
	sprintf(buffer, "   Score: %d    Opponent Score: %d    Highscore: %d   ", score, opponent_score, highscore);
	SDL_Surface *player_score_surface = TTF_RenderText_Solid(font, buffer, color); 
	
	SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, player_score_surface);
	SDL_QueryTexture(t, NULL, NULL, &score_rect.w, &score_rect.h);
	SDL_RenderCopy(renderer, t, NULL, &score_rect);	

	SDL_FreeSurface(player_score_surface);
	
	SDL_RenderPresent(renderer);
	SDL_DestroyTexture(t);
}

void program_quit(SDL_Window* w, SDL_Renderer* renderer){
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(w);
	TTF_Quit();
	SDL_Quit();
	exit(0);
}

void pause(SDL_Event event, SDL_Window* w, SDL_Renderer* renderer, SDL_Surface* s,
	 	   int score, int opponent_score, int highscore, TTF_Font* font){	
	while(1){
		while (SDL_PollEvent(&event)){
			if (event.type == SDL_QUIT){
				program_quit(w, renderer);
			}else if (event.type == SDL_KEYUP){
				switch(event.key.keysym.scancode){
					case SDL_SCANCODE_P:
						return ;
				}
			}
		}
	function_draw(renderer, s, font, score, opponent_score, highscore);
	}
}
void has_someone_won(FILE* f, int score, int opponent_score, int highscore, SDL_Renderer* r, SDL_Window* w){
	if (opponent_score >= WIN_POINTS){
		if (score >= highscore){
			highscore = score;
			fflush(f);
			fprintf(f, "%d\n", highscore);
		}
		fclose(f);
		printf("HL3_GAMEOVER_SCORE\n");
		program_quit(w, r);
	}
}


int main(){ 
	
	FILE *f = fopen("./highscore.txt", "r+");
	int ball_direction_x = 1;
	int	ball_direction_y = 1;
	int direction = 0;
	int multi = 1;
	float angle = 1;
	int score = -1, opponent_score = 0;
	int c;
	int highscore = 0;
	
	while ((c = fgetc(f)) != '\n'){
		highscore *= 10;
		highscore += c - '0';
	}
	rewind(f);


	TTF_Init();
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* w = SDL_CreateWindow("pong", 
						 SDL_WINDOWPOS_UNDEFINED, 
   						 SDL_WINDOWPOS_UNDEFINED,
   						 win.w,
   						 win.h, 
   						 SDL_WINDOW_RESIZABLE);

	Uint32 render_flags = SDL_RENDERER_ACCELERATED;
	SDL_Renderer* renderer = SDL_CreateRenderer(w, -1, render_flags);
	
	SDL_Surface* s;
	
	TTF_Font *font = TTF_OpenFont("./OpenSans-BoldItalic.ttf", 20);
	if (font == NULL) printf("TTF_OpenFont: %s\n", TTF_GetError()); 	

	function_draw(renderer, s, font, score, opponent_score, highscore);
	
	//SDL_FreeSurface(s);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	SDL_Event event;

	//main loop
	while (1){	
		while (SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_QUIT: 
					program_quit(w, renderer);
				break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode){
						case SDL_SCANCODE_UP:
							direction = -1;
						break;
						case SDL_SCANCODE_DOWN:
							direction = 1;
						break;
					}
				break;
				case SDL_KEYUP:
					switch (event.key.keysym.scancode){
						case SDL_SCANCODE_P:
							pause(event, w, renderer, s, score, opponent_score, highscore, font);
						break;
						case SDL_SCANCODE_UP:
						case SDL_SCANCODE_DOWN:
							direction = 0;
						break;
					}
				break;
			}	
		}
		has_someone_won(f, score, opponent_score, highscore, renderer, w);
		player_movement(&direction);
		ball_movement(&ball_direction_x, &ball_direction_y, &direction, &multi, &angle, 
					  &score, &opponent_score, &highscore);
		opponent_movement(&ball_direction_x, &ball_direction_y);
		function_draw(renderer, s, font, score, opponent_score, highscore);
	}
}
