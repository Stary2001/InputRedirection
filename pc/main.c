#include <stdio.h>
#include <SDL2/SDL.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#define JOY_DEADZONE 1000
#define CPAD_BOUND 0x5d0

#define BIT(n) (1U<<(n))

enum
{
	KEY_A       = BIT(0),       ///< A
	KEY_B       = BIT(1),       ///< B
	KEY_SELECT  = BIT(2),       ///< Select
	KEY_START   = BIT(3),       ///< Start
	KEY_DRIGHT  = BIT(4),       ///< D-Pad Right
	KEY_DLEFT   = BIT(5),       ///< D-Pad Left
	KEY_DUP     = BIT(6),       ///< D-Pad Up
	KEY_DDOWN   = BIT(7),       ///< D-Pad Down
	KEY_R       = BIT(8),       ///< R
	KEY_L       = BIT(9),       ///< L
	KEY_X       = BIT(10),      ///< X
	KEY_Y       = BIT(11),      ///< Y
	KEY_ZL      = BIT(14),      ///< ZL (New 3DS only)
	KEY_ZR      = BIT(15),      ///< ZR (New 3DS only)
	KEY_TOUCH   = BIT(20),      ///< Touch (Not actually provided by HID)
	KEY_CSTICK_RIGHT = BIT(24), ///< C-Stick Right (New 3DS only)
	KEY_CSTICK_LEFT  = BIT(25), ///< C-Stick Left (New 3DS only)
	KEY_CSTICK_UP    = BIT(26), ///< C-Stick Up (New 3DS only)
	KEY_CSTICK_DOWN  = BIT(27), ///< C-Stick Down (New 3DS only)
	KEY_CPAD_RIGHT = BIT(28),   ///< Circle Pad Right
	KEY_CPAD_LEFT  = BIT(29),   ///< Circle Pad Left
	KEY_CPAD_UP    = BIT(30),   ///< Circle Pad Up
	KEY_CPAD_DOWN  = BIT(31),   ///< Circle Pad Down

	// Generic catch-all directions
	KEY_UP    = KEY_DUP    | KEY_CPAD_UP,    ///< D-Pad Up or Circle Pad Up
	KEY_DOWN  = KEY_DDOWN  | KEY_CPAD_DOWN,  ///< D-Pad Down or Circle Pad Down
	KEY_LEFT  = KEY_DLEFT  | KEY_CPAD_LEFT,  ///< D-Pad Left or Circle Pad Left
	KEY_RIGHT = KEY_DRIGHT | KEY_CPAD_RIGHT, ///< D-Pad Right or Circle Pad Right
};

int16_t circle_x = 0;
int16_t circle_y = 0;
uint32_t hid_buttons = 0;

int8_t touching = 0;
int16_t touch_x = 0;
int16_t touch_y = 0;

int sock_fd = 0;
struct sockaddr_in sock_addr;

int connect_to_3ds(const char *addr)
{
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct hostent *server;
    server = gethostbyname(addr);
    if(server == NULL)
    {
    	return 0;
    }
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    memcpy(&sock_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    sock_addr.sin_port = htons(4950);

    return 1;
}

void send_frame()
{
	char v[12];
	uint32_t hid_state = ~hid_buttons;
	hid_state ^= (0xf << 12);
	uint32_t circle_state = 0x7FF7FF;
	uint32_t touch_state = 0x02000000;
	if(circle_x != 0 || circle_y != 0) // Do circle magic. 0x5d0 is the upper/lower bound of circle pad input
	{
		uint32_t x = circle_x;
		uint32_t y = circle_y;
		x = ((x * CPAD_BOUND) / 32768) + 2048;
		y = ((y * CPAD_BOUND) / 32768) + 2048;
		circle_state = (x | (y << 12)) + 0x01000000;
	}

	if(touching) // This is good enough.
	{
		uint32_t x = touch_x;
		uint32_t y = touch_y;
		x = (x * 4096) / 320;
		y = (y * 4096) / 240;
		touch_state = x | (y << 12) | (0x01 << 24);
	}

	memcpy(v, &hid_state, 4);
    memcpy(v + 4, &touch_state, 4);
    memcpy(v + 8, &circle_state, 4);

    int i = sendto(sock_fd, v, 12, 0, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_in));
}

void set(uint32_t a, uint32_t b)
{
	if(b)
	{
		hid_buttons |= a;
	}
	else
	{
		hid_buttons &= ~a;
	}
}

int main(int argc, char ** argv)
{
	if(argc < 2)
	{
		printf("usage: %s [ip]\n", argv[0]);
		return 1;
	}

	if(!connect_to_3ds(argv[1]))
	{
		printf("failed to connect to '%s'!\n", argv[1]);
		return 1;
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	printf("%i detected\n", SDL_NumJoysticks());
	if(SDL_NumJoysticks() == 0)
	{
		printf("no joysticks..\n");
	    SDL_Quit();
		return 0;
	}

	SDL_Window *win = SDL_CreateWindow("sdl thing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 240, 0);
	SDL_Renderer *sdlRenderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);

	SDL_Texture *screen_tex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
	SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);

	SDL_Joystick *joy = SDL_JoystickOpen(0);
	SDL_JoystickEventState(SDL_ENABLE);

	int i = 0;

	SDL_Event ev;

	int run = 1;

	while(run)
	{
		while(SDL_PollEvent(&ev))
		{
			switch(ev.type)
			{
				case SDL_QUIT:
					run = 0;
				break;

				case SDL_JOYAXISMOTION:
				{
					int16_t v = ev.jaxis.value;
					if(abs(v) < JOY_DEADZONE)
					{
						v = 0;
					}

					if(ev.jaxis.axis == 0)
					{
						circle_x = v;
					}
					else if(ev.jaxis.axis == 1)
					{
						if(v == -32768)
						{
							v++; // -32768 when negated is......itself?
						}
						circle_y = -v; // Y is inverted.
					}
					else
					{
						printf("unk axis %i val %i\n", ev.jaxis.axis, ev.jaxis.value);
					}
				}
				break;

				case SDL_JOYBUTTONDOWN:
				{
					uint8_t b = ev.jbutton.button;
					switch(b)
					{
						case 2: hid_buttons |= KEY_A; break; // a
						case 1: hid_buttons |= KEY_B; break; // b
						case 3: hid_buttons |= KEY_X; break; // x
						case 0: hid_buttons |= KEY_Y; break; // y
						case 4: hid_buttons |= KEY_L; break; // L
						case 5: hid_buttons |= KEY_R; break; // R
						case 8: hid_buttons |= KEY_SELECT; break; // select
						case 9: hid_buttons |= KEY_START; break; // start
						default:
							printf("unk button up %i\n", b);
						break;
					}
				}
				break;

				case SDL_JOYBUTTONUP:
				{
					uint8_t b = ev.jbutton.button;
					switch(b)
					{
						case 2: hid_buttons &= ~KEY_A; break; // a
						case 1: hid_buttons &= ~KEY_B; break; // b
						case 3: hid_buttons &= ~KEY_X; break; // x
						case 0: hid_buttons &= ~KEY_Y; break; // y
						case 4: hid_buttons &= ~KEY_L; break; // L
						case 5: hid_buttons &= ~KEY_R; break; // R
						case 8: hid_buttons &= ~KEY_SELECT; break; // select
						case 9: hid_buttons &= ~KEY_START; break; // start
						default:
							printf("unk button up %i\n", b);
						break;
					}
				}
				break;

				case SDL_JOYHATMOTION:
				{
					uint8_t v = ev.jhat.value;
					set(KEY_DUP, v & SDL_HAT_UP);
					set(KEY_DDOWN, v & SDL_HAT_DOWN);
					set(KEY_DLEFT, v & SDL_HAT_LEFT);
					set(KEY_DRIGHT, v & SDL_HAT_RIGHT);
				}
				break;

				case SDL_MOUSEBUTTONDOWN:
					touching = 1;
					touch_x = ev.button.x;
					touch_y = ev.button.y;
				break;

				case SDL_MOUSEMOTION:
					touch_x = ev.motion.x;
					touch_y = ev.motion.y;
				break;

				case SDL_MOUSEBUTTONUP:
					touching = 0;
					touch_x = 0;
					touch_y = 0;
				break;

				case SDL_KEYDOWN:
					if(ev.key.keysym.sym == SDLK_ESCAPE)
					{
						run = 0;
					}
				break;
			}
		}

		send_frame();

		SDL_RenderClear(sdlRenderer);
		SDL_RenderPresent(sdlRenderer);
	}

	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(win);

	SDL_Quit();
}