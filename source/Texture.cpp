#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}

		delete m_pRValue;
		delete m_pGValue;
		delete m_pBValue;
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		auto surface{ IMG_Load(path.c_str()) };
		return new Texture(surface);
	}

	ColorRGB Texture::Sample(const Vector2& uv)
	{
		const Vector2 scaledUV{ uv.x * m_pSurface->w, uv.y * m_pSurface->h };

		Uint32 pixel{ m_pSurfacePixels[static_cast<int>(scaledUV.y) * m_pSurface->h + static_cast<int>(scaledUV.x)] };

		SDL_GetRGB(pixel, m_pSurface->format, m_pRValue, m_pGValue, m_pBValue);

		return { *m_pRValue / 255.f, *m_pGValue / 255.f, *m_pBValue / 255.f };
	}
}