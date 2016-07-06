#include <SDL/SDL.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

/* Function Prototypes */
void PrintKeyInfo( SDL_KeyboardEvent *key );
void PrintModifiers( SDLMod mod );

uint32_t val(uint32_t k)
{
    switch(k)
    {
        case SDLK_RIGHT:
            return 1; // A
        break;
        case SDLK_LEFT:
            return 1 << 11; // Y
        break;
        case SDLK_UP:
            return 1 << 10; // X
        break;
        case SDLK_DOWN:
            return 1 << 1;
        break;

        case SDLK_w:
            return 1 << 6; // up
        break;
        case SDLK_a:
            return 1 << 5; // left
        break;
        case SDLK_s:
            return 1 << 7; // down
        break;
        case SDLK_d:
            return 1 << 4; // right
        break;

        case SDLK_RETURN: // start
            return 1 << 3;
        break;

        case SDLK_SPACE:
            return 1 << 2; // select
        break;

        default:
            return 0;
        break;
    }
}

void sendval(uint32_t v)
{
    static int sock = 0;
    static struct sockaddr_in saddr;

    if(sock == 0)
    {
        sock = socket(AF_INET, SOCK_DGRAM, 0);

        struct hostent *server;
        server = gethostbyname("192.168.0.4");
        bzero((char *) &saddr, sizeof(saddr));
        saddr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
          (char *)&saddr.sin_addr.s_addr, server->h_length);
        saddr.sin_port = htons(4950);

    }

    int i = sendto(sock, &v, 4, 0, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in));
}

/* main */
int main( int argc, char *argv[] )
{
    SDL_Event event;
    int quit = 0;
    
    /* Initialise SDL */
    if( SDL_Init( SDL_INIT_VIDEO ) < 0){
        fprintf( stderr, "Could not initialise SDL: %s\n", SDL_GetError() );
        exit( -1 );
    }

    /* Set a video mode */
    if( !SDL_SetVideoMode( 320, 200, 0, 0 ) ){
        fprintf( stderr, "Could not set video mode: %s\n", SDL_GetError() );
        SDL_Quit();
        exit( -1 );
    }

    /* Enable Unicode translation */
    SDL_EnableUNICODE( 1 );

    uint32_t keys = 0;

    /* Loop until an SDL_QUIT event is found */
    while( !quit ){

        /* Poll for events */
        while( SDL_PollEvent( &event ) ){
            
            switch( event.type ){
                /* Keyboard event */
                /* Pass the event data onto PrintKeyInfo() */
                case SDL_KEYDOWN:
                    printf("down %i\n", event.key.keysym.sym);
                    keys |= val(event.key.keysym.sym);
                    sendval(keys);
                break;

                case SDL_KEYUP:
                    printf("up %i\n", event.key.keysym.sym);
                    keys &= ~val(event.key.keysym.sym);
                    sendval(keys);
                    break;

                /* SDL_QUIT event (window close) */
                case SDL_QUIT:
                    quit = 1;
                    break;

                default:
                    break;
            }

        }

    }

    /* Clean up */
    SDL_Quit();
    exit( 0 );
}
