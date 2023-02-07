#include "Texture.h"

void Texture::destroy()
{
	SDL_FreeSurface(surface);
}
