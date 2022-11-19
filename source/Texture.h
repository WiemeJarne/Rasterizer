#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) ;

	private:
		Texture(SDL_Surface* pSurface);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };

		//pixel color to pass to the SDL_GetRGB fucntion
		Uint8 *m_pRValue{ new Uint8() }, *m_pGValue{ new Uint8() }, *m_pBValue{ new Uint8() };
	};
}